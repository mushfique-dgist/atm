#include "Atm.hpp"

#include <iostream>

#include "Account.hpp"
#include "Bank.hpp"
#include "Card.hpp"
#include "Transaction.hpp"

static const int CASH_BILL_VALUES[CASH_TYPE_COUNT] = {1000, 5000, 10000, 50000};

namespace {

bool BuildWithdrawalBundle(long long amount, const CashDrawer& inventory, CashDrawer& bundle) {
    bundle = CashDrawer();
    long long remaining = amount;
    for (int i = CASH_TYPE_COUNT - 1; i >= 0; --i) {
        int billValue = CASH_BILL_VALUES[i];
        long long needed = remaining / billValue;
        if (needed <= 0) {
            continue;
        }
        long long inStock = inventory.noteCounts[i];
        if (inStock > needed) {
            inStock = needed;
        }
        int usable = static_cast<int>(inStock);
        bundle.noteCounts[i] = usable;
        remaining -= static_cast<long long>(usable) * billValue;
    }
    return remaining == 0;
}

} // namespace

ATMFees ATMFees::CreateDefault() {
    ATMFees fees;
    fees.depositPrimary = 0;
    fees.depositNonPrimary = 1000;
    fees.withdrawalPrimary = 1000;
    fees.withdrawalNonPrimary = 2000;
    fees.transferPrimaryToPrimary = 1000;
    fees.transferPrimaryToOther = 2000;
    fees.transferNonPrimaryToNonPrimary = 4000;
    fees.cashTransferAny = 2000;
    return fees;
}

CashDrawer::CashDrawer() {
    for (int i = 0; i < CASH_TYPE_COUNT; ++i) {
        noteCounts[i] = 0;
    }
}

long long CashDrawer::TotalValue() const {
    long long total = 0;
    for (int i = 0; i < CASH_TYPE_COUNT; ++i) {
        total += static_cast<long long>(noteCounts[i]) * CASH_BILL_VALUES[i];
    }
    return total;
}

int CashDrawer::ItemCount() const {
    int total = 0;
    for (int i = 0; i < CASH_TYPE_COUNT; ++i) {
        total += noteCounts[i];
    }
    return total;
}

void CashDrawer::Add(const CashDrawer& other) {
    for (int i = 0; i < CASH_TYPE_COUNT; ++i) {
        noteCounts[i] += other.noteCounts[i];
    }
}

bool CashDrawer::HasEnoughBills(const CashDrawer& requested) const {
    for (int i = 0; i < CASH_TYPE_COUNT; ++i) {
        if (requested.noteCounts[i] > noteCounts[i]) {
            return false;
        }
    }
    return true;
}

void CashDrawer::Remove(const CashDrawer& requested) {
    if (!HasEnoughBills(requested)) {
        return;
    }

    for (int i = 0; i < CASH_TYPE_COUNT; ++i) {
        noteCounts[i] -= requested.noteCounts[i];
    }
}

SessionEvent::SessionEvent()
    : transactionType(ATMTransaction_Deposit),
      amount(0),
      feeCharged(0),
      sourceAccount(""),
      targetAccount(""),
      cashChange(),
      note("") {
}

SessionState::SessionState()
    : mode(ATMMode_Idle),
      card(NULL),
      adminCard(NULL),
      primaryAccount(NULL),
      isPrimaryBankCard(false),
      recordCount(0),
      withdrawalCount(0) {
}

ATM::ATM(const std::string& serialNumber,
         Bank* primaryBank,
         ATMBankAccess accessMode,
         bool bilingual)
    : serialNumber_(serialNumber),
      primaryBank_(primaryBank),
      acceptedBankCount_(0),
      accessMode_(accessMode),
      bilingual_(bilingual),
      language_(ATMLanguage_English),
      fees_(ATMFees::CreateDefault()),
      sessionActive_(false) {
    for (int i = 0; i < MAX_BANK_SLOTS; ++i) {
        acceptedBanks_[i] = NULL;
    }

    if (primaryBank_ != NULL && acceptedBankCount_ < MAX_BANK_SLOTS) {
        acceptedBanks_[acceptedBankCount_] = primaryBank_;
        ++acceptedBankCount_;
    }
}

const std::string& ATM::GetSerialNumber() const {
    return serialNumber_;
}

Bank* ATM::GetPrimaryBank() const {
    return primaryBank_;
}

void ATM::AddAcceptedBank(Bank* bank) {
    if (bank == NULL) {
        return;
    }

    if (accessMode_ == ATMBankAccess_SingleBank && bank != primaryBank_) {
        std::cout << "This ATM accepts only its primary bank.\n";
        return;
    }

    for (int i = 0; i < acceptedBankCount_; ++i) {
        if (acceptedBanks_[i] == bank) {
            return;
        }
    }

    if (acceptedBankCount_ >= MAX_BANK_SLOTS) {
        std::cout << "Accepted bank list is full.\n";
        return;
    }

    acceptedBanks_[acceptedBankCount_] = bank;
    ++acceptedBankCount_;
}

bool ATM::SupportsBank(const Bank* bank) const {
    if (bank == NULL) {
        return false;
    }

    for (int i = 0; i < acceptedBankCount_; ++i) {
        if (acceptedBanks_[i] == bank) {
            return true;
        }
    }

    return false;
}

ATMBankAccess ATM::GetBankAccessMode() const {
    return accessMode_;
}

bool ATM::IsBilingual() const {
    return bilingual_;
}

void ATM::SetLanguage(ATMLanguage language) {
    if (!bilingual_ && language == ATMLanguage_Korean) {
        std::cout << "This ATM supports English only.\n";
        language_ = ATMLanguage_English;
        return;
    }
    language_ = language;
}

ATMLanguage ATM::GetActiveLanguage() const {
    return language_;
}

const ATMFees& ATM::GetFees() const {
    return fees_;
}

void ATM::SetFees(const ATMFees& fees) {
    fees_ = fees;
}

const CashDrawer& ATM::GetCashInventory() const {
    return cashInventory_;
}

void ATM::LoadCash(const CashDrawer& cash) {
    cashInventory_.Add(cash);
}

bool ATM::TryGiveCash(const CashDrawer& cash) {
    if (!cashInventory_.HasEnoughBills(cash)) {
        std::cout << "Not enough cash available in the ATM.\n";
        return false;
    }

    cashInventory_.Remove(cash);
    return true;
}

void ATM::StartCustomerSession(const Card* card, Account* account, bool primaryBankCard) {
    if (sessionActive_) {
        std::cout << "A session is already running.\n";
        return;
    }

    ClearSession();
    sessionActive_ = true;
    sessionInfo_.mode = ATMMode_Customer;
    sessionInfo_.card = card;
    sessionInfo_.primaryAccount = account;
    sessionInfo_.isPrimaryBankCard = primaryBankCard;

    if (account == NULL) {
        std::cout << "Invalid card. Session ended.\n";
        EndSession();
    }
}

void ATM::StartAdminSession(const Card* card) {
    if (sessionActive_) {
        std::cout << "A session is already running.\n";
        return;
    }

    ClearSession();
    sessionActive_ = true;
    sessionInfo_.mode = ATMMode_Admin;
    sessionInfo_.adminCard = card;
}

void ATM::EndSession() {
    if (!sessionActive_) {
        return;
    }

    sessionActive_ = false;
    ClearSession();
}

bool ATM::HasActiveSession() const {
    return sessionActive_;
}

ATMMode ATM::GetActiveMode() const {
    if (!sessionActive_) {
        return ATMMode_Idle;
    }
    return sessionInfo_.mode;
}

const SessionState& ATM::GetSessionState() const {
    return sessionInfo_;
}

void ATM::RecordEvent(const SessionEvent& event) {
    if (!CheckSessionActive(ATMMode_Customer)) {
        return;
    }

    if (sessionInfo_.recordCount >= MAX_SESSION_EVENTS) {
        std::cout << "Session log is full. Event not recorded.\n";
        EndSession();
        return;
    }

    sessionInfo_.records[sessionInfo_.recordCount] = event;
    ++sessionInfo_.recordCount;
}

void ATM::PrintReceipt(std::ostream& out) const {
    if (!sessionActive_) {
        out << "No active session. Nothing to print.\n";
        return;
    }

    out << "ATM Serial: " << serialNumber_ << "\n";
    out << "Session Mode: " << (sessionInfo_.mode == ATMMode_Admin ? "Admin" : "Customer") << "\n";
    out << "Total Transactions: " << sessionInfo_.recordCount << "\n";

    for (int i = 0; i < sessionInfo_.recordCount; ++i) {
        const SessionEvent& entry = sessionInfo_.records[i];
        out << "  ";
        switch (entry.transactionType) {
        case ATMTransaction_Deposit:
            out << "Deposit";
            break;
        case ATMTransaction_Withdrawal:
            out << "Withdrawal";
            break;
        case ATMTransaction_AccountTransfer:
            out << "Account Transfer";
            break;
        case ATMTransaction_CashTransfer:
            out << "Cash Transfer";
            break;
        default:
            out << "Unknown";
            break;
        }
        out << " amount=" << entry.amount;
        out << " fee=" << entry.feeCharged;
        if (!entry.sourceAccount.empty()) {
            out << " from " << entry.sourceAccount;
        }
        if (!entry.targetAccount.empty()) {
            out << " to " << entry.targetAccount;
        }
        if (!entry.note.empty()) {
            out << " (" << entry.note << ")";
        }
        out << "\n";
    }
}

void ATM::RequestDeposit(const CashDrawer& cash, long long checkAmount, const CashDrawer& feeCash, int checkCount) {
    if (!CheckSessionActive(ATMMode_Customer)) {
        return;
    }

    int totalItems = cash.ItemCount() + checkCount;
    if (totalItems > MAX_INSERT_ITEMS) {
        std::cout << "Deposit exceeds the 50 item limit.\n";
        EndSession();
        return;
    }

    Account* account = sessionInfo_.primaryAccount;
    if (account == nullptr) {
        std::cout << "No account is linked to this session.\n";
        EndSession();
        return;
    }

    Bank* accountBank = account->getBank();
    if (accountBank == nullptr) {
        std::cout << "Unable to locate the bank for this account.\n";
        EndSession();
        return;
    }

    long long depositAmount = cash.TotalValue() + checkAmount;
    if (depositAmount <= 0) {
        std::cout << "Deposit amount must be positive.\n";
        EndSession();
        return;
    }

    SessionEvent event;
    event.transactionType = ATMTransaction_Deposit;
    event.amount = depositAmount;
    event.feeCharged = sessionInfo_.isPrimaryBankCard ? fees_.depositPrimary : fees_.depositNonPrimary;

    long long feeCashValue = feeCash.TotalValue();
    if (feeCashValue != event.feeCharged) {
        std::cout << "Fee cash must match the exact fee amount.\n";
        EndSession();
        return;
    }

    if (!accountBank->deposit(account, depositAmount)) {
        std::cout << "Deposit failed.\n";
        EndSession();
        return;
    }

    CashDrawer addedCash;
    if (cash.ItemCount() > 0) {
        cashInventory_.Add(cash);
        addedCash = cash;
    }
    event.cashChange = addedCash;
    event.sourceAccount.clear();
    event.targetAccount = account->getAccountNumber();
    if (feeCash.ItemCount() > 0) {
        cashInventory_.Add(feeCash);
    }

    if (checkAmount > 0) {
        event.note = "Check deposit completed";
    } else {
        event.note = "Deposit completed";
    }
    if (event.feeCharged > 0) {
        event.note += " (fee inserted separately)";
    }

    RecordEvent(event);

    const Card* card = sessionInfo_.card;
    std::string cardNumber = card != nullptr ? card->getNumber() : "";
    Transaction* transaction = new DepositTransaction(serialNumber_,
                                                      cardNumber,
                                                      account->getBankName(),
                                                      account->getAccountNumber(),
                                                      depositAmount,
                                                      event.feeCharged,
                                                      event.note);
    accountBank->addTransaction(transaction);
    account->recordTransaction(transaction);
}

void ATM::RequestWithdrawal(long long amount) {
    if (!CheckSessionActive(ATMMode_Customer)) {
        return;
    }

    if (sessionInfo_.withdrawalCount >= 3) {
        std::cout << "You reached the maximum of 3 withdrawals this session.\n";
        EndSession();
        return;
    }

    if (amount <= 0 || amount % 1000 != 0) {
        std::cout << "Enter an amount that is a positive multiple of 1,000.\n";
        EndSession();
        return;
    }

    if (amount > 500000) {
        std::cout << "Maximum withdrawal per transaction is 500,000.\n";
        EndSession();
        return;
    }

    CashDrawer bundle;
    if (!BuildWithdrawalBundle(amount, cashInventory_, bundle)) {
        std::cout << "ATM does not have the right bills for that amount.\n";
        EndSession();
        return;
    }

    if (!cashInventory_.HasEnoughBills(bundle)) {
        std::cout << "ATM is out of cash for that request.\n";
        EndSession();
        return;
    }

    Account* account = sessionInfo_.primaryAccount;
    if (account == nullptr) {
        std::cout << "No account is linked to this session.\n";
        EndSession();
        return;
    }

    Bank* accountBank = account->getBank();
    if (accountBank == nullptr) {
        std::cout << "Unable to locate the bank for this account.\n";
        EndSession();
        return;
    }

    SessionEvent event;
    event.transactionType = ATMTransaction_Withdrawal;
    event.amount = amount;
    event.cashChange = bundle;
    event.feeCharged = sessionInfo_.isPrimaryBankCard ? fees_.withdrawalPrimary : fees_.withdrawalNonPrimary;

    long long totalCost = amount + event.feeCharged;
    if (!accountBank->withdraw(account, totalCost)) {
        std::cout << "Insufficient funds in the account.\n";
        EndSession();
        return;
    }

    cashInventory_.Remove(bundle);
    event.sourceAccount = account->getAccountNumber();
    event.targetAccount.clear();
    event.note = "Withdrawal completed";
    if (event.feeCharged > 0) {
        event.note += " (fee deducted from account)";
    }

    ++sessionInfo_.withdrawalCount;
    RecordEvent(event);

    const Card* card = sessionInfo_.card;
    std::string cardNumber = card != nullptr ? card->getNumber() : "";
    Transaction* transaction = new WithdrawalTransaction(serialNumber_,
                                                         cardNumber,
                                                         account->getBankName(),
                                                         account->getAccountNumber(),
                                                         amount,
                                                         event.feeCharged,
                                                         event.note);
    accountBank->addTransaction(transaction);
    account->recordTransaction(transaction);
}

namespace {

long long DetermineTransferFee(const Bank* primaryBank,
                               Bank* sourceBank,
                               Bank* destinationBank,
                               const ATMFees& fees) {
    bool sourceIsPrimary = (primaryBank != nullptr && sourceBank == primaryBank);
    bool destinationIsPrimary = (primaryBank != nullptr && destinationBank == primaryBank);

    if (sourceIsPrimary && destinationIsPrimary) {
        return fees.transferPrimaryToPrimary;
    }
    if (sourceIsPrimary || destinationIsPrimary) {
        return fees.transferPrimaryToOther;
    }
    return fees.transferNonPrimaryToNonPrimary;
}

} // namespace

void ATM::RequestAccountTransfer(Account* destination, long long amount) {
    if (!CheckSessionActive(ATMMode_Customer)) {
        return;
    }

    if (destination == nullptr) {
        std::cout << "Destination account is invalid.\n";
        EndSession();
        return;
    }

    Account* source = sessionInfo_.primaryAccount;
    if (source == nullptr) {
        std::cout << "No source account is linked to this session.\n";
        EndSession();
        return;
    }

    if (source == destination) {
        std::cout << "Cannot transfer to the same account.\n";
        EndSession();
        return;
    }

    if (amount <= 0) {
        std::cout << "Transfer amount must be positive.\n";
        EndSession();
        return;
    }

    Bank* sourceBank = source->getBank();
    Bank* destinationBank = destination->getBank();
    if (sourceBank == nullptr || destinationBank == nullptr) {
        std::cout << "Unable to locate the banks for the accounts.\n";
        EndSession();
        return;
    }

    long long fee = DetermineTransferFee(primaryBank_, sourceBank, destinationBank, fees_);
    if (!sourceBank->transfer(source, destination, amount, fee)) {
        std::cout << "Transfer failed due to insufficient funds or invalid accounts.\n";
        EndSession();
        return;
    }

    SessionEvent event;
    event.transactionType = ATMTransaction_AccountTransfer;
    event.amount = amount;
    event.feeCharged = fee;
    event.sourceAccount = source->getAccountNumber();
    event.targetAccount = destination->getAccountNumber();
    event.note = "Account transfer completed";
    if (fee > 0) {
        event.note += " (fee deducted from source account)";
    }

    RecordEvent(event);

    const Card* card = sessionInfo_.card;
    std::string cardNumber = card != nullptr ? card->getNumber() : "";
    Transaction* transaction = new AccountTransferTransaction(serialNumber_,
                                                              cardNumber,
                                                              source->getBankName(),
                                                              source->getAccountNumber(),
                                                              destination->getBankName(),
                                                              destination->getAccountNumber(),
                                                              amount,
                                                              fee,
                                                              event.note);
    sourceBank->addTransaction(transaction);
    source->recordTransaction(transaction);
    destination->recordTransaction(transaction);
}

void ATM::RequestCashTransfer(Account* destination, const CashDrawer& cashInserted) {
    if (!CheckSessionActive(ATMMode_Customer)) {
        return;
    }

    if (destination == nullptr) {
        std::cout << "Destination account is invalid.\n";
        EndSession();
        return;
    }

    if (cashInserted.ItemCount() == 0) {
        std::cout << "Please insert cash to transfer.\n";
        EndSession();
        return;
    }

    Bank* destinationBank = destination->getBank();
    if (destinationBank == nullptr) {
        std::cout << "Unable to locate the bank for the destination account.\n";
        EndSession();
        return;
    }

    long long totalCash = cashInserted.TotalValue();
    long long fee = fees_.cashTransferAny;
    long long transferAmount = totalCash - fee;
    if (transferAmount <= 0) {
        std::cout << "Inserted cash does not cover the transfer fee. Insert more cash.\n";
        EndSession();
        return;
    }

    if (!destinationBank->deposit(destination, transferAmount)) {
        std::cout << "Cash transfer failed.\n";
        EndSession();
        return;
    }

    cashInventory_.Add(cashInserted);

    SessionEvent event;
    event.transactionType = ATMTransaction_CashTransfer;
    event.amount = transferAmount;
    event.feeCharged = fee;
    event.sourceAccount.clear();
    event.targetAccount = destination->getAccountNumber();
    event.cashChange = cashInserted;
    event.note = "Cash transfer completed";
    if (fee > 0) {
        event.note += " (fee inserted separately)";
    }

    RecordEvent(event);

    const Card* card = sessionInfo_.card;
    std::string cardNumber = card != nullptr ? card->getNumber() : "";
    Transaction* transaction = new CashTransferTransaction(serialNumber_,
                                                           cardNumber,
                                                           destination->getBankName(),
                                                           destination->getAccountNumber(),
                                                           transferAmount,
                                                           fee,
                                                           event.note);
    destinationBank->addTransaction(transaction);
    destination->recordTransaction(transaction);
}

void ATM::ClearSession() {
    sessionInfo_ = SessionState();
}

bool ATM::CheckSessionActive(ATMMode expectedMode) const {
    if (!sessionActive_) {
        std::cout << "Please start a session first.\n";
        return false;
    }

    if (expectedMode != ATMMode_Idle && sessionInfo_.mode != expectedMode) {
        std::cout << "That action is not allowed in this session.\n";
        return false;
    }

    return true;
}

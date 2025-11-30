#include "Atm.hpp"

#include <iostream>

#include "Account.hpp"
#include "Bank.hpp"
#include "Card.hpp"
#include "Transaction.hpp"

static const int CASH_BILL_VALUES[CASH_TYPE_COUNT] = {1000, 5000, 10000, 50000};

namespace {

std::string TLang(ATMLanguage lang, const std::string& en, const std::string& kr) {
    return (lang == ATMLanguage_Korean) ? kr : en;
}

void Say(ATMLanguage lang, const std::string& en, const std::string& kr) {
    std::cout << TLang(lang, en, kr);
}

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

}

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
      sessionActive_(false),
      transactions_(),
      totalSessions_(0),
      customerSessions_(0),
      adminSessions_(0) {
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
        Say(language_, "This ATM accepts only its primary bank.\n", "이 ATM은 기본 은행 카드만 허용합니다.\n");
        return;
    }

    for (int i = 0; i < acceptedBankCount_; ++i) {
        if (acceptedBanks_[i] == bank) {
            return;
        }
    }

    if (acceptedBankCount_ >= MAX_BANK_SLOTS) {
        Say(language_, "Accepted bank list is full.\n", "허용 가능한 은행 목록이 가득 찼습니다.\n");
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
        Say(language_, "This ATM supports English only.\n", "이 ATM은 영어만 지원합니다.\n");
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
        Say(language_, "Not enough cash available in the ATM.\n", "ATM에 충분한 현금이 없습니다.\n");
        return false;
    }

    cashInventory_.Remove(cash);
    return true;
}

void ATM::AddTransaction(Transaction* t) {
    if (t != nullptr) {
        transactions_.push_back(t);
    }
}

void ATM::StartCustomerSession(const Card* card, Account* account, bool primaryBankCard) {
    if (sessionActive_) {
        Say(language_, "A session is already running.\n", "이미 세션이 진행 중입니다.\n");
        return;
    }

    ClearSession();
    sessionActive_ = true;
    sessionInfo_.mode = ATMMode_Customer;
    sessionInfo_.card = card;
    sessionInfo_.primaryAccount = account;
    sessionInfo_.isPrimaryBankCard = primaryBankCard;

    if (account == NULL) {
        Say(language_, "Invalid card. Session ended.\n", "유효하지 않은 카드입니다. 세션을 종료합니다.\n");
        EndSession();
    }
}

void ATM::StartAdminSession(const Card* card) {
    if (sessionActive_) {
        Say(language_, "A session is already running.\n", "이미 세션이 진행 중입니다.\n");
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

long long ATM::GetDepositFeeForCurrentSession() const {
    if (!sessionActive_ || sessionInfo_.mode != ATMMode_Customer) {
        return 0;
    }
    return sessionInfo_.isPrimaryBankCard ? fees_.depositPrimary : fees_.depositNonPrimary;
}

void ATM::RecordEvent(const SessionEvent& event) {
    if (!CheckSessionActive(ATMMode_Customer)) {
        return;
    }

    if (sessionInfo_.recordCount >= MAX_SESSION_EVENTS) {
        Say(language_, "Session log is full. Event not recorded.\n",
            "세션 기록이 가득 찼습니다. 이벤트가 기록되지 않았습니다.\n");
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

    out << "\n========================================\n";
    out << TLang(language_, "           SESSION SUMMARY              ",
                "           세션 요약                     ") << "\n";
    out << "========================================\n";
    out << "  " << TLang(language_, "ATM Serial", "ATM 일련번호") << " : " << serialNumber_ << "\n";
    out << "  " << TLang(language_, "Mode", "모드") << "       : "
        << (sessionInfo_.mode == ATMMode_Admin ? TLang(language_, "Admin", "관리자")
                                               : TLang(language_, "Customer", "고객"))
        << "\n";
    out << "  " << TLang(language_, "Transactions", "거래 수") << ": " << sessionInfo_.recordCount << "\n";
    out << "----------------------------------------\n";

    if (sessionInfo_.recordCount == 0) {
        out << "  " << TLang(language_, "No transactions were recorded.\n", "기록된 거래가 없습니다.\n");
    } else {
        for (int i = 0; i < sessionInfo_.recordCount; ++i) {
            const SessionEvent& entry = sessionInfo_.records[i];
            out << "  #" << (i + 1) << " - ";
            switch (entry.transactionType) {
            case ATMTransaction_Deposit:
                out << TLang(language_, "Deposit", "입금");
                break;
            case ATMTransaction_Withdrawal:
                out << TLang(language_, "Withdrawal", "출금");
                break;
            case ATMTransaction_AccountTransfer:
                out << TLang(language_, "Account Transfer", "계좌 이체");
                break;
            case ATMTransaction_CashTransfer:
                out << TLang(language_, "Cash Transfer", "현금 이체");
                break;
            default:
                out << TLang(language_, "Unknown", "알 수 없음");
                break;
            }
            out << "\n";
            out << "    " << TLang(language_, "Amount", "금액") << " : " << entry.amount << "\n";
            out << "    " << TLang(language_, "Fee", "수수료") << "    : " << entry.feeCharged << "\n";
            if (!entry.sourceAccount.empty()) {
                out << "    " << TLang(language_, "From", "출금 계좌") << "   : " << entry.sourceAccount << "\n";
            }
            if (!entry.targetAccount.empty()) {
                out << "    " << TLang(language_, "To", "입금 계좌") << "     : " << entry.targetAccount << "\n";
            }
            if (!entry.note.empty()) {
                out << "    " << TLang(language_, "Note", "비고") << "   : " << entry.note << "\n";
            }
            out << "----------------------------------------\n";
        }
    }
    out << "========================================\n";
}

void ATM::RequestDeposit(const CashDrawer& cash, long long checkAmount, const CashDrawer& feeCash, int checkCount) {
    if (!CheckSessionActive(ATMMode_Customer)) {
        return;
    }

    int totalItems = cash.ItemCount() + checkCount;
    if (totalItems > MAX_INSERT_ITEMS) {
        Say(language_, "Deposit exceeds the 50 item limit.\n", "입금은 최대 50개까지만 가능합니다.\n");
        EndSession();
        return;
    }

    Account* account = sessionInfo_.primaryAccount;
    if (account == nullptr) {
        Say(language_, "No account is linked to this session.\n", "이 세션에 연결된 계좌가 없습니다.\n");
        EndSession();
        return;
    }

    Bank* accountBank = account->getBank();
    if (accountBank == nullptr) {
        Say(language_, "Unable to locate the bank for this account.\n", "계좌의 은행을 찾을 수 없습니다.\n");
        EndSession();
        return;
    }

    long long depositAmount = cash.TotalValue() + checkAmount;
    if (depositAmount <= 0) {
        Say(language_, "Deposit amount must be positive.\n", "입금 금액은 0보다 커야 합니다.\n");
        return;
    }

    SessionEvent event;
    event.transactionType = ATMTransaction_Deposit;
    event.amount = depositAmount;
    event.feeCharged = sessionInfo_.isPrimaryBankCard ? fees_.depositPrimary : fees_.depositNonPrimary;

    long long feeCashValue = feeCash.TotalValue();
    if (feeCashValue != event.feeCharged) {
        Say(language_, "Fee cash must match the exact fee amount.\n", "수수료 금액과 동일한 현금을 넣어야 합니다.\n");
        EndSession();
        return;
    }
    if (event.feeCharged > 0) {
        Say(language_, "Fee cash accepted.\n", "수수료 현금을 확인했습니다.\n");
    }

    if (!accountBank->deposit(account, depositAmount)) {
        Say(language_, "Deposit failed.\n", "입금에 실패했습니다.\n");
        EndSession();
        return;
    }
    Say(language_, "Deposit completed", "입금이 완료되었습니다");
    if (event.feeCharged > 0) {
        std::cout << TLang(language_,
                           " (fee ",
                           " (수수료 ") << event.feeCharged
                  << TLang(language_,
                           " paid in cash and not added to balance)",
                           "가 현금으로 지불되었으며 잔액에 추가되지 않습니다)");
    }
    std::cout << ".\n";

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
        if (event.feeCharged > 0) {
            event.note = TLang(language_,
                               "Check deposit completed (fee ",
                               "수표 입금 완료 (수수료 ");
            event.note += std::to_string(event.feeCharged);
            event.note += TLang(language_,
                                " paid in cash and not added to balance)",
                                "가 현금으로 지불되었으며 잔액에 추가되지 않음)");
        } else {
            event.note = TLang(language_,
                               "Check deposit completed",
                               "수표 입금 완료");
        }
    } else {
        if (event.feeCharged > 0) {
            event.note = TLang(language_,
                               "Deposit completed (fee ",
                               "입금 완료 (수수료 ");
            event.note += std::to_string(event.feeCharged);
            event.note += TLang(language_,
                                " paid in cash and not added to balance)",
                                "가 현금으로 지불되었으며 잔액에 추가되지 않음)");
        } else {
            event.note = TLang(language_,
                               "Deposit completed",
                               "입금 완료");
        }
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
    AddTransaction(transaction);
    accountBank->addTransaction(transaction);
    account->recordTransaction(transaction);
}

void ATM::RequestWithdrawal(long long amount) {
    if (!CheckSessionActive(ATMMode_Customer)) {
        return;
    }

    if (sessionInfo_.withdrawalCount >= 3) {
        Say(language_, "You reached the maximum of 3 withdrawals this session.\n",
            "이 세션에서 출금은 최대 3회까지 가능합니다.\n");
        return;
    }

    if (amount <= 0 || amount % 1000 != 0) {
        Say(language_, "Enter an amount that is a positive multiple of 1,000.\n",
            "1,000원 단위의 양수 금액을 입력하세요.\n");
        return;
    }

    if (amount > 500000) {
        Say(language_, "Maximum withdrawal per transaction is 500,000.\n",
            "한 번에 출금할 수 있는 최대 금액은 500,000원입니다.\n");
        return;
    }

    CashDrawer bundle;
    if (!BuildWithdrawalBundle(amount, cashInventory_, bundle)) {
        Say(language_, "ATM does not have the right bills for that amount.\n",
            "해당 금액을 만들 수 있는 지폐 구성이 없습니다.\n");
        return;
    }

    if (!cashInventory_.HasEnoughBills(bundle)) {
        Say(language_, "ATM is out of cash for that request.\n", "요청 금액을 지급할 현금이 부족합니다.\n");
        return;
    }

    Account* account = sessionInfo_.primaryAccount;
    if (account == nullptr) {
        Say(language_, "No account is linked to this session.\n", "이 세션에 연결된 계좌가 없습니다.\n");
        return;
    }

    Bank* accountBank = account->getBank();
    if (accountBank == nullptr) {
        Say(language_, "Unable to locate the bank for this account.\n", "계좌의 은행을 찾을 수 없습니다.\n");
        return;
    }

    SessionEvent event;
    event.transactionType = ATMTransaction_Withdrawal;
    event.amount = amount;
    event.cashChange = bundle;
    event.feeCharged = sessionInfo_.isPrimaryBankCard ? fees_.withdrawalPrimary : fees_.withdrawalNonPrimary;

    long long totalCost = amount + event.feeCharged;
    if (!accountBank->withdraw(account, totalCost)) {
        Say(language_, "Insufficient funds in the account.\n", "계좌 잔액이 부족합니다.\n");
        return;
    }

    cashInventory_.Remove(bundle);
    Say(language_, "Withdrawal complete", "출금이 완료되었습니다");
    if (event.feeCharged > 0) {
        std::cout << TLang(language_, "; fee ", "; 수수료 ") << event.feeCharged
                  << TLang(language_, " deducted from account", "가 계좌에서 차감되었습니다");
    }
    std::cout << ".\n";
    event.sourceAccount = account->getAccountNumber();
    event.targetAccount.clear();
    if (event.feeCharged > 0) {
        event.note = TLang(language_,
                           "Withdrawal completed (fee deducted from account)",
                           "출금 완료 (수수료가 계좌에서 차감됨)");
    } else {
        event.note = TLang(language_,
                           "Withdrawal completed",
                           "출금 완료");
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
    AddTransaction(transaction);
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
        Say(language_, "Destination account is invalid.\n", "목적지 계좌가 올바르지 않습니다.\n");
        return;
    }

    Account* source = sessionInfo_.primaryAccount;
    if (source == nullptr) {
        Say(language_, "No source account is linked to this session.\n", "이 세션에 출금 계좌가 없습니다.\n");
        return;
    }

    if (source == destination) {
        Say(language_, "Cannot transfer to the same account.\n", "동일한 계좌로는 이체할 수 없습니다.\n");
        return;
    }

    if (amount <= 0) {
        Say(language_, "Transfer amount must be positive.\n", "이체 금액은 0보다 커야 합니다.\n");
        return;
    }

    Bank* sourceBank = source->getBank();
    Bank* destinationBank = destination->getBank();
    if (sourceBank == nullptr || destinationBank == nullptr) {
        Say(language_, "Unable to locate the banks for the accounts.\n", "계좌의 은행을 찾을 수 없습니다.\n");
        return;
    }

    long long fee = DetermineTransferFee(primaryBank_, sourceBank, destinationBank, fees_);
    if (!sourceBank->transfer(source, destination, amount, fee)) {
        Say(language_, "Transfer failed due to insufficient funds or invalid accounts.\n",
            "잔액 부족 또는 잘못된 계좌로 인해 이체에 실패했습니다.\n");
        return;
    }
    Say(language_, "Account transfer complete", "계좌 이체가 완료되었습니다");
    if (fee > 0) {
        std::cout << TLang(language_, "; fee ", "; 수수료 ") << fee
                  << TLang(language_, " deducted from source account", "가 출금 계좌에서 차감되었습니다");
    }
    std::cout << ".\n";

    SessionEvent event;
    event.transactionType = ATMTransaction_AccountTransfer;
    event.amount = amount;
    event.feeCharged = fee;
    event.sourceAccount = source->getAccountNumber();
    event.targetAccount = destination->getAccountNumber();
    if (fee > 0) {
        event.note = TLang(language_,
                           "Account transfer completed (fee deducted from source account)",
                           "계좌 이체 완료 (수수료가 출금 계좌에서 차감됨)");
    } else {
        event.note = TLang(language_,
                           "Account transfer completed",
                           "계좌 이체 완료");
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
    AddTransaction(transaction);
    sourceBank->addTransaction(transaction);
    source->recordTransaction(transaction);
    destination->recordTransaction(transaction);
}

void ATM::RequestCashTransfer(Account* destination, const CashDrawer& cashInserted) {
    if (!CheckSessionActive(ATMMode_Customer)) {
        return;
    }

    Account* source = sessionInfo_.primaryAccount;
    if (source == nullptr) {
        Say(language_, "No source account is linked to this session.\n", "이 세션에 출금 계좌가 없습니다.\n");
        return;
    }

    if (destination == nullptr) {
        Say(language_, "Destination account is invalid.\n", "목적지 계좌가 올바르지 않습니다.\n");
        return;
    }

    if (cashInserted.ItemCount() == 0) {
        Say(language_, "Please insert cash to transfer.\n", "이체할 현금을 넣어 주세요.\n");
        return;
    }

    if (cashInserted.ItemCount() > MAX_INSERT_ITEMS) {
        Say(language_,
            "Cash transfer exceeds the 50 item limit.\n",
            "현금 이체는 최대 50개까지만 가능합니다.\n");
        return;
    }

    Bank* destinationBank = destination->getBank();
    if (destinationBank == nullptr) {
        Say(language_, "Unable to locate the bank for the destination account.\n", "목적지 계좌의 은행을 찾을 수 없습니다.\n");
        return;
    }

    long long totalCash = cashInserted.TotalValue();
    long long fee = fees_.cashTransferAny;
    long long transferAmount = totalCash - fee;
    if (transferAmount <= 0) {
        Say(language_, "Inserted cash does not cover the transfer fee. Insert more cash.\n",
            "넣은 현금이 수수료보다 적습니다. 현금을 더 넣어 주세요.\n");
        return;
    }

    if (!destinationBank->deposit(destination, transferAmount)) {
        Say(language_, "Cash transfer failed.\n", "현금 이체에 실패했습니다.\n");
        return;
    }

    cashInventory_.Add(cashInserted);
    Say(language_, "Cash transfer complete", "현금 이체가 완료되었습니다");
    if (fee > 0) {
        std::cout << TLang(language_, "; fee ", "; 수수료 ") << fee
                  << TLang(language_,
                           " paid in cash and not deposited to the destination account",
                           "가 현금으로 지불되었으며 입금 계좌에 추가되지 않습니다");
    }
    std::cout << ".\n";

    SessionEvent event;
    event.transactionType = ATMTransaction_CashTransfer;
    event.amount = transferAmount;
    event.feeCharged = fee;
    event.sourceAccount = source->getAccountNumber();
    event.targetAccount = destination->getAccountNumber();
    event.cashChange = cashInserted;
    if (fee > 0) {
        event.note = TLang(language_,
                           "Cash transfer completed (fee paid in cash and not deposited to the destination account)",
                           "현금 이체 완료 (수수료가 현금으로 지불되었으며 입금 계좌에 추가되지 않음)");
    } else {
        event.note = TLang(language_,
                           "Cash transfer completed",
                           "현금 이체 완료");
    }

    RecordEvent(event);

    const Card* card = sessionInfo_.card;
    std::string cardNumber = card != nullptr ? card->getNumber() : "";
    Transaction* transaction = new CashTransferTransaction(serialNumber_,
                                                           cardNumber,
                                                           source->getBankName(),
                                                           source->getAccountNumber(),
                                                           destination->getBankName(),
                                                           destination->getAccountNumber(),
                                                           transferAmount,
                                                           fee,
                                                           event.note);
    AddTransaction(transaction);
    destinationBank->addTransaction(transaction);
    destination->recordTransaction(transaction);
}

void ATM::ClearSession() {
    sessionInfo_ = SessionState();
}

bool ATM::CheckSessionActive(ATMMode expectedMode) const {
    if (!sessionActive_) {
        Say(language_, "Please start a session first.\n", "먼저 세션을 시작하세요.\n");
        return false;
    }

    if (expectedMode != ATMMode_Idle && sessionInfo_.mode != expectedMode) {
        Say(language_, "That action is not allowed in this session.\n", "이 세션에서는 해당 작업을 수행할 수 없습니다.\n");
        return false;
    }

    return true;
}

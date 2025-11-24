#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Account.hpp"
#include "Bank.hpp"
#include "Card.hpp"
#include "Transaction.hpp"
#include "atm.hpp"

struct SystemState {
    std::vector<Bank*> banks;
    std::vector<Account*> accounts;
    std::vector<Card*> cards;
    std::vector<ATM*> atms;
    std::vector<Transaction*> transactions;
    int totalSessions = 0;
    int customerSessions = 0;
    int adminSessions = 0;
};

namespace {

std::string T(ATMLanguage lang, const std::string& en, const std::string& kr) {
    return (lang == ATMLanguage_Korean) ? kr : en;
}

void ClearInputLine() {
    std::cin.clear();
    std::cin.ignore(100000, '\n');
}

int PromptInt(const std::string& message, int minValue) {
    int value = 0;
    while (true) {
        std::cout << message;
        if (std::cin >> value && value >= minValue) {
            return value;
        }
        std::cout << "Invalid input. Try again.\n";
        ClearInputLine();
    }
}

long long PromptLongLong(const std::string& message, long long minValue) {
    long long value = 0;
    while (true) {
        std::cout << message;
        if (std::cin >> value && value >= minValue) {
            return value;
        }
        std::cout << "Invalid input. Try again.\n";
        ClearInputLine();
    }
}

std::string PromptString(const std::string& message) {
    std::cout << message;
    std::string input;
    std::cin >> input;
    return input;
}

void PrintWelcomeBanner() {
    std::cout << "========================================\n";
    std::cout << "         DGIST ATM SIMULATOR            \n";
    std::cout << "========================================\n";
    std::cout << "Load sample_initial_condition.txt, then\n";
    std::cout << "enter admin cards/PINs for each bank.\n";
    std::cout << "----------------------------------------\n\n";
}

void PrintMainMenu() {
    std::cout << "\n========================================\n";
    std::cout << "              MAIN MENU                 \n";
    std::cout << "========================================\n";
    std::cout << "  [1] Use an ATM\n";
    std::cout << "  [2] Show snapshot\n";
    std::cout << "  [0] Exit\n";
    std::cout << "  [/] Quick snapshot\n";
    std::cout << "========================================\n";
}

Bank* FindBank(const std::vector<Bank*>& banks, const std::string& name) {
    for (Bank* bank : banks) {
        if (bank != nullptr && bank->getBankName() == name) {
            return bank;
        }
    }
    return nullptr;
}

Account* FindAccountByCard(const std::vector<Account*>& accounts, const std::string& cardNumber) {
    for (std::size_t i = 0; i < accounts.size(); ++i) {
        Account* account = accounts[i];
        if (account == nullptr) {
            continue;
        }
        Card* linkedCard = account->getLinkedCard();
        if (linkedCard != nullptr && linkedCard->getNumber() == cardNumber) {
            return account;
        }
    }
    return nullptr;
}

Account* FindAccountByNumber(const std::vector<Account*>& accounts, const std::string& accountNumber) {
    for (std::size_t i = 0; i < accounts.size(); ++i) {
        Account* account = accounts[i];
        if (account != nullptr && account->getAccountNumber() == accountNumber) {
            return account;
        }
    }
    return nullptr;
}

long long PromptCheckAmounts(int& checkCount) {
    long long total = 0;
    checkCount = 0;
    while (true) {
        long long amount = PromptLongLong("Enter check amount (0 to finish): ", 0);
        if (amount == 0) {
            break;
        }
        if (amount < 100000) {
            std::cout << "Each check must be at least 100,000 KRW.\n";
            continue;
        }
        total += amount;
        ++checkCount;
    }
    return total;
}

CashDrawer PromptCashDrawer(const std::string& label) {
    CashDrawer drawer;
    for (int i = 0; i < CASH_TYPE_COUNT; ++i) {
        drawer.noteCounts[i] = 0;
    }

    std::cout << "Enter bills for " << label << " (use non-negative integers).\n";
    int count50k = PromptInt("50,000 KRW bills: ", 0);
    int count10k = PromptInt("10,000 KRW bills: ", 0);
    int count5k = PromptInt("5,000 KRW bills: ", 0);
    int count1k = PromptInt("1,000 KRW bills: ", 0);

    drawer.noteCounts[0] = count1k;
    drawer.noteCounts[1] = count5k;
    drawer.noteCounts[2] = count10k;
    drawer.noteCounts[3] = count50k;
    return drawer;
}

void PrintSnapshot(const std::vector<Bank*>& banks, const std::vector<ATM*>& atms, ATMLanguage lang = ATMLanguage_English) {
    std::cout << "\n=== " << T(lang, "Snapshot", "스냅샷") << " ===\n";

    std::cout << T(lang, "ATMs (remaining cash):", "ATM (잔여 현금):") << "\n";
    for (const ATM* atm : atms) {
        if (atm == nullptr) {
            continue;
        }
        const Bank* primaryBank = atm->GetPrimaryBank();
        std::string bankName = primaryBank ? primaryBank->getBankName() : "Unknown";
        const CashDrawer& drawer = atm->GetCashInventory();
        long long totalCash = drawer.TotalValue();

        int count1k = drawer.noteCounts[0];
        int count5k = drawer.noteCounts[1];
        int count10k = drawer.noteCounts[2];
        int count50k = drawer.noteCounts[3];

        std::cout << bankName << " ATM [SN:" << atm->GetSerialNumber() << "] "
                  << T(lang, "Remaining cash: ", "잔여 현금: ") << totalCash
                  << T(lang, " | Left cash: ", " | 남은 지폐: ")
                  << count1k << T(lang, " x 1,000 won, ", " x 1,000원, ")
                  << count5k << T(lang, " x 5,000 won, ", " x 5,000원, ")
                  << count10k << T(lang, " x 10,000 won, ", " x 10,000원, ")
                  << count50k << T(lang, " x 50,000 won", " x 50,000원") << "\n";
    }

    std::cout << "\n" << T(lang, "Accounts (remaining balance):", "계좌 (잔액):") << "\n";
    for (const Bank* bank : banks) {
        if (bank == nullptr) {
            continue;
        }
        for (Account* account : bank->getAccounts()) {
            if (account == nullptr) {
                continue;
            }
            std::cout << T(lang, "Account", "계좌") << " ["
                      << T(lang, "Bank", "은행") << ": " << bank->getBankName()
                      << ", " << T(lang, "No.", "번호") << " " << account->getAccountNumber()
                      << ", " << T(lang, "Owner", "소유자") << ": " << account->getOwnerName()
                      << "] " << T(lang, "Balance", "잔액") << " : " << account->getBalance() << "\n";
        }
    }

    std::cout << "================\n";
}

CashDrawer PromptFeeCash(ATMLanguage lang, long long fee) {
    CashDrawer drawer;
    for (int i = 0; i < CASH_TYPE_COUNT; ++i) {
        drawer.noteCounts[i] = 0;
    }

    std::cout << T(lang,
                   "Enter bills for fee (this cash will not be added to your account; it pays the fee).\n",
                   "수수료 지폐 개수를 입력하세요 (이 현금은 계좌에 추가되지 않고 수수료로 사용됩니다).\n");
    std::cout << T(lang, "Exact fee amount: ", "정확한 수수료 금액: ") << fee << "\n";

    int count50k = PromptInt("50,000 KRW bills: ", 0);
    int count10k = PromptInt("10,000 KRW bills: ", 0);
    int count5k = PromptInt("5,000 KRW bills: ", 0);
    int count1k = PromptInt("1,000 KRW bills: ", 0);

    drawer.noteCounts[0] = count1k;
    drawer.noteCounts[1] = count5k;
    drawer.noteCounts[2] = count10k;
    drawer.noteCounts[3] = count50k;
    return drawer;
}

std::string LocalizedNoteForTransaction(ATMLanguage lang, const Transaction* t) {
    if (t == nullptr) {
        return "";
    }
    long long fee = t->getFee();

    if (dynamic_cast<const DepositTransaction*>(t) != nullptr) {
        if (fee > 0) {
            return T(lang,
                     "Deposit completed (fee " + std::to_string(fee) + " paid in cash and not added to balance)",
                     "입금 완료 (수수료 " + std::to_string(fee) + "가 현금으로 지불되었으며 잔액에 추가되지 않음)");
        }
        return T(lang, "Deposit completed", "입금 완료");
    }

    if (dynamic_cast<const WithdrawalTransaction*>(t) != nullptr) {
        if (fee > 0) {
            return T(lang,
                     "Withdrawal completed (fee deducted from account)",
                     "출금 완료 (수수료가 계좌에서 차감됨)");
        }
        return T(lang, "Withdrawal completed", "출금 완료");
    }

    if (dynamic_cast<const AccountTransferTransaction*>(t) != nullptr) {
        if (fee > 0) {
            return T(lang,
                     "Account transfer completed (fee deducted from source account)",
                     "계좌 이체 완료 (수수료가 출금 계좌에서 차감됨)");
        }
        return T(lang, "Account transfer completed", "계좌 이체 완료");
    }

    if (dynamic_cast<const CashTransferTransaction*>(t) != nullptr) {
        if (fee > 0) {
            return T(lang,
                     "Cash transfer completed (fee paid in cash and not deposited to the destination account)",
                     "현금 이체 완료 (수수료가 현금으로 지불되었으며 입금 계좌에 추가되지 않음)");
        }
        return T(lang, "Cash transfer completed", "현금 이체 완료");
    }

    return t->getNote();
}
ATMLanguage SelectLanguageForAtm(ATM* atm) {
    if (atm == nullptr) {
        return ATMLanguage_English;
    }
    if (!atm->IsBilingual()) {
        std::cout << "English only ATM. Proceeding in English.\n";
        atm->SetLanguage(ATMLanguage_English);
        return ATMLanguage_English;
    }
    while (true) {
        std::cout << "\nSelect language / 언어를 선택하세요\n";
        std::cout << "  [1] English\n";
        std::cout << "  [2] 한국어\n";
        int choice = PromptInt("Choice: ", 1);
        if (choice == 1) {
            atm->SetLanguage(ATMLanguage_English);
            return ATMLanguage_English;
        }
        if (choice == 2) {
            atm->SetLanguage(ATMLanguage_Korean);
            return ATMLanguage_Korean;
        }
        std::cout << "Invalid input. Try again.\n";
    }
}

void PrintTransactions(const std::vector<Transaction*>& transactions,
                       std::ostream& out,
                       ATMLanguage lang = ATMLanguage_English) {
    out << "========================================\n";
    out << T(lang, "        TRANSACTION HISTORY            ",
             "              거래 내역                ")
        << "\n";
    out << "========================================\n";
    if (transactions.empty()) {
        out << "  " << T(lang, "No transactions recorded yet.", "기록된 거래가 없습니다.") << "\n";
        out << "========================================\n";
        return;
    }
    for (const Transaction* transaction : transactions) {
        if (transaction == nullptr) {
            continue;
        }
        out << "ID: " << transaction->getId() << "\n";
        out << "  " << T(lang, "ATM", "ATM") << ": " << transaction->getAtmSerial() << "\n";
        out << "  " << T(lang, "Card", "카드") << ": " << transaction->getCardNumber() << "\n";
        out << "  " << T(lang, "Type", "유형") << ": " << transaction->getTypeName() << "\n";
        out << "  " << T(lang, "Amount", "금액") << ": " << transaction->getAmount() << "\n";
        out << "  " << T(lang, "Fee", "수수료") << ": " << transaction->getFee() << "\n";
        out << "  " << T(lang, "From", "출금 계좌") << ": " << transaction->getSourceBankName()
            << " / " << transaction->getSourceAccountNumber() << "\n";
        // If it is a transfer, show target info via dynamic_cast
        if (const AccountTransferTransaction* at =
                dynamic_cast<const AccountTransferTransaction*>(transaction)) {
            out << "  " << T(lang, "To", "입금 계좌") << ": " << at->getTargetBankName()
                << " / " << at->getTargetAccountNumber() << "\n";
        } else if (const CashTransferTransaction* ct =
                       dynamic_cast<const CashTransferTransaction*>(transaction)) {
            out << "  " << T(lang, "To", "입금 계좌") << ": " << ct->getTargetBankName()
                << " / " << ct->getTargetAccountNumber() << "\n";
        }
        std::string localizedNote = LocalizedNoteForTransaction(lang, transaction);
        if (!localizedNote.empty()) {
            out << "  " << T(lang, "Note", "비고") << ": " << localizedNote << "\n";
        }
        out << "----------------------------------------\n";
    }
    out << "========================================\n";
}

bool LoadInitialData(const std::string& filename, SystemState& state) {
    std::ifstream fin(filename);
    if (!fin) {
        std::cerr << "Error opening file: " << filename << "\n";
        return false;
    }

    int bankCount = 0;
    int accountCount = 0;
    int atmCount = 0;
    fin >> bankCount >> accountCount >> atmCount;

    state.banks.reserve(bankCount);
    state.accounts.reserve(accountCount);
    state.cards.reserve(accountCount);
    state.atms.reserve(atmCount);
    state.transactions.reserve(32);

    for (int i = 0; i < bankCount; ++i) {
        std::string bankName;
        fin >> bankName;
        auto* bank = new Bank(bankName, bankName, &state.banks, &state.transactions);
        state.banks.push_back(bank);
    }

    for (int i = 0; i < accountCount; ++i) {
        std::string bankName;
        std::string userName;
        std::string accountNumber;
        long long availableFunds = 0;
        std::string cardNumber;
        std::string password;
        fin >> bankName >> userName >> accountNumber >> availableFunds >> cardNumber >> password;

        Bank* bank = FindBank(state.banks, bankName);
        if (bank == nullptr) {
            std::cerr << "Bank " << bankName << " not found for account " << accountNumber << ".\n";
            return false;
        }
        auto* card = new Card(cardNumber, bankName, CardRole::User);
        auto* account = new Account(bank, userName, accountNumber, availableFunds, card, password);

        state.cards.push_back(card);
        state.accounts.push_back(account);
        bank->addAccount(account);
    }

    for (int i = 0; i < atmCount; ++i) {
        std::string primaryBankName;
        std::string serial;
        std::string accessModeStr;
        std::string languageStr;
        fin >> primaryBankName >> serial >> accessModeStr >> languageStr;

        Bank* primaryBank = FindBank(state.banks, primaryBankName);
        if (primaryBank == nullptr) {
            std::cerr << "Primary bank " << primaryBankName << " not found for ATM " << serial << ".\n";
            return false;
        }

        ATMBankAccess accessMode =
            (accessModeStr == "Single") ? ATMBankAccess_SingleBank : ATMBankAccess_MultiBank;
        bool bilingual = (languageStr == "Bilingual");

        auto* atm = new ATM(serial, primaryBank, accessMode, bilingual);

        int count50k = 0;
        int count10k = 0;
        int count5k = 0;
        int count1k = 0;
        fin >> count50k >> count10k >> count5k >> count1k;
        CashDrawer drawer;
        drawer.noteCounts[0] = count1k;
        drawer.noteCounts[1] = count5k;
        drawer.noteCounts[2] = count10k;
        drawer.noteCounts[3] = count50k;
        atm->LoadCash(drawer);

        if (accessMode == ATMBankAccess_MultiBank) {
            for (Bank* bank : state.banks) {
                atm->AddAcceptedBank(bank);
            }
        }

        state.atms.push_back(atm);
    }

    return true;
}

void ConfigureAdminCards(SystemState& state) {
    std::cout << "\n=== Admin Card Setup ===\n";
    for (Bank* bank : state.banks) {
        if (bank == nullptr) {
            continue;
        }
        std::cout << "Configuring admin card for bank: " << bank->getBankName() << "\n";
        std::string adminCardNumber = PromptString("  Enter a unique admin card number: ");
        std::string adminPassword = PromptString("  Enter a unique admin password/PIN: ");
        bank->setAdminCard(adminCardNumber, adminPassword);
    }
    std::cout << "========================\n";
}

void RunAdminMenu(ATM* atm,
                  const std::vector<Bank*>& banks,
                  const std::vector<ATM*>& atms,
                  ATMLanguage lang) {
    const std::vector<Transaction*>& transactions = atm->GetTransactions();
    int totalSessions = atm->GetTotalSessions();
    int customerSessions = atm->GetCustomerSessions();
    int adminSessions = atm->GetAdminSessions();
    while (true) {
        std::cout << "\n========================================\n";
        std::cout << "              ADMIN MENU                \n";
        std::cout << "========================================\n";
        std::cout << "  [1] " << T(lang, "Print all transactions", "모든 거래 출력") << "\n";
        std::cout << "  [2] " << T(lang, "Export transactions to file", "거래 내역 파일로 저장") << "\n";
        std::cout << "  [/] " << T(lang, "Snapshot", "스냅샷") << "\n";
        std::cout << "  [0] " << T(lang, "Exit admin menu", "관리자 메뉴 종료") << "\n";
        std::cout << "========================================\n";
        std::string choiceInput = PromptString(T(lang, "Select an option: ", "옵션을 선택하세요: "));
        if (choiceInput == "/") {
            PrintSnapshot(banks, atms, lang);
            continue;
        }
        int choice = 0;
        {
            bool parsed = false;
            if (!choiceInput.empty()) {
                bool isNumber = true;
                for (std::size_t i = 0; i < choiceInput.size(); ++i) {
                    if (choiceInput[i] < '0' || choiceInput[i] > '9') {
                        isNumber = false;
                        break;
                    }
                }
                if (isNumber) {
                    choice = static_cast<int>(std::stoi(choiceInput));
                    parsed = true;
                }
            }
            if (!parsed) {
                std::cout << T(lang, "Invalid input. Try again.\n", "잘못된 입력입니다. 다시 시도하세요.\n");
                continue;
            }
        }

        if (choice == 0) {
            break;
        }

        switch (choice) {
        case 1:
            std::cout << T(lang, "This ATM sessions: ", "이 ATM 세션 수: ")
                      << totalSessions
                      << T(lang, " (customer ", " (고객 ")
                      << customerSessions
                      << T(lang, ", admin ", ", 관리자 ")
                      << adminSessions << ")\n";
            PrintTransactions(transactions, std::cout, lang);
            break;
        case 2: {
            std::string filename = PromptString(T(lang, "Enter output filename: ", "출력할 파일 이름을 입력하세요: "));
            std::ofstream fout(filename);
            if (!fout) {
                std::cout << T(lang, "Failed to open file.\n", "파일을 열 수 없습니다.\n");
                break;
            }
            fout << T(lang, "This ATM sessions: ", "이 ATM 세션 수: ")
                 << totalSessions
                 << T(lang, " (customer ", " (고객 ")
                 << customerSessions
                 << T(lang, ", admin ", ", 관리자 ")
                 << adminSessions << ")\n";
            PrintTransactions(transactions, fout, lang);
            std::cout << T(lang, "Transactions exported to ", "거래 내역을 파일로 저장했습니다: ") << filename << "\n";
            break;
        }
        default:
            std::cout << T(lang, "Unknown choice.\n", "알 수 없는 선택입니다.\n");
            break;
        }
    }
}

void RunAtmMenu(ATM* atm,
                const std::vector<Account*>& accounts,
                const std::vector<Bank*>& banks,
                const std::vector<ATM*>& atms) {
    if (atm == nullptr) {
        return;
    }

    while (true) {
        ATMLanguage lang = atm->GetActiveLanguage();
        std::cout << "\n========================================\n";
        std::cout << T(lang, "          ATM MENU - Serial ", "          ATM 메뉴 - 일련번호 ") << atm->GetSerialNumber() << "         \n";
        std::cout << "========================================\n";
        std::cout << "  [1] " << T(lang, "Deposit", "입금") << "\n";
        std::cout << "  [2] " << T(lang, "Withdraw", "출금") << "\n";
        std::cout << "  [3] " << T(lang, "Account transfer", "계좌 이체") << "\n";
        std::cout << "  [4] " << T(lang, "Cash transfer", "현금 이체") << "\n";
        std::cout << "  [5] " << T(lang, "Print receipt", "영수증 출력") << "\n";
        std::cout << "  [/] " << T(lang, "Snapshot", "스냅샷") << "\n";
        std::cout << "  [0] " << T(lang, "End session", "세션 종료") << "\n";
        std::cout << "========================================\n";

        std::string choiceInput = PromptString(T(lang, "Select an option: ", "옵션을 선택하세요: "));
        if (choiceInput == "/") {
            PrintSnapshot(banks, atms, lang);
            continue;
        }

        int choice = 0;
        {
            bool parsed = false;
            if (!choiceInput.empty()) {
                bool isNumber = true;
                for (std::size_t i = 0; i < choiceInput.size(); ++i) {
                    if (choiceInput[i] < '0' || choiceInput[i] > '9') {
                        isNumber = false;
                        break;
                    }
                }
                if (isNumber) {
                    choice = static_cast<int>(std::stoi(choiceInput));
                    parsed = true;
                }
            }
            if (!parsed) {
                std::cout << "Invalid input. Try again.\n";
                continue;
            }
        }

        if (choice == 0) {
            atm->PrintReceipt(std::cout);
            std::cout << T(lang, "Session ended.\n", "세션이 종료되었습니다.\n");
            atm->EndSession();
            break;
        }

        switch (choice) {
        case 1: {
            CashDrawer cash = PromptCashDrawer(T(lang, "deposit", "입금"));
            int checkCount = 0;
            long long checkAmount = PromptCheckAmounts(checkCount);
            long long depositFee = atm->GetDepositFeeForCurrentSession();
            CashDrawer feeCash;
            if (depositFee == 0) {
                std::cout << T(lang,
                               "No cash fee is required for this deposit.\n",
                               "이 입금에는 현금 수수료가 필요하지 않습니다.\n");
                feeCash = CashDrawer();
            } else {
                feeCash = PromptFeeCash(lang, depositFee);
            }
            atm->RequestDeposit(cash, checkAmount, feeCash, checkCount);
            break;
        }
        case 2: {
            long long amount = PromptLongLong(T(lang, "Enter withdrawal amount: ", "출금 금액을 입력하세요: "), 0);
            atm->RequestWithdrawal(amount);
            break;
        }
        case 3: {
            std::string targetAccount = PromptString(T(lang, "Enter destination account number: ", "상대 계좌 번호를 입력하세요: "));
            Account* destination = FindAccountByNumber(accounts, targetAccount);
            if (destination == nullptr) {
                std::cout << T(lang, "Account not found.\n", "계좌를 찾을 수 없습니다.\n");
                break;
            }
            long long amount = PromptLongLong(T(lang, "Enter transfer amount: ", "이체 금액을 입력하세요: "), 1);
            atm->RequestAccountTransfer(destination, amount);
            break;
        }
        case 4: {
            std::string targetAccount = PromptString(T(lang, "Enter destination account number: ", "상대 계좌 번호를 입력하세요: "));
            Account* destination = FindAccountByNumber(accounts, targetAccount);
            if (destination == nullptr) {
                std::cout << T(lang, "Account not found.\n", "계좌를 찾을 수 없습니다.\n");
                break;
            }
            CashDrawer cash = PromptCashDrawer(T(lang, "cash transfer", "현금 이체"));
            atm->RequestCashTransfer(destination, cash);
            break;
        }
        case 5:
            atm->PrintReceipt(std::cout);
            break;
        default:
            std::cout << T(lang, "Unknown option.\n", "알 수 없는 선택입니다.\n");
            break;
        }

        if (!atm->HasActiveSession()) {
            std::cout << T(lang, "Session ended due to an error.\n", "오류로 인해 세션이 종료되었습니다.\n");
            break;
        }
    }
}

void RunConsole(SystemState& state) {
    if (state.atms.empty()) {
        std::cout << "No ATMs are configured. Exiting.\n";
        return;
    }

    while (true) {
        PrintMainMenu();
        std::string choiceInput = PromptString("Select an option: ");

        if (choiceInput == "/") {
            PrintSnapshot(state.banks, state.atms);
            continue;
        }

        int choice = 0;
        {
            bool parsed = false;
            if (!choiceInput.empty()) {
                bool isNumber = true;
                for (std::size_t i = 0; i < choiceInput.size(); ++i) {
                    if (choiceInput[i] < '0' || choiceInput[i] > '9') {
                        isNumber = false;
                        break;
                    }
                }
                if (isNumber) {
                    choice = static_cast<int>(std::stoi(choiceInput));
                    parsed = true;
                }
            }
            if (!parsed) {
                std::cout << "Invalid input. Try again.\n";
                continue;
            }
        }

        if (choice == 0) {
            std::cout << "Goodbye!\n";
            break;
        }

        if (choice == 2) {
            PrintSnapshot(state.banks, state.atms);
            continue;
        }

        std::cout << "\nAvailable ATMs:\n";
        for (size_t i = 0; i < state.atms.size(); ++i) {
            const ATM* atm = state.atms[i];
            std::cout << i << ") Serial " << atm->GetSerialNumber()
                      << " (Primary: "
                      << (atm->GetPrimaryBank() ? atm->GetPrimaryBank()->getBankName() : "Unknown")
                      << ")\n";
        }
        int atmIndex = PromptInt("Select ATM index: ", 0);
        if (atmIndex < 0 || static_cast<size_t>(atmIndex) >= state.atms.size()) {
            std::cout << "Invalid ATM selection.\n";
            continue;
        }

        ATM* atm = state.atms[atmIndex];
        ATMLanguage langChoice = SelectLanguageForAtm(atm);
        std::cout << T(langChoice, "1) Customer session\n", "1) 고객 세션\n");
        std::cout << T(langChoice, "2) Admin transaction history\n", "2) 관리자 거래 내역\n");
        std::cout << T(langChoice, "0) Cancel\n", "0) 취소\n");
        int sessionChoice = PromptInt(T(langChoice, "Select session type: ", "세션 유형을 선택하세요: "), 0);
        if (sessionChoice == 0) {
            continue;
        }

        if (sessionChoice == 2) {
            Bank* primaryBank = atm->GetPrimaryBank();
            if (primaryBank == nullptr) {
                std::cout << "This ATM does not have a primary bank configured.\n";
                continue;
            }

            atm->StartAdminSession(nullptr);
            bool authenticated = false;
            int attempts = 0;
            while (attempts < 3 && !authenticated) {
                std::string adminCardNumber = PromptString(T(langChoice, "Enter admin card number: ", "관리자 카드 번호를 입력하세요: "));
                std::string adminPassword = PromptString(T(langChoice, "Enter admin password: ", "관리자 비밀번호를 입력하세요: "));
                if (primaryBank->verifyAdminCredentials(adminCardNumber, adminPassword)) {
                    authenticated = true;
                    break;
                }
                std::cout << T(langChoice, "Wrong admin credentials.\n", "관리자 정보가 올바르지 않습니다.\n");
                ++attempts;
            }

            if (!authenticated) {
                std::cout << T(langChoice, "Admin authentication failed. Returning to main menu.\n",
                               "관리자 인증에 실패했습니다. 메인 메뉴로 돌아갑니다.\n");
                atm->EndSession();
                continue;
            }

            ++state.totalSessions;
            ++state.adminSessions;
            atm->IncrementAdminSession();
            RunAdminMenu(atm, state.banks, state.atms, atm->GetActiveLanguage());
            atm->EndSession();
            continue;
        }

        std::string cardNumber = PromptString(T(langChoice, "Enter card number (or type /cancel): ",
                                                            "카드 번호를 입력하세요 (/cancel 입력 시 취소): "));
        if (cardNumber == "/cancel") {
            continue;
        }

        Account* initialAccount = FindAccountByCard(state.accounts, cardNumber);
        if (initialAccount == nullptr) {
            std::cout << T(langChoice, "Card not recognized.\n", "인식되지 않는 카드입니다.\n");
            atm->StartCustomerSession(nullptr, nullptr, false);
            continue;
        }
        Bank* bank = initialAccount->getBank();
        if (bank == nullptr) {
            atm->StartCustomerSession(initialAccount->getLinkedCard(), initialAccount, false);
            std::cout << T(langChoice, "Account is not associated with a bank.\n", "계좌가 은행과 연결되어 있지 않습니다.\n");
            atm->EndSession();
            continue;
        }

        if (!atm->SupportsBank(bank)) {
            atm->StartCustomerSession(initialAccount->getLinkedCard(), initialAccount, false);
            std::cout << T(langChoice, "This ATM does not support the selected bank/card.\n",
                           "이 ATM은 선택한 은행/카드를 지원하지 않습니다.\n");
            atm->EndSession();
            continue;
        }

        bool isPrimary = (atm->GetPrimaryBank() == bank);
        atm->StartCustomerSession(initialAccount->getLinkedCard(), initialAccount, isPrimary);
        if (!atm->HasActiveSession()) {
            continue;
        }

        Account* verifiedAccount = nullptr;
        int attempts = 0;
        while (attempts < 3 && verifiedAccount == nullptr) {
            std::string password = PromptString(T(langChoice, "Enter PIN/password: ", "PIN/비밀번호를 입력하세요: "));
            Account* tempAccount = nullptr;
            if (bank->verifyUserCredentials(cardNumber, password, tempAccount)) {
                verifiedAccount = tempAccount;
                break;
            }
            std::cout << T(langChoice, "Wrong password.\n", "비밀번호가 올바르지 않습니다.\n");
            ++attempts;
        }

        if (verifiedAccount == nullptr) {
            std::cout << T(langChoice, "Too many wrong password attempts. Session aborted.\n",
                           "비밀번호 오류가 많아 세션을 종료합니다.\n");
            atm->EndSession();
            continue;
        }

        atm->IncrementCustomerSession();
        RunAtmMenu(atm, state.accounts, state.banks, state.atms);
    }
}

void Cleanup(SystemState& state) {
    for (Transaction* transaction : state.transactions) {
        delete transaction;
    }
    state.transactions.clear();

    for (ATM* atm : state.atms) {
        delete atm;
    }
    state.atms.clear();

    for (Account* account : state.accounts) {
        delete account;
    }
    state.accounts.clear();

    for (Card* card : state.cards) {
        delete card;
    }
    state.cards.clear();

    for (Bank* bank : state.banks) {
        delete bank;
    }
    state.banks.clear();
}

} // namespace

int main() {
    SystemState state;
    if (!LoadInitialData("sample_initial_condition.txt", state)) {
        return 1;
    }

    PrintWelcomeBanner();
    ConfigureAdminCards(state);
    RunConsole(state);
    Cleanup(state);
    return 0;
}

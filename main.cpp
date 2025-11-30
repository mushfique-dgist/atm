#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>

#include "Account.hpp"
#include "Bank.hpp"
#include "Card.hpp"
#include "Transaction.hpp"
#include "Atm.hpp"

struct SystemState {
    std::vector<Bank*> banks;                  // Maintains insertion order
    std::map<std::string, Bank*> banksByName;  // Fast lookup by name
    std::vector<Account*> accounts;
    std::vector<Card*> cards;
    std::vector<ATM*> atms;
    std::vector<Transaction*> transactions;
    int totalSessions = 0;
    int customerSessions = 0;
    int adminSessions = 0;
};

SystemState globalSystemState;
ATMLanguage globalLanguage = ATMLanguage_English;
ATM* globalCurrentATM = nullptr;
Account* globalCurrentAccount = nullptr;
Account* globalDestinationAccount = nullptr;

namespace {

void PrintSnapshot(const std::vector<Bank*>& banks, const std::vector<ATM*>& atms, ATMLanguage lang);

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
        std:: string input;
        std:: cin >> input;
         if (input == "/"){
                PrintSnapshot(globalSystemState.banks, globalSystemState.atms, globalLanguage);
            }
        try {
            value = std::stoi(input);
            if (value >= minValue) {
                return value;
            }   
        std::cout << "Invalid input. Try again.\n";
        ClearInputLine();
        } catch (...) {
        }
        
    }
}

int PromptIntWithMax(const std::string& message, int minValue, int maxValue) {
    int value = 0;
    while (true) {
        std::cout << message;
        std:: string input;
        std:: cin >> input;
        if (input == "/"){
                PrintSnapshot(globalSystemState.banks, globalSystemState.atms, globalLanguage);
            }
        try {
            value = std::stoi(input);
            if (value >= minValue && value <= maxValue) {
                return value;
            }   
        std::cout << "Invalid input. Try again.\n";
        ClearInputLine();
        } catch (...) {
            
        }
    }
}

long long PromptLongLong(const std::string& message, long long minValue) {
    long long value = 0;
    while (true) {
        std::cout << message;
        std:: string input;
        std:: cin >> input;
        if (input == "/"){
                PrintSnapshot(globalSystemState.banks, globalSystemState.atms, globalLanguage);
            }
        try {
            value = std::stoll(input);
            if (value >= minValue) {
                return value;
            }   
        std::cout << "Invalid input. Try again.\n";
        ClearInputLine();
        } catch (...) {
            
        }
        
    }
}

long long PromptLongLongWithMax(const std::string& message, long long minValue, long long maxValue) {
    long long value = 0;
    while (true) {
        std::cout << message;
        std:: string input;
        std:: cin >> input;
        try {
            value = std::stoll(input);
            if (value >= minValue && value <= maxValue) {
                return value;
            }   
        std::cout << "Invalid input. Try again.\n";
        ClearInputLine();
        } catch (...) {
            if (input == "/"){
                PrintSnapshot(globalSystemState.banks, globalSystemState.atms, globalLanguage);
            }
        }
        
    }
}

std::string PromptString(const std::string& message) {
    std::cout << message;
    std::string input;
    std::cin >> input;
    if (input == "/"){
        PrintSnapshot(globalSystemState.banks, globalSystemState.atms, globalLanguage);
    }
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

Bank* FindBank(const std::map<std::string, Bank*>& banksByName, const std::string& name) {
    auto it = banksByName.find(name);
    return (it != banksByName.end()) ? it->second : nullptr;
}

Account* FindAccountByCard(ATM* atm, const std::map<std::string, Bank*>& banksByName, const std::string& cardNumber) {
    // 1. Search primary bank of ATM first
    Bank* primaryBank = atm->GetPrimaryBank();
    if (primaryBank != nullptr) {
        Account* account = primaryBank->findAccountByCardNumber(cardNumber);
        if (account != nullptr) {
            return account;
        }
    }
    if (atm->GetBankAccessMode() == ATMBankAccess_SingleBank) {
        return nullptr; // No need to search other banks in single-bank mode
    }
    
    // 2. Search other banks
    for (auto it = banksByName.begin(); it != banksByName.end(); ++it) {
        Bank* bank = it->second;
        if (bank == primaryBank) {
            continue; // Skip primary bank already searched
        }
        Account* account = bank->findAccountByCardNumber(cardNumber);
        if (account != nullptr) {
            return account;
        }
    }
    return nullptr;
}

Account* FindAccountByNumber(ATM* atm, const std::map<std::string, Bank*>& banksByName, const std::string& accountNumber) {
    // 1. Search primary bank of ATM first
    Bank* primaryBank = atm->GetPrimaryBank();
    if (primaryBank != nullptr) {
        Account* account = primaryBank->findAccountByAccountNumber(accountNumber);
        if (account != nullptr) {
            return account;
        }
        
        // 2. Search other banks through primary bank
        return primaryBank->findAccountInOtherBanks(accountNumber);
    }
    return nullptr;
}

long long PromptCheckAmounts(int& checkCount) {
    long long total = 0;
    checkCount = 0;
    while (true) {
        if (checkCount >= 10) {
            std::cout << T(globalLanguage, 
                "Maximum of 10 checks reached.\n", "최대 10장의 수표만 입력할 수 있습니다.\n");
            break;
        }
        long long amount = PromptLongLong(T(globalLanguage, 
            "Enter check amount (0 to finish): ", "수표 금액을 입력하세요 (0 입력 시 종료): "), 0);
        if (amount == 0) {
            break;
        }
        if (amount < 100000) {
            std::cout << T(globalLanguage,
                "Each check must be at least 100,000 KRW.\n", "각 수표는 최소 100,000원이어야 합니다.\n");
            continue;
        }
        total += amount;
        ++checkCount;
    }
    return total;
}

CashDrawer PromptCashDrawer(const std::string& label, int maxBills) {
    CashDrawer drawer;
    for (int i = 0; i < CASH_TYPE_COUNT; ++i) {
        drawer.noteCounts[i] = 0;
    }

    std::cout << T(globalLanguage, 
        "Enter bills for " + label + " (use non-negative integers).\n",
         "지폐 개수를 입력하세요 " + label + " (음수가 아닌 정수를 사용하세요).\n");
    int count50k = PromptIntWithMax(T(globalLanguage, "50,000 KRW bills: ", "50,000원 지폐: "), 0, maxBills);
    maxBills -= count50k;
    int count10k = PromptIntWithMax(T(globalLanguage, "10,000 KRW bills: ", "10,000원 지폐: "), 0, maxBills);
    maxBills -= count10k;
    int count5k = PromptIntWithMax(T(globalLanguage, "5,000 KRW bills: ", "5,000원 지폐: "), 0, maxBills);
    maxBills -= count5k;
    int count1k = PromptIntWithMax(T(globalLanguage, "1,000 KRW bills: ", "1,000원 지폐: "), 0, maxBills);

    drawer.noteCounts[0] = count1k;
    drawer.noteCounts[1] = count5k;
    drawer.noteCounts[2] = count10k;
    drawer.noteCounts[3] = count50k;
    return drawer;
}

void PrintSnapshot(const std::vector<Bank*>& banks, const std::vector<ATM*>& atms, ATMLanguage lang = ATMLanguage_English) {
    std::cout << "\n=== " << T(lang, "Snapshot", "스냅샷") << " ===\n";

    std::cout << T(lang, "ATMs (remaining cash):", "ATM (잔여 현금):") << "\n";
    for (std::size_t i = 0; i < atms.size(); ++i) {
        const ATM* atm = atms[i];
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

        // Mark current ATM with text
        std::string marker = (atm == globalCurrentATM) ? T(lang, " (currently in use)", " (현재 사용 중)") : "";
        
        std::cout << bankName << " ATM [SN:" << atm->GetSerialNumber() << "]" << marker << " "
                  << T(lang, "Remaining cash: ", "잔여 현금: ") << totalCash
                  << T(lang, " | Left cash: ", " | 남은 지폐: ")
                  << count1k << T(lang, " x 1,000 won, ", " x 1,000원, ")
                  << count5k << T(lang, " x 5,000 won, ", " x 5,000원, ")
                  << count10k << T(lang, " x 10,000 won, ", " x 10,000원, ")
                  << count50k << T(lang, " x 50,000 won", " x 50,000원") << "\n";
    }

    std::cout << "\n" << T(lang, "Accounts (remaining balance):", "계좌 (잔액):") << "\n";
    for (std::size_t i = 0; i < banks.size(); ++i) {
        const Bank* bank = banks[i];
        if (bank == nullptr) {
            continue;
        }
        std::vector<Account*> accounts = bank->getAccounts();
        for (std::size_t j = 0; j < accounts.size(); ++j) {
            Account* account = accounts[j];
            if (account == nullptr) {
                continue;
            }
            // Mark current account and destination account with text
            std::string marker = "";
            if (account == globalCurrentAccount) {
                marker = T(lang, " (logged in)", " (로그인 중)");
            } else if (account == globalDestinationAccount) {
                marker = T(lang, " (transaction destination)", " (거래 대상)");
            }
            
            std::cout << T(lang, "Account", "계좌") << " ["
                      << T(lang, "Bank", "은행") << ": " << bank->getBankName()
                      << ", " << T(lang, "No.", "번호") << " " << account->getAccountNumber()
                      << ", " << T(lang, "Owner", "소유자") << ": " << account->getOwnerName()
                      << "]" << marker << " " << T(lang, "Balance", "잔액") << " : " << account->getBalance() << "\n";
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

    int count50k = PromptInt(T(lang, "50,000 KRW bills: ", "50,000원 지폐: "), 0);
    int count10k = PromptInt(T(lang, "10,000 KRW bills: ", "10,000원 지폐: "), 0);
    int count5k = PromptInt(T(lang, "5,000 KRW bills: ", "5,000원 지폐: "), 0);
    int count1k = PromptInt(T(lang, "1,000 KRW bills: ", "1,000원 지폐: "), 0);

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
        Bank* bank = new Bank(bankName, bankName, &state.banksByName, &state.transactions);
        state.banks.push_back(bank);
        state.banksByName[bankName] = bank;
    }

    for (int i = 0; i < accountCount; ++i) {
        std::string bankName;
        std::string userName;
        std::string accountNumber;
        long long availableFunds = 0;
        std::string cardNumber;
        std::string password;
        fin >> bankName >> userName >> accountNumber >> availableFunds >> cardNumber >> password;

        Bank* bank = FindBank(state.banksByName, bankName);
        if (bank == nullptr) {
            std::cerr << "Bank " << bankName << " not found for account " << accountNumber << ".\n";
            return false;
        }
        Card* card = new Card(cardNumber, bankName, CardRole::User);
        Account* account = new Account(bank, userName, accountNumber, availableFunds, card, password);

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

        Bank* primaryBank = FindBank(state.banksByName, primaryBankName);
        if (primaryBank == nullptr) {
            std::cerr << "Primary bank " << primaryBankName << " not found for ATM " << serial << ".\n";
            return false;
        }

        ATMBankAccess accessMode =
            (accessModeStr == "Single") ? ATMBankAccess_SingleBank : ATMBankAccess_MultiBank;
        bool bilingual = (languageStr == "Bilingual");

        ATM* atm = new ATM(serial, primaryBank, accessMode, bilingual);

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
            for (const auto& pair : state.banksByName) {
                atm->AddAcceptedBank(pair.second);
            }
        }

        state.atms.push_back(atm);
    }

    return true;
}

bool isCardInSystem(const SystemState& state, const std::string& cardNumber) {
    for (const Card* card : state.cards) {
        if (cardNumber == card->getNumber()) {
            return true;
        }
    }
    return false;
}

} // namespace


void ConfigureAdminCards(SystemState& state) {
    std::cout << "\n=== Admin Card Setup ===\n";
    for (std::size_t i = 0; i < state.banks.size(); ++i) {
        Bank* bank = state.banks[i];
        if (bank == nullptr) {
            continue;
        }
        std::cout << "Configuring admin card for bank:" << bank->getBankName() << "\n";
        std::string adminCardNumber = PromptString("  Enter an admin card number: ");
        while (adminCardNumber== "/") {
            adminCardNumber = PromptString("  Enter an admin card number: ");
        }
        while (true) {    
            if (!isCardInSystem(state, adminCardNumber)) {
                break;
            }
            std::cout << "  Card number already exists. Please enter a different admin card number.\n";
            adminCardNumber = PromptString(" Enter an unique admin card number: ");
        }
        std::string adminPassword = PromptString(" Enter an admin password/PIN: ");
        while (adminPassword== "/") {
            adminPassword = PromptString(" Enter an admin password/PIN: ");
        }
        bank->setAdminCard(adminCardNumber, adminPassword);
        state.cards.push_back(new Card(adminCardNumber, bank->getBankName(), CardRole::Admin));
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
        std::cout << "\n========================================"<<std::endl;
        std::cout << "              ADMIN MENU                "<<std::endl;
        std::cout << "========================================"<<std::endl;
        std::cout << "  [1] " << T(lang, "Print all transactions", "모든 거래 출력") <<std::endl;
        std::cout << "  [2] " << T(lang, "Export transactions to file", "거래 내역 파일로 저장") << std::endl;
        if (atm->IsBilingual()){
            std::cout << "  [3] " << T(lang, "Change language", "언어 변경") << std::endl;
        }
        std::cout << "  [/] " << T(lang, "Snapshot", "스냅샷") << std::endl;
        std::cout << "  [0] " << T(lang, "Exit admin menu", "관리자 메뉴 종료") << std::endl;
        std::cout << "========================================"<<std::endl;
        std::string choiceInput = PromptString(T(lang, "Select an option: ", "옵션을 선택하세요: "));
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
            fout << "/\n";
            if (lang == ATMLanguage_English) {
                fout << "This ATM sessions: " << totalSessions
                     << " (customer " << customerSessions
                     << ", admin " << adminSessions << ")\n";
            } else {
                fout << "이 ATM 세션 수: " << totalSessions
                     << " (고객 " << customerSessions
                     << ", 관리자 " << adminSessions << ")\n";
            }
            PrintTransactions(transactions, fout, lang);
            std::cout << T(lang, "Transactions exported to ", "거래 내역을 파일로 저장했습니다: ") << filename << "\n";
            break;
        }
        default:
            if (choice == 3 && atm->IsBilingual()) {
                ATMLanguage newLang = SelectLanguageForAtm(atm);
                lang = newLang;
                globalLanguage = newLang;
                std::cout << T(lang, "Language changed.\n", "언어가 변경되었습니다.\n");
                continue;
            }
            std::cout << T(lang, "Unknown choice.\n", "알 수 없는 선택입니다.\n");
            break;
        }
    }
}

void RunAtmMenu(ATM* atm,
                const std::vector<Account*>& accounts,
                const std::vector<Bank*>& banks,
                const std::map<std::string, Bank*>& banksByName,
                const std::vector<ATM*>& atms) {
    if (atm == nullptr) {
        return;
    }

    while (true) {
        ATMLanguage lang = atm->GetActiveLanguage();
        globalLanguage = lang;
        std::cout << "\n========================================\n";
        std::cout << T(lang, "          ATM MENU - Serial ", "          ATM 메뉴 - 일련번호 ") << atm->GetSerialNumber() << "         \n";
        std::cout << "========================================\n";
        std::cout << "  [1] " << T(lang, "Deposit", "입금") << "\n";
        std::cout << "  [2] " << T(lang, "Withdraw", "출금") << "\n";
        std::cout << "  [3] " << T(lang, "Account transfer", "계좌 이체") << "\n";
        std::cout << "  [4] " << T(lang, "Cash transfer", "현금 이체") << "\n";
        std::cout << "  [5] " << T(lang, "Print receipt", "영수증 출력") << "\n";
        if (atm->IsBilingual()){
            std::cout << "  [6] " << T(lang, "Change language", "언어 변경") << "\n";
        }
        std::cout << "  [/] " << T(lang, "Snapshot", "스냅샷") << "\n";
        std::cout << "  [0] " << T(lang, "End session", "세션 종료") << "\n";
        std::cout << "========================================\n";

        std::string choiceInput = PromptString(T(lang, "Select an option: ", "옵션을 선택하세요: "));
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
            CashDrawer cash = PromptCashDrawer(T(lang, "deposit", "입금"), atm->GetMaxDepositBillsPerSession());
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
            Account* destination = FindAccountByNumber(atm, banksByName, targetAccount);
            if (destination == nullptr) {
                std::cout << T(lang, "Account not found.\n", "계좌를 찾을 수 없습니다.\n");
                break;
            }
            globalDestinationAccount = destination;
            long long amount = PromptLongLong(T(lang, "Enter transfer amount: ", "이체 금액을 입력하세요: "), 1);
            atm->RequestAccountTransfer(destination, amount);
            globalDestinationAccount = nullptr;
            break;
        }
        case 4: {
            std::string targetAccount = PromptString(T(lang, "Enter destination account number: ", "상대 계좌 번호를 입력하세요: "));
            Account* destination = FindAccountByNumber(atm, banksByName, targetAccount);
            if (destination == nullptr) {
                std::cout << T(lang, "Account not found.\n", "계좌를 찾을 수 없습니다.\n");
                break;
            }
            globalDestinationAccount = destination;
            CashDrawer cash = PromptCashDrawer(T(lang, "cash transfer", "현금 이체"),atm->GetMaxDepositBillsPerSession());
            atm->RequestCashTransfer(destination, cash);
            globalDestinationAccount = nullptr;
            break;
        }
        case 5:
            atm->PrintReceipt(std::cout);
            break;
        case '/':
            break;
        case 6:
            if (atm->IsBilingual()){
                ATMLanguage newLang = SelectLanguageForAtm(atm);
                atm->SetLanguage(newLang);
                lang = newLang;
                globalLanguage = newLang;
                std::cout << T(lang, "Language changed.\n", "언어가 변경되었습니다.\n");
                break;
            }
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
        globalSystemState = state;
        globalLanguage = ATMLanguage_English;
        PrintMainMenu();
        std::string choiceInput = PromptString("Select an option: ");


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


        std::string cardNumber = PromptString(T(langChoice, "Enter card number (or type /cancel): ",
                                                            "카드 번호를 입력하세요 (/cancel 입력 시 취소): "));
        if (cardNumber == "/cancel") {
            continue;
        }
        if (atm->GetPrimaryBank()->isAdminCard(cardNumber)){
             Bank* primaryBank = atm->GetPrimaryBank();
            if (primaryBank == nullptr) {
                std::cout << T(langChoice, "This ATM does not have a primary bank configured.\n", "이 ATM에는 기본 은행이 구성되어 있지 않습니다.\n");
                continue;
            }
            std::cout<< T(langChoice, "Starting admin session...\n", "관리자 세션을 시작합니다...\n");
            atm->StartAdminSession(nullptr);
            bool authenticated = false;
            int attempts = 0;
            while (attempts < 3 && !authenticated) {
                std::string adminCardNumber = cardNumber;
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
            globalCurrentATM = atm;
            globalCurrentAccount = nullptr;
            globalLanguage = atm->GetActiveLanguage();
            RunAdminMenu(atm, state.banks, state.atms, atm->GetActiveLanguage());
            globalCurrentATM = nullptr;
            globalCurrentAccount = nullptr;
            atm->EndSession();
            continue;
            
        }

        Account* initialAccount = FindAccountByCard(atm, state.banksByName, cardNumber);
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
        globalCurrentATM = atm;
        globalCurrentAccount = verifiedAccount;
        globalLanguage = atm->GetActiveLanguage();
        RunAtmMenu(atm, state.accounts, state.banks, state.banksByName, state.atms);
        globalCurrentATM = nullptr;
        globalCurrentAccount = nullptr;
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

    for (std::size_t i = 0; i < state.cards.size(); ++i) {
        delete state.cards[i];
    }
    state.cards.clear();

    for (std::size_t i = 0; i < state.banks.size(); ++i) {
        delete state.banks[i];
    }
    state.banks.clear();
    state.banksByName.clear();
}

int main() {
    SystemState state;
    std::cout << "Loading initial data from file..."<<std::endl;
    std::cout<< "If the file input was not appropriately prepared, prepared data will be provided." <<std::endl;
    std:: cout << "Enter the path to the initial data file: ";
    std::string path;
    std::cin >> path;
    bool success = LoadInitialData(path, state);
    if (success) {
        std::cout << "Initial data loaded successfully from " << path << ".\n";
    } 
    else {
        std::cout << "Failed to load initial data from " << path << ". Using default sample data.\n";
        success = LoadInitialData("sample_initial_condition.txt", state);
        if (!success) {
            std::cerr << "Critical error: Cannot load default sample data. Exiting.\n";
            return 1;
        }
        std::cout << "Default data loaded successfully.\n";
    }
    globalSystemState = state;
    PrintWelcomeBanner();
    ConfigureAdminCards(state);
    globalSystemState = state;
    RunConsole(state);
    Cleanup(state);
    return 0;
}

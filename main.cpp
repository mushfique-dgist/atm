#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
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
    std::unordered_map<std::string, Account*> accountByNumber;
    std::unordered_map<std::string, Account*> accountByCard;
};

namespace {

void ClearInputLine() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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

Bank* FindBank(const std::vector<Bank*>& banks, const std::string& name) {
    for (Bank* bank : banks) {
        if (bank != nullptr && bank->getBankName() == name) {
            return bank;
        }
    }
    return nullptr;
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

void PrintSnapshot(const std::vector<Bank*>& banks, const std::vector<ATM*>& atms) {
    std::cout << "\n=== Snapshot ===\n";
    std::cout << "Banks and accounts:\n";
    for (const Bank* bank : banks) {
        if (bank == nullptr) {
            continue;
        }
        std::cout << "- Bank: " << bank->getBankName() << "\n";
        for (Account* account : bank->getAccounts()) {
            if (account == nullptr) {
                continue;
            }
            std::cout << "    Account " << account->getAccountNumber()
                      << " (Owner: " << account->getOwnerName()
                      << ", Balance: " << account->getBalance() << ")\n";
        }
    }

    std::cout << "\nATMs:\n";
    for (const ATM* atm : atms) {
        if (atm == nullptr) {
            continue;
        }
        std::cout << "- ATM Serial: " << atm->GetSerialNumber()
                  << " (Primary Bank: "
                  << (atm->GetPrimaryBank() ? atm->GetPrimaryBank()->getBankName() : "Unknown")
                  << ")\n";
    }
    std::cout << "================\n";
}

void PrintTransactions(const std::vector<Transaction*>& transactions, std::ostream& out) {
    if (transactions.empty()) {
        out << "No transactions recorded yet.\n";
        return;
    }
    for (const Transaction* transaction : transactions) {
        if (transaction != nullptr) {
            transaction->logToStream(out);
            out << "\n";
        }
    }
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
        state.accountByCard[cardNumber] = account;
        state.accountByNumber[accountNumber] = account;
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
        std::string adminCardNumber = PromptString("  Enter admin card number: ");
        std::string adminPassword = PromptString("  Enter admin password/PIN: ");
        bank->setAdminCard(adminCardNumber, adminPassword);
    }
    std::cout << "========================\n";
}

void RunAdminMenu(const std::vector<Transaction*>& transactions) {
    while (true) {
        std::cout << "\nAdmin Menu\n";
        std::cout << "1) Print all transactions\n";
        std::cout << "2) Export transactions to file\n";
        std::cout << "0) Exit admin menu\n";
        int choice = PromptInt("Select an option: ", 0);

        if (choice == 0) {
            break;
        }

        switch (choice) {
        case 1:
            PrintTransactions(transactions, std::cout);
            break;
        case 2: {
            std::string filename = PromptString("Enter output filename: ");
            std::ofstream fout(filename);
            if (!fout) {
                std::cout << "Failed to open file.\n";
                break;
            }
            PrintTransactions(transactions, fout);
            std::cout << "Transactions exported to " << filename << "\n";
            break;
        }
        default:
            std::cout << "Unknown choice.\n";
            break;
        }
    }
}

void RunAtmMenu(ATM* atm,
                const std::vector<Bank*>& banks,
                const std::unordered_map<std::string, Account*>& accountByNumber) {
    if (atm == nullptr) {
        return;
    }

    while (true) {
        std::cout << "\nATM Menu (Serial " << atm->GetSerialNumber() << ")\n";
        std::cout << "1) Deposit\n";
        std::cout << "2) Withdraw\n";
        std::cout << "3) Account transfer\n";
        std::cout << "4) Cash transfer\n";
        std::cout << "5) Print receipt\n";
        std::cout << "0) End session\n";
        int choice = PromptInt("Select an option: ", 0);

        if (choice == 0) {
            std::cout << "\nSession summary:\n";
            atm->PrintReceipt(std::cout);
            std::cout << "Session ended.\n";
            atm->EndSession();
            break;
        }

        switch (choice) {
        case 1: {
            CashDrawer cash = PromptCashDrawer("deposit");
            long long checkAmount = PromptLongLong("Enter check amount (0 if none): ", 0);
            atm->RequestDeposit(cash, checkAmount);
            break;
        }
        case 2: {
            long long amount = PromptLongLong("Enter withdrawal amount: ", 0);
            atm->RequestWithdrawal(amount);
            break;
        }
        case 3: {
            std::string targetAccount = PromptString("Enter destination account number: ");
            auto it = accountByNumber.find(targetAccount);
            if (it == accountByNumber.end()) {
                std::cout << "Account not found.\n";
                break;
            }
            long long amount = PromptLongLong("Enter transfer amount: ", 1);
            atm->RequestAccountTransfer(it->second, amount);
            break;
        }
        case 4: {
            std::string targetAccount = PromptString("Enter destination account number: ");
            auto it = accountByNumber.find(targetAccount);
            if (it == accountByNumber.end()) {
                std::cout << "Account not found.\n";
                break;
            }
            CashDrawer cash = PromptCashDrawer("cash transfer");
            atm->RequestCashTransfer(it->second, cash);
            break;
        }
        case 5:
            atm->PrintReceipt(std::cout);
            break;
        default:
            std::cout << "Unknown option.\n";
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
        std::cout << "\n=== Main Menu ===\n";
        std::cout << "1) Use an ATM\n";
        std::cout << "2) Show snapshot\n";
        std::cout << "0) Exit\n";
        int choice = PromptInt("Select an option: ", 0);

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
        std::cout << "1) Customer session\n";
        std::cout << "2) Admin transaction history\n";
        std::cout << "0) Cancel\n";
        int sessionChoice = PromptInt("Select session type: ", 0);
        if (sessionChoice == 0) {
            continue;
        }

        if (sessionChoice == 2) {
            Bank* primaryBank = atm->GetPrimaryBank();
            if (primaryBank == nullptr) {
                std::cout << "This ATM does not have a primary bank configured.\n";
                continue;
            }

            bool authenticated = false;
            int attempts = 0;
            while (attempts < 3 && !authenticated) {
                std::string adminCardNumber = PromptString("Enter admin card number: ");
                std::string adminPassword = PromptString("Enter admin password: ");
                if (primaryBank->verifyAdminCredentials(adminCardNumber, adminPassword)) {
                    authenticated = true;
                    break;
                }
                std::cout << "Wrong admin credentials.\n";
                ++attempts;
            }

            if (!authenticated) {
                std::cout << "Admin authentication failed. Returning to main menu.\n";
                continue;
            }

            RunAdminMenu(state.transactions);
            continue;
        }

        std::string cardNumber = PromptString("Enter card number (or type /cancel): ");
        if (cardNumber == "/cancel") {
            continue;
        }

        auto accountIt = state.accountByCard.find(cardNumber);
        if (accountIt == state.accountByCard.end()) {
            std::cout << "Card not recognized.\n";
            continue;
        }

        Account* initialAccount = accountIt->second;
        Bank* bank = initialAccount->getBank();
        if (bank == nullptr) {
            std::cout << "Account is not associated with a bank.\n";
            continue;
        }

        if (!atm->SupportsBank(bank)) {
            std::cout << "This ATM does not support the selected bank/card.\n";
            continue;
        }

        Account* verifiedAccount = nullptr;
        int attempts = 0;
        while (attempts < 3 && verifiedAccount == nullptr) {
            std::string password = PromptString("Enter PIN/password: ");
            Account* tempAccount = nullptr;
            if (bank->verifyUserCredentials(cardNumber, password, tempAccount)) {
                verifiedAccount = tempAccount;
                break;
            }
            std::cout << "Wrong password.\n";
            ++attempts;
        }

        if (verifiedAccount == nullptr) {
            std::cout << "Too many wrong password attempts. Session aborted.\n";
            continue;
        }

        bool isPrimary = (atm->GetPrimaryBank() == bank);
        atm->StartCustomerSession(verifiedAccount->getLinkedCard(), verifiedAccount, isPrimary);
        if (!atm->HasActiveSession()) {
            continue;
        }

        RunAtmMenu(atm, state.banks, state.accountByNumber);
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

    ConfigureAdminCards(state);
    RunConsole(state);
    Cleanup(state);
    return 0;
}

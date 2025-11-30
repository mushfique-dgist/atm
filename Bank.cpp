#include "Bank.hpp"
#include "Account.hpp"
#include "Card.hpp"
#include "Transaction.hpp"

std::hash<std::string> Bank::passwordhasher;

Bank::Bank(const std::string& bankName,
           const std::string& bankId,
           std::map<std::string, Bank*>* allBanks,
           std::vector<Transaction*>* transactions)
    : bankName_(bankName),
      bankID_(bankId),
      accountsByNumber_(),
      accountsByCard_(),
      cardsByNumber_(),
      allBanks_(allBanks),
      transactions_(transactions),
      adminCard_(0),
      adminPassword_(0),
      powerAccount_(nullptr) {
    // Create power account for inter-bank transfers
    powerAccount_ = new Account(this, "POWER", bankName_ + "-POWER", 0, nullptr, "POWER");
}

Bank::~Bank() {
    if (adminCard_ != 0) {
        delete adminCard_;
        adminCard_ = 0;
    }
    if (powerAccount_ != nullptr) {
        delete powerAccount_;
        powerAccount_ = nullptr;
    }
}

const std::string& Bank::getBankName() const {
    return bankName_;
}

const std::string& Bank::getBankID() const {
    return bankID_;
}

std::vector<Account*> Bank::getAccounts() const {
    std::vector<Account*> result;
    result.reserve(accountsByNumber_.size());
    for (const auto& pair : accountsByNumber_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<Card*> Bank::getCards() const {
    std::vector<Card*> result;
    result.reserve(cardsByNumber_.size());
    for (const auto& pair : cardsByNumber_) {
        result.push_back(pair.second);
    }
    return result;
}

void Bank::addAccount(Account* account) {
    if (account == nullptr) {
        return;
    }

    const std::string& accountNumber = account->getAccountNumber();
    if (accountsByNumber_.find(accountNumber) != accountsByNumber_.end()) {
        return; // Account already exists
    }

    accountsByNumber_[accountNumber] = account;

    Card* linkedCard = account->getLinkedCard();
    if (linkedCard != nullptr) {
        const std::string& cardNumber = linkedCard->getNumber();
        accountsByCard_[cardNumber] = account;
        cardsByNumber_[cardNumber] = linkedCard;
    }
}

Account* Bank::findAccountByAccountNumber(const std::string& accountNumber) const {
    auto it = accountsByNumber_.find(accountNumber);
    return (it != accountsByNumber_.end()) ? it->second : nullptr;
}

Account* Bank::findAccountByCardNumber(const std::string& cardNumber) const {
    auto it = accountsByCard_.find(cardNumber);
    return (it != accountsByCard_.end()) ? it->second : nullptr;
}

Account* Bank::findAccountInOtherBanks(const std::string& accountNumber) const {
    if (allBanks_ == nullptr) {
        return nullptr;
    }
    
    // Search in all banks except this bank
    for (auto it = allBanks_->begin(); it != allBanks_->end(); ++it) {
        Bank* bank = it->second;
        if (bank == this) {
            continue; // Skip this bank
        }
        Account* account = bank->findAccountByAccountNumber(accountNumber);
        if (account != nullptr) {
            return account;
        }
    }
    return nullptr;
}

void Bank::setAdminCard(const std::string& cardNumber, const std::string& password) {
    if (cardNumber.empty()) {
        return;
    }

    if (adminCard_ != 0) {
        cardsByNumber_.erase(adminCard_->getNumber());
        delete adminCard_;
        adminCard_ = 0;
    }

    adminCard_ = new Card(cardNumber, bankName_, CardRole::Admin);
    cardsByNumber_[cardNumber] = adminCard_;
    adminPassword_ = passwordhasher(password);
}

bool Bank::verifyUserCredentials(const std::string& cardNumber,
                                 const std::string& password,
                                 Account*& outAccount) const {
    outAccount = findAccountByCardNumber(cardNumber);
    if (outAccount == nullptr) {
        return false;
    }
    if (!outAccount->checkPassword(password)) {
        outAccount = nullptr;
        return false;
    }
    return true;
}

bool Bank::verifyAdminCredentials(const std::string& cardNumber,
                                  const std::string& password) const {
    if (adminCard_ == 0) {
        return false;
    }
    return adminCard_->getNumber() == cardNumber && adminPassword_ == passwordhasher(password);
}

void Bank::addTransaction(Transaction* transaction) {
    if (transaction == nullptr || transactions_ == nullptr) {
        return;
    }
    transactions_->push_back(transaction);
}

void Bank::setAllBanks(std::map<std::string, Bank*>* allBanks) {
    allBanks_ = allBanks;
}

std::map<std::string, Bank*>* Bank::getAllBanks() const {
    return allBanks_;
}

Account* Bank::getPowerAccount() const {
    return powerAccount_;
}

bool Bank::deposit(Account* account, long long amount) {
    if (account == nullptr || amount <= 0) {
        return false;
    }
    account->deposit(amount);
    return true;
}

bool Bank::withdraw(Account* account, long long amount) {
    if (account == nullptr || amount <= 0) {
        return false;
    }
    return account->withdraw(amount);
}

bool Bank::transfer(Account* fromAccount,
                    Account* toAccount,
                    long long amount,
                    long long fee) {
    if (fromAccount == nullptr || toAccount == nullptr) {
        return false;
    }
    if (amount <= 0 || fee < 0) {
        return false;
    }
    
    // 1. Check if fromAccount has sufficient funds
    const long long totalCost = amount + fee;
    if (fromAccount->getBalance() < totalCost) {
        return false;
    }
    
    Bank* fromBank = fromAccount->getBank();
    Bank* toBank = toAccount->getBank();
    
    // 2. Withdraw from source account (including fee)
    if (!fromAccount->withdraw(totalCost)) {
        return false;
    }
    
    // 3. If same bank, direct transfer
    if (fromBank == toBank) {
        toAccount->deposit(amount);
        return true;
    }
    
    // 4. Inter-bank transfer using power accounts
    // Step 1: Deposit to source bank's power account
    fromBank->getPowerAccount()->deposit(amount);
    
    // Step 2: Transfer between power accounts (virtual)
    if (fromBank->getPowerAccount()->getBalance() >= amount) {
        fromBank->getPowerAccount()->withdraw(amount);
        toBank->getPowerAccount()->deposit(amount);
    } else {
        // Rollback if power account transfer fails
        fromAccount->deposit(totalCost);
        return false;
    }
    
    // Step 3: Deposit to destination account from destination bank's power account
    if (toBank->getPowerAccount()->getBalance() >= amount) {
        toBank->getPowerAccount()->withdraw(amount);
        toAccount->deposit(amount);
    } else {
        // Rollback if final deposit fails
        toBank->getPowerAccount()->withdraw(amount);
        fromBank->getPowerAccount()->deposit(amount);
        fromAccount->deposit(totalCost);
        return false;
    }
    
    return true;
}

bool Bank::isAdminCard(const std::string& cardNumber) const {
    return adminCard_ != nullptr && adminCard_->getNumber() == cardNumber;
}

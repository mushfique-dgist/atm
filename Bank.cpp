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
      adminPassword_(0) {
}

Bank::~Bank() {
    if (adminCard_ != 0) {
        delete adminCard_;
        adminCard_ = 0;
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
    const long long totalCost = amount + fee;
    if (!fromAccount->withdraw(totalCost)) {
        return false;
    }
    toAccount->deposit(amount);
    return true;
}

bool Bank::isAdminCard(const std::string& cardNumber) const {
    return adminCard_ != nullptr && adminCard_->getNumber() == cardNumber;
}

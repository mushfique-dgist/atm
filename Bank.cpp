#include "Bank.hpp"

#include "Account.hpp"
#include "Card.hpp"
#include "Transaction.hpp"

Bank::Bank(const std::string& bankName,
           const std::string& bankId,
           std::vector<Bank*>* allBanks,
           std::vector<Transaction*>* transactions)
    : bankName_(bankName),
      bankID_(bankId),
      accounts_(),
      cards_(),
      allBanks_(allBanks),
      transactions_(transactions),
      adminCard_(0),
      adminPassword_() {
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

const std::vector<Account*>& Bank::getAccounts() const {
    return accounts_;
}

const std::vector<Card*>& Bank::getCards() const {
    return cards_;
}

void Bank::addAccount(Account* account) {
    if (account == nullptr) {
        return;
    }

    for (const Account* existing : accounts_) {
        if (existing != nullptr &&
            existing->getAccountNumber() == account->getAccountNumber()) {
            return;
        }
    }

    accounts_.push_back(account);

    Card* linkedCard = account->getLinkedCard();
    if (linkedCard == nullptr) {
        return;
    }

    bool found = false;
    for (std::size_t i = 0; i < cards_.size(); ++i) {
        if (cards_[i] == linkedCard) {
            found = true;
            break;
        }
    }
    if (!found) {
        cards_.push_back(linkedCard);
    }
}

Account* Bank::findAccountByAccountNumber(const std::string& accountNumber) const {
    for (Account* account : accounts_) {
        if (account != nullptr && account->getAccountNumber() == accountNumber) {
            return account;
        }
    }
    return nullptr;
}

Account* Bank::findAccountByCardNumber(const std::string& cardNumber) const {
    for (Account* account : accounts_) {
        if (account == nullptr) {
            continue;
        }
        Card* card = account->getLinkedCard();
        if (card != nullptr && card->getNumber() == cardNumber) {
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
        for (std::size_t i = 0; i < cards_.size(); ++i) {
            if (cards_[i] == adminCard_) {
                cards_.erase(cards_.begin() + static_cast<long>(i));
                break;
            }
        }
        delete adminCard_;
        adminCard_ = 0;
    }

    adminCard_ = new Card(cardNumber, bankName_, CardRole::Admin);
    cards_.push_back(adminCard_);
    adminPassword_ = password;
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
    return adminCard_->getNumber() == cardNumber && adminPassword_ == password;
}

void Bank::addTransaction(Transaction* transaction) {
    if (transaction == nullptr || transactions_ == nullptr) {
        return;
    }
    transactions_->push_back(transaction);
}

void Bank::setAllBanks(std::vector<Bank*>* allBanks) {
    allBanks_ = allBanks;
}

std::vector<Bank*>* Bank::getAllBanks() const {
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

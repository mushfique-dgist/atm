#include "Account.hpp"

#include "Bank.hpp"

std::hash<std::string> Account::passwordhasher;

Account::Account(Bank* owningBank,
                 const std::string& ownerName,
                 const std::string& accountNumber,
                 long long initialFunds,
                 Card* linkedCard,
                 const std::string& password)
    : bank_(owningBank),
      ownerName_(ownerName),
      accountNumber_(accountNumber),
      balance_(initialFunds >= 0 ? initialFunds : 0),
      accountCard_(linkedCard),
      password_(passwordhasher(password)) {
}

const std::string& Account::getAccountNumber() const {
    return accountNumber_;
}

const std::string& Account::getOwnerName() const {
    return ownerName_;
}

long long Account::getBalance() const {
    return balance_;
}

Card* Account::getLinkedCard() const {
    return accountCard_;
}

Bank* Account::getBank() const {
    return bank_;
}

const std::string& Account::getBankName() const {
    if (bank_ != nullptr) {
        return bank_->getBankName();
    }
    static const std::string emptyName;
    return emptyName;
}

void Account::deposit(long long amount) {
    if (amount <= 0) {
        return;
    }
    balance_ += amount;
}

bool Account::withdraw(long long amount) {
    if (amount <= 0) {
        return false;
    }
    if (amount > balance_) {
        return false;
    }
    balance_ -= amount;
    return true;
}

void Account::recordTransaction(Transaction* accountTransaction) {
    if (accountTransaction != nullptr) {
        transactionHistory_.push_back(accountTransaction);
    }
}

bool Account::checkPassword(const std::string& enteredPassword) const {
    return password_ == passwordhasher(enteredPassword);
}

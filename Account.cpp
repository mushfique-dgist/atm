#include "Account.hpp"

Account::Account(Bank* owningBank,
                 const std::string& ownerName,
                 const std::string& accountNumber,
                 double initialFunds,
                 Card* linkedCard,
                 int passwordHash)
    : bank_(owningBank),
      ownerName_(ownerName),
      accountNumber_(accountNumber),
      availableFunds_(initialFunds),
      accountCard_(linkedCard),
      passwordHash_(passwordHash) {
}

const std::string& Account::getAccountNumber() const {
    return accountNumber_;
}

const std::string& Account::getOwnerName() const {
    return ownerName_;
}

double Account::getAvailableFunds() const {
    return availableFunds_;
}

Card* Account::getLinkedCard() const {
    return accountCard_;
}

Bank* Account::getBank() const {
    return bank_;
}

void Account::credit(double amount) {
    if (amount > 0) {
        availableFunds_ += amount;
    }
}

bool Account::debit(double amount) {
    if (amount <= 0) {
        return false;
    }
    if (amount > availableFunds_) {
        return false;
    }
    availableFunds_ -= amount;
    return true;
}

void Account::recordTransaction(Transaction* accountTransaction) {
    if (accountTransaction != NULL) {
        transactionHistory_.push_back(accountTransaction);
    }
}

bool Account::checkPassword(int hashedPassword) const {
    return passwordHash_ == hashedPassword;
}

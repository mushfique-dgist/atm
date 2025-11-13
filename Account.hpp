#ifndef ACCOUNT_HPP
#define ACCOUNT_HPP

#include <string>
#include <vector>

class Bank;
class Card;
class Transaction;

class Account {
public:
    Account(Bank* owningBank,
            const std::string& ownerName,
            const std::string& accountNumber,
            double initialFunds,
            Card* linkedCard,
            int passwordHash);

    const std::string& getAccountNumber() const;
    const std::string& getOwnerName() const;
    double getAvailableFunds() const;
    Card* getLinkedCard() const;
    Bank* getBank() const;

    void credit(double amount);
    bool debit(double amount);
    void recordTransaction(Transaction* accountTransaction);
    bool checkPassword(int hashedPassword) const;

private:
    Bank* bank_;
    std::string ownerName_;
    std::string accountNumber_;
    double availableFunds_;
    Card* accountCard_;
    int passwordHash_;
    std::vector<Transaction*> transactionHistory_;
};

#endif // ACCOUNT_HPP

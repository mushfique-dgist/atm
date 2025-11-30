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
            long long initialFunds,
            Card* linkedCard,
            const std::string& password);

    const std::string& getAccountNumber() const;
    const std::string& getOwnerName() const;
    long long getBalance() const;
    Card* getLinkedCard() const;
    Bank* getBank() const;
    const std::string& getBankName() const;

    void deposit(long long amount);
    bool withdraw(long long amount);
    void recordTransaction(Transaction* accountTransaction);
    bool checkPassword(const std::string& password) const;


private:
    Bank* bank_;
    std::string ownerName_;
    std::string accountNumber_;
    long long balance_;
    Card* accountCard_;
    std::string password_;
    std::vector<Transaction*> transactionHistory_;
};

#endif // ACCOUNT_HPP

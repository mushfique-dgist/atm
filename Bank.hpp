#ifndef BANK_HPP
#define BANK_HPP

#include <string>
#include <vector>
#include <functional>

class Account;
class Card;
class Transaction;

class Bank {
public:
    Bank(const std::string& bankName,
         const std::string& bankId,
         std::vector<Bank*>* allBanks,
         std::vector<Transaction*>* transactions);

    ~Bank();

    const std::string& getBankName() const;
    const std::string& getBankID() const;
    const std::vector<Account*>& getAccounts() const;
    const std::vector<Card*>& getCards() const;

    void addAccount(Account* account);
    Account* findAccountByAccountNumber(const std::string& accountNumber) const;
    Account* findAccountByCardNumber(const std::string& cardNumber) const;

    void setAdminCard(const std::string& cardNumber, const std::string& password);
    bool verifyUserCredentials(const std::string& cardNumber,
                               const std::string& password,
                               Account*& outAccount) const;
    bool verifyAdminCredentials(const std::string& cardNumber,
                                const std::string& password) const;

    void addTransaction(Transaction* transaction);
    void setAllBanks(std::vector<Bank*>* allBanks);
    std::vector<Bank*>* getAllBanks() const;

    bool deposit(Account* account, long long amount);
    bool withdraw(Account* account, long long amount);
    bool transfer(Account* fromAccount,
                  Account* toAccount,
                  long long amount,
                  long long fee);
    bool isAdminCard(const std::string& cardNumber) const;

private:
    Bank(const Bank&) = delete;
    Bank& operator=(const Bank&) = delete;

    std::string bankName_;
    std::string bankID_;
    std::vector<Account*> accounts_; // Non-owning pointers to accounts registered with this bank.
    std::vector<Card*> cards_;       // Non-owning pointers; cards are owned by their account or adminCard_.
    std::vector<Bank*>* allBanks_;
    std::vector<Transaction*>* transactions_;
    Card* adminCard_;
    std::string adminPassword_;
};

#endif // BANK_HPP

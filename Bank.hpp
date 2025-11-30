#ifndef BANK_HPP
#define BANK_HPP

#include <string>
#include <vector>
#include <map>
#include <functional>

class Account;
class Card;
class Transaction;

class Bank {
public:
    Bank(const std::string& bankName,
         const std::string& bankId,
         std::map<std::string, Bank*>* allBanks,
         std::vector<Transaction*>* transactions);

    ~Bank();

    const std::string& getBankName() const;
    const std::string& getBankID() const;
    std::vector<Account*> getAccounts() const;
    std::vector<Card*> getCards() const;

    void addAccount(Account* account);
    Account* findAccountByAccountNumber(const std::string& accountNumber) const;
    Account* findAccountByCardNumber(const std::string& cardNumber) const;
    Account* findAccountInOtherBanks(const std::string& accountNumber) const;

    void setAdminCard(const std::string& cardNumber, const std::string& password);
    bool verifyUserCredentials(const std::string& cardNumber,
                               const std::string& password,
                               Account*& outAccount) const;
    bool verifyAdminCredentials(const std::string& cardNumber,
                                const std::string& password) const;

    void addTransaction(Transaction* transaction);
    void setAllBanks(std::map<std::string, Bank*>* allBanks);
    std::map<std::string, Bank*>* getAllBanks() const;

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
    std::map<std::string, Account*> accountsByNumber_; // Key: account number
    std::map<std::string, Account*> accountsByCard_;    // Key: card number
    std::map<std::string, Card*> cardsByNumber_;        // Key: card number
    std::map<std::string, Bank*>* allBanks_;
    std::vector<Transaction*>* transactions_;
    Card* adminCard_;
    std::size_t adminPassword_;
    static std::hash<std::string> passwordhasher;
};

#endif // BANK_HPP

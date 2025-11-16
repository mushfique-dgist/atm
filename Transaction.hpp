#ifndef TRANSACTION_HPP
#define TRANSACTION_HPP

#include <iostream>
#include <string>

// Base class that records common transaction information.
class Transaction {
public:
    Transaction(const std::string& atmSerial,
                const std::string& cardNumber,
                const std::string& sourceBankName,
                const std::string& sourceAccountNumber,
                long long amount,
                long long fee,
                const std::string& note);

    virtual ~Transaction() = default;

    long long getId() const;
    const std::string& getAtmSerial() const;
    const std::string& getCardNumber() const;
    const std::string& getSourceBankName() const;
    const std::string& getSourceAccountNumber() const;
    long long getAmount() const;
    long long getFee() const;
    const std::string& getNote() const;

    // Returns a concise string describing the transaction type (e.g., "Deposit").
    virtual std::string getTypeName() const = 0;

    // Writes the transaction summary to the given stream.
    // Derived classes may append more information but should call this first.
    virtual void logToStream(std::ostream& out) const;

protected:
    long long id_;
    std::string atmSerial_;
    std::string cardNumber_;
    std::string sourceBankName_;
    std::string sourceAccountNumber_;
    long long amount_;
    long long fee_;
    std::string note_;

private:
    static long long nextId_;
};

class DepositTransaction : public Transaction {
public:
    DepositTransaction(const std::string& atmSerial,
                       const std::string& cardNumber,
                       const std::string& sourceBankName,
                       const std::string& sourceAccountNumber,
                       long long amount,
                       long long fee,
                       const std::string& note);

    std::string getTypeName() const override;
};

class WithdrawalTransaction : public Transaction {
public:
    WithdrawalTransaction(const std::string& atmSerial,
                          const std::string& cardNumber,
                          const std::string& sourceBankName,
                          const std::string& sourceAccountNumber,
                          long long amount,
                          long long fee,
                          const std::string& note);

    std::string getTypeName() const override;
};

class AccountTransferTransaction : public Transaction {
public:
    AccountTransferTransaction(const std::string& atmSerial,
                               const std::string& cardNumber,
                               const std::string& sourceBankName,
                               const std::string& sourceAccountNumber,
                               const std::string& targetBankName,
                               const std::string& targetAccountNumber,
                               long long amount,
                               long long fee,
                               const std::string& note);

    const std::string& getTargetBankName() const;
    const std::string& getTargetAccountNumber() const;

    std::string getTypeName() const override;
    void logToStream(std::ostream& out) const override;

private:
    std::string targetBankName_;
    std::string targetAccountNumber_;
};

class CashTransferTransaction : public Transaction {
public:
    CashTransferTransaction(const std::string& atmSerial,
                            const std::string& cardNumber,
                            const std::string& targetBankName,
                            const std::string& targetAccountNumber,
                            long long amount,
                            long long fee,
                            const std::string& note);

    const std::string& getTargetBankName() const;
    const std::string& getTargetAccountNumber() const;

    std::string getTypeName() const override;
    void logToStream(std::ostream& out) const override;

private:
    std::string targetBankName_;
    std::string targetAccountNumber_;
};

#endif // TRANSACTION_HPP

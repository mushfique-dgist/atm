#include "Transaction.hpp"

long long Transaction::nextId_ = 1;

Transaction::Transaction(const std::string& atmSerial,
                         const std::string& cardNumber,
                         const std::string& sourceBankName,
                         const std::string& sourceAccountNumber,
                         long long amount,
                         long long fee,
                         const std::string& note)
    : id_(nextId_++),
      atmSerial_(atmSerial),
      cardNumber_(cardNumber),
      sourceBankName_(sourceBankName),
      sourceAccountNumber_(sourceAccountNumber),
      amount_(amount),
      fee_(fee),
      note_(note) {
}

long long Transaction::getId() const {
    return id_;
}

const std::string& Transaction::getAtmSerial() const {
    return atmSerial_;
}

const std::string& Transaction::getCardNumber() const {
    return cardNumber_;
}

const std::string& Transaction::getSourceBankName() const {
    return sourceBankName_;
}

const std::string& Transaction::getSourceAccountNumber() const {
    return sourceAccountNumber_;
}

long long Transaction::getAmount() const {
    return amount_;
}

long long Transaction::getFee() const {
    return fee_;
}

const std::string& Transaction::getNote() const {
    return note_;
}

void Transaction::logToStream(std::ostream& out) const {
    out << "ID=" << id_
        << " ATM=" << atmSerial_
        << " Card=" << cardNumber_
        << " Bank=" << sourceBankName_
        << " Account=" << sourceAccountNumber_
        << " Type=" << getTypeName()
        << " Amount=" << amount_
        << " Fee=" << fee_;
    if (!note_.empty()) {
        out << " Note=" << note_;
    }
}

DepositTransaction::DepositTransaction(const std::string& atmSerial,
                                       const std::string& cardNumber,
                                       const std::string& sourceBankName,
                                       const std::string& sourceAccountNumber,
                                       long long amount,
                                       long long fee,
                                       const std::string& note)
    : Transaction(atmSerial,
                  cardNumber,
                  sourceBankName,
                  sourceAccountNumber,
                  amount,
                  fee,
                  note) {
}

std::string DepositTransaction::getTypeName() const {
    return "Deposit";
}

WithdrawalTransaction::WithdrawalTransaction(const std::string& atmSerial,
                                             const std::string& cardNumber,
                                             const std::string& sourceBankName,
                                             const std::string& sourceAccountNumber,
                                             long long amount,
                                             long long fee,
                                             const std::string& note)
    : Transaction(atmSerial,
                  cardNumber,
                  sourceBankName,
                  sourceAccountNumber,
                  amount,
                  fee,
                  note) {
}

std::string WithdrawalTransaction::getTypeName() const {
    return "Withdrawal";
}

AccountTransferTransaction::AccountTransferTransaction(const std::string& atmSerial,
                                                       const std::string& cardNumber,
                                                       const std::string& sourceBankName,
                                                       const std::string& sourceAccountNumber,
                                                       const std::string& targetBankName,
                                                       const std::string& targetAccountNumber,
                                                       long long amount,
                                                       long long fee,
                                                       const std::string& note)
    : Transaction(atmSerial,
                  cardNumber,
                  sourceBankName,
                  sourceAccountNumber,
                  amount,
                  fee,
                  note),
      targetBankName_(targetBankName),
      targetAccountNumber_(targetAccountNumber) {
}

const std::string& AccountTransferTransaction::getTargetBankName() const {
    return targetBankName_;
}

const std::string& AccountTransferTransaction::getTargetAccountNumber() const {
    return targetAccountNumber_;
}

std::string AccountTransferTransaction::getTypeName() const {
    return "AccountTransfer";
}

void AccountTransferTransaction::logToStream(std::ostream& out) const {
    Transaction::logToStream(out);
    out << " TargetBank=" << targetBankName_
        << " TargetAccount=" << targetAccountNumber_;
}

CashTransferTransaction::CashTransferTransaction(const std::string& atmSerial,
                                                 const std::string& cardNumber,
                                                 const std::string& targetBankName,
                                                 const std::string& targetAccountNumber,
                                                 long long amount,
                                                 long long fee,
                                                 const std::string& note)
    : Transaction(atmSerial,
                  cardNumber,
                  "",
                  "",
                  amount,
                  fee,
                  note),
      targetBankName_(targetBankName),
      targetAccountNumber_(targetAccountNumber) {
}

const std::string& CashTransferTransaction::getTargetBankName() const {
    return targetBankName_;
}

const std::string& CashTransferTransaction::getTargetAccountNumber() const {
    return targetAccountNumber_;
}

std::string CashTransferTransaction::getTypeName() const {
    return "CashTransfer";
}

void CashTransferTransaction::logToStream(std::ostream& out) const {
    Transaction::logToStream(out);
    out << " TargetBank=" << targetBankName_
        << " TargetAccount=" << targetAccountNumber_;
}

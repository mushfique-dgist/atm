#pragma once
#include <string>
#include "types.hpp"

class Account {
private:      
    Bank BankName;  
    std::string userName;
    std::string accountNumber;
    std::double availableFunds;
    Card accounrCard;
    std::int password;    
    vector<Transaction*> transactionHistory;

public:
    Account();

    Card(const CardNumber& num,
         const BankName& bank,
         const std::string& pass,
         CardRole role = CardRole::User);

    string getAccountNumber();
    double getAvailableFunds()
    void credit(double amount);
    bool debit(double amount);
    void recordTransaction(Transaction* accountTransaction);
    bool checkPassword(string password);
};



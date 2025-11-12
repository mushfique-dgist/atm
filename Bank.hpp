#include <vector>
#include <string>

class Account;
class Card;
class Transaction;

class Bank {
private:
    std::string adminCardNumber{"20051206"};
    std::string adminCardPassword{"ZhannurIsTheBest"};

    BankID bankID;            
    BankName bankName;         
    Accounts accounts;         
    Cards cards;                   
    BanksList& allBanks;           
    TransactionsList& transactionList; }
    




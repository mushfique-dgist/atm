#pragma once
#include <string>
#include <vector>
#include "Account.hpp"
#include "Card.hpp"
#include "Transaction.hpp"

//variables for Card class
using CardNumber = std::string;
using BankName   = std::string;//Also used in

enum class CardRole { User, Admin };

//variables for Bank Clas
    using BankID = std::string ;
    using BankName = std::string;
    using Accounts = std::vector<Account*> ;
    using Cards = std::vector<Card*>;
    using transactionList = vector<Transaction*>

//BankSystem
    using BanksList = vector<Bank*>
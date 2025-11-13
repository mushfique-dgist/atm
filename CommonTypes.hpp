#ifndef COMMON_TYPES_HPP
#define COMMON_TYPES_HPP

#include <string>
#include <vector>

class Account;
class Bank;
class Card;
class Transaction;

using CardNumber = std::string;
using BankName = std::string;

enum class CardRole { User, Admin };

using BankID = std::string;
using Accounts = std::vector<Account*>;
using Cards = std::vector<Card*>;
using TransactionList = std::vector<Transaction*>;
using BanksList = std::vector<Bank*>;

#endif // COMMON_TYPES_HPP

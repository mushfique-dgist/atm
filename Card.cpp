#include "card.hpp" 

Card::Card(const CardNumber& num,
           const BankName& bank,
           const std::string& pass,
           CardRole role)
    : number_(num), bank_(bank), password_(pass), role_(role) {}

const CardNumber& Card::getNumber() const { return number_; }
const BankName& Card::getBank() const { return bank_; }
CardRole Card::getRole() const { return role_; }

bool Card::verifyPassword(const std::string& input) const {
    return input == password_;
}

bool Card::isAdmin() const {
    return role_ == CardRole::Admin;
}
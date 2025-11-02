#include "Card.hpp"

Card(const CardNumber& card_num,
     const BankName& bank,
     CardRole role = CardRole::User):
     cardNumber(card_num), BankName(card_bank), CardRole(role){}

const CardNumber& getNumber() const{
    return cardNumber;
}

const BankName& getBank() const{
    return bankName;
}

CardRole getRole() const{
    return role}

bool isAdmin() const{
    return role == CardRole::Admin;
};
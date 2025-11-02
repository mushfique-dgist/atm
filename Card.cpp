#include "Card.hpp"

Card::Card(const CardNumber& card_num,
           const BankName& card_bank,
           CardRole role_value)
    : cardNumber(card_num),
      bankName(card_bank),
      role(role_value) {}

const CardNumber& Card::getNumber() const {
    return cardNumber;
}

const BankName& Card::getBank() const {
    return bankName;
}

CardRole Card::getRole() const {
    return role;
}

bool Card::isAdmin() const {
    return role == CardRole::Admin;
}

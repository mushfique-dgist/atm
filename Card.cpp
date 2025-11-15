#include "Card.hpp"

Card::Card(const std::string& card_num,
           const std::string& card_bank,
           CardRole role_value)
    : cardNumber(card_num),
      bankName(card_bank),
      role(role_value) {}

const std::string& Card::getNumber() const {
    return cardNumber;
}

const std::string& Card::getBank() const {
    return bankName;
}

CardRole Card::getRole() const {
    return role;
}

bool Card::isAdmin() const {
    return role == CardRole::Admin;
}

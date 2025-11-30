#ifndef CARD_HPP
#define CARD_HPP

#include <string>

enum class CardRole { User, Admin };

class Card {
private:
    std::string cardNumber;
    std::string bankName;
    CardRole role;

public:
    Card(const std::string& card_num,
         const std::string& card_bank,
         CardRole role_value = CardRole::User);

    const std::string& getNumber() const;
    const std::string& getBank() const;
    CardRole getRole() const;
    bool isAdmin() const;
};

#endif // CARD_HPP

#ifndef CARD_HPP
#define CARD_HPP

#include <string>

// Card represents a simple credential token for ATM access.
// It holds the card number, the bank name, and a role (user/admin).
// The PIN/password is stored with the owning Account/Bank, not in this class.
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

    const std::string& getNumber() const { return cardNumber; }
    const std::string& getBank() const { return bankName; }
    CardRole getRole() const;
    bool isAdmin() const;
};

#endif // CARD_HPP

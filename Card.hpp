#include <string>
#include "types.hpp"

class Card {
private:
    CardNumber number;      
    BankName bank_;           
    std::string password_;    
    CardRole role_;           

public:
    Card(const CardNumber& num,
         const BankName& bank,
         const std::string& pass,
         CardRole role = CardRole::User);

    // Методы
    const CardNumber& getNumber() const;
    const BankName& getBank() const;
    CardRole getRole() const;
    bool verifyPassword(const std::string& input) const;
    bool isAdmin() const;
};
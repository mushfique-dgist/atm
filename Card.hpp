#pragma once
#include <string>
#include "types.hpp"

class Card {
private:
    std::string CardNumber ;      
    Bank BankName;           
    std::int  password;    
           

public:
    Card(const CardNumber& num,
         const BankName& bank,
         const std::string& pass,
         CardRole role = CardRole::User);

    const CardNumber& getNumber() const;
    const BankName& getBank() const;
    CardRole getRole() const;
    bool verifyPassword(const std::string& input) const;
    bool isAdmin() const;
};
#pragma once
#include <string>
#include "CommonTypes.hpp"

class Card {
private:
    CardNumber cardNumber ;      
    BankName bankName;           
    CardRole role; 

public:
    Card(const CardNumber& card_num,
         const BankName& card_bank,
         CardRole role = CardRole::User);

    const CardNumber& getNumber() const;
    const BankName& getBank() const;
    CardRole getRole() const;
    bool isAdmin() const;
};
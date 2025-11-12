#pragma once
#include <string>
#include "CommonTypes.hpp"

class Card {
private:
    CardNumber cardNumber ;
    BankName bankName;

public:
    Card(const CardNumber& card_num,
         const BankName& card_bank);
    const CardNumber& getNumber() const{return cardNumber;}
    const BankName& getBank() const{return bankName;}
    CardRole getRole() const;
    bool isAdmin() const;
};

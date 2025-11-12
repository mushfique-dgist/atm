#include <vector>
#include <string>
#include "Bank.hpp"
#include "CommonTypes.hpp"

class BankSystem {
    
private:
    BanksList allBanks;

public: 
    ~BankSystem();

    void AddBank(){}
    Bank* findBankByName(const BankName& name);
    BanksList& getBanks() { return banks; }

}




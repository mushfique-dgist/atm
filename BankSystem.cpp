#include "BankSystem.hpp"

BankSystem::~BankSystem() {
    for (Bank* b : allBanks) delete b;
    allBanks.clear();
}

Bank* BankSystem::findBankByName(const BankName& name) {
    for (Bank* b : allBanks) {
        if (b->getName() == name) return b;
    }
    return nullptr;
}

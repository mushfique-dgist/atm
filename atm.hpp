#ifndef ATM_HPP
#define ATM_HPP

#include <ostream>
#include <string>
#include "CommonTypes.hpp"

class Account;
class AdminCard;
class Bank;
class Card;

enum ATMMode {
    ATMMode_Idle,
    ATMMode_Customer,
    ATMMode_Admin
};

enum ATMBankAccess {
    ATMBankAccess_SingleBank,
    ATMBankAccess_MultiBank
};

enum ATMLanguage {
    ATMLanguage_English,
    ATMLanguage_Korean
};

enum ATMTransactionKind {
    ATMTransaction_Deposit,
    ATMTransaction_Withdrawal,
    ATMTransaction_AccountTransfer,
    ATMTransaction_CashTransfer
};

const int CASH_TYPE_COUNT = 4;
const int MAX_SESSION_EVENTS = 50;
const int MAX_BANK_SLOTS = 10;
const int MAX_INSERT_ITEMS = 50;

struct ATMFees {
    long long depositPrimary;
    long long depositNonPrimary;
    long long withdrawalPrimary;
    long long withdrawalNonPrimary;
    long long transferPrimaryToPrimary;
    long long transferPrimaryToOther;
    long long transferNonPrimaryToNonPrimary;
    long long cashTransferAny;

    static ATMFees CreateDefault();
};

struct CashDrawer {
    int noteCounts[CASH_TYPE_COUNT];

    CashDrawer();

    long long TotalValue() const;
    int ItemCount() const;
    void Add(const CashDrawer& other);
    bool HasEnoughBills(const CashDrawer& requested) const;
    void Remove(const CashDrawer& requested);
};

struct SessionEvent {
    ATMTransactionKind transactionType;
    long long amount;
    long long feeCharged;
    std::string sourceAccount;
    std::string targetAccount;
    CashDrawer cashChange;
    std::string note;

    SessionEvent();
};

struct SessionState {
    ATMMode mode;
    const Card* card;
    const AdminCard* adminCard;
    Account* primaryAccount;
    bool isPrimaryBankCard;
    SessionEvent records[MAX_SESSION_EVENTS];
    int recordCount;
    int withdrawalCount;

    SessionState();
};

class ATM {
public:
    ATM(const std::string& serialNumber,
        Bank* primaryBank,
        ATMBankAccess accessMode,
        bool bilingual);

    const std::string& GetSerialNumber() const;
    Bank* GetPrimaryBank() const;
    void AddAcceptedBank(Bank* bank);
    bool SupportsBank(const Bank* bank) const;

    ATMBankAccess GetBankAccessMode() const;
    bool IsBilingual() const;
    void SetLanguage(ATMLanguage language);
    ATMLanguage GetActiveLanguage() const;

    const ATMFees& GetFees() const;
    void SetFees(const ATMFees& fees);

    const CashDrawer& GetCashInventory() const;
    void LoadCash(const CashDrawer& cash);
    bool TryGiveCash(const CashDrawer& cash);

    void StartCustomerSession(const Card* card, Account* account, bool primaryBankCard);
    void StartAdminSession(const AdminCard* card);
    void EndSession();
    bool HasActiveSession() const;
    ATMMode GetActiveMode() const;
    const SessionState& GetSessionState() const;

    void RecordEvent(const SessionEvent& event);
    void PrintReceipt(std::ostream& out) const;

    void RequestDeposit(const CashDrawer& cash, long long checkAmount);
    void RequestWithdrawal(long long amount);
    void RequestAccountTransfer(Account* destination, long long amount);
    void RequestCashTransfer(long long amount);
    void CheckInsertedCard();

private:
    std::string serialNumber_;
    Bank* primaryBank_;
    Bank* acceptedBanks_[MAX_BANK_SLOTS];
    int acceptedBankCount_;
    ATMBankAccess accessMode_;
    bool bilingual_;
    ATMLanguage language_;
    ATMFees fees_;
    CashDrawer cashInventory_;
    SessionState sessionInfo_;
    bool sessionActive_;

    void ClearSession();
    
    bool CheckSessionActive(ATMMode expectedMode) const;
};

#endif // ATM_HPP



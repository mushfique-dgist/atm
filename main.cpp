//testing
#include <iostream>
#include <fstream>
#include <string>
#include <map>
using namespace std;


enum class transaction_type {
	deposit, withdrawal, remittance, paying,
};

// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.

class Card;
class Account;
class User;
class AdminCard;
class ATM;
class Transaction;
class Bank;
class KoreaBank;



class Card {
private:
	int Card_number;
	Account *account;
	Transaction** log;
	int log_counter;
	int password;

public:
	Card();
	~Card();
};


class Account {
private:
	bool Authorized = false;
	User* user;
	Transaction** log;
	Bank* bank;
	int hash_pass;
	hash<string> hasher;
	Card** cards;
	int log_count;
public:
	Account();
	~Account();
	void set_password(string);
	void authorize(string);

};


void Account::set_password(string password) {
	if (Authorized) {
		hash_pass = hasher(password);
		Authorized = false;
	}

}

void Account:: authorize(string password) {
	int given = hasher(password);
	if (hash_pass == given) {
		Authorized = true;
	}
}


class AdminCard {
	private:
public:
	AdminCard();
	~AdminCard();
};

class Transaction {
public:
	Transaction(transaction_type, int, Account, Account);
	~Transaction();
	void authorize();
	virtual string tostring() const; // override the string type

private:
	transaction_type transactoin;
	int amount, fee;
	Account subject, object;

};

class Language {
public:
	string welcome;
	string second;

};

class ATM {
private:
	const static Language* Korean;
	const static Language* English;
	Language* language;


	int ATM_ID;
	AdminCard admincard;

	Bank** banks;
	int banks_count;

	Transaction** log;
	int log_counter;
public:


};



class Bank {
private:
	static int banknumber;
	Account account;
	int id;
	Account **accounts;
	ATM **atms;

public:


};

class KoreaBank {

};


class User {
private:
	int userid;
	string username;
	Account account;
public:
	string getname;
	int* get_number;

};




class Interface;





class Single_Bank: public Bank {
	public:
};

class Multibank: public Bank {

};

class one_bank_user: public User {

};

class multi_bank_user: public User {

};








using namespace std;

int main(){
    string filename = "sample_initial_condition.txt";
	ifstream fin(filename);
    // If file fails to open, display an error message and exit
	if (!fin) {
		cerr << "Error opening file: " << filename << endl;
		return 1;
	}

    // Read initial conditions
	int bankCount, accountCount, atmCount;
	fin >> bankCount >> accountCount >> atmCount;

    // cout << bankCount << " " << accountCount << " " << atmCount << endl;

    // Read bank information
	string* bankNames = new string[bankCount];              // Dynamic array to hold bank names
	for (int i = 0; i < bankCount; ++i) {
		fin >> bankNames[i];
	}

    // Read Account information
	string* accountBanks = new string[accountCount];        // Dynamic array to hold account banks
	string* accountUsers = new string[accountCount];        // Dynamic array to hold account users
	string* accountNumbers = new string[accountCount];      // Dynamic array to hold account numbers
	int* accountBalances = new int[accountCount];           // Dynamic array to hold account balances
    string* accountCardNum = new string[accountCount];      // Dynamic array to hold account card numbers
	int* accountPasswords = new int[accountCount];          // Dynamic array to hold account passwords
	for (int i = 0; i < accountCount; ++i) {
		fin >> accountBanks[i] >> accountUsers[i] >> accountNumbers[i] >> accountBalances[i] >> accountCardNum[i] >> accountPasswords[i];
	}

    // Read ATM information
	string* atmBanks = new string[atmCount];                // Dynamic array to hold ATM banks
	int* atmSerials = new int[atmCount];                    // Dynamic array to hold ATM serial numbers
	bool* atmSingleBanks = new bool[atmCount];              // Dynamic array to hold ATM single bank status
	bool* atmUnilinguals = new bool[atmCount];              // Dynamic array to hold ATM unilingual status
	int** atmInitialCash = new int* [atmCount];             // Dynamic array to hold ATM initial cash
	for (int i = 0; i < atmCount; ++i) {
        // Dynamic allocation for atmInitialCash
		atmInitialCash[i] = new int[4]; // holding initial cash: [50000won, 10000won, 5000won, 1000won]

        // Store ATM Serial numbers
        fin >> atmBanks[i] >> atmSerials[i];

        // Store bool status of Single/Multi & Unilingual/Bilingual
		string singleBankStr, unilingualStr;
		fin >> singleBankStr >> unilingualStr;
		atmSingleBanks[i] = (singleBankStr == "Single");
		atmUnilinguals[i] = (unilingualStr == "Unilingual");

        // Read initial cash for the ATM
		for (int j = 0; j < 4; ++j) {
			fin >> atmInitialCash[i][j];
		}
	}

    // Print loaded informations
    // Print bank informations
    cout << "Banks:" << endl;
    for (int i = 0; i < bankCount; ++i) {
        cout << "  " << bankNames[i] << endl;
    }

    // Print account information
    cout << "accounts:" << endl;
    for (int i = 0; i < accountCount; ++i) {
        cout << "  Bank: " << accountBanks[i]
             << ", User: " << accountUsers[i]
             << ", Number: " << accountNumbers[i]
             << ", Balance: " << accountBalances[i]
             << ", Card Number: " << accountCardNum[i]
             << ", Password: " << accountPasswords[i] << endl;
    }

    // Print ATM information
    cout << "ATMs:" << endl;
    for (int i = 0; i < atmCount; ++i) {
        cout << "  Bank: " << atmBanks[i]
             << ", Serial: " << atmSerials[i]
             << ", Single Bank: " << (atmSingleBanks[i] ? "Yes" : "No")
             << ", Unilingual: " << (atmUnilinguals[i] ? "Yes" : "No")
             << ", Initial Cash: [";
        for (int j = 0; j < 4; ++j) {
            cout << atmInitialCash[i][j];
            if (j < 3) cout << ", ";
        }
        cout << "]" << endl;
    }

    // Clean up dynamic memory
	delete[] bankNames;
	delete[] accountBanks;
	delete[] accountUsers;
	delete[] accountNumbers;
	delete[] accountBalances;
	delete[] accountPasswords;
	delete[] atmBanks;
	delete[] atmSerials;
	delete[] atmSingleBanks;
	delete[] atmUnilinguals;
	for (int i = 0; i < atmCount; ++i) {
		delete[] atmInitialCash[i];
	}
	delete[] atmInitialCash;

	return 0;
}
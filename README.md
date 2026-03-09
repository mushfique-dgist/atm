<div align="center">

# ATM Simulator

**A full-featured, multi-bank ATM console simulation in C++14**

[![Language](https://img.shields.io/badge/language-C%2B%2B14-00599C?style=flat-square&logo=cplusplus)](https://en.cppreference.com/w/cpp/14)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey?style=flat-square)](https://github.com/mushfique-dgist/atm)
[![Lines of Code](https://img.shields.io/badge/lines%20of%20code-2%2C652-informational?style=flat-square)](https://github.com/mushfique-dgist/atm)
[![License](https://img.shields.io/badge/license-Academic-blueviolet?style=flat-square)](https://github.com/mushfique-dgist/atm)

---

*DGIST CSE201 Object-Oriented Programming — Term Project, Fall 2025*

</div>

---

## Overview

This project is a console-based ATM simulator that models a realistic multi-bank cash machine system. It supports four transaction types, bilingual menus (English / Korean), configurable per-ATM cash drawers, an admin audit trail, and a full session lifecycle — all built from scratch in standard C++14 with no external libraries.

The architecture follows clean OOP principles: abstract base classes for polymorphic transactions, encapsulated session state, value-semantic structs for cash and fee data, and strict boundary validation throughout.

---

## Table of Contents

- [ATM Simulator](#atm-simulator)
  - [Overview](#overview)
  - [Table of Contents](#table-of-contents)
  - [Features](#features)
    - [Core System](#core-system)
    - [Transactions](#transactions)
    - [Admin](#admin)
  - [Architecture](#architecture)
  - [Project Structure](#project-structure)
  - [Getting Started](#getting-started)
    - [Prerequisites](#prerequisites)
    - [Build](#build)
    - [Run](#run)
  - [Configuration Format](#configuration-format)
  - [Transactions \& Fees](#transactions--fees)
    - [Fee Schedule](#fee-schedule)
    - [Withdrawal: Fewest-Bills Algorithm](#withdrawal-fewest-bills-algorithm)
    - [Cash Transfer Flow](#cash-transfer-flow)
  - [Usage Walkthrough](#usage-walkthrough)
    - [Startup](#startup)
    - [Customer Session](#customer-session)
    - [Admin Session](#admin-session)
    - [Snapshot (available anywhere with `/`)](#snapshot-available-anywhere-with-)
  - [Design Decisions](#design-decisions)

---

## Features

### Core System

| Feature | Details |
|---|---|
| **Multi-bank support** | ATMs operate in Single-bank or Multi-bank mode; Single-bank ATMs reject cards from other banks |
| **Bilingual UI** | Bilingual ATMs let users pick English or Korean at session start; Unilingual ATMs lock to English |
| **Session management** | Customer and Admin sessions are fully isolated; only one active session per ATM at a time |
| **Authentication** | Card number + PIN with a 3-attempt lockout; Admin card + password with the same lockout |
| **System snapshot** | `/` command available at any menu — shows live cash inventory and account balances for the entire system |

### Transactions

| Type | Limit | Notes |
|---|---|---|
| **Deposit** | 50 items (cash bills + checks) | Checks must be ≥ 100,000 won each; fee paid separately in cash |
| **Withdrawal** | ≤ 3 per session, ≤ 500,000 won, multiples of 1,000 | Fewest-bills dispensing algorithm |
| **Account Transfer** | Unlimited per session | Fee tier depends on source/destination bank relationship |
| **Cash Transfer** | 50 bills | Inserts physical cash; fee deducted from total; remainder deposited to destination |

### Admin

- Per-ATM transaction log with full metadata (ID, card, type, amount, fee, accounts)
- Export transaction history to a file from the admin menu
- Session counters (total, customer, admin) displayed per ATM
- Admin cards configured at startup, one per bank

---

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                        main.cpp                         │
│   SystemState · I/O helpers · Console loop · Startup    │
└────────────────────────┬────────────────────────────────┘
                         │ owns / orchestrates
         ┌───────────────┼───────────────┐
         ▼               ▼               ▼
      Bank            Account           ATM
   ─────────        ─────────        ─────────
   bankName_        balance_         serial_
   accounts_        password_        cashDrawer_
   adminCard_       txHistory_       sessionState_
   allBanks_*  ◄──  bank_*      ◄──  primaryBank_*
                    card_*
                         │
                         ▼
                       Card
                   ──────────
                   cardNumber
                   bankName
                   role (User|Admin)
                         │
                         ▼ (created per operation)
                   Transaction  (abstract)
                   ────────────────────────────────
                   id_  atmSerial_  cardNumber_
                   amount_  fee_  note_
                         │
              ┌──────────┼───────────────┐
              ▼          ▼               ▼               ▼
         Deposit    Withdrawal   AccountTransfer   CashTransfer
         Transaction Transaction  Transaction      Transaction
```

**Key design patterns:**

- **Inheritance + Polymorphism** — `Transaction` is an abstract base with virtual `getTypeName()` and `logToStream()`; four concrete subclasses handle type-specific logging
- **State pattern** — `ATM` holds a `SessionState` struct that captures the active card, account, mode (`Idle / Customer / Admin`), and per-session event log; everything resets cleanly on `EndSession()`
- **Value structs as DTOs** — `CashDrawer`, `ATMFees`, and `SessionEvent` are plain structs passed by value/reference, keeping data flow explicit
- **Auto-incrementing IDs** — `Transaction::nextId_` is a static counter; every transaction gets a unique monotonic ID regardless of which ATM created it

---

## Project Structure

```
.
├── main.cpp            # Entry point, console UI, session routing, I/O helpers
├── Atm.hpp / Atm.cpp   # ATM class: session lifecycle, cash management, all transaction logic
├── Bank.hpp / Bank.cpp # Bank class: account registry, credential validation, fund transfers
├── Account.hpp / Account.cpp  # Account class: balance, password, transaction history
├── Card.hpp / Card.cpp        # Card class: card number, bank, role (User or Admin)
├── Transaction.hpp / Transaction.cpp  # Abstract Transaction + 4 concrete subclasses
├── initial_condition.txt       # Sample startup data (banks, accounts, ATMs, cash)
├── Guidelines.md               # Coding conventions used in this project
└── uml_docs/                   # UML diagrams generated during design phase
```

---

## Getting Started

### Prerequisites

- A C++14-compliant compiler: `g++ 7+`, `clang++ 4+`, or MSVC 19.14+
- No external libraries or build system required

### Build

```bash
g++ -std=c++14 main.cpp Atm.cpp Bank.cpp Account.cpp Card.cpp Transaction.cpp -o atm
```

On Windows with MSVC:
```powershell
cl /std:c++14 main.cpp Atm.cpp Bank.cpp Account.cpp Card.cpp Transaction.cpp /Fe:atm.exe
```

### Run

```bash
./atm
```

The program reads `initial_condition.txt` from the current directory, then prompts you to set an admin card and PIN for each bank before entering the main menu.

---

## Configuration Format

The `initial_condition.txt` file drives the entire startup state. Its format is:

```
<numBanks> <numAccounts> <numATMs>

<bank1Name> <bank2Name> ... <bankNName>

<bankName> <ownerName> <accountNumber> <initialBalance> <cardNumber> <password>
...  (one line per account)

<primaryBankName> <serialNumber> <AccessMode> <LanguageMode> <50K> <10K> <5K> <1K>
...  (one line per ATM)
```

| Field | Values |
|---|---|
| `AccessMode` | `Single` or `Multi` |
| `LanguageMode` | `Unilingual` or `Bilingual` |
| Cash columns | Count of 50,000 / 10,000 / 5,000 / 1,000 won bills loaded at startup |

**Example:**

```
3 5 3

Kakao Daegu Hana

Kakao Chris 100-100-111111 500000 1111-1111-1111 1234
Daegu Chris 200-200-222222 300000 2222-2222-2222 2345
Hana  Andor 300-300-333333 100000 3333-3333-3333 3456
Kakao Mira  100-100-444444 700000 4444-4444-4444 4567
Daegu Mira  200-200-555555 150000 5555-5555-5555 5678

Kakao 000111 Single Unilingual  2  3  1 20
Daegu 000222 Multi  Bilingual   1  0 10 50
Hana  000333 Multi  Bilingual   0  5  0  5
```

---

## Transactions & Fees

### Fee Schedule

| Transaction | Primary bank card | Non-primary bank card |
|---|---|---|
| Deposit | 0 won | 1,000 won |
| Withdrawal | 1,000 won | 2,000 won |
| Account Transfer (primary → primary) | 1,000 won | — |
| Account Transfer (primary ↔ other) | 2,000 won | — |
| Account Transfer (other → other) | 4,000 won | — |
| Cash Transfer | 2,000 won (fixed) | — |

Fee for deposits and withdrawals is paid **in cash** as a separate input before the transaction is processed. For transfers, the fee is deducted directly from the source account balance.

### Withdrawal: Fewest-Bills Algorithm

The ATM dispenses the requested amount using the minimum number of bills, prioritising higher denominations (50K → 10K → 5K → 1K). If the ATM cannot form the exact amount from its current inventory, the transaction is cancelled with an error.

### Cash Transfer Flow

```
User inserts N bills  →  ATM adds all bills to inventory
                      →  Fee (2,000 won) deducted from inserted total
                      →  Remainder deposited to destination account
```

The fee is retained in the ATM; it is not credited to any account.

---

## Usage Walkthrough

### Startup

```
$ ./atm
[Admin Setup] Enter admin card number for bank 'Kakao': ADMIN-KAKAO
[Admin Setup] Enter admin PIN for bank 'Kakao': ****

--- System Snapshot ---
ATM 000111 | Primary: Kakao | Single | Unilingual
  50,000 x 2 | 10,000 x 3 | 5,000 x 1 | 1,000 x 20
  Total: 155,000 won
...
```

### Customer Session

```
[Main Menu] Select ATM (or 0 to quit): 1
[ATM 000111] Session type — 1: Customer  2: Admin  0: Cancel: 1
Card number: 1111-1111-1111
PIN: ****

--- ATM Menu ---
1. Deposit
2. Withdraw
3. Account Transfer
4. Cash Transfer
5. Print Receipt
/  Snapshot
0. End Session
```

### Admin Session

```
[ATM 000111] Session type — 1: Customer  2: Admin  0: Cancel: 2
Admin card: ADMIN-KAKAO
Admin PIN: ****

--- Admin Menu ---
1. Print all transactions (this ATM)
2. Export transactions to file
/  Snapshot
0. Exit
```

### Snapshot (available anywhere with `/`)

```
--- System Snapshot ---
ATM 000111 | Primary: Kakao | Single | Unilingual
  50,000 x 2  10,000 x 3  5,000 x 1  1,000 x 20  |  Total: 155,000 won

ATM 000222 | Primary: Daegu | Multi | Bilingual
  50,000 x 1  10,000 x 0  5,000 x 10  1,000 x 50  |  Total: 150,000 won

--- Account Balances ---
[Kakao] Chris   100-100-111111  500,000 won
[Daegu] Chris   200-200-222222  300,000 won  [IN USE]
...
```

---

## Design Decisions

**Why plain `struct` for `CashDrawer` and `ATMFees`?**
These are pure data containers with no invariants to protect. Using structs keeps them lightweight, easy to copy/pass, and readable without unnecessary boilerplate.

**Why is `Transaction` abstract?**
Each transaction type has different display/log requirements. Making `logToStream()` and `getTypeName()` pure virtual forces every subclass to define its own format, preventing silent default-output bugs.

**Why are fees validated in cash for deposits and withdrawals?**
The project spec requires the user to physically insert fee cash as a separate step. This mirrors real ATM behaviour where the machine checks the cash tray before committing a transaction.

**Why per-ATM transaction history?**
The admin role is scoped to a single ATM. Keeping each ATM's history separate means an admin can audit only the machine they are logged into, which matches the access-control intent of the spec.

**Why no CMake or Makefile?**
The project has six translation units and zero external dependencies. A single `g++` invocation is the simplest and most portable build method for this scale.

---

<div align="center">

Built for DGIST CSE201 Object-Oriented Programming · Fall 2025

</div>

# DGIST ATM Simulator (CSE201 Fall 2025)

Fully implemented console ATM that satisfies the official `(release) 2025-fall-cse201-term project - v4` requirements and the TA clarifications in `qa_for_project.txt`. All core features (customer + admin flows, fees, language choice, per‑ATM histories, receipts, snapshots) are complete and documented.

## Build & Run

From the project root:
```bash
g++ -std=c++14 main.cpp Atm.cpp Bank.cpp Account.cpp Card.cpp Transaction.cpp -o main.exe
./main.exe
```
The program loads `sample_initial_condition.txt`, prompts for admin cards/PINs (per bank), and then shows the main menu.

## Quick Usage Guide

- **Select ATM & Language**: Pick an ATM index. If bilingual, choose English or Korean; unilingual ATMs auto‑select English.
- **Customer session**: Enter card number, then PIN (3 attempts). Menu: Deposit / Withdraw / Account transfer / Cash transfer / Print receipt / Snapshot / End session.
- **Admin session**: Enter bank’s admin card/PIN. Menu: Print all transactions (this ATM), Export transactions to file, Snapshot, Exit.
- **Slash snapshot (`/`)**: Available at main menu, customer ATM menu, and admin menu; shows all ATMs’ cash (per denomination) and all accounts’ balances.

## Features Implemented

- **Initialization (REQ 1.x)**: Builds Banks/Accounts/Cards/ATMs from the input file; applies single vs multi‑bank, unilingual vs bilingual, fee schedules, and loads initial cash drawers.
- **Language (REQ 8.x)**: Bilingual ATMs prompt for EN/KR; all menus/errors/receipts/admin history reflect the chosen language. Unilingual ATMs force English with a notice.
- **Sessions & Auth (REQ 2.x/3.x)**: Card + PIN with 3‑attempt limit; per‑session withdrawal cap (max 3), per‑transaction withdrawal cap (500,000), session event logging, and session summary/receipt.
- **Deposits (REQ 4.x)**: Per‑denomination cash input, multi‑check input, 50‑item limit (cash + checks), exact fee-in-cash validation, cash added to ATM, checks not added to ATM.
- **Withdrawals (REQ 5.x)**: Fewest-bills dispensing, cash/fee availability checks, fee charged to account, ATM inventory updated, limits enforced.
- **Transfers (REQ 6.x)**: Account transfers with correct primary/non‑primary fees; cash transfers accept cash only, enforce 50‑bill limit, fee paid in cash, deposit remainder to destination; source recorded in history.
- **Admin history (REQ 7.x)**: Per‑ATM transaction log with ID, card, type, amount, fee, from/to, note; export to file; session counters shown per ATM.
- **Snapshots (REQ 10.x)**: `/` shows all ATMs’ cash (with bill counts) and all accounts’ balances.
- **Exception handling (REQ 9.x)**: Clear, localized errors; fatal cases end session, non‑fatal cases cancel the transaction per TA guidance.

## Inputs and Prompts

- **Initial data**: `sample_initial_condition.txt` (banks, accounts, ATMs, initial cash). Grading files follow the same format.
- **Admin cards**: Enter at startup per bank (any string/format is allowed per TA).
- **Cash/fee/check prompts**: Cash by denomination counts; fee cash must match the fee; checks via multi‑entry amounts (>=100,000); 50‑item cap applies to deposits and cash transfers.

## Logs, Docs, and QA

- Internal docs (requirement checklist, task tracker, QA notes, overview, UML, and test transcripts) are bundled in the repo; see the `docs/` notes maintained during development.

## Notes and Known Choices

- Transaction history is **per ATM** (per TA acceptance), includes all required fields, and is exported per ATM.
- Fee cash is inserted separately and not credited to accounts; notes clarify this in both languages.
- Cash transfer capped at 50 bills to mirror the deposit item limit (allowed by TA).
- Unilingual ATMs force English; bilingual ATMs honor the chosen language for menus and history export; summaries/notes use the viewer’s language.

## Troubleshooting

- **Compile errors**: Use `g++ -std=c++14` as shown; ensure all `.cpp` files are included.
- **Permission denied on main.exe**: Close any running `main.exe` before rebuilding.
- **Card not recognized**: Only cards from the init file + runtime admin cards are valid.
- **Fee mismatch**: Deposit requires exact fee in cash; otherwise the transaction is canceled.

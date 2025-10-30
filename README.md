# ATM Term Project

This repository is the starting point for the Fall 2025 CSE201 term project. We are kicking off the work on a console based ATM simulator that must satisfy the full requirement list from the official project brief. Nothing here is final yet—this document captures how we plan to organise the effort and what is currently in the repository.

## Project Intent

- Translate the assignment PDF into a maintainable C++ design that can grow with the team’s feature work.
- Capture early decisions so everyone understands the direction before we dive into detailed implementation.
- Keep the main branch tidy while we experiment with different class layouts and session flows.

## Current Status

- The code compiles and reads the initial condition file to prove the parsing logic.
- Core classes (`ATM`, `Bank`, `Account`, `Card`, `Transaction`, etc.) exist as placeholder with minimal behaviour.
- No user facing features (deposits, withdrawals, transfers, admin menus, multilingual prompts) are implemented yet; all of that is planned in the roadmap below.

## Repository Layout

> The structure below reflects the initial bootstrap. Expect files to move around as we factor the implementation; nothing is locked in.

- `main.cpp` – entry point; currently handles file input and prints what was parsed.
- `Account.cpp`, `ATM.cpp`, `Bank.cpp`, `transaction.cpp`, `user.cpp` – placeholder translation units ready for real logic.
- `sample_initial_condition.txt` – example of the required startup data format (refer to REQ1.11).
- `.gitignore` – keeps editor specific folders such as `.vscode/` out of version control.

We will add or remove files as the design settles, so treat this as a snapshot rather than a finished layout.

## Getting Set Up

1. Install a C++20 capable toolchain (MSVC, clang, or g++).
2. Prepare an input file that matches the assignment format.
3. Build and run:
   ```
   g++ -std=c++20 main.cpp -o atm
   ./atm
   ```
   The program currently just echoes the parsed data—it is a scaffolding check, not a feature demo.

## Implementation Roadmap

We are working through the requirements section by section. Highlights:

- **Initialisation (REQ1.x)** – flesh out ATM cash drawers, fee schedules, and dynamic creation of banks/accounts.
- **Session Lifecycle (REQ2.x)** – add session tracking, per session limits, and end-of-session summaries.
- **Authorisation (REQ3.x)** – implement PIN validation, retry limits, and card lockouts.
- **Transactions (REQ4.x – REQ6.x)** – build deposit, withdrawal, cash transfer, and account transfer flows with a shared transaction log.
- **Admin Features (REQ7.x)** – wire the admin card path and the transaction history export.
- **Language Support (REQ8.x)** – centralise prompts and allow runtime language selection.
- **Exception Handling and Snapshots (REQ9.x, REQ10.x)** – design consistent error messaging and the slash command for graders.

Each bullet above still needs coding and tests; we will open issues or branches as we pick them up.

## Working Agreements

- Use feature branches and keep commits focused so reviews are manageable.
- Document requirement coverage in pull requests with console transcripts or notes.
- Sync regularly before touching shared headers to avoid merge headaches.

## Verification Plans

- Build lightweight console driven tests to rehearse requirement scenarios.
- Capture manual transcripts for complex flows (multi language, admin history dump) and store them in a `docs/tests/` folder once it exists.
- Re-run basic parsing and session smoke tests before merging into `main`.

## Upcoming Focus

- Finalise the class interfaces so we can divide implementation tasks.
- Decide how to persist transaction history across sessions (file output versus in-memory only for the demo).
- Draft the reporting template required for the final submission so screenshots and requirement references are collected along the way.

This README will evolve as we make progress. For now it records our launch point and the plan of attack. Once we begin landing features we will add changelog entries and link to design notes.

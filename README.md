# HinLIBS

HinLIBS is a desktop-based Library Management System built with C++ and Qt.  
It provides separate interfaces for patrons, librarians, and system administrators, supporting common library workflows such as borrowing, returning, and managing items.

This repository contains a cleaned and self-contained version of the project suitable for local builds and demonstration.

---

## Features

### Patron
- Browse available library items
- Borrow and return items
- Place and view holds
- View active loans and account status

### Librarian
- Manage library items
- View and manage borrowed items
- Oversee patron activity

### System Administrator
- Administrative-level access to the system

---

## Tech Stack

- **Language:** C++
- **GUI Framework:** Qt (Qt Widgets)
- **Database:** SQLite
- **Platform:** Linux (tested on Ubuntu)

---

## How to Build and Run

1. Open **Qt Creator**
2. Select **File → Open File or Project**
3. Navigate to the project root and open `hinlibs.pro`
4. Click the **Build** (hammer) icon
5. Click the **Run** (green triangle) icon

The application will launch immediately.  
No additional setup or configuration is required.

### Database Behavior
- The SQLite database is pre-initialized
- Each new build resets the database to its default state

---

## Demo Login Credentials

### Patron Accounts
- nikolai
- mike
- oyin
- abdulrahman
- mohammad

### Librarian Account
- **Username:** lib

### System Administrator Account
- **Username:** admin

(No passwords required.)

---

## Navigation Notes

- **Profile Menu (top-right):**
  - View active loans
  - View active holds
  - Log out

- **Back Button (top-left):**
  - Returns to the previous screen on most pages

The librarian interface includes a navigation menu for managing items and patron activity.

---

## Project Context

This project was originally developed as a **team-based software engineering project**.
This repository represents a personal, cleaned version intended for demonstration and portfolio purposes.


# 🖥️ Operating-Systems-IIT-Delhi

This repository contains coursework for the Operating Systems course at IIT Delhi. It is organized into two assignments, each in its own directory, demonstrating core OS concepts like paging and shell interfaces. All code is written in C and tested on Linux using GCC.

---

## 📁 Repository Structure

```
.
├── Page_Table_Simulator/  
│   ├── HW3.pdf          # Assignment specification for the page table simulator  
│   └── frames.c         # C implementation of the page table simulator  
└── Shell/                
    ├── Assignment_1_6f46970dabe4b291b878ac6c29bffd7d.pdf  # Assignment specification for the shell  
    └── shell.c          # C implementation of a simple Unix-like shell  
```

---

## 📝 Page_Table_Simulator

**Purpose**  
Simulate page table operations for a demand-paging system. Supports frame allocation, address translation, and page replacement policies.

**Files**  
- `HW3.pdf` – Detailed assignment specification.  
- `frames.c` – Source code implementing the simulator:  
  - Parses a sequence of logical addresses.  
  - Simulates mapping to physical frames.  
  - Implements page replacement (e.g., FIFO, LRU).

**Build & Run**  
```bash
cd Page_Table_Simulator
gcc -std=c11 -o frames frames.c
./frames < input_addresses.txt > output_results.txt
```

---

## 📝 Shell

**Purpose**  
Build a simple Unix-like shell supporting:  
- Command parsing & execution  
- I/O redirection (`>`, `<`)  
- Pipelines (`|`)  
- Background execution (`&`)

**Files**  
- `Assignment_1_6f46970dabe4b291b878ac6c29bffd7d.pdf` – Assignment specification.  
- `shell.c` – Source code for the shell implementation.

**Build & Run**  
```bash
cd Shell
gcc -std=c11 -o shell shell.c
./shell
```
> Type commands at the `myshell> ` prompt. Use `exit` to quit.

---

## 🛠️ Dependencies

- **GCC** (version 4.8+ recommended)
- **Linux** environment or equivalent POSIX-compatible shell

---

## 🔍 Testing & Verification

- Input files (for paging) or interactive command sessions (for shell) should follow the specs in the PDF documents.  
- Compare outputs (paging simulation) against expected results provided in class.

---

## 👤 Author

Abhishek Narayan  
B.Tech, IIT Delhi  

---

*Feel free to open an issue or pull request for any clarifications or enhancements.*

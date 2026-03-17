# 🎓 Student Tracker — Build & Run Guide

> Windows GUI application built with C++ and Win32 API.

---

## ⚙️ Prerequisites

- [MinGW / g++](https://www.mingw-w64.org/) installed and added to **PATH**
- **VS Code** or **Git Bash** terminal

---

## 🚀 How to Run

### Step 1 — Compile
```bash
g++ StudentTracker.cpp -o app.exe -mwindows -lcomctl32
```

### Step 2 — Run
```bash
.\app.exe
```

---

## 📝 Notes

- Run both commands inside **VS Code Terminal** or **Git Bash**
- The `-mwindows` flag hides the console window (GUI only)
- The `-lcomctl32` flag links the Windows Common Controls library

---

## 🏷️ Tags

`Windows` &nbsp; `C++` &nbsp; `MinGW` &nbsp; `Win32 API`

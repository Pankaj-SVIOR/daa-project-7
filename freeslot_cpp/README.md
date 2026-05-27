# Teacher Free Slot Finder — C++ Version

A web application that helps teachers find free classroom slots and book rooms.
Converted from Python/Flask to **C++14** using a custom WinSock2 HTTP server and SQLite.

---

## Project Structure

```
freeslot_cpp/
│
├── main.cpp              ← Entry point — all API routes live here
├── build.bat             ← One-click build script
├── freeslot.exe          ← Compiled executable (after build)
├── README.md             ← This file
│
├── models/
│   └── dsa.h             ← Data Structures: Stack, Queue, Linked List, Hash Table
│
├── utils/
│   ├── server.h          ← Simple HTTP server (WinSock2, no install needed)
│   └── database.h        ← SQLite open/close/setup helpers
│
├── sqlite/
│   ├── sqlite3.h         ← SQLite header (bundled — no install needed)
│   ├── sqlite3.c         ← SQLite source (bundled)
│   └── sqlite3.o         ← Compiled SQLite object (after build)
│
├── database/
│   └── free_slot_system.db  ← SQLite database file
│
└── views/                ← HTML pages (same as Python templates)
    ├── index.html
    ├── teacher_dashboard.html
    ├── book_room.html
    ├── free_slots.html
    └── weekly_schedule.html
```

---

## DAA Concepts Used (for Viva)

| Concept | Where Used | File |
|---|---|---|
| **Stack (LIFO)** | Undo last booking | `models/dsa.h` |
| **Queue (FIFO)** | Booking request queue | `models/dsa.h` |
| **Linked List** | Room booking history | `models/dsa.h` |
| **Hash Table** | Fast slot↔time lookup | `models/dsa.h` |
| **Hash Map** | Room ID→name, busy rooms | `main.cpp` |
| **Set** | Free room finder — O(1) busy check | `main.cpp` |
| **Sorting** | Weekly schedule order by day+slot | `main.cpp` |

---

## How to Build and Run

### Step 1 — Build (one command)
Open terminal inside `freeslot_cpp/` folder and run:
```
build.bat
```

### Step 2 — Run
```
freeslot.exe
```

### Step 3 — Open Browser
```
http://localhost:8080
```

---

## API Routes

| Method | Route | What it does |
|---|---|---|
| GET | `/` | Home page |
| GET | `/teacher_dashboard` | Teacher dashboard |
| GET | `/weekly-schedule` | Weekly schedule page |
| GET | `/teachers` | JSON list of all teachers |
| GET | `/free_rooms_today` | Free rooms for every slot today |
| GET | `/teacher_classes_today/<id>` | Today's classes for a teacher |
| GET | `/api/weekly_schedule/<id>` | Full weekly schedule |
| POST | `/book_room` | Book a room |
| POST | `/undo_booking` | Undo last booking (Stack) |
| GET | `/today_bookings` | All approved bookings |
| GET | `/history/<room_id>` | Room booking history (Linked List) |

---

## What Changed from Python Version

| Python | C++ |
|---|---|
| Flask `render_template` | Read HTML file with `ifstream` |
| Jinja2 `{{ teacher_id }}` | JavaScript reads from URL params |
| `python dict` | `std::map` |
| `python set` | `std::set` |
| `python list` | `std::vector` |
| `datetime.today()` | `ctime` / `localtime()` |

---

## Common Errors

**"Server running"  but "Bind failed"** → Port 8080 is in use. Kill the old process:
```
netstat -ano | findstr :8080
taskkill /PID <the_pid> /F
```

**"Cannot open database"** → Make sure `database/free_slot_system.db` exists in the same folder as `freeslot.exe`.

**"views/index.html not found"** → Run `freeslot.exe` from inside the `freeslot_cpp/` folder, not from elsewhere.

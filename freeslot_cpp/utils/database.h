#ifndef DATABASE_H
#define DATABASE_H

#include "../sqlite/sqlite3.h"
#include <string>
#include <iostream>

const std::string DB_PATH = "database/free_slot_system.db";

sqlite3* openDB() {
    sqlite3* db = NULL;
    if (sqlite3_open(DB_PATH.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Cannot open DB: " << sqlite3_errmsg(db) << std::endl;
        return NULL;
    }
    return db;
}

void closeDB(sqlite3* db) { if (db) sqlite3_close(db); }

// Get teacher_id by name (insert if not exists)
static int getTeacher(sqlite3* db, const char* name) {
    if (!name || name[0]=='\0') return 1;
    sqlite3_stmt* st = NULL;
    sqlite3_prepare_v2(db, "INSERT OR IGNORE INTO teachers(name,department,password) VALUES(?,'Faculty','pass123')", -1, &st, NULL);
    sqlite3_bind_text(st, 1, name, -1, SQLITE_STATIC);
    sqlite3_step(st); sqlite3_finalize(st);
    sqlite3_prepare_v2(db, "SELECT teacher_id FROM teachers WHERE name=?", -1, &st, NULL);
    sqlite3_bind_text(st, 1, name, -1, SQLITE_STATIC);
    int id = 1;
    if (sqlite3_step(st) == SQLITE_ROW) id = sqlite3_column_int(st, 0);
    sqlite3_finalize(st);
    return id;
}

// Get room_id by name (insert if not exists)
static int getRoom(sqlite3* db, const char* name) {
    if (!name || name[0]=='\0') return 1;
    sqlite3_stmt* st = NULL;
    sqlite3_prepare_v2(db, "INSERT OR IGNORE INTO rooms(room_name,room_type) VALUES(?,'Classroom')", -1, &st, NULL);
    sqlite3_bind_text(st, 1, name, -1, SQLITE_STATIC);
    sqlite3_step(st); sqlite3_finalize(st);
    sqlite3_prepare_v2(db, "SELECT room_id FROM rooms WHERE room_name=?", -1, &st, NULL);
    sqlite3_bind_text(st, 1, name, -1, SQLITE_STATIC);
    int id = 1;
    if (sqlite3_step(st) == SQLITE_ROW) id = sqlite3_column_int(st, 0);
    sqlite3_finalize(st);
    return id;
}

// Insert one timetable entry
static void tt(sqlite3* db, const char* sec, const char* day, int slot,
               const char* subj, const char* teacher, const char* room) {
    int tid = getTeacher(db, teacher);
    int rid = getRoom(db, room[0] ? room : "TBD");
    sqlite3_stmt* st = NULL;
    sqlite3_prepare_v2(db,
        "INSERT INTO timetable(room_id,day,slot,subject,teacher_id,section) VALUES(?,?,?,?,?,?)",
        -1, &st, NULL);
    sqlite3_bind_int(st, 1, rid);
    sqlite3_bind_text(st, 2, day,  -1, SQLITE_STATIC);
    sqlite3_bind_int(st, 3, slot);
    sqlite3_bind_text(st, 4, subj, -1, SQLITE_STATIC);
    sqlite3_bind_int(st, 5, tid);
    sqlite3_bind_text(st, 6, sec,  -1, SQLITE_STATIC);
    sqlite3_step(st); sqlite3_finalize(st);
}

void seedTimetable(sqlite3* db) {
    // Check if already seeded
    sqlite3_stmt* chk = NULL;
    sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM timetable WHERE section IN ('A1','A2','B1','B2','C1')", -1, &chk, NULL);
    int cnt = 0;
    if (sqlite3_step(chk) == SQLITE_ROW) cnt = sqlite3_column_int(chk, 0);
    sqlite3_finalize(chk);
    if (cnt > 0) return; // Already seeded

    std::cout << "Seeding timetable data for A1, A2, B1, B2, C1..." << std::endl;
    sqlite3_exec(db, "DELETE FROM timetable;", NULL, NULL, NULL);

    // ── SECTION A1 ──────────────────────────────────────────────────────────
    // MON
    tt(db,"A1","MON",1,"TCS 402 - Finite Automata","DR. VIKRANT SHARMA","LT 201");
    tt(db,"A1","MON",1,"XCS 401 - Career Skills","MR. GAURAV DOBRIYAL","CR 201");
    tt(db,"A1","MON",3,"TCS 421 - Virtualization & Cloud","DR. AMIT GUPTA","LT 302");
    tt(db,"A1","MON",3,"TCS 451 - Statistics and AI","MS. AMRITA TIWARI","CR 205");
    tt(db,"A1","MON",3,"TCS 495 - Cyber Security","DR. PRAKASH SRIVASTAVA","LT 301");
    tt(db,"A1","MON",4,"PROJECT BASED LEARNING","","TBD");
    tt(db,"A1","MON",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"A1","MON",8,"TCS 408 - Programming in Java","MR. AKASH CHAUHAN","LT 201");
    // TUE
    tt(db,"A1","TUE",1,"TCS 402 - Finite Automata","DR. VIKRANT SHARMA","LT 201");
    tt(db,"A1","TUE",2,"TCS 403 - Microprocessors","DR. V.P. DUBEY","CR 104");
    tt(db,"A1","TUE",3,"TCS 421 - Virtualization & Cloud","DR. AMIT GUPTA","LT 302");
    tt(db,"A1","TUE",3,"TCS 451 - Statistics and AI","MS. AMRITA TIWARI","CR 205");
    tt(db,"A1","TUE",3,"TCS 495 - Cyber Security","DR. PRAKASH SRIVASTAVA","LT 301");
    tt(db,"A1","TUE",5,"PCS 408 - Java Lab","MR. AKASH CHAUHAN","MICRO LAB 2");
    tt(db,"A1","TUE",6,"PCS 403 - Micro Lab","DR. V.P. DUBEY","MICRO LAB 1");
    tt(db,"A1","TUE",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"A1","TUE",8,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    // WED
    tt(db,"A1","WED",1,"TCS 402 - Finite Automata","DR. VIKRANT SHARMA","LT 201");
    tt(db,"A1","WED",2,"TCS 403 - Microprocessors","DR. V.P. DUBEY","CR 104");
    tt(db,"A1","WED",3,"TCS 421 - Virtualization & Cloud","DR. AMIT GUPTA","LT 302");
    tt(db,"A1","WED",3,"TCS 451 - Statistics and AI","MS. AMRITA TIWARI","CR 205");
    tt(db,"A1","WED",3,"TCS 495 - Cyber Security","DR. PRAKASH SRIVASTAVA","LT 301");
    tt(db,"A1","WED",5,"TCS 408 - Programming in Java","MR. AKASH CHAUHAN","LT 201");
    tt(db,"A1","WED",6,"PCS 403 - Micro Lab","DR. V.P. DUBEY","MICRO LAB 1");
    tt(db,"A1","WED",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    // THU
    tt(db,"A1","THU",1,"PCS 408 - Java Lab","MR. AKASH CHAUHAN","LAB 1");
    tt(db,"A1","THU",3,"PCS 408 - Java Lab","MR. AKASH CHAUHAN","LAB 1");
    tt(db,"A1","THU",5,"TCS 403 - Microprocessors","DR. V.P. DUBEY","CR 202");
    tt(db,"A1","THU",6,"XCS 401 - Career Skills","MR. GAURAV DOBRIYAL","CR 203");
    tt(db,"A1","THU",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"A1","THU",8,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    // FRI
    tt(db,"A1","FRI",1,"PCS 408 - Java Lab","MR. AKASH CHAUHAN","LAB 1");
    tt(db,"A1","FRI",3,"PCS 403 - Micro Lab","DR. V.P. DUBEY","MICRO LAB 2");
    tt(db,"A1","FRI",4,"TCS 403 - Microprocessors","DR. V.P. DUBEY","CR 104");
    tt(db,"A1","FRI",7,"CEC - Career Excellence","MR. HIMANSHU NAMDEV","NEW AUDI");
    tt(db,"A1","FRI",8,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"A1","FRI",9,"TCS 408 - Programming in Java","MR. AKASH CHAUHAN","LT 201");
    // SAT
    tt(db,"A1","SAT",1,"PCS 409 - DAA Lab","DR. SIDDHANTH THAPLIYAL","TCL 1");
    tt(db,"A1","SAT",3,"PCS 408 - Java Lab","MR. AKASH CHAUHAN","LAB 1");
    tt(db,"A1","SAT",8,"TCS 408 - Programming in Java","MR. AKASH CHAUHAN","LT 201");

    // ── SECTION A2 ──────────────────────────────────────────────────────────
    // MON
    tt(db,"A2","MON",1,"TCS 402 - Finite Automata","DR. VIKRANT SHARMA","LT 201");
    tt(db,"A2","MON",3,"TCS 421 - Virtualization & Cloud","DR. AMIT GUPTA","LT 302");
    tt(db,"A2","MON",3,"TCS 451 - Statistics and AI","MS. AMRITA TIWARI","CR 205");
    tt(db,"A2","MON",3,"TCS 495 - Cyber Security","DR. PRAKASH SRIVASTAVA","LT 301");
    tt(db,"A2","MON",4,"PROJECT BASED LEARNING","","TBD");
    tt(db,"A2","MON",6,"TCS 403 - Microprocessors","DR. V.P. DUBEY","CR 104");
    tt(db,"A2","MON",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"A2","MON",8,"TCS 408 - Programming in Java","MR. AKASH CHAUHAN","LT 201");
    tt(db,"A2","MON",9,"PESE 400 - Employability Skills","MS. SAKSHI PUNDIR","VENUE 1");
    // TUE
    tt(db,"A2","TUE",1,"TCS 402 - Finite Automata","DR. VIKRANT SHARMA","LT 201");
    tt(db,"A2","TUE",2,"TCS 408 - Programming in Java","MR. AKASH CHAUHAN","LT 201");
    tt(db,"A2","TUE",3,"TCS 421 - Virtualization & Cloud","DR. AMIT GUPTA","LT 302");
    tt(db,"A2","TUE",3,"TCS 451 - Statistics and AI","MS. AMRITA TIWARI","CR 205");
    tt(db,"A2","TUE",3,"TCS 495 - Cyber Security","DR. PRAKASH SRIVASTAVA","LT 301");
    tt(db,"A2","TUE",5,"TCS 408 - Programming in Java","MR. AKASH CHAUHAN","LT 201");
    tt(db,"A2","TUE",6,"PCS 403 - Micro Lab","DR. V.P. DUBEY","MICRO LAB 2");
    tt(db,"A2","TUE",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"A2","TUE",8,"TCS 403 - Microprocessors","DR. V.P. DUBEY","CR 104");
    // WED
    tt(db,"A2","WED",1,"TCS 402 - Finite Automata","DR. VIKRANT SHARMA","LT 201");
    tt(db,"A2","WED",2,"TCS 408 - Programming in Java","MR. AKASH CHAUHAN","LT 201");
    tt(db,"A2","WED",3,"TCS 421 - Virtualization & Cloud","DR. AMIT GUPTA","LT 302");
    tt(db,"A2","WED",3,"TCS 451 - Statistics and AI","MS. AMRITA TIWARI","CR 205");
    tt(db,"A2","WED",3,"TCS 495 - Cyber Security","DR. PRAKASH SRIVASTAVA","LT 301");
    tt(db,"A2","WED",5,"PCS 403 - Micro Lab","DR. V.P. DUBEY","MICRO LAB 2");
    tt(db,"A2","WED",6,"PCS 408 - Java Lab","MR. AKASH CHAUHAN","MICRO LAB 1");
    tt(db,"A2","WED",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"A2","WED",8,"TCS 403 - Microprocessors","DR. V.P. DUBEY","CR 104");
    // THU
    tt(db,"A2","THU",1,"PCS 408 - Java Lab","MR. AKASH CHAUHAN","UBUNTU LAB 1");
    tt(db,"A2","THU",2,"PCS 408 - Java Lab","MR. AKASH CHAUHAN","UBUNTU LAB 1");
    tt(db,"A2","THU",3,"PCS 409 - DAA Lab","DR. SIDDHANTH THAPLIYAL","TCL 4");
    tt(db,"A2","THU",4,"PCS 408 - Java Lab","MR. AKASH CHAUHAN","LAB 1");
    tt(db,"A2","THU",5,"TCS 408 - Programming in Java","MR. AKASH CHAUHAN","LT 201");
    tt(db,"A2","THU",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"A2","THU",8,"TCS 408 - Programming in Java","MR. AKASH CHAUHAN","LT 201");
    // FRI
    tt(db,"A2","FRI",1,"PCS 408 - Java Lab","MR. AKASH CHAUHAN","UBUNTU LAB 1");
    tt(db,"A2","FRI",2,"PCS 408 - Java Lab","MR. AKASH CHAUHAN","UBUNTU LAB 1");
    tt(db,"A2","FRI",3,"XCS 401 - Career Skills","MR. GAURAV DOBRIYAL","CR 404");
    tt(db,"A2","FRI",4,"PCS 409 - DAA Lab","DR. SIDDHANTH THAPLIYAL","CR 404");
    tt(db,"A2","FRI",5,"PCS 403 - Micro Lab","DR. V.P. DUBEY","MICRO LAB 1");
    tt(db,"A2","FRI",7,"CEC - Career Excellence","MR. HIMANSHU NAMDEV","NEW AUDI");
    tt(db,"A2","FRI",8,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"A2","FRI",9,"TCS 408 - Programming in Java","MR. AKASH CHAUHAN","LT 201");
    // SAT
    tt(db,"A2","SAT",1,"XCS 401 - Career Skills","MR. GAURAV DOBRIYAL","CR 104");
    tt(db,"A2","SAT",3,"PCS 409 - DAA Lab","DR. SIDDHANTH THAPLIYAL","TCL 4");
    tt(db,"A2","SAT",5,"PCS 403 - Micro Lab","DR. V.P. DUBEY","MICRO LAB 1");
    tt(db,"A2","SAT",8,"PESE 400 - Employability Skills","MS. SAKSHI PUNDIR","VENUE 1");

    // ── SECTION B1 ──────────────────────────────────────────────────────────
    // MON
    tt(db,"B1","MON",3,"TCS 421 - Virtualization & Cloud","DR. AMIT GUPTA","LT 302");
    tt(db,"B1","MON",3,"TCS 451 - Statistics and AI","MS. AMRITA TIWARI","CR 205");
    tt(db,"B1","MON",3,"TCS 495 - Cyber Security","DR. PRAKASH SRIVASTAVA","LT 301");
    tt(db,"B1","MON",4,"TCS 402 - Finite Automata","DR. TEEKAM SINGH","LT 301");
    tt(db,"B1","MON",5,"TCS 408 - Programming in Java","DR. PRATEEK SRIVASTAVA","LT 301");
    tt(db,"B1","MON",6,"TCS 403 - Microprocessors","MR. RAHUL CHAUHAN","CR 205");
    tt(db,"B1","MON",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"B1","MON",8,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    // TUE
    tt(db,"B1","TUE",1,"PCS 408 - Java Lab","DR. PRATEEK SRIVASTAVA","LAB 3");
    tt(db,"B1","TUE",2,"CEC - Career Excellence","MR. HIMANSHU NAMDEV","NEW AUDI");
    tt(db,"B1","TUE",3,"TCS 421 - Virtualization & Cloud","DR. AMIT GUPTA","LT 302");
    tt(db,"B1","TUE",3,"TCS 451 - Statistics and AI","MS. AMRITA TIWARI","CR 205");
    tt(db,"B1","TUE",3,"TCS 495 - Cyber Security","DR. PRAKASH SRIVASTAVA","LT 301");
    tt(db,"B1","TUE",4,"TCS 402 - Finite Automata","DR. TEEKAM SINGH","LT 301");
    tt(db,"B1","TUE",5,"TCS 408 - Programming in Java","DR. PRATEEK SRIVASTAVA","LT 201");
    tt(db,"B1","TUE",6,"TCS 403 - Microprocessors","MR. RAHUL CHAUHAN","CR 205");
    tt(db,"B1","TUE",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"B1","TUE",8,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    // WED
    tt(db,"B1","WED",1,"PCS 408 - Java Lab","DR. PRATEEK SRIVASTAVA","LAB 3");
    tt(db,"B1","WED",2,"PCS 408 - Java Lab","DR. PRATEEK SRIVASTAVA","LAB 3");
    tt(db,"B1","WED",3,"TCS 421 - Virtualization & Cloud","DR. AMIT GUPTA","LT 302");
    tt(db,"B1","WED",3,"TCS 451 - Statistics and AI","MS. AMRITA TIWARI","CR 205");
    tt(db,"B1","WED",3,"TCS 495 - Cyber Security","DR. PRAKASH SRIVASTAVA","LT 301");
    tt(db,"B1","WED",4,"TCS 402 - Finite Automata","DR. TEEKAM SINGH","LT 301");
    tt(db,"B1","WED",5,"TCS 408 - Programming in Java","DR. PRATEEK SRIVASTAVA","LT 201");
    tt(db,"B1","WED",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"B1","WED",8,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    // THU
    tt(db,"B1","THU",1,"PCS 403 - Micro Lab","MR. RAHUL CHAUHAN","MICRO LAB 1");
    tt(db,"B1","THU",2,"PCS 403 - Micro Lab","MR. RAHUL CHAUHAN","MICRO LAB 1");
    tt(db,"B1","THU",5,"TCS 403 - Microprocessors","MR. RAHUL CHAUHAN","CR 205");
    tt(db,"B1","THU",6,"TCS 408 - Programming in Java","DR. PRATEEK SRIVASTAVA","LT 201");
    tt(db,"B1","THU",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"B1","THU",9,"XCS 401 - Career Skills","MR. GAURAV DOBRIYAL","CR 205");
    tt(db,"B1","THU",10,"PCS 403 - Micro Lab","MR. RAHUL CHAUHAN","MICRO LAB 2");
    // FRI
    tt(db,"B1","FRI",3,"TCS 421 - Virtualization & Cloud","DR. AMIT GUPTA","LT 302");
    tt(db,"B1","FRI",3,"TCS 451 - Statistics and AI","MS. AMRITA TIWARI","CR 205");
    tt(db,"B1","FRI",3,"TCS 495 - Cyber Security","DR. PRAKASH SRIVASTAVA","LT 301");
    tt(db,"B1","FRI",5,"TCS 403 - Microprocessors","MR. RAHUL CHAUHAN","CR 205");
    tt(db,"B1","FRI",6,"TCS 403 - Microprocessors","MR. RAHUL CHAUHAN","LT 201");
    tt(db,"B1","FRI",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"B1","FRI",8,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    // SAT
    tt(db,"B1","SAT",4,"XCS 401 - Career Skills","MR. GAURAV DOBRIYAL","CR 205");
    tt(db,"B1","SAT",5,"PCS 403 - Micro Lab","MR. RAHUL CHAUHAN","MICRO LAB 1");

    // ── SECTION B2 ──────────────────────────────────────────────────────────
    // MON
    tt(db,"B2","MON",1,"PESE 400 - Employability Skills","MS. SAKSHI PUNDIR","VENUE 1");
    tt(db,"B2","MON",3,"TCS 421 - Virtualization & Cloud","DR. AMIT GUPTA","LT 302");
    tt(db,"B2","MON",3,"TCS 451 - Statistics and AI","MS. AMRITA TIWARI","CR 205");
    tt(db,"B2","MON",3,"TCS 495 - Cyber Security","DR. PRAKASH SRIVASTAVA","LT 301");
    tt(db,"B2","MON",4,"TCS 402 - Finite Automata","DR. TEEKAM SINGH","LT 301");
    tt(db,"B2","MON",5,"TCS 408 - Programming in Java","DR. PRATEEK SRIVASTAVA","LT 301");
    tt(db,"B2","MON",6,"TCS 403 - Microprocessors","MR. RAHUL CHAUHAN","CR 205");
    tt(db,"B2","MON",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    // TUE
    tt(db,"B2","TUE",1,"PCS 403 - Micro Lab","MR. RAHUL CHAUHAN","MICRO LAB 1");
    tt(db,"B2","TUE",2,"PCS 403 - Micro Lab","MR. RAHUL CHAUHAN","MICRO LAB 1");
    tt(db,"B2","TUE",3,"TCS 421 - Virtualization & Cloud","DR. AMIT GUPTA","LT 302");
    tt(db,"B2","TUE",3,"TCS 451 - Statistics and AI","MS. AMRITA TIWARI","CR 205");
    tt(db,"B2","TUE",3,"TCS 495 - Cyber Security","DR. PRAKASH SRIVASTAVA","LT 301");
    tt(db,"B2","TUE",4,"TCS 402 - Finite Automata","DR. TEEKAM SINGH","LT 301");
    tt(db,"B2","TUE",5,"TCS 408 - Programming in Java","DR. PRATEEK SRIVASTAVA","LT 301");
    tt(db,"B2","TUE",6,"TCS 403 - Microprocessors","MR. RAHUL CHAUHAN","LT 201");
    tt(db,"B2","TUE",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"B2","TUE",8,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    // WED
    tt(db,"B2","WED",1,"PCS 403 - Micro Lab","MR. RAHUL CHAUHAN","MICRO LAB 1");
    tt(db,"B2","WED",2,"PCS 403 - Micro Lab","MR. RAHUL CHAUHAN","MICRO LAB 1");
    tt(db,"B2","WED",3,"TCS 421 - Virtualization & Cloud","DR. AMIT GUPTA","LT 302");
    tt(db,"B2","WED",3,"TCS 451 - Statistics and AI","MS. AMRITA TIWARI","CR 205");
    tt(db,"B2","WED",3,"TCS 495 - Cyber Security","DR. PRAKASH SRIVASTAVA","LT 301");
    tt(db,"B2","WED",4,"TCS 402 - Finite Automata","DR. TEEKAM SINGH","LT 301");
    tt(db,"B2","WED",5,"TCS 408 - Programming in Java","DR. PRATEEK SRIVASTAVA","LT 201");
    tt(db,"B2","WED",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"B2","WED",8,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    // THU
    tt(db,"B2","THU",1,"PCS 403 - Micro Lab","MR. RAHUL CHAUHAN","MICRO LAB 1");
    tt(db,"B2","THU",2,"PCS 403 - Micro Lab","MR. RAHUL CHAUHAN","MICRO LAB 1");
    tt(db,"B2","THU",3,"PCS 409 - DAA Lab","DR. SIDDHANTH THAPLIYAL","TCL 3");
    tt(db,"B2","THU",4,"TCS 402 - Finite Automata","DR. TEEKAM SINGH","LT 301");
    tt(db,"B2","THU",5,"TCS 408 - Programming in Java","DR. PRATEEK SRIVASTAVA","LT 201");
    tt(db,"B2","THU",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"B2","THU",8,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    // FRI
    tt(db,"B2","FRI",1,"PCS 408 - Java Lab","DR. PRATEEK SRIVASTAVA","LAB 3");
    tt(db,"B2","FRI",2,"PCS 408 - Java Lab","DR. PRATEEK SRIVASTAVA","LAB 3");
    tt(db,"B2","FRI",3,"TCS 421 - Virtualization & Cloud","DR. AMIT GUPTA","LT 302");
    tt(db,"B2","FRI",3,"TCS 451 - Statistics and AI","MS. AMRITA TIWARI","CR 205");
    tt(db,"B2","FRI",3,"TCS 495 - Cyber Security","DR. PRAKASH SRIVASTAVA","LT 301");
    tt(db,"B2","FRI",5,"TCS 403 - Microprocessors","MR. RAHUL CHAUHAN","CR 205");
    tt(db,"B2","FRI",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"B2","FRI",8,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    // SAT
    tt(db,"B2","SAT",1,"XCS 401 - Career Skills","MR. GAURAV DOBRIYAL","CR 205");
    tt(db,"B2","SAT",2,"CEC - Career Excellence","MR. HIMANSHU NAMDEV","NEW AUDI");
    tt(db,"B2","SAT",4,"PCS 408 - Java Lab","DR. PRATEEK SRIVASTAVA","UBUNTU LAB 2");
    tt(db,"B2","SAT",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"B2","SAT",8,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");

    // ── SECTION C1 ──────────────────────────────────────────────────────────
    // MON
    tt(db,"C1","MON",3,"TCS 421 - Virtualization & Cloud","DR. AMIT GUPTA","LT 302");
    tt(db,"C1","MON",3,"TCS 451 - Statistics and AI","MS. AMRITA TIWARI","CR 205");
    tt(db,"C1","MON",3,"TCS 495 - Cyber Security","DR. PRAKASH SRIVASTAVA","LT 301");
    tt(db,"C1","MON",4,"TCS 402 - Finite Automata","DR. VIKRANT SHARMA","LT 201");
    tt(db,"C1","MON",5,"TCS 408 - Programming in Java","DR. AMIT GUPTA","LT 201");
    tt(db,"C1","MON",6,"TCS 408 - Programming in Java","DR. AMIT GUPTA","LT 201");
    tt(db,"C1","MON",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"C1","MON",9,"PROJECT BASED LEARNING","","TBD");
    // TUE
    tt(db,"C1","TUE",1,"XCS 401 - Career Skills","MR. GAURAV DOBRIYAL","CR 201");
    tt(db,"C1","TUE",3,"TCS 421 - Virtualization & Cloud","DR. AMIT GUPTA","LT 302");
    tt(db,"C1","TUE",3,"TCS 451 - Statistics and AI","MS. AMRITA TIWARI","CR 205");
    tt(db,"C1","TUE",3,"TCS 495 - Cyber Security","DR. PRAKASH SRIVASTAVA","LT 301");
    tt(db,"C1","TUE",4,"TCS 402 - Finite Automata","DR. VIKRANT SHARMA","LT 201");
    tt(db,"C1","TUE",5,"TCS 408 - Programming in Java","DR. AMIT GUPTA","LT 201");
    tt(db,"C1","TUE",6,"PESE 400 - Employability Skills","MS. SAKSHI PUNDIR","VENUE 1");
    tt(db,"C1","TUE",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"C1","TUE",8,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    // WED
    tt(db,"C1","WED",3,"TCS 421 - Virtualization & Cloud","DR. AMIT GUPTA","LT 302");
    tt(db,"C1","WED",3,"TCS 451 - Statistics and AI","MS. AMRITA TIWARI","CR 205");
    tt(db,"C1","WED",3,"TCS 495 - Cyber Security","DR. PRAKASH SRIVASTAVA","LT 301");
    tt(db,"C1","WED",4,"TCS 402 - Finite Automata","DR. VIKRANT SHARMA","LT 201");
    tt(db,"C1","WED",5,"TCS 408 - Programming in Java","DR. AMIT GUPTA","LT 201");
    tt(db,"C1","WED",6,"TCS 403 - Microprocessors","MR. DIVESH KUMAR","CR 104");
    tt(db,"C1","WED",7,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"C1","WED",8,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    // THU
    tt(db,"C1","THU",1,"TCS 408 - Programming in Java","DR. AMIT GUPTA","LT 201");
    tt(db,"C1","THU",3,"TCS 421 - Virtualization & Cloud","DR. AMIT GUPTA","LT 302");
    tt(db,"C1","THU",3,"TCS 451 - Statistics and AI","MS. AMRITA TIWARI","CR 205");
    tt(db,"C1","THU",3,"TCS 495 - Cyber Security","DR. PRAKASH SRIVASTAVA","LT 301");
    tt(db,"C1","THU",5,"TCS 403 - Microprocessors","MR. DIVESH KUMAR","CR 104");
    tt(db,"C1","THU",6,"TCS 403 - Microprocessors","MR. DIVESH KUMAR","CR 201");
    tt(db,"C1","THU",7,"TCS 402 - Finite Automata","DR. VIKRANT SHARMA","LT 301");
    tt(db,"C1","THU",8,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    // FRI
    tt(db,"C1","FRI",1,"TCS 408 - Programming in Java","DR. AMIT GUPTA","LT 201");
    tt(db,"C1","FRI",2,"CEC - Career Excellence","MR. HIMANSHU NAMDEV","NEW AUDI");
    tt(db,"C1","FRI",3,"TCS 421 - Virtualization & Cloud","DR. AMIT GUPTA","LT 302");
    tt(db,"C1","FRI",3,"TCS 451 - Statistics and AI","MS. AMRITA TIWARI","CR 205");
    tt(db,"C1","FRI",3,"TCS 495 - Cyber Security","DR. PRAKASH SRIVASTAVA","LT 301");
    tt(db,"C1","FRI",4,"PCS 403 - Micro Lab","MR. DIVESH KUMAR","MICRO LAB 2");
    tt(db,"C1","FRI",5,"TCS 403 - Microprocessors","MR. DIVESH KUMAR","CR 201");
    tt(db,"C1","FRI",7,"TCS 402 - Finite Automata","DR. VIKRANT SHARMA","LT 301");
    tt(db,"C1","FRI",8,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    // SAT
    tt(db,"C1","SAT",1,"XCS 401 - Career Skills","MR. GAURAV DOBRIYAL","CR 201");
    tt(db,"C1","SAT",2,"CEC - Career Excellence","MR. HIMANSHU NAMDEV","NEW AUDI");
    tt(db,"C1","SAT",3,"TCS 421 - Virtualization & Cloud","DR. AMIT GUPTA","LT 302");
    tt(db,"C1","SAT",3,"TCS 451 - Statistics and AI","MS. AMRITA TIWARI","CR 205");
    tt(db,"C1","SAT",3,"TCS 495 - Cyber Security","DR. PRAKASH SRIVASTAVA","LT 301");
    tt(db,"C1","SAT",4,"PCS 409 - DAA Lab","DR. SIDDHANTH THAPLIYAL","LAB 9");
    tt(db,"C1","SAT",5,"PCS 403 - Micro Lab","MR. DIVESH KUMAR","MICRO LAB 1");
    tt(db,"C1","SAT",8,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");
    tt(db,"C1","SAT",9,"TCS 409 - Design & Analysis","DR. SIDDHANTH THAPLIYAL","LT 202");

    std::cout << "Timetable seeded for A1, A2, B1, B2, C1." << std::endl;
}

void setupDB() {
    sqlite3* db = openDB();
    if (!db) return;

    const char* queries[] = {
        "CREATE TABLE IF NOT EXISTS teachers (teacher_id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT UNIQUE, department TEXT, email TEXT, password TEXT DEFAULT 'pass123');",
        "CREATE TABLE IF NOT EXISTS rooms (room_id INTEGER PRIMARY KEY AUTOINCREMENT, room_name TEXT UNIQUE, room_type TEXT, capacity INTEGER);",
        "CREATE TABLE IF NOT EXISTS timetable (id INTEGER PRIMARY KEY AUTOINCREMENT, room_id INTEGER, day TEXT, slot INTEGER, subject TEXT, teacher_id INTEGER, section TEXT DEFAULT '');",
        "CREATE TABLE IF NOT EXISTS bookings (booking_id INTEGER PRIMARY KEY AUTOINCREMENT, teacher_id INTEGER, room_id INTEGER, date TEXT, slot INTEGER, status TEXT);"
    };
    char* err = NULL;
    for (int i = 0; i < 4; i++) {
        sqlite3_exec(db, queries[i], NULL, NULL, &err);
        if (err) { sqlite3_free(err); err = NULL; }
    }
    sqlite3_exec(db, "ALTER TABLE teachers ADD COLUMN password TEXT DEFAULT 'pass123';", NULL, NULL, NULL);
    sqlite3_exec(db, "ALTER TABLE teachers ADD COLUMN email TEXT DEFAULT '';",           NULL, NULL, NULL);
    sqlite3_exec(db, "ALTER TABLE timetable ADD COLUMN section TEXT DEFAULT '';",        NULL, NULL, NULL);

    seedTimetable(db);
    closeDB(db);
    std::cout << "Database ready." << std::endl;
}

#endif

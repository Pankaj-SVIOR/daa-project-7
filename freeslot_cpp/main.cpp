#include "utils/server.h"
#include "utils/database.h"
#include "models/dsa.h"
#include "models/session.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <cctype>

std::map<int, std::string> slotToTime;
std::map<std::string, int> timeToSlot;

void initSlotMaps() {
    slotToTime[1]="08:00-08:55"; slotToTime[2]="08:55-09:50";
    slotToTime[3]="10:10-11:05"; slotToTime[4]="11:05-12:00";
    slotToTime[5]="12:00-12:55"; slotToTime[6]="12:55-01:50";
    slotToTime[7]="02:10-03:05"; slotToTime[8]="03:05-04:00";
    slotToTime[9]="04:00-04:55"; slotToTime[10]="04:55-05:50";
    std::map<int,std::string>::iterator it;
    for (it=slotToTime.begin(); it!=slotToTime.end(); ++it)
        timeToSlot[it->second] = it->first;
}

BookingStack undoStack;
BookingQueue bookQueue;
std::map<int, BookingHistory*> roomHistories;

int parseSlot(const std::string& s) {
    if (s.empty()) return -1;
    bool isNum = true;
    for (int i=0;i<(int)s.size();i++) if(!isdigit((unsigned char)s[i])){isNum=false;break;}
    if (isNum) return atoi(s.c_str());
    std::map<std::string,int>::iterator it = timeToSlot.find(s);
    return (it != timeToSlot.end()) ? it->second : -1;
}

std::string toUpper(std::string s) {
    for (int i=0;i<(int)s.size();i++) s[i]=toupper((unsigned char)s[i]);
    return s;
}

std::string todayDay() {
    time_t now = time(0); tm* t = localtime(&now);
    const char* d[]={"SUN","MON","TUE","WED","THU","FRI","SAT"};
    return d[t->tm_wday];
}

std::string readFile(const std::string& name) {
    std::ifstream f("views/" + name);
    if (!f.is_open()) return "<h1 style='color:red;font-family:sans-serif'>Error: views/" + name + " not found.</h1>";
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

std::string je(const std::string& s) {
    std::string r;
    for (int i=0;i<(int)s.size();i++) {
        char c=s[i];
        if(c=='"')r+="\\\""; else if(c=='\\')r+="\\\\"; else if(c=='\n')r+="\\n"; else if(c!='\r')r+=c;
    }
    return r;
}

std::string jarr(const std::vector<std::string>& v) {
    std::string s="[";
    for(int i=0;i<(int)v.size();i++){if(i)s+=","; s+=v[i];} return s+"]";
}

std::string jok(const std::string& msg)   { return "{\"status\":\"success\",\"message\":\""+je(msg)+"\"}"; }
std::string jfail(const std::string& msg) { return "{\"status\":\"failed\",\"message\":\""+je(msg)+"\"}"; }
std::string jerr(const std::string& msg)  { return "{\"error\":\""+je(msg)+"\"}"; }

std::map<std::string,std::string> parseJson(const std::string& body) {
    std::map<std::string,std::string> res;
    size_t pos=body.find('{'); if(pos==std::string::npos) return res; pos++;
    while(pos<body.size()) {
        while(pos<body.size()&&(body[pos]==' '||body[pos]=='\n'||body[pos]=='\r'||body[pos]=='\t'))pos++;
        if(pos>=body.size()||body[pos]=='}')break;
        if(body[pos]!='"'){pos++;continue;}
        pos++;
        std::string key; while(pos<body.size()&&body[pos]!='"')key+=body[pos++]; pos++;
        while(pos<body.size()&&(body[pos]==' '||body[pos]==':'))pos++;
        std::string val;
        if(pos<body.size()&&body[pos]=='"'){
            pos++;
            while(pos<body.size()&&body[pos]!='"'){if(body[pos]=='\\'&&pos+1<body.size())pos++; val+=body[pos++];}
            pos++;
        } else {
            while(pos<body.size()&&body[pos]!=','&&body[pos]!='}'&&body[pos]!='\n'&&body[pos]!='\r')val+=body[pos++];
            while(!val.empty()&&(val.back()==' '||val.back()=='\t'))val.pop_back();
        }
        if(!key.empty()) res[key]=val;
        while(pos<body.size()&&(body[pos]==','||body[pos]==' '||body[pos]=='\n'||body[pos]=='\r'))pos++;
    }
    return res;
}

int checkAuth(const Request& req) {
    std::string token = getCookie(req, COOKIE_NAME);
    if (token.empty()) return -1;
    return getSessionTeacherId(token);
}

int main() {
    initSlotMaps();
    setupDB();
    SimpleServer server;

    server.Get("/", [](const Request& req, Response& res) {
        if (checkAuth(req) > 0) res.redirect("/dashboard");
        else res.redirect("/login");
    });

    server.Get("/login", [](const Request&, Response& res) {
        res.set_content(readFile("login.html"), "text/html");
    });

    server.Get("/forgot-password", [](const Request&, Response& res) {
        res.set_content(readFile("forgot_password.html"), "text/html");
    });

    server.Get("/logout", [](const Request& req, Response& res) {
        std::string token = getCookie(req, COOKIE_NAME);
        if (!token.empty()) destroySession(token);
        res.clear_cookie(COOKIE_NAME);
        res.redirect("/login");
    });

    server.Get("/dashboard", [](const Request& req, Response& res) {
        if (checkAuth(req) < 0) { res.redirect("/login"); return; }
        res.set_content(readFile("dashboard.html"), "text/html");
    });

    server.Get("/today-classes", [](const Request& req, Response& res) {
        if (checkAuth(req) < 0) { res.redirect("/login"); return; }
        res.set_content(readFile("today_classes.html"), "text/html");
    });

    server.Get("/free-slots-page", [](const Request& req, Response& res) {
        if (checkAuth(req) < 0) { res.redirect("/login"); return; }
        res.set_content(readFile("free_slots.html"), "text/html");
    });

    server.Get("/free_slots_page", [](const Request& req, Response& res) {
        if (checkAuth(req) < 0) { res.redirect("/login"); return; }
        res.set_content(readFile("free_slots.html"), "text/html");
    });

    server.Get("/book-room-page", [](const Request& req, Response& res) {
        if (checkAuth(req) < 0) { res.redirect("/login"); return; }
        res.set_content(readFile("book_room.html"), "text/html");
    });

    server.Get("/book_room_page", [](const Request& req, Response& res) {
        if (checkAuth(req) < 0) { res.redirect("/login"); return; }
        res.set_content(readFile("book_room.html"), "text/html");
    });

    server.Get("/weekly-schedule", [](const Request& req, Response& res) {
        if (checkAuth(req) < 0) { res.redirect("/login"); return; }
        res.set_content(readFile("weekly_schedule.html"), "text/html");
    });

    server.Get("/profile", [](const Request& req, Response& res) {
        if (checkAuth(req) < 0) { res.redirect("/login"); return; }
        res.set_content(readFile("profile.html"), "text/html");
    });

    server.Get("/teacher_dashboard", [](const Request& req, Response& res) {
        if (checkAuth(req) < 0) { res.redirect("/login"); return; }
        res.set_content(readFile("dashboard.html"), "text/html");
    });

    server.Post("/api/login", [](const Request& req, Response& res) {
        std::map<std::string,std::string> data = parseJson(req.body);
        std::string name = data.count("name")     ? data["name"]     : "";
        std::string pass = data.count("password") ? data["password"] : "";
        if (name.empty() || pass.empty()) {
            res.set_content(jfail("Name and password required"), "application/json"); return;
        }
        sqlite3* db = openDB();
        if (!db) { res.set_content(jfail("DB error"), "application/json"); return; }
        sqlite3_stmt* st = NULL;
        sqlite3_prepare_v2(db,
            "SELECT teacher_id, name, department FROM teachers WHERE name=? AND password=?",
            -1, &st, NULL);
        sqlite3_bind_text(st, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(st, 2, pass.c_str(), -1, SQLITE_STATIC);
        int tid = -1; std::string tname, tdept;
        if (sqlite3_step(st) == SQLITE_ROW) {
            tid   = sqlite3_column_int(st, 0);
            tname = (const char*)sqlite3_column_text(st, 1);
            tdept = sqlite3_column_text(st, 2) ? (const char*)sqlite3_column_text(st, 2) : "";
        }
        sqlite3_finalize(st);
        closeDB(db);
        if (tid < 0) {
            res.set_content(jfail("Invalid name or password"), "application/json"); return;
        }
        std::string token = createSession(tid, tname);
        res.set_cookie(COOKIE_NAME, token, "HttpOnly");
        res.set_content(jok("Login successful"), "application/json");
    });

    server.Post("/api/reset-password", [](const Request& req, Response& res) {
        std::map<std::string,std::string> data = parseJson(req.body);
        std::string name    = data.count("name")         ? data["name"]         : "";
        std::string newPass = data.count("new_password") ? data["new_password"] : "";
        if (name.empty() || newPass.empty()) {
            res.set_content(jfail("Name and new password required"), "application/json"); return;
        }
        sqlite3* db = openDB();
        if (!db) { res.set_content(jfail("DB error"), "application/json"); return; }
        sqlite3_stmt* st = NULL;
        sqlite3_prepare_v2(db, "UPDATE teachers SET password=? WHERE name=?", -1, &st, NULL);
        sqlite3_bind_text(st, 1, newPass.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(st, 2, name.c_str(),   -1, SQLITE_STATIC);
        sqlite3_step(st); sqlite3_finalize(st);
        int rows = sqlite3_changes(db);
        closeDB(db);
        if (rows == 0) { res.set_content(jfail("Teacher not found"), "application/json"); return; }
        res.set_content(jok("Password reset successfully"), "application/json");
    });

    server.Post("/api/change-password", [](const Request& req, Response& res) {
        if (checkAuth(req) < 0) { res.set_content(jfail("Not logged in"), "application/json"); return; }
        std::map<std::string,std::string> data = parseJson(req.body);
        std::string name    = data.count("name")         ? data["name"]         : "";
        std::string oldPass = data.count("old_password") ? data["old_password"] : "";
        std::string newPass = data.count("new_password") ? data["new_password"] : "";
        if (name.empty()||oldPass.empty()||newPass.empty()) {
            res.set_content(jfail("All fields required"), "application/json"); return;
        }
        sqlite3* db = openDB();
        if (!db) { res.set_content(jfail("DB error"), "application/json"); return; }
        sqlite3_stmt* st = NULL;
        sqlite3_prepare_v2(db,"SELECT teacher_id FROM teachers WHERE name=? AND password=?",-1,&st,NULL);
        sqlite3_bind_text(st,1,name.c_str(),-1,SQLITE_STATIC);
        sqlite3_bind_text(st,2,oldPass.c_str(),-1,SQLITE_STATIC);
        bool valid = (sqlite3_step(st)==SQLITE_ROW);
        sqlite3_finalize(st);
        if (!valid) { closeDB(db); res.set_content(jfail("Current password is wrong"), "application/json"); return; }
        sqlite3_prepare_v2(db,"UPDATE teachers SET password=? WHERE name=?",-1,&st,NULL);
        sqlite3_bind_text(st,1,newPass.c_str(),-1,SQLITE_STATIC);
        sqlite3_bind_text(st,2,name.c_str(),-1,SQLITE_STATIC);
        sqlite3_step(st); sqlite3_finalize(st);
        closeDB(db);
        res.set_content(jok("Password changed successfully"), "application/json");
    });

    server.Get("/api/me", [](const Request& req, Response& res) {
        std::string token = getCookie(req, COOKIE_NAME);
        int tid = getSessionTeacherId(token);
        if (tid < 0) { res.set_content(jerr("Not logged in"), "application/json"); return; }
        sqlite3* db = openDB();
        if (!db) { res.set_content(jerr("DB error"), "application/json"); return; }
        sqlite3_stmt* st = NULL;
        sqlite3_prepare_v2(db,
            "SELECT teacher_id, name, department, email FROM teachers WHERE teacher_id=?",
            -1, &st, NULL);
        sqlite3_bind_int(st, 1, tid);
        std::string result = jerr("Not found");
        if (sqlite3_step(st) == SQLITE_ROW) {
            int id       = sqlite3_column_int(st, 0);
            std::string name  = (const char*)sqlite3_column_text(st, 1);
            std::string dept  = sqlite3_column_text(st, 2) ? (const char*)sqlite3_column_text(st, 2) : "";
            std::string email = sqlite3_column_text(st, 3) ? (const char*)sqlite3_column_text(st, 3) : "";
            result = "{\"teacher_id\":"+std::to_string(id)+",\"name\":\""+je(name)+"\","
                     "\"department\":\""+je(dept)+"\",\"email\":\""+je(email)+"\"}";
        }
        sqlite3_finalize(st); closeDB(db);
        res.set_content(result, "application/json");
    });

    server.Get("/teachers", [](const Request&, Response& res) {
        sqlite3* db = openDB();
        if (!db) { res.set_content(jerr("DB error"),"application/json"); return; }
        sqlite3_stmt* st = NULL;
        sqlite3_prepare_v2(db,"SELECT teacher_id, name FROM teachers ORDER BY name",-1,&st,NULL);
        std::vector<std::string> rows;
        while(sqlite3_step(st)==SQLITE_ROW)
            rows.push_back("{\"teacher_id\":"+std::to_string(sqlite3_column_int(st,0))
                          +",\"name\":\""+je((const char*)sqlite3_column_text(st,1))+"\"}");
        sqlite3_finalize(st); closeDB(db);
        res.set_content(jarr(rows), "application/json");
    });

    server.Get("/api/rooms", [](const Request&, Response& res) {
        sqlite3* db = openDB();
        if (!db) { res.set_content(jerr("DB error"),"application/json"); return; }
        sqlite3_stmt* st = NULL;
        sqlite3_prepare_v2(db,"SELECT room_id, room_name, room_type FROM rooms ORDER BY room_name",-1,&st,NULL);
        std::vector<std::string> rows;
        while(sqlite3_step(st)==SQLITE_ROW) {
            int rid = sqlite3_column_int(st, 0);
            std::string rname = (const char*)sqlite3_column_text(st, 1);
            std::string rtype = sqlite3_column_text(st, 2) ? (const char*)sqlite3_column_text(st, 2) : "";
            rows.push_back("{\"room_id\":"+std::to_string(rid)+",\"room_name\":\""+je(rname)+"\",\"room_type\":\""+je(rtype)+"\"}");
        }
        sqlite3_finalize(st); closeDB(db);
        res.set_content(jarr(rows), "application/json");
    });

    server.Get("/free_rooms_today", [](const Request&, Response& res) {
        std::string today = todayDay();
        sqlite3* db = openDB();
        if (!db) { res.set_content(jerr("DB error"),"application/json"); return; }
        std::map<int,std::string> allRooms;
        sqlite3_stmt* st = NULL;
        sqlite3_prepare_v2(db,"SELECT room_id, room_name FROM rooms",-1,&st,NULL);
        while(sqlite3_step(st)==SQLITE_ROW) allRooms[sqlite3_column_int(st,0)]=(const char*)sqlite3_column_text(st,1);
        sqlite3_finalize(st);
        std::map<int,std::set<int>> busyBySlot;
        sqlite3_prepare_v2(db,"SELECT room_id, slot FROM timetable WHERE day=?",-1,&st,NULL);
        sqlite3_bind_text(st,1,today.c_str(),-1,SQLITE_STATIC);
        while(sqlite3_step(st)==SQLITE_ROW) busyBySlot[sqlite3_column_int(st,1)].insert(sqlite3_column_int(st,0));
        sqlite3_finalize(st); closeDB(db);
        std::vector<std::string> result;
        for(int s=1;s<=10;s++) {
            if(!slotToTime.count(s)) continue;
            std::string timeStr=slotToTime[s];
            std::vector<std::string> freeRooms;
            std::map<int,std::string>::iterator rit;
            for(rit=allRooms.begin();rit!=allRooms.end();++rit) {
                bool busy = busyBySlot.count(s)&&busyBySlot[s].count(rit->first);
                if(!busy) freeRooms.push_back("\""+je(rit->second)+"\"");
            }
            std::string ra="["; for(int i=0;i<(int)freeRooms.size();i++){if(i)ra+=","; ra+=freeRooms[i];} ra+="]";
            result.push_back("{\"slot\":\""+timeStr+"\",\"free_rooms\":"+ra+"}");
        }
        res.set_content(jarr(result), "application/json");
    });

    server.Get("/teacher_classes_today/([0-9]+)", [](const Request& req, Response& res) {
        int tid = atoi(req.matches[0].c_str());
        std::string today = todayDay();
        sqlite3* db = openDB();
        if (!db) { res.set_content(jerr("DB error"),"application/json"); return; }
        const char* sql =
            "SELECT tt.day, tt.slot, tt.subject, r.room_name, t.name "
            "FROM timetable tt JOIN rooms r ON r.room_id=tt.room_id "
            "JOIN teachers t ON t.teacher_id=tt.teacher_id "
            "WHERE tt.teacher_id=? AND tt.day=? ORDER BY CAST(tt.slot AS INTEGER)";
        sqlite3_stmt* st = NULL;
        sqlite3_prepare_v2(db,sql,-1,&st,NULL);
        sqlite3_bind_int(st,1,tid); sqlite3_bind_text(st,2,today.c_str(),-1,SQLITE_STATIC);
        std::vector<std::string> rows;
        while(sqlite3_step(st)==SQLITE_ROW) {
            std::string day   = (const char*)sqlite3_column_text(st,0);
            int sn            = sqlite3_column_int(st,1);
            std::string subj  = (const char*)sqlite3_column_text(st,2);
            std::string room  = (const char*)sqlite3_column_text(st,3);
            std::string tname = (const char*)sqlite3_column_text(st,4);
            std::string ts    = slotToTime.count(sn)?slotToTime[sn]:std::to_string(sn);
            rows.push_back("{\"day\":\""+day+"\",\"slot\":\""+ts+"\",\"subject\":\""+je(subj)+"\","
                           "\"room\":\""+je(room)+"\",\"teacher\":\""+je(tname)+"\"}");
        }
        sqlite3_finalize(st); closeDB(db);
        res.set_content(jarr(rows), "application/json");
    });

    server.Get("/api/weekly_schedule/([0-9]+)", [](const Request& req, Response& res) {
        int tid = atoi(req.matches[0].c_str());
        sqlite3* db = openDB();
        if (!db) { res.set_content(jerr("DB error"),"application/json"); return; }
        std::map<int,std::string> roomMap;
        sqlite3_stmt* st = NULL;
        sqlite3_prepare_v2(db,"SELECT room_id, room_name FROM rooms",-1,&st,NULL);
        while(sqlite3_step(st)==SQLITE_ROW) roomMap[sqlite3_column_int(st,0)]=(const char*)sqlite3_column_text(st,1);
        sqlite3_finalize(st);
        const char* sql =
            "SELECT day, slot, subject, room_id FROM timetable WHERE teacher_id=? "
            "ORDER BY CASE day WHEN 'MON' THEN 1 WHEN 'TUE' THEN 2 WHEN 'WED' THEN 3 "
            "WHEN 'THU' THEN 4 WHEN 'FRI' THEN 5 WHEN 'SAT' THEN 6 END, CAST(slot AS INTEGER)";
        sqlite3_prepare_v2(db,sql,-1,&st,NULL);
        sqlite3_bind_int(st,1,tid);
        std::vector<std::string> rows;
        while(sqlite3_step(st)==SQLITE_ROW) {
            std::string day  = (const char*)sqlite3_column_text(st,0);
            int sn           = sqlite3_column_int(st,1);
            std::string subj = (const char*)sqlite3_column_text(st,2);
            int rid          = sqlite3_column_int(st,3);
            std::string room = roomMap.count(rid)?roomMap[rid]:"Unknown";
            std::string ts   = slotToTime.count(sn)?slotToTime[sn]:std::to_string(sn);
            rows.push_back("{\"day\":\""+day+"\",\"slot\":\""+ts+"\",\"subject\":\""+je(subj)+"\",\"room\":\""+je(room)+"\"}");
        }
        sqlite3_finalize(st); closeDB(db);
        res.set_content(jarr(rows), "application/json");
    });

    server.Get("/today_bookings", [](const Request&, Response& res) {
        sqlite3* db = openDB();
        if (!db) { res.set_content(jerr("DB error"),"application/json"); return; }
        sqlite3_stmt* st = NULL;
        sqlite3_prepare_v2(db,
            "SELECT t.name, r.room_name, b.slot, b.date FROM bookings b "
            "JOIN teachers t ON t.teacher_id=b.teacher_id "
            "JOIN rooms r ON r.room_id=b.room_id "
            "WHERE b.status='approved' ORDER BY b.date DESC, b.slot",
            -1,&st,NULL);
        std::vector<std::string> rows;
        while(sqlite3_step(st)==SQLITE_ROW) {
            std::string teacher = (const char*)sqlite3_column_text(st,0);
            std::string room    = (const char*)sqlite3_column_text(st,1);
            int sn              = sqlite3_column_int(st,2);
            std::string date    = (const char*)sqlite3_column_text(st,3);
            std::string ts      = slotToTime.count(sn)?slotToTime[sn]:std::to_string(sn);
            rows.push_back("{\"teacher\":\""+je(teacher)+"\",\"room\":\""+je(room)+"\","
                           "\"slot\":\""+ts+"\",\"date\":\""+date+"\"}");
        }
        sqlite3_finalize(st); closeDB(db);
        res.set_content(jarr(rows), "application/json");
    });

    server.Get("/free_rooms", [](const Request& req, Response& res) {
        std::string day=toUpper(getParam(req,"day")), slotSt=getParam(req,"slot");
        if(day.empty()||slotSt.empty()){res.status=400;res.set_content(jerr("Missing day or slot"),"application/json");return;}
        int sn=parseSlot(slotSt);
        if(sn<0){res.status=400;res.set_content(jerr("Invalid slot"),"application/json");return;}
        sqlite3* db=openDB();
        if(!db){res.set_content(jerr("DB error"),"application/json");return;}
        std::map<int,std::string> allRooms;
        sqlite3_stmt* st=NULL;
        sqlite3_prepare_v2(db,"SELECT room_id, room_name FROM rooms",-1,&st,NULL);
        while(sqlite3_step(st)==SQLITE_ROW) allRooms[sqlite3_column_int(st,0)]=(const char*)sqlite3_column_text(st,1);
        sqlite3_finalize(st);
        std::set<int> busy;
        sqlite3_prepare_v2(db,"SELECT room_id FROM timetable WHERE day=? AND slot=?",-1,&st,NULL);
        sqlite3_bind_text(st,1,day.c_str(),-1,SQLITE_STATIC); sqlite3_bind_int(st,2,sn);
        while(sqlite3_step(st)==SQLITE_ROW) busy.insert(sqlite3_column_int(st,0));
        sqlite3_finalize(st);
        sqlite3_prepare_v2(db,"SELECT room_id FROM bookings WHERE date=? AND slot=? AND status='approved'",-1,&st,NULL);
        sqlite3_bind_text(st,1,day.c_str(),-1,SQLITE_STATIC); sqlite3_bind_int(st,2,sn);
        while(sqlite3_step(st)==SQLITE_ROW) busy.insert(sqlite3_column_int(st,0));
        sqlite3_finalize(st); closeDB(db);
        std::vector<std::string> freeList;
        std::map<int,std::string>::iterator it;
        for(it=allRooms.begin();it!=allRooms.end();++it)
            if(!busy.count(it->first))
                freeList.push_back("{\"room_id\":"+std::to_string(it->first)+",\"room_name\":\""+je(it->second)+"\"}");
        res.set_content("{\"day\":\""+day+"\",\"slot\":\""+slotSt+"\",\"free_rooms\":"+jarr(freeList)+"}","application/json");
    });

    server.Post("/book_room", [](const Request& req, Response& res) {
        std::map<std::string,std::string> data=parseJson(req.body);
        std::string tidStr=data.count("teacher_id")?data["teacher_id"]:"";
        std::string roomIn=data.count("room_id")?data["room_id"]:"";
        std::string date=data.count("date")?data["date"]:"";
        std::string slotIn=data.count("slot")?data["slot"]:"";
        if(tidStr.empty()||roomIn.empty()||date.empty()||slotIn.empty()){res.status=400;res.set_content(jfail("Missing fields"),"application/json");return;}
        int tid=atoi(tidStr.c_str());
        int sn=parseSlot(slotIn);
        if(sn<0){res.status=400;res.set_content(jfail("Invalid slot"),"application/json");return;}
        std::string dayName=toUpper(date);
        sqlite3* db=openDB();
        if(!db){res.set_content(jfail("DB error"),"application/json");return;}
        int roomId=-1;
        bool isNum=!roomIn.empty();
        for(int i=0;i<(int)roomIn.size();i++) if(!isdigit((unsigned char)roomIn[i])){isNum=false;break;}
        if(isNum) roomId=atoi(roomIn.c_str());
        else {
            sqlite3_stmt* st=NULL;
            sqlite3_prepare_v2(db,"SELECT room_id FROM rooms WHERE room_name=?",-1,&st,NULL);
            sqlite3_bind_text(st,1,toUpper(roomIn).c_str(),-1,SQLITE_STATIC);
            if(sqlite3_step(st)==SQLITE_ROW) roomId=sqlite3_column_int(st,0);
            sqlite3_finalize(st);
        }
        if(roomId<0){closeDB(db);res.set_content(jfail("Invalid room"),"application/json");return;}
        sqlite3_stmt* st=NULL;
        sqlite3_prepare_v2(db,"SELECT 1 FROM timetable WHERE teacher_id=? AND day=? AND slot=?",-1,&st,NULL);
        sqlite3_bind_int(st,1,tid);sqlite3_bind_text(st,2,dayName.c_str(),-1,SQLITE_STATIC);sqlite3_bind_int(st,3,sn);
        if(sqlite3_step(st)==SQLITE_ROW){sqlite3_finalize(st);closeDB(db);res.set_content(jfail("Teacher busy at this slot"),"application/json");return;}
        sqlite3_finalize(st);
        sqlite3_prepare_v2(db,"SELECT 1 FROM timetable WHERE room_id=? AND day=? AND slot=?",-1,&st,NULL);
        sqlite3_bind_int(st,1,roomId);sqlite3_bind_text(st,2,dayName.c_str(),-1,SQLITE_STATIC);sqlite3_bind_int(st,3,sn);
        if(sqlite3_step(st)==SQLITE_ROW){sqlite3_finalize(st);closeDB(db);res.set_content(jfail("Room busy at this slot"),"application/json");return;}
        sqlite3_finalize(st);
        sqlite3_prepare_v2(db,"SELECT 1 FROM bookings WHERE teacher_id=? AND date=? AND slot=? AND status='approved'",-1,&st,NULL);
        sqlite3_bind_int(st,1,tid);sqlite3_bind_text(st,2,date.c_str(),-1,SQLITE_STATIC);sqlite3_bind_int(st,3,sn);
        if(sqlite3_step(st)==SQLITE_ROW){sqlite3_finalize(st);closeDB(db);res.set_content(jfail("Teacher already booked"),"application/json");return;}
        sqlite3_finalize(st);
        sqlite3_prepare_v2(db,"SELECT 1 FROM bookings WHERE room_id=? AND date=? AND slot=? AND status='approved'",-1,&st,NULL);
        sqlite3_bind_int(st,1,roomId);sqlite3_bind_text(st,2,date.c_str(),-1,SQLITE_STATIC);sqlite3_bind_int(st,3,sn);
        if(sqlite3_step(st)==SQLITE_ROW){sqlite3_finalize(st);closeDB(db);res.set_content(jfail("Room already booked"),"application/json");return;}
        sqlite3_finalize(st);
        sqlite3_prepare_v2(db,"INSERT INTO bookings(teacher_id,room_id,date,slot,status)VALUES(?,?,?,?,'approved')",-1,&st,NULL);
        sqlite3_bind_int(st,1,tid);sqlite3_bind_int(st,2,roomId);
        sqlite3_bind_text(st,3,date.c_str(),-1,SQLITE_STATIC);sqlite3_bind_int(st,4,sn);
        sqlite3_step(st); sqlite3_finalize(st); closeDB(db);
        undoStack.push(Booking(tid,roomId,date,sn));
        if(!roomHistories.count(roomId)) roomHistories[roomId]=new BookingHistory();
        roomHistories[roomId]->append(date+" slot "+std::to_string(sn));
        res.set_content(jok("Room booked successfully!"), "application/json");
    });

    server.Post("/undo_booking", [](const Request&, Response& res) {
        Booking last;
        if(!undoStack.pop(last)){res.set_content(jfail("Nothing to undo"),"application/json");return;}
        sqlite3* db=openDB();
        if(!db){res.set_content(jfail("DB error"),"application/json");return;}
        sqlite3_stmt* st=NULL;
        sqlite3_prepare_v2(db,"DELETE FROM bookings WHERE teacher_id=? AND room_id=? AND date=? AND slot=?",-1,&st,NULL);
        sqlite3_bind_int(st,1,last.teacher_id);sqlite3_bind_int(st,2,last.room_id);
        sqlite3_bind_text(st,3,last.date.c_str(),-1,SQLITE_STATIC);sqlite3_bind_int(st,4,last.slot);
        sqlite3_step(st);sqlite3_finalize(st);closeDB(db);
        res.set_content(jok("Undo successful"), "application/json");
    });

    server.Get("/history/([0-9]+)", [](const Request& req, Response& res) {
        int rid=atoi(req.matches[0].c_str());
        if(!roomHistories.count(rid)){res.set_content("{\"history\":[]}","application/json");return;}
        std::vector<std::string> hist=roomHistories[rid]->toList();
        std::string arr="[";
        for(int i=0;i<(int)hist.size();i++){if(i)arr+=","; arr+="\""+je(hist[i])+"\"";} arr+="]";
        res.set_content("{\"history\":"+arr+"}","application/json");
    });
    server.Get("/section-timetable", [](const Request& req, Response& res) {
        if (checkAuth(req) < 0) { res.redirect("/login"); return; }
        res.set_content(readFile("section_timetable.html"), "text/html");
    });

    server.Get("/sections", [](const Request&, Response& res) {
        res.set_content("[\"A1\",\"A2\",\"B1\",\"B2\",\"C1\"]", "application/json");
    });

    server.Get("/api/section_timetable/([A-Za-z0-9]+)", [](const Request& req, Response& res) {
        std::string sec = req.matches[0];
        sqlite3* db = openDB();
        if (!db) { res.set_content(jerr("DB error"),"application/json"); return; }
        std::map<int,std::string> roomMap;
        sqlite3_stmt* st = NULL;
        sqlite3_prepare_v2(db,"SELECT room_id, room_name FROM rooms",-1,&st,NULL);
        while(sqlite3_step(st)==SQLITE_ROW) roomMap[sqlite3_column_int(st,0)]=(const char*)sqlite3_column_text(st,1);
        sqlite3_finalize(st);
        const char* sql =
            "SELECT tt.day, tt.slot, tt.subject, tt.room_id, t.name "
            "FROM timetable tt JOIN teachers t ON t.teacher_id=tt.teacher_id "
            "WHERE tt.section=? "
            "ORDER BY CASE tt.day WHEN 'MON' THEN 1 WHEN 'TUE' THEN 2 WHEN 'WED' THEN 3 "
            "WHEN 'THU' THEN 4 WHEN 'FRI' THEN 5 WHEN 'SAT' THEN 6 END, CAST(tt.slot AS INTEGER)";
        sqlite3_prepare_v2(db,sql,-1,&st,NULL);
        sqlite3_bind_text(st,1,sec.c_str(),-1,SQLITE_STATIC);
        std::vector<std::string> rows;
        while(sqlite3_step(st)==SQLITE_ROW) {
            std::string day  = (const char*)sqlite3_column_text(st,0);
            int sn           = sqlite3_column_int(st,1);
            std::string subj = (const char*)sqlite3_column_text(st,2);
            int rid          = sqlite3_column_int(st,3);
            std::string tname= (const char*)sqlite3_column_text(st,4);
            std::string room = roomMap.count(rid)?roomMap[rid]:"TBD";
            std::string ts   = slotToTime.count(sn)?slotToTime[sn]:std::to_string(sn);
            rows.push_back("{\"day\":\""+day+"\",\"slot\":\""+ts+"\",\"slot_num\":"+std::to_string(sn)
                           +",\"subject\":\""+je(subj)+"\",\"room\":\""+je(room)+"\",\"teacher\":\""+je(tname)+"\"}");
        }
        sqlite3_finalize(st); closeDB(db);
        res.set_content(jarr(rows), "application/json");
    });

    std::cout << "========================================\n";

    std::cout << " Teacher Free Slot Finder — C++ Server \n";
    std::cout << " Open browser: http://localhost:8080   \n";
    std::cout << " Default password: pass123             \n";
    std::cout << " Press Ctrl+C to stop.                 \n";
    std::cout << "========================================\n";
    server.listen("0.0.0.0", 8080);
    return 0;
}

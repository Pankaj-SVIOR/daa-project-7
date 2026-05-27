#ifndef SESSION_H
#define SESSION_H
#include <string>
#include <map>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iomanip>

const std::string COOKIE_NAME = "fs_session";

std::map<std::string, int>sessionToTeacher;
std::map<std::string, std::string>sessionToName;

std::string generateToken() {
    srand((unsigned int)time(0) + rand());
    std::ostringstream ss;
    for (int i = 0; i < 8; i++)
        ss << std::hex << std::setw(4) << std::setfill('0') << (rand() % 0xFFFF);
    return ss.str();
}
std::string createSession(int teacherId, const std::string& name) {
    std::string token = generateToken();
    sessionToTeacher[token] = teacherId;
    sessionToName[token]    = name;
    return token;
}

int getSessionTeacherId(const std::string& token) {
    std::map<std::string,int>::iterator it = sessionToTeacher.find(token);
    if (it != sessionToTeacher.end()) return it->second;
    return -1;
}

std::string getSessionName(const std::string& token) {
    std::map<std::string,std::string>::iterator it = sessionToName.find(token);
    if (it != sessionToName.end()) return it->second;
    return "";
}

void destroySession(const std::string& token) {
    sessionToTeacher.erase(token);
    sessionToName.erase(token);
}

bool isLoggedIn(const std::string& token) {
    return sessionToTeacher.find(token) != sessionToTeacher.end();
}

#endif

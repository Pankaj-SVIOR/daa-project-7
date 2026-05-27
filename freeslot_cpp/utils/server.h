#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <windows.h>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <functional>

struct Request {
    std::string method;
    std::string path;
    std::string body;
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> cookies;
    std::vector<std::string> matches;
};

struct Response {
    int         status;
    std::string content;
    std::string content_type;
    std::vector<std::string> extra_headers;

    Response() : status(200) {}

    void set_content(const std::string& body, const std::string& ct) {
        content = body;
        content_type = ct;
    }

    void redirect(const std::string& url) {
        status = 302;
        extra_headers.push_back("Location: " + url);
        content = "";
        content_type = "text/html";
    }

    void set_cookie(const std::string& name, const std::string& value,
                    const std::string& extra = "") {
        std::string c = "Set-Cookie: " + name + "=" + value + "; Path=/";
        if (!extra.empty()) c += "; " + extra;
        extra_headers.push_back(c);
    }

    void clear_cookie(const std::string& name) {
        extra_headers.push_back("Set-Cookie: " + name + "=; Path=/; Max-Age=0");
    }
};

typedef std::function<void(const Request&, Response&)> Handler;

struct Route {
    std::string method;
    std::string pattern;
    Handler     handler;
};

class SimpleServer {
public:
    void Get(const std::string& pattern, Handler h) {
        routes.push_back({"GET", pattern, h});
    }

    void Post(const std::string& pattern, Handler h) {
        routes.push_back({"POST", pattern, h});
    }

    void listen(const std::string& host, int port) {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2,2), &wsa);

        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        int opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

        sockaddr_in addr;
        addr.sin_family      = AF_INET;
        addr.sin_port        = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;
        memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

        if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            std::cerr << "Bind failed on port " << port << std::endl;
            closesocket(sock); WSACleanup(); return;
        }

        ::listen(sock, 10);
        std::cout << "Server running at http://localhost:" << port << std::endl;

        while (true) {
            SOCKET client = accept(sock, NULL, NULL);
            if (client == INVALID_SOCKET) continue;
            handleClient(client);
            closesocket(client);
        }

        closesocket(sock);
        WSACleanup();
    }

private:
    std::vector<Route> routes;

    void handleClient(SOCKET sock) {
        std::string raw;
        char buf[8192];
        int bytes;

        DWORD timeout = 2000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

        while (true) {
            bytes = recv(sock, buf, sizeof(buf)-1, 0);
            if (bytes <= 0) break;
            buf[bytes] = '\0';
            raw += buf;
            if (raw.find("\r\n\r\n") != std::string::npos) {
                size_t hEnd = raw.find("\r\n\r\n");
                std::string hdrs = raw.substr(0, hEnd);
                int clen = 0;
                size_t clp = hdrs.find("Content-Length:");
                if (clp != std::string::npos) clen = atoi(hdrs.c_str() + clp + 15);
                std::string bod = raw.substr(hEnd + 4);
                if ((int)bod.size() >= clen) break;
            }
        }
        if (raw.empty()) return;

        Request req = parseRequest(raw);
        Response res;
        bool found = false;

        for (int i = 0; i < (int)routes.size(); i++) {
            if (routes[i].method != req.method) continue;
            std::vector<std::string> caps;
            if (matchPath(routes[i].pattern, req.path, caps)) {
                req.matches = caps;
                routes[i].handler(req, res);
                found = true;
                break;
            }
        }

        if (!found) {
            res.status = 404;
            res.set_content("<h1>404 Not Found: " + req.path + "</h1>", "text/html");
        }

        sendResponse(sock, res);
    }

    Request parseRequest(const std::string& raw) {
        Request req;
        size_t lineEnd = raw.find("\r\n");
        std::string firstLine = raw.substr(0, lineEnd);
        std::istringstream ss(firstLine);
        std::string fullPath, ver;
        ss >> req.method >> fullPath >> ver;

        size_t qPos = fullPath.find('?');
        if (qPos != std::string::npos) {
            req.path = fullPath.substr(0, qPos);
            parseQueryParams(fullPath.substr(qPos + 1), req.params);
        } else {
            req.path = fullPath;
        }

        size_t headerEnd = raw.find("\r\n\r\n");
        std::string headerBlock = raw.substr(lineEnd + 2, headerEnd - lineEnd - 2);
        parseCookies(headerBlock, req.cookies);

        if (headerEnd != std::string::npos)
            req.body = raw.substr(headerEnd + 4);

        return req;
    }

    void parseQueryParams(const std::string& qs, std::map<std::string,std::string>& params) {
        std::istringstream ss(qs);
        std::string token;
        while (std::getline(ss, token, '&')) {
            size_t eq = token.find('=');
            if (eq != std::string::npos)
                params[urlDecode(token.substr(0,eq))] = urlDecode(token.substr(eq+1));
        }
    }

    void parseCookies(const std::string& headers, std::map<std::string,std::string>& cookies) {
        size_t pos = headers.find("Cookie:");
        if (pos == std::string::npos) return;
        size_t start = pos + 7;
        size_t end   = headers.find("\r\n", start);
        std::string cookieLine = headers.substr(start, end - start);
        std::istringstream ss(cookieLine);
        std::string pair;
        while (std::getline(ss, pair, ';')) {
            while (!pair.empty() && pair[0] == ' ') pair = pair.substr(1);
            size_t eq = pair.find('=');
            if (eq != std::string::npos)
                cookies[pair.substr(0,eq)] = pair.substr(eq+1);
        }
    }

    std::string urlDecode(const std::string& s) {
        std::string r;
        for (int i = 0; i < (int)s.size(); i++) {
            if (s[i]=='%' && i+2 < (int)s.size()) {
                char h[3] = {s[i+1], s[i+2], 0};
                r += (char)strtol(h, NULL, 16);
                i += 2;
            } else if (s[i]=='+') r += ' ';
            else r += s[i];
        }
        return r;
    }

    bool matchPath(const std::string& pattern, const std::string& path,
                   std::vector<std::string>& captures) {
        captures.clear();
        if (pattern == path) return true;

        size_t paren = pattern.find('(');
        if (paren == std::string::npos) return false;

        std::string prefix = pattern.substr(0, paren);
        if (path.substr(0, prefix.size()) != prefix) return false;

        std::string rest = path.substr(prefix.size());
        size_t closeParen = pattern.find(')', paren);
        std::string suffix = (closeParen != std::string::npos) ? pattern.substr(closeParen+1) : "";

        std::string captured;
        if (suffix.empty()) {
            size_t sl = rest.find('/');
            captured = (sl == std::string::npos) ? rest : rest.substr(0, sl);
        } else {
            size_t sp = rest.find(suffix);
            if (sp == std::string::npos) return false;
            captured = rest.substr(0, sp);
        }
        if (captured.empty()) return false;
        captures.push_back(captured);
        return true;
    }

    void sendResponse(SOCKET sock, const Response& res) {
        std::string statusText = "OK";
        if      (res.status == 302) statusText = "Found";
        else if (res.status == 400) statusText = "Bad Request";
        else if (res.status == 401) statusText = "Unauthorized";
        else if (res.status == 404) statusText = "Not Found";
        else if (res.status == 500) statusText = "Internal Server Error";

        std::ostringstream out;
        out << "HTTP/1.1 " << res.status << " " << statusText << "\r\n";
        out << "Content-Type: " << res.content_type << "; charset=utf-8\r\n";
        out << "Content-Length: " << res.content.size() << "\r\n";
        out << "Access-Control-Allow-Origin: *\r\n";
        out << "Connection: close\r\n";
        for (int i = 0; i < (int)res.extra_headers.size(); i++)
            out << res.extra_headers[i] << "\r\n";
        out << "\r\n" << res.content;

        std::string str = out.str();
        send(sock, str.c_str(), (int)str.size(), 0);
    }
};

std::string getParam(const Request& req, const std::string& name) {
    std::map<std::string,std::string>::const_iterator it = req.params.find(name);
    return (it != req.params.end()) ? it->second : "";
}

std::string getCookie(const Request& req, const std::string& name) {
    std::map<std::string,std::string>::const_iterator it = req.cookies.find(name);
    return (it != req.cookies.end()) ? it->second : "";
}

#endif

#ifndef DSA_H
#define DSA_H

#include <string>
#include <vector>

struct Booking {
    int teacher_id;
    int room_id;
    std::string date;
    int slot;

    Booking() : teacher_id(0), room_id(0), slot(0) {}
    Booking(int tid, int rid, std::string d, int s)
        : teacher_id(tid), room_id(rid), date(d), slot(s) {}
};

class BookingStack {
public:
    void push(Booking b) {
        data.push_back(b);
    }

    bool pop(Booking& result) {
        if (data.empty()) return false;
        result = data.back();
        data.pop_back();
        return true;
    }

    bool isEmpty() {
        return data.empty();
    }

private:
    std::vector<Booking> data;
};

class BookingQueue {
public:
    void enqueue(Booking b) {
        data.push_back(b);
    }

    bool dequeue(Booking& result) {
        if (data.empty()) return false;
        result = data.front();
        data.erase(data.begin());
        return true;
    }

    bool isEmpty() {
        return data.empty();
    }

private:
    std::vector<Booking> data;
};

struct HistoryNode {
    std::string info;
    HistoryNode* next;
    HistoryNode(std::string val) : info(val), next(NULL) {}
};

class BookingHistory {
public:
    BookingHistory() : head(NULL) {}

    void append(std::string info) {
        HistoryNode* newNode = new HistoryNode(info);
        if (head == NULL) { head = newNode; return; }
        HistoryNode* curr = head;
        while (curr->next != NULL) curr = curr->next;
        curr->next = newNode;
    }

    std::vector<std::string> toList() {
        std::vector<std::string> result;
        HistoryNode* curr = head;
        while (curr != NULL) {
            result.push_back(curr->info);
            curr = curr->next;
        }
        return result;
    }

    ~BookingHistory() {
        HistoryNode* curr = head;
        while (curr != NULL) {
            HistoryNode* next = curr->next;
            delete curr;
            curr = next;
        }
    }

private:
    HistoryNode* head;
};

class SimpleHashTable {
public:
    SimpleHashTable(int size) : tableSize(size) {
        buckets.resize(size);
    }

    void set(std::string key, int value) {
        int index = hashFunc(key);
        for (int i = 0; i < (int)buckets[index].size(); i++) {
            if (buckets[index][i].first == key) {
                buckets[index][i].second = value;
                return;
            }
        }
        buckets[index].push_back(std::make_pair(key, value));
    }

    int get(std::string key) {
        int index = hashFunc(key);
        for (int i = 0; i < (int)buckets[index].size(); i++) {
            if (buckets[index][i].first == key)
                return buckets[index][i].second;
        }
        return -1;
    }

private:
    int tableSize;
    std::vector<std::vector<std::pair<std::string, int>>> buckets;

    int hashFunc(const std::string& key) {
        int sum = 0;
        for (int i = 0; i < (int)key.size(); i++) sum += (int)key[i];
        return sum % tableSize;
    }
};

#endif

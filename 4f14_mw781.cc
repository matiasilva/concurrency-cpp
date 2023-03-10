#include <iostream>
#include <queue>
#include <stack>
#include <string>
#include <mutex>
#include <thread>
#include <chrono>

//  data type for queue
struct Item {
    std::string str;
    int val;
};

class Queue {
public:
    void enqueue(const Item& item) {
        std::lock_guard<std::mutex> lock(mutex); // RAII
        items.push(item);
    }

    void dequeue() {
        std::lock_guard<std::mutex> lock(mutex);
        if (items.empty()) {
            std::cout << "Empty queue" << std::endl;
        } else {
            items.pop();
        }
    }

    bool isEmpty() const {
        std::lock_guard<std::mutex> lock(mutex);
        return items.empty();
    }

    // add up all the values by using a temp queue
    int sum() const {
        std::lock_guard<std::mutex> lock(mutex);
        int sum = 0;
        std::queue<Item> temp = items;
        while (!temp.empty()) {
            sum += temp.front().val;
            temp.pop();
        }
        return sum;
    }

    // reverse items using a stack as temp storage
    void reverse() {
        std::lock_guard<std::mutex> lock(mutex);

        std::stack<Item> s;
        while (!items.empty()) {
            s.push(items.front());
            items.pop();
        }
        while (!s.empty()) {
            items.push(s.top());
            s.pop();
        }
    }

    // print the items in a specific way
    void printItems() {
        std::lock_guard<std::mutex> lock(mutex);
        
        std::queue<Item> temp = items;
        while(!temp.empty()){
            Item item = temp.front();
            int pos = items.size() - temp.size();
            printf("pos: %2d | str: %5s | val: %3d   ", pos, item.str.c_str(), item.val);
            std::cout << std::endl;
            temp.pop();
        }
        std::cout << std::endl;
    }

    // use the random num gen to pick an "index" and remove it
    void removeRandomItem() {
        std::lock_guard<std::mutex> lock(mutex);

        int queueLength = items.size();
        int toRemove = rand() % queueLength;
        std::queue<Item> temp;
        for (int i = 0; i < queueLength; i++) {
            if(i != toRemove) {
                temp.push(items.front());
            }
            items.pop();
        }
        items = temp;
    }


private:
    std::queue<Item> items;
    mutable std::mutex mutex;
};

void populateQueue(Queue& q) {
    // can optionally seed number gen
    // std::srand(std::time(nullptr));

    for (int i = 0; i < 55; i++) {
        int strlen = rand() % 4 + 2; // random string length between 2 and 5 chars inclusive
        std::string nextStr = "";
        for (int j = 0; j < strlen; j++) {
            char c = 'a' + rand() % 26; // random lowercase letter
            nextStr += c;
        }
        int nextVal = rand() % 256; // random integer between 0 and 255 inclusive
        Item item = {nextStr, nextVal};
        q.enqueue(item);
    }
}

// workers for concurrency
void reverseQueueWorker(Queue& q) {
    while (!q.isEmpty()) {
        q.reverse();
        std::cout << "sum of values in reversed queue: " << q.sum() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void printQueueWorker(Queue& q) {
    while (!q.isEmpty()) {
        q.printItems();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void removerWorker(Queue& q) {
    while (!q.isEmpty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        q.removeRandomItem();
    }

}

int main() {
    Queue q;
    populateQueue(q);

    std::thread reverserThread(reverseQueueWorker, std::ref(q));
    std::thread printQueueThread(printQueueWorker, std::ref(q));
    std::thread removerThread(removerWorker, std::ref(q));

    // main thread free!

    removerThread.join();
    reverserThread.join();
    printQueueThread.join();
    // once queue empty we'll come back here

    return 0;
}

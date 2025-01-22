#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <algorithm>
#include <queue>
#include <condition_variable>

const int NUM_CONSUMERS = 4;

using namespace std;

mutex mtx;

template <typename T>
class blockingQueue{
  private:
    queue<T> bqueue;
    mutex bq_mtx;
    condition_variable cv;
  public:
    void push(T item) {
        unique_lock<mutex> lock(bq_mtx);
        bqueue.push(item);
        cv.notify_one();
    } 
    T pop() {
        unique_lock<mutex> lock(bq_mtx);
        cv.wait(lock, [this]() {return !bqueue.empty(); });
        T item = bqueue.front();
        bqueue.pop();
        return item;
    }
    bool empty() {
        if (bqueue.empty()) {
            return true;
        } else {
            return false;
        }
    }
};

void producer(const string filePath, blockingQueue<vector<string>>& b_queue) {
    {
        lock_guard<mutex> lock(mtx);
        cout << "Producer started" << endl;
    }

    vector<string> data;
    ifstream inputFile(filePath);
    if (inputFile.is_open()) {
        string line;
        while (getline(inputFile, line)) {
            data.push_back(line);
        }
        inputFile.close();
    } else {
        lock_guard<mutex> lock(mtx);
        cerr << "Failed to open the file\n";
        return;
    }

    int chunk_size = ((data.size() + NUM_CONSUMERS - 1) / NUM_CONSUMERS);
    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        int start = i * chunk_size;
        int end = min(start + chunk_size, int(data.size()));
        vector<string> chunk(data.begin() + start, data.begin() + end);
        {
            lock_guard<mutex> lock(mtx);
            cout << "Producer added a chunk to the blocking queue" << endl;
        }
        b_queue.push(chunk);
    }
    
}

void consumer(blockingQueue<vector<string>>& b_queue) {
    if (!b_queue.empty()) {
        for (string line : b_queue.pop()) {
            for (int i = 0; i < line.length(); i++) {
                line[i] = toupper(line[i]);
            }
            {
                lock_guard<mutex> lock(mtx);
                cout << "Consumer" << "[" << this_thread::get_id() << "]" << "  " << line << endl;
            }
        }
    }
}

int main() {
    const string inputFile = "./input.txt";
    blockingQueue<vector<string>> b_queue;
    producer(inputFile, b_queue);
    vector<thread> consumers;
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumers.emplace_back(consumer, ref(b_queue));
    }

    for (auto& consumer : consumers) {
        if (consumer.joinable()) {
            consumer.join();
        }
    }

    cout << "\033[32mProgram complete.\033[0m" << endl;

    return 0;
}

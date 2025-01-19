#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <thread>
#include <mutex>
#include <algorithm>

using namespace std;

mutex mtx;

void processTextChunk(const vector<string> &chunk) {
    for (string line : chunk) {
        for (int i = 0; i < line.length(); i++) {
            line[i] = toupper(line[i]);
        }

        lock_guard<mutex> lock(mtx);
        cout << "Thread" << "[" << this_thread::get_id() << "]" << "  " << line << endl;
    }
}

int main() {
    vector<string> data;
    ifstream inputFile("./input.txt");
    if (inputFile.is_open()) {
        string line;
        while (getline(inputFile, line)) {
            data.push_back(line);
        }
        inputFile.close();
    } else {
        cout << "Failed to open the file" << endl;
        return 0;
    }

    const int num_threads = 4;
    int chunk_size = ((data.size() + num_threads - 1) / num_threads);

    vector<thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        int start = i * chunk_size;
        int end = min(start + chunk_size, int(data.size()));
        vector<string> chunk(data.begin() + start, data.begin() + end);
        threads.emplace_back(processTextChunk, chunk);
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    cout << "Program complete." << endl;

    return 0;
}
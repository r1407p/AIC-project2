#include <iostream>
#include <thread>

// Function to be executed by the thread
void threadFunction(int id) {
    std::cout << "Thread " << id << " is running\n";
}

int main() {
    const int numThreads = 5;
    std::thread threads[numThreads];

    // Create and start the threads
    for (int i = 0; i < numThreads; ++i) {
        threads[i] = std::thread(threadFunction, i);
    }

    // Join the threads to the main thread
    for (int i = 0; i < numThreads; ++i) {
        threads[i].join();
    }

    std::cout << "All threads have completed\n";

    return 0;
}

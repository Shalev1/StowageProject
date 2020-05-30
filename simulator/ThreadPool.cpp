#include "ThreadPool.h"

ThreadPool::ThreadPool(int numOfThreads) : numOfThreads(numOfThreads) {
    workers.reserve(numOfThreads);
}

void ThreadPool::getTask(Simulation sim) {
    tasks.emplace_back(std::move(sim));
}

void ThreadPool::start() {
    for(int i = 0; i < numOfThreads; i++){
        workers.emplace_back([this] {workerFunc();});
    }
}

void ThreadPool::workerFunc() {
    while(!finished || nextTaskIndex.load() != (int)tasks.size()){
        int currTaskIndex = nextTaskIndex.load();
        if(currTaskIndex >= (int)tasks.size()){
            std::this_thread::yield(); // No task right now, yield and check again once get cpu again
            continue;
        }
        if(nextTaskIndex.compare_exchange_strong(currTaskIndex, currTaskIndex + 1)) {
            tasks[currTaskIndex].runSimulation();
        }
    }
}

void ThreadPool::finish() {
    finished = true;
    for(auto& worker : workers) {
        worker.join();
    }
}
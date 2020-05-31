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
    while(!finished || !tasks.empty()){
        if(tasks.empty()){
            std::this_thread::yield(); // No task right now, yield and check once get cpu again
            continue;
        }
        {
            std::lock_guard guard(tasks);

        }
    }
}

void ThreadPool::finish() {
    finished = true;
    for(auto& worker : workers) {
        worker.join();
    }
}
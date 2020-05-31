/**
 * Created by Tomer Yoeli
 * The thread pool class, handle all of the threads actions.
 * In charge of collecting <algorithm, travel> tasks and run them in multiple threads.
 */

#ifndef SHIPPROJECT_THREADPOOL_H
#define SHIPPROJECT_THREADPOOL_H

#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
//TODO: include simulation after merge

using std::thread;
using std::vector;
using std::atomic_int;
using std::mutex;

class Simulation{ // TODO: replace with real simulation
public:
    void runSimulation(){}
};

class ThreadPool {
private:
    int numOfThreads;
    vector<thread> workers;
    vector<Simulation> tasks;
    mutex tasksMutex;
    bool finished = false;

public:
    ThreadPool(int numOfThreads);

    /**
     * Get new task (Simulation object)
     */
    void getTask(Simulation sim);

    /**
     * Start the threadPool, create all of the threads
     */
    void start();

    /**
     * A single thread function. get task from the tasks vector and run it, until no more tasks left
     */
    void workerFunc();

    /**
     * Finish the threadPool. no more tasks will be given. also wait until all of the threads will done (using join)
     */
    void finish();

};

#endif //SHIPPROJECT_THREADPOOL_H

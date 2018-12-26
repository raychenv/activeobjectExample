//
// This version of the ActiveObject illustrates more advanced
// features such as calling public methods with references,
// calling methods with return values, and accessing the object's
// internal state.
//
//#include "stdafx.h"
#include <cassert>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

using Operation = std::function<void()>;

class DispatchQueue
{
    std::mutex qlock;
    std::queue<Operation> ops_queue;
    std::condition_variable empty;

 public:
    void put(Operation op)
    {
        std::lock_guard<std::mutex> guard(qlock);
        ops_queue.push(op);
        empty.notify_one();
    }

    Operation take()
    {
        std::unique_lock<std::mutex> lock(qlock);
        empty.wait(lock, [&] { return !ops_queue.empty(); });

        Operation op = ops_queue.front();
        ops_queue.pop();
        return op;
    }
};

class BecomeActiveObject
{
 private:
    double val;
    DispatchQueue dispatchQueue;
    std::atomic<bool> done;
    std::thread runnable;

 public:
    BecomeActiveObject() : val(0), done(false)
    {
        runnable = std::thread([=] { run(); });
    }
    ~BecomeActiveObject()
    {
        // Schedule a No-Op runnable to flush the dispatch queue
        dispatchQueue.put([&]() { done = true; });
        runnable.join();
    }

    double getVal() { return val; }

    void run()
    {
        while (!done)
        {
            dispatchQueue.take()();
        }
    }

    // This method returns a value, so it is blocking on the future result
    int doSomething()
    {
        std::promise<int> return_val;
        auto runnable = [&]() {
            int ret = 999;
            return_val.set_value(ret);
        };

        dispatchQueue.put(runnable);
        return return_val.get_future().get();
    }

    // This method accesses the object's internal state from within the closure
    // Because the access to the ActiveObject is serialized, we can safely access
    // the object's internal state.
    void doSomethingElse()
    {
        dispatchQueue.put(([this]() { this->val = 2.0; }));
    }

    // This method takes two params which we want to reference in the closure
    void doSomethingWithParams(int a, int b)
    {
        // This lambda function code will execute later from the context of a different thread,
        // but the integers {a, b} are bound now.
        // This is a beautiful way to write clear code
        dispatchQueue.put(([a, b]() {
            std::cout << "this is the internal implementation of doSomethingWithParams(";
            std::cout << a << "," << b << ")\n";
        }));
    }

    // This method takes two reference parameters so it must execute blocking
    void doSomethingWithReferenceParams(int& a, int& b)
    {
        std::promise<void> return_val;
        // This lambda function code will execute later from the context of a different thread,
        // but the integers {a, b} are bound now.
        // This is a beautiful way to write clear code
        dispatchQueue.put(([&a, &b, &return_val]() {
            std::cout << "this is the internal implementation of doSomethingWithReferenceParams(";
            std::cout << a << "," << b << ")\n";
            a = 1234;
            b = 5678;
            return_val.set_value();
        }));

        return_val.get_future().get();
    }
};


int main(int argc, char** argv)
{
    BecomeActiveObject active;
    int i = active.doSomething();
    assert(i = 999);
    // mix things up by starting another thread
    std::thread t1(&BecomeActiveObject::doSomethingElse, &active);
    active.doSomethingWithParams(5, 7);
    int a = 1, b = 2;
    active.doSomethingWithReferenceParams(a, b);
    assert(a == 1234 && b == 5678);
    t1.join();
    assert(active.getVal() == 2.0);
    return 0;
}

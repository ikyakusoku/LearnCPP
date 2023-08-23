#pragma once
#include"SafeQueue.h"
#include<mutex>
#include<functional>
#include<thread>
#include<future>

//线程池是在一开始创建多个线程，使用过程中反复的取利用这些线程，从而达到省去线程频繁创建和销毁的开销的作用，
//(感觉所有池的思想都是一样的，线程池、对象池、内存池...)
//先试想一下实现线程池，我们需要一个队列来管理可调用对象，然后一个线程数组来管理线程。线程池的具体功能就是
//在存在可调用对象的时候去用一个线程执行。但线程对象(thread)在创建的时候就需要绑定一个可调用对象了，如何让
//他们先创建好后，再去执行不同的可调用对象呢？这里的思路是所有线程先使用一个自定义的可调用对象，这个callable
//内部再去访问callable队列，在队列不为空的时候取出并执行。不过，这又有个问题，在队列为空的时候，这个可调用
//对象如何一直等待新的函数放进队列，并且免去执行的开销呢，如果只是循环等待，则相当于线程一直执行，这样的开销
//如何避免？这里是使用条件变量进行阻塞和通知。
//线程池的实现思路差不多就是这样。

class ThreadPool {
private:
    // class ThreadTask{
    // public:
    //     ThreadTask():thrd(){}

    //     void operator()()
    // private:
    //     std::thread thrd;
    // };
    void threadTask() {
        while (!shutDownFlag) {
            std::function<void()> func;
            {
                //判断是否队列是否为空必须与弹出一起执行，作为原子操作 x（没必要，线程安全的队列可以支持这一点，
                //如果两个线程都尝试tryPop()成功的那一个才会执行）
                std::unique_lock<std::mutex> lock(mMutex);
                notEmptyCV.wait(lock, [this]() {
                    //加上shutDownFlag的判断，以防有线程还没进入wait无法接受到nortify，或者接收了但队列为空无法退出阻塞
                    return !callableObjects.IsEmpty()||shutDownFlag;
                });
            }
            if (callableObjects.TryPop(func))
            {
                func();
            }
        }
    }

public:
    //ThreadPool(size_t threadNum = std::thread::hardware_concurrency()) :threads(threadNum, std::thread(&ThreadPool::threadTask, this))会报错，vector似乎
    //没有右值引用版本的接口如vector(size_t count,Ty&& val)的构造函数，只有const左值引用版本的，所以完美转发并不正确。而thread并没有拷贝构造所以报错.
    //.........................
    //改为ThreadPool(size_t threadNum = std::thread::hardware_concurrency()) :threads(threadNum,std::move(std::thread(&ThreadPool::threadTask, this)))后依然报错
    //显然这个接口是不打算让你调用移动构造的。
    //........................
    //没办法你只能在函数体中调用移动赋值来修改thread对象。
    ThreadPool(size_t threadNum = std::thread::hardware_concurrency()) :threads(threadNum) {
        for (auto& thrd : threads) {
            thrd = std::thread(&ThreadPool::threadTask, this);
        }
        std::cout << "number of threads:" << threadNum << std::endl;
    };
    ~ThreadPool() {
        ShutDown();
    }

    ThreadPool(ThreadPool&&) = delete;

    template<typename F, typename...Args>
    auto Submit(F&& func, Args&&...args) {
        //std::packaged_task<F&&> task(std::forward<F>(func), std::forward<Args>(args)...);
        auto sp = std::make_shared<std::packaged_task<decltype(func(args...))()>>(std::bind(std::forward<F>(func), std::forward<Args>(args)...));
        //利用lambda表达式的捕获列表去存储共享指针，使指针声明周期和lambda表达式一致，从而使得指向的package_task对象不会销毁。
        callableObjects.Push([sp]() {
            (*sp)();
        });
        notEmptyCV.notify_one();
        return sp->get_future();
    };

    void ShutDown() {
        shutDownFlag = true;
        notEmptyCV.notify_all();
        for (auto& thrd : threads) {
            if (thrd.joinable())
            {
                thrd.join();
            }
        }
    }

private:
    bool shutDownFlag = false;

    mutable std::mutex mMutex;
    std::condition_variable notEmptyCV;

    SafeQueue<std::function<void()>> callableObjects;
    std::vector<std::thread> threads;
};
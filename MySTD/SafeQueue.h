#pragma once
#include<queue>
#include<mutex>
#include<memory>

template<typename Type>
class SafeQueue
{
    //using namespace std;
public:
    SafeQueue() = default;
    ~SafeQueue() = default;

    SafeQueue(const SafeQueue& other) {
        //拷贝构造函数只用上锁拷贝的对象，不用上锁自己，因为还未构造完成，外部也用不了这里的接口
        //(不过想到，如果说在这块内存上完成了成员初始化但还没有执行函数体内的代码时，其他线程访问了这块内存是不是就出问题了呢？)
        //(不对，不应该是这里有问题而是暴露这块内存的代码块有问题吧)
        std::lock_guard<std::mutex> lock(other.mMutex);
        mQueue = other.mQueue;
    }

    SafeQueue& operator=(const SafeQueue& other) = delete;

    //添加一个元素进入队列
    template<typename Elem>
    void Push(Elem&& elemnt) {
        std::lock_guard<std::mutex> lock(mMutex);
        mQueue.emplace(std::forward<Elem>(elemnt));
        notEmptyCV.notify_one();
    }

    //弹出一个元素，失败不会等待（引用版本）
    bool TryPop(Type& receiver) {
        std::lock_guard<std::mutex> lock(mMutex);
        if (mQueue.empty())
        {
            return false;
        }
        receiver = std::move(mQueue.front());
        mQueue.pop();
        return true;
    }

    //弹出一个元素，失败不会等待（智能指针版）
    std::shared_ptr<Type> TryPop() {
        std::lock_guard<std::mutex> lock(mMutex);
        if (mQueue.empty())
        {
            //std::shared_ptr<Type>()也可以它也会将内部指针初始化为nullptr;
            //make_shared<>()会将内部指针初始化为指向一块值初始化的内存
            return std::shared_ptr<Type>(nullptr);
        }
        std::shared_ptr<Type> sp = std::make_shared<Type>(std::move(mQueue.front()));
        mQueue.pop();
        return sp;
    }

    //等待队列不为空弹出(引用版)
    void WaitAndPop(Type& receiver) {
        std::unique_lock<std::mutex> lock(mMutex);
        notEmptyCV.wait(lock, []() {
            return !mQueue.empty();
            });
        receiver = std::move(mQueue.front());
        mQueue.pop();
    }

    //等待队列不为空弹出(智能指针版)
    std::shared_ptr<Type> WaitAndPop() {
        std::unique_lock<std::mutex> lock(mMutex);
        notEmptyCV.wait(lock, []() {
            return !mQueue.empty();
            });
        std::shared_ptr<Type> sp = std::make_shared<Type>(std::move(mQueue.front()));
        mQueue.pop();
        return sp;
    }

    //有必要对empty()和size()上锁吗
    bool IsEmpty() const {
        std::lock_guard<std::mutex> lock(mMutex);
        return mQueue.empty();
    };

    size_t Size() const {
        std::lock_guard<std::mutex> lock(mMutex);
        return mQueue.size();
    };



private:
    std::queue<Type> mQueue;
    mutable std::mutex mMutex;
    std::condition_variable notEmptyCV;
};



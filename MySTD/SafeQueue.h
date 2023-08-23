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
        //�������캯��ֻ�����������Ķ��󣬲��������Լ�����Ϊ��δ������ɣ��ⲿҲ�ò�������Ľӿ�
        //(�����뵽�����˵������ڴ�������˳�Ա��ʼ������û��ִ�к������ڵĴ���ʱ�������̷߳���������ڴ��ǲ��Ǿͳ��������أ�)
        //(���ԣ���Ӧ����������������Ǳ�¶����ڴ�Ĵ�����������)
        std::lock_guard<std::mutex> lock(other.mMutex);
        mQueue = other.mQueue;
    }

    SafeQueue& operator=(const SafeQueue& other) = delete;

    //���һ��Ԫ�ؽ������
    template<typename Elem>
    void Push(Elem&& elemnt) {
        std::lock_guard<std::mutex> lock(mMutex);
        mQueue.emplace(std::forward<Elem>(elemnt));
        notEmptyCV.notify_one();
    }

    //����һ��Ԫ�أ�ʧ�ܲ���ȴ������ð汾��
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

    //����һ��Ԫ�أ�ʧ�ܲ���ȴ�������ָ��棩
    std::shared_ptr<Type> TryPop() {
        std::lock_guard<std::mutex> lock(mMutex);
        if (mQueue.empty())
        {
            //std::shared_ptr<Type>()Ҳ������Ҳ�Ὣ�ڲ�ָ���ʼ��Ϊnullptr;
            //make_shared<>()�Ὣ�ڲ�ָ���ʼ��Ϊָ��һ��ֵ��ʼ�����ڴ�
            return std::shared_ptr<Type>(nullptr);
        }
        std::shared_ptr<Type> sp = std::make_shared<Type>(std::move(mQueue.front()));
        mQueue.pop();
        return sp;
    }

    //�ȴ����в�Ϊ�յ���(���ð�)
    void WaitAndPop(Type& receiver) {
        std::unique_lock<std::mutex> lock(mMutex);
        notEmptyCV.wait(lock, []() {
            return !mQueue.empty();
            });
        receiver = std::move(mQueue.front());
        mQueue.pop();
    }

    //�ȴ����в�Ϊ�յ���(����ָ���)
    std::shared_ptr<Type> WaitAndPop() {
        std::unique_lock<std::mutex> lock(mMutex);
        notEmptyCV.wait(lock, []() {
            return !mQueue.empty();
            });
        std::shared_ptr<Type> sp = std::make_shared<Type>(std::move(mQueue.front()));
        mQueue.pop();
        return sp;
    }

    //�б�Ҫ��empty()��size()������
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



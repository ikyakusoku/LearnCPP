#pragma once
#include"SafeQueue.h"
#include<mutex>
#include<functional>
#include<thread>
#include<future>

//�̳߳�����һ��ʼ��������̣߳�ʹ�ù����з�����ȡ������Щ�̣߳��Ӷ��ﵽʡȥ�߳�Ƶ�����������ٵĿ��������ã�
//(�о����гص�˼�붼��һ���ģ��̳߳ء�����ء��ڴ��...)
//������һ��ʵ���̳߳أ�������Ҫһ������������ɵ��ö���Ȼ��һ���߳������������̡߳��̳߳صľ��幦�ܾ���
//�ڴ��ڿɵ��ö����ʱ��ȥ��һ���߳�ִ�С����̶߳���(thread)�ڴ�����ʱ�����Ҫ��һ���ɵ��ö����ˣ������
//�����ȴ����ú���ȥִ�в�ͬ�Ŀɵ��ö����أ������˼·�������߳���ʹ��һ���Զ���Ŀɵ��ö������callable
//�ڲ���ȥ����callable���У��ڶ��в�Ϊ�յ�ʱ��ȡ����ִ�С������������и����⣬�ڶ���Ϊ�յ�ʱ������ɵ���
//�������һֱ�ȴ��µĺ����Ž����У�������ȥִ�еĿ����أ����ֻ��ѭ���ȴ������൱���߳�һֱִ�У������Ŀ���
//��α��⣿������ʹ��������������������֪ͨ��
//�̳߳ص�ʵ��˼·������������

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
                //�ж��Ƿ�����Ƿ�Ϊ�ձ����뵯��һ��ִ�У���Ϊԭ�Ӳ��� x��û��Ҫ���̰߳�ȫ�Ķ��п���֧����һ�㣬
                //��������̶߳�����tryPop()�ɹ�����һ���Ż�ִ�У�
                std::unique_lock<std::mutex> lock(mMutex);
                notEmptyCV.wait(lock, [this]() {
                    //����shutDownFlag���жϣ��Է����̻߳�û����wait�޷����ܵ�nortify�����߽����˵�����Ϊ���޷��˳�����
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
    //ThreadPool(size_t threadNum = std::thread::hardware_concurrency()) :threads(threadNum, std::thread(&ThreadPool::threadTask, this))�ᱨ��vector�ƺ�
    //û����ֵ���ð汾�Ľӿ���vector(size_t count,Ty&& val)�Ĺ��캯����ֻ��const��ֵ���ð汾�ģ���������ת��������ȷ����thread��û�п����������Ա���.
    //.........................
    //��ΪThreadPool(size_t threadNum = std::thread::hardware_concurrency()) :threads(threadNum,std::move(std::thread(&ThreadPool::threadTask, this)))����Ȼ����
    //��Ȼ����ӿ��ǲ�������������ƶ�����ġ�
    //........................
    //û�취��ֻ���ں������е����ƶ���ֵ���޸�thread����
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
        //����lambda���ʽ�Ĳ����б�ȥ�洢����ָ�룬ʹָ���������ں�lambda���ʽһ�£��Ӷ�ʹ��ָ���package_task���󲻻����١�
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
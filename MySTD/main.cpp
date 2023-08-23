#include<iostream>
#include"ThreadPool.h"
#include<atomic>
#include<future>
#include<mutex>
std::atomic<int> sum = 0;
std::mutex mtx;

int add(int a, int b) {
	int sumVal=0;
	for (int i = a; i < b; i++) {
		sumVal += i;
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(80));
	mtx.lock();
	std::cout << "runing thread " << std::this_thread::get_id()<<" sumval:"<<sumVal << std::endl;
	mtx.unlock();
	sum += sumVal;
	return sumVal;
}



int main() {
	ThreadPool thrdpool;
	const int length = 100;
	std::vector<std::future<int>> v;
	for (int i = 0; i < 1000; i+=length) {
		v.push_back(thrdpool.Submit(add, i, i + length));
	}
	for (auto& vi : v)
	{
		vi.wait();
		mtx.lock();
		std::cout << "value:" << vi.get() << std::endl;
		mtx.unlock();
	}
	mtx.lock();
	std::cout << "sum:"<< sum << std::endl;
	mtx.unlock();

	int judge=0;
	for (int i = 0; i < 1000; i++)
	{
		judge += i;
	}
	mtx.lock();
	std::cout << "judge:" << judge << std::endl;
	mtx.unlock();
}
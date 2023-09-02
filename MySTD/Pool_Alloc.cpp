#include "Pool_Alloc.h"


char* PoolAlloc::start_free = nullptr;
char* PoolAlloc::end_free = nullptr;
size_t PoolAlloc::heap_size = 0;//这是？？
PoolAlloc::obj* PoolAlloc::free_list[PoolAlloc::NFREELISTS]{};

void* PoolAlloc::refill(size_t bytes)
{
	size_t nobjs = NOBJS;//表示实际申请了多少块
	//首先从池中去一块内存
	char* chunk = static_cast<char*>(chunk_alloc(bytes,nobjs));
	if (nobjs == 1)
	{
		return chunk;
	}
	//取出一块内存
	size_t index = FreeList_Index(bytes);
	obj* result = (obj*)chunk;
	free_list[index] = (obj*)(chunk+bytes);
	//剩下的内存切块
	obj* current_obj = nullptr;//当前块
	obj* next_obj = free_list[index];//下一块
	for (int i = nobjs - 1;; i--) {
		current_obj = next_obj;
		next_obj = (obj*)((char*)next_obj + bytes);
		if (i == 1) {
			current_obj->next = nullptr;
			break;
		}
		else {
			current_obj->next = next_obj;
		}
	}
	return result;
}

void* PoolAlloc::chunk_alloc(size_t bytes, size_t& nobjs)
{
	//根据需要申请的内存与战备池中的内存作比较，判断是否重新申请
	size_t totalSize = bytes * NOBJS;
	size_t leftSize = end_free - start_free;

	if (totalSize <= leftSize)
	{
		//剩余战备池内存足够，就分配出去totalSize
		auto result=start_free;
		start_free += totalSize;
		return result;
	}
	else if (bytes <= leftSize) {
		//剩余战备池内存只足大于1不到NOBJS个，那么取出一块，剩下的不管。
		nobjs = leftSize / bytes;
		totalSize = nobjs * bytes;
		auto result = start_free;
		start_free = start_free + totalSize;
		return result;
	}
	else {
		//剩余战备池内存不够一个了
		
		//如果有剩余的，先把剩下的内存挂到对应的freelist下面
		if (leftSize > 0) {
			auto index=FreeList_Index(leftSize);
			((obj*)start_free)->next = free_list[index];
			free_list[index] = (obj*)start_free;
		}
		//然后申请内存大小为2倍
		size_t bytes_to_get = 2 * totalSize + RoundUp(heap_size >> 4);//roundup()这里加的到底是什么阿
		start_free = (char*)malloc(bytes_to_get);
		//参考文章和代码中，判断start_free不先置为空，那如果我战备池有残余，但内存申请失败了呢？可见malloc估计是分配失败会返回空指针。
		if (!start_free) {
			//如果申请失败，没内存了，只好在之前已经分配的内存中拿一块出来，作为战备池
			//这里参考的文章以及TinySTL的写法有点问题，应该向后面那些挂着更大内存块的freelist中找内存，而不是整个遍历，
			//如果我找到的是比bytes小的，再次chunk_alloc()，还是会回到这里的窘境。等于白做了
			for (size_t i = bytes; i <= MAXBYTES; i += ALIGN) {
				size_t index = FreeList_Index(i);
				obj* curList = free_list[index];
				if (curList != nullptr) {
					free_list[index]=curList->next;
					start_free = (char*)curList;
					end_free = start_free + i;
					return chunk_alloc(bytes, nobjs);
				}
			}
			//这里参考文章写的也有问题，没有抛出异常也没有对应的返回，又是死循环，不断递归.我这里简单处理成立返回空指针，至少不会不停递归。
			end_free = 0;
			return nullptr;
		}
		//如果申请成功就直接返回，让refill切割
		heap_size += bytes_to_get;
		end_free = start_free + bytes_to_get;
		return chunk_alloc(bytes,nobjs);//这递归调用一下处理一个start_free
	}
}

void* PoolAlloc::allocate(size_t bytes)
{
	//首先判断申请的大小，超出范围就不负责了(交给malloc)
	if (bytes > 128)
	{
		return malloc(bytes);
	}
	//先看一下对齐之后，分配多大内存，并挂在哪个freelist上
	size_t index = FreeList_Index(bytes);
	obj* listHead = free_list[index];
	if (listHead != nullptr)
	{
		free_list[index] = listHead->next;
		return listHead;
	}
	return refill(RoundUp(bytes));
}

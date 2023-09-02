#include "Pool_Alloc.h"


char* PoolAlloc::start_free = nullptr;
char* PoolAlloc::end_free = nullptr;
size_t PoolAlloc::heap_size = 0;//���ǣ���
PoolAlloc::obj* PoolAlloc::free_list[PoolAlloc::NFREELISTS]{};

void* PoolAlloc::refill(size_t bytes)
{
	size_t nobjs = NOBJS;//��ʾʵ�������˶��ٿ�
	//���ȴӳ���ȥһ���ڴ�
	char* chunk = static_cast<char*>(chunk_alloc(bytes,nobjs));
	if (nobjs == 1)
	{
		return chunk;
	}
	//ȡ��һ���ڴ�
	size_t index = FreeList_Index(bytes);
	obj* result = (obj*)chunk;
	free_list[index] = (obj*)(chunk+bytes);
	//ʣ�µ��ڴ��п�
	obj* current_obj = nullptr;//��ǰ��
	obj* next_obj = free_list[index];//��һ��
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
	//������Ҫ������ڴ���ս�����е��ڴ����Ƚϣ��ж��Ƿ���������
	size_t totalSize = bytes * NOBJS;
	size_t leftSize = end_free - start_free;

	if (totalSize <= leftSize)
	{
		//ʣ��ս�����ڴ��㹻���ͷ����ȥtotalSize
		auto result=start_free;
		start_free += totalSize;
		return result;
	}
	else if (bytes <= leftSize) {
		//ʣ��ս�����ڴ�ֻ�����1����NOBJS������ôȡ��һ�飬ʣ�µĲ��ܡ�
		nobjs = leftSize / bytes;
		totalSize = nobjs * bytes;
		auto result = start_free;
		start_free = start_free + totalSize;
		return result;
	}
	else {
		//ʣ��ս�����ڴ治��һ����
		
		//�����ʣ��ģ��Ȱ�ʣ�µ��ڴ�ҵ���Ӧ��freelist����
		if (leftSize > 0) {
			auto index=FreeList_Index(leftSize);
			((obj*)start_free)->next = free_list[index];
			free_list[index] = (obj*)start_free;
		}
		//Ȼ�������ڴ��СΪ2��
		size_t bytes_to_get = 2 * totalSize + RoundUp(heap_size >> 4);//roundup()����ӵĵ�����ʲô��
		start_free = (char*)malloc(bytes_to_get);
		//�ο����ºʹ����У��ж�start_free������Ϊ�գ��������ս�����в��࣬���ڴ�����ʧ�����أ��ɼ�malloc�����Ƿ���ʧ�ܻ᷵�ؿ�ָ�롣
		if (!start_free) {
			//�������ʧ�ܣ�û�ڴ��ˣ�ֻ����֮ǰ�Ѿ�������ڴ�����һ���������Ϊս����
			//����ο��������Լ�TinySTL��д���е����⣬Ӧ���������Щ���Ÿ����ڴ���freelist�����ڴ棬����������������
			//������ҵ����Ǳ�bytesС�ģ��ٴ�chunk_alloc()�����ǻ�ص�����ľ��������ڰ�����
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
			//����ο�����д��Ҳ�����⣬û���׳��쳣Ҳû�ж�Ӧ�ķ��أ�������ѭ�������ϵݹ�.������򵥴���������ؿ�ָ�룬���ٲ��᲻ͣ�ݹ顣
			end_free = 0;
			return nullptr;
		}
		//�������ɹ���ֱ�ӷ��أ���refill�и�
		heap_size += bytes_to_get;
		end_free = start_free + bytes_to_get;
		return chunk_alloc(bytes,nobjs);//��ݹ����һ�´���һ��start_free
	}
}

void* PoolAlloc::allocate(size_t bytes)
{
	//�����ж�����Ĵ�С��������Χ�Ͳ�������(����malloc)
	if (bytes > 128)
	{
		return malloc(bytes);
	}
	//�ȿ�һ�¶���֮�󣬷������ڴ棬�������ĸ�freelist��
	size_t index = FreeList_Index(bytes);
	obj* listHead = free_list[index];
	if (listHead != nullptr)
	{
		free_list[index] = listHead->next;
		return listHead;
	}
	return refill(RoundUp(bytes));
}

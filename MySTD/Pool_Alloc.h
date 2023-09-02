#pragma once
#include<stdlib.h>

//С�᣺�ڴ�زο���һЩ���¡�TinySTL�Լ���ݵ�c++�ڴ�����ж�SGI���ڴ��ʵ�ַ����Ľ��⡣
//�ڴ�����۱����Ǻܺ����ģ����̳߳أ������һ����һ����ʡ������������ڴ�Ŀ�������������˿ռ������ʣ�
//��Ϊmalloc����������ڴ����cookie��debug�õ���Ϣ��������ô˵����������ǰ����һ�ѣ�֮��ֱ��ȡ���ã���������
//���ص������С���Ȼ�ھ��������У�ʹ�õ��������ŵĵ��б���νfreelist�ǡ�Ϊ��ͳһ�������е��ڴ���䣬����
//���˶�Ӧ�����ڴ��С��freelist��ѡ�����ڴ�Ҷ���ͳһ�ķ����������ˣ���������allocatorһ��ÿ����ͬ������в�
//ͬ�ķ�����ʵ�֡����о����ڴ���ɢ(memory diffusion),����freelist�ķ�����û�н�����Լ��ڴ���Ƭ��˵���׶����
//�����ڲ���Ƭ���������û�취���⡣
//��Ȼ��������²�һ�£��Լ��ο��Ķ�����΢�е����ˣ��ӿ���Ƹ��ҵĸо����ǹֵֹģ�����refill��chunk_alloc()��
//˵ʵ��������ӿڲ�������ͦ������ģ�һ����Ҫ�����߿��Ǵ�����Ƿ���һ����������ڴ�һ���ֲ��ã��Լ��ѿ�Ķ���
//��Ϊ���÷��ع���������ӿڴ�ԭ������������������Ҿ��ò�������������ǵ�����private��Ҳ����Ҫ�û�֪��ʲô����
//���ĺ���������ô�����ô���ˡ�
//ͬʱ�ο�TinySTL�����п���һЩ���󣬲��Ҹо�Ҳ�ǲο�SGIд�ġ�
//�������ȿ��˹���ʵ�ָ߲����ڴ�ص����£������ⶫ��̫�������˵㣬�����˸����ܵĲ��������ֻ�ǶԽӿڶ���������
//����Ҳ̫���ˣ����ټ���STL��allocator����Ʊ����һЩ���⣬��������������д����Ĺ���һ��Ҳ�����죬��ʣ��һ�㣬
//��Ȼ����ʼ���գ���֮������ֲ����ʣ��reallocate��deallocate�Ĳ�����ʵ���ˣ�����ܼ�ûʲô�ϰ�����Ȼ���ٽ�
//���ǲ������ˡ�

class PoolAlloc {
private:
	//3����Ա����abc,aҪ��bc��ʼ����bcֵ��ʼ�����������ᱻ��ʼ��Ϊʲô
	static constexpr size_t ALIGN = 8;
	static constexpr size_t MAXBYTES = 128;
	static constexpr size_t NFREELISTS = MAXBYTES / ALIGN;
	static constexpr size_t NOBJS = 20;
private:
	//��Ƕָ��
	struct obj{
		obj* next;
	};
	//freelistָ�����飬ÿ��Ԫ��Ϊָ��ָ�������С�ڴ��freelist
	static obj* free_list[NFREELISTS];

private:
	static char* start_free;
	static char* end_free;
	static size_t heap_size;//���ǣ���
private:
	static size_t RoundUp(size_t bytes) {
		return (bytes + ALIGN - 1) & ~(ALIGN - 1);
	}
	//ͨ���ϵ�����ڴ�ȷ��������ڴ�ҵ��Ǹ�freelist����
	static size_t FreeList_Index(size_t bytes) {
		return (bytes - 1) / ALIGN;
	}
	//���𽫽��ռ��п�,����Ϊ�������ڴ��С
	static void* refill(size_t bytes);

	//��������һ�����Ա�ָ��bytes��С�Ķ������ÿռ�,�ڶ����������ڷ�������ռ�Ŀ�����
	//Ҳ����˵����ӿ������ȫ�������п飬ֻ����취����һ����������bytes��С���ڴ�
	static void* chunk_alloc(size_t bytes, size_t& nobjs);
public:
	static void* allocate(size_t bytes);
	static void  deallocate(void* ptr, size_t bytes);
	static void* reallocate(void* ptr, size_t old_sz, size_t new_sz);

};
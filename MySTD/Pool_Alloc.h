#pragma once
#include<stdlib.h>

//小结：内存池参考了一些文章、TinySTL以及侯捷的c++内存管理中对SGI中内存池实现方案的讲解。
//内存池理论本身还是很好理解的，和线程池，对象池一样，一来节省反复申请分配内存的开销；二来提高了空间利用率，
//因为malloc本身申请的内存带有cookie和debug用的信息。不管怎么说方法就是提前申请一堆，之后直接取着用，用完了再
//返回到池子中。当然在具体的设计中，使用的是数组存放的单列表，所谓freelist们。为了统一管理所有的内存分配，所以
//有了对应多种内存大小的freelist可选。现在大家都用统一的分配器分配了，而不是像allocator一样每个不同的类会有不
//同的分配器实现。还有就是内存扩散(memory diffusion),这里freelist的方法并没有解决，以及内存碎片，说到底对齐产
//生的内部碎片，这个方法没办法避免。
//当然最后还是想吐槽一下，自己参考的东西稍微有点老了，接口设计给我的感觉总是怪怪的，比如refill和chunk_alloc()，
//说实话，这个接口参数让我挺不舒服的，一会有要调用者考虑传入的是否是一个对齐过的内存一会又不用，以及把块的多少
//作为引用返回过来，这个接口从原型上理解起来多少让我觉得不舒服，不过考虑到它是private，也不需要用户知道什么，内
//部的函数划分怎么舒服怎么来了。
//同时参考TinySTL过程中看到一些错误，并且感觉也是参考SGI写的。
//加上事先看了关于实现高并发内存池的文章，觉得这东西太过鸡肋了点，做不了高性能的并发（如果只是对接口都上锁，那
//开销也太大了），再加上STL中allocator的设计本身的一些问题，这种种让我整个写代码的过程一点也不畅快，还剩下一点，
//虽然想善始善终，但之后估计又不会对剩下reallocate和deallocate的部分做实现了，本身很简单没什么障碍，当然至少今
//天是不想碰了。

class PoolAlloc {
private:
	//3个成员变量abc,a要用bc初始化，bc值初始化，他们最后会被初始化为什么
	static constexpr size_t ALIGN = 8;
	static constexpr size_t MAXBYTES = 128;
	static constexpr size_t NFREELISTS = MAXBYTES / ALIGN;
	static constexpr size_t NOBJS = 20;
private:
	//内嵌指针
	struct obj{
		obj* next;
	};
	//freelist指针数组，每个元素为指向指定对齐大小内存的freelist
	static obj* free_list[NFREELISTS];

private:
	static char* start_free;
	static char* end_free;
	static size_t heap_size;//这是？？
private:
	static size_t RoundUp(size_t bytes) {
		return (bytes + ALIGN - 1) & ~(ALIGN - 1);
	}
	//通过上调后的内存确定申请的内存挂到那个freelist下面
	static size_t FreeList_Index(size_t bytes) {
		return (bytes - 1) / ALIGN;
	}
	//负责将将空间切块,参数为对齐后的内存大小
	static void* refill(size_t bytes);

	//负责申请一大块可以被指定bytes大小的对象利用空间,第二个参数用于返回申请空间的块数，
	//也就是说这个接口设计完全不考虑切块，只是想办法返回一大块可以容纳bytes大小的内存
	static void* chunk_alloc(size_t bytes, size_t& nobjs);
public:
	static void* allocate(size_t bytes);
	static void  deallocate(void* ptr, size_t bytes);
	static void* reallocate(void* ptr, size_t old_sz, size_t new_sz);

};
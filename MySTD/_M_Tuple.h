#pragma once
#include<iostream>
#include<tuple>
#include<memory>
#include<xutility>
#include<type_traits>
//tuple的实现方法让我感觉十分惊喜和巧妙，原来继承和模板还可以有这样的设计，当时看的时候大有这种感觉。实际上，一句话就能总结tuple的实现方法：
//利用模板类的继承递归的去实例化基类，从而存储不定数量的变量。我们可以从头开始思考这一点，如果让自己设计一个可以存储多种类型的变量的类型或者
//容器，自己会如何实现？我们能很快顺着思路想到，用一个普普通通的类，将给定固定数量的多种类型作为成员变量即可。那么假如现在要求数量不固定，也
//就是支持多个不同类型的变量的存储呢？显然组合的方式就不太行了，因为成员变量的数量从一开始就是固定的，难到我们还去为每一种不同数量的组合写一
//个单独的类？这显然不可能。tuple让我感觉巧妙的一点就是利用继承去处理数量不固定的变量的存储。通过配合模板参数包，让继承递归下去。例如一个
//tuple<int ,double,float,int, int>那么它实际上是继承于tuple<int, double, float, int>的，而tuple<int, double, float, int>又继承于tuple<int, double, float>
//以此类推，递归的继承，这就是tuple的实现方案。
//我这里并没有讨论这些变量在这些基类中又是如何存储的，实际上msvc就是作为成员变量存储的，而其他编译器可能不太一样，因为msvc的这种实现无法进行EBCO
//空基类优化。因为空类存储总是占用一个字节，目的是为了区分不同的对象，这样占一个字节就会有不同的地址了。但如此多的空基类放在tuple中，我们显然不想
//让他们都占用这一个字节的多余内存。因此编译器有了EBCO来解决。
//我在这里实现是支持EBCO的版本。
//
//问题1：可能你会跟我有同样的疑惑，get<>()是如何获取第几个元素的呢，因为看msvc的实现它并没有index作为模板参数，没办法通过index获取到对应的类型变量，这里get
//的实现实际上又用到了递归，通过模板递归的实例化知道index减少为1，则表示找到了那个arg 在< ...Args>中的位置。
//
//问题2：既然tuple是继承于这些元素的，那不会导致它转换为基类的其他元素吗？也就是说int* a=new tuple<int>(1);
//
//问题3：为什么有了万能引用版本的构造函数了，还要写一个const &引用版本的构造函数？

namespace MySTD {
	//非final(无法继承)且为空类
	template<typename _Type>
	using is_empty_and_not_final = typename std::conditional<std::is_final_v<_Type>, std::false_type, std::is_empty<_Type>>::type;

	template<size_t Index, typename _Element, bool = is_empty_and_not_final<_Element>::value>
	class _Tuple_elem;

	template<size_t Index, typename _Element>
	class _Tuple_elem<Index, _Element, true> :private _Element {
	public:
		constexpr _Tuple_elem() = default;
		template<typename _Type>
		constexpr _Tuple_elem(_Type&& _elem) :_Element(std::forward<_Type>(_elem)) {};
	};

	template<size_t Index, typename _Element>
	class _Tuple_elem < Index, _Element, false> {
	public:
		constexpr _Tuple_elem() = default;
		//constexpr _Tuple_elem(const _Element& _elem) :elem(_elem) {};
		template<typename _Type>
		constexpr _Tuple_elem(_Type&& _elem) :elem(std::forward<_Type>(_elem)) {};//static_cast<_Type&&>(elem)
	private:
		_Element elem;
	};

	template<size_t Index,typename ...Rest>
	class _Tuple_impl;

	template<size_t Index, typename _This, typename ..._Rest >
	class _Tuple_impl<Index,_This,_Rest...> :public _Tuple_impl<Index + 1, _Rest...>, private _Tuple_elem<Index, _This> {
	public:
		using _ThisType = _Tuple_elem<Index, _This>;
		using _BaseType = _Tuple_impl<Index + 1, _Rest...>;

		constexpr _Tuple_impl() = default;

		template<typename _UThis, typename ..._URest>
		constexpr _Tuple_impl(_UThis&& _elem, _URest&& ..._rest) :_ThisType(std::forward<_UThis>(_elem)), _BaseType(std::forward<_URest>(_rest)...) {};
	};

	template<size_t Index, typename _This>
	class _Tuple_impl<Index,_This> :public _Tuple_elem<Index, _This> {
	public:
		using _ThisType = _Tuple_elem<Index, _This>;
		constexpr _Tuple_impl() = default;

		template<typename _UThis>
		constexpr _Tuple_impl(_UThis&& _elem) :_ThisType(std::forward<_UThis>(_elem)) {};
	};

	template<typename ..._Elements >
	class Tuple :public _Tuple_impl<0, _Elements...> {
	public:
		using _ImplType = _Tuple_impl<0, _Elements...>;

		constexpr Tuple(const _Elements& ..._elems):_ImplType(_elems...) {};
	};

	


};
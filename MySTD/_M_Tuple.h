#pragma once
#include<iostream>
#include<tuple>
#include<memory>
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


//支持EBCO版本的tuple，需要自己实现一下向基类的转换。

namespace MySTD {

//总共分5块：HeadBase，_Tuple_impl,Tuple,tuple_element, get
/*
* 
* _Tuple_base/_Head_Base*************************
* 
*/
	//非final(无法继承)且为空类
	template<typename _Type>
	using is_empty_and_not_final = typename std::conditional<std::is_final_v<_Type>, std::false_type, std::is_empty<_Type>>::type;


	//HeadBase，tuple中负责存储每个元素的类。
	template<size_t Index, typename _Element, bool = is_empty_and_not_final<_Element>::value>
	class _Tuple_base;

	//特例化版本，当为空类且可以继承，这里必须为public，因为需要通过上行转换获取元素值。
	template<size_t Index, typename _Element>
	class _Tuple_base<Index, _Element, true> :public _Element {
	public:
		constexpr _Tuple_base() = default;

		template<typename _Type>
		constexpr _Tuple_base(_Type&& _elem) :_Element(std::forward<_Type>(_elem)) { std::cout << "true" << std::endl; };

		static constexpr _Element& _Base(_Tuple_base& _te) noexcept {
			return _te;
		};
		static constexpr const _Element& _Base(const _Tuple_base& _te) noexcept {
			return _te;
		};
	};

	//非空或者不可继承
	template<size_t Index, typename _Element>
	class _Tuple_base < Index, _Element, false> {
	public:
		constexpr _Tuple_base() = default;

		template<typename _Type>
		constexpr _Tuple_base(_Type&& _elem) :elem(std::forward<_Type>(_elem)) {};//static_cast<_Type&&>(elem)


		static constexpr _Element& _Base(_Tuple_base& _te) noexcept {
			return _te.elem;
		};
		static constexpr const _Element& _Base(const _Tuple_base& _te) noexcept {
			return _te.elem;
		};

	private:
		_Element elem;
	};


/*
* 
*_Tuple_impl********************************
* 
*/
	//Tuple的实现类
	template<size_t Index,typename ...Rest>
	class _Tuple_impl;

	template<size_t Index, typename _This, typename ..._Rest >
	class _Tuple_impl<Index,_This,_Rest...> :public _Tuple_impl<Index + 1, _Rest...>, private _Tuple_base<Index, _This> {
	public:
		using _BaseType = _Tuple_base<Index, _This>;
		using _InheritedType = _Tuple_impl<Index + 1, _Rest...>;

		constexpr _Tuple_impl() = default;

		template<typename _UThis, typename ..._URest>
		constexpr _Tuple_impl(_UThis&& _elem, _URest&& ..._rest) :_BaseType(std::forward<_UThis>(_elem)), _InheritedType(std::forward<_URest>(_rest)...) {};

		static constexpr _This& _Base(_Tuple_impl& _ti) noexcept{
			return _BaseType::_Base(_ti);
		}
		static constexpr const _This& _Base(const _Tuple_impl& _ti) noexcept{
			return _BaseType::_Base(_ti);
		}

	};

	template<size_t Index, typename _This>
	class _Tuple_impl<Index,_This> :private _Tuple_base<Index, _This> {
	public:
		using _BaseType = _Tuple_base<Index, _This>;
		constexpr _Tuple_impl() = default;

		template<typename _UThis>
		constexpr _Tuple_impl(_UThis&& _elem) :_BaseType(std::forward<_UThis>(_elem)) {};


		static constexpr _This& _Base(_Tuple_impl& _ti) noexcept {
			return _BaseType::_Base(_ti);
		}
		static constexpr const _This& _Base(const _Tuple_impl& _ti) noexcept {
			return _BaseType::_Base(_ti);
		}
	};


/*
* 
* Tuple************************************************
* 
*/
	//封装一下
	template<typename ..._Elements>
	class Tuple :public _Tuple_impl<0, _Elements...> {
	public:
		using _ImplType = _Tuple_impl<0, _Elements...>;

		constexpr Tuple(const _Elements& ..._elems):_ImplType(_elems...) {};

		template<typename ...UType>
		constexpr Tuple(UType&&...elems) :_ImplType(std::forward<UType>(elems)...) {};
	}; 



/*
* 
* _tuple_element************************************************
* 
*/
	//因为不需要知道强制转换的类型，我们的tuple_element只需要提供元素类型就可以了(vs中是提供转换到的类型，以及元素类型，
	//我们这里转换类型利用实参推导就搞定了，因为有类型中有模板参数Index，可以部分推导)，所以实际上我们可以写个泛用版本的
	//对于<Elements...>获取其中的第Index个类型
	// 
	//居然错误了，因为使用了模板实参推导，推导出的gethelper中的elements已经与get中的不一样了，比如get中是<2,int ,double, int >,gethelper中是
	//先找到符合index为2的tuple_impl也就是_Tuple_Impl<2,int>,所以其中的elements只是<int>.正因为这样，所以我们必须提供_Tupel_Impl作为模板参数
	//但是我们使用的版本有参数所以要多提供一个
	//
	//好吧，我又错了，我是傻逼，可以直接在get_helper中就把当前元素类型提出来啊，它强转之后的_This刚好就是
	//
	//又看了一下MSVC的，它用的将get声明为友元，来使得get能访问私有继承的基类类型，以达到强制转换的目的。
	//大失败！！！MSVC的EBCO有bug多继承中，只有第一个空基类会进行EBCO优化

	template<size_t Index, typename ...Elements>
	struct _tuple_element;
	//注意：这里的特例化版本，模板形参与主模板的不同,也就是说特例化版本的实际模板形参列表以模板名后面的<>为准
	template<size_t Index, typename _This, typename ..._Rest>
	struct _tuple_element<Index, _This, _Rest...> :_tuple_element<Index - 1, _Rest...> {};

	template<typename _This, typename ..._Rest>
	struct _tuple_element<0, _This, _Rest...>
	{
		using type = _This;
	};



/*
* 
* get************************************************
* 
*/
	//get的实现方式也分两种，g++和vs的不同
	//vs还是使用递归推导的方式，找到对应索引的元素，
	//g++则是利用了一个_get_helper，利用了模板参数推导，我们可以只给予部分模板实参，然后利用函数参数推导出对应的结果，
	//例如这里，我们想推导一个_Tuple_impl<2,,_Elements...>,传给函数的参数是_Tuple_impl<0,int,double,int,double>,那么
	//这里的_Tuple_impl<2,,_Elements...>就会被推导为_Tuple_impl<2,int,double>.这是跟据索引唯一确定的。因为是父类所以
	//可以上行转换.这样子的实现在c++14，函数返回值可以依靠auto推导之后，不需要类型萃取器去萃取tuple中元素的类型。
	//当然，为了在只支持c++11的环境中使用，类型萃取器还是有必要。
	//在vs的实现中，类型萃取器先萃取类型然后强制转换过去，因为都是成员变量，所以可以直接通过成员变量访问到元素，
	//g++中因为有些是继承，并且为了防止一个tuple转换到元素类型，所以在impl是private继承_Head_base。什么你说为什么不让headbase
	//private继承element？这里我觉得可以随意，总之，需要有一个地方private继承即可。g++为了能转换到private继承的基类，需要提供
	//转换函数，也就是在每个private的继承中子类中实现到private基类的转换，然后直接调用就可以了，这里为了统一接口，对于组合实现的
	//headbase也需要提供一个转换函数。
	//
	template<size_t Index, typename _This, typename ..._Elements>
	constexpr _This& _get_helper(_Tuple_impl<Index,_This,_Elements...>& tp) noexcept {
		using Type = _Tuple_impl<Index, _This, _Elements...>;
		return Type::_Base(tp);
	}

	template<size_t Index, typename _This, typename ..._Elements>
	constexpr const _This& _get_helper(const _Tuple_impl<Index, _This, _Elements...>& tp) noexcept {
		using Type = const _Tuple_impl<Index, _This, _Elements...>;
		return Type::_Base(tp);
	}

	template<size_t Index,typename ..._Elements>
	constexpr typename _tuple_element<Index, _Elements...>::type& get(Tuple<_Elements...>& tp) noexcept{
		return _get_helper<Index>(tp);
	}

	template<size_t Index, typename ..._Elements>
	constexpr typename _tuple_element<Index, _Elements...>::type& get(const Tuple<_Elements...>& tp) noexcept {
		return _get_helper<Index>(tp);
	}

};
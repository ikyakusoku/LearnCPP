#pragma once
#include<iostream>
#include<tuple>
#include<memory>
#include<type_traits>
//tuple��ʵ�ַ������Ҹо�ʮ�־�ϲ�����ԭ���̳к�ģ�廹��������������ƣ���ʱ����ʱ��������ָо���ʵ���ϣ�һ�仰�����ܽ�tuple��ʵ�ַ�����
//����ģ����ļ̳еݹ��ȥʵ�������࣬�Ӷ��洢���������ı��������ǿ��Դ�ͷ��ʼ˼����һ�㣬������Լ����һ�����Դ洢�������͵ı��������ͻ���
//�������Լ������ʵ�֣������ܺܿ�˳��˼·�뵽����һ������ͨͨ���࣬�������̶������Ķ���������Ϊ��Ա�������ɡ���ô��������Ҫ���������̶���Ҳ
//����֧�ֶ����ͬ���͵ı����Ĵ洢�أ���Ȼ��ϵķ�ʽ�Ͳ�̫���ˣ���Ϊ��Ա������������һ��ʼ���ǹ̶��ģ��ѵ����ǻ�ȥΪÿһ�ֲ�ͬ���������дһ
//���������ࣿ����Ȼ�����ܡ�tuple���Ҹо������һ��������ü̳�ȥ�����������̶��ı����Ĵ洢��ͨ�����ģ����������ü̳еݹ���ȥ������һ��
//tuple<int ,double,float,int, int>��ô��ʵ�����Ǽ̳���tuple<int, double, float, int>�ģ���tuple<int, double, float, int>�ּ̳���tuple<int, double, float>
//�Դ����ƣ��ݹ�ļ̳У������tuple��ʵ�ַ�����
//�����ﲢû��������Щ��������Щ������������δ洢�ģ�ʵ����msvc������Ϊ��Ա�����洢�ģ����������������ܲ�̫һ������Ϊmsvc������ʵ���޷�����EBCO
//�ջ����Ż�����Ϊ����洢����ռ��һ���ֽڣ�Ŀ����Ϊ�����ֲ�ͬ�Ķ�������ռһ���ֽھͻ��в�ͬ�ĵ�ַ�ˡ�����˶�Ŀջ������tuple�У�������Ȼ����
//�����Ƕ�ռ����һ���ֽڵĶ����ڴ档��˱���������EBCO�������
//��������ʵ����֧��EBCO�İ汾��
//
//����1��������������ͬ�����ɻ�get<>()����λ�ȡ�ڼ���Ԫ�ص��أ���Ϊ��msvc��ʵ������û��index��Ϊģ�������û�취ͨ��index��ȡ����Ӧ�����ͱ���������get
//��ʵ��ʵ�������õ��˵ݹ飬ͨ��ģ��ݹ��ʵ����֪��index����Ϊ1�����ʾ�ҵ����Ǹ�arg ��< ...Args>�е�λ�á�
//
//����2����Ȼtuple�Ǽ̳�����ЩԪ�صģ��ǲ��ᵼ����ת��Ϊ���������Ԫ����Ҳ����˵int* a=new tuple<int>(1);
//
//����3��Ϊʲô�����������ð汾�Ĺ��캯���ˣ���Ҫдһ��const &���ð汾�Ĺ��캯����


//֧��EBCO�汾��tuple����Ҫ�Լ�ʵ��һ��������ת����

namespace MySTD {

//�ܹ���5�飺HeadBase��_Tuple_impl,Tuple,tuple_element, get
/*
* 
* _Tuple_base/_Head_Base*************************
* 
*/
	//��final(�޷��̳�)��Ϊ����
	template<typename _Type>
	using is_empty_and_not_final = typename std::conditional<std::is_final_v<_Type>, std::false_type, std::is_empty<_Type>>::type;


	//HeadBase��tuple�и���洢ÿ��Ԫ�ص��ࡣ
	template<size_t Index, typename _Element, bool = is_empty_and_not_final<_Element>::value>
	class _Tuple_base;

	//�������汾����Ϊ�����ҿ��Լ̳У��������Ϊpublic����Ϊ��Ҫͨ������ת����ȡԪ��ֵ��
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

	//�ǿջ��߲��ɼ̳�
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
	//Tuple��ʵ����
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
	//��װһ��
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
	//��Ϊ����Ҫ֪��ǿ��ת�������ͣ����ǵ�tuple_elementֻ��Ҫ�ṩԪ�����;Ϳ�����(vs�����ṩת���������ͣ��Լ�Ԫ�����ͣ�
	//��������ת����������ʵ���Ƶ��͸㶨�ˣ���Ϊ����������ģ�����Index�����Բ����Ƶ�)������ʵ�������ǿ���д�����ð汾��
	//����<Elements...>��ȡ���еĵ�Index������
	// 
	//��Ȼ�����ˣ���Ϊʹ����ģ��ʵ���Ƶ����Ƶ�����gethelper�е�elements�Ѿ���get�еĲ�һ���ˣ�����get����<2,int ,double, int >,gethelper����
	//���ҵ�����indexΪ2��tuple_implҲ����_Tuple_Impl<2,int>,�������е�elementsֻ��<int>.����Ϊ�������������Ǳ����ṩ_Tupel_Impl��Ϊģ�����
	//��������ʹ�õİ汾�в�������Ҫ���ṩһ��
	//
	//�ðɣ����ִ��ˣ�����ɵ�ƣ�����ֱ����get_helper�оͰѵ�ǰԪ�����������������ǿת֮���_This�պþ���
	//
	//�ֿ���һ��MSVC�ģ����õĽ�get����Ϊ��Ԫ����ʹ��get�ܷ���˽�м̳еĻ������ͣ��Դﵽǿ��ת����Ŀ�ġ�
	//��ʧ�ܣ�����MSVC��EBCO��bug��̳��У�ֻ�е�һ���ջ�������EBCO�Ż�

	template<size_t Index, typename ...Elements>
	struct _tuple_element;
	//ע�⣺������������汾��ģ���β�����ģ��Ĳ�ͬ,Ҳ����˵�������汾��ʵ��ģ���β��б���ģ���������<>Ϊ׼
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
	//get��ʵ�ַ�ʽҲ�����֣�g++��vs�Ĳ�ͬ
	//vs����ʹ�õݹ��Ƶ��ķ�ʽ���ҵ���Ӧ������Ԫ�أ�
	//g++����������һ��_get_helper��������ģ������Ƶ������ǿ���ֻ���貿��ģ��ʵ�Σ�Ȼ�����ú��������Ƶ�����Ӧ�Ľ����
	//��������������Ƶ�һ��_Tuple_impl<2,,_Elements...>,���������Ĳ�����_Tuple_impl<0,int,double,int,double>,��ô
	//�����_Tuple_impl<2,,_Elements...>�ͻᱻ�Ƶ�Ϊ_Tuple_impl<2,int,double>.���Ǹ�������Ψһȷ���ġ���Ϊ�Ǹ�������
	//��������ת��.�����ӵ�ʵ����c++14����������ֵ��������auto�Ƶ�֮�󣬲���Ҫ������ȡ��ȥ��ȡtuple��Ԫ�ص����͡�
	//��Ȼ��Ϊ����ֻ֧��c++11�Ļ�����ʹ�ã�������ȡ�������б�Ҫ��
	//��vs��ʵ���У�������ȡ������ȡ����Ȼ��ǿ��ת����ȥ����Ϊ���ǳ�Ա���������Կ���ֱ��ͨ����Ա�������ʵ�Ԫ�أ�
	//g++����Ϊ��Щ�Ǽ̳У�����Ϊ�˷�ֹһ��tupleת����Ԫ�����ͣ�������impl��private�̳�_Head_base��ʲô��˵Ϊʲô����headbase
	//private�̳�element�������Ҿ��ÿ������⣬��֮����Ҫ��һ���ط�private�̳м��ɡ�g++Ϊ����ת����private�̳еĻ��࣬��Ҫ�ṩ
	//ת��������Ҳ������ÿ��private�ļ̳���������ʵ�ֵ�private�����ת����Ȼ��ֱ�ӵ��þͿ����ˣ�����Ϊ��ͳһ�ӿڣ��������ʵ�ֵ�
	//headbaseҲ��Ҫ�ṩһ��ת��������
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
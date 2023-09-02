#pragma once
#include<iostream>
#include<tuple>
#include<memory>
#include<xutility>
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

namespace MySTD {
	//��final(�޷��̳�)��Ϊ����
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
#pragma
#include<atomic>

struct RefCountType {
	std::atomic<size_t> count{};
	void increase() {
		count++;
	}
	void reduce() {
		count--;
	}
	bool isZero() {
		if (count == 0)
			return true;
		else {
			return false;
		}
	}

	size_t getVal() {
		return count;
	}
};

template<typename Type>
class SharedPtr {
public:
	constexpr SharedPtr() noexcept = default;
	constexpr SharedPtr(std::nullptr_t) noexcept {};
	constexpr SharedPtr(Type* _pointer) noexcept : pointer(_pointer),ref_count(new RefCountType())  {
		ref_count->increase();
	};

	constexpr SharedPtr(const SharedPtr& _other) noexcept :pointer(_other.pointer), ref_count(_other.ref_count) {
		ref_count->increase();
	}
	constexpr SharedPtr(SharedPtr&& _other) noexcept : pointer(_other.pointer), ref_count(_other.ref_count) {
		_other.pointer = nullptr;
		_other.ref_count = nullptr;
	}

	SharedPtr& operator=(const SharedPtr& _other) {
		if (pointer != nullptr) {
			release();
		}
		pointer = _other.pointer;
		ref_count = _other.ref_count;
		ref_count->increase();

		return *this;
	}

	SharedPtr& operator=(SharedPtr&& _other) {
		if (pointer != nullptr) {
			release();
		}
		pointer = _other.pointer;
		ref_count = _other.ref_count;
		
		_other.pointer = nullptr;
		_other.ref_count = nullptr;

		return *this;
	}
	
	Type* operator->() {
		return pointer;
	}

	void reset(Type* _pointer=nullptr) {
		release();
		if (_pointer == nullptr) {
			pointer = nullptr;
			ref_count = nullptr;
			return;
		}

		pointer = _pointer;
		ref_count=new RefCountType();
		ref_count->increase();
	}

	Type* get() {
		return pointer;
	}

	size_t use_count() {
		if (ref_count == nullptr)
			return 0;
		return ref_count->getVal();
	}

	~SharedPtr() {
		release();
	}

private:
	void release() {
		if (ref_count != nullptr)
		{
			ref_count->reduce();
			if (ref_count->isZero())
			{
				delete pointer;
				delete ref_count;
			}
		}
	}

private:

	Type* pointer{};
	RefCountType* ref_count{};
};
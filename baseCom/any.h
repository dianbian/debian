#pragma once
#include <typeinfo>
#include <algorithm>
//#include <typeindex> 
//#include <type_traits>

class any
{
public:
	any() : content(0)
	{
	}
	template<typename ValueType>
	any(const ValueType& value): content(new holder<ValueType>(value))
	{
	}
	any(const any & other)
		: content(other.content ? other.content->clone() : 0)
	{
	}

	~any()
	{
		delete content;
	}

public: // modifiers
	any & swap(any & rhs)
	{
		std::swap(content, rhs.content);
		return *this;
	}
	template<typename ValueType>
	any & operator=(const ValueType & rhs)
	{
		any(rhs).swap(*this);
		return *this;
	}

	any & operator=(any rhs)
	{
		any(rhs).swap(*this);
		return *this;
	}

public: // queries
	bool empty() const
	{
		return !content;
	}

	void clear()
	{
		any().swap(*this);
	}

	const std::type_info & type() const
	{
		return content ? content->type() : typeid(void);
	}

	class placeholder
	{
	public: // structors
		virtual ~placeholder()
		{
		}
	public: // queries
		virtual const std::type_info & type() const = 0;
		virtual placeholder * clone() const = 0;
	};
public:
	template<typename ValueType>
	class holder: public placeholder
	{
	public:
		holder(const ValueType& value): held(value)
		{
		}
		virtual const std::type_info& type() const 
		{
			return typeid(ValueType);
		}

		virtual placeholder * clone() const
		{
			return new holder(held);
		}
	public: // representation
		ValueType held;
	private: // intentionally left unimplemented
		holder & operator=(const holder &);
	};
	placeholder* content;    
};

inline void swap(any & lhs, any & rhs)
{
	lhs.swap(rhs);
}

class  bad_any_cast : public std::bad_cast
{
public:
	virtual const char* what() const 
	{
		return "bad_any_cast: "
			"failed conversion using any_cast";
	}
};

template<typename ValueType>
ValueType * any_cast(any * operand)
{
	return operand && 
#ifdef ANY_TYPE_ID_NAME
		std::strcmp(operand->type().name(), typeid(ValueType).name()) == 0
#else
		operand->type() == typeid(ValueType)
#endif
		? &static_cast<any::holder<ValueType> *>(operand->content)->held
		: 0;
}

template<typename ValueType>
inline const ValueType * any_cast(const any * operand)
{
	return any_cast<ValueType>(const_cast<any *>(operand));
}

template<typename ValueType>
ValueType any_cast( any& value)
{
	return static_cast<any::holder<ValueType>*>(value.content)->held;
}
template<typename ValueType>
ValueType any_cast( const any& value)
{
	return any_cast<ValueType>(const_cast<any &>(value));
}

#if 0 // test any

#include <iostream>
#include <string>
#include <vector>
void test_any()
{
	any any_a1(123);
	int a2 = any_cast<int>(any_a1);
	int* p_a2 = any_cast<int>(&any_a1);
	std::cout << "a2 = " << a2 <<" *p_a2="<<*p_a2<<std::endl;

	any any_b1(12.35);
	double b2 = any_cast<double>(any_b1);
	std::cout << "b2= " << b2 << std::endl;

	any any_str(std::string("abc"));
	std::string msg = any_cast<std::string>(any_str);
	std::cout << "msg= " << msg << std::endl;

	std::vector<float> values,new_values;
	for(int idx=0;idx<10;idx++)
	{
		values.push_back(0.5+idx+10);
	}
	any any_vector(values);
	new_values = any_cast<std::vector<float>>(any_vector);
	auto iter = new_values.begin();
	while( new_values.end() != iter )
	{
		std::cout << *iter <<" ";
		++ iter;
	}
	std::cout << std::endl;

}

#endif

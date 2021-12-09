//	2015-09-29
//	usage:
//	string operations including: equals, startWith, contains, find_after, replace, to_string, combineString.
//	each string operator has a case-insensitive version, such as contains/icontains, equals/iequals.
//	for example:
//
//		assert(contains("abcde", "bc"));
//		assert(icontains("abcde", "BC"));
//		assert(ireplace("abcabc", "BC", "23"), "a23a23");
//
//	to_string supports built-in conversion for non-string value to std::string
//	built-in types are primitives and std::exception
//	to extend the conversion types, please specify the stringable template.
//	for example:
//
//		template<>
//		struct stringable
//		{
//			std::string operator()(const json11::Json& json) { return json.dump(); }
//		};
//
//		std::string text = combineString("the json content is: ", json);
//

#pragma once

#include "stringable.hpp"
#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>
#include <unordered_set>

namespace string_algo
{
	//comparison operators
	//CharEqual compares char or wchar_t by value
	//ICharEqual compares char or wchar_t by case-insensitively
	struct CharEqual {
		template<typename CharType>
		bool operator()(CharType c1, CharType c2)const { return c1 == c2; }
	};

	struct ICharEqual {
		template<typename CharType>
		bool operator()(CharType c1, CharType c2)const { return std::toupper(c1) == std::toupper(c2); }
	};

	//RangeIter determines begin/end/length of a string-compatible type.
	//The purpose of using RangeIter is to avoid unecessary implicit type conversion.
	struct RangeIter
	{
		template<typename CharType>
		static auto begin(const std::basic_string<CharType>& v)->decltype(std::begin(v)) { return std::begin(v); }
		template<typename CharType, size_t N>
		static auto begin(const CharType v[N])->decltype(std::begin(v)) { return std::begin(v); }
		template<typename CharType>
		static const CharType* begin(const CharType* v) { return v; }
		template<typename CharType>
		static auto end(const std::basic_string<CharType>& v)->decltype(std::end(v)) { return std::end(v); }
		template<typename CharType, size_t N>
		static auto end(const CharType v[N])->decltype(std::end(v)) { return std::end(v); }
		static const char* end(const char* v) { return v + strlen(v); }
		static const wchar_t* end(const wchar_t* v) { return v + wcslen(v); }
		template<typename CharType>
		static size_t length(const std::basic_string<CharType>& v) { return v.size(); }
		template<typename CharType, size_t N>
		static size_t length(const CharType v[N]) { return N; }
		template<typename CharType>
		static size_t length(const CharType* v) { return end(v) - begin(v); }
	};
	//determines whether a string contains another string
	//usage: assert(contains("abcd", "bc")); assert(contains("abcd", "Bc"));
	template<typename Range1T, typename Range2T, typename Predicate = CharEqual>
	inline bool contains(const Range1T& text, const Range2T& subText) {
		return std::search(RangeIter::begin(text), RangeIter::end(text), RangeIter::begin(subText), RangeIter::end(subText), Predicate()) != RangeIter::end(text);
	}
	template<typename Range1T, typename Range2T>
	inline bool icontains(const Range1T& text, const Range2T& subText) {
		return contains<Range1T, Range2T, ICharEqual>(text, subText);
	}
	//usages:
	//auto ptr = find_after("abcde", "bc");
	//assert(std::string(ptr), "de");
	template<typename RangeIterator, typename Range2T, typename Predicate = CharEqual>
	inline RangeIterator find_after(const RangeIterator& textBegin, const RangeIterator& textEnd, const Range2T& subText) {
		auto it = std::search(textBegin, textEnd, RangeIter::begin(subText), RangeIter::end(subText), Predicate());
		if (textEnd == it)
			return it;
		else
			return it + RangeIter::length(subText);
	}
	template<typename RangeIterator, typename Range2T>
	inline RangeIterator ifind_after(const RangeIterator& textBegin, const RangeIterator& textEnd, const Range2T& subText) {
		return find_after<RangeIterator, Range2T, ICharEqual>(textBegin, textEnd, subText);
	}

	template<typename Range1T, typename Range2T, typename Predicate = CharEqual>
	inline bool equals(const Range1T& text1, const Range2T& text2) {
		for (size_t k = 0; text1[k] && text2[k]; ++k) {
			if (false == Predicate()(text1[k], text2[k]))
				return false;
		}
		return true;
	}
	template<typename Range1T, typename Range2T>
	inline bool iequals(const Range1T& text1, const Range2T& text2) {
		return equals<Range1T, Range2T, ICharEqual>(text1, text2);
	}

	template<typename Range1T, typename Range2T, typename Predicate = CharEqual>
	inline bool startsWith(const Range1T& text1, const Range2T& text2) {
		for (size_t k = 0; text2[k]; ++k) {
			if (false == Predicate()(text1[k], text2[k]))
				return false;
		}
		return true;
	}

	template<typename Range1T, typename Range2T>
	inline bool istartsWith(const Range1T& text1, const Range2T& text2) {
		return startsWith<Range1T, Range2T, ICharEqual>(text1, text2);
	}

	template<typename Range1T, typename Range2T, typename Range3T, typename Predicate = CharEqual>
	inline std::basic_string<typename std::remove_const<typename std::remove_reference<decltype((*(Range1T*)nullptr)[0])>::type>::type>
		replace(const Range1T& text, const Range2T& subText, const Range3T& newText) {
		auto end = RangeIter::end(text);
		auto posLast = RangeIter::begin(text);
		auto pos = std::search(posLast, end, RangeIter::begin(subText), RangeIter::end(subText), Predicate());
		auto subTextLength = RangeIter::length(subText);

		std::basic_string<typename std::remove_const<typename std::remove_reference<decltype(*pos)>::type>::type> result;
		while (pos != end)
		{
			result += decltype(result)(posLast, pos);
			result += newText;
			pos += subTextLength;
			posLast = pos;
			pos = std::search(pos, end, RangeIter::begin(subText), RangeIter::end(subText), Predicate());
		}
		result += decltype(result)(posLast, end);
		return result;
	}
	template<typename Range1T, typename Range2T, typename Range3T>
	inline std::basic_string<typename std::remove_const<typename std::remove_reference<decltype((*(Range1T*)nullptr)[0])>::type>::type>
		ireplace(const Range1T& text, const Range2T& subText, const Range3T& newText)
	{
		return replace<Range1T, Range2T, Range3T, ICharEqual>(text, subText, newText);
	}
	struct convert_primitive
	{
		template<typename T>
		std::string operator()(const T& value)
		{
			std::stringstream os;
			os << value;
			return os.str();
		}
		template<typename T>
		std::wstring convert_wstring(const T& value)
		{
			std::wstringstream os;
			os << value;
			return os.str();
		}
		template<typename T>
		T convert_back(const std::string& value)
		{
			std::stringstream os;
			os << value;
			T r;
			os >> r;
			return r;
		}
		template<typename T>
		T convert_back(const std::wstring& value)
		{
			std::wstringstream os;
			os << value;
			T r;
			os >> r;
			return r;
		}
	};

	template<typename Type>
	struct determine_stringable
	{
		typedef typename std::conditional <std::is_convertible<Type, const char*>::value || std::is_base_of<std::string, Type>::value, stringable<std::string>,
			typename std::conditional < std::is_convertible<Type, const wchar_t*>::value || std::is_base_of<std::wstring, Type>::value, stringable<std::wstring>,
			stringable<Type >>::type >::type type;
	};
	struct convert_general
	{
		template<typename T>
		std::string operator()(const T& value)
		{
			return typename determine_stringable<T>::type()(value);
		}
		template<typename T>
		std::wstring convert_wstring(const T& value)
		{
			return stringable<std::wstring>().convert_back(typename determine_stringable<T>::type()(value));
		}
		template<typename T>
		T convert_back(const std::string& value)
		{
			return typename determine_stringable<T>::type().convert_back(value);
		}
		template<typename T>
		T convert_back(const std::wstring& value)
		{
			return typename determine_stringable<T>::type().convert_back(stringable<std::wstring>()(value));
		}
	};
	template<typename Type>
	struct is_primitive
	{
		enum { value = std::is_integral<Type>::value || std::is_floating_point<Type>::value };
	};
	template<typename Type>
	struct is_string
	{
		enum { value = false };
	};
	template<typename Type>
	struct is_string<std::basic_string<Type>>
	{
		enum { value = true };
	};

	template<typename Type>
	struct determine_type
	{
		typedef typename std::conditional < is_primitive<Type>::value, convert_primitive, convert_general> ::type type;
	};
	template <typename T> inline std::string to_string(const T& value)
	{
		return typename determine_type<T>::type()(value);
	}
	template <typename Type> inline std::wstring to_wstring(const Type& value)
	{
		struct fast_convert
		{
			std::wstring operator()(const std::wstring& value)
			{
				return value;
			}
		};
		struct general_convert
		{
			std::wstring operator()(const Type& value)
			{
				return typename determine_type<Type>::type().template convert_wstring<Type>(value);
			}
		};
		return typename std::conditional<std::is_convertible<Type, const wchar_t*>::value || std::is_same<Type, std::wstring>::value,
			fast_convert, general_convert>::type()(value);;
	}
	template<typename T>
	inline std::string to_string(const std::vector<T>& values, char separator = ',') {
		if (0 == values.size())
			return "";

		std::string r;
		for (const auto& e : values)
			r += to_string(e) + separator;
		r.pop_back();
		return r;
	}

    template<typename T>
    inline std::string to_string(const std::unordered_set<T>& values, char separator = ',') {
        if (0 == values.size())
            return "";
        
        std::string r;
        for (const auto& e : values)
            r += to_string(e) + separator;
        r.pop_back();
        return r;
    }

	template<typename Type>
	inline std::string combineString(const Type& head)
	{
		return to_string(head);
	}

	template<typename Head, typename... Parameters>
	inline std::string combineString(const Head& head, const Parameters&... parameters)
	{
		std::string text = to_string(head);
		return text + combineString(parameters...);
	}

	template<typename Type>
	inline Type lexical_cast(const std::string& text)
	{
		return typename determine_type<Type>::type().template convert_back<Type>(text);
	}

	template<typename Type>
	inline Type lexical_cast(const std::wstring& text)
	{
		struct fast_convert
		{
			std::wstring operator()(const std::wstring& value)
			{
				return value;
			}
		};
		struct general_convert
		{
			Type operator()(const std::wstring& value)
			{
				return typename determine_type<Type>::type().template convert_back<Type>(value);
			}
		};
		return typename std::conditional<std::is_convertible<Type, const wchar_t*>::value || std::is_same<Type, std::wstring>::value,
			fast_convert, general_convert>::type()(text);
	}
}

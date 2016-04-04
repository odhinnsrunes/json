/*
Copyright (c) 2012-2015 James Baker

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

The official repository for this library is at https://github.com/odhinnsrunes/json

*/
/*!
 * \mainpage
 * For more information on the JSON standard, visit: http://www.json.org/
 *
 * This library is designed to seamlessly integrate JSON parsing, encoding
 * and data access into c++ in a way that looks similar to Javascript or c++ structures.
 *
 * For example:
 \code
 #include <iostream>
 #include "json.hpp"
 
 int main(int argc, char ** argv) {
 document doc;
 
 for (int i = 0; i < 10; i++) {
 doc["one"][i] = true;
 }
 
 std::cout << doc.write(true);
 
 return 0;
 }
 \endcode
 * Will produce the following JSON:
 \code
 {
 "one": [
 true,
 true,
 true,
 true,
 true,
 true,
 true,
 true,
 true,
 true
 ]
 }
 \endcode
 */

#if !defined JSON_HPP_ || defined OJSON_HPP_START
#ifndef OJSON_HPP_START
#define JSON_HPP_
#endif
#define JSON_DOCUMENT_VERSION "1.0.0"

#ifndef JSON_NUMBER_PRECISION
#define JSON_NUMBER_PRECISION 14
#endif

#if defined __BORLANDC__ && __BORLANDC__ < 0x0600
#pragma warn - 8026
#pragma warn - 8027
#endif

#include <stdio.h>
#include <string.h>
#include <string>
#include <sstream>
#include <map>
#if defined __BORLANDC__ && __BORLANDC__ < 0x0600
#pragma warn - 8012
#endif
#include <deque>
#if defined __BORLANDC__ && __BORLANDC__ < 0x0600
#pragma warn + 8012
#endif
#include <list>
#include <algorithm>

#if defined SUPPORT_ORDERED_JSON && !defined _USE_ADDED_ORDER_
#include "ojson.hpp"
#endif

#ifdef _USE_ADDED_ORDER_
#include "arbitrary_order_map.hpp"
#define JSON_NAMESPACE ojson
#ifndef SUPPORT_ORDERED_JSON
#define SUPPORT_ORDERED_JSON
#endif
#else 
#define JSON_NAMESPACE json
#endif


namespace JSON_NAMESPACE
{
	void debug(const char *format, ...);
	
	enum JSONTypes{
		JSON_VOID = -1,
		JSON_NULL,
		JSON_BOOLEAN,
		JSON_NUMBER,
		JSON_STRING,
		JSON_ARRAY,
		JSON_OBJECT,
	};
	
#if defined __clang__ || defined __GNUC__
#define MAKE_EXPLICIT explicit
#else
#define MAKE_EXPLICIT
#endif
	
#if defined _WIN32
#define i64 __int64
#define ui64 unsigned __int64
#define i32 __int32
#define ui32 unsigned __int32
#define i16 __int16
#define ui16 unsigned __int16
#else
#include <stdint.h>
#define i64 int64_t
#define ui64 uint64_t
#define i32 int32_t
#define ui32 uint32_t
#define i16 int16_t
#define ui16 uint16_t
#endif
	
	class instring;
	class MovingCharPointer;
	class object;
	class array;
	class document;
	class iterator;
	class reverse_iterator;
	/*! \brief 	This is the class that does the heavy lifting for the JSON library.
	 *			However, don't instaitiate this class direcly.  Instead use the
	 *			document class.
	 */
	class value
	{
	public:
		
		friend class object;
		friend class array;
		friend class document;
		friend class query;
		friend class iterator;
		friend class reverse_iterator;
#if defined SUPPORT_ORDERED_JSON && !defined _USE_ADDED_ORDER_
		friend class ojson::value;
		friend class ojson::document;
#elif defined _USE_ADDED_ORDER_
		friend class json::value;
		friend class json::document;
#endif		
		friend void objectParse(value& ret, instring& inputString, bool* bFailed);
		
		friend void arrayParse(value& arr, instring& inputString, bool* bFailed);
		
		friend void nullParse(value& ret, instring& inputString, bool* bFailed);
		friend void valueParse(value& a, instring& inputString, bool* bFailed);
		friend void numberParse(value& ret, instring& s, bool* bFailed);
		static void swap(value& lhs, value& rhs) {
			using std::swap;
			swap(lhs.m_number, rhs.m_number);
			swap(lhs.m_boolean, rhs.m_boolean);
			swap(lhs.str, rhs.str);
			swap(lhs.myType, rhs.myType);
			swap(lhs.obj, rhs.obj);
			swap(lhs.arr, rhs.arr);
		}
		
		value();
		value(const value& V);
		value(const document& V);

#if defined SUPPORT_ORDERED_JSON && !defined _USE_ADDED_ORDER_
		value(const ojson::value& V);
		value(const ojson::document& V);
#elif defined _USE_ADDED_ORDER_
		value(const json::value& V);
		value(const json::document& V);
#endif
		value(bool V);
		value(const char* V);
		value(char* V);
		value(std::string V);
		value(object& V);
		value(array& V);
		
		template <typename T>
		value(T V) {
			m_number = (double)V;
			m_boolean = !(V == 0.0);
			m_places = -1;
			myType = JSON_NUMBER;
			obj = NULL;
			arr = NULL;
			pParentObject = NULL;
			pParentArray = NULL;
		}
		
		~value();
		
		void setParentObject(object* pSetTo);

		void setParentArray(array* pSetTo);

		value& operator=(const value& V);
		// value& operator=(const bool &V);
		// value& operator=(const std::string& V);
		// value& operator=(const char* V);
		// template <typename T>
		// value& operator=(T V);
		
		int isA() const;
		bool isA(int i) const
		{
			return (isA() == i);
		}

		value& emptyArray();
		value& emptyObject();

		value& toArray();
		value& toObject(std::string key);
		value& toString();
		value& toString(int iDecimalPlaces);
		value& toNumber();
		value& toBool();
		value& toNull();
		value& fixedDecimal(int iPlaces);
		
		double number();
		float _float();
		i64 integer();
		ui64 _uint64();
		int _int();
		i64 _integer64();
		double _double();
        size_t _size_t();
		long _long();
		short _short();
		char _char();
		unsigned int _uint();
		unsigned long _ulong();
		unsigned short _ushort();
		unsigned char _uchar();

		bool boolean();
		std::string& string();
		const char* safeCString();
		const char* c_str();
		const char* cString();
		
		value& operator[](size_t index);
		value& operator[](std::string index);
		
		void push_back(const value& val);									   // Array
		void push_front(const value& val);									  // Array
		
		value pop_back();												// Array
		value pop_front();											   // Array
		
		void erase(size_t index);									   // Array
		size_t erase(std::string index);								  // Object
		iterator erase(iterator it);										// Array / Object
		iterator erase(iterator first, iterator last);					  // Array / Object

		bool exists(size_t index);
		bool exists(std::string index);
		
		iterator insert(size_t index, value V);							  // Array
		iterator insert(std::string index, value V);						 // Object
		iterator insert(iterator position, std::string key, value V);						 // Object
		iterator insert(iterator position, value V);						 // Array 
		void insert(iterator position, iterator first, iterator last);  // Array / Object (position ignored unless ojson)
		void insert(iterator first, iterator last);					 // Array (append) / Object
		
		void resize(size_t iCount);
		void resize(size_t iCount, value val);

		bool pruneEmptyValues();
		
		bool empty() const; // Is array empty or object empty or string empty.  Number and booleans return false, NULL and VOID return true.
		
		std::string getKey(size_t index); // VERY slow for objects.  Don't use inside a for loop.  Use iterators instead.
		
		value& at(size_t index);
		
		bool operator==(value V) const;
		bool operator!=(value V) const;
		bool operator>(value V) const;
		bool operator<(value V) const;
		bool operator>=(value V) const;
		bool operator<=(value V) const;
		
		value operator+(value V) const;
		value operator-(value V) const;
		value operator*(value V) const;
		value operator/(value V) const;
		value operator%(value V) const;
		
		value &operator+=(value V);
		value &operator-=(value V);
		value &operator*=(value V);
		value &operator/=(value V);
		value &operator%=(value V);
		
		value operator++(int);
		value &operator++();
		value operator--(int);
		value &operator--();
		
		value operator-();
		
		size_t size() const;
		size_t arraySize();
		void clear();
		void threadedClear();
		void destroy();
		void threadedDestroy();

#ifdef __GNUC__
        void sort(bool (*compareFunc)(const value&, const value&));
#else
        void sort(bool (*compareFunc)(value&, value&));
#endif
		value simpleSearch(value& searchFor, bool bSubStr = false);
		size_t simpleCount(value& searchFor, bool bSubStr = false);
		virtual value merge(value& V);
		
		iterator begin() const;
		iterator end() const;
		reverse_iterator rbegin() const;
		reverse_iterator rend() const;
		iterator find(size_t index) const;
		iterator find(std::string index) const;
		reverse_iterator rfind(size_t index) const;
		reverse_iterator rfind(std::string index) const;
		
		typedef void (*DEBUGPTR)(const char *, ...);
		void debugPrint() const { if (debug) { debug("%s\n", print(0, true).c_str()); } }
		static void setDebug(DEBUGPTR setTo) { debug = setTo; }

		static std::string typeName(JSONTypes type);
		const std::string & key() { return m_key; }
	protected:
		
        static void threadDelete(object *);
        static void threadDelete(array *);
        static void threadDeleteObjectWorker(object *);
        static void threadDeleteArrayWorker(array *);

		void cprint(MovingCharPointer& ptr, int depth = 1, bool bPretty = false) const;
		std::string print(int depth = 0, bool bPretty = false) const;
		
		size_t psize(int depth, bool bPretty) const;
		
		double m_number;
		int m_places;
		bool m_boolean;
		std::string str;
		JSONTypes myType;
		object* obj;
		array* arr;
		
		std::string m_key;

		object* pParentObject;
		array* pParentArray;
		static DEBUGPTR debug;

	private:
		static std::string &makeStringFromNumber(std::string & in, const int &iPlaces, double dNumber);
	};

	void numberParse(value& ret, instring& s, bool* bFailed);
	
	class instring
	{
	public:
		instring(const std::string& in);
		instring(const instring& in);
		instring(char* in);
		
		~instring();
		
		char &take();
		void skip();
		char &peek() const;
		size_t tell() const;
		size_t size() const;
		void seek(size_t newPos);
		char* getPos();
		
		instring& operator=(std::string& in);
		instring& operator=(const char* in);
		instring& operator=(instring& in);
		
		void set(std::string& in);
		void set(const char* in);
		
		
		operator std::string() const
		{
			return std::string(str);
		}
		
		instring operator+(std::string& V) const;
		instring operator+(const char* V) const;
		instring operator+(double V) const;
		
		std::string Str() const;
		std::string SoFar() const;
	private:
		char* str;
		size_t pos;
		size_t m_size;
	};
#ifdef _USE_ADDED_ORDER_
	typedef arbitrary_order_map<std::string, value> myMap;
#else
	typedef std::map<std::string, value> myMap;
#endif
	typedef std::deque<value> myVec;

	class object : private myMap
	{
	public:
		object()
		: myMap() 
		{
			bNotEmpty = false;
			pParentArray = NULL;
			pParentObject = NULL;
		}
		~object() {
		}
		
		object(const object& V)
		: myMap((myMap)V) 
		{
			bNotEmpty = V.bNotEmpty;
			pParentArray = NULL;
			pParentObject = NULL;
		}
		object(const object* V)
		: myMap(*(myMap*)V) 
		{
			bNotEmpty = V->bNotEmpty;
			pParentArray = NULL;
			pParentObject = NULL;
		}
#if defined SUPPORT_ORDERED_JSON && !defined _USE_ADDED_ORDER_
		friend class ojson::object;
		object(const ojson::object& V)
		: myMap(V.begin(), V.end()) 
		{
			bNotEmpty = V.bNotEmpty;
			pParentArray = NULL;
			pParentObject = NULL;
		}
		object(const ojson::object* V)
		: myMap(V->begin(), V->end()) 
		{
			bNotEmpty = V->bNotEmpty;
			pParentArray = NULL;
			pParentObject = NULL;
		}
#elif defined SUPPORT_ORDERED_JSON && defined _USE_ADDED_ORDER_
		friend class json::object;
		object(const json::object& V);
		object(const json::object* V);
#endif

		using myMap::size;
		using myMap::find;
		using myMap::begin;
		using myMap::end;
		using myMap::rbegin;
		using myMap::rend;
		using myMap::insert;
		using myMap::erase;
		using myMap::iterator;
		using myMap::reverse_iterator;
		using myMap::const_iterator;

		bool operator==(const object & rhs) const
		{
			return static_cast<const myMap&>(*this) == static_cast<const myMap&>(rhs);
		}

		bool operator>(const object & rhs) const
		{
			return static_cast<const myMap&>(*this) > static_cast<const myMap&>(rhs);
		}

		bool operator<(const object & rhs) const
		{
			return static_cast<const myMap&>(*this) < static_cast<const myMap&>(rhs);
		}

		bool operator>=(const object & rhs) const
		{
			return static_cast<const myMap&>(*this) >= static_cast<const myMap&>(rhs);
		}

		bool operator<=(const object & rhs) const
		{
			return static_cast<const myMap&>(*this) <= static_cast<const myMap&>(rhs);
		}

		bool operator!=(const object & rhs) const
		{
			return static_cast<const myMap&>(*this) != static_cast<const myMap&>(rhs);
		}

		value & operator[](const std::string & key)
		{
			return static_cast<myMap&>(*this)[key];
		}

		bool empty() const;
		void setNotEmpty();
        void setParentArray(array * pSetTo);
        void setParentObject(object * pSetTo);
        void cprint(MovingCharPointer& ptr, int depth = 1, bool bPretty = false) const;
		size_t psize(int depth, bool bPretty) const;
	protected:
		bool bNotEmpty;
	private:
		array* pParentArray;
		object* pParentObject;
	};
	
	class array : private myVec
	{
	public:
		array()
		: myVec() {
			bNotEmpty = false;
            pParentArray = NULL;
            pParentObject = NULL;
		}
		array(int C)
		: myVec(C) {
			bNotEmpty = true;
            pParentArray = NULL;
            pParentObject = NULL;
		}
#if defined SUPPORT_ORDERED_JSON && !defined _USE_ADDED_ORDER_
		friend class ojson::array;
		array(const ojson::array& V)
		: myVec(V.begin(), V.end()) {
			bNotEmpty = V.bNotEmpty;
            pParentArray = NULL;
            pParentObject = NULL;
		}
		array(const ojson::array* V)
		: myVec(V->begin(), V->end()) {
			bNotEmpty = V->bNotEmpty;
            pParentArray = NULL;
            pParentObject = NULL;
		}
#elif defined SUPPORT_ORDERED_JSON && defined _USE_ADDED_ORDER_
		friend class json::array;
		array(const json::array& V);
		array(const json::array* V);
#endif
		array(const array& V)
		: myVec((myVec)V) {
			bNotEmpty = V.bNotEmpty;
            pParentArray = NULL;
            pParentObject = NULL;
		}
		array(const array* V)
		: myVec(*(myVec*)V) {
			bNotEmpty = V->bNotEmpty;
            pParentArray = NULL;
            pParentObject = NULL;
		}
		~array() {
		}
		
		using myVec::size;
		using myVec::resize;
		using myVec::at;
		using myVec::begin;
		using myVec::end;
		using myVec::rbegin;
		using myVec::rend;
		using myVec::insert;
		using myVec::erase;
		using myVec::iterator;
		using myVec::reverse_iterator;
		using myVec::const_iterator;
		using myVec::pop_front;
		using myVec::pop_back;
		using myVec::push_front;
		using myVec::push_back;
		using myVec::emplace_front;
		using myVec::emplace_back;
		using myVec::front;
		using myVec::back;

		bool operator==(const array & rhs) const
		{
			return static_cast<const myVec&>(*this) == static_cast<const myVec&>(rhs);
		}

		bool operator>(const array & rhs) const
		{
			return static_cast<const myVec&>(*this) > static_cast<const myVec&>(rhs);
		}

		bool operator<(const array & rhs) const
		{
			return static_cast<const myVec&>(*this) < static_cast<const myVec&>(rhs);
		}

		bool operator>=(const array & rhs) const
		{
			return static_cast<const myVec&>(*this) >= static_cast<const myVec&>(rhs);
		}

		bool operator<=(const array & rhs) const
		{
			return static_cast<const myVec&>(*this) <= static_cast<const myVec&>(rhs);
		}

		bool operator!=(const array & rhs) const
		{
			return static_cast<const myVec&>(*this) != static_cast<const myVec&>(rhs);
		}

		bool empty() const;
		void setNotEmpty();
        void setParentArray(array * pSetTo);
        void setParentObject(object * pSetTo);
        void cprint(MovingCharPointer& ptr, int depth = 1, bool bPretty = false) const;
		size_t psize(int depth, bool bPretty) const;
	protected:
		bool bNotEmpty;
	private:
		array* pParentArray;
		object* pParentObject;
	};
	
	class iterator : public std::iterator<std::input_iterator_tag, value>{
	public:
		friend class reverse_iterator;
		iterator() {
			bNone = true;
			bIsArray = false;
			bSetKey = false;
		}
		iterator(const myMap::iterator & it) : obj_it(it) {
			bNone = false;
			// obj_it = it;
			bIsArray = false;
			bSetKey = false;
		}
		iterator(const myVec::iterator & it) : arr_it(it) {
			bNone = false;
			// arr_it = it;
			bIsArray = true;
			dumbRet.clear();
			bSetKey = false;
		}
		iterator(const iterator& it) : arr_it(it.arr_it), obj_it(it.obj_it), dumbRet() {
			bNone = it.bNone;
			// arr_it = it.arr_it;
			// obj_it = it.obj_it;
			bIsArray = it.bIsArray;
			bSetKey = false;
		}
		~iterator() {
			if (bSetKey) {
				obj_it->second.m_key.clear();
			}
		}
		
		iterator& operator++() {
			if (!bNone) {
				if (bIsArray) {
					++arr_it;
				} else {
					if (bSetKey) {
						obj_it->second.m_key.clear();
						bSetKey = false;
					}
					++obj_it;
				}
			}
			return *this;
		}
		
		iterator operator++(int) {
			iterator tmp(*this);
			operator++();
			return tmp;
		}
		
		iterator& operator--() {
			if (!bNone) {
				if (bIsArray) {
					--arr_it;
				} else {
					if (bSetKey) {
						obj_it->second.m_key.clear();
						bSetKey = false;
					}
					--obj_it;
				}
			}
			return *this;
		}
		
		iterator operator--(int) {
			iterator tmp(*this);
			operator--();
			return tmp;
		}
				
		bool operator==(const iterator& rhs) {
			if (bNone && rhs.bNone)
				return true;
			if (bIsArray) {
				return arr_it == rhs.arr_it;
			} else {
				return obj_it == rhs.obj_it;
			}
		}
		
		bool operator!=(const iterator& rhs) {
			if (bNone && rhs.bNone)
				return false;
			if (bIsArray) {
				return arr_it != rhs.arr_it;
			} else {
				return obj_it != rhs.obj_it;
			}
		}
		
		value& operator*() {
			if (!bNone) {
				if (bIsArray) {
					return *arr_it;
				} else {
					if (!bSetKey) {
						obj_it->second.m_key.assign(obj_it->first);
						bSetKey = true;
					}
					return obj_it->second;
				}
			} else {
				dumbRet = value();
				return dumbRet;
			}
		}
		value key() {
			if (!bNone) {
				if (bIsArray)
					return value();
				else
					return value(obj_it->first);
			} else {
				return value();
			}
		}
		
		bool None() { return bNone; }
		bool IsArray() { return bIsArray; }
		
		myVec::iterator & arr() { return arr_it; }
		myMap::iterator & obj() { return obj_it; }
		
	private:
		bool bNone;
		myVec::iterator arr_it;
		myMap::iterator obj_it;
		bool bIsArray;
		value dumbRet;
		bool bSetKey;
	};
	
	class reverse_iterator : public std::reverse_iterator<iterator>
	{
	public:
		reverse_iterator() {
			bNone = true;
			bIsArray = false;
			bSetKey = false;
		}
		reverse_iterator(const myMap::reverse_iterator & it) : obj_it(it) {
			bNone = false;
			// obj_it = it;
			bIsArray = false;
			bSetKey = false;
		}
		reverse_iterator(const myVec::reverse_iterator & it) : arr_it(it) {
			bNone = false;
			// arr_it = it;
			bIsArray = true;
			bSetKey = false;
		}
		reverse_iterator(const myMap::iterator & it) : obj_it(myMap::reverse_iterator(it)) {
			bNone = false;
			// obj_it = myMap::reverse_iterator(it);
			bIsArray = false;
			bSetKey = false;
		}
		reverse_iterator(const myVec::iterator & it) : arr_it(myVec::reverse_iterator(it)) {
			bNone = false;
			// arr_it =  myVec::reverse_iterator(it);
			bIsArray = true;
			bSetKey = false;
		}
		reverse_iterator(const reverse_iterator& it) : arr_it(it.arr_it), obj_it(it.obj_it), dumbRet() {
			bNone = it.bNone;
			// arr_it = it.arr_it;
			// obj_it = it.obj_it;
			bIsArray = it.bIsArray;
			bSetKey = false;
		}
		
		reverse_iterator(const JSON_NAMESPACE::iterator& it) : arr_it(myVec::reverse_iterator(it.arr_it)), obj_it(myMap::reverse_iterator(it.obj_it)) {
			bNone = it.bNone;
			// arr_it = myVec::reverse_iterator(it.arr_it);
			// obj_it = myMap::reverse_iterator(it.obj_it);
			bIsArray = it.bIsArray;
			bSetKey = false;
		}
		~reverse_iterator() 
		{
			if (bSetKey) {
				obj_it->second.m_key.clear();
			}
		}
		
		reverse_iterator& operator++() {
			if (!bNone) {
				if (bIsArray) {
					++arr_it;
				} else {
					if (bSetKey) {
						obj_it->second.m_key.clear();
						bSetKey = false;
					}
					++obj_it;
				}
			}
			return *this;
		}
		
		reverse_iterator operator++(int) {
			reverse_iterator tmp(*this);
			operator++();
			return tmp;
		}
		
		reverse_iterator& operator--() {
			if (!bNone) {
				if (bIsArray) {
					--arr_it;
				} else {
					if (bSetKey) {
						obj_it->second.m_key.clear();
						bSetKey = false;
					}
					--obj_it;
				}
			}
			return *this;
		}
		
		reverse_iterator operator--(int) {
			reverse_iterator tmp(*this);
			operator--();
			return tmp;
		}
				
		bool operator==(const reverse_iterator& rhs) {
			if (bNone && rhs.bNone)
				return true;
			if (bIsArray) {
				return arr_it == rhs.arr_it;
			} else {
				return obj_it == rhs.obj_it;
			}
		}
		
		bool operator!=(const reverse_iterator& rhs) {
			if (bNone && rhs.bNone)
				return false;
			if (bIsArray) {
				return arr_it != rhs.arr_it;
			} else {
				return obj_it != rhs.obj_it;
			}
		}
		
		value& operator*() {
			if (!bNone) {
				if (bIsArray) {
					return *arr_it;
				} else {
					if (!bSetKey) {
						(obj_it->second).m_key.assign(obj_it->first);
						bSetKey = true;
					}
					return obj_it->second;
				}
			} else {
				dumbRet = value();
				return dumbRet;
			}
		}
		value key() {
			if (!bNone) {
				if (bIsArray)
					return value();
				else
					return value(obj_it->first);
			} else {
				return value();
			}
		}
		
		bool None() { return bNone; }
		bool IsArray() { return bIsArray; }
		
		myVec::reverse_iterator & arr() { return arr_it; }
		myMap::reverse_iterator & obj() { return obj_it; }
		
	private:
		bool bNone;
		myVec::reverse_iterator arr_it;
		myMap::reverse_iterator obj_it;
		bool bIsArray;
		value dumbRet;
		bool bSetKey;
	};
	
	
	class document : public value
	{
	public:
		friend class value;
		document()
		: value() {
			bParseSuccessful = false;
		}
		
		document(const value& V)
		: value(V) {
			bParseSuccessful = true;
		}

		document(const document& V);
		
		typedef std::string& (*PREPARSEPTR)(const std::string& in, std::string& out, std::string fileName);
		typedef std::string& (*PREWRITEPTR)(const std::string& in, std::string& out);
		bool parse(const std::string& inStr, PREPARSEPTR = NULL, std::string preParseFileName = "");
		
		bool parse(const char* inStr, size_t len, PREPARSEPTR = NULL, std::string preParseFileName = "");
		
		bool parseFile(std::string instr, PREPARSEPTR = NULL, bool bReWriteFile = false);
		
		std::string write(bool bPretty = false, PREWRITEPTR = NULL) const;
		std::string write(int iDepth, bool bPretty = false, PREWRITEPTR = NULL) const;
		
		std::string print(bool bPretty = false, PREWRITEPTR = NULL) const
		{
			return write(bPretty);
		}
		
		bool writeFile(std::string inStr, bool bPretty = false, PREWRITEPTR = NULL) const;
		
		std::string parseResult() const
		{
			return strParseResult;
		}
		
		bool parseSuccessful() const
		{
			return bParseSuccessful;
		}

		template<typename T>
		document(T V) : value(V) { bParseSuccessful = true; }

		static int appendToArrayFile(std::string sFile, const document & atm, bool bPretty);

	protected:
		std::string strParseResult;
		bool bParseSuccessful;
	};
		
	std::ostream& operator<<(std::ostream& S, document& doc);
	std::ostream& operator<<(std::ostream& S, value& doc);
	
}
#if defined __BORLANDC__ && __BORLANDC__ < 0x0600
#pragma warn + 8026
#pragma warn + 8027
#endif

#undef JSON_NAMESPACE

#endif /* JSON_HPP_ */

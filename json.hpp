/*
Copyright (c) 2012-2020 James Baker

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
#if defined __GNUC__
#define DEPRECATED(func) func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED(func) __declspec(deprecated) func
#else
#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#define DEPRECATED(func) func
#endif

#if !defined JSON_HPP_ || defined OJSON_HPP_START
#if !defined OJSON_HPP_START
#define JSON_HPP_
#endif
#define JSON_DOCUMENT_VERSION "1.0.0"

#if !defined JSON_NUMBER_PRECISION
#define JSON_NUMBER_PRECISION 14
#endif

#include <stdio.h>
#include <string.h>
#include <string>
#include <sstream>
#include <map>
#include <deque>
#include <list>
#include <algorithm>

#if defined SUPPORT_ORDERED_JSON && !defined _USE_ADDED_ORDER_
#include "ojson.hpp"
#endif

#include <set>

#if defined _USE_ADDED_ORDER_
#include "arbitrary_order_map.hpp"
#define JSON_NAMESPACE ojson
#if !defined SUPPORT_ORDERED_JSON
#define SUPPORT_ORDERED_JSON
#endif
#else
#define JSON_NAMESPACE json
#endif

#include "sdstring.hpp"

namespace JSON_NAMESPACE
{
	bool fileExists(const char * szName);

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

#if !defined int64_t
#include <stdint.h>
#endif
#define i64 int64_t
#define ui64 uint64_t
#define i32 int32_t
#define ui32 uint32_t
#define i16 int16_t
#define ui16 uint16_t

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
			}

			value() : m_number(0), m_places(-1), m_boolean(false), myType(JSON_VOID), obj(nullptr), pParentObject(nullptr), pParentArray(nullptr) {}
			value(const value& V);
			value(value&& V);
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
			value(const sdstring& V);
			value(sdstring&& V);
			template<typename T = sdstring>
			value(const sdstring& V) : m_number(0), m_places(-1), m_boolean(false), str(V), myType(JSON_STRING), obj(NULL), pParentObject(NULL), pParentArray(NULL) {}

			template<typename T = sdstring>
			value(sdstring&& V) : m_number(0), m_places(-1), m_boolean(false), myType(JSON_STRING), obj(NULL), pParentObject(NULL), pParentArray(NULL) {}

#if !defined USE_STD_STRING
			value(const std::string& V) : m_number(0), m_places(-1), m_boolean(false), str(V.data(), V.size()), myType(JSON_STRING), obj(NULL), pParentObject(NULL), pParentArray(NULL) {}
#endif
			value(object& V);
			value(array& V);

			template <typename T>
			value(T V) : m_number((double)V), m_places(-1), m_boolean(!(m_number == 0.0)), myType(JSON_NUMBER), obj(NULL), pParentObject(NULL), pParentArray(NULL) {}

			~value();

			void setParentObject(object* pSetTo);

			void setParentArray(array* pSetTo);

			value& operator=(const value& V);
			value& operator=(value&& V);

			int isA() const;
			bool isA(int i) const
			{
				return (isA() == i);
			}

			bool IsVoid() { return (isA() == JSON_VOID); }
			bool IsNull() { return (isA() == JSON_NULL); }
			bool IsBoolean() { return (isA() == JSON_BOOLEAN); }
			bool IsNumber() { return (isA() == JSON_NUMBER); }
			bool IsString() { return (isA() == JSON_STRING); }
			bool IsArray() { return (isA() == JSON_ARRAY); }
			bool IsObject() { return (isA() == JSON_OBJECT); }

			value& emptyArray();
			value& emptyObject();

			value& toArray();
			value& toObject(const sdstring& key);
			value& toObject(const char * key);
			value& toString();
			value& toString(int iDecimalPlaces);
			value& toNumber();
			value& toBool();
			value& toNull();
			value& fixedDecimal(int iPlaces);

			double number() const;
			float _float() const;
			i64 integer() const;
			ui64 _uint64() const;
			int _int() const;
			i64 _integer64() const;
			double _double() const;
			size_t _size_t() const;
			long _long() const;
			short _short() const;
			char _char() const;
			unsigned int _uint() const;
			unsigned long _ulong() const;
			unsigned short _ushort() const;
			unsigned char _uchar() const;

			bool boolean() const;
			std::string& string();
			sdstring& _sdstring();
			//        std::string& string();
			DEPRECATED (const char* safeCString());
			const char* c_str();
			const char* cString();

			value& operator[](size_t index);
			value& operator[](const sdstring& index);

			void push_back(const value& val);				// Array
			void push_back(value&& val);					// Array
			void push_front(const value& val);				// Array

			value pop_back();								// Array
			value pop_front();							   	// Array

			value & front();								// Array / Object
			value & back();									// Array / Object

			void erase(size_t index);					   	// Array
			size_t erase(const sdstring &index);			// Object
			iterator erase(iterator it);					// Array / Object
			iterator erase(iterator first, iterator last);	// Array / Object

			bool exists(size_t index);
			bool exists(const sdstring& index);

			iterator insert(size_t index, value& V);			  // Array
			iterator insert(const sdstring& index, value& V);		 // Object
			iterator insert(iterator position, const sdstring& key, value& V);		 // Object
			iterator insert(iterator position, value &V);		 // Array
			void insert(iterator position, iterator first, iterator last);  // Array / Object (position ignored unless ojson)
			void insert(iterator first, iterator last);	 // Array (append) / Object

			void resize(size_t iCount);
			void resize(size_t iCount, value &val);

			bool pruneEmptyValues();

			bool compact();

			bool empty() const; // Is array empty or object empty or string empty.  Number and booleans return false, NULL and VOID return true.

			std::string getKey(size_t index); // VERY slow for objects.  Don't use inside a for loop.  Use iterators instead.

			value& at(size_t index);

			bool operator==(const value& V) const;
			bool operator!=(const value& V) const;
			bool operator>(const value& V) const;
			bool operator<(const value& V) const;
			bool operator>=(const value& V) const;
			bool operator<=(const value& V) const;

			value operator+(const value& V) const;
			value operator-(const value& V) const;
			value operator*(const value& V) const;
			value operator/(const value& V) const;
			value operator%(const value& V) const;

			value &operator+=(const value& V);
			value &operator-=(const value& V);
			value &operator*=(const value& V);
			value &operator/=(const value& V);
			value &operator%=(const value& V);

			value operator++(int);
			value &operator++();
			value operator--(int);
			value &operator--();

			value operator-() const;

			size_t size() const;
			size_t arraySize();
			size_t length();
			void clear();
			void destroy();

			void sort(bool (*compareFunc)(value&, value&));

			value simpleSearch(value& searchFor, bool bSubStr = false);
			size_t simpleCount(value& searchFor, bool bSubStr = false);
			value merge(value& V);

			iterator begin() const;
			iterator end() const;
			reverse_iterator rbegin() const;
			reverse_iterator rend() const;
			iterator find(size_t index) const;
			iterator find(const sdstring& index) const;
			iterator find(const char* index) const;
			reverse_iterator rfind(size_t index) const;
			reverse_iterator rfind(const sdstring& index) const;
			reverse_iterator rfind(const char* index) const;

			typedef void (*DEBUGPTR)(const char *, ...);
			void debugPrint() const { if (debug) { debug("%s\n", print(0, true).c_str()); } }
			static void setDebug(DEBUGPTR setTo) { debug = setTo; }

			static const char* typeName(JSONTypes type);
			const std::string& key() { return m_key; }

		protected:
			sdstring &stringC(sdstring &dest)const;

			void cprint(MovingCharPointer& ptr, size_t depth = 1, bool bPretty = false) const;
			sdstring print(size_t depth = 0, bool bPretty = false) const;

			size_t psize(size_t depth, bool bPretty) const;

			double m_number;
			int m_places;
			bool m_boolean;
			sdstring str;
			JSONTypes myType;
			union {
					object* obj;
					array* arr;
			};
			sdstring m_key;

			object* pParentObject;
			array* pParentArray;
			static DEBUGPTR debug;

		private:
			static sdstring &makeStringFromNumber(sdstring & in, int iPlaces, double dNumber);
	};

	void numberParse(value& ret, instring& s, bool* bFailed);
	void SkipWhitespace(instring& in);

	class instring
	{
		public:
			instring(const sdstring& in);
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

			instring& operator=(const sdstring& in);
			instring& operator=(const char* in);
			instring& operator=(const instring& in);
			instring& operator=(instring&& in);

			void set(const sdstring& in);
			void set(const char* in);


			operator sdstring() const
			{
				return sdstring(str);
			}

			instring operator+(sdstring& V) const;
			instring operator+(const char* V) const;
			instring operator+(double V) const;

			sdstring Str() const;
			sdstring SoFar() const;
		private:
			char* str;
			char* wpos;
			size_t m_size;
	};
#if defined _USE_ADDED_ORDER_
	typedef arbitrary_order_map<sdstring, value> myMap;
#else
	typedef std::map<sdstring, value, std::less<sdstring>, secure_delete_allocator<std::pair<const sdstring, value>>> myMap;
#endif
	typedef std::deque<value, secure_delete_allocator<value>> myVec;

	class object : public myMap
	{
		public:
			object() : myMap(), bNotEmpty(false), pParentArray(nullptr), pParentObject(nullptr) {}
			~object() {
			}

			object(const object& V) : myMap((myMap)V), bNotEmpty(V.bNotEmpty), pParentArray(nullptr), pParentObject(nullptr) {}

			object(object&& V) : myMap((myMap)V)
			{
				std::swap(bNotEmpty, V.bNotEmpty);
				std::swap(pParentArray, V.pParentArray);
				std::swap(pParentObject, V.pParentObject);
			}

			object(const object* V) : myMap(static_cast<myMap>(*V)), bNotEmpty(V->bNotEmpty), pParentArray(nullptr), pParentObject(nullptr) {}
#if defined SUPPORT_ORDERED_JSON && !defined _USE_ADDED_ORDER_
			friend class ojson::object;

			object(const ojson::object& V) : myMap(V.begin(), V.end()), bNotEmpty(V.bNotEmpty), pParentArray(nullptr), pParentObject(nullptr) {}
			object(const ojson::object* V) : myMap(V->begin(), V->end()), bNotEmpty(V->bNotEmpty), pParentArray(nullptr), pParentObject(nullptr) {}
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

			object& operator=(const object& rhs)
			{
				if (this != &rhs) {
					static_cast<myMap&>(*this) = static_cast<const myMap&>(rhs);
				}
				return *this;
			}

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

			value & operator[](const sdstring & key)
			{
				return static_cast<myMap&>(*this)[key];
			}

#if defined _USE_ADDED_ORDER_
			value & operator[](size_t index)
			{
				return static_cast<myMap&>(*this)[index];
			}
#endif

			bool empty() const;
			void setNotEmpty();
			void setParentArray(array * pSetTo);
			void setParentObject(object * pSetTo);
			void cprint(MovingCharPointer& ptr, size_t depth = 1, bool bPretty = false) const;
			size_t psize(size_t depth, bool bPretty) const;
			bool notEmpty() { return bNotEmpty; }
		protected:
			bool bNotEmpty;
		private:
			array* pParentArray;
			object* pParentObject;
	};

	class array : private myVec
	{
		public:
			array() : myVec(), bNotEmpty(false), pParentArray(nullptr), pParentObject(nullptr) {}
			array(size_t C) : myVec(C), bNotEmpty(true), pParentArray(nullptr), pParentObject(nullptr) {}
#if defined SUPPORT_ORDERED_JSON && !defined _USE_ADDED_ORDER_
			friend class ojson::array;

			array(const ojson::array& V) : myVec(V.begin(), V.end()), bNotEmpty(V.bNotEmpty), pParentArray(nullptr), pParentObject(nullptr) {}
			array(const ojson::array* V) : myVec(V->begin(), V->end()), bNotEmpty(V->bNotEmpty), pParentArray(nullptr), pParentObject(nullptr) {}
#elif defined SUPPORT_ORDERED_JSON && defined _USE_ADDED_ORDER_
			friend class json::array;
			array(const json::array& V);
			array(const json::array* V);
#endif
			array(const array& V)
				: myVec((myVec)V), bNotEmpty(V.bNotEmpty), pParentArray(nullptr), pParentObject(nullptr) {}

			array(array&& V)
				: myVec((myVec)V)
			{
				std::swap(bNotEmpty, V.bNotEmpty);
				std::swap(pParentArray, V.pParentArray);
				std::swap(pParentObject, V.pParentObject);
			}

			array(const array* V) : myVec(static_cast<myVec>(*V)), bNotEmpty(V->bNotEmpty), pParentArray(nullptr), pParentObject(nullptr) {}
			~array()  {}

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
			using myVec::operator=;

			array& operator=(const array& rhs)
			{
				if (this != &rhs) {
					static_cast<myVec&>(*this) = static_cast<const myVec&>(rhs);
				}
				return *this;
			}

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
			void cprint(MovingCharPointer& ptr, size_t depth = 1, bool bPretty = false) const;
			size_t psize(size_t depth, bool bPretty) const;
			bool notEmpty() { return bNotEmpty; }
		protected:
			bool bNotEmpty;
		private:
			array* pParentArray;
			object* pParentObject;
	};

class iterator : public std::iterator<std::input_iterator_tag, value>
	{
		public:
			friend class reverse_iterator;
		
			iterator() : bNone(true), bIsArray(false), bSetKey(false) {}
			iterator(const myMap::iterator & it) : obj_it(it), bNone(false), bIsArray(false), bSetKey(false) {}
			iterator(const myVec::iterator & it) : arr_it(it), bNone(false), bIsArray(true), bSetKey(false) {}
			iterator(const iterator& it) : arr_it(it.arr_it), obj_it(it.obj_it), bNone(it.bNone), bIsArray(it.bIsArray), bSetKey(false), dumbRet() {}
			iterator(iterator&& it);
			iterator& operator=(const iterator& it);
			iterator& operator=(iterator&& it);
			~iterator();

			iterator& operator++();
			iterator operator++(int);
			iterator& operator--();
			iterator operator--(int);
#if defined _WIN32
		bool operator==(const iterator& rhs) const;
		bool operator!=(const iterator& rhs) const;
#else
			bool operator==(const iterator& rhs);
			bool operator!=(const iterator& rhs);
#endif
			value& operator*();
			value key();

			bool Neither() { return bNone; } // changed to Neither because X.h defines None as 0L in the global namespace for some stupid reason.
			bool IsArray() { return bIsArray; }

			myVec::iterator & arr() { return arr_it; }
			myMap::iterator & obj() { return obj_it; }

		private:
			myVec::iterator arr_it;
			myMap::iterator obj_it;
			bool bNone;
			bool bIsArray;
			bool bSetKey;
			value dumbRet;
	};

	class reverse_iterator : public std::reverse_iterator<iterator>
	{
		public:
			reverse_iterator() : bNone(true), bIsArray(false), bSetKey(false) {}
			reverse_iterator(const myMap::reverse_iterator & it) : obj_it(it), bNone(false), bIsArray(false), bSetKey(false) {}
			reverse_iterator(const myVec::reverse_iterator & it) : arr_it(it), bNone(false), bIsArray(true), bSetKey(false) {}
			reverse_iterator(const myMap::iterator & it) : obj_it(myMap::reverse_iterator(it)), bNone(false), bIsArray(false), bSetKey(false) {}
			reverse_iterator(const myVec::iterator & it) : arr_it(myVec::reverse_iterator(it)), bNone(false), bIsArray(true), bSetKey(false) {}
			reverse_iterator(const reverse_iterator& it) : arr_it(it.arr_it), obj_it(it.obj_it), bNone(it.bNone), bIsArray(it.bIsArray), bSetKey(false), dumbRet() {}
			reverse_iterator(const JSON_NAMESPACE::iterator& it) : arr_it(myVec::reverse_iterator(it.arr_it)), obj_it(myMap::reverse_iterator(it.obj_it)) , bNone(it.bNone), bIsArray(it.bIsArray), bSetKey(false) {}

			reverse_iterator(reverse_iterator&& it);
			reverse_iterator & operator=(const reverse_iterator& it);
			reverse_iterator & operator=(reverse_iterator&& it);
			~reverse_iterator();

			reverse_iterator& operator++();
			reverse_iterator operator++(int);
			reverse_iterator& operator--();
			reverse_iterator operator--(int);
		bool operator==(const reverse_iterator& rhs) const;
		bool operator!=(const reverse_iterator& rhs) const;
			value& operator*();
			value key();

			bool Neither() { return bNone; }
			bool IsArray() { return bIsArray; }

			myVec::reverse_iterator & arr() { return arr_it; }
			myMap::reverse_iterator & obj() { return obj_it; }

		private:
			myVec::reverse_iterator arr_it;
			myMap::reverse_iterator obj_it;
			bool bNone;
			bool bIsArray;
			bool bSetKey;
			value dumbRet;
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
			document(document&& V);

			document& operator=(const document& V);
			document& operator=(document&& V);

			typedef sdstring& (*PREPARSEPTR)(const sdstring& in, sdstring& out, sdstring fileName);
			typedef sdstring& (*PREWRITEPTR)(const sdstring& in, sdstring& out);
			bool parse(const sdstring& inStr, PREPARSEPTR = NULL, const sdstring &preParseFileName = "");

			bool parse(const char* inStr, size_t len, PREPARSEPTR = NULL, const sdstring &preParseFileName = "");

			bool parseFile(const sdstring &instr, PREPARSEPTR = NULL, bool bReWriteFile = false);

			sdstring write(bool bPretty = false, PREWRITEPTR = NULL) const;
			sdstring write(size_t iDepth, bool bPretty = false, PREWRITEPTR = NULL) const;

			sdstring print(bool bPretty = false, PREWRITEPTR = NULL) const
			{
				return write(bPretty);
			}

			bool writeFile(const sdstring &inStr, bool bPretty = false, PREWRITEPTR = NULL) const;

			sdstring parseResult() const
			{
				return strParseResult;
			}

			bool parseSuccessful() const
			{
				return bParseSuccessful;
			}

			template<typename T>
			document(T V) : value(V) { bParseSuccessful = true; }

			static int appendToArrayFile(const sdstring &sFile, const document & atm, bool bPretty);

		protected:
			sdstring strParseResult;
			bool bParseSuccessful;
	};

	inline char &instring::take() {
		return *(wpos++);
	}

	inline void instring::skip() {
		++wpos;
	}

	inline char &instring::peek() const
	{
		return *(wpos);
	}

	inline size_t instring::tell() const
	{
		return wpos - str;
	}

	inline size_t instring::size() const
	{
		return m_size;
	}

	std::ostream& operator<<(std::ostream& S, document& doc);
	std::ostream& operator<<(std::ostream& S, value& doc);

#if !defined _USE_ADDED_ORDER_
	template <typename T>
	const char * enumKey(T e, const char * keyString)
	{
		(void) e;
		return keyString;
	}
#define JSONENUMKEY(x, y) json::enumKey<x>(y, #y)
#endif
}

#undef JSON_NAMESPACE

#endif /* JSON_HPP_ */

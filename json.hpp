/*
Copyright (c) 2012-2014 James Baker

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
 json::document doc;
 
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

#ifndef JSON_HPP_
#define JSON_HPP_

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

void JSONDebug(const char *format, ...);

namespace json
{
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
	 *			json::document class.
	 */
	class atom
	{
	public:
		
		friend class object;
		friend class array;
		friend class document;
		friend class query;
		friend class iterator;
		
		friend void objectParse(atom& ret, instring& inputString, bool* bFailed);
		
		friend void arrayParse(atom& arr, instring& inputString, bool* bFailed);
		
		friend void nullParse(atom& ret, instring& inputString, bool* bFailed);
		friend void valueParse(atom& a, instring& inputString, bool* bFailed);
        friend void numberParse(atom& ret, instring& s, bool* bFailed);
		static void swap(atom& lhs, atom& rhs) {
			using std::swap;
			swap(lhs.m_number, rhs.m_number);
			swap(lhs.m_boolean, rhs.m_boolean);
			swap(lhs.str, rhs.str);
			swap(lhs.myType, rhs.myType);
			swap(lhs.obj, rhs.obj);
			swap(lhs.arr, rhs.arr);
		}
		
		atom();
		atom(const atom& V);
#ifdef __BORLANDC__
		atom(document& V);
#else
		atom(const document& V);
#endif
		atom(bool V);
		atom(const char* V);
		atom(char* V);
		atom(std::string V);
		atom(object& V);
		atom(array& V);
		
		template <typename T>
		atom(T V) {
			m_number = (double)V;
			m_boolean = !(V == 0.0);
			
			myType = JSON_NUMBER;
			obj = NULL;
			arr = NULL;
		}
		
		~atom();
		
		atom& operator=(const atom& V);
		
		int isA() const;
		bool isA(int i) const
		{
			return (isA() == i);
		}

		atom& emptyArray();
		atom& emptyObject();
		
		double number();
		i64 integer();
		bool boolean();
		std::string& string();
		const char* safeCString();
		const char* c_str();
		const char* cString();
		
		atom& operator[](size_t index);
		atom& operator[](std::string index);
		
		void push_back(atom val);									   // Array
		void push_front(atom val);									  // Array
		
		atom pop_back();												// Array
		atom pop_front();											   // Array
		
		void erase(size_t index);									   // Array
		void erase(std::string index);								  // Object
		void erase(iterator it);										// Array / Object
		void erase(iterator first, iterator last);					  // Array / Object

		bool exists(size_t index);
		bool exists(std::string index);
		
		iterator insert(size_t index, atom V);							  // Array
		iterator insert(std::string index, atom V);						 // Object
		iterator insert(iterator position, atom V);						 // Array / Object
		void insert(iterator position, iterator first, iterator last);  // Array / Object (position ignored)
		void insert(iterator first, iterator last);					 // Array (append) / Object
		
		bool empty() const; // Is array empty or object empty or string empty.  Number and booleans return false, NULL and VOID return true.
		
		std::string getKey(size_t index); // VERY slow for objects.  Don't use inside a for loop.  Use iterators instead.
		
		// atom& get(std::string index, atom V);
		// atom& get(size_t index, atom V);
		
		atom& at(size_t index);
		
		// void set(std::string index, atom V);
		// void set(int index, atom V);
		
		bool operator==(atom V) const;
		bool operator!=(atom V) const;
		bool operator>(atom V) const;
		bool operator<(atom V) const;
		bool operator>=(atom V) const;
		bool operator<=(atom V) const;
		
		atom operator+(atom V) const;
		atom operator-(atom V) const;
		atom operator*(atom V) const;
		atom operator/(atom V) const;
		atom operator%(atom V) const;
		
		atom &operator+=(atom V);
		atom &operator-=(atom V);
		atom &operator*=(atom V);
		atom &operator/=(atom V);
		atom &operator%=(atom V);
		
		atom operator++(int);
		atom &operator++();
		atom operator--(int);
		atom &operator--();
		
		atom operator-();
		
		size_t size() const;
        void clear();
        void destroy();
		
		void sort(bool (*compareFunc)(atom, atom));
		
		atom simpleSearch(atom& searchFor, bool bSubStr = false);
		size_t simpleCount(atom& searchFor, bool bSubStr = false);
		virtual atom merge(atom& V);
		
		iterator begin() const;
		iterator end() const;
		reverse_iterator rbegin() const;
		reverse_iterator rend() const;
		iterator find(size_t index);
		iterator find(std::string index);
		reverse_iterator rfind(size_t index);
		reverse_iterator rfind(std::string index);
		
		typedef void (*DEBUGPTR)(const char *, ...);
		void debugPrint() const { if (debug) { debug("%s\n", print(0, true).c_str()); } }
		static void setDebug(DEBUGPTR setTo) { debug = setTo; }

		static std::string typeName(JSONTypes type);
	protected:
		
		void cprint(MovingCharPointer& ptr, int depth = 1, bool bPretty = false) const;
		std::string print(int depth = 0, bool bPretty = false) const;
		
		size_t psize(int depth, bool bPretty) const;
		
		double m_number;
		bool m_boolean;
		std::string str;
		JSONTypes myType;
		object* obj;
		array* arr;
		
		static DEBUGPTR debug;
	};
	
    void numberParse(atom& ret, instring& s, bool* bFailed);
    
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
		size_t size;
	};

	typedef std::map<std::string, json::atom> myMap;
	
	typedef std::deque<json::atom> myVec;

	class object : public myMap
	{
	public:
		object()
		: myMap() {
		}
		~object() {
		}
		
		object(const object& V)
		: myMap((myMap)V) {
		}
		object(const object* V)
		: myMap(*(myMap*)V) {
		}
		
		bool empty() const;
		void cprint(MovingCharPointer& ptr, int depth = 1, bool bPretty = false) const;
		size_t psize(int depth, bool bPretty) const;
	};
	
	class array : public myVec
	{
	public:
		array()
		: myVec() {
		}
		array(int C)
		: myVec(C) {
		}
		array(const array& V)
		: myVec((myVec)V) {
		}
		array(const array* V)
		: myVec(*(myVec*)V) {
		}
		~array() {
		}
		
		bool empty() const;
		void cprint(MovingCharPointer& ptr, int depth = 1, bool bPretty = false) const;
		size_t psize(int depth, bool bPretty) const;
	};
	
	class iterator : public std::iterator<std::input_iterator_tag, atom>{
	public:
        friend class json::reverse_iterator;
		iterator() {
			bNone = true;
			bIsArray = false;
		}
		iterator(const myMap::iterator & it) {
			bNone = false;
			obj_it = it;
//			arr_it = NULL;
			bIsArray = false;
		}
		iterator(const myVec::iterator & it) {
			bNone = false;
//			obj_it = NULL;
			arr_it = it;
			bIsArray = true;
		}
		iterator(const iterator& it) {
			bNone = it.bNone;
			arr_it = it.arr_it;
			obj_it = it.obj_it;
			bIsArray = it.bIsArray;
		}
		~iterator() {}
		
		iterator& operator++() {
			if (!bNone) {
				if (bIsArray) {
					++arr_it;
				} else {
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
		
		atom& operator*() {
			if (!bNone) {
				if (bIsArray) {
					return *arr_it;
				} else {
					return obj_it->second;
				}
			} else {
				dumbRet = atom();
				return dumbRet;
			}
		}
		atom key() {
			if (!bNone) {
				if (bIsArray)
					return atom();
				else
					return atom(obj_it->first);
			} else {
				return atom();
			}
		}
		
		bool None() { return bNone; }
		bool IsArray() { return bIsArray; }
		
		myVec::iterator & arr() { return arr_it; }
		myMap::iterator & obj() { return obj_it; }
		
	private:
		atom dumbRet;
		bool bNone;
		myVec::iterator arr_it;
		myMap::iterator obj_it;
		bool bIsArray;
	};
	
	class reverse_iterator : public std::iterator<std::input_iterator_tag, atom>{
	public:
		reverse_iterator() {
			bNone = true;
			bIsArray = false;
		}
		reverse_iterator(const myMap::reverse_iterator & it) {
			bNone = false;
			obj_it = it;
            //			arr_it = NULL;
			bIsArray = false;
		}
		reverse_iterator(const myVec::reverse_iterator & it) {
			bNone = false;
            //			obj_it = NULL;
			arr_it = it;
			bIsArray = true;
		}
		reverse_iterator(const myMap::iterator & it) {
			bNone = false;
			obj_it = myMap::reverse_iterator(it);
            //			arr_it = NULL;
			bIsArray = false;
		}
		reverse_iterator(const myVec::iterator & it) {
			bNone = false;
            //			obj_it = NULL;
			arr_it =  myVec::reverse_iterator(it);
			bIsArray = true;
		}
		reverse_iterator(const reverse_iterator& it) {
			bNone = it.bNone;
			arr_it = it.arr_it;
			obj_it = it.obj_it;
			bIsArray = it.bIsArray;
		}
        
		reverse_iterator(const json::iterator& it) {
			bNone = it.bNone;
			arr_it = myVec::reverse_iterator(it.arr_it);
			obj_it = myMap::reverse_iterator(it.obj_it);
			bIsArray = it.bIsArray;
		}
		~reverse_iterator() {}
		
		reverse_iterator& operator++() {
			if (!bNone) {
				if (bIsArray) {
					++arr_it;
				} else {
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
		
		atom& operator*() {
			if (!bNone) {
				if (bIsArray) {
					return *arr_it;
				} else {
					return obj_it->second;
				}
			} else {
				dumbRet = atom();
				return dumbRet;
			}
		}
		atom key() {
			if (!bNone) {
				if (bIsArray)
					return atom();
				else
					return atom(obj_it->first);
			} else {
				return atom();
			}
		}
		
		bool None() { return bNone; }
		bool IsArray() { return bIsArray; }
		
		myVec::reverse_iterator & arr() { return arr_it; }
		myMap::reverse_iterator & obj() { return obj_it; }
		
	private:
		atom dumbRet;
		bool bNone;
		myVec::reverse_iterator arr_it;
		myMap::reverse_iterator obj_it;
		bool bIsArray;
	};
	
	
	class document : public atom
	{
	public:
		friend class atom;
		document()
		: atom() {
			bParseSuccessful = false;
		}
		
		document(const std::string& in);
		document(const char*);
		
		document(const atom& V)
		: atom(V) {
			bParseSuccessful = true;
		}
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
		
		static int appendToArrayFile(std::string sFile, const document & atm, bool bPretty);

	protected:
		std::string strParseResult;
		bool bParseSuccessful;
	};
		
	std::ostream& operator<<(std::ostream& S, document& doc);
	std::ostream& operator<<(std::ostream& S, atom& doc);
}
#if defined __BORLANDC__ && __BORLANDC__ < 0x0600
#pragma warn + 8026
#pragma warn + 8027
#endif

#endif /* JSON_HPP_ */

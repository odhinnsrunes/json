/*
Copyright (c) 2012-2016 James Baker

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
#include "json.hpp"

namespace json
{
		class query : public document
	{
	public:
		query()
		: document() {
			depth = 0;
			m_to_search = NULL;
		}
		query(const char* V)
		: document(V) {
			depth = 0;
			m_to_search = NULL;
		}
		query(char* V)
		: document(V) {
			depth = 0;
			m_to_search = NULL;
		}
		
		query(value& V)
		: document(V) {
			depth = 0;
			m_to_search = NULL;
		}
		
		query(const char* V, value* insearch)
		: document(V) {
			depth = 0;
			m_to_search = NULL;
			setTarget(insearch);
		}
		query(char* V, value* insearch)
		: document(V) {
			depth = 0;
			m_to_search = NULL;
			setTarget(insearch);
		}
		
		template <typename T>
		query(T V, value* insearch)
		: document(V) {
			depth = 0;
			m_to_search = NULL;
			setTarget(insearch);
		}
		
		query(value& V, value* insearch)
		: document(V) {
			depth = 0;
			m_to_search = NULL;
			setTarget(insearch);
		}
		
		void setTarget(value* insearch);
		
		document& getResult() {
			depth = 0;
			*(value*)(&result) = doresult(*m_to_search);
			return result;
		}
		
		document& operator=(value& val) {
			depth = 0;
			*(value*)(&result) = doresult(*m_to_search, eAssign, val);
			return result;
		}
		
		document& remove() {
			depth = 0;
			*(value*)(&result) = doresult(*m_to_search, eRemove);
			return result;
		}
		
	private:
		enum eOperators{
			eNoOp = 0,
			eAssign,
			eIncrement,
			ePreIncrement,
			eDecrement,
			ePreDecrement,
			eAddTo,
			eSubtractFrom,
			eMultiplyBy,
			eDivideBy,
			eRemove
		};
		
		bool logic(value& working, value& params);
		
		value doresult(value& working, eOperators op = eNoOp, value val = value());
		
		value* m_to_search;
		array::iterator aIt;
		object::iterator oIt;
		
		document result;
		size_t depth;
	};
}

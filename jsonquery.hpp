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
		
		query(atom& V)
		: document(V) {
			depth = 0;
			m_to_search = NULL;
		}
		
		query(const char* V, atom* insearch)
		: document(V) {
			depth = 0;
			m_to_search = NULL;
			setTarget(insearch);
		}
		query(char* V, atom* insearch)
		: document(V) {
			depth = 0;
			m_to_search = NULL;
			setTarget(insearch);
		}
		
		template <typename T>
		query(T V, atom* insearch)
		: document(V) {
			depth = 0;
			m_to_search = NULL;
			setTarget(insearch);
		}
		
		query(atom& V, atom* insearch)
		: document(V) {
			depth = 0;
			m_to_search = NULL;
			setTarget(insearch);
		}
		
		void setTarget(atom* insearch);
		
		document& getResult() {
			depth = 0;
			*(atom*)(&result) = doresult(*m_to_search);
			return result;
		}
		
		document& operator=(atom& value) {
			depth = 0;
			*(atom*)(&result) = doresult(*m_to_search, eAssign, value);
			return result;
		}
		
		document& remove() {
			depth = 0;
			*(atom*)(&result) = doresult(*m_to_search, eRemove);
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
		
		bool logic(atom& working, atom& params);
		
		atom doresult(atom& working, eOperators op = eNoOp, atom value = atom());
		
		atom* m_to_search;
		array::iterator aIt;
		object::iterator oIt;
		
		document result;
		size_t depth;
	};
}

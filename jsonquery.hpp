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

#include "jsonquery.hpp"

namespace json
{
		void query::setTarget(atom* insearch) {
		m_to_search = insearch;
		result.clear();
		switch (m_to_search->myType) {
		case JSON_ARRAY:
			aIt = m_to_search->arr->begin();
			break;
		case JSON_OBJECT:
			oIt = m_to_search->obj->begin();
		default:
			break;
		}
	}

	bool query::logic(atom& working, atom& params) {
		bool bRetVal = false;
		if (params.isA(JSON_ARRAY)) {
			for (array::iterator it = params.arr->begin(); it != params.arr->end(); ++it) {
				if (working.isA(JSON_ARRAY)) {

					std::string op = (*it)["operator"].string();
					bool bTemp = false;
					if (op == "==") {
						if (working[(size_t)(*it)["key"].integer()] == (*it)["value"]) {
							bTemp = true;
						}
					} else if (op == ">") {
						if (working[(size_t)(*it)["key"].integer()] > (*it)["value"]) {
							bTemp = true;
						}
					} else if (op == ">=") {
						if (working[(size_t)(*it)["key"].integer()] >= (*it)["value"]) {
							bTemp = true;
						}
					} else if (op == "<") {
						if (working[(size_t)(*it)["key"].integer()] < (*it)["value"]) {
							bTemp = true;
						}
					} else if (op == "<=") {
						if (working[(size_t)(*it)["key"].integer()] <= (*it)["value"]) {
							bTemp = true;
						}
					} else if (op == "!=") {
						if (working[(size_t)(*it)["key"].integer()] != (*it)["value"]) {
							bTemp = true;
						}
					}
					if ((*it)["not"].boolean()) {
						bTemp = !bTemp;
					}
					if ((*it)["and"].boolean()) {
						bTemp = bTemp && logic(working, (*it)["and"]);
					}
					if (bTemp) {
						bRetVal = true;
						break;
					}
				} else if (working.isA(JSON_OBJECT)) {

					std::string op = (*it)["operator"].string();

					bool bTemp = false;
					if (op == "==") {
						if (working[(*it)["key"].safeCString()] == (*it)["value"]) {
							bTemp = true;
						}
					} else if (op == ">") {
						if (working[(*it)["key"].safeCString()] > (*it)["value"]) {
							bTemp = true;
						}
					} else if (op == ">=") {
						if (working[(*it)["key"].safeCString()] >= (*it)["value"]) {
							bTemp = true;
						}
					} else if (op == "<") {
						if (working[(*it)["key"].safeCString()] < (*it)["value"]) {
							bTemp = true;
						}
					} else if (op == "<=") {
						if (working[(*it)["key"].safeCString()] <= (*it)["value"]) {
							bTemp = true;
						}
					} else if (op == "!=") {
						if (working[(*it)["key"].safeCString()] != (*it)["value"]) {
							bTemp = true;
						}
					}
					if ((*it)["not"].boolean()) {
						bTemp = !bTemp;
					}
					if ((*it)["and"].boolean()) {
						bTemp = bTemp && logic(working, (*it)["and"]);
					}
					if (bTemp) {
						bRetVal = true;
						break;
					}

				} else {
					std::string op = (*it)["operator"].string();
					bool bTemp = false;
					if (op == "==") {
						if (working == (*it)["value"]) {
							bTemp = true;
						}
					} else if (op == ">") {
						if (working > (*it)["value"]) {
							bTemp = true;
						}
					} else if (op == ">=") {
						if (working >= (*it)["value"]) {
							bTemp = true;
						}
					} else if (op == "<") {
						if (working < (*it)["value"]) {
							bTemp = true;
						}
					} else if (op == "<=") {
						if (working <= (*it)["value"]) {
							bTemp = true;
						}
					} else if (op == "!=") {
						if (working != (*it)["value"]) {
							bTemp = true;
						}
					}
					if ((*it)["not"].boolean()) {
						bTemp = !bTemp;
					}
					if ((*it)["and"].boolean()) {
						bTemp = bTemp && logic(working, (*it)["and"]);
					}
					if (bTemp) {
						bRetVal = true;
						break;
					}
				}
			}
		}
		return bRetVal;
	}

	atom query::doresult(atom& working, eOperators op, atom value) {
		atom retVal;
		size_t myDepth = depth++;
		if (myDepth == (*this)["from"].size()) {
			// do where and select here
			bool bDoIt = true;
			if ((*this)["where"].boolean()) {
				bDoIt = logic(working, (*this)["where"]);
			}
			if (bDoIt) {

				if ((*this)["select"].boolean() && (working.myType == JSON_OBJECT || working.myType == JSON_ARRAY)) {
					if (working.isA(JSON_ARRAY)) {
						size_t i = 0;
						for (iterator it = (*this)["select"].begin(); it!= (*this)["select"].end(); ++it, i++) {
							iterator r = retVal.find(i);
							iterator w = working.find((size_t)(*it).integer());
							switch (op) {
								case eAssign:
									(*w) = value;
									(*r) = (*w);
									break;
								case eIncrement:
									(*r) = (*w)++;
									break;
								case ePreIncrement:
									(*r) = ++(*w);
									break;
								case eDecrement:
									(*r) = (*w)--;
									break;
								case ePreDecrement:
									(*r) = --(*w);
									break;
								case eAddTo:
									(*w) += value;
									(*r) = (*w);
									break;
								case eSubtractFrom:
									(*w) -= value;
									(*r) = (*w);
									break;
								case eMultiplyBy:
									(*w) *= value;
									(*r) = (*w);
									break;
								case eDivideBy:
									(*w) /= value;
									(*r) = (*w);
									break;
								case eRemove:
									(*w).clear();
									break;

								case eNoOp:
								default:
									(*r) = (*w);
							}
						}
					} else if (working.isA(JSON_OBJECT)) {
						for (iterator it = (*this)["select"].begin(); it != (*this)["select"].end(); ++it) {
							std::string sIndex = (*it).string();
							iterator w = working.find((*it).string());
							iterator r = retVal.find((*it).string());
							switch (op) {
								case eAssign:
									(*w) = value;
									(*r) = (*w);
									break;
								case eIncrement:
									(*r) = (*w)++;
									break;
								case ePreIncrement:
									(*r) = ++(*w);
									break;
								case eDecrement:
									(*r) = (*w)--;
									break;
								case ePreDecrement:
									(*r) = --(*w);
									break;
								case eAddTo:
									(*w) += value;
									(*r) = (*w);
									break;
								case eSubtractFrom:
									(*w) -= value;
									(*r) = (*w);
									break;
								case eMultiplyBy:
									(*w) *= value;
									(*r) = (*w);
									break;
								case eDivideBy:
									(*w) /= value;
									(*r) = (*w);
									break;
								case eRemove:
									(*w).clear();
									break;
									
								case eNoOp:
								default:
									(*r) = (*w);
							}
						}
					}
				} else {
					switch (op) {
						case eAssign:
							working = value;
							retVal = working;
							break;
						case eIncrement:
							retVal = working++;
							break;
						case ePreIncrement:
							retVal = ++working;
							break;
						case eDecrement:
							retVal = working--;
							break;
						case ePreDecrement:
							retVal = --working;
							break;
						case eAddTo:
							working += value;
							retVal = working;
							break;
						case eSubtractFrom:
							working -= value;
							retVal = working;
							break;
						case eMultiplyBy:
							working *= value;
							retVal = working;
							break;
						case eDivideBy:
							working /= value;
							retVal = working;
							break;
						case eRemove:
							working.clear();
							break;

						case eNoOp:
						default:
							retVal = working;
					}
				}
			}
		} else {
			if ((*this)["from"].boolean()) {
				if ((*this)["from"][myDepth] == "*") {
					if (working.isA(JSON_ARRAY) && working.size()) {
						int i = 0;
						for (array::iterator it = working.arr->begin(); it != working.arr->end(); ++it) {
							atom r = doresult(*it, op, value);
							if (r.size() || r.myType == JSON_STRING || myDepth < (*this)["from"].size() - 1)
								retVal[i++] = r;
						}
					} else if (working.isA(JSON_OBJECT) && working.size()) {
						for (object::iterator it = working.obj->begin(); it != working.obj->end(); ++it) {
							atom r = doresult(it->second, op, value);
							if (r.size() || r.myType == JSON_STRING)
								retVal[it->first] = r;
						}
					}
				} else {
					if ((*this)["from"][myDepth].isA(JSON_STRING) && working.isA(JSON_OBJECT)) {
						if (working[(*this)["from"][myDepth].safeCString()].myType != JSON_VOID) {
							atom r = doresult(working[(*this)["from"][myDepth].safeCString()], op, value);
							retVal[(*this)["from"][myDepth].safeCString()] = r;
						}
					} else if ((*this)["from"][myDepth].isA(JSON_NUMBER) && working.isA(JSON_ARRAY)) {
						if (working[(int)(*this)["from"][myDepth].number()].myType != JSON_VOID) {
							atom r = doresult(working[(int)(*this)["from"][myDepth].number()], op, value);
							retVal[0] = r;
						}
					}
				}
			}
		}
		depth--;
		return retVal;
	}
}

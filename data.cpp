#include "data.hpp"
#include <iomanip>
#include "tinyxml/tinyxml.h"
#include "tinyxml/tinystr.h"

namespace data
{

int isNumeric(std::string data) {
	if (data.size() > JSON_NUMBER_PRECISION) {
		return 0;
	}
	const char szOk[] = "1234567890";
	bool bHaveDot = false;
	for (std::string::iterator it = data.begin(); it != data.end(); ++it) {
		bool bOk = false;
		for (size_t i = 0; i < 10; i++) {
			if (szOk[i] == (*it)) {
				bOk = true;
				break;
			} else if (i == 0 && (*it) == '-') {
				bOk = true;
				break;
			} else if ((*it) == '.') {
				if (bHaveDot) {
					return 0;
				} else {
					bHaveDot = true;
					bOk = true;
					break;
				}
			}
		}
		if (bOk == false) {
			return 0;
		}
	}
	if (bHaveDot) {
		return 2;
	} else {
		return 1;
	}
}

void document::parseXMLElement(json::atom & ret, const TiXmlNode * elem)
{
	if (elem) {
		switch (elem->Type()) {
			default:
				break;

			case TiXmlNode::DOCUMENT:
				{				
					TiXmlElement * rootElem = ((TiXmlDocument*)elem)->RootElement();
					const TiXmlAttribute * att = rootElem->FirstAttribute();
					while (att) {
						std::string childName = "@";
						childName.append(att->Name());
						ret[childName] = att->Value();
						att = att->Next();
					}
					sRootTag = rootElem->Value();
					for (TiXmlElement * child = rootElem->FirstChildElement(); child; child = child->NextSiblingElement() ) {
						if (child != NULL) {
							switch (child->Type()) {
								case TiXmlNode::ELEMENT:
								{
									std::string childName = child->Value();
									if (childName[0] == '_' && strchr("1234567890", childName[1])) {
										childName = childName.substr(1);
									}
									if (ret.exists(childName)) {
										if (!ret[childName].isA(json::JSON_ARRAY)) {
											json::atom a = ret[childName];
											ret[childName][0] = a;
										}
										parseXMLElement(ret[childName][ret[childName].size()], child);
									} else {
										parseXMLElement(ret[childName], child);
									}
									break;
								}
									
								default:
									parseXMLElement(ret, child);
									break;
							}
						}
					}
					break;
				}
				
			case TiXmlNode::ELEMENT:
				{
					const TiXmlAttribute * att = elem->ToElement()->FirstAttribute();
					while (att) {
						std::string childName = "@";
						childName.append(att->Name());
						ret[childName] = att->Value();
						att = att->Next();
					}
					if (elem->Value()) {
						const TiXmlNode *child = NULL;
                        bool bEmpty = true;
						while ((child = elem->IterateChildren(child))) {
                            bEmpty = false;
							switch (child->Type()) {
								case TiXmlNode::ELEMENT:
									{
										std::string childName = child->Value();
										if (childName[0] == '_' && strchr("1234567890", childName[1])) {
											childName = childName.substr(1);
										}
										if (ret.exists(childName)) {
											if (!ret[childName].isA(json::JSON_ARRAY)) {
												json::atom a = ret[childName];
												ret.erase(childName);
												ret[childName][0] = a;
											}
											parseXMLElement(ret[childName][ret[childName].size()], child);
										} else {
											parseXMLElement(ret[childName], child);
										}
										break;
									}
	
								default:
									parseXMLElement(ret, child);
									break;
							}
						}
                        if (bEmpty){
                            if (ret.isA(json::JSON_OBJECT)){
                                ret["#value"] = "";
                            } else {
                                ret = "";
                            }
                        }
					}
					break;
				}

			case TiXmlNode::TEXT:
				if (elem->Value()) {
					int iValType = isNumeric(elem->Value());
					if (iValType != 0) {
                        std::string sValue = elem->Value();
                        
                        json::instring in(sValue);
                        bool bFailed = false;
                        if (ret.isA(json::JSON_OBJECT)){
                        	json::numberParse(ret["#value"], in, &bFailed);
                        } else {
                        	json::numberParse(ret, in, &bFailed);
                        }
					} else {
						if (ret.isA(json::JSON_OBJECT)){
							ret["#value"] = elem->Value();
						} else {
							ret = elem->Value();
						}
					}
				}
				break;
		}
	}
}

bool document::parseXML(std::string inStr, PREPARSEPTR preParser, std::string preParseFileName)
{
	if (arr)
		delete arr;
	arr = NULL;
	if (obj)
		delete obj;
	obj = NULL;
	myType = json::JSON_VOID;
	m_number = 0;
	m_boolean = false;
	str.clear();

	TiXmlDocument doc;
	doc.SetCondenseWhiteSpace(false);
	if (preParser == NULL) {
		doc.Parse(inStr.c_str());
	} else {
		std::string sOut;
		preParser(inStr, sOut, preParseFileName);
		if (sOut.size() == 0) {
			bParseSuccessful = false;
			strParseResult = "XML Document failed to pre-parse.";
			if (debug) {
				debug("%s", strParseResult.c_str());
				debug("%s", inStr.c_str());
			}
			return false;
		}
		doc.Parse(sOut.c_str());
	}
	if (doc.Error()) {
		bParseSuccessful = false;
		strParseResult = "XML Parsing failed: " + std::string(doc.ErrorDesc());
		return false;
	}
	sRootTag.clear();
	parseXMLElement(*this, &doc);

	bParseSuccessful = true;
	strParseResult = "Ok.";
	return true;
}

bool document::parseXMLFile(std::string inStr, PREPARSEPTR preParser, bool bReWriteFile)
{
		FILE* fd = fopen(inStr.c_str(), "rb");
		if (fd) {
			fseek(fd, 0, SEEK_END);
			size_t l = ftell(fd);
			fseek(fd, 0, SEEK_SET);
			char* buffer = new char[l + 1];

			buffer[l] = 0;
			fread(buffer, 1, l, fd);

			fclose(fd);
			bool bRetVal;
			std::string sDat(buffer, l);
			if (bReWriteFile) {
				bRetVal = parseXML(sDat, preParser, inStr);
			} else {
				bRetVal = parseXML(sDat, preParser);
			}
			bParseSuccessful = bRetVal;
			delete[] buffer;
			return bRetVal;
		}
		strParseResult = "Couldn't open file " + inStr;
		bParseSuccessful = false;
		return false;
}
	
	std::string XMLEscape(const std::string& in) {
		std::string out;
		for (std::string::const_iterator it = in.begin(); it != in.end(); ++it) {
			switch (*it) {
				default:
					out.push_back(*it);
					break;
				case '&':
					out.append("&amp;");
					break;
				case '\'':
					out.append("&apos;");
					break;
				case '\"':
					out.append("&quot;");
					break;
				case '>':
					out.append("&gt;");
					break;
				case '<':
					out.append("&lt;");
					break;
			}
		}
		return out;
	}

void document::writeXML(std::string & str, json::atom & ret, int depth, bool bPretty, bool bTabs)
{
	switch (ret.isA()) {
		default:
			break;
		case json::JSON_BOOLEAN:
			// if (bPretty) {
			// 	for (int i = 0; i < depth; i++) {
			// 		str.append("\t");
			// 	}
			// }
			str.append(ret.boolean() ? "true" : "false");
			break;

		case json::JSON_NUMBER:
		{
            str.append(XMLEscape(ret.string()));
			break;
		}
		case json::JSON_STRING:
			str.append(XMLEscape(ret.string()));
			break;

		case json::JSON_OBJECT:
		{
			for (json::iterator it = ret.begin(); it != ret.end(); ++it) {
				std::string key = it.key().c_str();
				if (key.size() > 1) {
					if (key[0] == '@') {
						continue;
					}
				}
				if (key == "#value") {
					str.append(XMLEscape((*it).string()));
					continue;
				}
				bool bIsNumericKey = false;
				if (strchr("1234567890", key[0])) {
					bIsNumericKey = true;
				}
//			for (size_t i = 0; i < ret.size(); i++) {
//				std::string key = ret.getKey((int)i);
				if (bPretty && str.size()) {
					if (str[str.size() - 1] != '\n')
						str.append("\n");
					
					if (bTabs){
						for (int k = 0; k < depth; k++) {
							str.append("\t");
						}
					}
				}
				if (ret[key].isA(json::JSON_ARRAY)) {
					for (size_t j = 0; j < ret[key].size(); j++) {
						if (bPretty && bTabs && j > 0) {
							for (int i = 0; i < depth; i++) {
								str.append("\t");
							}
						}
						str.push_back('<');
						if (bIsNumericKey) {
							str.append("_");
						}
						str.append(key);
						for (json::iterator it = ret[key][j].begin(); it != ret[key][j].end(); ++it) {
							std::string subKey = it.key().string();
							if (subKey.size() > 1) {
								if (subKey[0] == '@') {
									str.push_back(' ');
									str.append(subKey.substr(1));
									str.append("=\"");
									str.append((*it).string());
									str.push_back('\"');
								}
							}
						}
						str.push_back('>');
						writeXML(str, ret[key][j], depth + 1, bPretty, bTabs);
						if (bPretty && bTabs && str[str.size() - 1] == '\n') {
							for (int k = 0; k < depth; k++) {
								str.append("\t");
							}
						}
						str.append("</");
						if (bIsNumericKey) {
							str.append("_");
						}
						str.append(key);
						str.push_back('>');
						//str = str + "</" + key + ">";
						if (bPretty)
							str.append("\n");
					}
				} else {
					str.push_back('<');
					if (bIsNumericKey) {
						str.append("_");
					}
					str.append(key);
					for (json::iterator it = ret[key].begin(); it != ret[key].end(); ++it) {
						std::string subKey = it.key().string();
						if (subKey.size() > 1) {
							if (subKey[0] == '@') {
								str.push_back(' ');
								str.append(subKey.substr(1));
								str.append("=\"");
								str.append((*it).string());
								str.push_back('\"');
							}
						}
					}
					str.push_back('>');
					writeXML(str, ret[key], depth + 1, bPretty, bTabs);
					if (bPretty && bTabs && str[str.size() - 1] == '\n') {
						for (int k = 0; k < depth; k++) {
							str.append("\t");
						}
					}
					str.append("</");
					if (bIsNumericKey) {
						str.append("_");
					}
					str.append(key);
					str.push_back('>');
//					str = str + "</" + key + ">";
					if (bPretty)
						str.append("\n");
				}
			}
			break;
		}

	}
}

std::string document::writeXML(std::string rootElem, bool bPretty, bool bTabs, PREWRITEPTR preWriter)
{
	std::string ret;
	if (rootElem.size()) {
		sRootTag = rootElem;
	}
	int iStartDepth = 0;
	if (sRootTag.size()) {
		ret = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";
		iStartDepth = 1;
		if (bPretty) {
			ret.append("\n");
		}
	}
	if (sRootTag.size()) {
		ret.append("<");
		ret.append(sRootTag);
		ret.append(">");
		if (bPretty) {
			ret.append("\n");
		}
	}

	
	if (isA(json::JSON_OBJECT)) {
		// std::string key = this->getKey(0);
		// ret = ret + "<" + key + ">"
		writeXML(ret, *(json::atom*)this, iStartDepth, bPretty, bTabs);
		// ret = ret + "</" + key + ">"
		// if (bPretty)
		// 	ret.append("\n");
	}
	if (sRootTag.size()) {
		ret.append("</");
		ret.append(sRootTag);
		ret.append(">");
	}

	if (preWriter != NULL) {
		std::string sOut;
		return preWriter(ret, sOut);
	}

	return ret;
}

bool document::writeXMLFile(std::string inStr, std::string rootElem, bool bPretty, bool bTabs, PREWRITEPTR preWriter)
{
	FILE* fd = fopen(inStr.c_str(), "wb");
	if (fd) {
		std::string w = writeXML(rootElem, bPretty, bTabs, preWriter);
		fwrite(w.data(), 1, w.size(), fd);
		fclose(fd);
		return true;
	}
	return false;
}

void document::stripNameSpaces(json::atom & a, json::document jNameSpaces, bool begin)
{
	if(begin){
		for(json::iterator it = jNameSpaces.begin(); it != jNameSpaces.end(); ++it){
			std::string sNS = (*it).string();
			if(sNS[sNS.size() - 1] != ':'){
				sNS.append(":");
				(*it) = sNS;
			}
		}
	}
	if(a.isA(json::JSON_OBJECT)){
		json::atom temp;
		for(json::iterator it = a.begin(); it != a.end(); ++it){
			std::string sKey = it.key().string();
			for(json::iterator nit = jNameSpaces.begin(); nit != jNameSpaces.end(); ++nit){
				if(sKey.substr(0, (*nit).string().size()) == (*nit).string()){
					sKey = sKey.substr((*nit).string().size());
				}
			}
			temp[sKey] = (*it);
			stripNameSpaces(temp[sKey], jNameSpaces);
		}
		json::atom::swap(a, temp);
	} else if(a.isA(json::JSON_ARRAY)){
		for(json::iterator it = a.begin(); it != a.end(); ++it){
			stripNameSpaces((*it), jNameSpaces);
		}
	}

}

void document::stripNameSpace(json::atom & a, std::string sNameSpace, bool begin)
{
	if(begin)
		if(sNameSpace[sNameSpace.size() - 1] != ':')
			sNameSpace.append(":");
	if(a.isA(json::JSON_OBJECT)){
		json::atom temp;
		for(json::iterator it = a.begin(); it != a.end(); ++it){
			std::string sKey = it.key().string();
			if(sKey.substr(0, sNameSpace.size()) == sNameSpace){
				sKey = sKey.substr(sNameSpace.size());
			}
			temp[sKey] = (*it);
			stripNameSpace(temp[sKey], sNameSpace, false);
		}
		json::atom::swap(a, temp);
	} else if(a.isA(json::JSON_ARRAY)){
		for(json::iterator it = a.begin(); it != a.end(); ++it){
			stripNameSpace((*it), sNameSpace, false);
		}
	}
}

}

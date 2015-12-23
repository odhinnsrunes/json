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
#include "data.hpp"
#include <iomanip>
#include "tinyxml/tinyxml.h"
#include "tinyxml/tinystr.h"

namespace data
{

int isNumeric(std::string data) {
	size_t l = data.size();
	if (l == 0 || l > JSON_NUMBER_PRECISION) {
		return 0;
	}
	const char szOk[] = "1234567890";
	bool bHaveDot = false;
	bool bHaveMinus = false;
	for (std::string::iterator it = data.begin(); it != data.end(); ++it) {
		bool bOk = false;
		for (size_t i = 0; i < 10; i++) {
			if (szOk[i] == (*it)) {
				bOk = true;
				break;
			} else if (i == 0 && (*it) == '-') {
				if(bHaveMinus) {
					return 0;
				}
				bHaveMinus = true;
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
		if(data.size() > 1 && data[0] == '0'){
			return 0;
		}
		return 1;
	}
}

void document::parseXMLElement(json::value & ret, const TiXmlNode * elem)
{
	if (elem) {
		switch (elem->Type()) {
			default:
				break;

			case TiXmlNode::TINYXML_DOCUMENT:
				{	
					const TiXmlElement * rootElem = (elem->ToDocument())->RootElement();
					const TiXmlAttribute * att = rootElem->FirstAttribute();
					while (att) {
						std::string childName = "@";
						childName.append(att->Name());
						int iValType = isNumeric(att->Value());
						if (iValType != 0) {
	                        std::string sValue = att->Value();
	                        
	                        json::instring in(sValue);
	                        bool bFailed = false;
	                        if (ret[childName].isA(json::JSON_OBJECT)){
	                        	json::numberParse(ret[childName]["#value"], in, &bFailed);
	                        } else {
	                        	json::numberParse(ret[childName], in, &bFailed);
	                        }
						} else {
							std::string val = att->Value();
							if (ret[childName].isA(json::JSON_OBJECT)){
								if(val == "true" || val == "YES"){
									ret[childName]["#value"] = true;
								} else if(val == "false" || val == "NO"){
									ret[childName]["#value"] = false;
								} else {
									ret[childName]["#value"] = elem->Value();
								}
							} else {
								if(val == "true" || val == "YES"){
									ret[childName] = true;
								} else if(val == "false" || val == "NO"){
									ret[childName] = false;
								} else {
									ret[childName] = att->Value();
								}
							}
						}
						att = att->Next();
					}
					sRootTag = rootElem->Value();
					for (const TiXmlElement * child = rootElem->FirstChildElement(); child; child = child->NextSiblingElement() ) {
						if (child != NULL) {
							switch (child->Type()) {
								case TiXmlNode::TINYXML_ELEMENT:
								{
									std::string childName = child->Value();
									if (childName[0] == '_' && strchr("1234567890", childName[1])) {
										childName = childName.substr(1);
									}
									if (ret.exists(childName)) {
										if (!ret[childName].isA(json::JSON_ARRAY)) {
											json::value a = ret[childName];
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
					}
					break;
				}
				
			case TiXmlNode::TINYXML_ELEMENT:
				{
					const TiXmlAttribute * att = elem->ToElement()->FirstAttribute();
					while (att) {
						std::string childName = "@";
						childName.append(att->Name());
						int iValType = isNumeric(att->Value());
						if (iValType != 0) {
	                        std::string sValue = att->Value();
	                        
	                        json::instring in(sValue);
	                        bool bFailed = false;
	                        if (ret[childName].isA(json::JSON_OBJECT)){
	                        	json::numberParse(ret[childName]["#value"], in, &bFailed);
	                        } else {
	                        	json::numberParse(ret[childName], in, &bFailed);
	                        }
						} else {
							std::string val = att->Value();
							if (ret[childName].isA(json::JSON_OBJECT)){
								if(val == "true" || val == "YES"){
									ret[childName]["#value"] = true;
								} else if(val == "false" || val == "NO"){
									ret[childName]["#value"] = false;
								} else {
									ret[childName]["#value"] = elem->Value();
								}
							} else {
								if(val == "true" || val == "YES"){
									ret[childName] = true;
								} else if(val == "false" || val == "NO"){
									ret[childName] = false;
								} else {
									ret[childName] = att->Value();
								}
							}
						}
						att = att->Next();
					}
					if (elem->Value()) {
						const TiXmlNode *child = NULL;
                        bool bEmpty = true;
						do {
							child = elem->IterateChildren(child);
							if (child) {
	                            bEmpty = false;
								switch (child->Type()) {
									case TiXmlNode::TINYXML_ELEMENT:
										{
											std::string childName = child->Value();
											if (childName[0] == '_' && strchr("1234567890", childName[1])) {
												childName = childName.substr(1);
											}
											if (ret.exists(childName)) {
												if (!ret[childName].isA(json::JSON_ARRAY)) {
													json::value a = ret[childName];
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
						} while (child);
                        if (bEmpty){
                            if (!ret.isA(json::JSON_OBJECT)){
                                ret = "";
                            }
                        }
					}
					break;
				}

			case TiXmlNode::TINYXML_TEXT:
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
						std::string val = elem->Value();
						if (ret.isA(json::JSON_OBJECT)){
							if(val == "true" || val == "YES"){
								ret["#value"] = true;
							} else if(val == "false" || val == "NO"){
								ret["#value"] = false;
							} else {
								ret["#value"] = elem->Value();
							}
						} else {
							if(val == "true" || val == "YES"){
								ret = true;
							} else if(val == "false" || val == "NO"){
								ret = false;
							} else {
								ret = elem->Value();
							}
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
	
std::string XMLEscape(const std::string& in, bool bAttribute) {
	std::string out;
	for (std::string::const_iterator it = in.begin(); it != in.end(); ++it) {
		switch (*it) {
			default:
				out.push_back(*it);
				break;
			case '&':
				out.append("&amp;");
				break;
			case '>':
				out.append("&gt;");
				break;
			case '<':
				out.append("&lt;");
				break;
			case '\'':
				if(bAttribute)
					out.append("&apos;");
				else
					out.push_back(*it);
				break;
			case '\"':
				if(bAttribute)
					out.append("&quot;");
				else
					out.push_back(*it);
				break;
			case '\r':
				if(bAttribute)
					out.append("&#xD;");
				else
					out.push_back(*it);
				break;
			case '\n':
				if(bAttribute)
					out.append("&#xA;");
				else
					out.push_back(*it);
				break;
			case '\t':
				if(bAttribute)
					out.append("&#x9;");
				else
					out.push_back(*it);
				break;
		}
	}
	return out;
}

void document::writeXML(std::string & str, json::value & ret, int depth, bool bPretty, bool bTabs)
{
	switch (ret.isA()) {
		default:
			break;
		case json::JSON_BOOLEAN:
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
			
			for (json::value & val : ret) {
				bool bEmpty = true;
				std::string key = val.key();
				if (key.size() > 1) {
					if (key[0] == '@') {
						continue;
					}
				}
				if (key == "#value") {
					str.append(XMLEscape(val.string()));
					continue;
				}
				bool bIsNumericKey = false;
				if (strchr("1234567890", key[0])) {
					bIsNumericKey = true;
				}
				if (bPretty && str.size() && !val.isA(json::JSON_VOID)) {
					if (str[str.size() - 1] != '\n')
						str.append("\n");
					
					if (bTabs){
						// for (int k = 0; k < depth; k++) {
							str.append(depth, '\t');
						// }
					}
				}
				if (val.isA(json::JSON_ARRAY)) {
					bEmpty = false;
					size_t l = val.size();
					for (size_t j = 0; j < l; j++) {
						if (bPretty && bTabs && j > 0) {
							// for (int i = 0; i < depth; i++) {
								str.append(depth, '\t');
							// }
						}
						str.push_back('<');
						if (bIsNumericKey) {
							str.append("_");
						}
						str.append(key);
						for (json::value & val2 : val[j]) {
							std::string subKey = val2.key();
							if (subKey.size() > 1) {
								if (subKey[0] == '@') {
									str.push_back(' ');
									str.append(subKey.substr(1));
									str.append("=\"");
									str.append(XMLEscape(val2.string(), true));
									str.push_back('\"');
								}
							}
						}
						if (val[j].empty() || val[j].size() == 0) {
							str.append(" />");
							if (bPretty)
								str.append("\n");
						} else {
							str.push_back('>');
							writeXML(str, val[j], depth + 1, bPretty, bTabs);
							if (bPretty && bTabs && str[str.size() - 1] == '\n') {
								//for (int k = 0; k < depth; k++) {
									str.append(depth, '\t');
								//}
							}
							str.append("</");
							if (bIsNumericKey) {
								str.append("_");
							}
							str.append(key);
							str.push_back('>');
							if (bPretty)
								str.append("\n");
						}
					}
				} else if(!val.isA(json::JSON_VOID)){
					bEmpty = false;
					str.push_back('<');
					if (bIsNumericKey) {
						str.append("_");
					}
					str.append(key);
					for (json::value & val2 : val) {
						std::string subKey = val2.key();
						if (subKey.size() > 1) {
							if (subKey[0] == '@') {
								str.push_back(' ');
								str.append(subKey.substr(1));
								str.append("=\"");
								str.append(XMLEscape(val2.string(), true));
								str.push_back('\"');
							}
						}
					}
					if (val.empty() || val.size() == 0) {
						str.append(" />");
						if (bPretty)
							str.append("\n");
					} else {
						str.push_back('>');
						writeXML(str, val, depth + 1, bPretty, bTabs);
						if (bPretty && bTabs && str[str.size() - 1] == '\n') {
							// for (int k = 0; k < depth; k++) {
								str.append(depth, '\t');
							// }
						}
						str.append("</");
						if (bIsNumericKey) {
							str.append("_");
						}
						str.append(key);
						str.push_back('>');
						if (bPretty)
							str.append("\n");
					}
				}
				if(bEmpty && !(val.empty() || val.size() == 0)) {
					if (bPretty && bTabs && str[str.size() - 1] == '\n') {
						// for (int k = 0; k < depth; k++) {
							str.append(depth, '\t');
						// }
					}
					str.push_back('<');
					if (bIsNumericKey) {
						str.append("_");
					}
					str.append(key);
					str.append(" />");
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
		size_t spacePos = sRootTag.find_first_of(' ');
		if (spacePos == std::string::npos && isA(json::JSON_OBJECT)) {
			for (json::value & val : *this) {
				std::string key = val.key();
				if (key.size() > 1) {
					if (key[0] == '@'){
						ret.append(" ");
						ret.append(key.substr(1));
						ret.append("=\"");
						ret.append(XMLEscape(val.string(), true));
						ret.append("\"");
					}
				}
			}
		}
		 
		ret.append(">");
		if (bPretty) {
			ret.append("\n");
		}
	}

	
	if (isA(json::JSON_OBJECT)) {
		writeXML(ret, *(json::value*)this, iStartDepth, bPretty, bTabs);
	}
	if (sRootTag.size()) {
		ret.append("</");
		size_t spacePos = sRootTag.find_first_of(' ');
		if (spacePos != std::string::npos) {
			ret.append(sRootTag.substr(0, spacePos));
		} else {
			ret.append(sRootTag);
		}
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

void document::stripNameSpaces(json::value & a, json::document jNameSpaces, bool begin)
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
		json::value temp;
		for(json::iterator it = a.begin(); it != a.end(); ++it){
			std::string sKey = it.key().string();
			for(json::iterator nit = jNameSpaces.begin(); nit != jNameSpaces.end(); ++nit){
                if(sKey.size() > (*nit).string().size()){
                    if(sKey.substr(0, (*nit).string().size()) == (*nit).string()){
                        sKey = sKey.substr((*nit).string().size());
                    }
                }
			}
			temp[sKey] = (*it);
			stripNameSpaces(temp[sKey], jNameSpaces);
		}
		json::value::swap(a, temp);
	} else if(a.isA(json::JSON_ARRAY)){
		for(json::iterator it = a.begin(); it != a.end(); ++it){
			stripNameSpaces((*it), jNameSpaces);
		}
	}

}

void document::stripNameSpace(json::value & a, std::string sNameSpace, bool begin)
{
	if(begin)
		if(sNameSpace[sNameSpace.size() - 1] != ':')
			sNameSpace.append(":");
	if(a.isA(json::JSON_OBJECT)){
		json::value temp;
		for(json::iterator it = a.begin(); it != a.end(); ++it){
			std::string sKey = it.key().string();
            if(sKey.size() > sNameSpace.size()){
                if(sKey.substr(0, sNameSpace.size()) == sNameSpace){
                    sKey = sKey.substr(sNameSpace.size());
                }
            }
			temp[sKey] = (*it);
			stripNameSpace(temp[sKey], sNameSpace, false);
		}
		json::value::swap(a, temp);
	} else if(a.isA(json::JSON_ARRAY)){
		for(json::iterator it = a.begin(); it != a.end(); ++it){
			stripNameSpace((*it), sNameSpace, false);
		}
	}
}

}

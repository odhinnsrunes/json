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
#include <iomanip>
#include "tinyxml/tinyxml.h"
#include "tinyxml/tinystr.h"

#ifdef _USE_ADDED_ORDER_
#undef _USE_ADDED_ORDER_
#ifndef SUPPORT_ORDERED_JSON
#define SUPPORT_ORDERED_JSON
#endif
#include "data.hpp"
#define _USE_ADDED_ORDER_
// #undef _DATA_HPP
// #undef JSON_HPP_
// #include "data.hpp"
#define JSON_NAMESPACE ojson
#define DATA_NAMESPACE odata
#else 
#include "data.hpp"
#define JSON_NAMESPACE json
#define DATA_NAMESPACE data
#endif

namespace DATA_NAMESPACE
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
				if (bHaveMinus) {
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
		if (data.size() > 1 && data[0] == '0') {
			return 0;
		}
		return 1;
	}
}

void document::parseXMLElement(JSON_NAMESPACE::value & ret, const TiXmlNode * elem)
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
							
							JSON_NAMESPACE::instring in(sValue);
							bool bFailed = false;
							if (ret[childName].isA(JSON_NAMESPACE::JSON_OBJECT)) {
								JSON_NAMESPACE::numberParse(ret[childName]["#value"], in, &bFailed);
							} else {
								JSON_NAMESPACE::numberParse(ret[childName], in, &bFailed);
							}
						} else {
							std::string val = att->Value();
							if (ret[childName].isA(JSON_NAMESPACE::JSON_OBJECT)) {
								if (val == "true" || val == "YES") {
									ret[childName]["#value"] = true;
								} else if (val == "false" || val == "NO") {
									ret[childName]["#value"] = false;
								} else {
									ret[childName]["#value"] = elem->Value();
								}
							} else {
								if (val == "true" || val == "YES") {
									ret[childName] = true;
								} else if (val == "false" || val == "NO") {
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
										if (!ret[childName].isA(JSON_NAMESPACE::JSON_ARRAY)) {
											JSON_NAMESPACE::value a = ret[childName];
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
							
							JSON_NAMESPACE::instring in(sValue);
							bool bFailed = false;
							if (ret[childName].isA(JSON_NAMESPACE::JSON_OBJECT)) {
								JSON_NAMESPACE::numberParse(ret[childName]["#value"], in, &bFailed);
							} else {
								JSON_NAMESPACE::numberParse(ret[childName], in, &bFailed);
							}
						} else {
							std::string val = att->Value();
							if (ret[childName].isA(JSON_NAMESPACE::JSON_OBJECT)) {
								if (val == "true" || val == "YES") {
									ret[childName]["#value"] = true;
								} else if (val == "false" || val == "NO") {
									ret[childName]["#value"] = false;
								} else {
									ret[childName]["#value"] = elem->Value();
								}
							} else {
								if (val == "true" || val == "YES") {
									ret[childName] = true;
								} else if (val == "false" || val == "NO") {
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
												if (!ret[childName].isA(JSON_NAMESPACE::JSON_ARRAY)) {
													JSON_NAMESPACE::value a = ret[childName];
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
						if (bEmpty) {
							if (!ret.isA(JSON_NAMESPACE::JSON_OBJECT)) {
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
						
						JSON_NAMESPACE::instring in(sValue);
						bool bFailed = false;
						if (ret.isA(JSON_NAMESPACE::JSON_OBJECT)) {
							JSON_NAMESPACE::numberParse(ret["#value"], in, &bFailed);
						} else {
							JSON_NAMESPACE::numberParse(ret, in, &bFailed);
						}
					} else {
						std::string val = elem->Value();
						if (ret.isA(JSON_NAMESPACE::JSON_OBJECT)) {
							if (val == "true" || val == "YES") {
								ret["#value"] = true;
							} else if (val == "false" || val == "NO") {
								ret["#value"] = false;
							} else {
								ret["#value"] = elem->Value();
							}
						} else {
							if (val == "true" || val == "YES") {
								ret = true;
							} else if (val == "false" || val == "NO") {
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
	myType = JSON_NAMESPACE::JSON_VOID;
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
		size_t br = fread(buffer, 1, l, fd);
		if (debug && br != l) {
			debug("File size mismatch in %s.", inStr.c_str());
		}
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
			{
				if (*it < ' ' && *it > 0) {
					std::stringstream stream;
					char szByte[6];
					sprintf(szByte, "%.2x", char(*it) & 0xFF);
					stream << "&#x" << szByte << ";";
					out.append(stream.str());
				} else {
					out.push_back(*it);
				}
				break;
			}
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
				if (bAttribute)
					out.append("&apos;");
				else
					out.push_back(*it);
				break;
			case '\"':
				if (bAttribute)
					out.append("&quot;");
				else
					out.push_back(*it);
				break;
			case '\r':
				if (bAttribute)
					out.append("&#xD;");
				else
					out.push_back(*it);
				break;
			case '\n':
				if (bAttribute)
					out.append("&#xA;");
				else
					out.push_back(*it);
				break;
			case '\t':
				if (bAttribute)
					out.append("&#x9;");
				else
					out.push_back(*it);
				break;
		}
	}
	return out;
}

void document::writeXML(std::string & str, JSON_NAMESPACE::value & ret, int depth, bool bPretty, bool bTabs)
{
	switch (ret.isA()) {
		default:
			break;
		case JSON_NAMESPACE::JSON_BOOLEAN:
			str.append(ret.boolean() ? "true" : "false");
			break;

		case JSON_NAMESPACE::JSON_NUMBER:
		{
			str.append(XMLEscape(ret.string()));
			break;
		}
		case JSON_NAMESPACE::JSON_STRING:
			str.append(XMLEscape(ret.string()));
			break;

		case JSON_NAMESPACE::JSON_OBJECT:
		{
			
			for (JSON_NAMESPACE::value & val : ret) {
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
				if (bPretty && str.size() && !val.isA(JSON_NAMESPACE::JSON_VOID)) {
					if (str[str.size() - 1] != '\n')
						str.append("\n");
					
					if (bTabs) {
						// for (int k = 0; k < depth; k++) {
							str.append(depth, '\t');
						// }
					}
				}
				if (val.isA(JSON_NAMESPACE::JSON_ARRAY)) {
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
						size_t attCount = 0;
						for (JSON_NAMESPACE::value & val2 : val[j]) {
							std::string subKey = val2.key();
							if (subKey.size() > 1) {
								if (subKey[0] == '@' && !val2.isA(JSON_NAMESPACE::JSON_VOID)) {
									str.push_back(' ');
									str.append(subKey.substr(1));
									str.append("=\"");
									str.append(XMLEscape(val2.string(), true));
									str.push_back('\"');
									attCount++;
								}
							}
						}
						if (val[j].empty() || val[j].size() == 0/* || val[j].size() <= attCount*/) {
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
				} else if (!val.isA(JSON_NAMESPACE::JSON_VOID)) {
					bEmpty = false;
					str.push_back('<');
					if (bIsNumericKey) {
						str.append("_");
					}
					str.append(key);
					size_t attCount = 0;
					for (JSON_NAMESPACE::value & val2 : val) {
						std::string subKey = val2.key();
						if (subKey.size() > 1) {
							if (subKey[0] == '@' && !val2.isA(JSON_NAMESPACE::JSON_VOID)) {
								str.push_back(' ');
								str.append(subKey.substr(1));
								str.append("=\"");
								str.append(XMLEscape(val2.string(), true));
								str.push_back('\"');
								attCount++;
							}
						}
					}
					if (val.empty() || val.size() == 0 /*|| val.size() <= attCount*/) {
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
				if (bEmpty && !(val.empty() || val.size() == 0)) {
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
	if (rootElem.size()) {
		sRootTag = rootElem;
	}
	return writeXML(bPretty, bTabs, preWriter);
}

std::string document::writeXML(bool bPretty, bool bTabs, PREWRITEPTR preWriter)
{
	std::string ret;
	int iStartDepth = 0;
	if (!bNoXMLHeader && (sRootTag.size() || bForceXMLHeader)) {
		if (bStandAlone) {
			ret = "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>";
		} else {
			ret = "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
		}
		if (bPretty) {
			ret.append("\n");
		}
		iStartDepth = 1;
	}
	if (sRootTag.size()) {
		ret.append("<");
		ret.append(sRootTag);
		size_t spacePos = sRootTag.find_first_of(' ');
		if (spacePos == std::string::npos && isA(JSON_NAMESPACE::JSON_OBJECT)) {
			for (JSON_NAMESPACE::value & val : *this) {
				std::string key = val.key();
				if (key.size() > 1) {
					if (key[0] == '@') {
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

	
	if (isA(JSON_NAMESPACE::JSON_OBJECT)) {
		writeXML(ret, *(JSON_NAMESPACE::value*)this, iStartDepth, bPretty, bTabs);
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
#ifdef _JSON_TEMP_FILES_
	char szTempFile[L_tmpnam + 1];
	if (tmpnam(szTempFile) == NULL) {
		debug("Failed creating temp file name for %s.", inStr.c_str());
		return false;
	}
	FILE* fd = fopen(szTempFile, "wb");
	if (fd) {
		std::string w = writeXML(rootElem, bPretty, bTabs, preWriter);
		if (fwrite(w.data(), 1, w.size(), fd) != w.size()) {
			debug("Failed Writing to %s.", inStr.c_str());
			fclose(fd);
			return false;
		} else {
			fclose(fd);
			if (json::fileExists((inStr + ".bak").c_str())) {
				remove((inStr + ".bak").c_str());
			}
			if (json::fileExists(inStr.c_str())) {
				if (rename(inStr.c_str(), (inStr + ".bak").c_str()) != 0) {
					debug("Failed to backup %s.", inStr.c_str());
					return false;
				}
			}
			if (rename(szTempFile, inStr.c_str()) != 0) {
				debug("Failed rename temp file to %s.", inStr.c_str());
				if (rename((inStr + ".bak").c_str(), inStr.c_str()) != 0) {
					debug("Failed restore backup of %s.", inStr.c_str());
				}
				return false;
			}

			if (json::fileExists((inStr + ".bak").c_str())) {
				if (remove((inStr + ".bak").c_str()) != 0) {
					debug("Failed remove backup of %s.", inStr.c_str());
				}
			}

			return true;
		}
	}
#else
	FILE* fd = fopen(inStr.c_str(), "wb");
	if (fd) {
		std::string w = writeXML(rootElem, bPretty, bTabs, preWriter);
		fwrite(w.data(), 1, w.size(), fd);
		fclose(fd);
		return true;
	}
#endif
	return false;
}

bool document::writeXMLFile(std::string inStr, bool bPretty, bool bTabs, PREWRITEPTR preWriter)
{
	FILE* fd = fopen(inStr.c_str(), "wb");
	if (fd) {
		std::string w = writeXML(bPretty, bTabs, preWriter);
		fwrite(w.data(), 1, w.size(), fd);
		fclose(fd);
		return true;
	}
	return false;
}

void document::stripNameSpaces(JSON_NAMESPACE::value & a, JSON_NAMESPACE::document jNameSpaces, bool begin)
{
	if (begin) {
		for (JSON_NAMESPACE::iterator it = jNameSpaces.begin(); it != jNameSpaces.end(); ++it) {
			std::string sNS = (*it).string();
			if (sNS[sNS.size() - 1] != ':') {
				sNS.append(":");
				(*it) = sNS;
			}
		}
	}
	if (a.isA(JSON_NAMESPACE::JSON_OBJECT)) {
		JSON_NAMESPACE::value temp;
		for (JSON_NAMESPACE::iterator it = a.begin(); it != a.end(); ++it) {
			std::string sKey = it.key().string();
			for (JSON_NAMESPACE::iterator nit = jNameSpaces.begin(); nit != jNameSpaces.end(); ++nit) {
				if (sKey.size() > (*nit).string().size()) {
					if (sKey.substr(0, (*nit).string().size()) == (*nit).string()) {
						sKey = sKey.substr((*nit).string().size());
					}
				}
			}
			temp[sKey] = (*it);
			stripNameSpaces(temp[sKey], jNameSpaces);
		}
		JSON_NAMESPACE::value::swap(a, temp);
	} else if (a.isA(JSON_NAMESPACE::JSON_ARRAY)) {
		for (JSON_NAMESPACE::iterator it = a.begin(); it != a.end(); ++it) {
			stripNameSpaces((*it), jNameSpaces);
		}
	}

}

void document::stripNameSpace(JSON_NAMESPACE::value & a, std::string sNameSpace, bool begin)
{
	if (begin)
		if (sNameSpace[sNameSpace.size() - 1] != ':')
			sNameSpace.append(":");
	if (a.isA(JSON_NAMESPACE::JSON_OBJECT)) {
		JSON_NAMESPACE::value temp;
		for (JSON_NAMESPACE::iterator it = a.begin(); it != a.end(); ++it) {
			std::string sKey = it.key().string();
			if (sKey.size() > sNameSpace.size()) {
				if (sKey[0] == '@') {
					if (sKey.substr(1, sNameSpace.size()) == sNameSpace) {
						sKey = std::string("@") + sKey.substr(sNameSpace.size() + 1);
					}
				} else {
					if (sKey.substr(0, sNameSpace.size()) == sNameSpace) {
						sKey = sKey.substr(sNameSpace.size());
					}
				}
			}
			temp[sKey] = (*it);
			stripNameSpace(temp[sKey], sNameSpace, false);
		}
		JSON_NAMESPACE::value::swap(a, temp);
	} else if (a.isA(JSON_NAMESPACE::JSON_ARRAY)) {
		for (JSON_NAMESPACE::iterator it = a.begin(); it != a.end(); ++it) {
			stripNameSpace((*it), sNameSpace, false);
		}
	}
}

void document::addNameSpace(JSON_NAMESPACE::value & a, std::string sNameSpace, bool begin)
{
	if (begin)
		if (sNameSpace[sNameSpace.size() - 1] != ':')
			sNameSpace.append(":");
	if (a.isA(JSON_NAMESPACE::JSON_OBJECT)) {
		JSON_NAMESPACE::value temp;
		for (JSON_NAMESPACE::iterator it = a.begin(); it != a.end(); ++it) {
			std::string sKey = it.key().string();
			if (sKey[0] == '@') {
				sKey.insert(1, sNameSpace);
			} else {
				sKey.insert(0, sNameSpace);
			}
			temp[sKey] = (*it);
			addNameSpace(temp[sKey], sNameSpace, false);
		}
		JSON_NAMESPACE::value::swap(a, temp);
	} else if (a.isA(JSON_NAMESPACE::JSON_ARRAY)) {
		for (JSON_NAMESPACE::iterator it = a.begin(); it != a.end(); ++it) {
			addNameSpace((*it), sNameSpace, false);
		}
	}
}

}

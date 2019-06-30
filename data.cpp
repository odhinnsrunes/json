/*
Copyright (c) 2012-2019 James Baker

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

#if defined _USE_ADDED_ORDER_
#undef _USE_ADDED_ORDER_
#if !defined SUPPORT_ORDERED_JSON
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

	int isNumeric(sdstring data) {
		size_t l = data.size();
		if (l == 0 || l > JSON_NUMBER_PRECISION) {
			return 0;
		}
		const char szOk[] = "1234567890";
		bool bHaveDigit = false;
		bool bHaveDot = false;
		bool bHaveSpace = false;
		bool bHaveMinus = false;
		for (sdstring::iterator it = data.begin(); it != data.end(); ++it) {
			bool bOk = false;
			for (size_t i = 0; i < 10; i++) {
				if (szOk[i] == (*it)) {
					if (bHaveSpace) {
						return 0;
					}
					bHaveDigit = true;
					bOk = true;
					break;
				} else if ((*it) == '-') {
					if (bHaveDigit || bHaveMinus || bHaveDot || bHaveSpace) {
						return 0;
					}
					bHaveMinus = true;
					bOk = true;
					break;
				} else if ((*it) == ' ') {
					bHaveSpace = true;
					bOk = true;
					break;
				} else if ((*it) == '.') {
					if (bHaveDot || bHaveSpace) {
						return 0;
					}
					bHaveDot = true;
					bOk = true;
					break;
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

	int IsBool(const sdstring & str)
	{
		char first = str[0];
		size_t l = str.size();
		if (first == 't' && l == 4) {
			if (str == "true") {
				return 2;
			} else {
				return 0;
			}
		} else if (first == 'Y' && l == 3) {
			if (str == "YES") {
				return 2;
			} else {
				return 0;
			}
		} else if (first == 'f' && l == 5) {
			if (str == "false") {
				return 1;
			} else {
				return 0;
			}
		} else if (first == 'N' && l == 2) {
			if (str == "NO") {
				return 1;
			} else {
				return 0;
			}
		}
		return 0;
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
					if (rootElem == nullptr) {
						break;
					}
					const TiXmlAttribute * att = rootElem->FirstAttribute();
					while (att) {
						sdstring childName = "@";
						childName.append(att->Name());
						JSON_NAMESPACE::value& childref = ret[childName];
						int iValType = isNumeric(att->Value());
						if (iValType != 0) {
							sdstring sValue = att->Value();

							JSON_NAMESPACE::instring in(sValue);
							bool bFailed = false;
							if (childref.isA(JSON_NAMESPACE::JSON_OBJECT)) {
								JSON_NAMESPACE::numberParse(childref[DATA_VAL], in, &bFailed);
							} else {
								JSON_NAMESPACE::numberParse(childref, in, &bFailed);
							}
						} else {
							sdstring val = att->Value();
							if (childref.isA(JSON_NAMESPACE::JSON_OBJECT)) {
								switch(IsBool(val)) {
									default:
										childref[DATA_VAL] = elem->Value();
										break;
									case 2:
										childref[DATA_VAL] = true;
										break;
									case 1:
										childref[DATA_VAL] = false;
										break;

								}
								// if (val == "true" || val == "YES") {
								// 	childref[DATA_VAL] = true;
								// } else if (val == "false" || val == "NO") {
								// 	childref[DATA_VAL] = false;
								// } else {
								// 	childref[DATA_VAL] = elem->Value();
								// }
							} else {
								switch(IsBool(val)) {
									default:
										childref = att->Value();
										break;
									case 2:
										childref = true;
										break;
									case 1:
										childref = false;
										break;

								}
								// if (val == "true" || val == "YES") {
								// 	ret[childName] = true;
								// } else if (val == "false" || val == "NO") {
								// 	ret[childName] = false;
								// } else {
								// 	ret[childName] = att->Value();
								// }
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
										sdstring childName = child->Value();
										if (childName[0] == '_' && strchr("1234567890", childName[1])) {
											childName = childName.substr(1);
										}
										if (ret.exists(childName)) {
											if (!ret[childName].isA(JSON_NAMESPACE::JSON_ARRAY)) {
												JSON_NAMESPACE::value a;
	/*											JSON_NAMESPACE::value& b = ret[childName];
												std::swap(a, b);
												std::swap(b[0], a);*/
												std::swap(ret[childName], a);
												std::swap(ret[childName][0], a);
											}
											parseXMLElement(ret[childName][ret[childName].size()], child);
										} else {
											parseXMLElement(ret[childName], child);
										}
									}
									break;

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
							sdstring childName("@");
					
							childName.append(att->Name());
					
							int iValType = isNumeric(att->Value());
					
							JSON_NAMESPACE::value& childref = ret[childName];
					
							if (iValType != 0) {
								sdstring sValue = att->Value();

								JSON_NAMESPACE::instring in(sValue);
					
								bool bFailed = false;
					
								if (childref.isA(JSON_NAMESPACE::JSON_OBJECT)) {
									JSON_NAMESPACE::numberParse(childref[DATA_VAL], in, &bFailed);
								} else {
									JSON_NAMESPACE::numberParse(childref, in, &bFailed);
								}
							} else {
								sdstring val = att->Value();
					
								if (childref.isA(JSON_NAMESPACE::JSON_OBJECT)) {
									switch(IsBool(val)) {
										default:
											childref[DATA_VAL] = elem->Value();
											break;
										case 2:
											childref[DATA_VAL] = true;
											break;
										case 1:
											childref[DATA_VAL] = false;
											break;

									}
								} else {
									switch(IsBool(val)) {
										default:
											childref = att->Value();
											break;
										case 2:
											childref = true;
											break;
										case 1:
											childref = false;
											break;

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
													JSON_NAMESPACE::value& jChild = ret[childName];
													if (!jChild.isA(JSON_NAMESPACE::JSON_ARRAY)) {
														JSON_NAMESPACE::value a;
			
														std::swap(jChild, a);
														std::swap(jChild[0], a);
													}
													parseXMLElement(jChild[jChild.size()], child);
												} else {
													parseXMLElement(ret[childName], child);
												}
											}
											break;

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
					}
					break;

				case TiXmlNode::TINYXML_TEXT:
				if (elem->Value()) {
					int iValType = isNumeric(elem->Value());
					if (iValType != 0) {
						sdstring sValue = elem->Value();

						JSON_NAMESPACE::instring in(sValue);
						bool bFailed = false;
						if (ret.isA(JSON_NAMESPACE::JSON_OBJECT)) {
							JSON_NAMESPACE::numberParse(ret[DATA_VAL], in, &bFailed);
						} else {
							JSON_NAMESPACE::numberParse(ret, in, &bFailed);
						}
					} else {
						sdstring val = elem->Value();
						if (ret.isA(JSON_NAMESPACE::JSON_OBJECT)) {
							switch(IsBool(val)) {
								default:
									ret[DATA_VAL] = elem->Value();
									break;
								case 2:
									ret[DATA_VAL] = true;
									break;
								case 1:
									ret[DATA_VAL] = false;
									break;

							}
							// if (val == "true" || val == "YES") {
							// 	ret[DATA_VAL] = true;
							// } else if (val == "false" || val == "NO") {
							// 	ret[DATA_VAL] = false;
							// } else {
							// 	ret[DATA_VAL] = elem->Value();
							// }
						} else {
							switch(IsBool(val)) {
								default:
									ret = elem->Value();
									break;
								case 2:
									ret = true;
									break;
								case 1:
									ret = false;
									break;

							}
							// if (val == "true" || val == "YES") {
							// 	ret = true;
							// } else if (val == "false" || val == "NO") {
							// 	ret = false;
							// } else {
							// 	ret = elem->Value();
							// }
						}
					}
				}
				break;
			}
		}
	}

	bool document::parseXML(const sdstring &inStr, PREPARSEPTR preParser, const sdstring &preParseFileName)
	{
		if (myType == JSON_NAMESPACE::JSON_ARRAY) {
			delete arr;
			arr = NULL;
		} else if (myType == JSON_NAMESPACE::JSON_OBJECT) {
			delete obj;
			obj = NULL;
		}
		myType = JSON_NAMESPACE::JSON_VOID;
		m_number = 0;
		m_boolean = false;
		str.clear();

		TiXmlDocument doc;
		doc.SetCondenseWhiteSpace(false);
		if (preParser == NULL) {
			doc.Parse(inStr.c_str());
		} else {
			sdstring sOut;
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
			strParseResult = "XML Parsing failed: " + sdstring(doc.ErrorDesc());
			return false;
		}
		sRootTag.clear();
		parseXMLElement(*this, &doc);

		bParseSuccessful = true;
		strParseResult = "Ok.";
		return true;
	}

	bool document::parseXMLFile(const sdstring &inStr, PREPARSEPTR preParser, bool bReWriteFile)
	{
		FILE* fd = fopen(inStr.c_str(), "rb");
		if (fd) {
			fseek(fd, 0, SEEK_END);
			size_t l = (size_t)ftell(fd);
			fseek(fd, 0, SEEK_SET);
			char* buffer = static_cast<char*>(malloc(l + 1));

			buffer[l] = 0;
			size_t br = fread(buffer, 1, l, fd);
			if (debug && br != l) {
				debug("File size mismatch in %s.", inStr.c_str());
			}
			fclose(fd);
			bool bRetVal;
			sdstring sDat(buffer, l);
			if (bReWriteFile) {
				bRetVal = parseXML(sDat, preParser, inStr);
			} else {
				bRetVal = parseXML(sDat, preParser);
			}
			bParseSuccessful = bRetVal;
			memset(buffer, 0, l + 1);
			free(buffer);
			return bRetVal;
		}
		strParseResult = "Couldn't open file " + inStr;
		bParseSuccessful = false;
		return false;
	}

	sdstring XMLEscape(const sdstring& in, bool bAttribute) {
		sdstring out;
		for (const char &c: in) {
			switch (c) {
				default:
				{
					if (c < ' ' && c > 0) {
						std::stringstream stream;
						char szByte[6];
						sprintf(szByte, "%.2x", c & 0xFF);
						stream << "&#x" << szByte << ";";
						out.append(stream.str().c_str());
					} else {
						out.push_back(c);
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
						out.push_back(c);
					break;
				case '\"':
					if (bAttribute)
						out.append("&quot;");
					else
						out.push_back(c);
					break;
				case '\r':
					if (bAttribute)
						out.append("&#xD;");
					else
						out.push_back(c);
					break;
				case '\n':
					if (bAttribute)
						out.append("&#xA;");
					else
						out.push_back(c);
					break;
				case '\t':
					if (bAttribute)
						out.append("&#x9;");
					else
						out.push_back(c);
					break;
			}
		}
		return out;
	}

	void document::writeXML(sdstring & strParameter, JSON_NAMESPACE::value & ret, size_t depth, bool bPretty, bool bTabs)
	{
		switch (ret.isA()) {
			default:
			break;
			case JSON_NAMESPACE::JSON_BOOLEAN:
			strParameter.append(ret.boolean() ? "true" : "false");
			break;

			case JSON_NAMESPACE::JSON_NUMBER:
			{
				strParameter.append(XMLEscape(ret.string()));
				break;
			}
			case JSON_NAMESPACE::JSON_STRING:
			strParameter.append(XMLEscape(ret.string()));
			break;

			case JSON_NAMESPACE::JSON_OBJECT:
			{

				for (JSON_NAMESPACE::value & val : ret) {
					bool bEmpty = true;
					sdstring key = val.key();
					if (key.size() > 1) {
						if (key[0] == '@') {
							continue;
						}
					}
					if (key == DATA_VAL) {
						strParameter.append(XMLEscape(val.string()));
						continue;
					}
					bool bIsNumericKey = false;
					if (strchr("1234567890", key[0])) {
						bIsNumericKey = true;
					}
					if (bPretty && strParameter.size() && !val.isA(JSON_NAMESPACE::JSON_VOID)) {
						if (strParameter[strParameter.size() - 1] != '\n')
							strParameter.append("\n");

						if (bTabs) {
						// for (int k = 0; k < depth; k++) {
							strParameter.append(depth, '\t');
						// }
						}
					}
					if (val.isA(JSON_NAMESPACE::JSON_ARRAY)) {
						bEmpty = false;
						size_t l = val.size();
						for (size_t j = 0; j < l; j++) {
							if (bPretty && bTabs && j > 0) {
							// for (int i = 0; i < depth; i++) {
								strParameter.append(depth, '\t');
							// }
							}
							strParameter.push_back('<');
							if (bIsNumericKey) {
								strParameter.append("_");
							}
							strParameter.append(key);
							size_t attCount = 0;
							for (JSON_NAMESPACE::value & val2 : val[j]) {
								sdstring subKey = val2.key();
								if (subKey.size() > 1) {
									if (subKey[0] == '@' && !val2.isA(JSON_NAMESPACE::JSON_VOID)) {
										strParameter.push_back(' ');
										strParameter.append(subKey.substr(1));
										strParameter.append("=\"");
										strParameter.append(XMLEscape(val2.string(), true));
										strParameter.push_back('\"');
										attCount++;
									}
								}
							}
							if (val[j].empty() || val[j].size() == 0 || val[j].size() <= attCount) {
								strParameter.append(" />");
								if (bPretty) {
									strParameter.append("\n");
								}
							} else {
								strParameter.push_back('>');
								writeXML(strParameter, val[j], depth + 1, bPretty, bTabs);
								if (bPretty && bTabs && strParameter[strParameter.size() - 1] == '\n') {
									strParameter.append(depth, '\t');
								}
								strParameter.append("</");
								if (bIsNumericKey) {
									strParameter.append("_");
								}
								strParameter.append(key);
								strParameter.push_back('>');
								if (bPretty)
									strParameter.append("\n");
							}
						}
					} else if (!val.isA(JSON_NAMESPACE::JSON_VOID)) {
						bEmpty = false;
						strParameter.push_back('<');
						if (bIsNumericKey) {
							strParameter.append("_");
						}
						strParameter.append(key);
						size_t attCount = 0;
						for (JSON_NAMESPACE::value & val2 : val) {
							sdstring subKey = val2.key();
							if (subKey.size() > 1) {
								if (subKey[0] == '@' && !val2.isA(JSON_NAMESPACE::JSON_VOID)) {
									strParameter.push_back(' ');
									strParameter.append(subKey.substr(1));
									strParameter.append("=\"");
									strParameter.append(XMLEscape(val2.string(), true));
									strParameter.push_back('\"');
									attCount++;
								}
							}
						}
						if (val.empty() || val.size() == 0 || val.size() <= attCount) {
							strParameter.append(" />");
							if (bPretty) {
								strParameter.append("\n");
							}
						} else {
							strParameter.push_back('>');
							writeXML(strParameter, val, depth + 1, bPretty, bTabs);
							if (bPretty && bTabs && strParameter[strParameter.size() - 1] == '\n') {
								strParameter.append(depth, '\t');
							}
							strParameter.append("</");
							if (bIsNumericKey) {
								strParameter.append("_");
							}
							strParameter.append(key);
							strParameter.push_back('>');
							if (bPretty)
								strParameter.append("\n");
						}
					}
					if (bEmpty && !(val.empty() || val.size() == 0)) {
						if (bPretty && bTabs && strParameter[strParameter.size() - 1] == '\n') {
						// for (int k = 0; k < depth; k++) {
							strParameter.append(depth, '\t');
						// }
						}
						strParameter.push_back('<');
						if (bIsNumericKey) {
							strParameter.append("_");
						}
						strParameter.append(key);
						strParameter.append(" />");
						if (bPretty)
							strParameter.append("\n");
					}
				}
				break;
			}

		}
	}

	sdstring document::writeXML(const sdstring &rootElem, bool bPretty, bool bTabs, PREWRITEPTR preWriter)
	{
		if (rootElem.size()) {
			sRootTag = rootElem;
		}
		return writeXML(bPretty, bTabs, preWriter);
	}

	sdstring document::writeXML(bool bPretty, bool bTabs, PREWRITEPTR preWriter)
	{
		sdstring ret;
		size_t iStartDepth = 0;
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
			if (spacePos == sdstring::npos && isA(JSON_NAMESPACE::JSON_OBJECT)) {
				for (JSON_NAMESPACE::value & val : *this) {
					sdstring key = val.key();
					if (key.size() > 1) {
						if (key[0] == '@' && !val.isA(JSON_NAMESPACE::JSON_VOID)) {
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
			if (spacePos != sdstring::npos) {
				ret.append(sRootTag.substr(0, spacePos));
			} else {
				ret.append(sRootTag);
			}
			ret.append(">");
		}

		if (preWriter != NULL) {
			sdstring sOut;
			return preWriter(ret, sOut);
		}

		return ret;
	}

	bool document::writeXMLFile(const sdstring &inStr, const sdstring &rootElem, bool bPretty, bool bTabs, PREWRITEPTR preWriter)
	{
#if defined _JSON_TEMP_FILES_
		sdstring sTempFile(inStr);
		sTempFile.append(".tmp");

		FILE* fd = fopen(sTempFile.c_str(), "wb");
		if (fd) {
			sdstring w = writeXML(rootElem, bPretty, bTabs, preWriter);
			if (fwrite(w.data(), 1, w.size(), fd) != w.size()) {
				if (debug) {
					debug("Failed Writing to %s.", inStr.c_str());
				}
				fclose(fd);
				return false;
			} else {
				fclose(fd);
				sdstring sInstrPlusBak(inStr);
				sInstrPlusBak.append(".bak");
				if (json::fileExists(sInstrPlusBak.c_str())) {
					remove((inStr + ".bak").c_str());
				}
				if (json::fileExists(inStr.c_str())) {
					if (rename(inStr.c_str(), sInstrPlusBak.c_str()) != 0) {
						if (debug) {
							debug("Failed to backup %s.", inStr.c_str());
						}
						return false;
					}
				}
				if (rename(sTempFile.c_str(), inStr.c_str()) != 0) {
					if (debug) {
						debug("Failed rename temp file to %s.", inStr.c_str());
					}
					if (rename(sInstrPlusBak.c_str(), inStr.c_str()) != 0 && debug) {
						debug("Failed restore backup of %s.", inStr.c_str());
					}
					return false;
				}

				if (json::fileExists(sInstrPlusBak.c_str())) {
					if (remove(sInstrPlusBak.c_str()) != 0 && debug) {
						debug("Failed remove backup of %s.", inStr.c_str());
					}
				}

				return true;
			}
		}
#else
		FILE* fd = fopen(inStr.c_str(), "wb");
		if (fd) {
			sdstring w = writeXML(rootElem, bPretty, bTabs, preWriter);
			if (w.size()) {
				fwrite(w.data(), 1, w.size(), fd);
			}
			fclose(fd);
			return true;
		}
#endif
		return false;
	}

	bool document::writeXMLFile(const sdstring &inStr, bool bPretty, bool bTabs, PREWRITEPTR preWriter)
	{
		FILE* fd = fopen(inStr.c_str(), "wb");
		if (fd) {
			sdstring w = writeXML(bPretty, bTabs, preWriter);
			fwrite(w.data(), 1, w.size(), fd);
			fclose(fd);
			return true;
		}
		return false;
	}

	void document ::stripNameSpaces(JSON_NAMESPACE::value & a)
	{
		if (a.isA(JSON_NAMESPACE::JSON_OBJECT)) {
			JSON_NAMESPACE::value temp;
			JSON_NAMESPACE::iterator aend = a.end();
			for (JSON_NAMESPACE::iterator it = a.begin(); it != aend; ++it) {
				sdstring sKey = it.key().string();
				if (sKey.size() > 2) {
					if (sKey[0] == '@') {
						size_t stLast = sKey.find_last_of(':');
						if (stLast != sdstring::npos) {
							sKey = sKey.substr(stLast + 1);
							sKey.insert(sKey.begin(), '@');
						}
					} else {
						size_t stLast = sKey.find_last_of(':');
						if (stLast != sdstring::npos) {
							sKey = sKey.substr(stLast + 1);
						}
					}
				}
				temp[sKey] = (*it);
				stripNameSpaces(temp[sKey]);
			}
			JSON_NAMESPACE::value::swap(a, temp);
		} else if (a.isA(JSON_NAMESPACE::JSON_ARRAY)) {
			JSON_NAMESPACE::iterator aend = a.end();
			for (JSON_NAMESPACE::iterator it = a.begin(); it != aend; ++it) {
				stripNameSpaces((*it));
			}
		}
	}

	void document::stripNameSpaces(JSON_NAMESPACE::value & a, JSON_NAMESPACE::document & jNameSpaces, bool begin)
	{
		if (begin) {
			JSON_NAMESPACE::iterator end = jNameSpaces.end();
			for (JSON_NAMESPACE::iterator it = jNameSpaces.begin(); it != end; ++it) {
				sdstring sNS = (*it).string();
				if (sNS[sNS.size() - 1] != ':') {
					sNS.append(":");
					(*it) = sNS;
				}
			}
		}
		if (a.isA(JSON_NAMESPACE::JSON_OBJECT)) {
			JSON_NAMESPACE::value temp;
			JSON_NAMESPACE::iterator aend = a.end();
			for (JSON_NAMESPACE::iterator it = a.begin(); it != aend; ++it) {
				sdstring sKey = it.key().string();
				JSON_NAMESPACE::iterator end = jNameSpaces.end();
				for (JSON_NAMESPACE::iterator nit = jNameSpaces.begin(); nit != end; ++nit) {
					if (sKey[0] == '@') {
						if (sKey.size() > (*nit).string().size()) {
							if (sKey.substr(1, (*nit).string().size()) == (*nit)._sdstring()) {
								sKey = sdstring("@") + sKey.substr((*nit).string().size() + 1);
							}
						}
					} else {
						if (sKey.size() > (*nit).string().size()) {
							if (sKey.substr(0, (*nit).string().size()) == (*nit)._sdstring()) {
								sKey = sKey.substr((*nit).string().size());
							}
						}
					}
				}
				temp[sKey] = (*it);
				stripNameSpaces(temp[sKey], jNameSpaces);
			}
			JSON_NAMESPACE::value::swap(a, temp);
		} else if (a.isA(JSON_NAMESPACE::JSON_ARRAY)) {
			JSON_NAMESPACE::iterator aend = a.end();
			for (JSON_NAMESPACE::iterator it = a.begin(); it != aend; ++it) {
				stripNameSpaces((*it), jNameSpaces, false);
			}
		}

	}

	void document::stripNameSpace(JSON_NAMESPACE::value & a, sdstring sNameSpace, bool begin)
	{
		if (begin) {
			if (sNameSpace[sNameSpace.size() - 1] != ':') {
				sNameSpace.append(":");
			}
		}
		if (a.isA(JSON_NAMESPACE::JSON_OBJECT)) {
			JSON_NAMESPACE::value temp;
			JSON_NAMESPACE::iterator aend = a.end();
			for (JSON_NAMESPACE::iterator it = a.begin(); it != aend; ++it) {
				if ((*it).isA(JSON_NAMESPACE::JSON_VOID) ) {
					continue;
				}
				sdstring sKey = it.key().string();
				if (sKey.size() > sNameSpace.size()) {
					if (sKey[0] == '@') {
						if (sKey.substr(1, sNameSpace.size()) == sNameSpace) {
							sKey = sdstring("@") + sKey.substr(sNameSpace.size() + 1);
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
			JSON_NAMESPACE::iterator aend = a.end();
			for (JSON_NAMESPACE::iterator it = a.begin(); it != aend; ++it) {
				stripNameSpace((*it), sNameSpace, false);
			}
		}
	}

	void document::addNameSpace(JSON_NAMESPACE::value & a, sdstring sNameSpace, bool begin)
	{
		if (begin) {
			if (sNameSpace[sNameSpace.size() - 1] != ':') {
				sNameSpace.append(":");
			}
		}

		if (a.isA(JSON_NAMESPACE::JSON_OBJECT)) {
			JSON_NAMESPACE::value temp;
			JSON_NAMESPACE::iterator aend = a.end();
			for (JSON_NAMESPACE::iterator it = a.begin(); it != aend; ++it) {
				sdstring sKey = it.key().string();
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
			JSON_NAMESPACE::iterator aend = a.end();
			for (JSON_NAMESPACE::iterator it = a.begin(); it != aend; ++it) {
				addNameSpace((*it), sNameSpace, false);
			}
		}
	}

	void document::stripMyNameSpaces()
	{
		size_t stLast = sRootTag.find_last_of(':');
		if (stLast != sdstring::npos) {
			sRootTag = sRootTag.substr(stLast + 1);
		}
		stripNameSpaces(*this);
	}

	void document::stripMyNameSpaces(JSON_NAMESPACE::document & jNameSpaces)
	{
		JSON_NAMESPACE::iterator end = jNameSpaces.end();
		for (JSON_NAMESPACE::iterator it = jNameSpaces.begin(); it != end; ++it) {
			sdstring sNS = (*it).string();
			if (sNS[sNS.size() - 1] != ':') {
				sNS.append(":");
				(*it) = sNS;
			}
			if (sRootTag.size() > (*it).string().size()) {
				if (sRootTag.substr(0, (*it).string().size()) == (*it)._sdstring()) {
					sRootTag = sRootTag.substr((*it).string().size());
				}
			}
		}
		stripNameSpaces(*this, jNameSpaces, false);
	}

	void document::stripMyNameSpace(sdstring sNameSpace)
	{
		if (sNameSpace[sNameSpace.size() - 1] != ':') {
			sNameSpace.append(":");
		}
		if (sRootTag.size() > sNameSpace.size()) {
			if (sRootTag.substr(0, sNameSpace.size()) == sNameSpace) {
				sRootTag = sRootTag.substr(sNameSpace.size());
			}
		}
		stripNameSpace(*this, sNameSpace, false);
	}

	void document::addMyNameSpace(sdstring sNameSpace)
	{
		if (sNameSpace[sNameSpace.size() - 1] != ':') {
			sNameSpace.append(":");
		}
		sRootTag.insert(0, sNameSpace);
		addNameSpace(*this, sNameSpace, false);
	}


}

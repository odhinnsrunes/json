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
#include <iomanip>
#include "tinyxml/tinyxml.h"
#include "tinyxml/tinystr.h"
#include <chrono>

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

							JSON_NAMESPACE::instring in(sValue.data());
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

								JSON_NAMESPACE::instring in(sValue.data());
					
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

						JSON_NAMESPACE::instring in(sValue.data());
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

	sdstring generateError(JSON_NAMESPACE::instring& inputString, const sdstring& szError) {
		sdstring in = inputString.SoFar();
		size_t l = in.size();
		size_t pos = 1;
		size_t line = 1;
		for (size_t i = 0; i < l; i++) {
			if (in[i] == '\n') {
				line++;
				pos = 1;
			} else {
				if (in[i] != '\r')
					pos++;
			}
		}

		std::ostringstream s;
		s << szError << "  Line: " << line <<  " Column: " << pos;
		return s.str().c_str();
	}

	bool document::fastParse(JSON_NAMESPACE::instring& in, JSON_NAMESPACE::value& out, sdstring& parseResult)
	{
		enum eState {
			eRoot,
			eTag,
			eEndTag,
			eAttribute,
			eValue,
			eComment,
			eEscape,
			eInfo,
		};
		eState state = eRoot;
		eState lastState = eRoot;
		char* ptr = in.getPos();
		char* retVal = ptr;
		sdstring sTag;
		sdstring sAttribute;
		JSON_NAMESPACE::value * pTag = nullptr;
		bool bHadChars = false;
		bool bHadNonNumber = false;
		bool bHadDecimal = false;
		bool bHadSign = false;
		bool bHaveAtts = false;
		size_t decimalPos = 0;
		for (;;) {
//			std::cout << sz << ", " << tl << std::endl;
			char &c = in.take();
			switch (state) {
				case eRoot:
				{
					switch (c) {
						default: {
							*ptr = c;
							++ptr;
							if (c == '+' || c == '-') {
								if(!bHadChars) {
									bHadSign = true;
								} else {
									bHadNonNumber = true;
								}
							} else if (c == '.') {
								if (!bHadNonNumber) {
									if (bHadDecimal) {
										bHadNonNumber = true;
									} else {
										decimalPos = size_t(ptr - retVal);
										bHadDecimal = true;
									}
								} else {
									bHadNonNumber = true;
								}
							} else if (c < '0' || c > '9')  {
								bHadNonNumber = true;
							}
							if (c != ' ' && c != '\r' && c != '\n' && c != '\t') {
								bHadChars = true;
							}
							break;
						}
						case '<':
						{
							JSON_NAMESPACE::SkipWhitespace(in);
							if (bHadChars && in.peek() != '/') {
								char sMessage[256];
								sprintf (sMessage, "tags inline with text not permitted. '%c'", in.peek());
								parseResult = generateError(in, sMessage);
								return false;
							}
							if (bHadChars) {
								size_t len = (size_t)(ptr - retVal);
								if (bHadNonNumber || (*retVal == '0' && *(retVal+1) != '.' && len > 1)) {
									sdstring sTemp(std::string(retVal, len));
									int iIsBool = IsBool(sTemp);
									switch (iIsBool) {
										default:
										case 0:
										{
											JSON_NAMESPACE::value temp = sTemp;
											std::swap(out, temp);
											break;
										}
										case 1:
										{
											JSON_NAMESPACE::value temp = false;
											std::swap(out, temp);
											break;
										}
										case 2:
										{
											JSON_NAMESPACE::value temp = true;
											std::swap(out, temp);
											break;
										}
									}
								} else {
									std::istringstream convert(std::string(retVal, len));
									double d = 0.0;
									if (!(convert >> d)) {
										out = 0.0;
										out.fixedDecimal(1);
									} else {
										out = d;
										if (bHadDecimal) {
											out.fixedDecimal(len - decimalPos);
										}
									}
								}
//								out = std::string(retVal, (size_t)(ptr - retVal));
							}
							bHadChars = false;
							bHadSign = false;
							bHadDecimal = false;
							bHadNonNumber = false;
							decimalPos = 0;
							state = eTag;
							ptr = in.getPos();
							retVal = ptr;
							break;
						}
						case '&':
						{
							lastState = state;
							state = eEscape;
							break;
						}
						case '>':
						{
							parseResult = generateError(in, "Unexpected >");
							return false;
						}
					}		
					break;
				}

				case eTag:
				{
					switch (c) {
						default:
						{
							*ptr = c;
							++ptr;
							bHadChars = true;
							break;
						}

						case '\r':
							break;

						case '\n':
						case ' ':
						{
							sTag.assign(retVal, (size_t)(ptr - retVal));
							if (out.exists(sTag)) {
								JSON_NAMESPACE::value& jChild = out[sTag];
								if (!jChild.isA(JSON_NAMESPACE::JSON_ARRAY)) {
									JSON_NAMESPACE::value a;

									std::swap(jChild, a);
									std::swap(jChild[0], a);
								} else {
									jChild.resize(jChild.size() + 1);
								}
								pTag = &(jChild[jChild.size()]);
							} else {
								pTag = &(out[sTag]);
							}

							ptr = in.getPos();
							retVal = ptr;
							state = eAttribute;
							JSON_NAMESPACE::SkipWhitespace(in);
							break;
						}

						case '>':
						{
							sTag.assign(retVal, (size_t)(ptr - retVal));
							JSON_NAMESPACE::value& jChild(out[sTag]);
							int iIs = jChild.isA();
							if (iIs != JSON_NAMESPACE::JSON_VOID) {
								if (iIs != JSON_NAMESPACE::JSON_ARRAY) {
									JSON_NAMESPACE::value a;

									std::swap(jChild, a);
									std::swap(jChild[0], a);
								} else {
									jChild.resize(jChild.size() + 1);
								}
								pTag = &(jChild[jChild.size()]);
							} else {
								pTag = &(jChild);
							}
							if(fastParse(in, *pTag, parseResult) == false) {
								return false;
							} else {
								pTag = nullptr;
								bHadNonNumber = false;
								bHadDecimal = false;
								decimalPos = 0;
								bHadSign = false;
								bHadChars = false;
								bHaveAtts = false;
								state = eRoot;
							}
							ptr = in.getPos();
							retVal = ptr;

							break;
						}

						case '/':
						{
							if (bHadChars) {
								JSON_NAMESPACE::SkipWhitespace(in);
								if (in.take() != '>') {
									parseResult = generateError(in, "Improper end tag");
									return false;
								}
								JSON_NAMESPACE::value &jValue = out[sTag];
								if (jValue.isA(JSON_NAMESPACE::JSON_VOID)) {
									jValue = "";
								}
								state = eRoot;
								pTag = nullptr;
								bHadNonNumber = false;
								bHadDecimal = false;
								decimalPos = 0;
								bHadSign = false;
								bHadChars = false;
								bHaveAtts = false;
							} else {
								state = eEndTag;
							}
							break;
						}

						case '?':
						{
							if (!bHadChars) {
								lastState = state;
								state = eInfo;
								break;
							} else {
								parseResult = generateError(in, "Unexpected ?");
								return false;
							}
						}

						case '!':
						{
							if (!bHadChars) {
								if (in.take() != '-') {
									parseResult = generateError(in, "Improperly formed comment.");
								}
								if (in.take() != '-') {
									parseResult = generateError(in, "Improperly formed comment.");
								}
								lastState = state;
								state = eComment;
								break;
							} else {
								parseResult = generateError(in, "Unexpected !");
								return false;
							}
						}
					}
					break;
				}

				case eEndTag:
				{
					if (out.isA(JSON_NAMESPACE::JSON_VOID)) {
						out = "";
					}
					switch (c) {
						default:
						{
							*ptr = c;
							++ptr;
							bHadChars = true;
							break;
						}
						case ' ':
						{
							JSON_NAMESPACE::SkipWhitespace(in);
							if (in.take() != '>') {
								parseResult = generateError(in, "Unexpected data in end tag.");
								return false;
							}
							parseResult.assign(retVal, (size_t)(ptr - retVal));
							JSON_NAMESPACE::SkipWhitespace(in);
							return true;
						}
						case '>':
						{
//							JSON_NAMESPACE::value &jValue = out[sTag];
//							if (jValue.isA(JSON_NAMESPACE::JSON_VOID)) {
//								jValue = "";
//							}
							parseResult.assign(retVal, (size_t)(ptr - retVal));
							JSON_NAMESPACE::SkipWhitespace(in);
							return true;
						}

					}
					break;
				}

				case eEscape:
				{
					switch (c) {
						default:
						{
							parseResult = generateError(in, "Invalid Escape.");
							return false;
						}
						case 'a':
						{
							char & c2 = in.take();
							if (c2 == 'm') {
								if (in.take() == 'p' && in.take() == ';') {
									*ptr = '&';
									++ptr;
									bHadChars = true;
									state = lastState;
									break;
								}
							} else if (c2 == 'p') {
								if (in.take() == 'o' && in.take() == 's' && in.take() == ';') {
									*ptr = '\'';
									++ptr;
									bHadChars = true;
									state = lastState;
									break;
								}
							}
							parseResult = generateError(in, "Invalid Escape.");
							return false;
						}
						case 'g':
							if (in.take() == 't' && in.take() == ';') {
								*ptr = '>';
								++ptr;
								bHadChars = true;
								state = lastState;
								break;
							}
							parseResult = generateError(in, "Invalid Escape.");
							return false;

						case 'l':
							if (in.take() == 't' && in.take() == ';') {
								*ptr = '<';
								++ptr;
								bHadChars = true;
								state = lastState;
								break;
							}
							parseResult = generateError(in, "Invalid Escape.");
							return false;

						case 'q':
							if (in.take() == 'u' && in.take() == 'o' && in.take() == 't' && in.take() == ';') {
								*ptr = '"';
								++ptr;
								bHadChars = true;
								state = lastState;
								break;
							}
							parseResult = generateError(in, "Invalid Escape.");
							return false;

					}
					break;
				}

				case eAttribute:
				{
					switch (c) {
						default:
						{
							*ptr = c;
							++ptr;
							bHadChars = true;
							break;
						}

						case ' ':
						{
							JSON_NAMESPACE::SkipWhitespace(in);
							char & cNext = in.take();
							if (cNext != '=' && cNext != '>' && cNext != '/') {
								parseResult = generateError(in, "Unexpected data.");
								return false;
							} else if (cNext == '>') {
								if (out.exists(sTag)) {
									JSON_NAMESPACE::value& jChild = out[sTag];
									if (jChild.isA(JSON_NAMESPACE::JSON_ARRAY)) {
										pTag = &(jChild[jChild.size()]);
									} else  {
										pTag = &jChild;
									}
								} else {
									pTag = &(out[sTag]);
								}
								if (fastParse(in, *pTag, parseResult) == false) {
									return false;
								} else {
									pTag = nullptr;
									ptr = in.getPos();
									retVal = ptr;
									bHadNonNumber = false;
									bHadDecimal = false;
									decimalPos = 0;
									bHadSign = false;
									bHadChars = false;
									state = eRoot;
								}
							} else if (cNext == '/') {
								JSON_NAMESPACE::SkipWhitespace(in);
								if (in.take() != '>') {
									parseResult = generateError(in, "Unexpected data.");
									return false;
								}
								return true;
							} else if (cNext == '=') {
								JSON_NAMESPACE::SkipWhitespace(in);
								if (in.take() == '"') {
									sAttribute.assign(retVal, (size_t)(ptr - retVal));
									ptr = in.getPos();
									retVal = ptr;
									sAttribute.insert(0, "@");
									state = eValue;
									bHadNonNumber = false;
									bHadDecimal = false;
									decimalPos = 0;
									bHadSign = false;
									bHadChars = false;
									break;
								} else {
									parseResult = generateError(in, "Attributes must be in quotes.");
									return false;
								}
							}
							JSON_NAMESPACE::SkipWhitespace(in);
							break;
						}

						case '>':
						{
							ptr = in.getPos();
							retVal = ptr;
							if (out.exists(sTag)) {
								JSON_NAMESPACE::value& jChild = out[sTag];
								if (jChild.isA(JSON_NAMESPACE::JSON_ARRAY)) {
									pTag = &(jChild[jChild.size() - 1]);
								} else {
									pTag = &(jChild);
								}
							} else {
								pTag = &(out[sTag]);
							}
							if(fastParse(in, *pTag, parseResult) == false) {
								return false;
							} else {
								pTag = nullptr;
								ptr = in.getPos();
								retVal = ptr;
								bHadNonNumber = false;
								bHadDecimal = false;
								decimalPos = 0;
								bHadSign = false;
								bHadChars = false;
								state = eRoot;
							}
							break;
						}

						case '=':
						{
							// do something with it.
							JSON_NAMESPACE::SkipWhitespace(in);
							if (in.take() == '"') {
								sAttribute.assign(retVal, (size_t)(ptr - retVal));
								ptr = in.getPos();
								retVal = ptr;
								sAttribute.insert(0, "@");
								bHadChars = false;
								bHadNonNumber = false;
								bHadSign = false;
								bHadDecimal = false;
								decimalPos = 0;
								state = eValue;
								break;
							} else {
								parseResult = generateError(in, "Attributes must be in quotes.");
								return false;
							}
						}

						case '/':
						{
							JSON_NAMESPACE::SkipWhitespace(in);
							if (in.take() != '>') {
								parseResult = generateError(in, "invalid end tag.");
								return false;
							}
							bHadChars = false;
							bHadNonNumber = false;
							bHadSign = false;
							bHadDecimal = false;
							decimalPos = 0;
							JSON_NAMESPACE::value &jValue = out[sTag];
							if (jValue.isA(JSON_NAMESPACE::JSON_VOID)) {
								jValue = "";
							}
							state = eRoot;
							break;
						}
					}
					break;
				}

				case eValue:
				{
					switch (c) {
						default:
						{
							*ptr = c;
							++ptr;
							if (c == '+' || c == '-') {
								if(!bHadChars) {
									bHadSign = true;
								} else {
									bHadNonNumber = true;
								}
							} else if (c == '.') {
								if (!bHadNonNumber) {
									if (bHadDecimal) {
										bHadNonNumber = true;
									} else {
										bHadDecimal = true;
									}
								} else {
									bHadNonNumber = true;
								}
							} else if (c < '0' || c > '9')  {
								bHadNonNumber = true;
							}
							bHadChars = true;
							break;
						}
						case '&':
						{
							lastState = state;
							state = eEscape;
							break;
						}
						case '"':
						{
							size_t len = (size_t)(ptr - retVal);
							if (bHadNonNumber || !bHadChars || (*retVal == '0' && *(retVal+1) != '.' && len > 1)) {
								sdstring sTemp(std::string(retVal, len));
								int iIsBool = IsBool(sTemp);
								switch (iIsBool) {
									default:
									case 0:
									{
										(*pTag)[sAttribute] = sdstring(retVal, len);
										break;
									}
									case 1:
									{
										(*pTag)[sAttribute] = false;
										break;
									}
									case 2:
									{
										(*pTag)[sAttribute] = true;
										break;
									}
								}
							} else {
								std::istringstream convert(sdstring(retVal, len));
								double d = 0.0;
								if (!(convert >> d)) {
									(*pTag)[sAttribute] = 0.0;
									(*pTag)[sAttribute].fixedDecimal(1);
								} else {
									(*pTag)[sAttribute] = d;
									if (bHadDecimal) {
										(*pTag)[sAttribute].fixedDecimal(len - decimalPos);
									}
								}
							}
							ptr = in.getPos();
							retVal = ptr;
							state = eAttribute;
							JSON_NAMESPACE::SkipWhitespace(in);
							bHaveAtts = true;
							bHadChars = false;
							bHadSign = false;
							bHadNonNumber = false;
							bHadDecimal = false;
							decimalPos = 0;
							break;
						}
						case '>':
						{
							JSON_NAMESPACE::value &jValue = out[sTag];
							if (jValue.isA(JSON_NAMESPACE::JSON_VOID)) {
								jValue = "";
							}
							parseResult = generateError(in, "Unexpected >.");
							return false;
						}
					}
					break;
				}

				case eComment:
				{
					switch (c) 
					{
						default:
							*ptr = c;
							++ptr;
							break;
						case '-':
							if (in.take() != '-') {
								*ptr = c;
								++ptr;
								break;
							}
							if (in.take() != '>') {
								*ptr = c;
								++ptr;
								break;
							}
							ptr = in.getPos();
							retVal = ptr;
							state = eRoot;
							break;
					}
					break;
				}

				case eInfo:
				{
					switch (c) 
					{
						default:
							*ptr = c;
							++ptr;
							break;
						case '>':
							ptr = in.getPos();
							retVal = ptr;
							state = eRoot;
							JSON_NAMESPACE::SkipWhitespace(in);
							break;
					}
					break;
				}
			}
			if (in.tell() >= in.size()) {
				break;
			}
		}
		return true;
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
		if (preParser == NULL) {
			JSON_NAMESPACE::instring in(inStr.data());
			JSON_NAMESPACE::SkipWhitespace(in);
			bParseSuccessful = fastParse(in, *this, strParseResult);
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
			JSON_NAMESPACE::instring in(inStr.data());
			JSON_NAMESPACE::SkipWhitespace(in);
			bParseSuccessful = fastParse(in, *this, strParseResult);
		}
		if (isA(JSON_NAMESPACE::JSON_OBJECT) && size() == 1) {
			JSON_NAMESPACE::value a;
			auto beg = this->begin();
			sRootTag = beg.key().string();
			std::swap(*(beg), a);
			std::swap(*(static_cast<JSON_NAMESPACE::value*>(this)), a);
			return bParseSuccessful;
		} else {
			strParseResult = "XML Requires there be only one root tag.";
			return false;
		}
	}

	bool document::parseXML_old(const sdstring &inStr, PREPARSEPTR preParser, const sdstring &preParseFileName)
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

	bool document::parseXMLFile_old(const sdstring &inStr, PREPARSEPTR preParser, bool bReWriteFile)
	{
//		auto start = std::chrono::steady_clock::now();
		FILE* fd = fopen(inStr.c_str(), "rb");
//		auto mainStart = start;
		if (fd) {
//			auto end = std::chrono::steady_clock::now();
//			std::cout << "open: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
//			start = end;

			fseek(fd, 0, SEEK_END);
			size_t l = (size_t)ftell(fd);
			fseek(fd, 0, SEEK_SET);

//			end = std::chrono::steady_clock::now();
//			std::cout << "size: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
//			start = end;

			char* buffer = static_cast<char*>(malloc(l + 1));
//			end = std::chrono::steady_clock::now();
//			std::cout << "allocate: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
//			start = end;

			buffer[l] = 0;
			size_t br = fread(buffer, 1, l, fd);
//			end = std::chrono::steady_clock::now();
//			std::cout << "read: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
//			start = end;
			if (debug && br != l) {
				debug("File size mismatch in %s.", inStr.c_str());
			}
			fclose(fd);
//			end = std::chrono::steady_clock::now();
//			std::cout << "close: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
//			start = end;
			bool bRetVal;
			sdstring sDat(buffer, l);
//			end = std::chrono::steady_clock::now();
//			std::cout << "sdstring: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
//			start = end;
			if (bReWriteFile) {
				bRetVal = parseXML_old(sDat, preParser, inStr);
			} else {
				bRetVal = parseXML_old(sDat, preParser);
			}
//			end = std::chrono::steady_clock::now();
//			std::cout << "parsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
//			start = end;
			bParseSuccessful = bRetVal;
			memset(buffer, 0, l + 1);
			free(buffer);
//			end = std::chrono::steady_clock::now();
//			std::cout << "freed: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
//			std::cout << "TOTAL: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - mainStart).count() << std::endl << std::endl;
//			start = end;
			return bRetVal;
		}
		strParseResult = "Couldn't open file " + inStr;
		bParseSuccessful = false;
		return false;
	}

	bool document::parseXMLFile(const sdstring &inStr, PREPARSEPTR preParser, bool bReWriteFile)
	{
//		auto start = std::chrono::steady_clock::now();
//		auto mainStart = start;
		FILE* fd = fopen(inStr.c_str(), "rb");
		if (fd) {
//			auto end = std::chrono::steady_clock::now();
//			std::cout << "open: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
//			start = end;
			fseek(fd, 0, SEEK_END);
			size_t l = (size_t)ftell(fd);
			fseek(fd, 0, SEEK_SET);
//			end = std::chrono::steady_clock::now();
//			std::cout << "size: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
//			start = end;
			char* buffer = static_cast<char*>(malloc(l + 1));
//			end = std::chrono::steady_clock::now();
//			std::cout << "allocate: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
//			start = end;
			buffer[l] = 0;
			size_t br = fread(buffer, 1, l, fd);
//			end = std::chrono::steady_clock::now();
//			std::cout << "read: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
//			start = end;
			if (debug && br != l) {
				debug("File size mismatch in %s.", inStr.c_str());
			}
			fclose(fd);
//			end = std::chrono::steady_clock::now();
//			std::cout << "close: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
//			start = end;
			bool bRetVal;
			sdstring sDat(buffer, l);
//			end = std::chrono::steady_clock::now();
//			std::cout << "sdstring: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
//			start = end;
			if (bReWriteFile) {
				bRetVal = parseXML(sDat, preParser, inStr);
			} else {
				bRetVal = parseXML(sDat, preParser);
			}
//			end = std::chrono::steady_clock::now();
//			std::cout << "parsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
//			start = end;
			bParseSuccessful = bRetVal;
			memset(buffer, 0, l + 1);
			free(buffer);
//			end = std::chrono::steady_clock::now();
//			std::cout << "freed: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
//			start = end;
//			std::cout << "TOTAL: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - mainStart).count() << std::endl << std::endl;
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
								sKey = sdstring("@") + (sdstring)sKey.substr((*nit).string().size() + 1);
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
							sKey = sdstring("@") + (sdstring)sKey.substr(sNameSpace.size() + 1);
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

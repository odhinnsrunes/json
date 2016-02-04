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
#if !defined _DATA_HPP || defined _ODATA_HPP_START
#ifndef _ODATA_HPP_START
#define _DATA_HPP
#endif

class TiXmlNode;
#include "json.hpp"

#ifdef _USE_ADDED_ORDER_
#define JSON_NAMESPACE ojson
#define DATA_NAMESPACE odata
#else 
#define JSON_NAMESPACE json
#define DATA_NAMESPACE data
#endif

namespace DATA_NAMESPACE
{
	std::string XMLEscape(const std::string& in, bool bAttribute = false);
	
	class document : public JSON_NAMESPACE::document
	{
	public:
		document() : JSON_NAMESPACE::document() { }

		document(std::string& in) 
		{
			parseXML(in);
		}
		document(const char* in) {
			parseXML(in);
		}
		document(const JSON_NAMESPACE::value& V) : JSON_NAMESPACE::document(V) { }

		typedef std::string& (*PREPARSEPTR)(const std::string& in, std::string& out, std::string fileName);
		typedef std::string& (*PREWRITEPTR)(const std::string& in, std::string& out);
		bool parseXML(std::string inStr, PREPARSEPTR = NULL, std::string preParseFileName = "");

		bool parseXMLFile(std::string inStr, PREPARSEPTR = NULL, bool bReWriteFile = false);

		std::string writeXML(std::string rootElem, bool bPretty = true, bool bTabs = true, PREWRITEPTR = NULL);

		bool writeXMLFile(std::string inStr, std::string rootElem, bool bPretty = true, bool bTabs = true, PREWRITEPTR = NULL);	
	
		std::string rootTag() { return sRootTag; }
		void rootTag(std::string rootElem) { sRootTag = rootElem; }

		static void stripNameSpaces(JSON_NAMESPACE::value & jDoc, JSON_NAMESPACE::document jNameSpaces, bool begin = true);
		static void stripNameSpace(JSON_NAMESPACE::value & jDoc, std::string sNameSpace, bool begin = true);

	private:
		void parseXMLElement(JSON_NAMESPACE::value& ret, const TiXmlNode * elem);
		void writeXML(std::string & str, JSON_NAMESPACE::value & ret, int depth, bool bPretty = true, bool bTabs = true);

		std::string sRootTag;
		
	};
}

#undef JSON_NAMESPACE
#undef DATA_NAMESPACE

#endif //_DATA_HPP

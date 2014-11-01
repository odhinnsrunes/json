#ifndef _DATA_HPP
#define _DATA_HPP
class TiXmlNode;
#include "json.hpp"
namespace data
{
	std::string XMLEscape(const std::string& in);
	
	class document : public json::document
	{
	public:
		document() : json::document() { }

		document(std::string& in) 
		{
			parseXML(in);
		}
		document(const char* in) {
			parseXML(in);
		}
		document(const json::atom& V) : json::document(V) { }

		typedef std::string& (*PREPARSEPTR)(const std::string& in, std::string& out, std::string fileName);
		typedef std::string& (*PREWRITEPTR)(const std::string& in, std::string& out);
		bool parseXML(std::string inStr, PREPARSEPTR = NULL, std::string preParseFileName = "");

		bool parseXMLFile(std::string inStr, PREPARSEPTR = NULL, bool bReWriteFile = false);

		std::string writeXML(std::string rootElem, bool bPretty = true, bool bTabs = true, PREWRITEPTR = NULL);

		bool writeXMLFile(std::string inStr, std::string rootElem, bool bPretty = true, bool bTabs = true, PREWRITEPTR = NULL);	
	
		std::string rootTag() { return sRootTag; }
		void rootTag(std::string rootElem) { sRootTag = rootElem; }

	private:
		void parseXMLElement(json::atom& ret, const TiXmlNode * elem);
		void writeXML(std::string & str, json::atom & ret, int depth, bool bPretty = true, bool bTabs = true);

		std::string sRootTag;
		
	};
}

#endif //_DATA_HPP

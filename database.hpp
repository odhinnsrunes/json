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

The official repository for this library is at https://github.com/odhinnsrunes/json

*/

#include "json.hpp"
#include <mutex>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace json
{
	class database{
		public:
			database(std::string sSetPath);
            ~database();
			document getConfig();
			document setConfig(document jSetConfig);

			document getConfigValue(std::string sKey);
			document setConfigValue(std::string sKey, value aValue);

			document getDocument(std::string id, std::string rev = "");
			document setDocument(document doc);

			std::string generateUUID();

			document save();

			typedef document (*MAPPTR)(value & doc);
			typedef document (*REDUCEPTR)(value & keys, value & values, bool bReReduce);

			document addView(std::string sSetName, std::string sSetVersion, MAPPTR setMap, REDUCEPTR setReduce = NULL);
			document getView(document & ret, std::string sName, document keys = document(), bool bReduce = true, size_t limit = 0);

			document cleanUpViews();

		private:
			static bool viewSort(json::value &a, json::value &b);
			document indexView(std::string &sName, std::string &sKeys, value &keys);
			size_t matchLevel(value& keys, value& mappedResult);
			value&  getViewWorker(value & ret, std::string & sName, value & keys, bool bReduce);

			struct view{
				MAPPTR map;
				REDUCEPTR reduce;
			};

			std::recursive_mutex mtx;
			std::string sPath;
			std::map <std::string, view> views;
			document data;
	};
	

}
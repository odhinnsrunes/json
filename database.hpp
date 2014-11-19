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
			document setConfigValue(std::string sKey, atom aValue);

			document getDocument(std::string id, std::string rev = "");
			document setDocument(document doc);

			std::string generateUUID();

			document save();

			typedef document (*MAPPTR)(atom & doc);
			typedef document (*REDUCEPTR)(document & keys, document & values, bool bReReduce);

			document addView(std::string sSetName, std::string sSetVersion, MAPPTR setMap, REDUCEPTR setReduce = NULL);
			document getView(std::string sName, document keys = document(), bool bReduce = true);

		private:
			static bool viewSort(json::atom a, json::atom b);
			void indexView(std::string sName, document& jDoc);

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
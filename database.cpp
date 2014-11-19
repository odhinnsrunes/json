#include "database.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace json
{
	database::database(std::string sSetPath)
	{
		mtx.lock();
		sPath = sSetPath;
		data.parseFile(sPath);
		if(!data.exists("config")){
			data["config"]["autoSave"] = true;
			data["config"]["pretty"] = false;
			data["config"]["maxRevisions"] = 0;

		}
		mtx.unlock();
	}

	document database::save()
	{
		mtx.lock();
		document ret;
		ret["success"] = data.writeFile(sPath, data["config"]["pretty"].boolean());
		mtx.unlock();
		return ret;
	}

	std::string database::generateUUID()
	{
		boost::uuids::uuid u = boost::uuids::random_generator()(); // initialize uuid

		return to_string(u);
	}

	document database::getConfig()
	{
		mtx.lock();
		document ret = data["config"];
		mtx.unlock();
		return ret;
	}

	document database::setConfig(document jSetConfig)
	{
		mtx.lock();
		document ret;
		data["config"] = jSetConfig;
		if(data["config"]["autoSave"].boolean()){
			ret = save();
		}
		mtx.unlock();
		return ret;
	}

	document database::getConfigValue(std::string sKey)
	{
		mtx.lock();
		document ret = data["config"][sKey];
		mtx.unlock();
		return ret;
	}

	document database::setConfigValue(std::string sKey, atom aValue)
	{
		mtx.lock();
		document ret;
		data["config"][sKey] = aValue;
		if(data["config"]["autoSave"].boolean()){
			ret = save();
		}
		mtx.unlock();
		return ret;
	}

	document database::getDocument(std::string id, std::string rev)
	{
		mtx.lock();
		document ret;
		if(data["data"].exists(id)){
			if(!rev.empty()){
				ret = data["data"][id]["docs"][rev];
			} else {
				iterator it = data["data"][id]["revs"].begin();
				if(it != data["data"][id]["revs"].end()){
					ret = data["data"][id]["docs"][(*it).string()];
				}
			}
		}
		mtx.unlock();
		return ret;
	}

	document database::setDocument(document doc)
	{
		mtx.lock();
		document ret;
		bool bOk = true;;
		if(doc.exists("_id")){
			if(!doc.exists("_rev")){
				ret["error"] = "Document already exists and no _rev was given.";
				bOk = false;
			} else {
				document oldDoc = getDocument(doc["_id"].string()); // TODO, do this without getDocument to make it faster
				if(doc["_rev"] != oldDoc["_rev"]){
					ret["error"] = "Document already exists and _rev given is not up to date.";
					bOk = false;
				}
			}
		} else {
			doc["_id"] = generateUUID();
		}
		if(bOk){
			doc["_rev"] = generateUUID();
			data["data"][doc["_id"].string()]["revs"].push_front(doc["_rev"].string());
			data["data"][doc["_id"].string()]["docs"][doc["_rev"].string()] = doc;
			size_t lMax = (size_t)data["config"]["maxRevisions"].integer();
			if(lMax > 0){
				size_t l = data["data"][doc["_id"].string()]["revs"].size();
				for(size_t lIndex = lMax; lIndex < l; lIndex++){
					data["data"][doc["_id"].string()]["docs"].erase(data["data"][doc["_id"].string()]["revs"].pop_back().string());
				}
			}
			ret["_id"] = doc["_id"];
			ret["_rev"] = doc["_rev"];
			for(std::map<std::string, view>::iterator it = views.begin(); it != views.end(); ++it){
				indexView(it->first, doc);
			}
			if(data["config"]["autoSave"].boolean()){
				ret["save"] = save();
			}
		}
		mtx.unlock();
		return ret;
	}

	document database::addView(std::string sSetName, std::string sSetVersion, MAPPTR setMap, REDUCEPTR setReduce)
	{
		mtx.lock();
		bool bReIndex = false;
		if(data["views"][sSetName]["version"] != sSetVersion){
			bReIndex = true;
		}
		views[sSetName].map = setMap;
		views[sSetName].reduce = setReduce;
		data["views"][sSetName]["version"] = sSetVersion;
		document ret;
		if(bReIndex){
			document temp;
			indexView(sSetName, temp);
			if(data["config"]["autoSave"].boolean()){
				ret["save"] = save();
			}
		}
		mtx.unlock();
		return ret;
	}

	bool database::viewSort(json::atom a, json::atom b)
	{
		return (a["key"] > b["key"]);
	}

	document database::getView(std::string sName, document keys, bool bReduce)
	{
		mtx.lock();
		document ret;
		if(!keys.empty()){
			size_t lRows = 0;
			for(iterator it = data["indeces"][sName].begin(); it != data["indeces"][sName].end(); ++it){
				if((*it)["key"] == keys){
					ret["data"].push_back((*it));
					lRows++;
				}
			}
			ret["rows"] = lRows;
		} else {
			ret["rows"] = data["indeces"][sName].size();
			ret["data"] = data["indeces"][sName];
		}
		ret["name"] = sName;
		mtx.unlock();
		return ret;
	}

	void database::indexView(std::string sName, document& jDoc)
	{
		mtx.lock();
		if(views.find(sName) != views.end()){
			if(views[sName].map){
				bool bNeedsSort = false;
				if(jDoc.empty()){
					data["indeces"][sName].clear();
					for(iterator it = data["data"].begin(); it != data["data"].end(); ++it){
						document ret = views[sName].map((*it)["docs"][(*it)["revs"][0].string()]);
						if(!ret.empty()){
							if(!ret["key"].isA(JSON_OBJECT)){
								document index;
								index["_id"] = it.key().string();
								
								index["key"] = ret["key"];
								index["value"] = ret["value"];
								data["indeces"][sName].push_back(index);
								bNeedsSort = true;
							}
						}
					}
				} else {
					document ret = views[sName].map(jDoc);
					if(!ret.empty()){
						bool bFound = false;
						document ret = views[sName].map(jDoc);
						if(!ret["key"].isA(JSON_OBJECT)){
							for(iterator it = data["indeces"][sName].begin(); it != data["indeces"][sName].end(); ++it){
								if((*it)["_id"] == jDoc["_id"]){
									bFound = true;								
									(*it)["key"] = ret["key"];
									(*it)["value"] = ret["value"];
									bNeedsSort = true;
								}
							}
							if(!bFound){
								document index;
								index["_id"] = jDoc["_id"];
								
								index["key"] = ret["key"];
								index["value"] = ret["value"];
								data["indeces"][sName].push_back(index);
								bNeedsSort = true;
							}
						}
					} else {
						for(iterator it = data["indeces"][sName].begin(); it != data["indeces"][sName].end(); ++it){
							if((*it)["_id"] == jDoc["_id"]){
								data["indeces"][sName].erase(it);
								break;
							}
						}
					}
				}
				if(bNeedsSort){
					data["indeces"][sName].sort(&viewSort);
				}
			}
		}
		mtx.unlock();
	}
}


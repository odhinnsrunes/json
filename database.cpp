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
		if(!data.exists("sequence")){
			data["sequence"] = 0;
		}
		mtx.unlock();
	}

	database::~database()
	{
		save();
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
		static boost::uuids::basic_random_generator<boost::mt19937> gen;
		boost::uuids::uuid u = gen();
//		boost::uuids::uuid u = boost::uuids::random_generator()(); // initialize uuid

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

	document database::setConfigValue(std::string sKey, value aValue)
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
		bool bOk = true;
		std::string id;

		if(doc.exists("_id")){
			id.assign(doc["_id"].string());
			if(!doc.exists("_rev") && data["data"].exists(id)){
				ret["error"] = "Document already exists and no _rev was given.";
				bOk = false;
			} else {
				if(data["data"].exists(id)){			
					std::string oldRev;
					iterator it = data["data"][id]["revs"].begin();
					if(it != data["data"][id]["revs"].end()){
						oldRev = (*it).string();
					}		
					if(doc["_rev"] != oldRev){
						ret["error"] = "Document already exists and _rev given is not up to date.";
						bOk = false;
					}
				}	
			}
		} else {
			id.assign(generateUUID());
			doc["_id"] = id;
		}
		if(bOk){
			doc["_rev"] = generateUUID();
			size_t lRevs = data["data"][doc["_id"].string()]["revs"].size();
			data["data"][doc["_id"].string()]["revs"].push_front(doc["_rev"].string());
			data["data"][doc["_id"].string()]["docs"][doc["_rev"].string()] = doc;
			data["data"][doc["_id"].string()]["sequence"] = data["sequence"];
			data["sequenceIndex"][data["sequence"].integer()]["_id"] = doc["_id"].string();
			data["sequenceIndex"][data["sequence"].integer()]["_rev"] = doc["_rev"].string();
			data["sequence"] = data["sequence"] + 1;
			size_t lMax = (size_t)data["config"]["maxRevisions"].integer();
			if(lMax > 0){
				for(size_t lIndex = lMax; lIndex < lRevs; lIndex++){
					data["data"][doc["_id"].string()]["docs"].erase(data["data"][doc["_id"].string()]["revs"].pop_back().string());
				}
			}
			ret["_id"] = doc["_id"];
			ret["_rev"] = doc["_rev"];
			if(data["config"]["autoSave"].boolean()){
				ret["save"] = save();
			}
		}
		mtx.unlock();
		return ret;
	}

	document database::deleteDocument(std::string id, std::string rev)
	{
		mtx.lock();
		document ret;
		if(data["data"].exists(id)){
			document doc = setDocument(getDocument(id));
			data["data"][id]["deleted"][doc["_rev"].string()] = true;
			ret["_id"] = id;
			ret["_rev"] = doc["_rev"];
			ret["deleted"] = true;
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
			data["indeces"].erase(sSetName);
			if(data["config"]["autoSave"].boolean()){
				ret["save"] = save();
			}
		}
		mtx.unlock();
		return ret;
	}

	bool database::viewSort(json::value &a, json::value &b)
	{
		return (a["key"] < b["key"]);
	}

	size_t database::matchLevel(value& keys, value& mappedResult)
	{
		if(keys.isA(JSON_ARRAY) && mappedResult["key"].isA(JSON_ARRAY)){
			size_t l = keys.size();
			size_t ret = 0;
			for(; ret < l; ret++){
				if(keys[ret] != mappedResult["key"][ret]){
					break;
				}
			}
			return ret;
		} else if(keys == mappedResult["key"]){
			return 1;
		} else {
			return 0;
		}
	}

	document database::getView(document & ret, std::string sName, document keys, bool bReduce, size_t limit, size_t offset)
	{
		mtx.lock();
		if(views.find(sName) == views.end()){
			mtx.unlock();
			return document();
		}
        ret.clear();
		if(keys.exists("key")){
			getViewWorker(ret, sName, keys["key"], bReduce);
			if(offset && ret["rows"].isA(JSON_ARRAY)){
				myVec::iterator it = ret["rows"].begin().arr();
				it += offset;
				if(it != ret["rows"].end().arr()){
					ret["rows"].erase(ret["rows"].begin(), it);
				}
			}
			if(limit){
				ret["rows"].resize(limit);
			}
		} else if(keys.exists("keys")){
			size_t i = 0;
			document temp;
			ret["total_rows"] = 0;
			for(value & at : keys["keys"]){
				getViewWorker(temp[i++], sName, at, bReduce);
			}
			for(value & val : temp){
				ret["total_rows"] += val["total_rows"];
				ret["rows"] += val["rows"];
			}
			if(offset && ret["rows"].isA(JSON_ARRAY)){
				myVec::iterator it = ret["rows"].begin().arr();
				it += offset;
				if(it != ret["rows"].end().arr()){
					ret["rows"].erase(ret["rows"].begin(), it);
				}
			}
			if(limit){
				ret["rows"].resize(limit);
			}
			if(bReduce && views[sName].reduce){
				value rere; 
				// this is where grouping needs to happen.
				for(value & val : ret["rows"]){
					rere.push_back(val["value"]);
				}
				ret["rows"].clear();
				value n = value((char*)NULL);
				ret["rows"][0]["value"] = views[sName].reduce(n, rere, true);
				ret["rows"][0]["key"] = (char*)NULL;
				ret["total_rows"] = 1;
			}
		} else {
			getViewWorker(ret, sName, keys, bReduce);
		}
		return ret;

	}

	value & database::getViewWorker(value & ret, std::string & sName, value & keys, bool bReduce)
	{
		std::string sKeys = document(keys).write();
		indexView(sName, sKeys, keys);
		
		ret["total_rows"] = data["indeces"][sName][sKeys]["data"].size();
		ret["rows"] = data["indeces"][sName][sKeys]["data"];

		if(bReduce && views[sName].reduce){
			document values;
			values.emptyArray();
			for(value & val : ret["rows"]){
				values.push_back(val["value"]);
			}
			ret["rows"].clear();
			ret["rows"][0]["value"] = views[sName].reduce(keys, values, false);
			ret["rows"][0]["key"] = (char*)NULL;
			ret["total_rows"] = 1;
		}
		ret["name"] = sName;
		mtx.unlock();
		return ret;
	}


	document database::indexView(std::string &sName, std::string &sKeys, value &keys)
	{
		mtx.lock();
		document ret;
		if(views.find(sName) != views.end()){
			if(views[sName].map){
				data["indeces"][sName][sKeys]["data"];
				iterator itIndex = data["indeces"][sName][sKeys].find("data");
				bool bEmpty = (*itIndex).empty();
				size_t sequence = data["indeces"][sName][sKeys]["sequence"].integer();
				size_t latest = data["sequence"].integer();
				data["indeces"][sName][sKeys]["sequence"] = latest;
				document newChanges;
				size_t keysSize = keys.size();
				for(size_t i = sequence + 1; i < latest; i++){
					std::string id = data["sequenceIndex"][i]["_id"].string();
					std::string rev = data["sequenceIndex"][i]["_rev"].string();
					if(data["data"][id]["sequence"] == i){ // skip out of date documents
						document ret2 = views[sName].map(data["data"][id]["docs"][rev]);
						bool bDeleted = false;
						if(data["data"][id]["deleted"][rev].boolean() == true){
							bDeleted = true;
						}
						if(!ret2.empty() && !(bEmpty && bDeleted)){
							if(!ret2["key"].isA(JSON_OBJECT)){
								if(matchLevel(keys, ret2) >= keysSize){
									document index;
									index["id"] = id;
									
									index["key"] = ret2["key"];
									index["value"] = ret2["value"];
									if(bDeleted){
										index["deleted"] = true;
									}
									newChanges.push_back(index);
								}
							}
						}
					}
				}
				if(newChanges.empty()){
					mtx.unlock();
					return ret;
				}

				newChanges.sort(&viewSort);

				if(bEmpty){
					(*itIndex) = newChanges;
					if(data["config"]["autoSave"].boolean()){
						ret["save"] = save();
					}
					mtx.unlock();
					return ret;
				}

				iterator itNew = newChanges.begin();
				if(itNew != newChanges.end()){
					size_t l = (*itIndex).size();
					for(size_t i = 0; i < l; i++){
						if((*itIndex)[i]["id"] == (*itNew)["id"]){
							(*itIndex).erase(i);
							l--;
							++itNew;
							if(itNew == newChanges.end()){
								break;
							}
						}
					}
					itNew = newChanges.begin();
					for(size_t i = 0; i < l; i++){
						if((*itNew)["deleted"].boolean()){
							++itNew;
							if(itNew == newChanges.end()){
								break;
							}
						} else if((*itIndex)[i]["key"] > (*itNew)["key"]){
							(*itIndex).insert(i, (*itNew));
							l++;
							++itNew;
							if(itNew == newChanges.end()){
								break;
							}
						}
					}
					for(;itNew != newChanges.end(); ++itNew){
						if(!(*itNew)["deleted"].boolean())
							(*itIndex).push_back((*itNew));
					}
					if(data["config"]["autoSave"].boolean()){
						ret["save"] = save();
					}
				}				
			}
		}
		mtx.unlock();
		return ret;
	}
	
	document database::cleanUpViews()
	{
		mtx.lock();
		data["indeces"].clear();
		document ret;
		if(data["config"]["autoSave"].boolean()){
			ret["save"] = save();
		}
		mtx.unlock();
		return ret;
	}
}


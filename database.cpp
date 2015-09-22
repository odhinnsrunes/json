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

#include "database.hpp"

extern "C"
{
#ifdef WIN32
#include <Rpc.h>
#else
#include <uuid/uuid.h>
#endif
}

namespace json
{
	database::database()
	{

	}

	database::database(std::string sSetPath)
	{
		init(sSetPath);
	}

	void database::init(const std::string &sSetPath)
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
	#ifdef WIN32
		UUID uuid;
		UuidCreate ( &uuid );

		unsigned char * str;
		UuidToStringA ( &uuid, &str );

		std::string s( ( char* ) str );

		RpcStringFreeA ( &str );
	#else
		uuid_t uuid;
		uuid_generate_random ( uuid );
		char s[37];
		uuid_unparse ( uuid, s );
	#endif
		return s;	

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
		iterator it = data["data"].find(id);
		if(it != data["data"].end()){
             if(!rev.empty()){
                 if(data["data"][id]["deleted"][rev] == true){
                     ret["_id"] = id;
                     ret["_rev"] = rev;
                     ret["_deleted"] = true;
                 } else {
                     ret = (*it)["docs"][rev];
                 }
             } else if(data["data"][id].exists("deleted")) {
                 ret["error"] = "not_found";
                 ret["reason"] = "deleted";
             } else {
				iterator rit = (*it)["revs"].begin();
				if(rit != (*it)["revs"].end()){
					ret = (*it)["docs"][(*rit).string()];
                } else {
                    ret["error"] = "not_found";
                    ret["reason"] = "missing";
                }
			}
        } else {
            ret["error"] = "not_found";
            ret["reason"] = "missing";
        }
		mtx.unlock();
		return ret;
	}

	document database::setDocument(document doc, bool bDontSave)
	{
		mtx.lock();
		document ret;
		bool bOk = true;
		std::string id;

		if(doc.exists("_id")){
			id.assign(doc["_id"].string());
			if(!doc.exists("_rev") && data["data"].exists(id)){
				ret["error"] = "conflict";
                ret["reason"] = "Document update conflict.";
				bOk = false;
			} else {
				if(data["data"].exists(id)){			
					std::string oldRev;
					iterator it = data["data"][id]["revs"].begin();
					if(it != data["data"][id]["revs"].end()){
						oldRev = (*it).string();
					}		
					if(doc["_rev"] != oldRev){
                        ret["error"] = "conflict";
                        ret["reason"] = "Document update conflict.";
						bOk = false;
					}
				}	
			}
		} else {
			id.assign(generateUUID());
			doc["_id"] = id;
		}
		if(bOk){
            size_t lRevs = data["data"][id]["revs"].size();
            i64 revIndex = data["data"][id]["revindex"].integer() + 1;
            data["data"][id]["revindex"] = revIndex;
            std::string newRev = std::to_string(revIndex);
            newRev.append("-");
            newRev.append(generateUUID());
			doc["_rev"] = newRev;
			data["data"][id]["revs"].push_front(newRev);
			data["data"][id]["docs"][newRev] = doc;
			data["data"][id]["sequence"] = data["sequence"];
            data["sequenceIndex"][data["sequence"]._size_t()]["_id"] = id;
            data["sequenceIndex"][data["sequence"]._size_t()]["_rev"] = newRev;
			data["sequence"] = data["sequence"] + 1;
			size_t lMax = (size_t)data["config"]["maxRevisions"].integer();
			if(lMax > 0){
				for(size_t lIndex = lMax; lIndex < lRevs; lIndex++){
					data["data"][id]["docs"].erase(data["data"][id]["revs"].pop_back().string());
				}
			}
			ret["_id"] = doc["_id"];
			ret["_rev"] = doc["_rev"];
			if(data["config"]["autoSave"].boolean() && !bDontSave){
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
        
        json::document doc = getDocument(id);
        if(doc.exists("error")){
            ret = doc;
        } else {
            if(data["data"][id]["deleted"][doc["_rev"].string()].boolean()) {
                ret["error"] = "not_found";
                ret["reason"] = "deleted";
            } else if(doc["_rev"] == rev){
                json::document res = setDocument(doc, true);
                data["data"][id]["deleted"][res["_rev"].string()] = true;
                if(data["config"]["autoSave"].boolean()){
                    ret["save"] = save();
                }
                ret["ok"] = true;
                ret["id"] = id;
                ret["rev"] = res["_rev"];
            } else {
                ret["error"] = "conflict";
                ret["reason"] = "Document update conflict.";
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

#ifdef __GNUC__
    bool database::viewSort(const json::value &a, const json::value &b)
#else
    bool database::viewSort(json::value &a, json::value &b)
#endif
	{
		return (*(a.find("key")) < *(b.find("key")));
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

	document &database::getView(document & ret, const std::string sName, document keys, bool bReduce, size_t limit, size_t offset)
	{
		mtx.lock();
		std::string mmName = sName;
		if(views.find(mmName) == views.end()){
			ret["error"] = "not_found";
            ret["reason"] = "missing_named_view";
            mtx.unlock();
            return ret;
		}
		if(bReduce && views[mmName].reduce == NULL){
            ret["error"] = "query_parse_error";
            ret["reason"] = "Reduce is invalid for map-only views.";
            mtx.unlock();
            return ret;
        }
		ret.clear();
		if(keys.exists("key")){
			getViewWorker(ret, mmName, keys["key"], bReduce);
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
				getViewWorker(temp[i++], mmName, at, bReduce);
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
			if(bReduce && views[mmName].reduce){
				value rere; 
				// this is where grouping needs to happen.
				for(value & val : ret["rows"]){
					rere.push_back(val["value"]);
				}
				ret["rows"].clear();
				value n = value((char*)NULL);
				ret["rows"][0]["value"] = views[mmName].reduce(n, rere, true);
				ret["rows"][0]["key"] = (char*)NULL;
//				ret["total_rows"] = 1;
			}
		} else {
			getViewWorker(ret, mmName, keys, bReduce);
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
		}
        ret["update_seq"] = data["sequence"];
		return ret;

	}

	value & database::getViewWorker(value & ret, const std::string & sName, value & keys, bool bReduce)
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
//		ret["name"] = sName;
		mtx.unlock();
		return ret;
	}


	document database::indexView(const std::string &sName, std::string &sKeys, value &keys)
	{
		mtx.lock();
		document ret;
		if(views.find(sName) != views.end()){
			if(views[sName].map){
				data["indeces"][sName][sKeys]["data"];
				iterator itIndex = data["indeces"][sName][sKeys].find("data");
				bool bEmpty = (*itIndex).empty();
                size_t sequence = data["indeces"][sName][sKeys]["sequence"]._size_t();
                size_t latest = data["sequence"]._size_t();
				data["indeces"][sName][sKeys]["sequence"] = latest;
				document newChanges;
				size_t keysSize = keys.size();
				for(size_t i = sequence; i < latest; i++){
					std::string id = data["sequenceIndex"][i]["_id"].string();
					std::string rev = data["sequenceIndex"][i]["_rev"].string();
					if(data["data"][id]["sequence"] == i){ // skip out of date documents
						document ret2 = views[sName].map(data["data"][id]["docs"][rev]);
						bool bDeleted = false;
						if(data["data"][id]["deleted"][rev].boolean() == true){
							bDeleted = true;
						}
						if(!ret2.empty() && !(bEmpty && bDeleted)){
							if(ret2.isA(JSON_ARRAY)){
								for(value & val : ret2){
									if(val.isA(JSON_OBJECT)){
										if(!val["key"].isA(JSON_OBJECT)){
											if(matchLevel(keys, val) >= keysSize){
												document index;
												index["id"] = id;
												
												index["key"] = val["key"];
												index["value"] = val["value"];
												if(bDeleted){
													index["deleted"] = true;
												}
												newChanges.push_back(index);
											}
										}
									}
								}
							} else if (ret2.isA(JSON_OBJECT)) {
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


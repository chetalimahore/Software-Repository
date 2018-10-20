#pragma once
///////////////////////////////////////////////////////////////////////
// Checkin.h - Provides checkin of files into the repository storage  //
// ver 1.0														   	  //
// Chetali Mahore, CSE687 - Object Oriented Design, Spring 2018		  //
//////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides Checkin class:
* - This class contains functions like checkin, create dependency and parent map and closing the file
*
* The package also provides functions:
* - checkin() - provides the checkin operation based on the current status of the file whether open or close
* - findDependent() - finds the dependent files for storing them in the maps
* - createMap() - creates the dependency and the parent map for performing the checkin operation
* - closeFile() - recursive call to close the dependency files if open and then close itself 
*
* Required Files:
* ---------------
* DbCore.h
*
* Maintenance History:
* --------------------
* ver 1.0 - 2nd March 2018
* - handled transitive and circular dependency of files
*/


#include "../NoSQLDB/Executive/NoSqlDb.h"
#include "../CodeRepo/CodeRepo.h"

using namespace NoSqlDb;

template<typename P>
class Checkin {
  public:
	  static DbCore<P>& checkin(DbCore<P>& repo_dbcore, std::string path,DbElement<P> prev,DbElement<P> dbelem, std::string fullfile, int version);
	  static void findDependent(DbCore<P>& repo_dbcore, std::string filename, std::vector<std::string>& dependency);
	  static void closeFile(DbCore<P>& repo_dbcore, std::string filename);
	  static void createMap(DbCore<P> & repo_dbcore, std::string filename, std::vector<std::string> dependency);
private:
	static std::unordered_map<Key, std::vector<std::string>> dependantmap;
	static std::unordered_map<Key, std::vector<std::string>> parentMap;

};

template<typename P>
std::unordered_map<Key, std::vector<std::string>>  Checkin<P>::dependantmap = {};

template<typename P>
std::unordered_map<Key, std::vector<std::string>> Checkin<P>::parentMap = {};


/*------------finds the dependent files for storing them in the maps------------*/

template<typename P>
inline void Checkin<P>::findDependent(DbCore<P>& repo_dbcore, std::string filename, std::vector<std::string>& dependency) {
	for (size_t i = 0; i < dependency.size(); i++) {
		DbElement<P> dbelem = repo_dbcore[dependency[i]];
		if (dbelem.payLoad().getStatus() != open) {
			if (dbelem.payLoad().getStatus() == closing) {
				if (parentMap.find(filename) != parentMap.end()) {
					std::vector<std::string> keys = parentMap[filename];
					for (size_t j = 0;j < keys.size(); j++) {
						if (keys[j] == dependency[i]) {
							dependency.erase(dependency.begin() + i);
							i--;
						}
					}
				}
			}
			else {
				dependency.erase(dependency.begin() + i);
				i--;
			}
		}
	}
	createMap(repo_dbcore, filename, dependency);
}

/*------------------creates the dependency and the parent map for performing the checkin operation---------------*/

template<typename P>
inline void Checkin<P>::createMap(DbCore<P>& repo_dbcore, std::string filename, std::vector<std::string> dependency) {
	if (dependency.size() > 0) {
		//dependantmap.insert(filename, dependency);
		dependantmap[filename] = dependency;
		for (size_t i = 0;i < dependency.size(); i++) {
			//parentMap.insert(dependency[i], filename);
			if (parentMap.find(dependency[i]) != parentMap.end()) {
				parentMap[dependency[i]].push_back(filename);
			}
			else {
				std::vector<std::string> parents;
				parents.push_back(filename);
				parentMap[dependency[i]] = parents;
			}
		}
		DbElement<P> dbelem = repo_dbcore[filename];
		dbelem.payLoad().setStatus(closing);
		repo_dbcore[filename] = dbelem;
	}
	else {
		closeFile(repo_dbcore, filename);
	}
}

/*------------------recursive call to close the dependency files if open and then close itself ------------*/

template<typename P>
inline void Checkin<P>::closeFile(DbCore<P> & repo_dbcore, std::string filename) {
	if (parentMap.find(filename) != parentMap.end()) {
		std::vector<std::string> parents = parentMap[filename];
		for (size_t i = 0; i < parents.size(); i++) {
			std::vector<std::string> child = dependantmap[parents[i]];
			for (size_t j = 0; j < child.size(); j++) {
				if (child[j] == filename) {
					child.erase(child.begin() + j);
					j--;
					dependantmap[parents[i]] = child;
					break;
				}
			}
			if (child.empty()) {
				closeFile(repo_dbcore, parents[i]);
			}
		}
		}
		repo_dbcore[filename].payLoad().setStatus(close);

}

/*--------------provides the checkin operation based on the current status of the file whether open or close-------------*/

template<typename P>
inline DbCore<P>& Checkin<P>::checkin(DbCore<P>& repo_dbcore, std::string path, DbElement<P> prev, DbElement<P> dbelem, std::string fullfile, int version) {

	if (prev.payLoad().getStatus() == open) {
		std::string source = dbelem.payLoad().value();
		std::string destination;
		destination.append(path).append("/").append(dbelem.payLoad().getNamespace()).append("/").append(CodeRepo<P>::extractFilename(source)).append(".").append(std::to_string(version));
		bool flag = FileSystem::File::copy(source, destination, false);
		if (flag) {
			dbelem.payLoad().value(destination);
			repo_dbcore.addRecord(fullfile, dbelem);
		}
	}
	else if(prev.payLoad().getStatus() == close || prev.payLoad().getStatus() == closing)  {
		int ver = ++version;
		fullfile = fullfile.substr(0, fullfile.find_last_of(".") + 1).append(std::to_string(ver));
		//success = copyFileToRepo(path, filename, dbelem);
		std::string source = dbelem.payLoad().value();
		std::string destination;

		destination.append(path).append("/").append(dbelem.payLoad().getNamespace()).append("/").append(CodeRepo<P>::extractFilename(source)).append(".").append(std::to_string(ver));
		bool flag = FileSystem::File::copy(source, destination, false);
		if (flag) {
			dbelem.payLoad().value(destination);
			repo_dbcore.addRecord(fullfile, dbelem);
			
		}
	}
	if (dbelem.payLoad().getStatus() == close) {
		std::vector<std::string> dependency;
		dependency.push_back(fullfile);
		CodeRepo<P>::findDependencies(repo_dbcore, dependency);
		std::string filename = dependency[0];
		dependency.erase(dependency.begin()+ 0);
		if (dependency.size() > 0) {
			findDependent(repo_dbcore, filename, dependency);
		}
		else {
		 closeFile(repo_dbcore, filename);
		}
	}

	return repo_dbcore;
}

#pragma once

////////////////////////////////////////////////////////////////////////////////////////
// CodeRepo.h - Common entry point for performing checkin, checkout, browse operations  //
// ver 1.0																				//
// Chetali Mahore, CSE687 - Object Oriented Design, Spring 2018							//
/////////////////////////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides CodeRepo class:
* - This class contains utilities functions like extractFilename, findDependencies, openFile, checkDirExist
*
* The package also provides functions:
* - checkDirExist() - returns bool whether the directory exists or not, if not, it creates the directory
* - checkInFiles() - it passes the required arguments to the checkin function of the checkin package
* - extractFilename() - it extracts the filename from the specified path and returns it
* - checkOut() - it passes the required arguments to the checkout function of the checkout package
* - contains() - utility function to check whether an element is present in the vector or not
* - findDependencies() - utility function to fetch all the dependency and transitive dependency files
* - getKeys() - it returns the list of keys with the matching regex expression
* - browseFile() - Fetch the required parameters and pass it to the browse function of the Browse package
* - persist() - Persist the database into XML file
* 
* 
* Required Files:
* ---------------
* NoSqlDb.h, Payload.h
* FileMgr.h, Process.h
* Checkout.h, Browse.h
* Version.h, Checkin.h
*
* Maintenance History:
* --------------------
* ver 1.2 : 30th April 2018
* - added the browse query
*
* ver 1.1 : 2nd March 2018
* - added calls to checkin, browse, version and checkout packages
*
* ver 1.0 : 28th Feb 2018
* - added utility functions of extracting filename, contains the keys, finding dependencies
*/

#include<iostream>
#include<string>
#include "../NoSQLDB/Executive/NoSqlDb.h"
#include "../NoSQLDB/PayLoad/PayLoad.h"
#include "../FileMgr/FileMgr.h"
#include "../Process/Process/Process.h"
#include "../Checkout/Checkout.h"
#include "../Browse/Browse.h"
#include "../Version/Version.h"
#include "../Checkin/Checkin.h"

using namespace NoSqlDb;

template<typename P>
class CodeRepo {

public:

	CodeRepo(DbCore<P>& dbcore) { repo_dbcore = dbcore; }
	DbCore<P>& getRepoDb() { return repo_dbcore; }

	bool checkDirExist(std::string& path, std::string namespace_);
	DbCore<P>& checkInFiles(DbElement<P> & dbelem);

	void setRepoDirectory(std::string repoDir) { repoDirectory = repoDir; }
	std::string getRepoDirectory() { return repoDirectory; }

	void setCheckoutDirectory(std::string checkout) { checkoutDirectory = checkout; }
	std::string getCheckoutDirectory() { return checkoutDirectory; }

	static std::string extractFilename(std::string filepath);

	static std::string extractFile(std::string filename);

	std::vector<std::string> checkOut(std::string filename, std::string version);

	static bool contains(std::vector<std::string> list_vectors, std::string filename);

	std::vector<std::string> browseFile(std::string filename, std::string version);

	void persist(std::string filepath);

	std::vector<std::string> getKeys(std::string filename);

	static std::vector<std::string>& findDependencies(DbCore<P> & repo_dbcore, std::vector<std::string>& dependency);

	std::vector<std::string> queryDatabase(std::string category, std::string dependency, std::string file_parent);

	std::vector<std::string> returnParent();

	std::vector<std::string> returnCategory(std::string category);

	std::vector<std::string> returnDependency(std::string dependency);

	std::vector<std::string> returnQueryResult(std::vector<std::string> resultKeys);
	
private:
	 DbCore<P> repo_dbcore;
	std::string repoDirectory;
	std::string checkoutDirectory;
	static std::string root;
};

template<typename P>
std::string CodeRepo<P>::root = "Chetali";

using namespace FileManager;
using Dir = FileSystem::Directory;

/*---------returns bool whether the directory exists or not, if not, it creates the directory------------*/

template<typename P>
inline bool CodeRepo<P>::checkDirExist(std::string & path, std::string namespace_) {

	std::vector<std::string> subdirs = FileSystem::Directory::getDirectories(path, "*.*");
	for (auto itr : subdirs) {
		if (itr == namespace_) {
			return true;
		}
	}
	return false;
}

/*------------it extracts the filename from the specified path and returns it--------------*/

template<typename P>
inline std::string CodeRepo<P>::extractFilename(std::string filename) {
	const size_t last_slash_idx = filename.find_last_of("\\/");
	if (std::string::npos != last_slash_idx) {
		filename.erase(0, last_slash_idx + 1);
	}
	return filename;
}


template<typename P>
inline std::string CodeRepo<P>::extractFile(std::string filename) {
	const size_t last_slash_idx = filename.find_last_of("\\.");
	if (std::string::npos != last_slash_idx) {
		filename.erase(last_slash_idx, filename.length());
	}

	const size_t last_slash = filename.find_last_of("\\:");
	if (std::string::npos != last_slash) {
		filename.erase(0, last_slash + 1);
	}

	return filename;
}



using File = FileSystem::File;

/*-------------it returns the list of keys with the matching regex expression-------------------------*/

template<typename P>
inline std::vector<std::string> CodeRepo<P>::getKeys(std::string fullfile) {

	Query<P> query(repo_dbcore);
	std::vector<std::string> regkeys = query.regexKeys(fullfile).keys();
	return regkeys;
}

/*---------------utility function to fetch all the dependency and transitive dependency files----------------*/

template<typename P>
inline std::vector<std::string>& CodeRepo<P>::findDependencies(DbCore<P> & repo_dbcore, std::vector<std::string> &dependency)
{
	// TODO: insert return statement here
	for(size_t i = 0; i < dependency.size(); i++) {
		DbElement<P> dbelem = repo_dbcore[dependency[i]];
		for (auto child : dbelem.children()) {
			if (!CodeRepo<P>::contains(dependency, child)) {
				dependency.push_back(child);
			}
		}
	}
	return dependency;
}

/*--------it passes the required arguments to the checkin function of the checkin package----------------*/

template<typename P>
inline DbCore<P>& CodeRepo<P>::checkInFiles(DbElement<P> & dbelem){

	std::string key = dbelem.name();
	if (key != root) {
		std::cout << "---------------------------------";
		std::cout << "\n Unauthorised user is not allowed to checkin in the repository \n";
	}
	else {
		std::string path = getRepoDirectory();
		std::string namespace_ = dbelem.payLoad().getNamespace();
		std::string filepath = dbelem.payLoad().value();
		std::string filename = extractFilename(filepath);
		std::string fullfile;
		fullfile.append(namespace_).append("::").append(filename);

		if (!checkDirExist(path, namespace_)) {
			std::cout << "\n Creating directory '" << namespace_ << "' \n \n ";
			FileSystem::Directory::create(path.append("/").append(namespace_));
		}

		std::vector<std::string> regkeys = getKeys(fullfile);
		if (!regkeys.empty()) {
			int version = Version::extractVersion(regkeys);
			DbElement<P> dbelement = repo_dbcore[fullfile.append(".").append(std::to_string(version))];
			Checkin<P>::checkin(repo_dbcore, path, dbelement, dbelem, fullfile, version);

		}
		else {
			std::string source = dbelem.payLoad().value();
			std::string destination;
			destination.append(getRepoDirectory()).append("/").append(dbelem.payLoad().getNamespace()).append("/").append(filename).append(".").append("1");
			bool flag = FileSystem::File::copy(source, destination, false);
			if (flag) {
				dbelem.payLoad().value(destination);
				repo_dbcore.addRecord(fullfile.append(".1"), dbelem);

			}
		}
	}
	

	return repo_dbcore;
}

/*-------------utility function to check whether an element is present in the vector or not--------------------*/

template<typename P>
inline bool CodeRepo<P>::contains(std::vector<std::string> list_vectors, std::string filename){
	for (auto itr : list_vectors) {
		if (itr == filename) {
			return true;
		}
	}
	return false;
}

/*-------it passes the required arguments to the checkout function of the checkout package---------------*/

template<typename P>
inline std::vector<std::string> CodeRepo<P>::checkOut(std::string filename, std::string version) {
	Query<P> query(repo_dbcore);
	std::vector<std::string> checkfiles; 
	if (version == "0") {
		std::vector<std::string> regkeys = query.regexKeys(filename).keys();
		if (!regkeys.empty()) {
			int version = Version::extractVersion(regkeys);
			filename.append(".").append(std::to_string(version));
			std::cout << "\n The latest version of " << filename << "is : " << version << "\n ";
		}
	}
	else {
		filename.append(".").append(version);
	}
	DbElement<P> dbelem = repo_dbcore[filename];
	std::vector<std::string> dependency;
	dependency.push_back(filename);
	int index = 0;

	findDependencies(repo_dbcore, dependency);

	checkfiles = Checkout::checkout(repo_dbcore, dependency, filename, getCheckoutDirectory());

	return checkfiles;
}

/*---------------Persist the database into XML file---------------*/

template<typename P>
void CodeRepo<P>::persist(std::string filepath) {
	Persist<P> persist(repo_dbcore);
	std::string xml = persist.toXml();
	std::ofstream outputFile(filepath);
	outputFile << xml;
	outputFile.close();
}

/*--------------Fetch the required parameters and pass it to the browse function of the Browse package---------------*/

template<typename P>
inline std::vector<std::string> CodeRepo<P>::browseFile(std::string filename, std::string version) {
	Query<P> query(repo_dbcore);
	std::vector<std::string> checkfiles;
	std::vector<std::string> dependency;

	if (version == "0") {
		std::vector<std::string> regkeys = query.regexKeys(filename).keys();
		if (!regkeys.empty()) {
			int version = Version::extractVersion(regkeys);
			filename.append(".").append(std::to_string(version));
		}
	}
	else {
		filename.append(".").append(version);
	}

	dependency.push_back(filename);

	//Browse<P>::browse(repo_dbcore, filename);

	checkfiles = Checkout::checkout(repo_dbcore, dependency, filename, getCheckoutDirectory());

	return checkfiles;
}

/*------------helper function to return files with no dependencies-----------*/

template<typename P>
inline std::vector<std::string> CodeRepo<P>::returnParent() {
	Query<P> query2(repo_dbcore);
	std::vector<std::string> vecKeys;
	auto hasChild = [](DbElement<PayLoad>& elem) {
		return (elem.children().size() == 0);
	};
	vecKeys = query2.select(hasChild).keys();

	return vecKeys;
}

/*--------helper function to return filenames with a specific category--------------*/

template<typename P>
inline std::vector<std::string> CodeRepo<P>::returnCategory(std::string category) {
	Query<P> query1(repo_dbcore);
	std::vector<std::string> vecKeys;
	auto hasCategory = [&category](DbElement<PayLoad>& elem) {
		return  (elem.payLoad()).hasCategory(category);
	};
	vecKeys = query1.select(hasCategory).keys();
	return vecKeys;
}

/*--------helper function to return filenames with a specific dependency--------------*/

template<typename P>
inline std::vector<std::string> CodeRepo<P>::returnDependency(std::string dependency) {
	Query<P> query(repo_dbcore);
	std::vector<std::string> vecKeys;
	Keys keys_dep{ dependency };
	Conditions<PayLoad> conds0;
	conds0.children(keys_dep);
	vecKeys = query.select(conds0).keys();

	return vecKeys;
}

/*--------------helper function to return filenames without the root file-----------*/

template<typename P>
inline std::vector<std::string> CodeRepo<P>::returnQueryResult(std::vector<std::string> resultKeys) {
	std::vector<std::string> checkfiles, dependen1;

	if (resultKeys.size() > 0) {
		if (resultKeys.at(0) == "root") {
			resultKeys.erase(resultKeys.begin());
		}
		if (resultKeys.size() > 0)
			dependen1.push_back(resultKeys.at(0));
		checkfiles = Checkout::checkout(repo_dbcore, dependen1, resultKeys.at(0), getCheckoutDirectory());
	}
	return resultKeys;
}


/*-----------------Query the database and return the appropriate result--------------------*/

template<typename P>
inline std::vector<std::string> CodeRepo<P>::queryDatabase(std::string category, std::string dependency, std::string file_parent) {
	
	std::vector<std::string> depen, dependen1, catkeys, resultKeys, parentKeys;
	
	if (file_parent == "true" ) {
		parentKeys = returnParent();
	}
	if (category != "") {
		
		catkeys = returnCategory(category);
	}
	if (dependency != "") {
		
		depen = returnDependency(dependency);
	}
	if (file_parent == "true") {
		resultKeys = parentKeys;
	}
	else if (category == "") {
		resultKeys = depen;
	}
	else if (dependency == "") {
		resultKeys = catkeys;
	}
	else if(dependency != "" && category != ""){
		for (size_t i = 0; i < catkeys.size(); i++)
		{
			if (contains(depen, catkeys[i])) {
				resultKeys.push_back(catkeys[i]);
			}
		}
	}
	resultKeys = returnQueryResult(resultKeys);
	return resultKeys;
}


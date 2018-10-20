#pragma once
//////////////////////////////////////////////////////////////////////////////
// Checkout.h - Provides checkout of files into the checkedOutFiles storage  //
// ver 1.0														   			 //
// Chetali Mahore, CSE687 - Object Oriented Design, Spring 2018				 //
//////////////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides Checkout class:
* - This class contains checkout function which will copy the file and its dependencies from the repository storage to the checkedOutFiles folder
*
* Required Files:
* ---------------
* DbCore.h
*
* Maintenance History:
* --------------------
* ver 1.0 - 2nd March 2018
* - Checkedout the files and the dependencies
*/

#include<vector>
#include<string>
#include "../NoSQLDB/Executive/NoSqlDb.h"
#include "../CodeRepo/CodeRepo.h"

using namespace NoSqlDb;


class Checkout {
public:
	template<typename P> static std::vector<std::string> checkout(DbCore<P> &dbcore, std::vector<std::string> fileList, std::string filename, std::string destination);
};

/*----------copy the file and its dependencies from the repository storage to the checkedOutFiles folder-------------*/


template<typename P> std::vector<std::string> Checkout::checkout(DbCore<P> &repo_dbcore, std::vector<std::string> fileList, std::string filename, std::string destination) {
	bool flag;
	std::vector<std::string> checkfiles;
	std::cout << " \n File to be checked out : " << filename <<"\n";
	std::cout << "\n Destination of files to be copied to : " << destination << std::endl;
	std::cout << "\n List of files to be checked out - dependent files : \n" << std::endl;
	for (size_t i = 0; i < fileList.size(); ++i) {
		std::cout << fileList[i] << "\n";
	}
	for (size_t i = 0; i < fileList.size(); ++i) {
		DbElement<P> dbelem = repo_dbcore[fileList[i]];
		std::string source = dbelem.payLoad().value();
		std::string destinationpath = destination;
		destinationpath.append("/").append(CodeRepo<P>::extractFile(CodeRepo<P>::extractFilename(source)));
		flag = FileSystem::File::copy(source, destinationpath, false);
		
		checkfiles.push_back(CodeRepo<P>::extractFile(fileList[i]));

	}

	return checkfiles;
}


#pragma once
//////////////////////////////////////////////////////////////////////////
// Version.h - Retrieves the latest version of the file					//
// ver 1.0														   		//
// Chetali Mahore, CSE687 - Object Oriented Design, Spring 2018			//
/////////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides Version class:
* - This class retrieves the latest version of a specified file
*
* Maintenance History:
* --------------------
* ver 1.0 - 1st March 2018
* - Function for extracting the current version of a specified file
*/



#include<vector>
#include<iostream>
#include<string>
#include <sstream>

class Version {
public:
	static int extractVersion(std::vector<std::string> regkeys);
};

/*-------------extracting the current version of a specified file---------------------------*/

inline int Version::extractVersion(std::vector<std::string> regkeys) {

	size_t max = 0;
	// code to return latest version
	for (auto itr : regkeys) {
		size_t indexofdot = itr.find_last_of(".");
		std::string version = itr.substr(indexofdot + 1, itr.size());
		std::stringstream stream(version);
		size_t version_int = 0;
		stream >> version_int;
		if (max < version_int) {
			max = version_int;
		}
	}
	return max;
}

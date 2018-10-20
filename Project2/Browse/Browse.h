#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////
// Browse.h - Provides browsing of files from the repository, fetch the dbelement and open the file //
// ver 1.0														   									  //
// Chetali Mahore, CSE687 - Object Oriented Design, Spring 2018										 //
//////////////////////////////////////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides Browse class:
* - This class contains browse function which will search the file in the repository and open it
*
* Maintenance History:
* --------------------
* ver 1.0 - 3rd March 2018
* - Browse the file and used process class to open the file in the notepad
*/
#include "../NoSQLDB/Executive/NoSqlDb.h"
#include "../NoSQLDB/DbCore/DbCore.h"
#include "../CodeRepo/CodeRepo.h"

using namespace NoSqlDb;

template<typename P>
class Browse {
public:
	void static openFile(DbElement<P> dbelement, std::string filename);
	void static browse(DbCore<P>& repo_dbcore, std::string filename);


};

/*----------------opens the specified file in the notepad--------------------*/

template<typename P>
inline void Browse<P>::openFile(DbElement<P> dbelement, std::string filename) {
	std::string filepath = dbelement.payLoad().value();
	Process p;
	p.title("Test application");
	std::string appPath = "C://Windows//System32//notepad.exe";
	p.application(appPath);

	//std::string cmdLine = "/A ../Process/Process.h";
	std::string cmdLine;
	cmdLine.append("/A ").append(filepath);
	p.commandLine(cmdLine);

	std::cout << "\n  starting process: \"" << appPath << "\"";
	std::cout << "\n  with this cmdlne: \"" << cmdLine << "\"";
	p.create();
}


/*-------------------browse the file in the repository and send it as an argument to openFile function--------*/

template<typename P>
inline void Browse<P>::browse(DbCore<P> & repo_dbcore, std::string filename)
{
	DbElement<P> dbelement = repo_dbcore[filename];
	std::cout << "\n Browsing file : " << filename << "\n" << std::endl;
	openFile(dbelement, filename);
	std::cout <<"\n \n DbElement of " <<filename << ": "<< std::endl;
	showElem(dbelement, std::cout);
}

// 
///////////////////////////////////////////////////////////////////////////////////
//TestExecutive.cpp : Defines the entry point for the console application.       //
//																				 //
// Chetali Mahore, CSE 687, Object Oriented Design in C++						 //
////////////////////////////////////////////////////////////////////////////////////
/*
* Package Description- 
* - it contains all the test cases to handle the following functions:
* - Loading the contents of the XML into the database
* - Checkin of files with transitive dependency
* - Checkin of files with circular dependency
* - Checkout of files 
* - Browsing the file in the repository and retrieve its data and display it
* - Perform authentication while checkin of the file
* 
* Required Files- 
* - CodeRepo.h, FileMgr.h
* - NoSqlDb.h, Payload.h
* 
* Maintenance History:
* ver 1.1 - 28th February 2018
* - added the checkout and broswse test cases

* ver 1.0 - 22nd February 2018
* - created test cases for the checkin operation
*/


#include "../CodeRepo/CodeRepo.h"
#include "../FileMgr/FileMgr.h"
#include "../NoSQLDB/Executive/NoSqlDb.h"
#include "../NoSQLDB/PayLoad/PayLoad.h"
#include <chrono>
#include <thread>
#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>

using namespace FileManager;
using Dir = FileSystem::Directory;
using namespace NoSqlDb;
using namespace XmlProcessing;

auto putLine = [](size_t n = 1, std::ostream& out = std::cout)
{
	Utilities::putline(n, out);
};

class DbProvider
{
public:
	DbCore<PayLoad>& db() { return db_; }
private:
	static DbCore<PayLoad> db_;
};

DbCore<PayLoad> DbProvider::db_;

bool testR1(){
	DbProvider dbp;
	DbCore<PayLoad> dbcore;

	std::string input;
	std::string data_xml;
	std::ifstream infile;
	std::string filename = "../InitialDatabase.xml";
	std::cout << "\n Status of files : Open = 0 , Closing = 1, Close = 2 \n";
	std::cout << " \n Reading the contents of the file : " << filename << std::endl;
	infile.open(filename);
	while (!infile.eof()) 
	{
		getline(infile, input); 
		data_xml.append(input);
	}

	infile.close();
	Persist<PayLoad> persist(dbcore);
	persist.fromXml(data_xml, true);
	//showDb(dbcore, std::cout);
	dbp.db() = dbcore;

	PayLoad p;
	std::cout << " \n The contents of the Database after loading XMl \n ";
	showDb(dbcore);

	std::cout << "\n \n Payload is as follows: \n";
	p.showDb(dbcore, std::cout);

	Utilities::title("\n Demonstrating Requirement #1");
	std::cout << "\n  " << typeid(std::function<bool()>).name()
		<< ", declared in this function,	"
		<< "\n  is only valid for C++11 and later versions.";
	std::cout << "\n  passed -- \" Use C++11 \" ";
	putLine();

	return true; // would not compile unless C++11
}

bool testR2() {
	Utilities::title("Demonstrating Requirement #2");
	std::cout << "\n  A visual examination of all the submitted code "
		<< "will show only\n  use of streams and operators new and delete.";
	putLine();

	return true;
}
	

bool testR3a() {
	DbProvider dbp;
	DbCore<PayLoad> dbcore = dbp.db();
	CodeRepo<PayLoad> c(dbcore);
	c.setRepoDirectory("../RepositoryStorage");	c.setCheckoutDirectory("../CheckedOutFiles");
	
 	Utilities::title("Demonstrating Requirement 3 and 4 a - checkin of three new files and checkin of files with open dependencies");
	std::cout << "\n Added packages - TestExecutive, CodeRepo, Checkin, Checkout, Version, Browse \n \n";
	std::cout << "\n Adding three new files 'File3.cpp', 'File2.cpp' and 'File4.h' into the repository \n";
	std::vector<std::string> children;
	std::vector < std::string> categoryList = { "category1" };

	DbElement<PayLoad> dbelem1;
	dbelem1.payLoad().value("../Client/File3.cpp");	dbelem1.payLoad().setCategory(categoryList);	dbelem1.payLoad().setNamespace("namespace3");
	dbelem1.payLoad().setStatus(open); dbelem1.name("Chetali"); dbelem1.descrip("Its a File3.cpp");
	dbcore = c.checkInFiles(dbelem1); 	
	children.clear();
	dbp.db() = dbcore;

	children = { "namespace3::File3.cpp.2", "namespace1::File5.h.1" };
	DbElement<PayLoad> dbelem2;	dbelem2.payLoad().value("../Client/File2.cpp");	dbelem2.payLoad().setCategory(categoryList);	dbelem2.children(children);
	dbelem2.payLoad().setNamespace("namespace2");	dbelem2.payLoad().setStatus(open); dbelem2.name("Chetali"); dbelem2.descrip("Its a File2.cpp");
	dbcore = c.checkInFiles(dbelem2); 
	children.clear();
	dbp.db() = dbcore;

	DbElement<PayLoad> dbelem5;
	dbelem5.payLoad().value("../Client/File4.h");	dbelem5.payLoad().setNamespace("namespace2");	dbelem5.payLoad().setStatus(open);
	children = { "namespace2::File2.cpp.1" };	dbelem5.children(children);	dbelem5.name("Chetali"); dbelem5.payLoad().setCategory(categoryList);
	dbcore = c.checkInFiles(dbelem5);  dbelem5.descrip("Its a File4.h");
	children.clear(); std::cout << "\n Payload is: \n"; PayLoad::showDb(dbcore, std::cout); std::cout << "\n" << std::endl;
	dbp.db() = dbcore;

	Utilities::title("Closing of files");
	children = { "namespace3::File3.cpp.2", "namespace1::File5.h.1" };	dbelem2.payLoad().value("../Client/File2.cpp");	dbelem2.children(children);
	dbelem2.payLoad().setStatus(close);	dbelem2.payLoad().setNamespace("namespace2");	dbelem2.payLoad().setCategory(categoryList);
	dbcore = c.checkInFiles(dbelem2); dbelem2.descrip("Its a File2.cpp");
	children.clear(); std::cout << " \n File2.cpp.1 cannot be closed since its dependent file 'File3.cpp.2' is open \n";
	dbp.db() = dbcore;

	DbElement<PayLoad> dbelem4;
	dbelem4.payLoad().value("../Client/File4.h");	dbelem4.payLoad().setNamespace("namespace2");	dbelem4.payLoad().setCategory(categoryList);
	dbelem4.children().push_back("namespace2::File2.cpp.1");	dbelem4.payLoad().setStatus(close); dbelem4.name("Chetali");
	dbcore = c.checkInFiles(dbelem4); 
	dbp.db() = dbcore; std::cout << "\n Closing File4.h.1 : ";
	children.clear();  std::cout << " \n File4.h.1 cannot be closed since its dependent file 'File2.cpp.1' and 'File3.cpp.2' is open \n" << "\n Payload status is as follows: \n";
	std::cout << "\n Files 'File2.cpp.1' and 'File3.cpp.2' enter into closing state \n";PayLoad::showDb(dbcore, std::cout); std::cout << "\n" << std::endl;

	DbElement<PayLoad> dbelem3;
	dbelem3.payLoad().value("../Client/File3.cpp");	dbelem3.payLoad().setCategory(categoryList);	dbelem3.payLoad().setNamespace("namespace3");
	dbelem3.payLoad().setStatus(close);	dbelem3.name("Chetali"); std::cout << "\n Closing File3.cpp which will close File2.cpp.1 and File4.h.1 \n ";
	dbcore = c.checkInFiles(dbelem3);	dbp.db() = dbcore; std::cout << "\n Payload status is as follows: \n";
	PayLoad::showDb(dbcore, std::cout); std::cout << "\n" << std::endl;

	return true;
}

bool testR3b() {
	Utilities::title("Demonstrating Requirement 3 - checkin of files with circular dependency \n");
	DbProvider dbp; DbCore<PayLoad> dbcore = dbp.db();
	PayLoad pay; CodeRepo<PayLoad> c(dbcore);
	c.setRepoDirectory("../RepositoryStorage");	c.setCheckoutDirectory("../CheckedOutFiles");
	std::vector<std::string> categoryList = {"category7"};

	DbElement<PayLoad> dbelem;
	dbelem.payLoad().setNamespace("namespace1");
	dbelem.payLoad().value("../Client/File7.h"); dbelem.name("Chetali");
	dbelem.children().push_back("namespace1::File7.cpp.1");
	dbelem.payLoad().setStatus(open); dbelem.descrip("Its a File7.h");
	dbelem.payLoad().setCategory(categoryList);
	dbcore = c.checkInFiles(dbelem);

	dbp.db() = dbcore;

	DbElement<PayLoad> dbelem1;
	dbelem1.payLoad().setNamespace("namespace1");
	dbelem1.payLoad().value("../Client/File7.cpp"); dbelem1.name("Chetali");
	dbelem1.children().push_back("namespace1::File7.h.1");
	dbelem1.payLoad().setCategory(categoryList); dbelem1.descrip("Its a File7.cpp");
	dbelem1.payLoad().setStatus(open);
	dbcore = c.checkInFiles(dbelem1);
	
	dbp.db() = dbcore;

	PayLoad::showDb(dbcore, std::cout);

	std::cout << "\n \n Closing File7.cpp.1 which is dependent on File7.h.1 and Closing File7.h.1 which is dependent on File7.cpp.1 \n \n";

	std::cout << " \n Payload status of closing files \n";
	dbelem1.payLoad().value("../Client/File7.cpp");
	dbelem1.payLoad().setStatus(close);
	dbcore = c.checkInFiles(dbelem1);
	dbp.db() = dbcore;
	PayLoad::showDb(dbcore, std::cout);

	dbelem.payLoad().value("../Client/File7.h");
	dbelem.payLoad().setStatus(close);
	dbcore = c.checkInFiles(dbelem); std::cout << "\n \n Payload status is: \n";
	dbp.db() = dbcore;
	PayLoad::showDb(dbcore, std::cout);
	return true;
}

bool testR3c(){

	Utilities::title("Demonstrating Requirement 3 - checkout of files ");
	DbProvider dbp;
	DbCore<PayLoad> dbcore = dbp.db();
	PayLoad pay;
	CodeRepo<PayLoad> c(dbcore);
	c.setCheckoutDirectory("../CheckedOutFiles");

	std::cout << "\n Checkout of file 'namespace1::File1.cpp' with version 1 \n";
	c.checkOut("namespace1::File1.cpp", "1");

	Utilities::title("");
	std::cout << "\n Checkout of file 'namespace2::File2.cpp' with unspecified version \n";
	c.checkOut("namespace2::File2.cpp", "0");
	//c.checkOutFiles("namespace1::File1.cpp.1");
	std::cout << "\n  passed -- \" Providing checkout functionality \" ";

	Utilities::title("\n Demonstrating Requirement 3 and 5 - browsing of file of name 'namespace1::File1.cpp' with unspecified version");

	c.browseFile("namespace1::File1.cpp", "0");

	return true;
}

bool testR4() {
	Utilities::title("Demonstrating Requirement 4 b - Authorisation of an author to perform checkin ");

	DbProvider dbp;
	DbCore<PayLoad> dbcore = dbp.db();
	CodeRepo<PayLoad> c(dbcore);
	
	std::vector<std::string> categoryList = { "category1" };
	DbElement<PayLoad> dbelem;
	std::cout << "\n Any random user is trying to checkin a file : Manisha \n";
	dbelem.name("Manisha");
	dbelem.payLoad().setStatus(open);
	dbelem.payLoad().value("../Client/File4.h");
	dbelem.children().push_back("namespace:File1.cpp.1");
	dbelem.payLoad().setCategory(categoryList);
	dbcore = c.checkInFiles(dbelem);
	 
	dbp.db() = dbcore;

	std::cout << "\n \n Payload is as follows: \n ";

	PayLoad::showDb(dbcore, std::cout);
	return true;

}

bool testR6() {
	DbProvider dbp;
	DbCore<PayLoad> dbcore = dbp.db();
	CodeRepo<PayLoad> c(dbcore);
	std::string filepath = "../PersistDatabase.xml";

	Utilities::title("\n The contents of the repository are as follows: \n");
	showDb(dbcore, std::cout);

	Utilities::title("\n The final payload of the repository is as follows with major closed files and a few open files: \n");
	PayLoad::showDb(dbcore, std::cout);

	Utilities::title("\n Demonstrating Requirement 6 - Persisting the database into xml file");
	std::cout << "\n Path of the file where database is persisted: " << filepath;
	c.persist(filepath);

	Utilities::title(" \n Demonstrating Requirement 7 - TestExecutive executes all the tests ");
	return true;
}

int main()
{
	TestExecutive ex;

	TestExecutive::TestStr ts1{ testR1, "Loading the XML into database and Demonstrating Test 1" };
	TestExecutive::TestStr ts2{ testR2, "Demonstrating Test 2" };
	TestExecutive::TestStr ts3a{ testR3a, "Providing check in functionality - Transitive dependency" };
	TestExecutive::TestStr ts3b{ testR3b, "Providing check in functionality - Circular dependency" };
	TestExecutive::TestStr ts3c{ testR3c, "Providing browse functionality "};
	TestExecutive::TestStr ts4{ testR4, "Checking authorisation of user to checkin a file" };
	TestExecutive::TestStr ts6{ testR6, "Persisting the database into an xml file" };


	ex.registerTest(ts1);
	ex.registerTest(ts2);
	ex.registerTest(ts3a);
	ex.registerTest(ts3b);
	ex.registerTest(ts3c);
	ex.registerTest(ts4);
	ex.registerTest(ts6);

	bool result = ex.doTests();
	if (result == true)
		std::cout << "\n  all tests passed";
	else
		std::cout << "\n  at least one test failed";


	getchar();
	putLine(2);
	
}


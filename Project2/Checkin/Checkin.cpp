#include "Checkin.h"

///////////////////////////////////////////////////////////////////////
// Checkin.cpp - Test Stub - Provides checkin of files into the repository storage  //
// ver 1.0														   	  //
// Chetali Mahore, CSE687 - Object Oriented Design, Spring 2018		  //
//////////////////////////////////////////////////////////////////////



#ifdef TEST_CHECKIN
int main()
{
	DbCore<PayLoad> dbcore;
	CodeRepo<PayLoad> c(dbcore);
	c.setRepoDirectory("../RepositoryStorage");	c.setCheckoutDirectory("../CheckedOutFiles");

	Utilities::title("Demonstrating Requirement 3 a and 4 a - checkin of three new files and checkin of files with open dependencies");
	std::cout << "\n Adding three new files 'File3.cpp', 'File2.cpp' and 'File4.h' into the repository \n";
	std::vector<std::string> children;
	std::vector < std::string> categoryList = { "category1" };

	DbElement<PayLoad> dbelem1;
	dbelem1.payLoad().value("../Client/File3.cpp");	dbelem1.payLoad().setCategory(categoryList);	dbelem1.payLoad().setNamespace("namespace3");
	dbelem1.payLoad().setStatus(open); dbelem1.name("Chetali");
	dbcore = c.checkInFiles(dbelem1);
	children.clear();

	children = { "namespace3::File3.cpp.1"};
	DbElement<PayLoad> dbelem2;	dbelem2.payLoad().value("../Client/File2.cpp");	dbelem2.payLoad().setCategory(categoryList);	dbelem2.children(children);
	dbelem2.payLoad().setNamespace("namespace2");	dbelem2.payLoad().setStatus(open); dbelem2.name("Chetali");
	dbcore = c.checkInFiles(dbelem2);
	children.clear();

	DbElement<PayLoad> dbelem5;
	dbelem5.payLoad().value("../Client/File4.h");	dbelem5.payLoad().setNamespace("namespace2");	dbelem5.payLoad().setStatus(open);
	children = { "namespace2::File2.cpp.1" };	dbelem5.children(children);	dbelem5.name("Chetali"); dbelem5.payLoad().setCategory(categoryList);
	dbcore = c.checkInFiles(dbelem5);
	children.clear();  PayLoad::showDb(dbcore, std::cout); std::cout << "\n" << std::endl;


	Utilities::title("Closing of files");
	children = { "namespace3::File3.cpp.1"};	dbelem2.payLoad().value("../Client/File2.cpp");	dbelem2.children(children);
	dbelem2.payLoad().setStatus(close);	dbelem2.payLoad().setNamespace("namespace2");	dbelem2.payLoad().setCategory(categoryList);
	dbcore = c.checkInFiles(dbelem2);
	children.clear(); std::cout << " \n File2.cpp.1 cannot be closed since its dependent file 'File3.cpp.1' is open \n";

	DbElement<PayLoad> dbelem4;
	dbelem4.payLoad().value("../Client/File4.h");	dbelem4.payLoad().setNamespace("namespace2");	dbelem4.payLoad().setCategory(categoryList);
	dbelem4.children().push_back("namespace2::File2.cpp.1");	dbelem4.payLoad().setStatus(close); dbelem4.name("Chetali");
	dbcore = c.checkInFiles(dbelem4);
	children.clear();  std::cout << " \n File4.h.1 cannot be closed since its dependent file 'File2.cpp.1' and 'File3.cpp.1' is open \n" << "\n Payload status is as follows: \n";
	PayLoad::showDb(dbcore, std::cout); std::cout << "\n" << std::endl;

	DbElement<PayLoad> dbelem3;
	dbelem3.payLoad().value("../Client/File3.cpp");	dbelem3.payLoad().setCategory(categoryList);	dbelem3.payLoad().setNamespace("namespace3");
	dbelem3.payLoad().setStatus(close);	dbelem3.name("Chetali"); std::cout << "\n Closing File3.cpp which will close File2.cpp.1 and File4.h.1 \n ";
	dbcore = c.checkInFiles(dbelem3); std::cout << "\n Payload status is as follows: \n";
	PayLoad::showDb(dbcore, std::cout); std::cout << "\n" << std::endl;
	getchar();
	return 0;
}
#endif // TEST_CHECKIN

#include<string>
#include<vector>
#include "CodeRepo.h"

////////////////////////////////////////////////////////////////////////////////////////
// CodeRepo.cpp - Common entry point for performing checkin, checkout, browse operations  //
// ver 1.0																				//
// Chetali Mahore, CSE687 - Object Oriented Design, Spring 2018							//
/////////////////////////////////////////////////////////////////////////////////////////


#ifdef TEST_CODEREPO

int main()
{
	Utilities::title("Demonstrating checkin operation");
	DbCore<PayLoad> dbcore;
	PayLoad pay; CodeRepo<PayLoad> c(dbcore);
	c.setRepoDirectory("../RepositoryStorage");	c.setCheckoutDirectory("../CheckedOutFiles");
	std::vector<std::string> categoryList = { "category7" };
	DbElement<PayLoad> dbelem;	dbelem.payLoad().setNamespace("namespace1");	dbelem.payLoad().value("../Client/File7.h"); dbelem.name("Chetali");
	dbelem.children().push_back("namespace1::File7.cpp.1");	dbelem.payLoad().setStatus(open);	dbelem.payLoad().setCategory(categoryList);
	dbcore = c.checkInFiles(dbelem);

	DbElement<PayLoad> dbelem1;	dbelem1.payLoad().setNamespace("namespace1");	dbelem1.payLoad().value("../Client/File7.cpp"); dbelem1.name("Chetali");
	dbelem1.children().push_back("namespace1::File7.h.1");	dbelem1.payLoad().setCategory(categoryList);	dbelem1.payLoad().setStatus(open);
	dbcore = c.checkInFiles(dbelem1);

	PayLoad::showDb(dbcore, std::cout);
	std::cout << "\n \n Closing File7.cpp.1 which is dependent on File7.h.1 and Closing File7.h.1 which is dependent on File7.cpp.1 \n \n";

	dbelem1.payLoad().value("../Client/File7.cpp");
	dbelem1.payLoad().setStatus(close);	dbcore = c.checkInFiles(dbelem1);	PayLoad::showDb(dbcore, std::cout);

	dbelem.payLoad().value("../Client/File7.h");	dbelem.payLoad().setStatus(close);	dbcore = c.checkInFiles(dbelem); std::cout << "\n Payload status is: \n";
	PayLoad::showDb(dbcore, std::cout);	Utilities::title("Demonstrating checkout operation");
	std::vector<std::string> fileList = { "namespace2::File2.cpp.1", "namespace2::File3.cpp.1", "namespace4::File4.h.1" };
	std::string filename = "namespace1::File1.cpp.1";
	std::string destination = "../CheckedOutFiles";	Checkout::checkout(dbcore, fileList, filename, destination);

	Utilities::title("Demonstrating extracting version operation");
	std::cout << "\n Following is the list of files \n ";
	std::cout << "\n namespace1:File1.cpp.1 \n ";
	std::cout << "\n namespace1::File1.cpp.2 \n";
	std::cout << "\n namespace1::File1.cpp.3 \n";
	std::vector<std::string> regkeys = { "namespace1:File1.cpp.1", "namespace1::File1.cpp.2", "namespace1::File1.cpp.3" };
	std::cout << "\n The latest version is :" << Version::extractVersion(regkeys);

	filename = "namespace1::File1.cpp.1";
	DbElement<PayLoad> dbelem2;	dbelem2.name("Chetali");	dbelem2.payLoad().setStatus(close);	dbelem2.payLoad().setNamespace("namespace1");
	dbelem2.children().push_back("namespace7::File9.h.1");	dbcore[filename] = dbelem2;	Utilities::title("Demonstrating browsing operation");
	Browse<PayLoad>::browse(dbcore, filename);	getchar();
}
#endif // TEST_CODEREPO

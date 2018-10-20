#include "Browse.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////
// Browse.cpp - test stub - Provides browsing of files from the repository, fetch the dbelement and open the file //
// ver 1.0														   									  //
// Chetali Mahore, CSE687 - Object Oriented Design, Spring 2018										 //
//////////////////////////////////////////////////////////////////////////////////////////////////////


#ifdef TEST_BROWSE
int main() 
{
	DbCore<PayLoad> dbcore;
	std::string filename = "namespace1::File1.cpp.1";
	DbElement<PayLoad> dbelem;
	dbelem.name("Chetali");
	dbelem.payLoad().setStatus(close);
	dbelem.payLoad().setNamespace("namespace1");
	dbelem.children().push_back("namespace7::File9.h.1");
	dbcore[filename] = dbelem;
	Browse<PayLoad>::browse(dbcore, filename);

	getchar();
	return 0;
}
#endif // TEST_BROWSE

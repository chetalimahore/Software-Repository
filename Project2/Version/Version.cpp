#include "Version.h"

//////////////////////////////////////////////////////////////////////////
// Version.cpp - Test Stub - Retrieves the latest version of the file					//
// ver 1.0														   		//
// Chetali Mahore, CSE687 - Object Oriented Design, Spring 2018			//
/////////////////////////////////////////////////////////////////////////



#ifdef TEST_VERSION
int main()
{
	std::cout << "\n Following is the list of files \n ";
	std::cout << "\n namespace1:File1.cpp.1 \n ";
	std::cout << "\n namespace1::File1.cpp.2 \n";
	std::cout << "\n namespace1::File1.cpp.3 \n";
	std::vector<std::string> regkeys = { "namespace1:File1.cpp.1", "namespace1::File1.cpp.2", "namespace1::File1.cpp.3" };
	std::cout<< "\n The latest version is :"<< Version::extractVersion(regkeys);

	getchar();
	return 0;

}
#endif // TEST_TESTVERSION

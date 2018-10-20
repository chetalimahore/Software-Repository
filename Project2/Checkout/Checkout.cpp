#include "Checkout.h"

//////////////////////////////////////////////////////////////////////////////
// Checkout.cpp - Test Stub - Provides checkout of files into the checkedOutFiles storage  //
// ver 1.0														   			 //
// Chetali Mahore, CSE687 - Object Oriented Design, Spring 2018				 //
//////////////////////////////////////////////////////////////////////////////



#ifdef TEST_CHECKOUT
int main() 
{
	
	DbCore<PayLoad> dbcore;
	std::vector<std::string> fileList = {"namespace2::File2.cpp.1", "namespace2::File3.cpp.1", "namespace4::File4.h.1"};
	std::string filename = "namespace1::File1.cpp.1";
	std::string destination = "../CheckedOutFiles";

	Checkout::checkout(dbcore, fileList, filename, destination);
	//Checkout::checkout(dbcore, fileList, filename, destination);
	getchar();

}
#endif // TEST_CHECKOUT

#ifndef PAYLOAD_H
#define PAYLOAD_H
///////////////////////////////////////////////////////////////////////////////
// PayLoad.h - application defined payload									  //
// ver 1.1	
// Author - Chetali Mahore
// Source - Jim Fawcett, CSE687 - Object Oriented Design, Spring 2018         //
///////////////////////////////////////////////////////////////////////////////
/*
*  Package Operations:
*  -------------------
*  This package provides a single class, PayLoad:
*  - holds a payload string and vector of categories
*  - provides means to set and access those values
*  - provides methods used by Persist<PayLoad>:
*    - Sptr toXmlElement();
*    - static PayLoad fromXmlElement(Sptr elem);
*  - provides a show function to display PayLoad specific information
*  - PayLoad processing is very simple, so this package contains only
*    a header file, making it easy to use in other packages, e.g.,
*    just include the PayLoad.h header.
*
*  Required Files:
*  ---------------
*    PayLoad.h, PayLoad.cpp - application defined package
*    DbCore.h, DbCore.cpp
*
*  Maintenance History:
*  --------------------
*  ver 1.2 : 25th February 2018
*  - added fields like namespace, status of the files
*  ver 1.1 : 19 Feb 2018
*  - added inheritance from IPayLoad interface
*  Ver 1.0 : 10 Feb 2018
*  - first release
*
*/

#include <string>
#include <vector>
#include <iostream>
/*
#include "../XmlDocument/XmlDocument/XmlDocument.h"
#include "../XmlDocument/XmlElement/XmlElement.h"
#include "../DbCore/Definitions.h"
#include "../DbCore/DbCore.h"
*/

//#include "../../NoSQLDB/XmlDocument/XmlDocument/XmlDocument.h"
//#include "../../NoSQLDB/XmlDocument/XmlElement/XmlElement.h"
//#include "../../NoSQLDB/DbCore/Definitions.h"
//#include "../../NoSQLDB/DbCore/DbCore.h"

#include "../../NoSQLDB/Executive/NoSqlDb.h"
#include "IPayLoad.h"

///////////////////////////////////////////////////////////////////////
// PayLoad class provides:
// - a std::string value which, in Project #2, will hold a file path
// - a vector of string categories, which for Project #2, will be 
//   Repository categories
// - methods used by Persist<PayLoad>:
//   - Sptr toXmlElement();
//   - static PayLoad fromXmlElement(Sptr elem);


namespace NoSqlDb
{
  enum status {open, closing, close};

  class PayLoad : public IPayLoad<PayLoad>
  {
  public:
    PayLoad() = default;
    PayLoad(const std::string& val) : value_(val) {}
    static void identify(std::ostream& out = std::cout);
    PayLoad& operator=(const std::string& val)
    {
      value_ = val;
      return *this;
    }
    operator std::string() { return value_; }

    std::string value() const { return value_; }
    std::string& value() { return value_; }
    void value(const std::string& value) { value_ = value; }

	std::string getNamespace() const { return namespace_; }
	std::string& getNamespace() { return namespace_; }
	void setNamespace(const std::string& name) { namespace_ = name; }

	status getStatus() const { return file_status; }
	status& getStatus() { return file_status; }
	void setStatus(status status_) { file_status = status_; }

    std::vector<std::string>& categories() { return categories_; }
    std::vector<std::string> categories() const { return categories_; }
	void setCategory(std::vector<std::string> categoryList) { categories_ = categoryList; }

    bool hasCategory(const std::string& cat)
    {
      return std::find(categories().begin(), categories().end(), cat) != categories().end();
    }

    Sptr toXmlElement();
    static PayLoad fromXmlElement(Sptr elem);

    static void showPayLoadHeaders(std::ostream& out = std::cout);
    static void showElementPayLoad(NoSqlDb::DbElement<PayLoad>& elem, std::ostream& out = std::cout);
    static void showDb(NoSqlDb::DbCore<PayLoad>& db, std::ostream& out = std::cout);

  private:
    std::string value_;
    std::vector<std::string> categories_;
	std::string namespace_;
	status file_status;

  };

  //----< show file name >---------------------------------------------

  inline void PayLoad::identify(std::ostream& out)
  {
    out << "\n  \"" << __FILE__ << "\"";
  }
  //----< create XmlElement that represents PayLoad instance >---------
  /*
  * - Required by Persist<PayLoad>
  */
  inline Sptr PayLoad::toXmlElement()
  {
    Sptr sPtr = XmlProcessing::makeTaggedElement("payload");
    XmlProcessing::XmlDocument doc(makeDocElement(sPtr));
    Sptr sPtrVal = XmlProcessing::makeTaggedElement("value",value_);
    sPtr->addChild(sPtrVal);
    Sptr sPtrCats = XmlProcessing::makeTaggedElement("categories");
    sPtr->addChild(sPtrCats);
    for (auto cat : categories_)
    {
      Sptr sPtrCat = XmlProcessing::makeTaggedElement("category", cat);
      sPtrCats->addChild(sPtrCat);
    }
	Sptr sPtrNamespace = XmlProcessing::makeTaggedElement("namespace", namespace_);
	sPtr->addChild(sPtrNamespace);
	Sptr sPtrStatus = XmlProcessing::makeTaggedElement("status", std::to_string(file_status));
	sPtr->addChild(sPtrStatus);
    return sPtr;
  }
  //----< create PayLoad instance from XmlElement >--------------------
  /*
  * - Required by Persist<PayLoad>
  */
  inline PayLoad PayLoad::fromXmlElement(Sptr pElem)
  {
    PayLoad pl;
    for (auto pChild : pElem->children())
    {
      std::string tag = pChild->tag();
      std::string val = pChild->children()[0]->value();
      if (tag == "value")
      {
        pl.value(val);
      }
      if (tag == "categories")
      {
		  pl.categories().push_back(val);
      }
	  if (tag == "namespace") {
		  pl.setNamespace(val);
	  }
	  if (tag == "status") {
		  status state;
		  if (val == "Open") {
			  state = open;
		  }
		  else if (val == "Close") {
			  state = close;
		  }
		  pl.setStatus(state);
	  }

    }
    return pl;
  }
  /////////////////////////////////////////////////////////////////////
  // PayLoad display functions

  inline void PayLoad::showPayLoadHeaders(std::ostream& out)
  {
    out << "\n  ";
    out << std::setw(10) << std::left << "Name";
    out << std::setw(60) << std::left << "Payload Value";
	out << std::setw(20) << std::left << "Namespace";
	out << std::setw(20) << std::left << "Status";
    out << std::setw(20) << std::left << "Categories";
	
    out << "\n  ";
    out << std::setw(10) << std::left << "--------";
    out << std::setw(60) << std::left << "-----------------------";
    out << std::setw(20) << std::left << "-----------";
	out << std::setw(20) << std::left << "-----------";
	out << std::setw(20) << std::left << "-----------";
  }


  inline void PayLoad::showElementPayLoad(NoSqlDb::DbElement<PayLoad>& elem, std::ostream& out)
  {
    out << "\n  ";
    out << std::setw(10) << std::left << elem.name().substr(0, 8);
    out << std::setw(60) << std::left << elem.payLoad().value();
	out << std::setw(20) << std::left << elem.payLoad().getNamespace();
	out << std::setw(20) << std::left << elem.payLoad().getStatus();
//	out << std::setw(50) << std::left << elem.payLoad().value();
	//out << std::setw(20) << std::left << elem.payLoad().categories_[0];
    for (auto cat : elem.payLoad().categories())
    {
      out << cat << " ";
    }

  }

  inline void PayLoad::showDb(NoSqlDb::DbCore<PayLoad>& db, std::ostream& out)
  {
    showPayLoadHeaders(out);
    for (auto item : db)
    {
      NoSqlDb::DbElement<PayLoad> temp = item.second;
      PayLoad::showElementPayLoad(temp, out);
    }
  }
}
#endif

/////////////////////////////////////////////////////////////////////////
// ServerPrototype.cpp - Console App that processes incoming messages  //
// ver 1.0															   //
// Author: Chetali Mahore											   //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2018           //
/////////////////////////////////////////////////////////////////////////

#include "ServerPrototype.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include <chrono>
#include "../Project2/RepoExec/RepoExec.h"
#include "../Project2/CodeRepo/CodeRepo.h"
#include "../Project2/FileMgr/FileMgr.h"
#include "../Project2/NoSQLDB/PayLoad/PayLoad.h"
#include <sstream>

namespace MsgPassComm = MsgPassingCommunication;

using namespace Repository;
using namespace FileSystem;
using Msg = MsgPassingCommunication::Message;
using namespace NoSqlDb;
using namespace XmlProcessing;

class DbProvider
{
public:
	DbCore<PayLoad>& db() { return db_; }
private:
	static DbCore<PayLoad> db_;
};

DbCore<PayLoad> DbProvider::db_;


Files Server::getFiles(const Repository::SearchPath& path)
{
	return Directory::getFiles(path);
}

Dirs Server::getDirs(const Repository::SearchPath& path)
{
	return Directory::getDirectories(path);
}

template<typename T>
void show(const T& t, const std::string& msg)
{
	std::cout << "\n  " << msg.c_str();
	for (auto item : t)
	{
		std::cout << "\n    " << item.c_str();
	}
}

//-----------< echoes the message back to the client >------------

std::function<Msg(Msg)> echo = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	return reply;

};

//------------< transfers the file to the client upon request >-------------

std::function<Msg(Msg)> openFile = [](Msg msg) {
	std::vector<std::string> checkfiles;
	std::string check, version;

	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command(msg.command());
	//reply.attribute("sender", "server");

	std::string namespaces = msg.value("namespaces");
	std::string filenames = msg.value("filename");

	const size_t last_slash_idx = filenames.find_last_of("\\.");
	if (std::string::npos != last_slash_idx) {
		version = filenames.substr(last_slash_idx+1, filenames.length());
		filenames.erase(last_slash_idx, filenames.length());
	}

	std::string path = namespaces + "/" + filenames;			// Test1/filename.cpp
	reply.attribute("filename", filenames);
	//reply.attribute("file", path);
	reply.attribute("content", "File Sending in progress from Server to Client :" + filenames);
	reply.attribute("call_function", msg.value("call_function"));
	reply.attribute("file_name", namespaces + "::" + filenames + "." + version);

	if (msg.value("call_function") == "checkout") {
		DbProvider dbp;
		DbCore<PayLoad> dbcore = dbp.db();
		CodeRepo<PayLoad> c(dbcore);
		c.setCheckoutDirectory("../Storage");
		checkfiles = c.checkOut(namespaces+"::"+filenames, version);
	}
	else if (msg.value("call_function") == "browse") {
		DbProvider dbp;
		DbCore<PayLoad> dbcore = dbp.db();
		CodeRepo<PayLoad> c(dbcore);
		c.setCheckoutDirectory("../Storage");
		checkfiles = c.browseFile(namespaces + "::" + filenames, version);
	}
	
	for (size_t i = 0; i < checkfiles.size(); i++) {
		check.append(checkfiles[i] + " ");
	}
	
	std::string check1 = check.substr(0, check.length() - 1);

	reply.attribute("dep_files", check1);

	return reply;
};

//------------< handler for checkin of file >-----------------

std::function<Msg(Msg)> checkinProgress = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("checkinProgress");
	std::string content = "File " + msg.value("filename") + " checkin to location: ../Storage in progress";
	reply.attribute("content", content);
	reply.remove("file");

	return reply;
};


//--------------< checks in the file into the server repository >--------------

std::function<Msg(Msg)> checkinFile = [](Msg msg) {

	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("checkin");
	DbProvider dbp; DbCore<PayLoad> dbcore = dbp.db();
	std::string content = "File " + msg.value("filename") + " checkin to location: ../Storage";
	reply.attribute("content", content);
	std::string owner = dbcore["root"].name();
	if (msg.value("owner") != owner) {
		reply.attribute("checkStatus" , "Failed" );
		return reply;
	}
	reply.attribute("checkStatus", "Success");
	DbElement<PayLoad> dbelem;
	dbelem.payLoad().setNamespace(msg.value("namespace"));
	dbelem.payLoad().value("../Storage/"+ msg.value("filename"));
	if (msg.value("status") == "Open") {
		dbelem.payLoad().setStatus(open);
	}
	else if (msg.value("status") == "Close") {
		dbelem.payLoad().setStatus(close);
	}
	dbelem.descrip(msg.value("description"));
	dbelem.name(msg.value("name"));
	
	std::vector<std::string> categories;
	std::string catlist = msg.value("categories");
	std::istringstream iss(catlist);
	std::string s;
	while (getline(iss, s, ' ')) {
	    categories.push_back(s.c_str());
	}
	dbelem.payLoad().setCategory(categories);

	std::vector<std::string> children;
	std::string dependent = msg.value("dep_files");
	std::istringstream isstr(dependent);
	std::string str;
	while (getline(isstr, str, ' ')) {
		children.push_back(str.c_str());
	}

	dbelem.children(children);
	dbelem.dateTime(DateTime().now());

	PayLoad pay; CodeRepo<PayLoad> c(dbcore);
	c.setRepoDirectory("../root");	

	dbcore = c.checkInFiles(dbelem);
	dbp.db() = dbcore;

	c.persist("../PersistDatabase.xml");
	return reply;

};

//------------< request the specified files from the storage and store it in client folder >------------------


std::function<Msg(Msg)> requestFiles = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.file(msg.value("filename"));
	reply.attribute("sender", "server");
	reply.attribute("call_function", msg.value("call_function"));
	//reply.attribute("filename", msg.value("filename"));

	if (msg.value("queryKeys") == "queryKeys") {
		reply.command("file_sent");
	}
	DbProvider dbp;
	DbCore<PayLoad> dbcore = dbp.db();
	CodeRepo<PayLoad> c(dbcore);
	DbElement<PayLoad> dbelem = dbcore[msg.value("file_name")];
	std::string status, category, dependency;
	std::vector<std::string> categories, dependencies;

	if (dbelem.payLoad().getStatus() == close) {
		status = "close";
	}
	else if (dbelem.payLoad().getStatus() == open) {
		status = "open";
	}
	else if (dbelem.payLoad().getStatus() == closing) {
		status = "opening";
	}

	categories = dbelem.payLoad().categories();
	for (size_t i = 0; i < categories.size(); i++) {
		category.append(categories[i] + " ");
	}

	category = category.substr(0, category.length() - 1);

	dependencies = dbelem.children();
	for (size_t i = 0; i < dependencies.size(); i++) {
		dependency.append(dependencies[i] + " ");
	}

	dependency = dependency.substr(0, dependency.length() - 1);

	reply.attribute("description", dbelem.descrip());
	reply.attribute("datetime", dbelem.dateTime());
	reply.attribute("status", status);
	reply.attribute("categories", category);
	reply.attribute("dependent_files", dependency);
	reply.attribute("value", dbelem.payLoad().value());
	
	return reply;
};



//-------------< viewing the metadata of the file >------------------

std::function<Msg(Msg)> viewMeta = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("viewMeta");
	reply.attribute("call_function", "viewMeta");
	reply.attribute("filename", msg.value("filename"));
	
	DbProvider dbp;
	DbCore<PayLoad> dbcore = dbp.db();
	CodeRepo<PayLoad> c(dbcore);
	DbElement<PayLoad> dbelem = dbcore[msg.value("namespaces") + "::" + msg.value("filename")];
	std::string status, category, dependency;
	std::vector<std::string> categories, dependencies;

	if (dbelem.payLoad().getStatus() == close) {
		status = "close";
	}
	else if (dbelem.payLoad().getStatus() == open) {
		status = "open";
	}
	else if (dbelem.payLoad().getStatus() == closing) {
		status = "closing";
	}

	categories = dbelem.payLoad().categories();
	for (size_t i = 0; i < categories.size(); i++) {
		category.append(categories[i] + " ");
	}

	category = category.substr(0, category.length() - 1);

	dependencies = dbelem.children();
	for (size_t i = 0; i < dependencies.size(); i++) {
		dependency.append(dependencies[i] + " ");
	}

	dependency = dependency.substr(0, dependency.length() - 1);

	reply.attribute("description", dbelem.descrip());
	reply.attribute("datetime", dbelem.dateTime());
	reply.attribute("status", status);
	reply.attribute("categories", category);
	reply.attribute("dependent_files", dependency);
	reply.attribute("value", dbelem.payLoad().value());

	return reply;
};

//----------------< query the database for fetching the files which belong to specific category and dependency >----------------

std::function<Msg(Msg)> queryDb = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());

	std::string category = reply.value("category");
	std::string dependency = reply.value("dependency");
	std::string file_parent = reply.value("file_parent");

	DbProvider dbp;
	DbCore<PayLoad> dbcore = dbp.db();
	CodeRepo<PayLoad> c(dbcore);
	c.setCheckoutDirectory("../Storage");
	std::vector<std::string> checkfiles; std::string check, filename;

	checkfiles = c.queryDatabase(category, dependency, file_parent);

	for (size_t i = 0; i < checkfiles.size(); i++) {
		check.append(checkfiles[i] + " ");
	}

	check.substr(0, check.length() - 1);

	reply.attribute("dep_files", check);
	reply.attribute("call_function", "queryDb");

	if (checkfiles.size() > 0) {
		filename = checkfiles[0];
		const size_t last_slash_idx = filename.find_last_of("\\.");
		if (std::string::npos != last_slash_idx) {
			filename.erase(last_slash_idx, filename.length());
		}

		const size_t last_slash = filename.find_last_of("\\:");
		if (std::string::npos != last_slash) {
			filename.erase(0, last_slash + 1);
		}


		reply.attribute("file_name", checkfiles[0]);
		reply.attribute("filename", filename);
		reply.attribute("status", "true");
	}
	else {
		reply.attribute("status", "false");
	}
	

	return reply;
};

//------------< get the files from a specified directory >---------------

std::function<Msg(Msg)> getFiles = [](Msg msg) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("getFiles");
	reply.attribute("call_function", msg.value("call_function"));
	std::string path = msg.value("path");
	if (path != "")
	{
		std::string searchPath = storageRoot;
		if (path != ".")
			searchPath = searchPath + "\\" + path;
		Files files = Server::getFiles(searchPath);
		size_t count = 0;
		for (auto item : files)
		{
			std::string countStr = Utilities::Converter<size_t>::toString(++count);
			reply.attribute("file" + countStr, item);
		}
	}
	else
	{
		std::cout << "\n  getFiles message did not define a path attribute";
	}
	return reply;
};


//--------------< gets the directory from a specified path >--------------

std::function<Msg(Msg)> getDirs = [](Msg msg) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("getDirs");
	reply.attribute("call_function", msg.value("call_function"));
	std::string path = msg.value("path");
	if (path != "")
	{
		std::string searchPath = storageRoot;
		if (path != ".")
			searchPath = searchPath + "\\" + path;
		Files dirs = Server::getDirs(searchPath);
		size_t count = 0;
		for (auto item : dirs)
		{
			if (item != ".." && item != ".")
			{
				std::string countStr = Utilities::Converter<size_t>::toString(++count);
				reply.attribute("dir" + countStr, item);
			}
		}
	}
	else
	{
		std::cout << "\n  getDirs message did not define a path attribute";
	}
	return reply;
};

//-----------------<persisting the database into the xml file>----------------------


std::function<Msg(Msg)> persistDb = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	DbProvider dbp;
	DbCore<PayLoad> dbcore = dbp.db();
	CodeRepo<PayLoad> c(dbcore);
	std::string filepath = msg.value("filepath");

	Utilities::title("\n The contents of the repository are as follows: \n");
	showDb(dbcore, std::cout);

	Utilities::title("\n The final payload of the repository is as follows with major closed files and a few open files: \n");
	PayLoad::showDb(dbcore, std::cout);

	std::cout << "\n Path of the file where database is persisted: " << filepath;
	c.persist(filepath);

	return reply;

};

//-----------------< connects the server >-----------------------

std::function<Msg(Msg)> connectServer = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("connectServer");
	reply.attribute("content", "Client is connected to server at 8080 port");
	std::cout << "Client is connected to server at 8080 port /n";

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

	return reply;
};





int main()
{
	std::cout << "\n  Testing Server Prototype";
	std::cout << "\n ==========================";
	std::cout << "\n";
	Server server(serverEndPoint, "ServerPrototype");
	server.start();
	std::cout << "\n  testing getFiles and getDirs methods";
	std::cout << "\n --------------------------------------";
	Files files = server.getFiles();
	show(files, "Files:");
	Dirs dirs = server.getDirs();
	show(dirs, "Dirs:");
	std::cout << "\n";
	std::cout << "\n  testing message processing";
	std::cout << "\n ----------------------------";
	server.addMsgProc("echo", echo);
	server.addMsgProc("getFiles", getFiles);
	server.addMsgProc("getDirs", getDirs);
	server.addMsgProc("serverQuit", echo);
	server.addMsgProc("connect", connectServer);
	server.addMsgProc("openFile", openFile);
	server.addMsgProc("checkinProgress", checkinProgress);
	server.addMsgProc("checkin", checkinFile);
	server.addMsgProc("viewMeta", viewMeta);
	server.addMsgProc("requestFiles", requestFiles);
	server.addMsgProc("query", queryDb);
	server.addMsgProc("persist", persistDb);
	server.processMessages();
	Msg msg(serverEndPoint, serverEndPoint);  // send to self
	msg.name("msgToSelf");
	msg.command("echo");
	msg.attribute("verbose", "show me");
	server.postMessage(msg);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	msg.command("getFiles");
	msg.remove("verbose");
	msg.attributes()["path"] = storageRoot;
	server.postMessage(msg);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	msg.command("getDirs");
	msg.attributes()["path"] = storageRoot;
	server.postMessage(msg);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	std::cout << "\n  press enter to exit";
	std::cin.get();
	std::cout << "\n";
	msg.command("serverQuit");
	server.postMessage(msg);
	server.stop();
	return 0;
}

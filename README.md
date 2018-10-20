# Software-Repository

The software is a simulated remote repository which performs operations of checkin, checkout, browse and view data of files stored in the repository.

The application provides a WPF-based GUI for easy access to repository operations. The core functionality of the repository is written in C++. So, for easy interoperability between the WPF frontend and C++ backend, C++\Cli Package contains one class, Translater. Its purpose is to is to convert
managed messages, CsMessage, to native messages, Message, and back.

The repository acts as a Server running on a defined 8080 port and the clients/users of the repository connect to the server at this port. The repository listens to the commands of the clients and serves them accordingly.

The operations performed by the users/clients can be described as follows:
1. Checkin - The checking package provides checkin of files along with its dependent files and metadata into the repository storage. The functionality of handling transitive and circular dependency of files is also provided
2. Checkout - The checkout package contains checkout function which will copy the file and its dependencies from the repository storage to the checkedOutFiles folder
3. Browse - The package helps in searching the file fetched into the local repository and open the file for viewing it
4. View Metadata - This helps in viewing the metadata of the file


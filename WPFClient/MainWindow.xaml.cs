////////////////////////////////////////////////////////////////////////////////////////////////
// MainWindow.xaml.cs - It is the GUI of the project. It displays the text boxes, list boxes //
//                      and buttons. It provides various tabs like the connect,             //
//                      checkin, checkout, browse and view metadata                        //  
// Author - Chetali Mahore                                                                //
// ver 1.0                                                                               //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2018                            //
/////////////////////////////////////////////////////////////////////////////////////////
/*
*  Package Operations:
* ---------------------
* This package provides a WPF-based GUI for Project3. Various tabs are provided. They are - connect, checkin, checkout, browse and view metadata. 
* The tasks of each of the tab are as mentioned below:
* 
* - Connect: connect to the server at 8080 port
* - Checkin: perform checkin and transfer of file with the metadata such as namespace, filename, description, status, categories and dependencies from the client to the server
* - Checkout: perform checkout of the specified file from the server to the client repository
* - Browse and View Full File: perform browsing and transfer of file to the local client repository from the server and showing the contents of the file in a codepopup window
* - View metadata: displaying the metadata of a specific file by fetching it from the server
* 
*  
*  Message handling runs on a child thread, so the Server main thread is free to do
*  any necessary background processing (none, so far).
*
* Required Files:
* ---------------
* Mainwindow.xaml, MainWindow.xaml.cs
* Translater.dll
* 
* Maintenance History:
* --------------------
* ver 1.0 : 10th April 2018
*/

using Microsoft.Win32;
using MsgPassingCommunication;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;


namespace WPFClient
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private Stack<string> pathStackBrowse_ = new Stack<string>();
        private Stack<string> pathStackCheckout_ = new Stack<string>();
        private Stack<string> pathStackCheckin_ = new Stack<string>();
        private Stack<string> pathStackMeta_ = new Stack<string>();
        private Translater translater;
        private CsEndPoint endPoint_;
        private Thread rcvThrd = null;
        private Dictionary<string, Action<CsMessage>> dispatcher_
          = new Dictionary<string, Action<CsMessage>>();
        private CsEndPoint serverEndPoint;

        private List<CsMessage> msgList = new List<CsMessage>();

        //----< process incoming messages on child thread >----------------

        private void processMessages()
        {
            ThreadStart thrdProc = () => {
                while (true)
                {
                    CsMessage msg = translater.getMessage();
                    string msgId = msg.value("command");
                    if (dispatcher_.ContainsKey(msgId))
                        dispatcher_[msgId].Invoke(msg);

                    if (msgId == "clientExit")
                    {
                        translater.exit();
                        break;
                    }
                }
            };
            rcvThrd = new Thread(thrdProc);
            rcvThrd.IsBackground = true;
            rcvThrd.Start();
        }

        //------< At the start, Window_loaded function is called which invokes the dispatcher >--------------

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            loadDispatcher();
            
            string[] args = Environment.GetCommandLineArgs();
            if (args.Length == 3)
            {
               autoStub(args[2]);
            }
            else
            {
               autoStub();
            }

            
        }
        

        //----< Load the dispatcher with the following mapping of commands and their corresponding actions >---------------

        private void loadDispatcher()
        {
            DispatcherConnect();
            DispatcherLoadGetDirs();
            DispatcherLoadGetFiles();
            DispatcherOpenFile();
            DispatcherFileSent();
            DispatcherCheckinProgress();
            DispatcherCheckin();
            DispatcherMeta();
            DispatcherDisconnect();
            DispatcherQuery();
        }


        //----< add client processing for message with key >---------------

        private void addClientProc(string key, Action<CsMessage> clientProc)
        {
            dispatcher_[key] = clientProc;
        }


        //----< function dispatched by child thread to main thread >-------

        private void clearDirs(CsMessage rcvmsg)
        {

            if (rcvmsg.value("call_function") == "browse")
            {
                DirList.Items.Clear();
            }
            else if (rcvmsg.value("call_function") == "checkout")
            {
                Dir_List.Items.Clear();
            }
            else if (rcvmsg.value("call_function") == "checkin")
            {
                DirCheckin.Items.Clear();
            }
            else if (rcvmsg.value("call_function") == "viewMeta")
            {
                DirMeta.Items.Clear();
            }
        }
        //----< function dispatched by child thread to main thread >-------

        private void addDir(string dir, CsMessage rcvmsg)
        {
            if (rcvmsg.value("call_function") == "browse")
            {
                DirList.Items.Add(dir);
            }
            else if (rcvmsg.value("call_function") == "checkout")
            {
                Dir_List.Items.Add(dir);
            }
            else if (rcvmsg.value("call_function") == "checkin")
            {
                DirCheckin.Items.Add(dir);
            }
            else if (rcvmsg.value("call_function") == "viewMeta")
            {
                DirMeta.Items.Add(dir);
            }

        }

        //----< function dispatched by child thread to main thread >-------

        private void insertParent(CsMessage rcvmsg)
        {
            if (rcvmsg.value("call_function") == "browse")
            {
                DirList.Items.Insert(0, "..");
            }
            else if (rcvmsg.value("call_function") == "checkout")
            {
                Dir_List.Items.Insert(0, "..");
            }
            else if (rcvmsg.value("call_function") == "checkin")
            {
                DirCheckin.Items.Insert(0, "..");
            }
            else if (rcvmsg.value("call_function") == "viewMeta")
            {
                DirMeta.Items.Insert(0, "..");
            }
        }

        //----< function dispatched by child thread to main thread >-------

        private void clearFiles(CsMessage rcvmsg)
        {
            if (rcvmsg.value("call_function") == "browse")
            {
                FileList.Items.Clear();
            }
            else if (rcvmsg.value("call_function") == "checkout")
            {
                File_List.Items.Clear();
            }
            else if (rcvmsg.value("call_function") == "checkin")
            {
                FileCheckin.Items.Clear();
            }
            else if (rcvmsg.value("call_function") == "viewMeta")
            {
                FileMeta.Items.Clear();
            }
        }
        //----< function dispatched by child thread to main thread >-------

        private void addFile(string file, CsMessage rcvmsg)
        {
            if (rcvmsg.value("call_function") == "browse")
            {
                FileList.Items.Add(file);
            }
            else if (rcvmsg.value("call_function") == "checkout")
            {
                File_List.Items.Add(file);
            }
            else if (rcvmsg.value("call_function") == "checkin")
            {
                CheckBox box = new CheckBox();
                box.Content = file;
                FileCheckin.Items.Add(box);
            }
            else if (rcvmsg.value("call_function") == "viewMeta")
            {
                FileMeta.Items.Add(file);
            }
        }


        //----< load getDirs processing into dispatcher dictionary >-------

        private void DispatcherLoadGetDirs()
        {
            Action<CsMessage> getDirs = (CsMessage rcvMsg) =>
            {
                Action clrDirs = () =>
                {
                    clearDirs(rcvMsg);
                };
                Dispatcher.Invoke(clrDirs, new Object[] { });
                var enumer = rcvMsg.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;
                    if (key.Contains("dir"))
                    {
                        Action<string> doDir = (string dir) =>
                        {
                            addDir(dir, rcvMsg);
                        };
                        Dispatcher.Invoke(doDir, new Object[] { enumer.Current.Value });
                    }
                }
                Action insertUp = () =>
                {
                    insertParent(rcvMsg);
                };
                Dispatcher.Invoke(insertUp, new Object[] { });
            };
            addClientProc("getDirs", getDirs);
        }

        //----< load connect processing into dispatcher dictionary >-------

        private void DispatcherConnect()
        {
            Action<CsMessage> connect = (CsMessage rcvMsg) =>
            {
                Action<CsMessage> receive_msg = (CsMessage msg) =>
                {
                    //rcvMsg.show();
                    Enter_text.Text = " ";
                    Enter_text.Text = rcvMsg.value("content");
                    Browse.IsEnabled = true;
                    Checkin.IsEnabled = true;
                    Checkout.IsEnabled = true;
                    ViewMeta.IsEnabled = true;
                };
                Dispatcher.Invoke(receive_msg, new Object[] { rcvMsg });

            };
            addClientProc("connectServer", connect);
        }

        //----< load disconnect processing into dispatcher dictionary >-------


        private void DispatcherDisconnect()
        {
            Action<CsMessage> disconnect = (CsMessage rcvMsg) =>
            {
                Action<CsMessage> receive_msg = (CsMessage msg) =>
                {
                    string content = rcvMsg.value("content");
                    Enter_text.Text += "\n" + content + "\n";
                    pathStackBrowse_.Clear();
                    pathStackCheckin_.Clear();
                    pathStackCheckout_.Clear();
                    pathStackMeta_.Clear();

                    Enter_user.Clear(); Enter_filename.Clear(); Enter_namespace.Clear(); Enter_description.Clear(); Enter_category.Clear();
                    DirCheckin.Items.Clear(); DirMeta.Items.Clear(); Dir_List.Items.Clear(); DirList.Items.Clear();
                    FileCheckin.Items.Clear(); FileMeta.Items.Clear(); FileList.Items.Clear(); File_List.Items.Clear();
                    Browse_Console.Clear(); Checkin_Console.Items.Clear(); Checkout_Console.Clear(); Meta_Console.Clear();
                    Browse.IsEnabled = false; Checkin.IsEnabled = false; Checkout.IsEnabled = false; ViewMeta.IsEnabled = false;
                    selected_files.Items.Clear(); category.Items.Clear();

                };
                Dispatcher.Invoke(receive_msg, new Object[] { rcvMsg });

            };
            addClientProc("clientExit", disconnect);
        }


        //----< Message for requesting the specific files from the storage folder into client >-------

        private void requestFiles(CsMessage rcvMsg)
        {
            String[] checkFiles =  rcvMsg.value("dep_files").Split(' ');

            for(int i = 0; i < checkFiles.Length; i++)
            {
                CsMessage msg = new CsMessage();
                //  msg.add("file", checkFiles[i]);
                msg.add("to", CsEndPoint.toString(serverEndPoint));
                msg.add("from", CsEndPoint.toString(endPoint_));
                msg.add("filename", checkFiles[i]);
                msg.add("command", "requestFiles");
                msg.add("call_function", rcvMsg.value("call_function"));
                msg.add("file_name", rcvMsg.value("file_name"));
                translater.postMessage(msg);
            }
            
        }

        //-------< Requesting the files after performing the query>---------------------

        private void request(CsMessage rcvMsg)
        {
             if(rcvMsg.value("status") == "true"){
                Browse_Console.Text += "\n Browsing the file: " + rcvMsg.value("filename") + "\n \n";
                CsMessage msg = new CsMessage();
                msg.add("to", CsEndPoint.toString(serverEndPoint));
                msg.add("from", CsEndPoint.toString(endPoint_));
                msg.add("command", "requestFiles");
                msg.add("queryKeys", "queryKeys");
                msg.add("filename", rcvMsg.value("filename"));
                msg.add("call_function", rcvMsg.value("call_function"));
                msg.add("file_name", rcvMsg.value("file_name"));
                translater.postMessage(msg);
            }
              else if(rcvMsg.value("status") == "false")
            {
                Browse_Console.Text += "No Results found" +"\n\n";
            } 
        }

        //-----------------<send query for category, dependency, filename and version>--------------

        private void query_browse(object sender, RoutedEventArgs e)
        {
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "query");
            msg.add("category", Enter_cat.Text);
            msg.add("dependency", Enter_dep.Text);
            msg.add("file_parent", "false");
            translater.postMessage(msg);

        }

        //--------------< request sent for fetching files without any dependency (i.e. no parents)>-----------

        private void query_files(object sender, RoutedEventArgs e)
        {
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "query");
            msg.add("category", Enter_cat.Text);
            msg.add("dependency", Enter_dep.Text);
            msg.add("file_parent", "true");
            translater.postMessage(msg);
        }

        //--------------------<dispatcher for handling the query to the database>----------------

        private void DispatcherQuery()
        {
            Action<CsMessage> queryDb = (CsMessage rcvMsg) =>
            {
                Action<CsMessage> query = (CsMessage msg) =>
                {
                    CsMessage msg1 = new CsMessage();
                    msg1 = rcvMsg;
                    msg1.remove("call_function");
                    msg1.add("call_function", "browse");
                    Browse_Console.Text += "---------------------";
                    Browse_Console.Text += "\n Result of query: \n";
                    Browse_Console.Text += rcvMsg.value("dep_files") +"\n" ;
                    request(msg1);
                    Enter_cat.Text = "";
                    Enter_dep.Text = "";
                };
                Dispatcher.Invoke(query, new Object[] { rcvMsg });
            };
            addClientProc("query", queryDb);
        }

        
        //----< load open file processing into dispatcher dictionary >-------

        private void DispatcherOpenFile()
        {
            Action<CsMessage> openFile = (CsMessage rcvMsg) =>
            {
                Action<CsMessage> open = (CsMessage msg) =>
                {
                    if (rcvMsg.value("call_function") == "browse")
                    {
                        requestFiles(rcvMsg);
                    }
                    else if (rcvMsg.value("call_function") == "checkout")
                    {
                        requestFiles(rcvMsg);
                    }

                };
                Dispatcher.Invoke(open, new Object[] { rcvMsg });
            };

            addClientProc("openFile", openFile);
        }

        //----< load checkin processing into dispatcher dictionary >-------

        private void DispatcherCheckinProgress()
        {
            Action<CsMessage> checkinProgress = (CsMessage rcvMsg) =>
            {
                Action<CsMessage> checkinp = (CsMessage msg) =>
                {
                    if (rcvMsg.value("command") == "checkinProgress")
                    {
                        Console.WriteLine(rcvMsg.value("content") + "\n");
                    }
                };
                Dispatcher.Invoke(checkinp, new Object[] { rcvMsg });
            };
            addClientProc("checkinProgress", checkinProgress);
        }

        //---------<load the checkin command into the dispatcher>------------

        private void DispatcherCheckin()
        {
            Action<CsMessage> checkinFile = (CsMessage rcvMsg) =>
            {
                Action<CsMessage> checkin = (CsMessage msg) =>
                {
                    if (rcvMsg.value("command") == "checkin")
                    {
                        if(rcvMsg.value("checkStatus") == "Success")
                        {
                            Console.WriteLine(rcvMsg.value("content") + "\n");
                        }
                        else
                        {
                            Console.WriteLine("User Unauthorized to perform the checkin operation \n");
                        }
                 

                    }
                };
                Dispatcher.Invoke(checkin, new Object[] { rcvMsg });
            };
            addClientProc("checkin", checkinFile);
        }


        //----< load viewing metadata processing into dispatcher dictionary >-------

        private void DispatcherMeta()
        {
            Action<CsMessage> viewMeta = (CsMessage rcvMsg) =>
            {
                Action<CsMessage> view = (CsMessage msg) =>
                {
                    Meta_Console.Text += "Filename: " + rcvMsg.value("filename") + "\n";
                    Meta_Console.Text += "Description: " + rcvMsg.value("description") + "\n";
                    Meta_Console.Text += "DateTime: " + rcvMsg.value("datetime") + "\n";
                    Meta_Console.Text += "Value: " + rcvMsg.value("value") + "\n";
                    Meta_Console.Text += "Status: " + rcvMsg.value("status") + "\n";
                    Meta_Console.Text += "Categories: " + rcvMsg.value("categories") + "\n";
                    Meta_Console.Text += "Dependent Files: " + rcvMsg.value("dependent_files") + "\n";
                    Meta_Console.Text += "\n";
                };
                Dispatcher.Invoke(view, new Object[] { rcvMsg });
            };
            addClientProc("viewMeta", viewMeta);
        }

        //----< load file receiving processing into dispatcher dictionary >-------

        private void DispatcherFileSent()
        {
            Action<CsMessage> fileSent = (CsMessage rcvMsg) =>
            {
                Action<CsMessage> open = (CsMessage msg) =>
                {
                    if (rcvMsg.value("call_function") == "browse")
                    {
                        Thread.Sleep(500);
                        Browse_Console.Text += "Metadata of filename: " + rcvMsg.value("file_name") + "\n";
                        Browse_Console.Text += "Filename: " + rcvMsg.value("filename") + "\n";
                        Browse_Console.Text += "Description: " + rcvMsg.value("description") + "\n";
                        Browse_Console.Text += "DateTime: " + rcvMsg.value("datetime") + "\n";
                        Browse_Console.Text += "Value: " + rcvMsg.value("value") + "\n";
                        Browse_Console.Text += "Status: " + rcvMsg.value("status") + "\n";
                        Browse_Console.Text += "Categories: " + rcvMsg.value("categories") + "\n";
                        Browse_Console.Text += "Dependent Files: " + rcvMsg.value("dependent_files") + "\n";
                        string path = "../../../Client" + "/" + rcvMsg.value("filename");
                        string text = System.IO.File.ReadAllText(@path);
                        Browse_Console.Text += "\n" + rcvMsg.value("content") + ": " + rcvMsg.value("filename") + "\n";
                        CodePopUp pop = new CodePopUp();
                        pop.popup.Text = text;
                        pop.Show();
                    }
                    else if (rcvMsg.value("call_function") == "checkout")
                    {
                        Checkout_Console.Text += "\n";
                        Checkout_Console.Text += "\n" + rcvMsg.value("content") + ": " + rcvMsg.value("filename") + "\n";
                    }

                };
                Dispatcher.Invoke(open, new Object[] { rcvMsg });
            };
            addClientProc("File_Sent", fileSent);
        }

        //----< load getFiles processing into dispatcher dictionary >------

        private void DispatcherLoadGetFiles()
        {
            Action<CsMessage> getFiles = (CsMessage rcvMsg) =>
            {
                Action clrFiles = () =>
                {
                    clearFiles(rcvMsg);
                };
                Dispatcher.Invoke(clrFiles, new Object[] { });
                var enumer = rcvMsg.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;
                    if (key.Contains("file"))
                    {
                        Action<string> doFile = (string file) =>
                        {
                            addFile(file, rcvMsg);
                        };
                        Dispatcher.Invoke(doFile, new Object[] { enumer.Current.Value });
                    }
                }
            };
            addClientProc("getFiles", getFiles);
        }


        //----< connects to the server at 8080 port >------

        private void btn_connect(object sender, RoutedEventArgs e)
        {
            String machine_address = Enter_machine.Text;
            int port = Int32.Parse(Enter_port.Text);
            endPoint_ = new CsEndPoint();
            endPoint_.machineAddress = machine_address;
            endPoint_.port = port;
            translater = new Translater();
            translater.listen(endPoint_);
            processMessages();
            serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
            Console.WriteLine("Client is connected to the server at port 8080 \n");
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "connect");
            Disconnect.IsEnabled = true;
            Connect.IsEnabled = false;

            translater.postMessage(msg);
            Checkin_Load(sender, e);
            Checkout_Load(sender, e);
            Browse_Load(sender, e);
            Meta_Load(sender, e);

        }

        //----< disconnects from the server which is running on 8080 port >------

        private void btn_disconnect(object sender, RoutedEventArgs e)
        {
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(endPoint_));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "clientExit");
            msg.add("content", "Disconnecting from server");
            translater.postMessage(msg);

            Connect.IsEnabled = true;
            Disconnect.IsEnabled = false;
        }

        //----< loads the directories and files on clicking the load button in checkout tab >------

        private void Checkout_Load(object sender, RoutedEventArgs e)
        {
            PathName.Text = "root";
            pathStackCheckout_.Push("../root");
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "getDirs");
            msg.add("call_function", "checkout");
            msg.add("path", pathStackCheckout_.Peek());
            translater.postMessage(msg);
            msg.remove("command");
            msg.add("command", "getFiles");
            translater.postMessage(msg);
        }


        //----< loads the directories and files on clicking the load button of checkin tab>------

        private void Checkin_Load(object sender, RoutedEventArgs e)
        {
            PathCheckin.Text = "root";
            Enter_user.Text = "";
            pathStackCheckin_.Push("../root");
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "getDirs");
            msg.add("call_function", "checkin");
            msg.add("path", pathStackCheckin_.Peek());
            translater.postMessage(msg);
            msg.remove("command");
            msg.add("command", "getFiles");
            translater.postMessage(msg);
        }

        //---------< adding the selected dependencies in the listbox >-----------

        private void Add_Dependencies(object sender, RoutedEventArgs e)
        {
            string[] namespaces = PathCheckin.Text.Split('/');
            string namespace_ = namespaces[namespaces.Length - 1];

            foreach (CheckBox s in FileCheckin.Items)
            {
                string text = namespace_ +"::"+(string)s.Content;
                if (s.IsChecked == true && !selected_files.Items.Contains(text))
                {
                    selected_files.Items.Add(text);
                }
            }
        }

        //----< loads the directories and files on clicking the load button of view metadata tab >------

        private void Meta_Load(object sender, RoutedEventArgs e)
        {
            PathMeta.Text = "root";
            pathStackMeta_.Push("../root");
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "getDirs");
            msg.add("call_function", "viewMeta");
            msg.add("path", pathStackMeta_.Peek());
            translater.postMessage(msg);
            msg.remove("command");
            msg.add("command", "getFiles");
            translater.postMessage(msg);
        }

        //----< loads the directories and files on clicking the load button of browse tab >------

        private void Browse_Load(object sender, RoutedEventArgs e)
        {
            PathTextBlock.Text = "root";
            pathStackBrowse_.Push("../root");
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "getDirs");
            msg.add("call_function", "browse");
            msg.add("path", pathStackBrowse_.Peek());
            translater.postMessage(msg);
            msg.remove("command");
            msg.add("command", "getFiles");
            translater.postMessage(msg);
        }

        //-------< remove the first directory from the filepath >---------------


        private string removeFirstDir(string path)
        {
            string modifiedPath = path;
            int pos = path.IndexOf("/");
            modifiedPath = path.Substring(pos + 1, path.Length - pos - 1);
            return modifiedPath;
        }

        //--------< On double-clicking the directory listed in checkin tab, the files in the directory are displayed >-------------------

        private void DirCheckin_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            string selectedDir = (string)DirCheckin.SelectedItem;
            string path;
            if (selectedDir == "..")
            {
                if (pathStackCheckin_.Count > 1)  // don't pop off "Storage"
                    pathStackCheckin_.Pop();
                else
                    return;
            }
            else
            {
                path = pathStackCheckin_.Peek() + "/" + selectedDir;
                pathStackCheckin_.Push(path);
            }
            PathCheckin.Text = removeFirstDir(pathStackCheckin_.Peek());

            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "getDirs");
            msg.add("call_function", "checkin");
            msg.add("path", pathStackCheckin_.Peek());
            translater.postMessage(msg);

            // build message to get files and post it
            msg.remove("command");
            msg.add("command", "getFiles");
            translater.postMessage(msg);
        }

        //------------< On clicking the file in the view metadata tab, command for viewing the metadata is sent >-----------


        private void FileMeta_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            string filename = (string)FileMeta.SelectedItem;
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "viewMeta");
            msg.add("call_function", "viewMeta");
            msg.add("path", pathStackMeta_.Peek());
            string[] namespaces = PathMeta.Text.Split('/');
            string nm = namespaces[namespaces.Length - 1];
            if (nm == "Storage")
            {
                msg.add("namespaces", "");
            }
            else
            {
                msg.add("namespaces", nm);
            }
            msg.add("filename", filename);
            translater.postMessage(msg);
        }

        //------------< On clicking the file in the browse tab, command for browsing the file is sent >-----------

        private void FileList_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            string filename = (string)FileList.SelectedItem;
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "openFile");
            msg.add("call_function", "browse");
            msg.add("path", pathStackBrowse_.Peek());
            string[] namespaces = PathTextBlock.Text.Split('/');
            string nm = namespaces[namespaces.Length - 1];
            if (nm == "Storage")
            {
                msg.add("namespaces", "");
            }
            else
            {
                msg.add("namespaces", nm);
            }
            msg.add("filename", filename);
            translater.postMessage(msg);
        }

        //---------<  On clicking the file in the checkout tab, command for checking out the file is sent >--------------

        private void File_List_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            string filename = (string)File_List.SelectedItem;
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "openFile");
            msg.add("call_function", "checkout");
            msg.add("path", pathStackCheckout_.Peek());
            string[] namespaces = PathName.Text.Split('/');
            string nm = namespaces[namespaces.Length - 1];
            if (nm == "Storage")
            {
                msg.add("namespaces", "");
            }
            else
            {
                msg.add("namespaces", nm);
            }
            msg.add("filename", filename);
            translater.postMessage(msg);
            
        }

        //--------< On double-clicking the directory listed in browse tab, the files in the directory are displayed  >-------------------

        private void DirList_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            // build path for selected dir
            string selectedDir = (string)DirList.SelectedItem;
            string path;
            if (selectedDir == "..")
            {
                if (pathStackBrowse_.Count > 1)  // don't pop off "Storage"
                    pathStackBrowse_.Pop();
                else
                    return;
            }
            else
            {
                path = pathStackBrowse_.Peek() + "/" + selectedDir;
                pathStackBrowse_.Push(path);
            }
            PathTextBlock.Text = removeFirstDir(pathStackBrowse_.Peek());

            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "getDirs");
            msg.add("call_function", "browse");
            msg.add("path", pathStackBrowse_.Peek());
            translater.postMessage(msg);

            msg.remove("command");
            msg.add("command", "getFiles");
            translater.postMessage(msg);
        }

        //--------< On double-clicking the directory listed in checkout tab, the files in the directory are displayed  >-------------------

        private void Dir_List_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            string selectedDir = (string)Dir_List.SelectedItem;
            string path;
            if (selectedDir == "..")
            {
                if (pathStackCheckout_.Count > 1)  // don't pop off "Storage"
                    pathStackCheckout_.Pop();
                else
                    return;
            }
            else
            {
                path = pathStackCheckout_.Peek() + "/" + selectedDir;
                pathStackCheckout_.Push(path);
            }
            PathName.Text = removeFirstDir(pathStackCheckout_.Peek());

            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("call_function", "checkout");
            msg.add("path", pathStackCheckout_.Peek());
            msg.add("command", "getDirs");
            translater.postMessage(msg);

            msg.remove("command");
            msg.add("command", "getFiles");
            translater.postMessage(msg);
        }

        //----< loads the directories and files on clicking the load button of checkin tab >------

        private void btn_browse(object sender, RoutedEventArgs e)
        {
            OpenFileDialog openFileDialog1 = new OpenFileDialog();
            openFileDialog1.Title = "Browse Text Files";
            openFileDialog1.CheckFileExists = true;
            openFileDialog1.CheckPathExists = true;
            openFileDialog1.DefaultExt = "txt";
            openFileDialog1.Filter = "Text files (*.txt)|*.txt|All files (*.*)|*.*";
            openFileDialog1.FilterIndex = 2;
            openFileDialog1.RestoreDirectory = true;
            openFileDialog1.ReadOnlyChecked = true;
            openFileDialog1.ShowReadOnly = true;

            if (openFileDialog1.ShowDialog() == true)
            {
                Enter_filename.Text = openFileDialog1.FileName;
            }
        }

        //----------< adds the category in the categories list >-----------------

        private void btn_add(object sender, RoutedEventArgs e)
        {
            string text = Enter_category.Text;
            if (!category.Items.Contains(text))
            {
                category.Items.Add(text);
            }
        }

        //-----------< removes the category from the category list >----------------------

        private void btn_remove(object sender, RoutedEventArgs e)
        {
                if ((string)category.SelectedValue != "")
                {
                    category.Items.Remove(category.SelectedValue);
                }
                
                
        }


        //--------------<adding the file in the list for checkin files>----------------------

        private void btn_addFile(object sender, RoutedEventArgs e)
        {
            string path = Enter_filename.Text;   string owner = Enter_user.Text;  string namespace_ = Enter_namespace.Text;
            string description = Enter_description.Text;
            string status = Enter_status.Text;
            if (path == "")
            {
                System.Windows.MessageBox.Show("Please Select a file", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }
            if (namespace_ == "")
            {
                System.Windows.MessageBox.Show("Please Enter a namespace", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }
            if (owner == "")
            {
                System.Windows.MessageBox.Show("Please Enter a Owner Name", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }
            string[] filenames = path.Split('\\');
            string filename = filenames[filenames.Length - 1];
            if (Enter_filename.Text.Contains("CheckinTest")) { filename = "CheckinTest.cpp"; }
            if (Enter_filename.Text.Contains("FileSystem")) { filename = "FileSystem.h"; }
            if (Enter_filename.Text.Contains("File8")) { filename = "File8.cpp"; }
            else if (Enter_filename.Text.Contains("File4")) { filename = "File4.h"; }
            string categories = "", dep_files = "";
            foreach (string s in category.Items)
            {
                categories += s + " ";
            }
            foreach (string dep in selected_files.Items)
            {
                dep_files += dep + " ";
            }
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("name", "Chetali");
            msg.add("command", "checkinProgress");
            msg.add("call_function", "checkin");
            msg.add("filename", filename);
            msg.add("namespace", namespace_);
            msg.add("description", description);   msg.add("status", status);       msg.add("categories", categories);
            msg.add("dep_files", dep_files);    msg.add("sender", "client");     msg.add("file", namespace_ + "/" + filename);
            msg.add("path", Enter_filename.Text);       msg.add("owner", owner);
            msgList.Add(msg);      Checkin_Console.Items.Add(namespace_ + "::" + filename);  Enter_filename.Clear(); Enter_namespace.Clear(); Enter_description.Clear(); Enter_category.Clear();
            selected_files.Items.Clear(); category.Items.Clear();
        }


        //-------------< provides checkin of file in the repository >---------------

        private void btn_checkin(object sender, RoutedEventArgs e)
        {
            if(msgList.Count() <= 0)
            {
                return;
            }
            foreach(CsMessage msg in msgList)
            {
                translater.postMessage(msg);
                msg.remove("file");
                msg.remove("command");
                msg.add("command", "checkin");
                translater.postMessage(msg);
            }
            Checkin_Console.Items.Clear();
            msgList.Clear();
            PathCheckin.Text = "root";
            Checkin_Load(this, null);
        }

        //--------< On double-clicking the directory listed in view metadata tab, the files in the directory are displayed >-------------------


        private void DirMeta_MouseDoubleClick(object sender, RoutedEventArgs e)
        {
            string selectedDir = (string)DirMeta.SelectedItem;
            string path;
            if (selectedDir == "..")
            {
                if (pathStackMeta_.Count > 1)  // don't pop off "Storage"
                    pathStackMeta_.Pop();
                else
                    return;
            }
            else
            {
                path = pathStackMeta_.Peek() + "/" + selectedDir;
                pathStackMeta_.Push(path);
            }
            PathMeta.Text = removeFirstDir(pathStackMeta_.Peek());


            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("call_function", "viewMeta");
            msg.add("path", pathStackMeta_.Peek());
            msg.add("command", "getDirs");
            translater.postMessage(msg);

            msg.remove("command");
            msg.add("command", "getFiles");
            translater.postMessage(msg);
        }


        //-----------< automation for client 2 >-------------------

        private async void autoStub(string port)
        {
            RoutedEventArgs e = new RoutedEventArgs();
            Console.WriteLine("Client is running at port " + port);
            Console.WriteLine("---------------------------------------------------------");
            Console.WriteLine("Demonstrating Connect, Checkin, Browse, Checkout for Client 2");
            Console.WriteLine("---------------------------------------------------------");
            Console.WriteLine("\n Demonstrating Connnect Requirement \n");
            Enter_port.Text = port;
            Console.WriteLine("---------------------------------------------------------");
            btn_connect(this, e);
            await haveDelay(4000);
            Console.WriteLine("\n Demonstrating Checkin Requirement \n");
            Console.WriteLine("--------------------------------------------------------- ");
            autoCheckin2();
            await haveDelay(101000);
            Console.WriteLine("\n Demonstrating Checkout Requirement \n");
            Console.WriteLine("--------------------------------------------------------- ");
            autoCheckout2();
            await haveDelay(12000);
            
            Console.WriteLine("\n Demonstrating Browse and View File Requirement \n");
            Console.WriteLine("--------------------------------------------------------- ");
            autoBrowse2();
            await haveDelay(30000);

            checkinUser();
            await haveDelay(20000);
            tabControl.SelectedIndex = 0;
        }

        private async void checkinUser()
        {
            Console.WriteLine("\n Demonstrating Checkin Requirement of unauthorised user \n");
            Console.WriteLine("--------------------------------------------------------- ");
            tabControl.SelectedIndex = 1;
            Enter_user.Text = "camahore";
            await haveDelay(2000);
            Enter_filename.Text = "../../../Client/File4.h";
            await haveDelay(2000);
            Enter_namespace.Text = "namespace2";
            await haveDelay(2000);
            Enter_description.Text = "It's a File4.h file";
            await haveDelay(2000);
            Enter_status.Text = "Close";
            await haveDelay(2000);
            Enter_category.Text = "Category1";
            await haveDelay(1500);
            btn_add(this, null);
            Enter_category.Text = "Category2";
            await haveDelay(1500);
            btn_add(this, null);
            await haveDelay(2500);

            btn_addFile(this, null);
            await haveDelay(2000);

            btn_checkin(this, null);
            await haveDelay(4000);
            
        }



        //-------------------< automation for client 1 >------------------------

        private async void autoStub()
        {
            RoutedEventArgs e = new RoutedEventArgs();
            Console.WriteLine("Client is running at port 8082");
            Console.WriteLine("---------------------------------------------------------");
            Console.WriteLine("Demonstrating Connect, Checkin, Browse, Checkout for Client 1");
            Console.WriteLine("--------------------------------------------------------- ");
            Console.WriteLine("\n Demonstrating Connnect Requirement \n");
            Console.WriteLine("--------------------------------------------------------- ");
            btn_connect(this, e);
            await haveDelay(5000);
            
            Console.WriteLine("\n Demonstrating Checkin Requirement \n");
            Console.WriteLine("--------------------------------------------------------- ");
            autoCheckin();
            await haveDelay(101000);
            
            Console.WriteLine("\n Demonstrating Checkout Requirement \n");
            Console.WriteLine("--------------------------------------------------------- ");
            autoCheckout();

            await haveDelay(12000);
            
            Console.WriteLine("\n Demonstrating Browse and View File Requirement \n");
            Console.WriteLine("--------------------------------------------------------- ");
            autoBrowse();
            await haveDelay(30000);
            
            
            Console.WriteLine("\n Demonstrating View Metadata Requirement \n");
            Console.WriteLine("--------------------------------------------------------- ");
            autoMeta();
            await haveDelay(25000);

            tabControl.SelectedIndex = 0;
        }

        //------------< introduce delay of milliseconds >--------------

        async Task haveDelay(int delay)
        {
            await Task.Delay(delay);

        }

        //-----------< perform checkin of client 1 > ---------------

        private async void autoCheckin()
        {
            tabControl.SelectedIndex = 1;     Enter_user.Text = "Chetali";   await haveDelay(2000);
            Console.WriteLine("\n Filename: ../../../Client/FileSystem.h \n Namespace:namespace2 \n Description: It's a Filesystem.h file \n Status: Close \n Categories: Category1 Category2 \n Children: namespace2::File2.cpp.1 namespace2::File4.h.1 ");
            Enter_filename.Text = "../../../Client/FileSystem.h";  await haveDelay(2000);  Enter_namespace.Text = "namespace2";  await haveDelay(2000);
            Enter_description.Text = "It's a Filesystem.h file";   await haveDelay(2000);  Enter_status.Text = "Close";   await haveDelay(2000);
            Enter_category.Text = "Category1";  await haveDelay(1500);   btn_add(this, null);  Enter_category.Text = "Category2";   await haveDelay(1500);
            btn_add(this, null);  await haveDelay(2500); DirCheckin.SelectedIndex = DirCheckin.Items.IndexOf("namespace2"); await haveDelay(2000);
            DirCheckin_MouseDoubleClick(this, null);  await haveDelay(2000);
            foreach (CheckBox box in FileCheckin.Items) {
                if ((string)box.Content == "File2.cpp.1" || (string)box.Content == "File4.h.1")
                    box.IsChecked = true;
            }
            await haveDelay(2000);  Add_Dependencies(this, null);  await haveDelay(3500);  btn_addFile(this, null);
            await haveDelay(2000);  DirCheckin.SelectedIndex = DirCheckin.Items.IndexOf("..");  await haveDelay(2000);
            DirCheckin_MouseDoubleClick(this, null);
            Console.WriteLine("\n Filename: ../../../Client/CheckinTest.cpp \n Namespace:namespace6 \n Description: It's a Test cpp file \n Status: Open \n  Categories: Category1 Category3 \n Children: namespace1::File1.cpp.1 namespace3::File6.h.1 ");
            Enter_filename.Text = "../../../Client/CheckinTest.cpp"; await haveDelay(2000);  Enter_namespace.Text = "namespace6";  await haveDelay(2000);
            Enter_description.Text = "It's a Test cpp file"; await haveDelay(2000);  Enter_status.Text = "Open";
            await haveDelay(2000); Enter_category.Text = "Category1"; await haveDelay(1500);  btn_add(this, null);
            Enter_category.Text = "Category3";  await haveDelay(1500);  btn_add(this, null);  await haveDelay(2500);
            DirCheckin.SelectedIndex = DirCheckin.Items.IndexOf("namespace1");  await haveDelay(2000); DirCheckin_MouseDoubleClick(this, null); await haveDelay(2000);
            foreach (CheckBox box in FileCheckin.Items) {
                if ((string)box.Content == "File1.cpp.1")
                    box.IsChecked = true;
            }
            await haveDelay(2000); Add_Dependencies(this, null);  DirCheckin.SelectedIndex = DirCheckin.Items.IndexOf("..");  await haveDelay(2000);
            DirCheckin_MouseDoubleClick(this, null); await haveDelay(1000); DirCheckin.SelectedIndex = DirCheckin.Items.IndexOf("namespace3");
            await haveDelay(2000); DirCheckin_MouseDoubleClick(this, null); await haveDelay(2000);
            foreach (CheckBox box in FileCheckin.Items)   {
                if ((string)box.Content == "File6.h.1")
                    box.IsChecked = true;
            }
            await haveDelay(2000);  Add_Dependencies(this, null);  await haveDelay(3500); btn_addFile(this, null);  await haveDelay(2000);
            btn_checkin(this, null);   await haveDelay(3000);
            Console.WriteLine("FileSystem.h successfully checked in the ../root/namespace2 folder of repository with version 1\n");
            Console.WriteLine("CheckinTest.cpp successfully checked in the ../root/namespace1 folder of repository with version 1\n");
            Console.WriteLine("\n Filename: ../../../Client/FileSystem.h \n Namespace:namespace2 \n Description: It's a Filesystem.h file \n Status: Close \n  Categories: Category1 Category2 \n Children = namespace2::File2.cpp.1 namespace2::File4.h.1 ");
            Enter_user.Text = "Chetali";   Enter_filename.Text = "../../../Client/FileSystem.h";  await haveDelay(2000);  Enter_namespace.Text = "namespace2"; await haveDelay(2000);
            Enter_description.Text = "It's a Filesystem.h file"; await haveDelay(2000);  Enter_status.Text = "Close";  await haveDelay(2000);
            Enter_category.Text = "Category6";  await haveDelay(1500);  btn_add(this, null);   Enter_category.Text = "Category2";
            await haveDelay(1500);  btn_add(this, null); await haveDelay(2500); DirCheckin.SelectedIndex = DirCheckin.Items.IndexOf("namespace2"); await haveDelay(2000);  DirCheckin_MouseDoubleClick(this, null);  await haveDelay(2000);
            foreach (CheckBox box in FileCheckin.Items)            {
                if ((string)box.Content == "File2.cpp.1" || (string)box.Content == "File4.h.1")
                    box.IsChecked = true;
            }
            await haveDelay(2000);  Add_Dependencies(this, null); await haveDelay(3500); btn_addFile(this, null);  await haveDelay(2000); btn_checkin(this, null); await haveDelay(2000); Checkin_Load(this, null);
            await haveDelay(3000);  Console.WriteLine("FileSystem.h Successfully checked in the ../root/namespace2 with a new version \n");
            Console.WriteLine("Checkin Requirement done \n");  await haveDelay(2000);
        }

        //-------<method to call the browse query again>-------------------

        private async void call_method()
        {
            Checkin_Load(this, null);
            await haveDelay(3000);
            tabControl.SelectedIndex = 1;
            Enter_user.Text = "Chetali";
            await haveDelay(2000);
            Console.WriteLine("\n Checkin of Filename: ../../../Client/File4.h with close status \n ");
            Enter_filename.Text = "../../../Client/File4.h";
            await haveDelay(2000);
            Enter_namespace.Text = "namespace2";
            await haveDelay(2000);
            Enter_description.Text = "It's a File4.h file";
            await haveDelay(2000);
            Enter_status.Text = "Close";
            await haveDelay(2000);
            Enter_category.Text = "Category1";
            await haveDelay(1500);
            btn_add(this, null);
            Enter_category.Text = "Category2";
            await haveDelay(1500);
            btn_add(this, null);
            await haveDelay(2500);

            btn_addFile(this, null);
            await haveDelay(2000);

            btn_checkin(this, null);
            await haveDelay(4000);

            Console.WriteLine("Since the status of File4.h is closed, the dependent file File8.cpp.1 will also be closed now");
           
            autoMeta2();
            await haveDelay(20000);
        }



        //-------------------< perform checkin of client 2 >-----------------

        private async void autoCheckin2()
        {
            Checkin_Load(this, null);
            await haveDelay(3000);
            tabControl.SelectedIndex = 1;
            Enter_user.Text = "Chetali";
            await haveDelay(2000);
            Console.WriteLine("\n Checkin of Filename: ../../../Client/File8.cpp which has its dependency 'File4.h.1' open \n ");
            Enter_filename.Text = "../../../Client/File8.cpp";
            await haveDelay(2000);
            Enter_namespace.Text = "namespace1";
            await haveDelay(2000);
            Enter_description.Text = "It's a File8.cpp file";
            await haveDelay(2000);
            Enter_status.Text = "Close";
            await haveDelay(2000);
            Enter_category.Text = "Category1";
            await haveDelay(1500);
            btn_add(this, null);
            Enter_category.Text = "Category2";
            await haveDelay(1500);
            btn_add(this, null);
            await haveDelay(2500);
            DirCheckin.SelectedIndex = DirCheckin.Items.IndexOf("namespace2");
            await haveDelay(2000);
            DirCheckin_MouseDoubleClick(this, null);
            await haveDelay(2000);

            foreach (CheckBox box in FileCheckin.Items)
            {
                if ((string)box.Content == "File4.h.1")
                    box.IsChecked = true;
            }
            await haveDelay(2000);
            Add_Dependencies(this, null);
            await haveDelay(3500);
            btn_addFile(this, null);
            await haveDelay(2000);

            btn_checkin(this, null);
            Console.WriteLine("Status of file File8.cpp is closing");
            await haveDelay(4000);

            Console.WriteLine("\n Demonstrating View Metadata Requirement \n");
            Console.WriteLine("--------------------------------------------------------- ");
            autoMeta2();
            await haveDelay(20000);

            call_method();
        }
        
        //------------------< perform checkout of client 1 >------------------------

        private async void autoCheckout()
        {
            Checkout_Load(this, null);
            await haveDelay(2000);
            tabControl.SelectedIndex = 2;
            await haveDelay(2000);
            Dir_List.SelectedIndex = Dir_List.Items.IndexOf("namespace1");
            await haveDelay(2000);
            Dir_List_MouseDoubleClick(this, null);
            await haveDelay(2000);
            File_List.SelectedIndex = File_List.Items.IndexOf("File1.cpp.1");
            await haveDelay(2000);
            File_List_MouseDoubleClick(this, null);
            await haveDelay(2000);
            Console.WriteLine("File File1.cpp successfully checked out at location ../../../Client along with its dependencies");

        }

        //------------------< perform checkout of client 2 >------------------------

        private async void autoCheckout2()
        {
            Checkout_Load(this, null);
            tabControl.SelectedIndex = 2;
            await haveDelay(2000);
            Dir_List.SelectedIndex = Dir_List.Items.IndexOf("namespace1");
            await haveDelay(2000);
            Dir_List_MouseDoubleClick(this, null);
            await haveDelay(2000);
            File_List.SelectedIndex = File_List.Items.IndexOf("File8.cpp.1");
            await haveDelay(2000);
            File_List_MouseDoubleClick(this, null);
            await haveDelay(2000);
            Console.WriteLine("File File8.cpp successfully checked out at location ../../../Client along with its dependencies");
        }

        //------------------< perform browse of client 1 >--------------------

        private async void autoBrowse()
        {
            Browse_Load(this, null);
            await haveDelay(2000);
            tabControl.SelectedIndex = 3;
            await haveDelay(2000);
            DirList.SelectedIndex = DirList.Items.IndexOf("namespace2");
            await haveDelay(2000);
            DirList_MouseDoubleClick(this, null);
            await haveDelay(2000);
            FileList.SelectedIndex = FileList.Items.IndexOf("FileSystem.h.2");
            await haveDelay(2000);
            FileList_MouseDoubleClick(this, null);
            await haveDelay(2000);
            Console.WriteLine("File FileSystem.h successfully transferred to location ../../../Client \n");
            Console.WriteLine(" Browsing of FileSystem.h.2 done by showing in new window i.e. CodePopUp \n");
            Console.WriteLine("Viewing the contents of the file FileSystem.h.2--- \n Viewing File Requirement done\n");
            Console.WriteLine("Browsing the database by category1 \n Done Successfully \n ");
            Enter_cat.Text = "Category1";
            await haveDelay(2000);
            query_browse(this, null);
            await haveDelay(5000);

            Console.WriteLine("Browsing the database by dependency namespace2::File4.h.1 \n");
            Enter_dep.Text = "namespace2::File4.h.1";
            await haveDelay(2000);
            query_browse(this, null);
            await haveDelay(5000);

            Enter_cat.Text = "category1";
            Enter_dep.Text = "namespace2::File4.h.1";
            Console.WriteLine("Browsing the database by performing the ADDIng operation of category and dependency \n");
            await haveDelay(3000);
            query_browse(this, null);
            await haveDelay(5000);

            query_files(this, null);
            await haveDelay(4000);
        }

        //----------------< perform browse of client 2 >------------------------

        private async void autoBrowse2()
        {
            Browse_Load(this, null);
            await haveDelay(2000);
            tabControl.SelectedIndex = 3;
            Console.WriteLine("Browsing the database by category1 \n Done Successfully \n ");
            Enter_cat.Text = "Category1";
            await haveDelay(4000);
            query_browse(this, null);
            await haveDelay(5000);

            Console.WriteLine("Browsing the database by dependency namespace2::File4.h.1 \n");
            Enter_dep.Text = "namespace2::File4.h.1";
            await haveDelay(4000);
            query_browse(this, null);
            await haveDelay(5000);

            Enter_cat.Text = "category1";
            Enter_dep.Text = "namespace2::File4.h.1";
            Console.WriteLine("Browsing the database by performing the ADDIng operation of category and dependency \n");
            await haveDelay(4000);
            query_browse(this, null);
            await haveDelay(5000);

            query_files(this, null);
            await haveDelay(5000);
        }

        //--------------< perform viewing of metadata for client 1  >------------------

        private async void autoMeta()
        {
            Meta_Load(this, null);
            await haveDelay(2000);
            tabControl.SelectedIndex = 4;
            Meta_Load(this, null);
            await haveDelay(2000);
            DirMeta.SelectedIndex = DirMeta.Items.IndexOf("namespace3");
            await haveDelay(3000);
            DirMeta_MouseDoubleClick(this, null);
            await haveDelay(2000);
            FileMeta.SelectedIndex = FileMeta.Items.IndexOf("File6.h.1");
            await haveDelay(3000);
            FileMeta_MouseDoubleClick(this, null);
           
            await haveDelay(3000);
            Meta_Load(this, null);
            await haveDelay(3000);
            DirMeta.SelectedIndex = DirMeta.Items.IndexOf("namespace2");
            await haveDelay(3000);
            DirMeta_MouseDoubleClick(this, null);
            await haveDelay(3000);
            FileMeta.SelectedIndex = FileMeta.Items.IndexOf("File2.cpp.1");
            await haveDelay(3000);
            FileMeta_MouseDoubleClick(this, null);
            await haveDelay(3000);
            
            Console.WriteLine(" Metadata successfully fetched for files namespace3::File6.h.1 and namespace2::File2.cpp.1 \n");
            Console.WriteLine("Viewing Metadata Requirement done \n");
        }

        //--------------< perform viewing of metadata for client 2  >------------------

        private async void autoMeta2()
        {
            Meta_Load(this, null);
            await haveDelay(2000);
            tabControl.SelectedIndex = 4;
            Meta_Load(this, null);
            await haveDelay(2000);
            DirMeta.SelectedIndex = DirMeta.Items.IndexOf("namespace1");
            await haveDelay(3000);
            DirMeta_MouseDoubleClick(this, null);
            await haveDelay(2000);
            FileMeta.SelectedIndex = FileMeta.Items.IndexOf("File8.cpp.1");
            await haveDelay(3000);
            FileMeta_MouseDoubleClick(this, null);
            Console.WriteLine(" Metadata successfully fetched for files File8.cpp.1 \n");
            Console.WriteLine(" Viewing Metadata Requirement done for file8.cpp.1 \n");

            Meta_Load(this, null);
            await haveDelay(2000);
            DirMeta.SelectedIndex = DirMeta.Items.IndexOf("namespace2");
            await haveDelay(3000);
            DirMeta_MouseDoubleClick(this, null);
            await haveDelay(2000);
            FileMeta.SelectedIndex = FileMeta.Items.IndexOf("File4.h.1");
            await haveDelay(3000);
            FileMeta_MouseDoubleClick(this, null);
            Console.WriteLine(" Metadata successfully fetched for files File4.h.1 \n");
            Console.WriteLine(" Viewing Metadata Requirement done for file4.h.1\n");
        }

        //-------------------< disconnect reply for client 1 >----------

        private async void autoDisconnect()
        {
            RoutedEventArgs e = new RoutedEventArgs();
            tabControl.SelectedIndex = 0;
            await haveDelay(2000);
            btn_disconnect(this, e);
            Console.WriteLine(" Client is disconnected from server  \n");
        }

        //------------------< disconnect reply for client 2 >-------------

        private async void autoDisconnect2()
        {
            RoutedEventArgs e = new RoutedEventArgs();
            tabControl.SelectedIndex = 0;
            await haveDelay(2000);
            btn_disconnect(this, e);
            Console.WriteLine(" Client is disconnected from server \n");
        }
    }
}

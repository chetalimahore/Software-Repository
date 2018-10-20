//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CodePopUp.xaml.cs -  On clicking the file in the browse tab, the contents of the file are displayed.  //  
// Author - Chetali Mahore                                                                              //
// ver 1.0                                                                                             //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2018                                          //
///////////////////////////////////////////////////////////////////////////////////////////////////////
/*
*
* Package operations:
* -----------------------
* 
* When we browse the file and double click on it, a window is popped up which shows the contents 
* of the file.
* 
* Required Files:
* ------------------------
* CodePopUp.xaml, CodePopUp.xaml.cs
* 
* Maintenance History:
* --------------------------- 
* ver 1.0 : 9th April 2018
* 
* 
*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
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
    /// Interaction logic for pop_up_window.xaml
    /// </summary>
    public partial class CodePopUp : Window
    {
        public CodePopUp()
        {
            InitializeComponent();
        }
    }
}

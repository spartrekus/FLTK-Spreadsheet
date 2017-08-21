//sprsht v1.2.2
/*Copyright (C) 2007 norman a bakker

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// based on exercisetablerow -- Exercise all aspects of the Fl_Table_Row widget
// and on single input : now takes all types of input and leaves it to the pgm to interpret
// also testkeyboardnav for selection of cells
// Test Jean-Marc's mods for keyboard nav and mouse selection, using a modified
// version of the singleinput program.
//
//	1.00 04/18/03 Mister Satan      -- Initial implementation
//      1.10 05/17/03 Greg Ercolano     -- Small mods to follow changes to Fl_Table
//      1.20 02/22/04 Jean-Marc Lienher -- Keyboard nav and mouse selection
//      1.21 02/22/04 Greg Ercolano     -- Small reformatting mods, comments
//

// norman bakker 20060927
// added resizable safe array types SDRarray and SDRarray2D adapted from examples
// given in code from the book 	Data Structures and Algorithms
//								with Object Oriented Design Patterns in C++
// 	copyright (c) 1998 by Bruno R. Preiss
//	The Complete Reference C++ 4th edition
//  coyyright (c) Herbert Schildt
//	WARNING  these safearrays reset out of bound indices to 0 or maxarraylength -1 as appropriate

#ifdef _WIN32
#include <stdlib.h>	// atoi
#endif /*_WIN32*/
#include "edwin.xpm" //in XPM 3 format

#include <iostream>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Adjuster.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Slider.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Box.H>
#include <FL/fl_show_colormap.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_PNG_Image.H>

#include <string>
#include <algorithm>
#include <limits>
#include <errno.h>
#include <bitset>
#include <ctime>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <ctime>
#include <cmath>
#include <list>
#include <locale>
#include <iomanip>
#include <stack>
#include <deque>
#include <cassert>

//includes in local folder
#include "sprsht-0-definitions.h"
//#include "StreamIO.h"
#include "sprsht-1-sdrarrays.cxx"
//order is important in following
#include "sprsht-1a-fl_table.cxx"
#include "sprsht-1b-classes.cxx"
#include "sprsht-2a-Sprsht-classfns.cxx"
#include "sprsht-2b-Sprsht-classfns.cxx"
#include "sprsht-3a-parser-userfns.cxx"
#include "sprsht-3b-parser-classfns.cxx"
#include "sprsht-4-userfns.cxx"
#include "sprsht-4a-pipeflowfns.cxx"
#include "sprsht-4b-pipenetwork.cxx"
#include "sprsht-5-menu-functions.cxx"
#include "sprsht-6-menuitems.cxx"


using namespace std;
// some cb fns  ================================================

void input_cb(Fl_Widget* w, void* v){ ((Spreadsheet*)v)-> set_value();}
/*
void sheetname_cb(Fl_Widget* w, void *data) {//change sheetname display; works but not instantaneous
// only after clicking on name
	Fl_Output * sn = (Fl_Output *) data;
    sn->value(ss->sheetname[ss->currentsheet].c_str());
    Fl::redraw(); //provides a cleaner redraw than sn->redraw() for some reason
}
*/
void quit_cb(Fl_Widget*, void*) {exit(0);}

void precision_cb(Fl_Widget* w, void*data)  //number precision for selection from widget
{
	Fl_Counter *cnt = (Fl_Counter*) data;
	int prec = (int) cnt->value();
	for (int r=ss->s_top;r<ss->s_bottom+1;r++) {
		for (int c= ss->s_left; c< ss->s_right+1; c++) {
		//ss->numprecision[c]=prec;
		ss->cells[r][c].precisionx(prec);
		}
	}
	ss->redraw();
}

// this is used to stop Esc from exiting the program: 20070823
int handle(int e) {  //see keyboard.cxx in test folder of fltk-1.1.8
 	return (e == FL_SHORTCUT); // eat all keystrokes see add_handler() below
}

// ===============================================================
int main(int argc, char * argv[])
{
try	{
	//enter no of sheets as argument to main or if not provided here below
	const char * msg =
	"usage:  sprsht  -<switches> number (nn) or string ss\n\
	-d    :  accept all defaults\n\
	-S nn :  number of sheets (1 to 16)    -    [5]\n\
	-r nn :  number of rows                -  [100]\n\
	-c nn :  number of cols                -   [25]\n\
	-W nn :  window width                  - [1200]\n\
	-H nn :  window height                 -  [580]\n\
	-p nn :	 start program nn              -    [0]\n\
	where    1 : pipeflow\n\
	         2 : pipenetwork\n\
 	         3 : pipenetwork example\n";
 //   -b nn :	 background colour             -  [205,192,176]\n";

	int n=1, n0=0;
	if (argc<2) {printf("%s",msg); fl_message("%s",msg);} //should test for a console?
	char ag;
	while (n<argc-1) {
		ag=argv[n][1];
		if (ag=='d') {n=argc;cout<<"using defaults\n";continue;} //bypass further tests
		n0=atoi(argv[n+1]);
		if (n0>0) {
			switch (ag) {
				case 'S': {ss->nosheets=n0;break;}
				case 'r': {ss->norows=n0;break;}
				case 'c': {ss->nocols=n0;break;}
				case 'W': {windowW = n0;break;}
				case 'H': {windowH = n0;break;}
				case 'b': break;
				default : break;
			}
		}
		n+=2;
	}

	ss->cells.sdrcols(ss->nocols); //resize cells array
	ss->cells.sdrrows(ss->norows);

	if (ss->nosheets<1) ss->nosheets=1;
	if (ss->nosheets>16) ss->nosheets=16; //this will be used below

	cout<<"sheets "<<ss->nosheets<<" rows "<<ss->norows<<" columns "<<ss->nocols<<endl;
	Fl::visual(FL_DOUBLE|FL_INDEX);//does this do anything usually?

	Fl_Double_Window win(windowW, windowH, VERSION);

	win.box((Fl_Boxtype) FL_THIN_DOWN_BOX);
	//Fl::foreground(240,140,230);

	ss = new Spreadsheet(20, 20+35, windowW-40, windowH-100, 0);
    ss->when(FL_WHEN_RELEASE|FL_WHEN_CHANGED);
    ss->table_box(FL_NO_BOX);
	ss->col_resize_min(4);
    ss->row_resize_min(4);
	//following clears all cells of garbage
	ss->clearallcells();
	ss->clear_inputvalue();

    // ROWS
    ss->row_header(1);
    ss->row_header_width(COLWIDTH/2);//note /2
    ss->row_header_color(HDRCOLOR);
    ss->row_resize(1);
    ss->rows(ss->norows);
    ss->row_height_all(ROWHEIGHT);

    // COLS
	ss->cols(ss->nocols);
    ss->col_header(2);
    ss->col_header_color(HDRCOLOR);
    ss->col_header_height(COLHDRHEIGHT);
    ss->col_resize(1);
    ss->col_width_all(COLWIDTH);

    // Add children to window
    win.begin();
    Fl::add_handler(&handle); //turn off escape button on keyboard to avoid premature close down 20070823
    Fl_Menu_Bar menubar(20,0,windowW-40,30);
    menubar.copy(menutable);
    menubar.textsize(18);
    menubar.textcolor(FL_DARK_BLUE);
    menubar.textfont(FONT);
    menubar.box(FL_NO_BOX);
    menubar.callback(menubar_cb, (void*)&menubar);

    //add sheet menu
    ss->sheet.length(ss->nosheets);
    ss->sheetname.length(ss->nosheets);
    ss->shptrs.length(ss->nosheets);
    cout<<"adding sheet menu to menubar\n";
    ss->create_sheet_menu(&menubar);//add a Sheet menu to the menubar with up to 16 sheets
    //reset number to nosheets to start (can be increased if required; all 16 sheets will be saved as strings
    //once getdata is called nosheets can't be changed: pointer stale?


//.................................
    int crh = ss->col_header_height();
	//line styles

  //20071128 now by cell rather than column
    Fl_Counter precision(windowW-140,25, 40,15);
    precision.labelsize(12);
    precision.textsize(12);
    precision.label("precision");
	precision.tooltip("sets significant digits for selection");
	precision.align(FL_ALIGN_RIGHT);
	precision.type(FL_SIMPLE_COUNTER);
	precision.range(0,63.);
	precision.precision(0);
	precision.value(PRECISION); //initial
	precision.when(FL_WHEN_RELEASE | FL_WHEN_NOT_CHANGED); //can use displayed precision
	precision.callback(precision_cb, (void*)&precision);

	Fl_Menu_Button mb(20,20 + 2 * crh,windowW-40,windowH-200); //-2 * crh);    //size over table cells only
	mb.type(Fl_Menu_Button::POPUP3);  //type of POPUP doesn't seem to matter
	mb.box(FL_NO_BOX);
  	mb.menu(rightclick); //called by rightbutton
   	mb.callback(menubar_cb);

	ss->showsizes();

    for (int n=1;n<ss->nosheets;n++) ss->save_sheet(n); //need to initialise sheets first time round
    ss->_restart =0;

	ss->set_selection(-1,-1,-1,-1);
	//=======================================================================================
	//for option -p nn start of a program place command here
	switch (n0) {
	//case 0:break;
	case 1:{
				previous_function=810;//allows ALT-z operation immediately
				windowW=1000;
				windowH=900;
				win.size(windowW,windowH);
				ss->size(windowW-40,windowH-100);
				ss->sheetname[0]="pipeflow";
				pipeflow();
				break;
			}
	case 2:{
		previous_function=812;//allows ALT-z operation immediately
		windowW=1300;
		windowH=900;
		win.size(windowW,windowH);
		ss->size(windowW-40,windowH-100);
		ss->sheetname[0]="pipe network";
		pipe_network();
		break;
	}
     case 3: {
		 previous_function=812;//allows ALT-z operation immediately
		// windowW=1300;
		// windowH=800;
		 win.size(windowW,windowH);
		 ss->size(windowW-40,windowH-100);
		 pipe_network();
		 ss->savefilename="/home/norman/spreadsheet/data/test16newloopcolrow.sps.gz";
		 ss->getdata(0);
		// ss->sheet[0] = DEMOSHEET1;
		// cout << "===============================================\n";
		// cout << ss->sheet[0]<<endl;
		// cout << "===============================================\n";
		 break;
	}
	}
	//=======================================================================================
    //win.set_modal(); don't -> help files won't close if this statement included
    win.end();
    win.resizable(*ss);
	win.show();
    return(Fl::run());
}
	catch(...) {cout<< "error found in spreadsheet main\n"; }
}







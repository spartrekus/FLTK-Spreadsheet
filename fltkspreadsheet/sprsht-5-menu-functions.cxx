//sprsht v1.2.2
/*
Copyright (C) 2007 norman a bakker

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
#define 	SELST for (int r= ss->s_top; r< ss->s_bottom+1; r++)	{\
				for (int c= ss->s_left; c< ss->s_right+1; c++)	{
#define		SELEND }}}

void adjust_font(Fl_Font fontchoice) {
SELST	ss->cells[r][c].fontx(fontchoice);
SELEND
void adjust_fontsize(int fontsize) {
SELST	ss->cells[r][c].sizex(fontsize);
SELEND
void adjust_alignment(Fl_Align al) {
SELST	ss->cells[r][c].alignx(al);
SELEND
void adjust_colour(Fl_Color colour) {
SELST	ss->cells[r][c].colourx(colour);
SELEND
void adjust_bgcolour(Fl_Color colour) {
SELST	ss->cells[r][c].bgcolourx(colour);
SELEND
void zerofn() {
SELST	if (ss->cells[r][c].zero()) ss->cells[r][c].zero(0); else ss->cells[r][c].zero(1);
SELEND
void plusfn() {
SELST	if (ss->cells[r][c].plus()) ss->cells[r][c].plus(0); else ss->cells[r][c].plus(1);
SELEND
void negativefn() {
SELST	if (ss->cells[r][c].colournegative()) ss->cells[r][c].colournegative(0); else ss->cells[r][c].colournegative(1);
SELEND
void thousandsfn() {
SELST if (ss->cells[r][c].thousands()) ss->cells[r][c].thousands(0); else ss->cells[r][c].thousands(1);
SELEND
void currencyfn() {
SELST if (ss->cells[r][c].currency()) ss->cells[r][c].currency(0); else ss->cells[r][c].currency(1);
SELEND
void lockfn() {
SELST ss->cells[r][c].lock();
SELEND
void hidefn() {
SELST ss->cells[r][c].hide();
SELEND
void unlockfn() {
SELST ss->cells[r][c].unlock();
SELEND
void unhidefn() {
SELST ss->cells[r][c].unhide();
SELEND
void setdrawingflagfn() {
SELST ss->cells[r][c].set_drawingflag();
SELEND
void cleardrawingflagfn() {
SELST ss->cells[r][c].clear_drawingflag();
SELEND
void setnodrawflagfn() {
SELST ss->cells[r][c].set_cellnodrawflag();
SELEND
void clearnodrawflagfn() {
SELST ss->cells[r][c].clear_cellnodrawflag();
SELEND
void formatfn() {
SELST if (ss->cells[r][c].format()) ss->cells[r][c].format(0); else ss->cells[r][c].format(1);
SELEND
void parsefn() {
SELST if (ss->cells[r][c].parse()) ss->cells[r][c].parse(0); else ss->cells[r][c].parse(1);
SELEND
void digitsfn() {//changes from significant digits shown to no after decimal point
SELST if (ss->cells[r][c].digits()) ss->cells[r][c].digits(0); else ss->cells[r][c].digits(1);
SELEND

void  menubar_cb(Fl_Widget* w, void *data) {
#define SELROWS (int r= ss->s_top; r< ss->s_bottom+1; r++)
#define SELCOLS (int c= ss->s_left; c< ss->s_right+1; c++)
	//get address of current menubar
	Fl_Double_Window * helpwin = new Fl_Double_Window(0,0,550,650); //to show help windows
	Fl_Help_View * help = new Fl_Help_View(0,0,550,620);
	helpwin->resizable(helpwin);
	helpwin->hide();
	int fontchoice = 0;
	int fontsize = FONTSIZE;
	Fl_Color colour = TEXTCOLOR;
	int prec = PRECISION;
	float percentvalue = 100.;
	Fl_Menu_* mw = (Fl_Menu_*)w;
	const Fl_Menu_Item* m = mw->mvalue();
	if (!m)	{ printf("NULL\n"); return; }
	if(!(long) m->user_data()) {  //ALT-z used
		current_function = previous_function + 1; //reserve next case no for continuation without reset
	}
	else {//Alt-z not used
		current_function =(long) m->user_data();
		if (current_function > 800) previous_function = current_function;//restrict continueations to user functions
	}
	// Sheet segment
	if (0<current_function && current_function<17) {	//change current sheet to another max no is 16
		ss->currentsheet = current_function-1; //0 origin
		if (ss->currentsheet!=ss->oldsheet) {
			ss->save_sheet(ss->oldsheet);//NB new version called since 20080321
			ss->oldsheet = ss->currentsheet;
			ss->get_sheet(ss->currentsheet);
			ss->redraw();
		}
		return;
	}
	if (current_function == 17)	{
	//change current sheet name (control max no of characters)
 		string shn = fl_input("change current sheet name ",ss->sheetname[ss->currentsheet].c_str());
		ss->change_sheet_name(shn,ss->currentsheet);
		return;
	}
	if (current_function == 18)	{
	//reset sheet name in the menu to the one showing at the bottom (needed in CLEARALL) - doesn't always work stale pointers
		//for (int n = 0; n< ss->nosheets; n++) {
		ss->change_sheet_name(ss->sheetname[ss->currentsheet],ss->currentsheet);
		//}
		return;
	} 
	cout << "Current function is "<<current_function<<endl;
	switch (current_function) {
		case  0: {ss->set_selection(-1,-1,-1,-1) ; break;}  //deselects on Enter key
		case 20: {ss->savefilename="data/default.sps.gz";ss->getdata(-1); break; }//new
		//case 22: {ss->set_get_file(-2);break;}//open all sheets available
		case 23: {ss->set_get_file(-1); break;}//open all -1
		case 24: {ss->getdata(-1); break;} //reopen all sheets available
		case 25: {ss->compressdata=1; ss->savedata(0); break;}//use to save files saved in newer format
		case 27: {ss->compressdata=0; ss->savedata(0); break;}//save all available sheets uncompressed
		case 28: {ss->compressdata=1; ss->saveas(0); break;}//save all available sheets
		//case 29: {ss->savedatastring(); cout << ss->_saveddatastring <<endl; break;}
		//case 30: {ss->getdatastring(); break;} // cout << ss->_saveddatastring <<endl; break;}
		case 32: {ss->set_get_file(ss->currentsheet); break;}//get the first sheet of another file
		case 34: {ss->getdata(ss->currentsheet);break;}
		case 40: {ss->printdata(); break;}
		case 45: ss->clearallcells(); break;
		case 50: {	ss->compressdata=1; ss->savedata(0); exit(0); break;}//quit command - save current data in compressed format
		//case 50: {ss->compressdata=1; ss->savefilename="data/recent.sps.gz"; ss->savedata(0);exit(0); break;}
		case 60: { //goto a cell nominated as an "x23" string  Alt-G
			int R,C;
			const char* rcin = fl_input("cell address", "x24");
			if (rcin)	{
				if (!ss->parse_cellreference(rcin)) break;
				//get first and second tokens
				R = atoi(ss->rowstr.c_str())-1;
				C = ss->label_to_int(ss->colstr);
				if (R < 0) R = 0;
				if (R > ss->rows()) R = ss->rows()-1;
				ss->row_position(R);
				ss->redraw();					//essential to do this in two steps
				ss->set_selection(R,C,R,C);
				if (C < 0) C = 0;
				if (C > ss->cols()) C = ss->cols()-1;
				ss->col_position(C);
			}
			break;
		}
	/*	case 61: {// set global parse 20070823
			for (int r=0;r<ss->rows();r++)	for (int c=0;c<ss->cols();c++) ss->cells[r][c].parse(1);
			break;
		} */
		case 62: {//clear all view bit flags
			for SELROWS	{
				for SELCOLS {
					if (ss->cells[r][c].get_lockstate()==0) {  //20070823
					ss->cells[r][c].zero(0);
					ss->cells[r][c].plus(0);
					ss->cells[r][c].colournegative(0);
					ss->cells[r][c].thousands(0);
					ss->cells[r][c].currency(0);
					ss->cells[r][c].unlock();
					ss->cells[r][c].unhide();
					}
				}
			} break;
		}
	/*	case 63: {// set global lock  20070823
			for (int r=0;r<ss->rows();r++)	for (int c=0;c<ss->cols();c++) ss->cells[r][c].lock();
			break;
		}
		case 64: {// clear global lock  20070824
			for (int r=0;r<ss->rows();r++)	for (int c=0;c<ss->cols();c++) ss->cells[r][c].unlock();
			break;
		}
		case 65: {// hide all 20071215
			for (int r=0;r<ss->rows();r++)	for (int c=0;c<ss->cols();c++) ss->cells[r][c].hide();
			break;
		}
		case 66: {// unhide all 20071215
			for (int r=0;r<ss->rows();r++)	for (int c=0;c<ss->cols();c++) ss->cells[r][c].unhide();
			break;
		} */
		case 67: {//reset global default formats and row, col sizes
   			ss->row_header(1);
    		ss->row_header_width(COLWIDTH);
   	 		ss->row_resize(1);
    		ss->rows(ROWS);
    		ss->row_height_all(ROWHEIGHT);
		    // COLS
			ss->cols(COLS);
    		ss->col_header(2);
    		ss->col_header_height(ROWHEIGHT);
    		ss->col_resize(1);
    		ss->col_width_all(COLWIDTH);
			for SELROWS {
				for SELCOLS {
					ss->cells[r][c].setdefault();
				}
			}
			break;
		}
		case 68: { //clear all special drawapplications
			//ss->drawing_count=0;
			ss->drawingmap.clear();
			for SELROWS {
				for SELCOLS {
					if (ss->cells[r][c].get_drawingflag()) ss->cells[r][c].clear_drawingflag();
				}
			}
			break;
		}
		 //view scale factor - default is 100(%)
		case 71: percentvalue = 40.; ss->adjust_viewscale(percentvalue);break;
		case 72: percentvalue = 60.; ss->adjust_viewscale(percentvalue);break;
		case 73: percentvalue = 80.; ss->adjust_viewscale(percentvalue);break;
		case 74: percentvalue = 90.;ss->adjust_viewscale(percentvalue);break;
		case 75: ss->reset_viewscale(); break; //100% resets to defaults
		case 76: percentvalue = 110.;ss->adjust_viewscale(percentvalue);break;
		case 77: percentvalue = 120.;ss->adjust_viewscale(percentvalue);break;
		case 78: percentvalue = 140.;ss->adjust_viewscale(percentvalue);break;
		case 79: {
			const char *vs = fl_input("view size pc", "100");
			if (vs) percentvalue = (float) atoi(vs);
			ss->adjust_viewscale(percentvalue);
			break;
		}
		case 80: ss->reset_viewscale(); break;
		case  105:		// cut and save selection in resized ss clipboard
		case  110:	{	// copy and save selection in resized ss clipboard
			ss->clipboard.sdrrowscols(ss->s_bottom - ss->s_top  + 1,
			ss->s_right  - ss->s_left + 1);
			//if (ss->coutflag) cout << "clipboard size is " << ss->clipboard.sdrrows() <<
			//		", " << ss->clipboard.sdrcols() << endl;
			ss->copy_s_top   =ss->s_top; //needed for pasting relative cell references
			ss->copy_s_bottom=ss->s_bottom;
			ss->copy_s_left  =ss->s_left;
			ss->copy_s_right =ss->s_right;
			int rr =0;
			for SELROWS	{
				int cc = 0;
				for SELCOLS	{
					ss->clipboard[rr][cc] = ss->cells[r][c];
					if (current_function!=110 && ss->cells[r][c].get_lockstate()==0) ss->cells[r][c].clearcell_value(); //20070823
					cc++;
				}
				rr++;
			}
			if (current_function!=110) ss->clear_inputvalue();   //clear value from input buffer otherwise it bounces back
			break;
		}
		case 112:	{// paste relative cell references : equations only
			int rowadj = ss->s_top-ss->copy_s_top; //adjustments from copy position
			int coladj = ss->s_left-ss->copy_s_left; // to paste position
			string start="", temp;
			string next;
			int notoks=0;
			if (ss->clipboard[0][0].cval()[0]=='=') {
				// cells to paste will be in clipboard so work on its contents
				// and then complete paste
				for (int r=0;r<ss->clipboard.sdrrows();r++) {
					for (int c=0;c<ss->clipboard.sdrcols();c++) {
						start = ss->clipboard[r][c].cval();
						next ="";
						temp = ss->formula.delimiters;//save tokenise delimiters
						ss->formula.delimiters=temp+":,;";//add these temporarily as may be mulitiple args or range spec
						notoks = ss->formula.tokenise(start); //also tokenise on : , ;
						ss->formula.delimiters=temp;//restore normal delimiters
						for (int i=0;i<notoks;i++) {
							if (ss->parse_cellreference(ss->formula.token[i].val))
								{next=next+ss->relcellseq(ss->formula.token[i].val,rowadj,coladj);}
							//cout <<next<<endl; }
							else next = next + ss->formula.token[i].val;
						}
						ss->clipboard[r][c].cval(next);
					}
				}
				// now paste at new location
			int rr = 0;
			for (int r= ss->s_top; r < ss->s_top + ss->clipboard.sdrrows(); r++){
				int cc = 0;
				for (int c= ss->s_left; c < ss->s_left + ss->clipboard.sdrcols(); c++){
					ss->cells[r][c] = ss->clipboard[rr][cc];  //pastes all
					cc++;
				}
				rr++;
			}
			ss->set_inputvalue(ss->clipboard[0][0].cval());  //reset input buffer
			}
			break;
		}
		case 115:  						// paste values from clipboard at cursor position
		case 120:	{					// paste all including formats
			int rr = 0;
			for (int r= ss->s_top; r < ss->s_top + ss->clipboard.sdrrows(); r++) {
				int cc = 0;
				for (int c= ss->s_left; c < ss->s_left + ss->clipboard.sdrcols(); c++)	{
					if (ss->cells[r][c].get_lockstate()==0) { //20070823
					if (current_function==115) 	ss->cells[r][c].cval(ss->clipboard[rr][cc].cval());  //pastes values
					else ss->cells[r][c] = ss->clipboard[rr][cc];  //pastes all
					}
					cc++;
				}
				rr++;
			}
			ss->set_inputvalue(ss->clipboard[0][0].cval());  //reset input buffer
			break;
		}
		case 121: { //undo function 20070826
			if (!ss->undoqueue.empty()) {
				string tmp = ss->undoqueue.front();
				ss->set_inputvalue(tmp); //see also event_callback2 in 2a
				ss->undoqueue.pop_front();
				ss->undoqueue.push_back(tmp); //rotate to back
			} break;
		}
		case 122: { //redo function 20070826
			if (!ss->undoqueue.empty()) {
				string tmp = ss->undoqueue.back();
				ss->set_inputvalue(tmp); //see also event_callback2 in 2a
				ss->undoqueue.pop_back();
				ss->undoqueue.push_front(tmp);  //rotate to front
			} break;
		}
		case 128:  {//toggle console output flag
			ss->coutflag = ! ss->coutflag;
			break;
		}
		case  130:	{					//insert no of rows depending on selection
			ss->lacell();
			int oldrows = ss->rows();
			int extrarows = ss->s_bottom - ss->s_top +1;
			int freerows = ss->rows() - ss->larow -1;
			if ((freerows - extrarows) < 0)  {		//see if need more cells
				ss->rows(oldrows + extrarows - freerows); 			//update rows()
				ss->cells.sdrrows(oldrows + extrarows -freerows);
				cout << "resizing spreadsheet by adding "<< extrarows - freerows << "additional rows" <<endl;
				//add extra rows - won't overwrite existing		}
			}
			for (int r = ss->larow ; r >= ss->s_top; r--) {  //move existing data down
				for (int c = 0; c < ss->lacol+1; c++)	{
					ss->cells[r+extrarows][c] = ss->cells[r][c];
					ss->cells[r][c].clearcell_value();
					ss->cells[r][c].setdefault();
				}
			}
			ss->clear_inputvalue();
			break;
		}
		case  131:	{					//insert no of cols depending on selection
			ss->lacell();
			int oldcols = ss->cols();
			int extracols = ss->s_right - ss->s_left +1;
			int freecols  = ss->cols() - ss->lacol - 1;
			//see if need more cells
			if ((freecols-extracols) < 0) {
				// save old values
				ss->clipboard.sdrrowscols(1+ss->larow,1+ss->lacol);
				for (int r= 0; r< ss->larow+1; r++)	{
					for (int c= 0; c< ss->lacol+1; c++)	{
						ss->clipboard[r][c] = ss->cells[r][c];
						ss->cells[r][c].clearcell_value();
					}
				}
				ss->cols(oldcols + extracols - freecols); 					//update cols()
				ss->cells.sdrcols(oldcols + extracols -freecols);			//add extra
				cout << "resizing spreadsheet by adding "<< extracols - freecols << "additional columns" <<endl;

				//paste old values back leaving spaces in new columns
				int rr = 0;
				for (int r= 0; r < ss->larow+1; r++) {
					int cc = 0;
					for (int c= 0; c < ss->cols(); c++)	{
						if (c==ss->s_left) c = c + extracols  ;
						ss->cells[r][c] = ss->clipboard[r][cc];  //pastes all
						cc++;
					}
					rr++;
				}
			}
			else
			{
				for (int c = ss->lacol ; c >= ss->s_left; c--) {   	//move existing data to right
					for (int r = 0; r < ss->larow+1; r++)	{
						ss->cells[r][c+extracols] = ss->cells[r][c];
						ss->cells[r][c].clearcell_value();
						ss->cells[r][c].setdefault();
					}
				}
			}
			ss->clear_inputvalue();
			break;
		}
		case 125:
		case 134:	{					//delete values and reset formats unless locked 20070823
			for SELROWS	{
				for SELCOLS	{
					if (current_function==134 && ss->cells[r][c].get_lockstate()==0) ss->cells[r][c].cval(""); //clearvalues only
					else {
						if(ss->cells[r][c].get_lockstate()==0) {
						ss->cells[r][c].clearcell_value(); //and reset formats
						ss->cells[r][c].setdefault();
						}
					}
				}
			}
			if(current_function==134) ss->clear_inputvalue(); break; //don't change col widths
			for (int c= ss->s_left; c< ss->s_right+1; c++)	{
				//ss->numprecision[c]=PRECISION;
			}
			ss->row_height_all(ROWHEIGHT);
			ss->col_width_all(COLWIDTH);
			ss->clear_inputvalue(); //clear value from input buffer
			break;
		}
		case 136:	{					//delete no of rows depending on selection -don't resize table
			ss->lacell();
			int no_rows_to_delete = ss->s_bottom - ss->s_top +1;
			for (int r = ss->s_top ; r < ss->larow+1; r++)   {//move existing data up
				for (int c = 0; c < ss->lacol+1; c++)	{
					ss->cells[r][c] = ss->cells[r+no_rows_to_delete][c];
					ss->cells[r+no_rows_to_delete][c].clearcell_value();
				}
			}
			ss->set_inputvalue(ss->cells[ss->s_top][ss->s_left].cval());
			break;
		}
		case  138:	{					//delete no of cols depending on selection-don't resize table
			ss->lacell();
			int no_cols_to_delete = ss->s_right - ss->s_left +1;
			for (int c = ss->s_left ; c < ss->lacol+1; c++)  { 	//move existing data left
				for (int r = 0; r < ss->larow+1; r++) {
					ss->cells[r][c] = ss->cells[r][c+no_cols_to_delete];
					ss->cells[r][c+no_cols_to_delete].clearcell_value();
				}
			}
			ss->set_inputvalue(ss->cells[ss->s_top][ss->s_left].cval());
			break;
		}
		//following toggle bit for  selection
		case 140: parsefn(); break;
		case 141: zerofn(); break; // toggle show zero
		case 142: plusfn();break;
		case 144: negativefn();break;
		case 146: thousandsfn();break;
		case 147: digitsfn();break;
		case 148: currencyfn();break;
		case 149: formatfn();break;
		case 150:  {						//set number of rows
			const char* in = fl_input("rows ", ss->itoa(ss->rows()));
			if (in) {
				int rows = atoi(in);
				if ( rows < 1 ) rows = 1;
				if ( rows * ss->cols() > MAXCELLS) rows = MAXCELLS /ss->cols();
				ss->rows(rows);
				ss->cells.sdrrows(rows);
			}
		break;
		}
		case 151: { 						//set number of cols
			int oldcols = ss->cols();
			const char* in = fl_input("cols ", ss->itoa(ss->cols()));
			if (in) {
				int cols = atoi(in);
				if ( cols < 1 ) cols = 1;
				if ( cols * ss->rows() > MAXCELLS) cols = MAXCELLS / ss->rows();
				ss->cols(cols);
				//ss->colformat.length(cols);
				ss->cells.sdrcols(cols);
				if (cols > oldcols)		//set format for additional columns
				{
					//for (int c=oldcols; c<cols; c++) ss->colformat[c] = ss->bf(WIDTH,PRECISION);
					//LONGLONG def = ss->inputcell.getdefault_v64();
					for (int r=0;r<ss->rows();r++)  { //clean out olddata
						for (int c=0; c<cols; c++)	{
							ss->cells[r][c].cval(""); //ss->itoa(def_i));
							ss->cells[r][c].setdefault();
						}
					}
				}
			}
			break;
		}
		case 152: ss->rowcolformat = 1; break; //save in rows
		case 153: ss->rowcolformat = 0; break; //save in columns
		case 156: { //row height
			const char* val = fl_input("row height ",ss->itoa(ss->row_height(0)));
			if (val) {
				int v = atoi(val);
				if ( v < 0 )  v = 0;
				ss->row_height_all(v);
			}
			break;
		}
		case 157: {
			const char* val = fl_input("col width ",ss->itoa(ss->col_width(0)));
			if (val) {
				int v = atoi(val);
				if ( v < 0 )  val = 0;
				ss->col_width_all(v);
			}
			break;
		}
		case 158: {
			const char* val = fl_input("row header width ",ss->itoa(ss->row_header_width()));
			if (val) {
				int v = atoi(val);
				if (v < 1) v = 1;
				ss->row_header_width(v);
			}
			break;
		}
		case 160: {
			const char* val = fl_input("col headerheight",
					  ss->itoa(ss->col_header_height()));
			if (val) {
				int v = atoi(val);
				if (v < 1) v = 1;
				ss->col_header_height(v);
			}
			break;
		}
		case 162: {
			const char* val = fl_input("width scale factor for auto cell size",
			ss->ftoa(ss->widthscalefactor));
			if (val) {
				int v = atoi(val);
				if (v < 1) v = 1;
				ss->widthscalefactor=atof(val);
			}
			break;
		}
		case 164: {
			const char* val = fl_input("height scale factor for auto cell size",
			ss->ftoa(ss->heightscalefactor));
			if (val) {
				int v = atoi(val);
				if (v < 1) v = 1;
				ss->heightscalefactor=atof(val);
			}
			break;
		}
		case 166: {
			Fl_Color c = ss->row_header_color();
			c = fl_show_colormap(c);
			ss->row_header_color(Fl_Color(c));
			ss->col_header_color(Fl_Color(c));
			ss->hdrcolor = c;
			cout<<"hdr colour no "<<c<<endl;
			break;
		}
		case 168:		{
			Fl_Color c = FL_WHITE;
			c = fl_show_colormap(c);
			ss->hdrtextcolor = c;
			cout<<"hdr text colour no "<<c<<endl;
			break;
		}
		case 170:		{
			Fl_Color c = FL_WHITE;
			c = fl_show_colormap(c);
			ss->localbackgroundcolor = c;
			cout<<"local background colour no "<<c<<endl;
			break;
		}
		case 172:		{
			Fl_Color c = FL_YELLOW;
			c = fl_show_colormap(c);
			ss->selectioncolor = c;
			cout<<"selection colour no "<<c<<endl;
			break;
		}
		case 174: { //default colours
			ss->selectioncolor 			= SELECTIONCOLOR;
			ss->localbackgroundcolor 	= BACKGROUNDCOLOR;
			ss->hdrtextcolor 			= HDRTEXTCOLOR;
			ss->row_header_color(HDRCOLOR);
			ss->hdrcolor = HDRCOLOR;
			ss->col_header_color(HDRCOLOR);
			ss->colourneg = FL_RED;
			break;
		}
		case 176:  {//toggle A-Z column labels
			ss->col_labeltype = ! ss->col_labeltype;
			break;
		}
		case 177: {

			break;
		}
		case 200: {//set default format
	 		for SELROWS {
				for SELCOLS {

					ss->cells[r][c].setdefault();
					//ss->numprecision[c]=PRECISION;
					ss->cells[c][r].precisionx(PRECISION);
					ss->row_height_all(ROWHEIGHT);
				    ss->col_width_all(COLWIDTH);
				}
			}
			break;
		}
		case 206:  	{	// C-style format string
			const char * w = fl_input("format string","%12.4f");
			if (w) {
				string cn;
				string temp = w;
				for SELROWS {
					for SELCOLS {
						ss->cells[r][c].format(0);
						cn = ss->convert_rc_address(r,c);
						ss->cformatmap[cn]= temp;//store cell address as 'b23' style
					}
				}

			}
			break;
		}
		case 207:  	{	// clear C-style format string to empty
			string cn;
			for SELROWS {
				for SELCOLS {
					ss->cells[r][c].format(0);
					cn = ss->convert_rc_address(r,c);
					ss->cformatmap[cn].clear();
					ss->cells[r][c].format(1);
				}
			}
			break;
		}
		case 208: { 			//set column width for selection
			const char* cw = fl_input("col width ", "1");
			if (cw)	{
				for SELCOLS	{
					ss->col_width(c,atoi(cw));
				}
			}
			break;
		}
		case 210: { 			//auto column width takes into account formatting
			for SELCOLS ss->adjustcolumnwidth(c);
			break;
		}
		case 212:  {			//auto row height takes into account formatting
			for SELROWS ss->adjustrowheight(r);
			break;
		}
		case 214:  {			//auto cell size takes into account formatting
			for SELCOLS ss->adjustcolumnwidth(c);
			for SELROWS ss->adjustrowheight(r);
			break;
		}
		case 230: //font type
		case 231:
		case 232:
		case 233:
		case 234:
		case 235:
		case 236:
		case 237:
		case 238:
		case 239:
		case 240:
 		case 241:
		case 242:
		case 243:
		case 244: fontchoice = current_function-230; adjust_font((Fl_Font) fontchoice); break;
		case 245: {  //picks up system fonts between 0 and 255 - a bit of a lucky dip!
		  	const char *fnt = fl_input("font", "0");
			if (fnt) {fontchoice = atoi(fnt);adjust_font((Fl_Font) fontchoice);}
			break;
		}

		case 270 :   //font size
		case 271 :
		case 272 :
		case 273 :
		case 274 :
		case 275 :
		case 276 :
		case 277 :
		case 278 : fontsize = 2 * (current_function-270)+8;adjust_fontsize(fontsize);break;
		case 279 : {
			const char *fs = fl_input("font size", "26");
			if (fs) {fontsize = atoi(fs);adjust_fontsize(fontsize);}
			break;
		}
		//alignment
		case 280 : adjust_alignment(FL_ALIGN_CENTER); break;
		case 281 : adjust_alignment(FL_ALIGN_TOP)	; break;
		case 282 : adjust_alignment(FL_ALIGN_BOTTOM); break;
		case 283 : adjust_alignment(FL_ALIGN_LEFT); break;
		case 284 : adjust_alignment(FL_ALIGN_RIGHT); break;
		case 285 : adjust_alignment(FL_ALIGN_INSIDE); break;
		case 286 : adjust_alignment(FL_ALIGN_TEXT_OVER_IMAGE); break;
		case 287 : adjust_alignment(FL_ALIGN_IMAGE_OVER_TEXT); break;
		case 288 : adjust_alignment(FL_ALIGN_CLIP); break;
		case 289 : adjust_alignment(FL_ALIGN_WRAP); break;
		case 290 : adjust_alignment(Fl_Align (FL_ALIGN_TOP | FL_ALIGN_LEFT)); break;
		case 291 : adjust_alignment(Fl_Align (FL_ALIGN_TOP | FL_ALIGN_RIGHT)); break;
		case 292 : adjust_alignment(Fl_Align (FL_ALIGN_BOTTOM | FL_ALIGN_LEFT)); break;
		case 293 : adjust_alignment(Fl_Align (FL_ALIGN_BOTTOM | FL_ALIGN_RIGHT)); break;
		case 294 : adjust_alignment(Fl_Align (0)); break;

		//text colour
		case 295: colour=fl_show_colormap(colour); adjust_colour(colour);break;
		case 296: {
			const char * col = fl_input("colour no ", "0");
			if (col) {colour = (Fl_Color) atoi(col);adjust_colour(colour);}
			break;
		}
		//background  colour
		case 302: colour=fl_show_colormap(colour); adjust_bgcolour(colour); ss->localbackgroundcolor = colour; break;
		case 303: {
			const char * col = fl_input("colour no ", "0");
			if (col) {colour = (Fl_Color) atoi(col);adjust_bgcolour(colour);}
			break;
		}
		case 304: ss->line_style = 0; break; // solid
		case 305: ss->line_style = 1; break; // dash
		case 306: ss->line_style = 2; break; // dot
		case 307: {
			const char * w = fl_input("enter line width :","1");
			if (w) ss->line_width = atoi(w);
			break;
		}
		case 308: { //line colour
			colour = fl_show_colormap(colour);
			ss->line_colour = colour;
			break;
		}
		case 309: { //fill colour
			colour = fl_show_colormap(colour);
			ss->fill_colour = colour;
			break;
		}
		//precision
		case 310:
		case 311:
		case 312:
		case 313:
		case 314:
		case 315:
		case 316: {
					for SELROWS {
						for SELCOLS {
							//ss->numprecision[c]=2*(current_function-310);
							ss->cells[r][c].precisionx(2*(current_function-310));
						}
					}
					break;
				}
		case 317:{
					const char *pr = fl_input("precision", "14");
					if (pr) {
						prec = atoi(pr);
						//for (int c=ss->s_left; c<ss->s_right+1;c++)ss->numprecision[c]=atoi(pr);}
						for SELROWS {
							for SELCOLS {
								ss->cells[r][c].precisionx(atoi(pr));
							}
						}
						break;
					}
				}
		case 400:  { //enter a user defined variable (8 chars) and its value (string)
			const char* uv = fl_input("uservar , val", "user,0");
			if (uv)
				{
					//get first and second tokens
					string t1,t2;
					while((*uv)!=',') 	{t1=t1+*uv; uv++;} //parse on the spot
					uv++; //skip comma
					while (*uv) 		{t2=t2+*uv;uv++;}
					mstring tm1 = t1.c_str();
					//ss->coutflag << tm1 << ' ' << t2<<endl;
					ss->formula.vmap[tm1]= t2;
				}
				break;
		}
		case 402: lockfn(); break; //lock
		case 403: unlockfn(); break; //unlock
		case 404: hidefn(); break; //hide
		case 405: unhidefn(); break; //unhide
		//case 406: {setbdryflagfn(); lockfn(); break;} //need to lock as well to avoid input cell showing on click
		//case 407: {clearbdryflagfn();unlockfn(); break;}
		case 408: {setnodrawflagfn();lockfn();break;}
		case 409: {clearnodrawflagfn();unlockfn();break;}
		case 411: fl_message("%s",ss->itoa( fl_show_colormap(colour)) );break;
	/*	case 412: { 	string	temp = "boxtypes_and_enumerations";
						system(temp.c_str());
					 	break;
				  }
		case 413: {  	string	temp = "mypixmap_browser";
					 	system(temp.c_str());
					 	break;
				  }
		case 414: {  	string	temp = "mybrowser";//this in binary form in the sprsht folder
						//!!DONT allow any input to temp at this level to avoid code injection
					 	system(temp.c_str());
					 	break;
				  } */
		case 430:  {
			int ch; //show all of input file on stdout
			ifstream in(ss->savefilename.c_str(), ios::in | ios::binary);
			if (!in) { if (ss->coutflag) cout << "cannot open "<<ss->savefilename << endl; break;}
			while(in) { ch = in.get();if (ss->coutflag) cout << (char) ch;}
			break;
		}
		//tools
		case  431: {		//for testing - fill all cells with index //ZAPS everything
			int msg = fl_choice("about to write over all cells","Cancel","OK",0 ); //20070823
			if (msg) {
				Cells *cc = &ss->cells[0][0];
				int n = 1;
				do	{
					cc->cval(ss->itoa(n));
					cc++;
					n++;
				} while (n < ss->rows() * ss->cols());
				ss->set_inputvalue(ss->itoa(1+ ss->s_left + ss->s_top * ss->cols()));
			}
		break;
		}
		case 432: {		//for testing - fill selection with index
			int sn= atoi(ss->cells[ss->s_top][ss->s_left].cval().c_str());
			int inc=atoi(ss->cells[ss->s_top+1][ss->s_left].cval().c_str());
			inc = (inc>0) ? inc : 1;
			int n=0;
			//use value in first cell as starter and one below as increment
			for SELROWS	{
				for SELCOLS	{
					if (ss->cells[r][c].get_lockstate()==0) { //20070823
					ss->cells[r][c].cval(ss->itoa(sn+n));
					ss->cells[r][c].alignx(8);
					n+=inc;}
				}
				ss->set_inputvalue(ss->itoa(sn));
			}
			break;
		}
		case 440:  { //ALT+'s'	// fill selection with a sequence based on cell contents  (Cell references and formulae)
			int sb,st,sl,sr;
			sb=ss->s_bottom;
			st=ss->s_top;
			sl=ss->s_left;
			sr=ss->s_right;
			// direction down or to the right
			if (sl==sr && sb>st) { //selection down a col - find start sequence in first one or first two cells
				ss->fill_down(st,sl);
			}
			if (st==sb && sr>sl) { //selection along a row - find start sequence in first one or first two cells
				ss->fill_right(st,sl);
			}
			break; //excludes selection across a rectangle
		}
		case 442:  { //ALT+'s'	// fill selection with a sequence based on integer NUMBERS in first two cells
			int sb,st,sl,sr;
			sb=ss->s_bottom;
			st=ss->s_top;
			sl=ss->s_left;
			sr=ss->s_right;
			// direction down or to the right
			if (sl==sr && sb>st) { //selection down a col - find start sequence in first one or first two cells
				ss->fill_down_with_numbers(st,sl);
			}
			if (st==sb && sr>sl) { //selection along a row - find start sequence in first one or first two cells
				ss->fill_right_with_numbers(st,sl);
			}
			break; //excludes selection across a rectangle
		}
		case 460: { //area for test fn ALT-z

        break; 
	}

		//case 470: value = fl_show_mymenu(1); cout << value << endl; break;
		// help windows
		case 710: help->load("doc/intro.html");helpwin->show();break;
		case 720: help->load("doc/quickguide.html"); helpwin->show();break;
		case 730: help->load("doc/file.html"); helpwin->show();break;
		case 740: help->load("doc/edit.html");helpwin->show(); break;
		case 745: help->load("doc/view.html");helpwin->show(); break;
		case 750: help->load("doc/format.html");helpwin->show(); break;
		case 760: help->load("doc/tools.html");helpwin->show(); break;
		case 765: help->load("doc/inserts.html");helpwin->show();break;
		case 770: help->load("doc/rightmouseclick.html");helpwin->show(); break;
		case 799: help->load("doc/about.html");helpwin->show(); break;

		case 800: {// SAMPLE PROGRAM home loan calculator-determines fixed periodic amount to pay of a loan
			ss->_restart = 1;	
			homeloan();//0 denotes first time
			break;
		}
		case 801: ss->_restart = 0; homeloan(); break;
		case 807:{
			const char* uv = fl_input("change current name");
			if (uv) {
				string temp = "";
				temp=uv;
				ss->temptest.assign(temp,0,8);
				}

		break;


		}
		case 810: ss->_restart = 1; pipeflow();  break;
		case 811: ss->_restart = 0; pipeflow();  break;
		case 812: ss->_restart = 1; pipe_network(); break; //restart corresponds to 1
		case 813: ss->_restart = 0; pipe_network(); break;

		case 803: break;
		case 804: break;
		case 805: break;
		case 806: break;

	 	case SHOWALL: //display any hidden specials on sheet - trawl through map entries within selection
			ss->display_all_multimapentries(1); break;
	 	case LISTALL: ss->show_all_multimapentries(); break;	//show multimap entries
	 	case SHOWCOMMENTS: //show any hidden specials on sheet - trawl through map entries within selection
			ss->display_all_comments(1); break;
	 	case HIDEALL: ss->display_all_multimapentries(0); break;
	 	case HIDECOMMENTS: //drawings,mergings etc within selection
			ss->display_all_comments(0);break;
	 	case DELETEALL:  //delete all specials in selected area
			ss->delete_all_multimapentries(); break;
		case DELETECOMMENTS: { //delete all comments in selected area
			ss->delete_all_comments(); break;
		}
		case EDITMAP:
			fl_message("Can show or edit map entries\nto delete an entry set number of rows for that entry to 0");
			ss->edit_move_multimapentries(0); break;
		case SHOWMAP:  ss->edit_move_multimapentries(1); break;
	}

	int cf = current_function;
	if (TYPESSTART < cf && cf < TYPESEND ) { //for drawing structure items
	//merge text in selected cells or if no text found, enter some on prompting
	//ditto for pixmap (title)
		string temp="";
		ss->cells[ss->s_top][ss->s_left].set_drawingflag();
		short norows = 1 + ss->s_bottom -ss->s_top;
		short nocols = 1 + ss->s_right  -ss->s_left;
		short box_type = FL_FLAT_BOX;
		short fill_colour = ss->localbackgroundcolor;
		short text_align = FL_ALIGN_WRAP;
		short text_size = FONTSIZE;
		short text_font = FONT;
		short text_colour = TEXTCOLOR;
		if ( cf == COMMENT ) {
			box_type = FL_RSHADOW_BOX;
			fl_color(250,255,191);
			fill_colour = fl_color();
		}
		if (cf == MERGE || cf == PIXMAP || cf == PICTURE) {

			if (cf == MERGE || cf == PIXMAP) { //merge text for merge and pixmap
				for SELROWS {
					for SELCOLS {
						if (!ss->cells[r][c].empty())	temp += ss->cells[r][c].cval() + ' ';
					}
				}
			}
		}
		unsigned j = 0;
		if (cf == MERGE || cf == PIXMAP || cf == PICTURE || cf == COMMENT) {
			//if no text found, read some in
			while	( isspace(temp[j]) && j < temp.size() ) j++;
			if ( j == temp.size() ) {
				const char * w = fl_input("enter text","");
				if (w) temp = w;
			}
		}
		if (cf == HORIZLINE || cf == VERTLINE || cf == RECTANGLE) {
			//draw a horizontal line along the bottom boundary/vertical line along rhs of selected cells
			//draw a rectangle around selected cells
		}

		ss->fill_drawingmap(1, ss->s_top,ss->s_left,norows,nocols, cf, temp, box_type,0,1,
		fill_colour,text_align,text_size,text_font,text_colour);	//leading 1 is to make this entry active
		//ss->drawing_count++;
	}
	ss->redraw();
}


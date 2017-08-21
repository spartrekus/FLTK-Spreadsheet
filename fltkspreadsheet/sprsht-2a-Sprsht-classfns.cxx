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

// Callback whenever someone left clicks on different parts of the table
void Spreadsheet::event_callback(Fl_Widget*, void *data)
{
	Spreadsheet *o = (Spreadsheet*)data;
	o->event_callback2();
}
/*
void color_cb(Fl_Widget* button, void* v) {
  Fl_Color c;
  switch ((long)v) {
  case 0: c = FL_BACKGROUND2_COLOR; break;
  case 1: c = FL_SELECTION_COLOR; break;
  default: c = FL_FOREGROUND_COLOR; break;
  }
  uchar r,g,b; Fl::get_color(c, r,g,b);
  if (fl_color_chooser(0,r,g,b)) {
    Fl::set_color(c,r,g,b); Fl::redraw();
    button->labelcolor(fl_contrast(FL_BLACK,c));
    button->redraw();
  }
} */

void Spreadsheet::event_callback2()
{
	Rcurrent = callback_row();  //this only shows leftclick button as rightclick is used by menutables
	Ccurrent = callback_col(); //shows row and column where of last leftclick
	TableContext context = callback_context();
	//from here 20071209
	//cout<<sheetname[currentsheet].c_str()<<endl;
	//if (!cells[Rcurrent][Ccurrent].empty()) temp =(cells[Rcurrent][Ccurrent].cval());
	//printf("'%s' callback: ", (label() ? label() : "?"));
	//printf("Row=%d Col=%d Context=%d EventButton=%d InteractiveResize? %d value %s\n",
	//	   Rcurrent, Ccurrent, (int)context, (int)Fl::event_button(), (int)is_interactive_resize(), dummycell.cval().c_str());
	switch ( context ) {
	case CONTEXT_CELL: {
	if (input->visible()) input->do_callback();
	row_edit = Rcurrent;
	col_edit = Ccurrent;
	int XX,YY,WW,HH;
	find_cell(CONTEXT_CELL, Rcurrent, Ccurrent, XX, YY, WW, HH);
	input->resize(XX,YY,WW,HH);
	input->box(FL_THIN_UP_BOX);
	input->align(cells[Rcurrent][Ccurrent].alignx());	//use same format as current contents
	input->textcolor(cells[Rcurrent][Ccurrent].colourx());

	input->textfont(cells[Rcurrent][Ccurrent].fontx());
	input->textsize(cells[Rcurrent][Ccurrent].sizex());

	//show number for editing in reduced precision -careful as this changes precision of data
	/*	double nmb = atof(cells[Rcurrent][Ccurrent].cval().c_str());
static char buffer1[24];
if (nmb) {
//cout<<cells[Rcurrent][Ccurrent].cval().c_str()<<endl;
	bf(0,cells[Rcurrent][Ccurrent].precisionx());//place format string in buffer
	snprintf(buffer1,23,buffer,nmb);
	input->value(buffer1);
	//cout<<buffer<<endl;
	}
else  */
	input->value(cells[Rcurrent][Ccurrent].cval().c_str());
	//shows existing value to enable editing
    if (!cells[Rcurrent][Ccurrent].empty()) {//save any non-empty input for undo
		undoqueue.push_front(cells[Rcurrent][Ccurrent].cval());
		if (undoqueue.size()>(unsigned)UNDOLEVEL) undoqueue.pop_back();//keep size at UNDOLEVEL but accept new entry
	}
	if (cells[Rcurrent][Ccurrent].get_lockstate()==0) input->show();
	end();
	//set_selection(-1,-1,-1,-1);//avoids focus on edited cell after Enter key
	input->take_focus(); //ensures focus moves to next cell clicked
	//==========================================================
	//do some parser cleaning : eliminate all temp variables
	//in case of changed content
	//triggered when clicking on any spreadsheet cell with content and =
	//leftkeypressed = 1;
	//cout<<"LKP" <<leftkeypressed<<endl;
	if (!cells[Rcurrent][Ccurrent].empty() && (cells[Rcurrent][Ccurrent].cval())[0]=='=') {
	formula.tmap.clear();
	textcolor(BRIGHT,YELLOW,BLACK);
	cout <<"erasing temp variables\n";
	textcolor();
	}
	}
	default: return;
	}
}

// Handle drawing all cells in table ..called by Fl_Table
void Spreadsheet::draw_cell(TableContext context,int R, int C, int X, int Y, int W, int H) {
// handle input here by checking first what the value string contains
	int X0,Y0,X1,Y1,w,h;
	switch ( context ) {
	case CONTEXT_STARTPAGE:
	// Get kb nav + mouse 'selection region' for use below
		get_selection(s_top, s_left, s_bottom, s_right);
	//printf("selection r%d, c%d ; r%d, c%d \n", s_top+1, s_left+1, s_bottom+1, s_right+1);  //show
		break;

	case CONTEXT_COL_HEADER:
		fl_push_clip(X, Y, W, H); {
			fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, col_header_color());
			fl_color(hdrtextcolor);
			fl_font(FONT,FONTSIZE);
			if (!col_labeltype) sprintf(_buffer,"%d", C);//prints in 0 origin; also for rows below
			else snprintf(_buffer,BUFFER-1,"%s",int_to_col_label(C).c_str());
			fl_draw(_buffer, X, Y, W, H, (Fl_Align) FL_ALIGN_CENTER); //_buffer size = BUFFER
		}
		fl_pop_clip();
		return;
	case CONTEXT_ROW_HEADER:
		fl_push_clip(X, Y, W, H); {
			fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, row_header_color());
			fl_color(hdrtextcolor);
			fl_font(FONT,FONTSIZE);
			if (!col_labeltype) snprintf(_buffer,BUFFER-1,"%d", R);
			else snprintf(_buffer,BUFFER-1,"%d", R+1);
			fl_draw(_buffer, X, Y, W, H, (Fl_Align) FL_ALIGN_CENTER);
		} 
		fl_pop_clip();
		return;
	case CONTEXT_CELL:  {//only acts on visible cells
		if (R == row_edit && C == col_edit && input->visible()) return;
		fl_push_clip(X, Y, W, H);
		//===========================
		//the following processes cell contents to determine display format
		// and any need to evaluate a formula
		// only rhs and bottom cell bdrys need to be drawn
		displayedvalue(R,C); //places formatted data in _buffer
		//===========================
		// Keyboard nav and mouse selection highlighting-reverse colors
		Cells cc = cells[R][C];
		if (cc.get_cellnodrawflag()) { //this flag not used at the moment
			fl_pop_clip(); return;
		}
		if (R >= s_top && R <= s_bottom && C >= s_left && C <= s_right && !cc.get_lockstate() ) { // selection draw
			 //  removed  && !cc.get_drawingflag()
			fl_draw_box(FL_FLAT_BOX, X, Y, W, H, selectioncolor); 
			fl_color(FL_GRAY_RAMP);
			fl_line_style(FL_DOT,0);
			fl_line(X+W-1,Y+1,X+W-1,Y+H-1); //rhs vertical boundary
			fl_line(X,Y+H-1,X+W,Y+H-1); //bottom horizontal boundary
			fl_font(cc.fontx(),cc.sizex());
			fl_color(cc.colourx());
			fl_draw(_buffer, X, Y, W, H,(Fl_Align) cc.alignx());
		}
		else {//normal draw
			fl_draw_box(FL_FLAT_BOX, X, Y, W, H, cc.bgcolourx());
			fl_color(FL_GRAY_RAMP);
			fl_line_style(FL_DOT,0);
			fl_line(X+W-1,Y+1,X+W-1,Y+H-1);//rhs vertical boundary
			fl_line(X,Y+H-1,X+W,Y+H-1);//bottom horizontal boundary
			fl_font( cc.fontx(), cc.sizex() );
		//20071218
			if (cc.get_isnegativestate() && cc.colournegative()) fl_color(FL_RED); else fl_color(cc.colourx());
			if (!cc.get_hidestate()) fl_draw(_buffer, X, Y, W-2, H, (Fl_Align) cc.alignx());
			//20070809 add -2 to prevent
			//text too close to the rh cell border
			//-varies with fontsize
			//hiding best done here as selection will show contents anyway
		}
		fl_pop_clip();
		return;
	}
	case CONTEXT_TABLE: {
		fprintf(stderr, "TABLE CONTEXT CALLED\n");
		return;
	}
	case CONTEXT_RC_RESIZE:	{
		if (!input->visible()) return;
		find_cell(CONTEXT_TABLE, row_edit, col_edit, X, Y, W, H);
		if (X==input->x() && Y==input->y() && W==input->w() && H==input->h()) return;
		input->resize(X,Y,W,H);
		return;
	}
	case CONTEXT_ENDPAGE: {
	//====================================
		//refresh sheet name here
		//get visible cells

		string content =  sheetname[currentsheet];
		get_bounds(CONTEXT_TABLE,X0,Y0,w,h);
		Y0 += h - SHEETNAMEBOXHEIGHT;
		X0 = w - content.size();
		w = content.size();
		fl_draw_box(FL_FLAT_BOX, X0, Y0, w , SHEETNAMEBOXHEIGHT, localbackgroundcolor); //fl_rgb_color(237,237,237));
		fl_color(FL_BLUE);
		fl_font(SHEETNAMEFONT,SHEETNAMEFONTSIZE);
		fl_draw(content.c_str(), X0 , Y0, w, SHEETNAMEBOXHEIGHT, (Fl_Align) FL_ALIGN_RIGHT);

	//===================================
		//check any special draw 

		multimap <mstring,drawing>::iterator z;
		string adr,eadr;
		Fl_Pixmap *pixmap = new Fl_Pixmap (edwin_xpm);//pixmap->desaturate(); //converts to greyscale

		z = drawingmap.begin(); //trawl through drawing map for entries in region bounded by toprow,botrow,leftcol,rightcol
		while (z != drawingmap.end()) {

			if (z->second.active == 0) { z++; continue; }

 			int rs, cs, nr, nc;
			if (!parse_cellreference(z->first)) { cout <<"error in drawing startaddress : "<<z->first<<'\n'; return;}
			rs = atoi(rowstr.c_str())-1;
			cs =  label_to_int(colstr);
			//restrict search to visible cells only : toprow, botrow, leftcol, rightcol from fl_table
			if (botrow < rs || rs < toprow || rightcol < cs || cs < leftcol) { z++; continue; }
			nr = z->second.norows; 
			nc = z->second.nocols; 
			find_cell(CONTEXT_CELL, rs, cs, X0, Y0, w, h);//this is in 0 origin
			find_cell(CONTEXT_CELL, rs+nr , cs+nc, X1, Y1, w, h);//find adjacent cell (to give true bnds)
			int typ = z->second.type;

			fl_color(FL_GRAY_RAMP); //reset to normal drawing type
			fl_line_style(FL_DOT,0);
			
			switch (typ) {
				case FLOATINGCOMMENT: { //no line connecting to anchor cell is provided
					fl_color( (Fl_Color) z->second.fillcolour);
					//read boxtype and size from boxtype and norows,nocols
					fl_draw_box( (Fl_Boxtype) z->second.boxtype, X0+20, Y0-20, 80*z->second.norows, 40*z->second.nocols, fl_color());
					fl_font(z->second.textfont,z->second.textsize);
					fl_color( (Fl_Color) z->second.textcolour);
					fl_draw(z->second.dataholder.c_str(),X0+20, Y0-20, 80*z->second.norows,
							40*z->second.nocols,(Fl_Align) z->second.textalign);					
					break;
				}
				case COMMENT: { //anchored comment
					fl_color( (Fl_Color) z->second.fillcolour);
					//read boxtype and size from boxtype and norows,nocols
					fl_draw_box( (Fl_Boxtype) z->second.boxtype, X0+180, Y0-20, 80*z->second.norows, 40*z->second.nocols, fl_color());
					fl_line( X0, Y0, X0+180, Y0-20);
					fl_font(z->second.textfont,z->second.textsize-4);
					fl_color( (Fl_Color) z->second.textcolour);
					fl_draw(z->second.dataholder.c_str(),X0+180, Y0-20, 80*z->second.norows,
						 40*z->second.nocols,(Fl_Align) z->second.textalign);
					break;
				}
				case MERGE:
				case PIXMAP:
				case PICTURE: {
					fl_color( (Fl_Color) z->second.fillcolour);//fill colour
					fl_draw_box( (Fl_Boxtype) z->second.boxtype, X0, Y0, X1-X0, Y1-Y0, fl_color());
					fl_font(z->second.textfont,z->second.textsize);
					fl_color( (Fl_Color) z->second.textcolour);
					fl_line( X0, Y1-1, X1, Y1-1);// replace bottom line
					fl_line( X1-1, Y0-1, X1-1, Y1-1);//replace right side line
					switch (typ) {
					case MERGE:
						fl_draw(z->second.dataholder.c_str(),X0, Y0, X1-X0, Y1-Y0,(Fl_Align) z->second.textalign);
						break;
						case PIXMAP:
							fl_draw(z->second.dataholder.c_str(),X0, Y0, X1-X0, Y1-Y0,(Fl_Align) z->second.textalign, pixmap);
							break;
						case PICTURE:
							break;
					}
					break;
				}
				case HORIZLINE:
				case VERTLINE:
				case RECTANGLE: {
					w = z->second.linewidth;
					fl_color( (Fl_Color) z->second.linecolour);
					fl_line_style( z->second.linestyle, w);
					w /= 2; //average for placement calculation
					switch (typ) {
						case HORIZLINE: //draw a line along the bottom boundary of selected cells
							fl_line( X0-1, Y1-w-1, X1, Y1-w-1);//bottomfl_line( X0, Y1-1, X1, Y1-1)
							break;
						case VERTLINE:
							fl_line( X1-w-1, Y0, X1-w-1, Y1);//bottomfl_line( X0, Y1-1, X1, Y1-1)
							break;
						case RECTANGLE: //draw a rectangle around selected cells to fit exactly inside outer bndries
							fl_loop(X0+w-1, Y0+w, X1-w-1, Y0+w, X1-w-1, Y1-w, X0+w-1, Y1-w); //faster?
							break;
					}
					break;
				}
			}
			z++; //get next entry if any

		} //while end
		return;
		} //endpage end
	case CONTEXT_NONE: return;
	}
}

long Spreadsheet::label_to_int(string &label)
{
	//convert a column label to an integer value (max three letters- upper or lower case)
	//limit to zzz - returns 0 origin label
	long val=0;
	switch (label.size())
	{
		case 1: val =  toupper(label[0]) - 'A'; break;
		case 2: val =  (1 + toupper(label[0]) -'A')*26+toupper(label[1]) - 'A'; break;
		case 3: val =  (1 + toupper(label[0]) -'A')*26*26+(1 + toupper(label[1]) -'A')*26 +toupper(label[2])-'A';break;
		default:val = 0;
	}
		if (val < 0) return 0;
		else if (val > cols()) return cols();
		else return val;
}
long Spreadsheet::label_to_int(const char * label)
{
	//convert a column label to an integer value (max three letters- upper or lower case)
	//limit to zzz - returns 0 origin label
	long val=0;
	switch (strlen(label))
	{
		case 1: val =  toupper(label[0]) - 'A'; break;
		case 2: val =  (1 + toupper(label[0]) -'A')*26+toupper(label[1]) - 'A'; break;
		case 3: val =  (1 + toupper(label[0]) -'A')*26*26+(1 + toupper(label[1]) -'A')*26 +toupper(label[2])-'A';break;
		default:val = 0;
	}
		if (val < 0) return 0;
		else if (val > cols()) return cols();
		else return val;
}
string Spreadsheet::int_to_col_label(long n, char startletter)
{	//t is either 'A' (default) or 'a'
	// convert an integer no up to 26*26*26 + 26*26 + 26 = 18278 (ZZZ) into abc type column headings
	// after that leave as integer number
	char st[4] = "";
	if (n < 26) { st[0] = n + startletter; return st;}
	if (26 <= n && n < 26*26 + 26)
	{
		st[0] =  n / 26  + startletter -1;
		st[1] = (n % 26) + startletter;
		return st;
	}
	if (702 <= n && n < 26*26*26 + 26*26  )	//seems to lose accuracy >18,252 ZYZ
	{
		st[0] = n / (26*26) + startletter - 1;
		n 	  = n % (26*26 ) ;
		st[1] = n /  26 + startletter - 1;
		st[2] = n %  26 + startletter;
		return st;
	}
	if (26*26*26 + 26*26 <= n && n < 26*26*26 +26*26 +26)  //fudge
	{
		//printf("n> %d\n", (int) n);
		st[0] = 'Z';
		st[1] = 'Z';
		st[2] = n - (26*26*26 +26*26) +startletter;
		return st;
	}
	return itoa(n);		//leave as integer
}
string Spreadsheet::getlocaltime()
{
	time_t      now;				//based on ex in wikipedia 20061101
	struct tm   ts;
	string currenttime;
	char buf[BUFFER];
	time(&now);						//get the current time and place it in global variable currenttime
	ts = *localtime(&now);			//format and print the time, "ddd yyyy-mm-dd hh:mm:ss zzz"
	strftime(buf, sizeof(buf), "%a %Y %m %d %H:%M:%S%Z", &ts);
	return  currenttime = buf;
}
string Spreadsheet::evaluate(string &s, int row, int col) {
	cout << "times evaluate called ";
	textcolor(BRIGHT,RED,BLACK);
	cout << eval_counter++ << endl;
	textcolor();
	//formula on spreadsheet starts with an '=' token ex "=a=12.3"
	//start by replacing any formulae with their evaluted numbers
	//check whether any defined fns and evaluate them first
 	//first check which type we are dealing with
	int stringflag=0;
	unsigned i=0;
	unsigned l=s.size();
	//determine whether a string or numeric fn
	while (i<l) {
		 if (isupper(s[i]) || s[i]=='"') { //string - first occurrence determines
			 stringflag=1; break;
		 }
		 if (islower(s[i])) {//numeric
			 stringflag=0; break;
		 }
		 else i++; //bypass any junk
	}
	//trap error if expression incomplete
	while (s[l-1]==' ') l--; //first eliminate any trailing spaces !!!
    if (strchr("+-*/%^",s[l-1])) return "expr incomplete"; //one of these delims cant be in last position
	//save the result in the current cell
	stringstream ststr;
	//=====================
	if (stringflag!=1) {
		formula.evalfns(s.erase(0,1)); 	//order of these two not to be changed
		//if (errorflag) { return temp = "error";}
		//cout << "evalexp is sent "<< s <<endl;
		temp = formula.evalexp(s,row,col);
		ststr <<int_to_col_label(col,'a') <<row+1<<ends; //+1 because of row 0 = 1
	}
	else {
		sformula.sevalfns(s.erase(0,1));
		//if (errorflag) {return temp = "error";}
		temp = sformula.sevalexp(s);
		ststr << int_to_col_label(col,'A') <<row+1<<ends;
	}
	//=====================
	string cn  = ststr.str();
    //save in temp variables
	formula.tmap[cn]=temp; //save in temp variables
	//cout.precision(numprecision[col]);
	//cout << cn<< " not defined, created temp var "<<cn<<" containing "<< temp<<" with prec "<< numprecision[col]<<endl;
	return temp; //}
}
void Spreadsheet::textcolor(int attr, int fg, int bg) {
	//changes colors of console output
	//Pradeep Padala on Tue, 2005-10-11 01:00.  www.linuxjournal.com/article/8603
	//example  	textcolor(BRIGHT,RED,BLACK); followed by textcolor() to reset
	char command[13];
    //command is the control command to the terminal
   	sprintf(command, "%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
   	printf("%s", command);
}
void Spreadsheet::defaultview() { //resets default format and view for all cells
	for (int r=0;r<rows();r++) for (int c=0;c<cols();c++) {
		cells[r][c].setdefault();
	}
}

void Spreadsheet::newformat(int &r,int &c, string &number) {
	//called by displayedvalue
	//formats a no string with opt currency, thousands and decimalpt marks
	//don't do anything if in scientific notation
	stringstream strstr;
	if (number.find('e')!=string::npos || number.find('E')!=string::npos) {
		strncpy(_buffer,number.c_str(),BUFFER-1);
		return;
	}
	double num;
	if (!cells[r][c].format()) {
    	//format flag is zero; check whether there is a C format for this cell
		map<mstring,string>::iterator p;
		strstr <<int_to_col_label(c,'a') <<r+1<<ends;
		string cn = strstr.str();//create cell address in 'd26' style
		p = cformatmap.find(cn);
		if (p!=cformatmap.end() && p->second!="") {
			num = atof(number.c_str());
			snprintf(_buffer,BUFFER-1,(p->second).c_str(),num);
			return;
		}
		else { //no formatting at all
			strncpy(_buffer,number.c_str(),BUFFER-1);
		return;
		}
	}
   	//struct lconv * lc = localeconv();   // in Spreadsheet constructor
	string ps="", dummy="";
	num =  atof(number.c_str());
	if (cells[r][c].thousands()) strstr.imbue(locale(""));//ensures thousands separator is used
	if (cells[r][c].plus()) {ps="+";}
	//check for negative
	if (num<0) {cells[r][c].set_isnegative();ps="";} //this flags to output routine that a negative number is present
	else cells[r][c].clear_isnegative();
	//20071218 correction to ensure unique cell colour can be maintained after a change from negative
	if (cells[r][c].digits()) { //precision here is no of significant digits
		strstr.precision(cells[r][c].precisionx()+4); //+4 added to avoid scientific notation for nos >xxxxx
		//if (!cells[r][c].currency()) {
		strstr <<  num; // 20080311 : // << ends;
		dummy = ps+strstr.str();
	}
	else { //where precision is interpreted as no of digits after decimal point
		//strstr << fixed << showpoint <<setprecision(numprecision[c]) << num<<ends;
		strstr << fixed << showpoint <<setprecision(cells[r][c].precisionx()) << num; //<<ends;
		dummy = strstr.str();
		if (cells[r][c].currency())  {dummy = " " +  ps + dummy; dummy = currencymark + dummy; }
		//dummy = (*lc->currency_symbol) + " " +  ps + dummy;
		else dummy = ps + dummy;
	}
	strncpy(_buffer,dummy.c_str(),BUFFER-1); //place output in _buffer
	return;
}

const char * Spreadsheet::displayedvalue(int &r, int &c) {
// returns the contents of cell r,c as it will be shown after all formatting
// follows former draw_cell segment exactly for formatting str
	string str = cells[r][c].cval();//20071202 reinstated but why commented out??
    //if (!cells[r][c].format())	{snprintf(_buffer,BUFFER-1,"%s",str.c_str());return _buffer;}
		char str0 = str[0];
		if (str0 == '=' && cells[r][c].parse()){ //20070823 replaced parser() with individual parse
			str=evaluate(str,r,c);//go parse formula
		}
		double vtemp = atof(str.c_str()); //try if a number
		if (vtemp || str0=='0') {
		//if str evaluates to a number or a number which is zero
			if (vtemp == 0 && !cells[r][c].zero()) str=" "; //controls display of any zeros
			else newformat(r, c,str);
		}
		else snprintf(_buffer,BUFFER-1,"%s",str.c_str());	//print plain old string
	return _buffer;
}

void Spreadsheet::adjustcolumnwidth(int &c) {
	//adjust column width automaticall based on a selected range
	//and formatted values in that range for each column
	lacell(); //update active cell check
	float cw, tr;
	cw = 2.;
	for (int r = 0; r <larow + 1; r++)	{
		tr = fl_width(displayedvalue(r,c));
		tr = tr * ((float)cells[r][c].sizex())/(widthscalefactor-showcurrency); //a fudge set for favourite font, size and formats
		if (tr > cw) cw = tr;
	}
	col_width(c,(int) rint(cw));
	redraw();
	//if (coutflag) cout << "col wdth " << cw<<endl;
}

void Spreadsheet::adjustrowheight(int &r) {
	//adjust row height automaticall based on a selected range
	//and formatted values in that range for each row
	lacell(); //update active cell check
	float tr, rh;
	int w, h;
	rh = 2.;
	for (int c = 0; c <lacol + 1; c++)	{
		fl_measure(displayedvalue(r,c),w,h);
		tr = ((float) h) * ((float) cells[r][c].sizex())/heightscalefactor;
		if (tr > rh) rh = tr;
	}
	row_height(r,(int) rint(rh));
	redraw(); //seems to be needed
	//if (coutflag) cout << "row ht " <<rh <<endl;
}

LONGLONG Spreadsheet::stoll(string &s) {
	//convert a string to a longlong value
	//mstring ms(s);
	//return ms.getindex();
	//if (coutflag) cout << s << endl;
	string lower, upper;
	upper.assign(s,0,8);
	lower.assign(s,8,8);
	union {
		LONGLONG ll;
		int i[2];
	};
	i[0]=atoi(lower.c_str());
	i[1]=atoi(upper.c_str());
	return ll;
}
int Spreadsheet::parse_cellreference (mstring ref) {
//splits cellref into row and col and store in static colstr & rowstr
//NB these references are view based - remember if converting to  0,0 origin with atoi for rows subtract 1
//label_to_int automatically returns 0 origin reference
//returns length if a valid address, 0 otherwise
//check validity first
	string cellref = ref.getname();
	if (!isalpha(cellref[0])) return 0; //must start with alpha
	int length=cellref.size();
	if (!isdigit(cellref[length-1])) return 0;
	int i=0;
	while (i<length) {if (!isalnum(cellref[i])) return 0; else i++;}//chars within a-Z,0-9
	//a candidate
	unsigned j=0;
	while (isalpha(cellref[j])) j++; //split address into col and row components
	colstr = temp.assign(cellref,0,j);
	rowstr = temp.assign(cellref,j,length);
	return length;
}
int Spreadsheet::parse_cellreference (string &cellref) {
//splits cellref into row and col and store in static colstr & rowstr
//NB these references are view based - remember if converting to  0,0 origin with atoi for rows subtract 1
//label_to_int automatically returns 0 origin reference
//returns length if a valid address, 0 otherwise
//check validity first
	if (!isalpha(cellref[0])) return 0; //must start with alpha
	int length=cellref.size();
	if (!isdigit(cellref[length-1])) return 0;
	int i=0;
	while (i<length) {if (!isalnum(cellref[i])) return 0; else i++;}//chars within a-Z,0-9
	//a candidate
	unsigned j=0;
	while (isalpha(cellref[j])) j++; //split address into col and row components
	colstr = temp.assign(cellref,0,j);
	rowstr = temp.assign(cellref,j,length);
	return length;
}
int Spreadsheet::parse_cellreference (const char * cellref) {
//splits cellref into row and col and store in static colstr & rowstr
//NB these references are view based - remember if converting to  0,0 origin with atoi for rows subtract 1
//label_to_int automatically returns 0 origin reference
//returns length if a valid address, 0 otherwise
//check validity first
	if (!isalpha(cellref[0])) return 0; //must start with alpha
	int length=strlen(cellref); //.size();
	if (!isdigit(cellref[length-1]))return 0;
	int i=0;
	while (i<length) {if (!isalnum(cellref[i])) return 0; else i++;}//chars within a-Z,0-9
	//a candidate
	unsigned j=0;
	while (isalpha(cellref[j])) j++; //split address into col and row components
	colstr = temp.assign(cellref,0,j);
	rowstr = temp.assign(cellref,j,length);
	return length;
}
string Spreadsheet::relcellseq(string &cellref, int rowadjust, int coladjust) {
// adjusts celladdress using rowadjust and coladjust for cell refs
	//cout<<cellref<<' '<<parse_cellreference(cellref)<<' '<<colstr<<' '<<rowstr<<endl;
	if (parse_cellreference(cellref)<2) return cellref; //not a cell ref
	int c,r;
	string row="",col="";
	char labelst;
	//by next cell
	c = label_to_int(colstr);
	r = atoi((rowstr.c_str()));
	if (isupper(cellref[0])) labelst='A';
	else labelst='a';
	row = itoa(r+rowadjust);
	col = int_to_col_label(c+coladjust,labelst);
	return col+row;
}
void Spreadsheet::fill_down_with_numbers(int r, int c) { //arithmetic progression
//selection of 1 or more cell refs down a col - find start sequence in first cell
//rowinc defaults to 1 unless there is a number in 2nd row when that is used
	int rowinc=1, rr=0, startval=0, num=0;
	r++; c++; //cell fns are one origin
	if (cells[r-1][c-1].isnumber()) {//number sequence only
		startval= atoi(cell(r,c).c_str());
		num = startval;
		rowinc=atoi(cell(r+1,c).c_str())-startval;
	}
	rr= r;
	while (rr < s_bottom) {
		rr++;
		num += rowinc;
		cell (rr,c,num);}
}
void Spreadsheet::fill_right_with_numbers(int r, int c) { //arithmetic progression
//selection of 1 or more cell refs across a row - find start sequence in first cell
//colinc defaults to 1 unless there is a number in 2nd col when that is used
	int colinc=1, cc=0, startval=0, num=0;
	r++; c++; //cell fns are one origin
	if (cells[r-1][c-1].isnumber()) {//number sequence only
		startval= atoi(cell(r,c).c_str());
		num = startval;
		colinc=atoi(cell(r,c+1).c_str())-startval;
	}
	cc= c;
	while (cc < s_right) {
		cc++;
		num += colinc;
		cell (r,cc,num);}
}

void Spreadsheet::fill_down(int r, int c) { //for formulae and cell with cellreferences
//selection of 1 or more cell refs down a col - find start sequence in first cell
//can be either upper or lower case, in or outside a formula
//rowinc defaults to 1 unless there is a number (only a number!) in 2nd row when that is used
//to play insert a number in 2nd col to increment col refs by that no
	int rowinc=1, colinc=0, rr=0;
	string temp;
	string saveddelims=formula.delimiters;
	formula.delimiters=formula.delimiters+",;: ";//extra delims needed
	r++; c++; //cell fns are one origin
	int notoks=formula.tokenise(cell(r,c));//use numeric tokeniser with expanded delimeter list
	if (atoi(cell(r+1,c).c_str())) {rowinc=atoi(cell(r+1,c).c_str());}//cout<<"rowinc "<<rowinc<<endl;}
	if (atoi(cell(r,c+1).c_str())) {colinc=atoi(cell(r,c+1).c_str());}//cout<<"colinc "<<rowinc<<endl;}
	rr= r;
	while (rr < s_bottom) {
		temp="";
		for (int n=0;n<notoks;n++) {
			if (parse_cellreference(formula.token[n].val)) {
				formula.token[n].val = relcellseq(formula.token[n].val,rowinc,colinc);
			}
			temp=temp+formula.token[n].val;
		}
		rr++;
		cell(rr,c,temp);
	} 	formula.delimiters=saveddelims;
}
void Spreadsheet::fill_right(int r, int c) {
//selection of 1 or more cell refs down a col - find start sequence in first cell
//can be either upper or lower case, in or outside a formula
//colinc defaults to 1 unless there is a number in 2nd col (only a number!) when that is used
//to play insert a number in 2nd row to increment row refs by that no
	int colinc=1, rowinc=0, cc=0;
	string temp;
	string saveddelims=formula.delimiters;
	formula.delimiters=formula.delimiters+",;: ";
	r++; c++; //cell fns are one origin
	int notoks=formula.tokenise(cell(r,c));//use numeric tokeniser as it has the more comprehensive delimiter list
	if (atoi(cell(r,c+1).c_str())) colinc=atoi(cell(r,c+1).c_str());
	if (atoi(cell(r+1,c).c_str())) rowinc=atoi(cell(r+1,c).c_str());
	cc= c;
	while (cc < s_right ) {
		temp="";
		for (int n=0;n<notoks;n++) {
			if (parse_cellreference(formula.token[n].val)) {
				formula.token[n].val = relcellseq(formula.token[n].val,rowinc,colinc);
			}
			temp=temp+formula.token[n].val;
		}
		cc++;
		cell(r,cc,temp);
	}	formula.delimiters=saveddelims;
}

double Spreadsheet::rounddouble(double doValue, int nPrecision) {
	//source simon hughes http://www.codeproject.com/cpp/floatutils.asp 20070818
    static const double doBase = 10.0;
    double doComplete5, doComplete5i;

    doComplete5 = doValue * pow(doBase, (double) (nPrecision + 1));

    if(doValue < 0.0)
        doComplete5 -= 5.0;
    else
        doComplete5 += 5.0;

    doComplete5 /= doBase;
    modf(doComplete5, &doComplete5i);

    return doComplete5i / pow(doBase, (double) nPrecision);
}

void Spreadsheet::create_sheet_menu(Fl_Menu_Bar* menubar) {
//now add sheet menu 20071201
//difficult to keep pointer fresh; need to control sheetnaming tightly at this stage
	//menu choice number for menu_functions corresponds with sheet number see : (void *) n below
	menubar->add("Sheet/Change name",0, 0, (void *) 17,0 ); //max 16 sheets
	//menubar->add("Sheet/Reset menu name",0,0, (void *) 18, FL_MENU_DIVIDER);
    for (int n=1;n<nosheets+1;n++) {//max no of sheets is 16 due to current case no limitation in menubar_cb
    	temp="Sheet/Sheet ";
    	temp=temp + itoa(n);
    	//temp.append(SHEETNAMESIZE - temp.size(),' '); //keep size at max allowed here
    	//sheetname[n-1]=temp;
    	menubar->add(temp.c_str(), 0, menubar_cb, (void*) n, FL_MENU_RADIO);//initial name will be changed but fix on std size
     	shptrs[n-1]= (Fl_Menu_Item*) menubar->find_item(temp.c_str());
    }
	//create_sheet_names();
}
/*
void Spreadsheet::create_sheet_names() { //these are fixed in menu although name displayed on a sheet can vary 
	shptrs[0]->setonly();
	string temp;
	for (int n = 0; n<nosheets; n++) {
		temp ="Sheet ";
		temp = temp + itoa(n+1); 
		change_sheet_name(temp,n);
	}
}  */
	
void Spreadsheet::change_sheet_name(string& sn, int cs) {
	//change current sheet name (control max no of characters)
	int sizen = sn.size();
	if (sizen>SHEETNAMESIZE) sizen=SHEETNAMESIZE; //avoid copying junk 20071203
	sheetname[cs].assign(sn,0,sizen);
	//Fl_Menu_Item *m = shptrs[cs];
	//if (m) m->label(sheetname[cs].c_str());
}
/*
void Spreadsheet::change_visible_sheet_number(Fl_Menu_Bar * menubar, int N) { //not used - does this work?
	//reinstate all buttons up to and including N sheets
	if (0<N && N<nosheets+1) {
    for (int n=0;n<nosheets;n++) {
        shptrs[n]= (Fl_Menu_Item*) menubar->find_item(sheetname[n].c_str());
    	if (shptrs[n]) shptrs[n]->show();
    }
	//clear all following sheet labels
	for (int n=N;n<nosheets;n++) shptrs[n]->hide();
	return;
	} 
} */

void Spreadsheet::adjust_viewscale(float value) {
	int oldviewscale = viewscale;
	float factor = value/oldviewscale;//float of value ensures floating pt division
	//cout << "vscale value factor "<<oldviewscale<<' ' << value <<' ' << factor<<endl;
	for (int c=0; c< cols(); c++) col_width(c,(int) (col_width(c) * factor) );
	for (int r=0; r< rows(); r++) row_height(r, (int) (row_height(r) * factor) );
	for (int r=0; r< rows(); r++) for (int c=0; c< cols(); c++)
		 cells[r][c].sizex((int) (cells[r][c].sizex()*factor)  );
	viewscale = (int) value;
}

void Spreadsheet::reset_viewscale() {
	//resets to default sizes
	for (int c=0; c< cols(); c++) col_width(c,COLWIDTH );
	for (int r=0; r< rows(); r++) row_height(r,ROWHEIGHT);
	for (int r=0; r< rows(); r++) for (int c=0; c< cols(); c++) cells[r][c].sizex(FONTSIZE);
	viewscale = 100;
}

void Spreadsheet::edit_move_multimapentries(int showonlyflag = 0) {
	multimap <mstring,drawing>::iterator z;
	z = drawingmap.begin();
	if ( ! drawingmap.size() ) { cout << "No multimap entries\n";return; }
	cout << "Multimap entries "<<drawingmap.size()<<endl;
	list <string>::iterator d;
	unsigned int _notokens;
	string c0;
	string startsel = convert_rc_address(s_top,s_left) ;
	string endsel   = convert_rc_address(s_bottom,s_right) ;
	while (z != drawingmap.end()) {

		stringstream strstr;
		strstr << z->first;
		c0 = strstr.str();
		if ( startsel > c0 || c0 > endsel ) { z++; continue;}
		cout <<"existing at     "<< z->first <<DL<<z->second<<endl;
		strstr.str("");
		strstr <<"startcell "<<c0 << " norows "<<z->second.norows<<" nocols "<<z->second.nocols; //<<endl;
		strstr << " active "<<z->second.active<<" type "<<z->second.type;
		strstr << " boxtype "<<z->second.boxtype<<" linestyle "<<z->second.linestyle;
		strstr <<" linewidth "<<z->second.linewidth; //<<endl;
		strstr << " linecolour "<<z->second.linecolour<<" fill colour "<<z->second.fillcolour;
		strstr <<" textalign "<<z->second.textalign<<" textsize "<<z->second.textsize<<" textfont "<<z->second.textfont;
		strstr <<" textcolour "<<z->second.textcolour<<endl;
		strstr << "text " << z->second.dataholder<<endl;
		string temp = strstr.str();
		strstr.str("");
		strstr << c0 <<DL<<z->second;
		string actual =  strstr.str();
		if (showonlyflag == 1) { fl_message("%s",temp.c_str()); return; }
		//want to edit
		const char * inp = fl_input("%s",temp.c_str(), actual.c_str());
		if (inp) {
			temp = inp;
			//parse temp and place in spd drawing struct and then in multimap
			newtokenise(temp,_notokens, DL);
			if (_notokens != 16) {cout << "token error in multimap "<< _notokens << endl; return; }
			string c1 = get_token(1);

			int tint;
			drawing spd;
			tint = atoi(get_token(2).c_str());
			if (tint >= 0) spd.norows = tint;

			if (! spd.norows ) { //can erase entry by setting no of rows to 0; original can be left or deleted
				drawingmap.erase(z);
				cout << "erased one at " << c0 << endl;
				//reset_drawingflags();
				//cout << "reset drawingflags\n";
				break; //cant continue after an erasure
			}
			tint = atoi(get_token(3).c_str());
			if (0 < tint && tint < cols()) spd.nocols = tint;
			tint = atoi(get_token(4).c_str());
			if (0 <= tint && tint <= 1) spd.active = tint;
			spd.type = z->second.type;	//disallow change of type - too ambiguous
			spd.boxtype =  atoi(get_token(6).c_str());
			spd.linestyle = atoi(get_token(7).c_str());
			spd.linewidth = atoi(get_token(8).c_str());
			spd.linecolour = atoi(get_token(9).c_str());
			spd.fillcolour = atoi(get_token(10).c_str());
			spd.spare = atoi(get_token(11).c_str());
			spd.textalign = atoi(get_token(12).c_str());
			spd.textsize = atoi(get_token(13).c_str());
			spd.textfont = atoi(get_token(14).c_str());
			spd.textcolour = atoi(get_token(15).c_str());
			spd.dataholder = get_token(16);
			if (c0 == c1) { //no copy or relocation; if c0 is changed then a copy is placed in the new location c1
				z->second = spd;
				cout<<"edited entry at "<<c0<<DL<<z->second<<endl;
			}
			else { //c1 is not equal to c0; relocate this entry; optionally keep the original

				drawingmap.insert(make_pair(mstring(c1), drawing(spd)));
				cout<<"copied entry to "<<c1<<DL<<spd<<endl;
				parse_cellreference(c1);
				cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].set_drawingflag();
				int msg = fl_choice("Keep the original?","Yes","No",0 );
				if (msg == 1) {
					drawingmap.erase(z);
					cout << "erased entry at " << c0 << endl;
					//reset_drawingflags();
					//cout << "reset drawingflags\n";
					return;
				}
			}
		}	z++;
	}	return;
}

void Spreadsheet::display_all_multimapentries (int displayflag = 1) {
	multimap <mstring,drawing>::iterator z;
	z = drawingmap.begin();
	string startsel = convert_rc_address(s_top,s_left) ;
	string endsel   = convert_rc_address(s_bottom,s_right) ;
	if (displayflag) cout << "unhiding all drawings, mergings etc in range"<< startsel<<" to "<< endsel<< endl;
	else cout << "hiding all drawings, mergings etc in range"<< startsel<<" to "<< endsel<< endl;
	int count = 0;

	while (z != drawingmap.end()) {
		if (((mstring) startsel) <= z->first && z->first <= ((mstring) endsel)) {
		cout <<"found one at "<< z->first<< endl;
		cout << z->second << endl;
		if (displayflag) z->second.active = 1;
		else z->second.active = 0;
		z++;
		count++;
		}
		else z++;
	}
	cout << "total found "<< count <<endl;
	return;
}
void Spreadsheet::display_all_comments (int displayflag = 1) {
	multimap <mstring,drawing>::iterator z;
	z = drawingmap.begin();
	string startsel = convert_rc_address(s_top,s_left) ;
	string endsel   = convert_rc_address(s_bottom,s_right) ;
	if (displayflag) cout << "unhiding all comments in range "<<startsel<<" to "<<endsel<<endl;
	else cout<< "hiding all comments in range "<<startsel<<" to "<<endsel<<endl;
	while (z != drawingmap.end()) {
		if (((mstring) startsel) <= z->first && z->first <= ((mstring) endsel)
					&& ( z->second.type == COMMENT || z->second.type == FLOATINGCOMMENT ))
		{
		cout <<"found one at "<<z->first<<endl;
		if (displayflag) z->second.active = 1;
		else z->second.active = 0;
		z++;
		}
		else z++;
	}
	return;
}

void Spreadsheet::delete_all_comments() {//could easily be converted to delete any nominated type
	string addr="";
	string startsel = convert_rc_address(s_top,s_left) ;
	string endsel   = convert_rc_address(s_bottom,s_right) ;
	cout << "deleting all comments in range "<<startsel<<" to "<<endsel<<endl;
	multimap <mstring,drawing>::iterator q;
	for (int r= s_top; r< s_bottom+1; r++)	{
		for (int c= s_left; c< s_right+1; c++)	{
			//if (cells[r][c].get_drawingflag()) {
				addr = convert_rc_address(r,c);
				q = drawingmap.find(addr);
				if (q == drawingmap.end()) continue;
				else { //at least one
					for (unsigned n = 0; n < drawingmap.count(addr); n++) {
						if (q->second.type == COMMENT || q->second.type == FLOATINGCOMMENT) {
			 				drawingmap.erase(q);
			 			}
			 			q++;
			 		}
				}
			//}
		}
	}
	//reset_drawingflags(); //in case a comment shared a cell with a non-comment overlay
	return;
}

void Spreadsheet::delete_all_multimapentries() {
	string addr="";
	string startsel = convert_rc_address(s_top,s_left) ;
	string endsel   = convert_rc_address(s_bottom,s_right) ;
	cout << "deleting all overlays in range "<<startsel<<" to "<<endsel<<endl;
	multimap <mstring,drawing>::iterator q;
	for (int r= s_top; r< s_bottom+1; r++) {
		for (int c= s_left; c< s_right+1; c++) {
			//if (cells[r][c].get_drawingflag()) {
				addr = convert_rc_address(r,c);
				q = drawingmap.find(addr);
				if (q == drawingmap.end()) continue;
				else {
					drawingmap.erase(addr); //note erasing all with this key here
			 		cells[r][c].clear_drawingflag();
			 	}
			//}
		}
	}
	return;
}




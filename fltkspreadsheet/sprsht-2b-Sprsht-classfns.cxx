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

void Spreadsheet::set_savefilename()  {
	const char* name;
	//preferably include a unique(?) filetype .sps
	name =fl_file_chooser("select ","*.sps*\tAll Files(*)","data/default.sps");
	if (name) savefilename=name;
}

void Spreadsheet::set_get_file(int no) {
	set_savefilename();
	getdata(no);
}

void Spreadsheet::saveas(int i) {
	const char* name;
	name = fl_input("enter data file name", savefilename.c_str());
	if (name) {
		savefilename = name;
		savedata(0);
	}
}

long Spreadsheet::filesize(const char * file_name = savefilename.c_str()) {
	long begin,end;
	ifstream myfile (file_name);
	begin = myfile.tellg();
	myfile.seekg (0, ios::end);
	end = myfile.tellg();
	myfile.close();
	cout << "size is: " << (end-begin) << " Bytes.\n";
	return end-begin;
}

void Spreadsheet::row_col_size_compact(string &_temp, char delim, int _rowcol, int type = 10) {
	/* compresses a sequence of general (numbers) values
	   in cells up to _row or _col by searching for a repeat occurrence
	   of a value and replacing it with a count <marker>nn
	   generally known as run length encoding
	   the string produced can be expanded by the uncompact fn
	   the string produced can be expanded by the uncompact fn
	   or it can be applied directly
	   example : 8`8`8`4`4`2`3`3`3`3`3 becomes : 8`3`4`2`2`1`3`5
	*/
	stringstream strstr;
	string olds,news;
	olds = "";
	strstr.str("");
	int counter = 0;

	if (type > 10) {
		news = itoa( (unsigned short) col_width(0));
		for (int c=0;c<_rowcol;c++) {
			news = itoa( (unsigned short) col_width(c));
			if (!counter) 		{ strstr << news ; olds = news; }//skip initial  multiplier  counter sequence
			if (news != olds)  	{ olds = news; strstr << delim << counter << delim << news ; counter = 1; }
			else counter++; //no change
		}
	}
	else {
		news = itoa( (unsigned short) row_height(0));
		for (int r=0;r<_rowcol;r++) {
			news = itoa( (unsigned short) row_height(r));
			if (!counter) 		{ strstr << news ; olds = news; }//skip initial  multiplier  counter sequence
			if (news != olds)  	{ olds = news; strstr << delim << counter << delim << news ; counter = 1; }
			else counter++; //no change
		}
	}
	strstr << delim << counter << delim <<endl;
	_temp = strstr.str();
}

void Spreadsheet::compact(string &_temp, char delim, char _multchar, int _row, int _col, int type = -1) {
	/* compresses a sequence of delimited cval() or ucx(n) values
	   in cells up to _row and _col by searching for a repeat occurrence
	   of a value and replacing it with a count <marker>nn
	   generally known as run length encoding
	   the string produced can be expanded by the uncompact fn
	   or it can be applied directly
	   example : 8`8`8`4`4`2`3`3`3`3`3 becomes : 8`3`4`2`2`1`3`5
	*/
	stringstream strstr;
	string olds,news;
	olds = "";
	strstr.str("");
	int counter = 0;
	//type is used to pick between ucx (cell format values) or if -1 to assign values (cval())
	//if (type > -1) 	news = itoa( (unsigned short) cells[0][0].ucx(type));
	//else 			news = cells[0][0].cval();
	if ( rowcolformat == 1 ) { // in row col format
		for (int r=0;r<_row;r++) {
			for (int c=0;c<_col;c++) {
				if ( type > -1) 	news = itoa((unsigned short) cells[r][c].ucx(type));
				else 				news = cells[r][c].cval();
				if (!counter) 		{ strstr << news ; olds = news; }//skip initial  multiplier  counter sequence
				if (news != olds)  	{ olds = news; strstr << _multchar << counter << delim << news ; counter = 1; }
				else counter++; //no change
			}
		}
	}
	else {
		for (int c=0;c<_col;c++) { // in col,row format
			for (int r=0;r<_row;r++) {
				if ( type > -1) 	news = itoa((unsigned short) cells[r][c].ucx(type));
				else 				news = cells[r][c].cval();
				if (!counter) 		{ strstr << news ; olds = news; }//skip initial  multiplier  counter sequence
				if (news != olds)  	{ olds = news; strstr << _multchar << counter << delim << news ; counter = 1; }
				else counter++; //no change
			}
		}
	}
		strstr << _multchar << counter << delim <<endl;
	_temp = strstr.str();
}

void Spreadsheet::uncompact(string &_s, char c, char _multchar) {
	/*reverse of compact when that is finally expanded
	  uncompresses a string by searching for a repeat occurrence
	  of <marker>nn and replacing it with nn times the preceeding value
	  delimited by c
	  reverses compact  but not used here*/
	string _t = "", _temp = "", _val = "";
	unsigned n = 0;
	while (n < _s.size() + 1) {
		if (_s[n] != _multchar)  { _t += _s[n]; _val += _s[n]; }
		else {
			n++;
			while (_s[n] != c) {_temp += _s[n]; n++;} //the counter
			for (int m = 0; m < atoi(_temp.c_str()) - 1; m++) _t += c + _val;
			_temp = "";
			_val = "";
			_t += c;
		}
		n++;
	}
	_s = _t +'\n';
	//cout<<"uncompacted data\n";
}

void Spreadsheet::newtokenise(string &t, unsigned &no_of_tokens, char datadelim = datadelimiter) {
	//almost identical to tokenise_arguments in Newparser
	//splits up on datadelimiter (defined in class) eg '`'
	register unsigned n=0;
	register unsigned i;
	unsigned ss = t.size();//was t.size()-1 gives error if end of string is two delimiters!! 20080313
	string temp;
	datatoken.clear();//clear list of any prior arguments
	for (i=n;i<ss;i++) {
		if (t[i]==datadelim ) {
			if (i>n) {
				//save this token
				datatoken.push_back(temp.assign(t,n,i-n));
				n=i+1; //save start new arg in n
			}
			else  {datatoken.push_back("");n++;}// a default token
		}
	}
	//final argument, if there is one other than a delimiter
	if (n<ss) {
		datatoken.push_back(temp.assign(t,n,ss-n));
	}
	no_of_tokens = datatoken.size();
	return;
}


string Spreadsheet::get_token(int n) {
//gets the nth token in a datatoken list that has been filled with the newtokenise function- if valid
	list <string>::iterator d;
	d = ss->datatoken.begin();
	for (int i=0;i<n-1;i++) if (d != ss->datatoken.end()) d++;
	return *d;
}

void Spreadsheet::printdata() {
	//outputs curretn sheet as semi-formatted data in csv format which can be read by spreadsheet pgms
	//which may then be used for further processing or printing
	lacell(); 	//set last active row and last active column larow/lacol
				//end each line of data with a ';'
	if (lacol==0) lacol = 1;
	if (larow==0) larow = 1;
	ofstream out (printfilename.c_str()) ; //output normal file see p543 schildt
	if (!out) { fl_message("cannot open file.\n"); return;  }

	//line 0
	out << savefilename << " sprsht " << getlocaltime()  << endl;

	//line 0.1j
	out <<   endl;

	//line(s) 7 ->
	for (int r=0;r<larow+1;r++) {
		for (int c=0;c<lacol+1;c++) {
			displayedvalue(r,c);
			out << _buffer << ',';
}
		out << endl;
		cout<<endl;
}
	out << "end of data;" << endl;
	cout << "saved print data to " << printfilename << "\n";
	fl_message("saved print data to %s \n",printfilename.c_str());
	out.close();
}

void Spreadsheet::savedata(int silent) {
	//revised and simplified 20071128 with allowance for multiple sheets
	//now save nosheets number of sheets (all sheets)-but some may be empty.
	stringstream strs;
	ofstream out ((savefilename).c_str()) ; //output normal file see p543 schildt
	if (!out) { fl_message("cannot open file for output.\n"); return;  }
	//line 0
	//for (int n=0;n<ss->nosheets;n++) ss->save_sheet(n,1);

	save_sheet(currentsheet); //to ensure latest changes are retained (since last sheet change)
	out << savefilename << " sprsht " << getlocaltime()  << endl;
	out << rows()<<datadelimiter<<cols()<<datadelimiter<<" , total rows/cols;"<<endl;//save here as can't be varied between sheets
	out << windowW << datadelimiter<< windowH<<datadelimiter<<" , window sizes"<<endl;

	out << nosheets << datadelimiter <<" - number of sheets "<< endl;

	for (int n=0;n<nosheets;n++) {
	    out<<sheetname[n]<< datadelimiter<< n+1 << datadelimiter << " -name and sheetno"<<endl;//n is read in as sheet no
		out << sheet[n]<< endl;
		out << endofsheetmarker <<endl;
	}

	out.close();
	if (!compressdata) {
		cout << "data file has been saved as : " + savefilename << endl;
		filesize();
		return;
	}
	//compress data file further - getdata can open this also if it finds a file with a .gz suffix
	temp ="gzip -f " + savefilename;//may need a different file compressor for gzip in Win// -f to overwrite an existing file
	if (! system(temp.c_str())) {
		temp ="data file has been compressed using gzip and saved as : " + savefilename+".gz";
		cout << temp << endl;
		fl_message("%s",temp.c_str());
		temp = savefilename +".gz";
		filesize(temp.c_str());
		return;
	}
	temp ="compression not used or failed; saved as .sps file";
	fl_message("%s",temp.c_str());
}

void Spreadsheet::getdata(int allsheets) {
	//if allsheets <0 gets all; else gets the first and places it in the current sheet
	//revised and simplified
	//see p546 schildt
    // open input file - must allow for quite long lines so use binary
	//valid data file all are of type sps or sps.gz 
	//savefilename is the prefix name
	//ss->row_position(0);//reposition cursor
	//ss->redraw();					//essential to do this in two steps
	set_selection(-1,-1,-1,-1);
	input->hide();//attempt to fix problem of missing data at cursor cell

	for (int r=0;r<ss->rows();r++)	for (int c=0;c<ss->cols();c++) ss->cells[r][c].unlock(); //clear all locks
	list <string>::iterator d; //data token list
	cout << "getting data from " << savefilename << "\n";
	//uncompress file if necessary using either gzip
	//open a compressed file if it exists (name must end in .gz)
	string ztemp = savefilename;
	unsigned long z = savefilename.find(".gz");//see if compressed with gzip
	if (z!=string::npos) {
		ztemp="gunzip -f " + savefilename;
		if (! system(ztemp.c_str())) {
			savefilename.erase(savefilename.size()-3,3);//eliminate .gz suffix as gunzip has done this too 
			cout << "opened " << ztemp  << "\n";
		}
		else {
			ztemp = "failed to open : " + ztemp;
			cout << ztemp << endl;
			fl_message("%s", ztemp.c_str());
			return;
		}
	}
	else {
	//for an Open after a compressed Save reinstate .gz suffix to savefilename  ?????
		cout <<"looking for compressed file\n";
		ztemp="gunzip -f " + savefilename + ".gz";
		if (! system(ztemp.c_str())) {
			ztemp = "failed to open : " + ztemp;
			cout << ztemp << endl;
			fl_message("%s", ztemp.c_str());
			return;
		}		
	}
	ifstream in;
	in.open((savefilename).c_str()); //, ios::in |ios::binary);
	if (!in) {cout << "cannot open file for input "+savefilename<<"\n"; return;  }
	//clear existing
	clearallcells();
	clear_inputvalue(); //clear value from input buffer
 	string str1, temp;
 	unsigned _notokens;

	getline(in,str1,'\n');
	textcolor(BRIGHT,YELLOW,BLACK);
	cout<<str1<<endl;//file name and save info
	textcolor();
	//line 2   total rows, cols

	getline(in,str1,'\n');
	newtokenise(str1, _notokens);
	d = datatoken.begin();
	int norows = atoi((*d).c_str());d++;
	int nocols = atoi((*d).c_str());
	cout << norows <<' ' << nocols << " max rows,cols" <<endl;
	if (nocols>cols()) {cols(nocols); cells.sdrcols(nocols);redraw(); cout<<"increasing columns to "<<nocols<<endl;} //make sure sufficient cols
	if (norows>rows()) {rows(norows); cells.sdrrows(norows);redraw();cout<<"increasing rows to "<<norows<<endl;}

	getline(in,str1,'\n');
	cout << str1 <<endl;// info only on window sizes - don't know how to change this dynamically from here

 	getline(in,str1,'\n');
 	int no_sheets_here = 1; //show at least one if next line fails
	no_sheets_here = atoi(str1.c_str()); //read number at start of line
	cout <<"no of sheets is "<< no_sheets_here <<endl;

	if (no_sheets_here > nosheets && allsheets<0) {
		temp = "no sheets required is ";
		temp += itoa(no_sheets_here);
		temp += " - available ";
		temp += itoa(nosheets);
		cout << temp<<endl;
		fl_message("%s",temp.c_str());
		no_sheets_here = nosheets;// don't exceed available sheets - reset spreadsheet at start up if necessary
	}
	//line 3 sheetname
	int sizen;
	if (allsheets>0) {//read all sheets
		for (int n=0; n<no_sheets_here; n++) {
			temp = "";
			getline(in,str1,'\n');
			newtokenise(str1, _notokens);
			d = datatoken.begin();
			if (_notokens > 0) temp = *d;
			sizen = temp.size();
			//provide a default sheet name if none read in
			if ( sizen == 0 ) { temp = "Sheet "; temp += itoa(n+1); sizen = temp.size(); }
			cout << "sheet no is : "<< n+1 << "  name is : "<< temp << endl;

			if ( sizen > SHEETNAMESIZE ) sizen = SHEETNAMESIZE; //limit name size
			sheetname[n].assign(temp,0,sizen);

			//now get rest of sheet
			getline(in,sheet[n],endofsheetmarker);
			// move down one line to start of a following sheet
		//	getline(in,str1,'\n');
		}
	}
	else { //read only the first into currentsheet
		getline(in,str1,'\n');
		newtokenise(str1, _notokens);
		d = datatoken.begin();
		if (_notokens > 0) temp = *d;
		sizen = temp.size();
		cout << "sheetname " << temp << endl;
		if (sizen>SHEETNAMESIZE) sizen=SHEETNAMESIZE; //limit name size
		sheetname[currentsheet].assign(temp,0,sizen);  //don't change current sheet name is

			//now get rest of sheet
		getline(in,sheet[currentsheet],endofsheetmarker);
			// move down one line to start of a following sheet
		//getline(in,str1,'\n');
	}

	cout<<"current sheet is no "<<currentsheet+1<<endl;
	in.close();

	get_sheet(currentsheet);	//synchronise the currently active sheet on opening a new file
	temp="";
	z = savefilename.rfind('/');
	if (z!=string::npos) temp.assign(savefilename,z+1,savefilename.size());
	ss->cells[0][0].cval(temp);	//show loaded file name at [0][0]
	ss->_restart = 1; //ensure a restart
	//scale according to current setting
	//adjust_viewscale((float) viewscale);
};

void Spreadsheet::save_sheet(int sheetno) {
	//revised and simplified 20071128 with allowance for multiple sheets
	//each sheet is saved to a string sheet[n]
	//userunlengthcompaction is default, 0 saves uncompacted data for testing only
	stringstream out, out1;
	string temp;
	char _dl = datadelimiter;

	lacell(); 	//set last active row and last active column larow/lacol

	lacol++;
	larow++;

	out.str("");
	if (lacol==1 && larow==1) { //empty sheet
		out << "0--------valid sps\n";
		sheet[sheetno]=out.str();
		return;
	}
	if  (rowcolformat == 1) out << '1';//save in rows
	else out << '2'; //save in columns

	//line 0.1
	out <<  "--------valid sps" << endl;
	//place larow and lacol in second line for later easy retrieval
	out << larow << _dl << lacol << _dl << previous_function << _dl <<  " - active rows/cols/previous_function;" << endl;
	char tm;
	if (thousandsmark==',') tm ='c';
	else tm='p';
	cout <<"saving "<< sheetname[sheetno]<< " active rows and cols "<< larow <<',' << lacol <<'\n';
	//line 1
	//20 off including 5 spare
	out << selectioncolor <<_dl<<localbackgroundcolor<<_dl<<hdrtextcolor<<_dl;
	out << hdrcolor<<_dl<<'?'<<_dl<<ROWHEIGHT<<_dl;
	out <<COLWIDTH<<_dl<<ss->row_header_width()<<_dl<<ss->col_header_height()<<_dl;
	out << decimalpoint<<_dl<<tm<<_dl<<currencymark<<_dl;
	out << widthscalefactor<<_dl<<heightscalefactor<<_dl<<viewscale<<_dl<<'?'<<_dl<<'?'<<_dl<<'?'<<_dl<<'?'<<_dl<<'?'<<_dl;
	out << " - current settings " << endl;

	out1.str("");
	for (int c=0;c<lacol ;c++)	out1 << (unsigned short) col_width(c)<<_dl;
	out1 << endl;
	temp = out1.str();
	//if ( userunlengthcompaction )
	row_col_size_compact(temp,_dl,lacol,11);
	out << temp;

	out1.str("");
	for (int r=0;r<larow;r++)	out1 << (unsigned short) row_height(r)<<_dl;
	out1<<endl;
	temp = out1.str();
	//if ( userunlengthcompaction )
	row_col_size_compact(temp,_dl,larow,10);
	out << temp;

	out << "cell contents and properties\n";
/*	if (! userunlengthcompaction) {

	for (int r=0;r<larow;r++) {
		for (int c=0;c<lacol;c++) {
			out<<cells[r][c].cval()<<_dl;
		}
	}
	out << endl;
	for (int n=0; n<8; n++) {
		for (int r=0;r<larow;r++) {
			for (int c=0;c<lacol;c++) {
				out<<(int) cells[r][c].ucx(n)<<_dl;
			}
		}
		out << endl;
	}
	} */
	//else {
	//OUTPUT CVAL() DATA

	compact(temp,_dl,_dl,larow,lacol,-1);
	out << temp;

	//OUTPUT UCX() DATA
	for (int n=0; n<8; n++) { compact(temp,_dl,_dl,larow,lacol,n); out << temp; }

	//}
 	//user defined C-style formats
	map <mstring, string>::iterator p;
	p = cformatmap.begin();
	out << "C-formats - continuous\n";
	//count the elements only with content in p->second (key value is not eliminated for some reason
	int noofelements=0;
	while (p != cformatmap.end()) {	if (p->second!="") noofelements++; p++;}
	out << noofelements<<endl;
	//now output
	if (noofelements) {
		p = cformatmap.begin();
		while (p != cformatmap.end()) {	if (p->second!="") out <<  p->first << _dl << p->second << _dl; p++;}
		out << endl;
	}

	map <mstring,drawing>::iterator q;
	out << "drawings - one per line\n";
	noofelements = drawingmap.size();
	out << noofelements<<endl;

	//now output
	if (noofelements) {
		q = drawingmap.begin();
		while (q != drawingmap.end()) {
			out <<  q->first << _dl << q->second << endl; q++;
		}
	}
	out << "end of data " << endl;
	sheet[sheetno]=out.str();
	cout <<"saved "<< sheetname[sheetno]<<'\n';
	if (!sheetno) cout<<sheet[sheetno]<<endl<<sheet[sheetno].size()<<endl;
}

void Spreadsheet::get_sheet(int sheetno) {
	//revised and simplified
	//userunlengthcompaction = 1 expands compacted data here
	char _dl=datadelimiter;
	int _multiplier, _value , _row, _col, _rowcol;
	unsigned _nt = 0; //no of tokens

	clock_t t,t1;
    t = clock();

	for (int r=0;r<ss->larow;r++) {
		for (int c=0;c<lacol;c++) {
			ss->cells[r][c].setdefault();
			ss->cells[r][c].cval("");
		}
	}
	cout << "clearing to row,col "<<larow<<','<<lacol<<endl;
	//clear maps
	drawingmap.clear();
	cformatmap.clear();

	list <string>::iterator d; //data token list

	ss->set_selection(-1,-1,-1,-1);
	//clear existing
	clear_inputvalue(); //clear value from input buffer

	//line 0
	//get first line
try {  //catch other input errors input errors
	string str1;
	stringstream temp;
	temp.str("");
	temp<<sheet[sheetno];//place sheetdata string into temp
	cout<<"getting sheet "<<sheetno+1<<endl;
	//cout << sheet[sheetno] <<endl;
	//line 0.1
	getline(temp,str1,'\n');//break data up into lines and allocate
	cout << str1 << endl;
	if (atoi(str1.c_str()) == 0 ) {
		cout << "empty sheet! - nothing more to do.\n";
		return;
	}
	if (atoi(str1.c_str()) == 1 ) {
		cout << "sheet was saved in row,col format\n";
		rowcolformat = 1;
	}
	else {
		cout << "sheet was saved in col,row format\n";
		rowcolformat = 0;
	}

	if (str1[2]!='-'&& str1[4]!='-') throw 10; //not the right type of file!!! expects a line of n-----
	//line 1
	unsigned narows = 0, nacols = 0;

    //first thing to be read in is active rows and columns
	getline(temp,str1,'\n');
	newtokenise(str1, _nt);
	d = datatoken.begin();
	while (d != datatoken.end()) {
		narows = atoi((*d).c_str());d++; //better to determine from the data available
		nacols = atoi((*d).c_str());d++;  //determine from the data available
		previous_function = atoi((*d).c_str());
		d = datatoken.end();
	}
	cout << "got active rows cols : "<< narows << ' ' << nacols << " function no : " << previous_function<< endl;
	cout << str1 <<endl;
	getline(temp,str1,'\n'); //get current settings
	newtokenise(str1, _nt);
	d = datatoken.begin();
	int spare;
	while ( d != datatoken.end() ) {
		selectioncolor = (Fl_Color) atoi((*d).c_str());d++;
		localbackgroundcolor = (Fl_Color)atoi((*d).c_str());d++;
		hdrtextcolor = (Fl_Color) atoi((*d).c_str());d++;
		hdrcolor = (Fl_Color) atoi((*d).c_str());d++;
		spare = atoi((*d).c_str());d++;
		row_height_all(atoi((*d).c_str()));d++;
		col_width_all(atoi((*d).c_str()));d++;
		row_header_width(atoi((*d).c_str()));d++;
		col_header_height(atoi((*d).c_str()));d++;
		decimalpoint = (char) (*d)[0]; d++;
		thousandsmark = (char) (*d)[0]; d++;
		if (thousandsmark == 'c') thousandsmark = ','; //a comma is thousands separator
		if (thousandsmark == 'p') thousandsmark ='.';
		//char currencymark = (char) (*d)[0]; d++;
		d++;
		widthscalefactor = atof((*d).c_str());d++;
		heightscalefactor = atof((*d).c_str());d++;
		viewscale = atoi((*d).c_str());
		d = datatoken.end();
	}
	cout << "got current settings\n";
	cout << str1 <<endl;

		//newer more efficient uncompaction process where compacted values are assigned directly
		//using value x multiplier
		//rather than being expanded to a string and then tokenised.
		//line 6 column widths
		getline(temp,str1,'\n');
		newtokenise( str1, _nt );
		d = datatoken.begin();
		_rowcol = 0;
		for (unsigned n = 0; n < _nt/2; n++) {
			_value = atoi((*d).c_str());
			d++;
			_multiplier = atoi((*d).c_str());
			d++;
			for (int c = 0; c < _multiplier; c++ ) {
				//assign_intdata( &(Fl_Table::col_width)(_rowcol, _value) );
				col_width(_rowcol, _value);
				_rowcol++;
			}
		}
		cout <<"got column widths\n";

		getline(temp,str1,'\n');
		newtokenise( str1, _nt );
		d = datatoken.begin();
		_rowcol = 0;

		for (unsigned n = 0; n < _nt/2; n++) {
			_value = atoi((*d).c_str());
			d++;
			_multiplier = atoi((*d).c_str());
			d++;
			for (int r = 0; r < _multiplier; r++ ) {
				row_height(_rowcol,_value);
				_rowcol++;
			}
		}
		cout << "got row heights\n";

 	getline(temp,str1,'\n');	//skip cell contents heading
 	cout << "got cell contents heading\n";

	// CVAL DATA
	getline(temp,str1,'\n');
	//cout << str1 << endl;
	newtokenise( str1, _nt );
	d = datatoken.begin();
	//cout << "no of tokens " << _nt << endl;
	_rowcol = 0;
	string _strval;
	for (unsigned nn = 0; nn < _nt/2; nn++) {
		_strval = *d;
		d++;
		_multiplier = atoi((*d).c_str());
		d++;
		if ( _strval == "") { _rowcol += _multiplier; continue;	} //if default, jump to next token pair
		for (int nnn = 0; nnn < _multiplier; nnn++ ) {
			if (rowcolformat == 1) {
				_row = _rowcol / nacols; //determine appropriate row
				_col = _rowcol - _row * nacols; //determine column
			}
			else {
				_col = _rowcol / narows;
				_row = _rowcol - _col * narows;
			}
			cells[_row][_col].cval(_strval);
			_rowcol++;
		}
	}

	cout << "got cval data "<<endl;
	// UCX(N) DATA

	dummycell.setdefault(); // setup defaults in dummycell to be able to test against them

	for (int n=0;n<8;n++) {

		getline(temp,str1,'\n');

		newtokenise( str1, _nt );
		d = datatoken.begin();
		_rowcol = 0;

		for (unsigned nn = 0; nn < _nt/2; nn++) {
			_value = atoi((*d).c_str());
			d++;
			_multiplier = atoi((*d).c_str());
			d++;
			if ( _value == dummycell.ucx(n)) { _rowcol += _multiplier; continue; }//if default, jump to next token pair
			for (int nnn = 0; nnn < _multiplier; nnn++ ) {
				if (rowcolformat == 1) {
					_row = _rowcol / nacols; //determine appropriate row
					_col = _rowcol - _row * nacols; //determine column
				}
				else {
					_col = _rowcol / narows;
					_row = _rowcol - _col * narows;
				}
				cells[_row][_col].ucx(n,_value);
				_rowcol++;
			}
		}
	}

	cout << "got ucx(n) data "<<endl;
//}
	//C-style formats
	getline(temp,str1,'\n');
	//cout<<str1<<endl;
	getline(temp,str1,'\n');
	int noofelements = atoi(str1.c_str());
	//cout << noofelements<<endl;
	if (noofelements) {
		getline(temp,str1,'\n');
		if(count(str1.begin(),str1.end(),_dl)!=2 * noofelements) throw 6;
		//cout<<str1<<endl;
		newtokenise(str1,_nt);
		d = datatoken.begin();
		cformatmap.clear();
		string key, value;
 		for (int n=0;n<noofelements;n++)	{
			key = *d;
			d++;
			value = *d;
			cformatmap[key]=value;
			d++;
		}
	}
	cout <<"got C-style formats\n";
	//drawings, merge overlays in multimap
	getline (temp,str1,'\n');
	cout <<str1 <<endl; //contains caption 'drawings'
	getline (temp,str1,'\n');
	newtokenise(str1, _nt);
	noofelements = atoi( get_token(1).c_str() );
	//d = datatoken.begin();
	//noofelements = atoi((*d).c_str());

	//cout << "number of elements "<<noofelements<<endl;
	//drawing_count=0;
	drawing spd;
	if (noofelements) {
 		drawingmap.clear();
		string c0;
		for (int n=0;n<noofelements;n++)	{
			getline(temp,str1,'\n');
			if(count(str1.begin(),str1.end(),_dl)!= 16 ) { cout<<"should be " <<16<<_dl<<" ->  "<<str1<<endl;throw 11;}
			newtokenise(str1, _nt);
			c0 =  get_token(1);
			cout <<"key : "<<c0<<endl;
			//make sure any drawings present get displayed by setting drawingflag !!
			if (!parse_cellreference(c0)) { cout<<c0<<" : "<<endl; throw 12; }//can't read this address!

			cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].set_drawingflag();

			spd.norows = atoi(get_token(2).c_str());
			spd.nocols = atoi(get_token(3).c_str());
			spd.active = atoi(get_token(4).c_str());
			spd.type = atoi(get_token(5).c_str());
			spd.boxtype =  atoi(get_token(6).c_str());
			spd.linestyle = atoi(get_token(7).c_str());
			spd.linewidth = atoi(get_token(8).c_str());
			spd.linecolour =  atoi(get_token(9).c_str());
			spd.fillcolour =  atoi(get_token(10).c_str());
			spd.spare =  atoi(get_token(11).c_str());
			spd.textalign = atoi(get_token(12).c_str());
			spd.textsize = atoi(get_token(13).c_str());
			spd.textfont = atoi(get_token(14).c_str());
			spd.textcolour = atoi(get_token(15).c_str());
			spd.dataholder = get_token(16);
			cout<<"overlay map entry for "<<c0<<": "<<spd<<endl;
			drawingmap.insert(make_pair(mstring(c0), drawing(spd)));
		}
		//drawing_count = noofelements;
	}
	cout << "got overlay data\n";
	cout <<"got sheet "<< sheetname[sheetno]<<'\n';
	t1 = clock() -t;
	printf("get_sheet(%d) data fetch took %g milli clock cycles\n", sheetno+1, 1000. * t1);
	} //end of try
catch(int i) {
	const char *e[] = {
	"error in cell align count",
	"error in cell font count",
	"error in cell size count",
    "error in cell colour count",
	"error in row height count", // index 4 to here
	"error in col width count",
	"error in C-style format count",
	"error in col precision count",
	"error in current settings count",
	"cell count error in cval() fetch in getsheet",
 	"not a valid sprsht input file!!!!",
 	"error in merge,drawing overlay count",
 	"overlay address error",
 	"cell count error in ucx(n) fetch in getsheet",
	"active row & col fetch failed",
	};
	fl_message("%s",e[i]);
	cout<<e[i]<<endl;
	return;
	}
	//printf("\nend of data\n");		//printf("%s\n",str1.c_str());
	//in.close();
}



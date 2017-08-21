//sprsht	v1.2.2
//added more overloaded cell functions for formatting
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


//PROVIDE HEADERS FOR USER FUNCTIONS HERE IF PARSER involved=========================================
void  menubar_cb(Fl_Widget* w, void *data);
Fl_Menu_Bar menubar(0,0,0,0);
Fl_Output disp(0,0,0,0);
Fl_Double_Window win(0,0,0);
void homeloan();//if restart = 0 goes thru setup
void pipeflow();
void pipe_network();
//================================================================================

//following based on 2002-2-24 Wu Yongwei using a short string to improve map access time
//http://www.geocities.com/yongweiwu

union mstring
{
    char        name[8];  //mapped variable names truncated to 7 usable characters
    LONGLONG    index;
    mstring(const char* str) {strncpy(name, str,8);}
    mstring(string &str) {strncpy(name,str.c_str(),8);}//easier using a string constructor
    LONGLONG getindex() {return index;}
    string getname() {string temp = name; return temp;}
	friend ostream &operator<<(ostream &stream, mstring ob) { //output stream operator
		stream << ob.name;
		return stream;
	}
	friend bool operator < (const mstring x, const mstring y){return x.index < y.index;}
	friend bool operator <= (const mstring x, const mstring y){return x.index <= y.index;}

};

class drawing {//for line, rectangle , circle draws over existing text with/without hiding it
public:
	drawing() {boxtype = FL_RSHADOW_BOX; }// fudge for this type of box, seems to need initialisation before read in of data
	~drawing() {};
	int norows;
	int nocols; //endcelladdress;
	short active; //flag to show or not
	short type; //line, rectangle etc
	short boxtype;
	short linestyle; //dashed or solid
	short linewidth;
	short linecolour;
	short fillcolour;
	short spare;
	short textalign;
	short textsize;
	short textfont;
	short textcolour;
	string dataholder;
	//string endcelladdress;//for use with savedata etc
	friend ostream &operator<<(ostream &stream, drawing ob) { //output stream operator
		stream << ob.norows<<DL<<ob.nocols<<DL<<ob.active<<DL<<ob.type<<DL<<ob.boxtype<<DL<<ob.linestyle<<DL;
		stream << ob.linewidth<<DL<<ob.linecolour<<DL<<ob.fillcolour<<DL<<ob.spare<<DL<<ob.textalign<<DL;
		stream<<ob.textsize<<DL<<ob.textfont<<DL<<ob.textcolour<<DL<<ob.dataholder<<DL;
		return stream;
	}
};

class Cells
{ //each cell has a string (contents) and some formatting
	string  val; // 4 bytes
	unsigned char uc[8];//8 bytes
	/* 	uc[0] alignment
		uc[1] size
		uc[2] font
		uc[3] colour
		uc[4] bits :show zero,plus,neg,thousands,currency, use std format, parse (formula evaluation) flag, digits (precision_type)
		uc[5] bits :store lock, hide, isnegative, spare, drawingflag, cellnodraw
		uc[6] bg colour
		uc[7] precision
	*/
	public:
		Cells()	{setdefault();val = "";}
		Cells(unsigned j, string s) {uc[0] = j;	val = s;}
		~Cells() 	{ };
		Cells(Cells &c) {
			val=c.val;
			for (int n=0;n<8;n++) uc[n]=c.uc[n];
		}
		Cells operator = (Cells temp) {
			val		= temp.val;
			for (int n=0;n<8;n++) uc[n]=temp.uc[n];
			return *this;
		}
 		friend ostream &operator<<(ostream &stream, Cells o) { //output stream operator
			//char dl='`';
			stream<<o.val;
			stream<<o.uc[0]<<o.uc[1]<<o.uc[2]<<o.uc[3]<<o.uc[4]<<o.uc[5]<<o.uc[6]<<o.uc[7];
 			return stream;
 		}
 		friend istream &operator>>(istream &stream, Cells o) { //input stream operator
			stream>>o.val;
			stream>>o.uc[0]>>o.uc[1]>>o.uc[2]>>o.uc[3]>>o.uc[4]>>o.uc[5]>>o.uc[6]>>o.uc[7];
			return stream;
 		}
		void setattributes(string &str);//assign values in packed uc string (save & getdata)
		string showattributes() { //show attributes in readable (hex) form)
			char buf[BUFFER];
			snprintf(buf,BUFFER-1,"prec %4u bgcolr %4u hide_lock_negative %4u view %4u colr %4u size %4u font %4u align%4u",
			uc[7],uc[6],uc[5],uc[4],uc[3],uc[2],uc[1],uc[0]);
			return buf;
		}
		unsigned char ucx(int n) const {return uc[n];} //for getting and saving data
		void ucx(int n, int c) {uc[n]=c;}
		void ucx(int n, string c) {uc[n]=atoi(c.c_str());}
		string packuc() { //pack ucs (attributes) into delimited numbers
			char buf[BUFFER];
			string temp = "";
			temp = temp +"%u"+DL+"%u"+DL+"%u"+DL+"%u"+DL+"%u"+DL+"%u"+DL+"%u"+DL+"%u"+DL;
			snprintf(buf,BUFFER-1,temp.c_str(),uc[7],uc[6],uc[5],uc[4],uc[3],uc[2],uc[1],uc[0]);
			return buf;
		}
		string packucnodl() { //pack ucs (attributes) w/o delimiter
			char buf[BUFFER];
			snprintf(buf,BUFFER-1,"%u%u%u%u%u%u%u%u",
			 uc[7],uc[6],uc[5],uc[4],uc[3],uc[2],uc[1],uc[0]);
			return buf;
		}
		//double stoLL(string &s);
		void    clearcell_value() {val = "";}//clear value but retain format in that cell
		void 	clearcell_all(){
				val = ""; 
				setdefault();
		}
		//clear and reset formats and all flags
		void    setdefault() {	//sets default formats
			alignx(ALIGN);fontx(FONT);sizex(FONTSIZE);
			colourx(TEXTCOLOR);bgcolourx(BACKGROUNDCOLOR);precisionx(PRECISION);
			//following set or clear as shown
			//set zero, clear +,. set -ve red, set 000, clear $,  set std fmt, set parse,
			//clear precision type (!digits=decimal places or no of significant digits)
			setbit4(0);clrbit4(1);setbit4(2);setbit4(3);clrbit4(4);setbit4(5);setbit4(6);clrbit4(7);
			//clear lock and clear hide and clear isnegative
			clrbit5(0);clrbit5(1);clrbit5(2);clrbit5(3);clrbit5(4);clrbit5(5);clrbit5(6);clrbit5(7);
		}
		//cell content write and read
		void cval(string s)	{val = s;}
		string cval() const { return val;}

		int empty() const 	{return val==" "|| val=="";} //20070201 mod
		//bool notempty()	const {return val!=" " && val!="";}//20070201 mod
		//alignment
		unsigned alignx() const {return uc[0];}
		void alignx(unsigned a) {uc[0]=a;}
		//font
		Fl_Font fontx() const {return (Fl_Font) uc[1];}
		void 	fontx(unsigned f) {uc[1] = f;}
		//size
		unsigned sizex() const {return uc[2];}
		void sizex(unsigned s) {uc[2] = s;}
		//color
		Fl_Color colourx() const {return (Fl_Color) uc[3];}
		void colourx(Fl_Color c) {uc[3] = c;}
		//bg color
		Fl_Color bgcolourx() const {return (Fl_Color) uc[6];}
		void bgcolourx(Fl_Color c) {uc[6] = c;}
		//precision
		int precisionx() const   {return (int) uc[7];}
		void precisionx(unsigned short p) {uc[7]=p;}
		//uc[4] flag bits
		void setbit4(unsigned  n) {uc[4] = uc[4] | (1<<n);}
		void clrbit4(unsigned  n) {uc[4] = uc[4] & ~ (1<<n);}
		//uc[5] flags
		void setbit5(unsigned  n) {uc[5] = uc[5] | (1<<n);}
		void clrbit5(unsigned  n) {uc[5] = uc[5] & ~ (1<<n);}
		//uc[6]
		void setbit6(unsigned  n) {uc[6] = uc[6] | (1<<n);}
		void clrbit6(unsigned  n) {uc[6] = uc[6] & ~ (1<<n);}
		//uc[7]
		void setbit7(unsigned  n) {uc[7] = uc[7] | (1<<n);}
		void clrbit7(unsigned  n) {uc[7] = uc[7] & ~ (1<<n);}
		//zero flag
		//flags are returned as 0, or a number >0 (not necessarily 1!!)
		unsigned zero() const {return uc[4] & 1;} 	//0th lower order bit i & (1<<n) returns 1 if set
		void zero(unsigned z) {if (!z) clrbit4(0); else setbit4(0);}
		//plus sign flag
		unsigned plus() const {return uc[4] & (1<<1);} 	//1th lower order bit i & (1<<n) returns 2 if set
		void plus(unsigned z) {if (!z) clrbit4(1); else setbit4(1);}
		//colour negative numbers
		unsigned colournegative() const {return uc[4] & (1<<2);} 	//2th lower order bit i & (1<<n) returns 4 if set
		void colournegative(unsigned z) {if (!z) clrbit4(2); else setbit4(2);}
		// show thousands mark
		unsigned thousands() const {return uc[4] & (1<<3);} 	//3th lower order bit i & (1<<n) etc
		void thousands(unsigned z) {if (!z) clrbit4(3); else setbit4(3);}
		// show currency symbol
		unsigned currency() const {return uc[4] & (1<<4);} 	//4st lower order bit i & (1<<n)
		void currency(unsigned z) {if (!z) clrbit4(4); else setbit4(4);}
		//std format
		unsigned format() const {return uc[4] & (1<<5);} //turns std formatting off to allow use of C style format or none
		void format(unsigned z)	{if (!z) clrbit4(5); else setbit4(5);}
		//parse flag
		unsigned parse() const {return uc[4] & (1<<6);}
		void parse(unsigned z)	{if (!z) clrbit4(6); else setbit4(6);}
		//
		unsigned digits() const {return uc[4] & (1<<7);} //number of significant digits type format
		void digits(unsigned z) {if (!z) clrbit4(7); else setbit4(7);}
		//lock flag
		unsigned get_lockstate() const {return uc[5] & 1;} 	//0th lower order bit i & (1<<n)
		void lock() {setbit5(0);}
		void unlock() {clrbit5(0);}
		//hide flag
		unsigned get_hidestate() const {return uc[5] & (1<<1); } 	//1th lower order bit i & (1<<n)
		void hide() {setbit5(1);}
		void unhide() {clrbit5(1);}
		//isnegative flag
		unsigned get_isnegativestate() const {return uc[5] & (1<<2); } 	//2nd lower order bit i & (1<<n)
		void set_isnegative() {setbit5(2);}
		void clear_isnegative() {clrbit5(2);}
		//special draw flag to enable signalling draw routine 
		unsigned get_spare () const {return uc[5] & (1<<3); } 	//3rd lower order bit i & (1<<n)
		void set_spareflag() {setbit5(3);}
		void clear_spareflag() {clrbit5(3);}
		//drawing flag
		unsigned get_drawingflag () const {return uc[5] & (1<<4); } 	//4th lower order bit i & (1<<n)
		void set_drawingflag() {setbit5(4);}
		void clear_drawingflag() {clrbit5(4);}
		//no cell drawing flag
		unsigned get_cellnodrawflag () const {return uc[5] & (1<<5); } 	//5th lower order bit i & (1<<n)
		void set_cellnodrawflag() {setbit5(5);}
		void clear_cellnodrawflag() {clrbit5(5);}
		//end of flags
		//number test
		int isnumber() const {
 			//returns a 1 if val can be turned into a number
 			//	returns 0 if it cant
 			//	returns -1 if overflow
 			// underflow is accepted as 0
			char *endptr;
  	 	 	errno = 0;
 	  	 	double value = strtod(val.c_str(), &endptr);
   		 	//Check for some errors
  		  	if (value > 1.7976931348623157e+308) return -1;
   		 	if (endptr == val.c_str()) return 0;
   		 	else return 1;
		}
		int valsize()const {return val.size();}  //return size of val string
		string noleadspace(string s)  {
			int j=0;while (s[j]==' ') j++;
			return s.erase(0,j);
		}
		string nospace(string s) { //eliminates all spaces from strings
			while (string::npos != (s.find(' '))) s.erase(s.find(' '),1);
			return s;
		}
};

//void Cells::setattributes(string &str) {
	//needs reworking
//	string t;
//	for (int n=7;n>=0;n--) {
//		t.assign(str,n*3,3);
//		uc[3-n]= atoi(t.c_str());
//	}
//}
/*
double Cells::stoLL(string &s) {
	string temp=s;
	string t;
	//cout<<"inside stoLL "<<s<<" length " << s.size()<<endl;
	for (int n=0;n<4;n++) {
		t.assign(temp,n*2,2);
		//cout << t<<endl;
		uc[3-n]= atoi(t.c_str());
	}
	return 0.; //mask;
} */

// parser header====================================================
const int MAXTOKENS = 128; //this approach 4x faster than an equivalent list

struct tokens {
	string val;
	char type;
};


ostream &operator<<(ostream &stream, tokens ob) { //output stream operator
		stream << ob.type << ' ' << ob.val;
		return stream;
}

enum types {DELIM='D',VAR='V',NUM='N',STRING='S',STRINGVAR='$',EMPTY='E'};

class Parser {
	//holds common routines and variables for NumParser and Strparser
public:
	Parser() ;
	~Parser() ;
	inline void puttoken() {tokno--; currenttoken=&token[tokno];}
	inline void gettoken() {//to ensure proper interpreting of predefined vars
		tokno++;				//at start of expr don't reset currenttoken if tokno=notoks
		if(tokno<notoks) currenttoken=&token[tokno];
	}
	void nospace(string &s) { //eliminates all spaces from strings
		while (string::npos != s.find(' ')) s.erase(s.find(' '),1); return; }
	string &getvar(mstring &name);
	string &findvar(mstring &name); //& slight speed improvement
	//void findfnend(string &exp,unsigned i);//keep here
	void range_expander(string &args);
	void tokenise_arguments(string &t);
	void findfnend(string &exp,unsigned i);//keep here
	void extractfn(string &exp);//keep here
	void getcellvar(string &t);
	string serror(int error);
	int st; //index of fn startvoid Numparser::findfnstart (string &exp, int i=1)
	int en; //and fn end
	int arg; //index of start of fn argument
	tokens token[MAXTOKENS];
	tokens *currenttoken; // pointer gives 25% reduction in time taken)
	int tokno; // current token
	int notoks;
	string fntoken;  //holder for function eg "area(2)"
	string stmp;
	static map <mstring, string> vmap; //map of permanent variables
	static map <mstring, string> tmap; //map of temp user defined vars except for reals
	static map <mstring, int> fmap; //Ptype or floating fn map -> value is index to find a fn
	list<string> argtoken;
	static int destructorcount;
};
int   Parser::destructorcount=0;
map <mstring, string> Parser::vmap;
map <mstring, string> Parser::tmap;
map <mstring,    int> Parser::fmap;

class Strparser : public Parser {
private:
 	void sevalexp1(string &sresult);
	void sevalexp2(string &sresult);
	void sevalexp5(string &sresult);
	void sevalexp6(string &sresult);
	void satom(string &sresult);
	string &eval_a_sfn(int cfn);
	int currentfn;
public:
	Strparser() {};
	~Strparser(){};
	int stokenise(string &t);
	void sfindfnstart (string &exp, unsigned i=1);//has its own temporary delimiter list so specialised
	void sevalfns(string &exp);
	string & sevalexp(string &exp, int r=0, int c=0);
	//void getcellvar(string &t);
	static string delimiters;
	string sresult;
	string temp;
};
string Strparser::delimiters="+-=()";

template <class Ptype> class Numparser : public Parser {
private:
	void evalexp1(Ptype &result);
	void evalexp2(Ptype &result);
	void evalexp3(Ptype &result);
	void evalexp4(Ptype &result);
	void evalexp5(Ptype &result);
	void evalexp6(Ptype &result);
	void atom(Ptype &result);
	Ptype &eval_a_fn(int cfn);
	int currentfn;
public:
	Numparser() {};
	~Numparser() {};
	int tokenise(string &t);
	void findfnstart (string &exp, unsigned i=1);//has its own temporary delimiter list so specialised
	void evalfns(string &exp);
	string &evalexp(string &exp, int r=0, int c=0);
	static Ptype presult;
	static string delimiters;
	string sresult;
	string temp;
};

template <class Ptype> Ptype Numparser<Ptype>::presult;
template <class Ptype> string Numparser<Ptype>::delimiters="+-*/%^=()"; //may be varied elsewhere eg in selection sequences


//====================================================
Fl_Callback input_cb; //used in Spreadsheet below see also sprsht.cxx
//====================================================

class Spreadsheet : public Fl_Table  { // derived from Fl_Table
	Fl_Input* input;
	int row_edit, col_edit;
	//bool parserflag;  20070823
	protected:
		void draw_cell(TableContext context,  			// table cell drawing
					   int R=0, int C=0, int X=0, int Y=0, int W=0, int H=0);
		static void event_callback(Fl_Widget*, void*);
		void 		event_callback2();					// callback for table events
	public:
		//static SDRarray <int> numprecision;
		static Cells dummycell;
		static SDRarray2D <Cells> cells;
		//20071127 based on nosheets being set dynamically
		static SDRarray <string> sheet;
		static SDRarray <string> sheetname;
		static SDRarray <Fl_Menu_Item *> shptrs;
		//20071127
		static SDRarray2D <Cells> clipboard;
		static map <mstring,string> cformatmap;//for holding a cell's C type format string
		static multimap <mstring,drawing> drawingmap;
		static deque <string> undoqueue; // for input undo 20070826
		Numparser <Ptype>formula; //long double, int etc
		Strparser sformula;
		void clear_inputvalue() {input->value("");}
		string get_inputvalue() {return input->value();}
		void set_inputvalue(string s) {
			input->value(s.c_str());
			cells[row_edit][col_edit].cval(input->value());
		}
		const char* nls(const char* s) {while (*s==' ') s++; return s;} //see also noleadspace
		inline void set_value(){cells[row_edit][col_edit].cval(nls(input->value()));input->hide();}
		inline int iszero(const char* s);  //test whether string evaluates to number 0
		inline void bf(int wid, int prec) {snprintf(_buffer,BUFFER-1,"%%%d.%df",wid,prec);}
		inline int clipboardrows() {return clipboard.sdrrows();}
		inline int clipboardcols() {return clipboard.sdrcols();}

		Spreadsheet(int x,int y,int w,int h,const char*l=0):Fl_Table(x,y,w,h,l) {
			Fl::set_fonts("-*"); //set all ISO5889-1 fonts
			dummycell.setdefault();
			dummycell.cval("");
			cformatmap.clear();
			drawingmap.clear();
   			setlocale(LC_MONETARY,""); //need fltk2 for utf encoding
   			struct lconv * lc = localeconv();
   			currencymark = (*lc->currency_symbol);
			callback(&event_callback, (void*)this);
			input = new Fl_Input(w/2,h/2,0,0);
			input->box(FL_NO_BOX);
			input->callback(input_cb, (void*)this);
			input->when(FL_WHEN_ENTER_KEY_ALWAYS);
			input->maximum_size(BUFFER-1);   //MAX_INPUT of 255 built into fltk?
			end();
		}
		~Spreadsheet() {
			cformatmap.clear();
			drawingmap.clear();
			undoqueue.clear();
		}

		void rows(int val) { if (input->visible()) input->do_callback(); Fl_Table::rows(val); } //set no or rows
		void cols(int val) { if (input->visible()) input->do_callback(); Fl_Table::cols(val); }
		inline int rows() { return Fl_Table::rows(); } //get no of rows
		inline int cols() { return Fl_Table::cols(); }
		void clearallcells() { //clear all cells of values and reformat to default on current sheet
			for(int r=0; r< rows(); r++) for (int c= 0; c< cols(); c++) cells[r][c].clearcell_all(); 
			cformatmap.clear();//clear maps and queues
			drawingmap.clear();
			undoqueue.clear();
			reset_viewscale();
			clear_inputvalue(); //clear value from input buffer
			string temp = "Sheet ";
			temp+= itoa(currentsheet+1);
			sheetname[currentsheet] = temp; 
		}
		
		inline void clearallunlockedcells() { //clear all cells of values and reformat to default
			for(int r=0; r< rows(); r++)
			for (int c= 0; c< cols(); c++) if (cells[r][c].get_lockstate()==0) cells[r][c].clearcell_all(); //20070823
			clear_inputvalue(); //clear value from input buffer
		}
		string noleadeq(string s) {int j=0;while(s[j]=='=') j++;return s.erase(0,j);}

		//overloaded cell functions  usually more convenient than using cells[r][c]. fnx==========================================
		//setting functions
		void cell1(int r, int c, string val) {cells[r-1][c-1].cval(val);Row1=r;Col1=c;} //r and c values in static
		void cell1(const char *cellref,string val) {
			if (!parse_cellreference(cellref)) return;
			Row1=atoi(rowstr.c_str());
			Col1=label_to_int(colstr)+1;
			cells[Row1-1][Col1-1].cval(val);
		}
		void cellR(string val) {cells[Row1-1][Col1].cval(val); Col1++;}//move to right
		void cellD(string val) {cells[Row1][Col1-1].cval(val); Row1++;}//move down

		void cell(int r, int c, string val) {cells[r-1][c-1].cval(val);}//NB 1 origin adressing only here
		void cell(int r, const char*col, string val) {cells[r-1][label_to_int(col)].cval(val);} // eg cell(5,"bc","hello") ->"bc5"
		void cell(int r, const char*col, double val) {cells[r-1][label_to_int(col)].cval(dtoa(val));} // eg cell(5,"bc","hello")->"bc5"
		//void cell(int r, const char*col, long double val) {cells[r-1][label_to_int(col)].cval(ldtoa(val));}
		void cell(const char* row,int c, string val) {cells[atoi(row)-1][c-1].cval(val);} // "5", 3 -> "c5"
		void cell(const char *cellref,string val) {
			if (!parse_cellreference(cellref)) return;
			cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].cval(val);
		}
		void cell(const char * firstcell,const char * lastcell, string val) { //allows input to a range of cells
			int r0,c0;
			if (!parse_cellreference(firstcell)) return;
			r0=atoi(rowstr.c_str())-1;
			c0=label_to_int(colstr);
			if (!parse_cellreference(lastcell)) return;
			for (int r=r0;r<atoi(rowstr.c_str());r++) for (int c=c0;c<label_to_int(colstr)+1;c++) cells[r][c].cval(val);
		}
		void cell(int r, int c, int val) {cells[r-1][c-1].cval(itoa(val));}
		void cell(const char *cellref,int val){
			if (!parse_cellreference(cellref)) return;
			cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].cval(itoa(val));
		}
		void cell(int r, int c, unsigned val) {cells[r-1][c-1].cval(itoa(val));}
		void cell(const char *cellref,unsigned val){
			if (!parse_cellreference(cellref)) return;
			cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].cval(itoa(val));
		}
		void cell(int r, int c, float val) {cells[r-1][c-1].cval(ftoa(val));}
		void cell(const char *cellref,float val){
			if (!parse_cellreference(cellref)) return;
			cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].cval(ftoa(val));
		}
		void cell(int r, int c, double val) {cells[r-1][c-1].cval(dtoa(val));}
		void cell(const char *cellref,double val){
			if (!parse_cellreference(cellref)) return;
			cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].cval(dtoa(val));
		}
		//void cell(int r, int c, long double val) {cells[r-1][c-1].cval(ldtoa(val));}
		//void cell(const char *cellref,long double val){
		//	if (!parse_cellreference(cellref)) return;
		//	cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].cval(ldtoa(val));
		//}
		//INDIVIDUAL CELL FORMATTING DON"T USE DEFAULT VALUES - CAUSES AMBIGUITY IN OVERLOADED FNS
		void cellcolour(int r, const char*col, Fl_Color clr)
			{cells[r-1][label_to_int(col)].colourx(clr);}
		void cellcolour(const char* row, int c, Fl_Color clr)
			{cells[atoi(row)-1][c-1].colourx(clr);}
		void cellcolour(const char *cellref,Fl_Color clr){
			if (!parse_cellreference(cellref)) return;
			cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].colourx(clr);
		}
		void cellcolour(const char * firstcell,const char * lastcell, Fl_Color clr) { //range of cells
			int r0,c0;
			if (!parse_cellreference(firstcell)) return;
			r0=atoi(rowstr.c_str())-1;
			c0=label_to_int(colstr);
			if (!parse_cellreference(lastcell)) return;
			for (int r=r0;r<atoi(rowstr.c_str());r++) for (int c=c0;c<label_to_int(colstr)+1;c++) cells[r][c].colourx(clr);
		}
		void bgcellcolour(int r, const char*col, Fl_Color clr)
			{cells[r-1][label_to_int(col)].colourx(clr);}
		void bgcellcolour(const char* row, int c, Fl_Color clr)
			{cells[atoi(row)-1][c-1].colourx(clr);}
		void bgcellcolour(const char *cellref,Fl_Color clr){
			if (!parse_cellreference(cellref)) return;
			cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].colourx(clr);
		}
		void bgcellcolour(const char * firstcell,const char * lastcell, Fl_Color clr) { //range of cells
			int r0,c0;
			if (!parse_cellreference(firstcell)) return;
			r0=atoi(rowstr.c_str())-1;
			c0=label_to_int(colstr);
			if (!parse_cellreference(lastcell)) return;
			for (int r=r0;r<atoi(rowstr.c_str());r++) for (int c=c0;c<label_to_int(colstr)+1;c++) cells[r][c].colourx(clr);
		}
		void cellfontsize(int r, const char*col, unsigned val)
			{cells[r-1][label_to_int(col)].sizex(val);}
		void cellfontsize(const char* row, int c, unsigned val)
			{cells[atoi(row)-1][c-1].sizex(val);}
		void cellfontsize(const char *cellref,unsigned val){
			if (!parse_cellreference(cellref)) return;
			cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].sizex(val);
		}
		void cellfontsize(const char * firstcell,const char * lastcell, unsigned val) { //range of cells
			int r0,c0;
			if (!parse_cellreference(firstcell)) return;
			r0=atoi(rowstr.c_str())-1;
			c0=label_to_int(colstr);
			if (!parse_cellreference(lastcell)) return;
			for (int r=r0;r<atoi(rowstr.c_str());r++) for (int c=c0;c<label_to_int(colstr)+1;c++) cells[r][c].sizex(val);
		}
		void cellfont(int r, const char*col, Fl_Font font)
			{cells[r-1][label_to_int(col)].fontx(font);}
		//void cellfont(const char* row, int c, Fl_Font font)
		//	{cells[atoi(row)-1][c-1].fontx(font);}
		void cellfont(const char *cellref,Fl_Font font,int size = FONTSIZE){
			if (!parse_cellreference(cellref)) return;
			cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].fontx(font);
			cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].sizex(size);
		}
		void cellfont(const char * firstcell,const char * lastcell, Fl_Font font) { //range of cells
			int r0,c0;
			if (!parse_cellreference(firstcell)) return;
			r0=atoi(rowstr.c_str())-1;
			c0=label_to_int(colstr);
			if (!parse_cellreference(lastcell)) return;
			for (int r=r0;r<atoi(rowstr.c_str());r++) for (int c=c0;c<label_to_int(colstr)+1;c++) cells[r][c].fontx(font);
		}
		void cellalign(int r, const char*col, Fl_Align aln)
			{cells[r-1][label_to_int(col)].alignx(aln);}
		void cellalign(const char* row, int c, Fl_Align aln)
			{cells[atoi(row)-1][c-1].alignx(aln);}
		void cellalign(const char *cellref,Fl_Align aln){
			if (!parse_cellreference(cellref)) return;
			cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].alignx(aln);
		}
		void cellalign(const char * firstcell,const char * lastcell, Fl_Align aln) { //range of cells
			int r0,c0;
			if (!parse_cellreference(firstcell)) return;
			r0=atoi(rowstr.c_str())-1;
			c0=label_to_int(colstr);
			if (!parse_cellreference(lastcell)) return;
			for (int r=r0;r<atoi(rowstr.c_str());r++) for (int c=c0;c<label_to_int(colstr)+1;c++) cells[r][c].alignx(aln);
		}
		void cellprec(int r, const char*col, int val)
			{cells[r-1][label_to_int(col)].precisionx(val);}
		void cellprec(const char* row, int c, int val)
			{cells[atoi(row)-1][c-1].precisionx(val);}
		void cellprec(const char *cellref,int val){
			if (!parse_cellreference(cellref)) return;
			cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].precisionx(val);
		}
		void cellprec(const char * firstcell,const char * lastcell, int val) { //range of cells
			int r0,c0;
			if (!parse_cellreference(firstcell)) return;
			r0=atoi(rowstr.c_str())-1;
			c0=label_to_int(colstr);
			if (!parse_cellreference(lastcell)) return;
			for (int r=r0;r<atoi(rowstr.c_str());r++) for (int c=c0;c<label_to_int(colstr)+1;c++) cells[r][c].precisionx(val);
		}
		//"a5" style lock, unlock, hide, unhide
		void celllock(int r, const char*col) {cells[r-1][label_to_int(col)].lock();}
		void celllock(const char* row, int c) {cells[atoi(row)-1][c-1].lock();}
		void celllock(const char *cellref) {
			if (!parse_cellreference(cellref)) return;
			cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].lock();
		}
		void celllock(const char * firstcell,const char * lastcell) { //range of cells
			int r0,c0;
			if (!parse_cellreference(firstcell)) return;
			r0=atoi(rowstr.c_str())-1;
			c0=label_to_int(colstr);
			if (!parse_cellreference(lastcell)) return;
			for (int r=r0;r<atoi(rowstr.c_str());r++) for (int c=c0;c<label_to_int(colstr)+1;c++) cells[r][c].lock();
		}
		void cellunlock(int r, const char*col) {cells[r-1][label_to_int(col)].unlock();}
		void cellunlock(const char* row, int c) {cells[atoi(row)-1][c-1].unlock();}
		void cellunlock(const char *cellref) {
			if (!parse_cellreference(cellref)) return;
			cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].unlock();
		}
		void cellunlock(const char * firstcell,const char * lastcell) { //range of cells
			int r0,c0;
			if (!parse_cellreference(firstcell)) return;
			r0=atoi(rowstr.c_str())-1;
			c0=label_to_int(colstr);
			if (!parse_cellreference(lastcell)) return;
			for (int r=r0;r<atoi(rowstr.c_str());r++) for (int c=c0;c<label_to_int(colstr)+1;c++) cells[r][c].unlock();
		}
		void cellhide(int r, const char*col) {cells[r-1][label_to_int(col)].hide();}
		void cellhide(const char* row, int c) {cells[atoi(row)-1][c-1].hide();}
		void cellhide(const char *cellref) {
			if (!parse_cellreference(cellref)) return;
			cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].hide();
		}
		void cellhide(const char * firstcell,const char * lastcell) { //range of cells
			int r0,c0;
			if (!parse_cellreference(firstcell)) return;
			r0=atoi(rowstr.c_str())-1;
			c0=label_to_int(colstr);
			if (!parse_cellreference(lastcell)) return;
			for (int r=r0;r<atoi(rowstr.c_str());r++) for (int c=c0;c<label_to_int(colstr)+1;c++) cells[r][c].hide();
		}
		void cellunhide(int r, const char*col) {cells[r-1][label_to_int(col)].unhide();}
		void cellunhide(const char* row, int c) {cells[atoi(row)-1][c-1].unhide();}
		void cellunhide(const char *cellref) {
			if (!parse_cellreference(cellref)) return;
			cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].unhide();
		}
		void cellunhide(const char * firstcell,const char * lastcell) { //range of cells
			int r0,c0;
			if (!parse_cellreference(firstcell)) return;
			r0=atoi(rowstr.c_str())-1;
			c0=label_to_int(colstr);
			if (!parse_cellreference(lastcell)) return;
			for (int r=r0;r<atoi(rowstr.c_str());r++) for (int c=c0;c<label_to_int(colstr)+1;c++) cells[r][c].unhide();
		}
		void cellwidth(const char * firstcol,const char * lastcol, int w) { //range of cells
			int c0,c1;
			c0 = label_to_int(firstcol);
			c1 = label_to_int(lastcol);
			for (int c=c0;c<c1+1;c++) col_width(c,w);//note effect earlier change of origin
		}
		//referring functions
		Cells cell(int r, const char*col) {return cells[r-1][label_to_int(col)];}//needs more thought
		//FETCHING FUNCTIONS
		string &cell(int r, int c)  {return temp=cells[r-1][c-1].cval();}
		string cell(string cellref) {
			if (!parse_cellreference(cellref)) return "";
			else return cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].cval();
		}
		const char * ccell(int r, int c) const {return cells[r-1][c-1].cval().c_str();}
		const char* ccell(const char * cellref) {// slowest for some reason!!!!
			if (!parse_cellreference(cellref)) return "";
			 return cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].cval().c_str();
		}
		const char * ccell(string &cellref) {//marginally fastest
			if (!parse_cellreference(cellref)) return "";
			else return cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].cval().c_str();
    	}
		double fcell(int r,int c) const {return atof(cells[r-1][c-1].cval().c_str());}
		double fcell(int r,const char *col) {return atof(cells[r-1][label_to_int(col)].cval().c_str());}
		double fcell(string &cellref) {
			if (!parse_cellreference(cellref)) return 0.;
			else return atof(cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].cval().c_str());
		}
		double fcell(const char * cellref) {
			if (!parse_cellreference(cellref)) return 0.;;
			return atof(cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].cval().c_str());
		}
		long double ldcell(int r,int c) const {return atof(cells[r-1][c-1].cval().c_str());}
		long double ldcell(int r,const char *col) {return atof(cells[r-1][label_to_int(col)].cval().c_str());}
		long double ldcell(string &cellref) {
			if (!parse_cellreference(cellref)) return 0.;
			else return atof(cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].cval().c_str());
		}
		long double ldcell(const char * cellref) {
			if (!parse_cellreference(cellref)) return 0.;;
			return atof(cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].cval().c_str());
		}
		int	   icell(int r,int c) const {return atoi(cells[r-1][c-1].cval().c_str());}
		int    icell(int r,const char *col) {return atoi(cells[r-1][label_to_int(col)].cval().c_str());}
		int    icell(string &cellref) {
			if (!parse_cellreference(cellref)) return 0;
			else return atoi(cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].cval().c_str());//NB -1 for row
		}
		int    icell(const char * cellref) {
			if (!parse_cellreference(cellref)) return 0;
			else return atoi(cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].cval().c_str());
		}
		//========================================
		//column and row collections
		void ccol_font(const char *col,int f) {for (int r=0;r<rows();r++) cells[r][label_to_int(col)].fontx((Fl_Font) f);}
		void rrow_font(const char *row,int f) {for (int c=0;c<cols();c++) cells[atoi(row)-1][c].fontx((Fl_Font) f);}
		void ccol_width(const char *col, int w) {col_width(label_to_int(col), w);} //overload for ease of col reference
		void rrow_height(const char *row, int h) {row_height(atoi(row)-1, h);} //overload for ease of col reference
		void ccol_align(const char *col,Fl_Align a) {for (int r=0;r<rows();r++) cells[r][label_to_int(col)].alignx(a);}
		void ccol_lock(const char *col){for (int r=0;r<rows();r++) cells[r][label_to_int(col)].lock();}
		void rrow_lock(const char *row){for (int c=0;c<cols();c++) cells[atoi(row)-1][c].lock();}
		void ccol_unlock(const char *col){for (int r=0;r<rows();r++) cells[r][label_to_int(col)].unlock();}
		void rrow_unlock(const char *row){for (int c=0;c<cols();c++) cells[atoi(row)-1][c].unlock();}
		void ccol_noformat(const char*col) {for (int r=0;r<rows();r++)  cells[r][label_to_int(col)].format(0); }
		void ccol_precision(const char*col, int p) {for (int r=0;r<rows();r++) cells[r][label_to_int(col)].precisionx(p); }
		void column(int c, string val){ //set data in nominated column
			for (int r=0;r<rows();r++) cells[r][c].cval(val);
		}
		//string to number functions
		long a_tol(const char* str) {//cant replace atoi without consequences
			char *endptr;
    		long val;
    		errno = 0;
    		val = strtol(str, &endptr,10);
    		//Check for some errors
    		if (endptr == str) return 0;
    		return val;
		}
		void apply_C_format(string cellref, string formatstring) {//not used so far
		//sets up the formatstring format to a cellref eg "a2","%4.1e"
			if (!parse_cellreference(cellref)) return;
			cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].format(0);
			cformatmap[cellref]= formatstring; //place in map
		}
		void apply_C_format(int r, int c, string formatstring) {//0 origin
		//sets up the formatstring format to a cellref eg 1,2,"%4.1e"
			string cn = convert_rc_address(r,c);
			cformatmap[cn]= formatstring;
		}
		void fill_drawingmap(int act, int rb,int cb,short no_rows,short no_cols,
			short typ = 1, string data="", short box_type = FL_FLAT_BOX,short line_style = 0,
			short line_width = 1, short fill_colour = localbackgroundcolor,
			short text_align = FL_ALIGN_WRAP, short text_size = FONTSIZE,
			short text_font = FONT, short text_colour = localtextcolor )		{

			cout <<"inside dwgmap\n";
			string c0 = convert_rc_address(rb,cb); //will be converted to mstring
			drawing spd;
			spd.norows = no_rows;
			spd.nocols = no_cols;
			spd.active = act;
			spd.type = typ;
			spd.boxtype =  box_type;
			spd.linestyle = line_style;
			spd.linewidth = line_width;
			spd.linecolour = line_colour;
			spd.fillcolour = fill_colour;
			spd.spare = 0;
			spd.textalign = text_align;
			spd.textsize = text_size;
			spd.textfont = text_font;
			spd.textcolour = text_colour;
			spd.dataholder = data;
			drawingmap.insert(make_pair(mstring(c0), drawing(spd)));
		}
		void show_all_multimapentries() {
			multimap <mstring,drawing>::iterator z;
			z = drawingmap.begin();
			while (z != drawingmap.end()) {
				cout << z->first << DL << z->second <<endl;
				z++;
			}
		}
		void display_all_multimapentries (int displayflag);
		void display_all_comments (int displayflag);
		void edit_move_multimapentries(int showonlyflag);
		void delete_all_multimapentries();
		void delete_all_comments();
	/*	void clear_all_drawingflags() {
			for (int r = 0; r < rows(); r++) for (int c = 0; c< cols(); c++) { cells[r][c].clear_drawingflag(); }
		}
		void reset_drawingflags() { //by going through multimap and setting a drawingflag if there is an entry
			clear_all_drawingflags();
			multimap <mstring,drawing>::iterator z;
			z = drawingmap.begin();
			stringstream strstr;
			while (z != drawingmap.end()) {
				string addr;
				strstr << z->first;
				addr = strstr.str();
				strstr.str("");
				parse_cellreference(addr);
				cells[atoi(rowstr.c_str())-1][label_to_int(colstr)].set_drawingflag();
				z++;
			}
		} */
		string convert_rc_address(int r, int c) {//inverse of parsecelladdress //0 origin arguments
			stringstream strstr;
			strstr << int_to_col_label(c,'a') << r + 1;//don't use '<<ends' as it puts a null on the end
			return strstr.str();	//20080429 change of index origin of arguments
		}

		string getlocaltime();
		long label_to_int(string &label);
		long label_to_int(const char *label);
		string int_to_col_label(long n, char startletter='A');
		inline const char* itoa(long val) {//convert an int into a char array
			static char s[BUFFER];
			snprintf(s,BUFFER-1,"%ld", val);
			return s;
		}
		inline char* ftoa(float val, int prec=18){
			//convert a float into a character array don't use long double not windows compatible
			//note prec of 18 will save all numbers with 18 digits after decimal point
			static char s[BUFFER];
			snprintf(s,BUFFER-1,"%.*f",prec,val);
			return s;
		}
		inline const char* dtoa(double val, int prec=18){	//convert a double into a character array don't use long double not windows compatible
			static char s[BUFFER];
			snprintf(s,BUFFER-1,"%.*f",prec,val); //bug in ver 1.0.1  "%L.f" !!!!! MAKE 18 to 6 (6 was 18 20080219)
			return s;
		}
/*		//and possibly superfluous or worse, non-portable
		inline const char* ldtoa(double val){	//convert a double into a character array don't use long double not windows compatible
			static char s[BUFFER];
			snprintf(s,BUFFER-1,"%.*f",18,val); //bug in ver 1.0.1  "%L.f" !!!!! MAKE 18 to 6 (6 was 18 20080219)
			return s;
		} */

		LONGLONG stoll(string &val);//convert a string to a long long value

		inline void lacell() {	//set bottom right edge of active table
			larow = lacol = 0;  //in case all empty or no textflags
			for (int r = rows()-1; r>=0; r--) {
				for (int c = cols()-1; c>=0; c--) {
					if (!cells[r][c].empty() || cells[r][c].get_drawingflag()) {
						larow = r;
						for (int c = cols()-1; c>=0; c--) {
							for (int r = larow; r>=0; r--) {
								if (!cells[r][c].empty() || cells[r][c].get_drawingflag()) {lacol = c; return;}
							}
						}
					}
				}
			}
		}

		string selection_expander (string &s);
		string var_evaluate(string &cellname);
		string evaluate(string &s, int r,int c);//this includes cell reference
		//void toggle_evaluateflag() {parserflag = !parserflag;}   20070823
		string &noleadspace(string &s)  {
			int j=0;while (s[j]==' ') j++;return s.erase(0,j);
		}
		string nospace(string s) { //eliminates all spaces from strings
			while (string::npos != (s.find(' '))) s.erase(s.find(' '),1);
			return s;
		}
		void row_col_size_compact(string &s, char delim, int rowcol, int type);
		void compact(string &s, char delim, char _mult, int r, int c, int type);
		void uncompact(string &s, char delim, char _mult);
		void textcolor(int attr = BRIGHT, int fg = WHITE, int bg = BLACK);
		void defaultview();
		void newformat (int &r, int &c, string &number);
		const char * displayedvalue(int &r, int &c);
		void adjustcolumnwidth(int &c);	//adjust column width automatically
		void adjustrowheight(int &r);
		int parse_cellreference(mstring cellref);
		int parse_cellreference(string &cellref);
		int parse_cellreference(const char * cellref);
		string relcellseq(string & celladdress, int coladjust, int rowadjust);
		void fill_down(int r,int c);
		void fill_down_with_numbers(int r, int c);
		void fill_right(int r,int c);
		void fill_right_with_numbers(int r, int c);
		double rounddouble(double doValue, int nPrecision);
		//void save_cells(int sheetno=0, int version =0);
		//void get_cells(int sheetno=0, int vdf);
   		void set_savefilename();
		void set_get_file(int no);
		void saveas(int i);
		void tokenise(string &t, char delim);
		void newtokenise(string &t, unsigned &nooftokens, char delim);
		string get_token(int n);
		void savedata(int silent);
		long filesize(const char * fn);
		int lzwcompress(const char * fn);
		int lzwuncompress(const char * fn);
		void printdata();  //produces a comma delimited clean file for export to a printer capable pgm (until I work out how to print directly!!!)
		void getdata(int allsheets = 0);
		void save_sheet(int sheetno =0);
		void get_sheet(int sheetno =0);
		int GetIndexByName(Fl_Menu_Bar* menubar, const char *findname);
		Fl_Menu_Item * GetMenuItemByName(Fl_Menu_Bar* menubar, const char *findname);
		void SetRadioByName(Fl_Menu_Bar *menubar, const char *menuname);
		void create_sheet_menu(Fl_Menu_Bar* menubar);
		void change_visible_sheet_number(Fl_Menu_Bar * menubar, int N);
		string get_sheet_name(int sheetno) { return sheetname[sheetno];}
		void create_sheet_names();
		void change_sheet_name(string& sn, int cs);
		void adjust_viewscale(float value);
		void reset_viewscale();
		void showsizes() {
			lconv * lc = localeconv();
			cout <<"size of Cells is "<<sizeof (Cells);
			cout <<"; drawing struct is "<<sizeof (drawing);
			cout <<"; Spreadsheet is "<<sizeof(Spreadsheet)<<endl;
			cout <<"short is "<<sizeof(short);
			cout <<"; int is "<<sizeof(int);
			cout <<"; long is "<<sizeof(long);
			cout <<"; double is "<<sizeof(double);
			cout <<"; string is "<<sizeof(string)<<endl;
			cout <<"RAND_MAX is "<<RAND_MAX<<endl;
			cout <<"currency symbol is "<<*lc->currency_symbol<<endl;
		}
		//void intfn(int,int);
		//void assign_intdata( void (*intfn)(int,int) );
 //...............................................................................
		static int Rcurrent;//currently active row 
		static int Ccurrent;
		static char _buffer[BUFFER];
		//static char formatstring[32]; 
		static Fl_Font  localfont;
		static Fl_Color localtextcolor;
		static Fl_Color hdrtextcolor;
		static Fl_Color hdrcolor;
		static Fl_Color localbackgroundcolor;
		static Fl_Color selectioncolor;
		static Fl_Color tablecolor;
		static int		s_left, copy_s_left,
						s_top, copy_s_top,
						s_right,copy_s_right,
 						s_bottom, copy_s_bottom;		// kb nav + mouse selection see also Rcurrent,Ccurrent
		static string range;
		static int larow;				//last row with any non blank content
		static int lacol;				//last col with any non blank content
		static int eval_counter;				//for testing
		static int var_eval_counter;
   		static string savefilename;
		static string printfilename;
		static string _saveddatastring;
		static string temp;
		static const char* ctemp;
		static int col_labeltype;
		static list <string> datatoken; //used in getdata
		static int errorflag;
		static char decimalpoint;
		static char thousandsmark;
		static int colourneg; //colour negative numbers
		static int showplus;
		static char currencymark;
		static int numberisneg;
		static int showthousands;
		static int showcurrency;
		static int coutflag;
		static float widthscalefactor;
		static float heightscalefactor;
		static int viewscale;
		static char datadelimiter;
		static char textmultiplier;
		static char numbermultiplier;
		static char endofsheetmarker;
		static string colstr;//used in cell sequencing to save a column name
		static string rowstr;
		static int norows;
		static int nocols;
		static int nosheets;
		static int currentsheet;
		static int oldsheet;
		static string temptest;
		static int Row1;
		static int Col1;
		static int _restart;
 		static int text_count;
 		static int savetext_count;
 		static int showdrawings;
 		static int compressdata;
 		static int rowcolformat;
		static int line_style;
		static int line_width;
		static int line_colour;
		static int fill_colour;
};
//===========================a==========================

SDRarray2D <Cells> Spreadsheet::clipboard(1,1); //resized when used
SDRarray2D <Cells> Spreadsheet::cells(1,1);
SDRarray <string>  Spreadsheet::sheet(1);
SDRarray <string>  Spreadsheet::sheetname(1); ////len =16 maximum
SDRarray <Fl_Menu_Item *> Spreadsheet::shptrs(1);//this saves ptrs to facilitate sheet name change
Cells Spreadsheet::dummycell;
map <mstring,string> Spreadsheet::cformatmap;
multimap <mstring,drawing> Spreadsheet::drawingmap;
deque <string> Spreadsheet::undoqueue; //for use with input undo function   20070826

char		Spreadsheet::_buffer[BUFFER];   //used for text output to draw fn
int			Spreadsheet::Rcurrent=0;
int 		Spreadsheet::Ccurrent=0;
Fl_Font		Spreadsheet::localfont 		    = FONT;//(Fl_Font) 0 typically Helvetica
Fl_Color 	Spreadsheet::localtextcolor 	= TEXTCOLOR;
Fl_Color 	Spreadsheet::hdrtextcolor 		= HDRTEXTCOLOR;
Fl_Color  	Spreadsheet::hdrcolor 			= HDRCOLOR;
Fl_Color  	Spreadsheet::localbackgroundcolor = BACKGROUNDCOLOR;
Fl_Color  	Spreadsheet::selectioncolor		= SELECTIONCOLOR;
int 		Spreadsheet::s_left 	= 	-1;
int 		Spreadsheet::s_right 	= 	-1;
int 		Spreadsheet::s_top 	= 	-1;
int			Spreadsheet::s_bottom	= 	-1;
int 		Spreadsheet::copy_s_left  = -1;
int 		Spreadsheet::copy_s_right = -1;
int			Spreadsheet::copy_s_top   = -1;
int			Spreadsheet::copy_s_bottom= -1;
string		Spreadsheet::range    =    "";
int 		Spreadsheet::larow 	= 	0;
int 		Spreadsheet::lacol    = 	0;
int 		Spreadsheet::eval_counter 	=	0;
int			Spreadsheet::var_eval_counter = 0;
int 		Spreadsheet::col_labeltype = 1;  //uc - numeric -s 0
string		Spreadsheet::savefilename = "default.sps";
string		Spreadsheet::printfilename = "export.csv";
string 		Spreadsheet::_saveddatastring = "";
string		Spreadsheet::temp="";
const char *Spreadsheet::ctemp=0;
list <string> Spreadsheet::datatoken;
int			Spreadsheet::errorflag=0;
char		Spreadsheet::decimalpoint='.';
char		Spreadsheet::thousandsmark=',';
char		Spreadsheet::currencymark; //default set in constructor using locale fn can override here
int			Spreadsheet::numberisneg=0;
int	  	Spreadsheet::colourneg=1;
int	  	Spreadsheet::showplus=0;
int	  	Spreadsheet::showthousands=1;
int	 	Spreadsheet::showcurrency=0;
int		Spreadsheet::coutflag=1;
int	 	Spreadsheet::viewscale=100;
float  Spreadsheet::widthscalefactor=5.; //fudges - works with default font & size decrease to increase width provided
float  Spreadsheet::heightscalefactor=11.;//by autocolumnwidth() and autorowheight()
char  Spreadsheet::datadelimiter=DL; //use this in save and getdata to free up ',' for parser etc
char  Spreadsheet::textmultiplier=TM;
char  Spreadsheet::numbermultiplier=NM;
char  Spreadsheet::endofsheetmarker=ES;
string	  Spreadsheet::colstr="";//used in cell sequencing to preserve a column name
string	  Spreadsheet::rowstr="";
int	  Spreadsheet::nosheets=5;//these are the sheets actually shown in the sheet menu can be reset in main argument
int	  Spreadsheet::norows=ROWS;
int	  Spreadsheet::nocols=COLS;
int   Spreadsheet::currentsheet=0;
int   Spreadsheet::oldsheet=0;
string	  Spreadsheet::temptest="12345678";
int	  Spreadsheet::Row1=1;
int	  Spreadsheet::Col1=1;
int   Spreadsheet::_restart=1;
int   Spreadsheet::text_count=0; //overall count to save searching if none required
//int   Spreadsheet::savetext_count=0;
//int   Spreadsheet::drawing_count=0; //overall count to save searching if none required
//int   Spreadsheet::savedrawing_count=0;
int	  Spreadsheet::showdrawings=1;
int   Spreadsheet::compressdata=1;
int   Spreadsheet::rowcolformat=0; //default is colum/rows;
//int   Spreadsheet::merge_or_readflag=1; //default is to merge contents of cells
int   Spreadsheet::line_style = FL_SOLID;
int	  Spreadsheet::line_width = 2;
int   Spreadsheet::line_colour = FL_BLUE;
int   Spreadsheet::fill_colour = FL_WHITE;
//Fl_Button Spreadsheet::sheetbuttons[10];

//================================= =====================needed here by (ia) Parser
static 	  Spreadsheet *ss = 0;
//=======================================================





// Constants for use with spreadsheet
const char* VERSION = "sprsht v 1.2.2";
//first nunber major rewrite, second new feature (eg changed data input), last minor improvements and bug fixes

typedef double Ptype; //for use with numeric parser change here if required

const char DL = '`';//data delimiter
const char TM = 0x1F;//textmultiplier (compact fns )
const char NM = '*';//number multiplier
const char ES = '|';//end of sheet marker
const long COLS = 	50;//set default size here but no rows, cols can be changed dynamically
const long ROWS = 	100;
const int MAXCELLS = 1000000;  // pending a better way of limiting allocation errors and speed
const int BUFFER = 	255;   //limits max size of input and therefore of strings
const Fl_Font FONT 			= (Fl_Font) 0;
const Fl_Font SHEETNAMEFONT = FL_HELVETICA_BOLD_ITALIC;
const int FONTSIZE 			= 12;
const int SHEETNAMEFONTSIZE = 16;
const Fl_Align ALIGN 			= FL_ALIGN_RIGHT;
const Fl_Color TEXTCOLOR 		= FL_DARK_BLUE;
const Fl_Color HDRTEXTCOLOR 	= FL_WHITE;
const Fl_Color HDRCOLOR 		= (Fl_Color) 45;
const Fl_Color BACKGROUNDCOLOR 	= FL_WHITE;
const Fl_Color SELECTIONCOLOR 	= FL_YELLOW;
const int COLHDRHEIGHT= 18;
const int COLWIDTH		= 80;
const int ROWHEIGHT 	= 25;
const int WIDTH 		= 10; //format width specifier
const int PRECISION	= 2;
const int UNDOLEVEL = 10; //for undoqueue below 20070826
const int SHEETNAMESIZE = 64;
const int SHEETNAMEBOXHEIGHT = 14;

#define  SCROLLBAR_SIZE 16  //used by fl_table

//following used to colour console output
//see Pradeep Padala on Tue, 2005-10-11 01:00. www.linuxjournal.com/article/8603
#define RESET           0
#define BRIGHT          1
#define DIM             2
#define UNDERLINE       3
#define BLINK           5 //was 4
#define REVERSE         7
#define HIDDEN          8
#define BLACK           0
#define RED             1
#define GREEN           2
#define YELLOW          3
#define BLUE            4
#define MAGENTA         5
#define CYAN            6
#define WHITE           7
#define TYPESSTART		500
#define MERGE 			501
#define PIXMAP			504
#define PICTURE 		508
#define COMMENT			512
#define FLOATINGCOMMENT 513
#define HORIZLINE		520
#define VERTLINE		522
#define RECTANGLE		524
#define TYPESEND		530
#define HIDEALL			550
#define SHOWALL			551
#define SHOWCOMMENTS    552
#define HIDECOMMENTS	555
#define DELETEALL		576
#define DELETECOMMENTS  580
#define LISTALL			597
#define	SHOWMAP			598
#define EDITMAP			599

using namespace std;
#ifndef __GNUC__
typedef __int64 LONGLONG;
#else
typedef long long LONGLONG;
#endif // __GNUC__

static int  previous_function= 1,
 			current_function= 0;
int 		windowW = 1200,
			windowH = 680;

#define DEMOSHEET1 "Sheet 1   `1` -name and sheetno\n2------------\n15`11` , active rows/cols;\n95`255`255`45`?`25`80`40`18`.`c`$`5`11`100`?`?`?`?`?` - current settings\n80`11`\n25`15`\ncell contents and properties\ntestdraw.sps`1``164`\n8`165`\n0`165`\n12`165`\n136`165`\n109`165`\n0`22`16`1`0`27`16`1`0`53`16`1`0`51`16`1`0`8`\n255`165`\n2`165`\nC-formats - continuous\n0\ndrawings - one per line\n4\nd6`3`4`1`512`19`0`1`216`-16640`0`128`28`3`5`Now is the time for all good men to come to the aid of the party!!`\nk7`2`2`1`501`1`0`1`216`255`0`128`8`0`136`0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 `\nb8`7`3`1`504`19`0`1`216`255`0`128`18`0`6`Edwin aged 10months`\ng15`1`1`1`512`19`0`1`216`-16640`0`128`12`0`136`standard size comment`\nend of data\n"

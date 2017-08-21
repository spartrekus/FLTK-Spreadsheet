//sprsht v1.2.2
/*
copyright norman bakker (2007) */
//equations for determining steady state flow in (gas) pipeline networks

//GLOBAL VARIABLES 

static int nosegs=0;
static int nosectns=0;
static int 
	rowoffset= 0,
	coloffset=0,
	//flagc=0,
   	identc=	0,
	namec=	0,
	kpc=0,
	pupc= 	0,
	pdownc=	0,
	flowc=	0,
	brflowc=0,
	diamc=  0,
	dragfc= 0,
	roughc=	0,
	lengc=  0,
	tabsc=	0,
	z_calc=	0,
	el_data=0,
	el_calc=0,
	del_calc=0,
	densc=0,
	headc=0,
	ray0=0,
	carray0=1, 
	S0=0,
	Sid=0,//run array column assignments
	Sno=0,
	Stype=0,
	Sstart=0, 
	Send=0,
	Sfrom=0,
	Sto=0,
	Scon1=0,
	Scon2=0, //loop start and end for cont line use
	Sits=0,
	Spracc=0,
	Sflacc=0,
	SOK=0,
	run_OK=1;   
//static double 	flow,pu,pd,diam,rough,dragf,leng,deltah,tabs,sg,eta,n2,co2,hhv,z ;//keep previous value unless new data
const int RUNSMAX = 50;
static double 	upstr_flow, loopee_pu, loopee_pd, looper_pu, looper_pd, main_pu, sum_ringmain,sum_main, p_in, p_out;
static double 	flow_min,local_flow,total_length,xl_minimum,xh_maximum,diam0,diamn,xl,xh, it_tol;
static int 			current_seg,segstart,segend,rowfrom,rowto,stype,iterationcount,flag=0;
static SDRarray	<int> arrayindex(1);//arrayindex for process array sorting
static SDRarray2D <string>	tempstr(1,1);//set up temp strings for transfer in sorting 
static SDRarray <int> data_row(1);
static SDRarray <int> iteration_flag(1);
static SDRarray <double> distance_from_inlet(1);
static SDRarray <double> elevn_data(1);
// elevn is absolute elevation (if provided) deltah (elevn change) has to be determined for each pipe element before
// calculation

//HEADERS FOR FNS USED HERE
int 	count_sections();
//int count_segments();  
void 	clean_secID();
void 	setup_Gas();
void 	setup_Pipe();
int 	setup_elevations();
void 	errormessage(int n);
void 	place_runarray_titles();
void 	pipe_network(int restart);
void 	setup_data(int thisrow);
void 	show_data();
void 	setup_segment_data(int _s);//all current segment
void 	setup_all_data();//all rows
int 	count_segments();
int 	find_segments();
int 	find_which_segment(int r);
string 	&get_segment_ID (int row); 
int 	analyse_runarray(int atrow);
void 	show_runarray(int nosegs);
//void 	process_rarray (int atrow);
int 	find_ID_row (string &IDnn);
void 	rebuild_runarrayrow_index(int s, int s1);
void 	rearrange_by_index();
inline 	void move_a_runarray_row(int rowfrom, int rowto);
void	swap_runarray_rows(int first, int second);
inline 	void erase_a_runarray_row(int row);
int 	remove_dummy_runarray_segments(int nosegs);
int 	sort_runarray_old(int key,int nos);
void 	sort_runarray(int key);
int		move_below_range();
void 	rearrange_by_index();
void 	cut_cells(int top,int bottom,int left,int right);
void 	copy_cells(int top,int bottom,int left,int right);
void 	paste_cells(int top,int left);
void 	showdata(int thisrow);
void 	checkdata(double &pu , double &flow );
int 	checkrun();
void 	mainpressrun();
double 	sum_of_segment_off_flows(int segstart, int segend);
double 	diameter_adjusted_total_length(int segstart, int segend);
void 	next_pressure_down(int thisrow, double _pu, double _flow);
void 	next_pressure_down_reverse_flow(int thisrow, double _pd, double _flow);
void 	next_pressure_up(int thisrow, double _pd, double _flow);
double 	loopfn(double looper_flow);
double 	ringmainfn(double _flow); 
double 	pressfn_positive_flow(double _flow);
double 	pressfn_negative_flow(double flow);
int 	ifunc(int);
double 	dfunc(double);//double function pointer
int 	zbrac(double (*dfunc)(double), double *x1, double *x2, double *xl_minimum, double *xh_maximum);
double 	zbrent(double (*dfunc) (double),double x1,double x2,double tol);
void 	runarray_sort();

void pipe_network() {
/*
	ss->cell("a14","Clear sheet and reset y/n?");
	if (ss->cell("b14")[0]=='y' || ss->cell("a14")[0]=='Y') { 
		ss->clearallcells();
		cout<<"restart is "<<ss->_restart<<" COLD START of pipe network\n";
	} */
	//if (current_function != 812 || current_function !=811) ss->clearallcells();
	ss->sheetname[0]="pipe network";
	setup_Gas();
	setup_Pipe();
	ss->cell("a9","New runarray y/n?");
	//ss->cell("b9","Y"); //runarray relocated down past last section
	//ss->parse_cellreference("a9");	
	//double xxx = atold("1.23l");
	ss->cell("a10","sectns");
	ss->cell("a11","segs");
	ss->cell("a8","iteration tolerance");
	if (ss->cell("b9")[0]=='y' || ss->cell("b9")[0]=='Y') 
	{
	//allow rarray processing to be bypassed - facilitates manual adjustment if desired
	if (!count_sections()) return;//no sections found
	ray0 = 9+nosectns; 
	if (ray0<20) ray0=20;//keep below row 20
	nosegs = count_segments(); //includes dot segments but these will be ignored later
	if (!nosegs) return; //segment error, probably non-unique ID other than '.'
	for (int s = 0;s<nosegs*2;s++) for (int c=S0;c<SOK+1;c++) ss->cells[ray0+s-1][c-1].cval("");//clear previous runarray
	place_runarray_titles();
	nosegs = find_segments();//segment error, probably non-unique ID other than '.' can have crept in between runs
	if (!nosegs) return;
	
	int result = analyse_runarray(ray0);
	if (!result) return;
	
	runarray_sort(); //do a sort to ensure all from and to are in correct segment order in runarray
	
	if (!setup_elevations())  {cout << "elevation error\n";return;}
	
	ss->cell("b10",nosectns);
	ss->cell("b11",nosegs);
	}
	else 
	{
		nosectns=ss->icell("b10");
		nosegs=ss->icell("b11");
		if (!setup_elevations()) {cout << "elevation error\n";return;}
	}

	//show_types();//show segment types in runarray
	
	_SG  = ss->fcell("b2"); 	if (_SG  == 0.)  {_SG  = 0.625;ss->cell("b2",_SG  );}
	_N2 =	ss->fcell("b3"); 	if (_N2 == 0.)  {_N2 = 1.0;ss->cell("b3",_N2 );}
	_CO2 = ss->fcell("b4");		if (_CO2 == 0)  {_CO2 = 2.0;ss->cell("b4",_CO2 );}
	_Eta  = ss->fcell("b5")/1e6;if (_Eta  == 0.) {_Eta  = 11;ss->cell("b5",_Eta  );}
	_HHV = ss->fcell("b6");		if (_HHV == 0.) {_HHV = 39.;ss->cell("b6",_HHV );}
	ss->apply_C_format("b8","%5.2e");
	it_tol = ss->fcell("b8");	if (it_tol == 0.) it_tol = 1e-5; ss->cell("b8",it_tol);
	//set up known data in each section
	setup_all_data();//do this here to allow changes on the sheet between runs 
//}
	if (!ss->_restart) cout <<" Normal continuation of pipeline network with number of segments = "<<nosegs<<'\n';
	if (!nosegs) return;
	clock_t t,t1;
    t = clock();
	mainpressrun();
	t1 = clock() -t;
	cout << "mainpressrun " << t1 << " clocks\n";
	ss->cell(ray0+nosegs+1,Scon2,"Cycles");
	ss->cell(ray0+nosegs+1,Sits,(int)t1);
}

void setup_all_data() {
	//read in variables all rows 
	for (int r=rowoffset;r<nosectns+rowoffset;r++) {
		if (ss->cell(r,identc)[0]=='.') continue;
		setup_data(r);
	}
}
	
int compare(string &str1,string &str2) {
	unsigned sz=str1.size();
	if (str2.size()<sz) sz=str2.size();
	for (unsigned n=0;n<sz;n++) {
		if (!str1[n]==str2[n]) return 0;
		}
	return 1;
}

void setup_Gas() {
//provides a standard display of input variables defining gas properties
	ss->cell("a2","S.G." ); 		ss->cell("c2","-");			ss->cellprec("b2",3);
	ss->cell("a3","N2" ); 			ss->cell("c3","%v/v");
	ss->cell("a4","CO2" ); 			ss->cell("c4","%v/v");		
	ss->cell("a5","dyn viscosity" );ss->cell("c5","microPa/s");
	ss->cell("a6","HHV" ); 			ss->cell("c6","MJ/m3");		ss->cellprec("b3","b6",2);
	ss->cellcolour("a2","c6",ss->localtextcolor);
	ss->cellalign("a2","c6",FL_ALIGN_RIGHT);
	cout <<"setup_Gas\n";
//data is in "b"
}

void setup_Pipe() {
	//provides a standard display of mainly input variables except for Pout and flow
	//Pout is calculated, flow is inherited from s element (start) or an inlet or outlet type
	//ss->cell("d2","flag");
	ss->cell("e2","ident");//ident of network item pipe, offtake, etc
	ss->parse_cellreference("e2"); //if not starting in h2 reset rowoffset and coloffset
	rowoffset = 2 + atoi((ss->rowstr.c_str())); //in 1 origin!!
	coloffset = 1 + ss->label_to_int(ss->colstr);
	cout << "row and col offset " <<rowoffset<<' '<<coloffset<<endl;
	//flagc=	coloffset-1;
   	identc=	coloffset;
	namec=	coloffset+1;
	kpc=	coloffset+2;
	pupc= 	coloffset+3;
	pdownc=	coloffset+4;
	flowc=	coloffset+5;
	brflowc=coloffset+6;
	diamc=  coloffset+7;
	dragfc= coloffset+8;
	roughc=	coloffset+9;
	lengc=  coloffset+10;
	tabsc=	coloffset+11;
	z_calc=	coloffset+12;
	el_data=coloffset+13;
	el_calc=coloffset+14;
	del_calc=coloffset+15; 
	densc=	coloffset+19;//temp for testing
	headc=  coloffset+20;
	ss->cell("f2","name");//unique name
	ss->cell("g2","kp");//distance measure from start of pipeline
	ss->cell("h2","Pup");		ss->cell("h3","kPag");
	ss->cell("i2","Pdown");		ss->cell("i3","kPag");
	ss->cell("j2","flow");		ss->cell("j3","sm3/hr");//alternatively TJ/day
	ss->cell("k2","brflow");	ss->cell("k3","sm3/hr");
	ss->cell("l2","diam");		ss->cell("l3","mm");
	ss->cell("m2","dragf");		ss->cell("m3","-");
	ss->cell("n2","rough");		ss->cell("n3","micron");
	ss->cell("o2","length ");	ss->cell("o3","km");
	ss->cell("p2","temp");  	ss->cell("p3","dC");
	ss->cell("q2","z");   		ss->cell("q3","-");
	ss->cell("r2","el data");	ss->cell("r3","m");
	ss->cell("s2","el calc");	ss->cell("s3","m");
	ss->cell("t2","deltah");	ss->cell("t3","m");
	ss->cell("x2","density");	ss->cell("x3","kg/m3");//temp for testing
	ss->cell("y2","head");		ss->cell("y3","kPa");//temp for testing
	ss->cellwidth("a","a",115);
	ss->cellwidth("b","b",60);
	ss->cellwidth("c","c",60);
	ss->cellwidth("f","z",50);
	ss->cellwidth("d","e",60);
	ss->cellwidth("j","k",60);
	cout <<"setup_Pipe\n";

	//set preferred precisions (using cells so indexorigin is 0
	for (int r=rowoffset-1;r<ss->rows();r++) {
		ss->cells[r][kpc-1].precisionx(0);
		ss->cells[r][pupc-1].precisionx(0);
		ss->cells[r][pdownc-1].precisionx(0);		
		ss->cells[r][flowc-1].precisionx(0);
		ss->cells[r][brflowc-1].precisionx(0);
		ss->cells[r][diamc-1].precisionx(1);		
		ss->cells[r][dragfc-1].precisionx(2);		
		ss->cells[r][roughc-1].precisionx(0);	
		ss->cells[r][lengc-1].precisionx(1);		
		ss->cells[r][tabsc-1].precisionx(0);		
		ss->cells[r][z_calc-1].precisionx(3);
		ss->cells[r][el_data-1].precisionx(0);
		ss->cells[r][el_calc-1].precisionx(0);
		ss->cells[r][del_calc-1].precisionx(0);	
		ss->cells[r][densc-1].precisionx(2);	//temp for testing
		ss->cells[r][headc-1].precisionx(2);	//temp for testing
	}	
}

int setup_elevations() {
//in nominated segment
/*
Elevation above sealevel has to be provided as data so any elevation difference along a pipe section 
can be determined (deltah). If not given in the very first cell no elevation data will be assumed (deltah=0 
and elevation=0 unless the segment is connected to another pipe at both or either ends for which data has been set.

Since we need the difference in the elevations between the inlet and the outlet of a pipe to derive delta H
(deltah) an n section pipe would be specified with n+1 elevations. Providing the last point is untidy, but
to get around this for the moment, the elevation at the outlet of the last pipe in the segment should be provided in 
a following dot segment (dummy) under el_data.

However, where the last section (AS SHOWN ON THE SHEET) joins another pipe (ie has an sto or an scon2 value ) 
with a known elevation the latter's elevation the one it should use, but it is the outlet of this last section that should take the elevation of the pipe joined too. 

If an elevation value is given in the first cell and none in any following all pipesection elevations 
will be set to be the same. If a pipe connects to another it will take its inlet/outlet elevation from that pipe. 

Elevations between pipesections, where not given as data, are interpolated linearly on distance between a start and end point.
ALL ELEVNS ARE ASSUMED TO BE AS AT THE START OF A PIPE SECTION AND THE DETERMINATION OF DELTAH ALWAYS PROCEEDS FROM TOP TO BOTTOM.

Must be run after at least analyse_runarray and preferably after process_rarray as it needs Sfrom,Sto scon1 and Scon2 data 
as appropriate
*/  
	double elev_in,elev_out,elevr,delelevn, len, len_total;
	int sfrom,sto,scon1,scon2,sstart,send, stype;
	int ok, ok_overall;
	ok_overall=1;
	for (int seg=0;seg<nosegs;seg++) {
		ok = 1;
		setup_segment_data(seg);
		stype = abs(ss->icell(ray0+seg,Stype));
		sfrom = ss->icell(ray0+seg,Sfrom);
		sto = ss->icell(ray0+seg,Sto);
		sstart = ss->icell(ray0+seg,Sstart);
		send = ss->icell(ray0+seg,Send); 
		scon1 = ss->icell(ray0+seg,Scon1);
		scon2 = ss->icell(ray0+seg,Scon2);
		//cout<<"seg type start end from to con1 con2\n";
		//determine where inlet and outlet data on elevation may come from
		if (stype<20) {
			sto = send+1; //an open ended segment
		}
		else if (scon1 && scon2) {
			sfrom = scon1;
			sto = scon2;
		}
		
		//cout<<seg+1<<' '<<stype<<' '<<sstart<<' '<<send<<' '<<sfrom<<' '<<sto<<' '<<scon1<<' '<<scon2 <<endl;	
		elev_out = (ss->fcell(sto,el_data)>0.) ? ss->fcell(sto,el_data) : ss->fcell(sto,el_calc);		
		elev_in = (ss->fcell(sfrom,el_data)>0.) ? ss->fcell(sfrom,el_data) : ss->fcell(sfrom,el_calc);//to zero set el_data = 0.001 say
		
		//can safely copy elev-in to beginning point in this segment (as we now know it)
		ss->cell(sstart,el_data,elev_in);
		ss->cell(sstart,el_calc,elev_in);
		//must assign elev_out later
	
		int count = 0;
		
		data_row.length(2 + send - sstart); //max size of datapoints is all points in segment (last row uses elev_out for deltah)
		distance_from_inlet.length(2 + send -sstart);
		elevn_data.length(2 + send -sstart);
	
		
		data_row[0] = sstart; //always  include
		distance_from_inlet[0] = 0; //length of sections between data points (for interpolation)
		elevn_data[0] = elev_in;
		len =  0; 
		for (int r=sstart+1;r<send;r++) { //find where, if any data rows are
			len += ss->fcell(r,lengc);
			elevr = ss->fcell(r,el_data);
			if (elevr!=0.) { //if elevn is really zero make a very small no to override this
				count++;
				data_row[count] = r;
				elevn_data[count] = elevr;
				ss->cell(r,el_calc,elevr);
				distance_from_inlet[count] = len;
			}
			
		}
		count++;//always add end row
		data_row[count] = send; //always include
		elevn_data[count] = ss->fcell(send,el_data);//may contain nothing
		ss->cell(send,el_calc,elevn_data[count]);
		len_total=0.;
	
		for (int r=sstart;r<send;r++) len_total+= ss->fcell(r,lengc);
		distance_from_inlet[count]= len_total;
		
		cout<< "row  distance  elevation    in segm "<< seg+1<<endl;
		for (int n=0;n<count+1;n++) {
			cout << data_row[n]<<' '<< distance_from_inlet[n];
			cout<<' '<<elevn_data[n]<<endl;
		}
		//now determine elevn down section (linear interpolation)
		for (int n=0;n<count;n++) {
			len = 0.;
			len_total = distance_from_inlet[n+1]-distance_from_inlet[n];
			delelevn = elevn_data[n+1]-elevn_data[n];
			for (int r=data_row[n]+1;r<data_row[n+1];r++) {
				len += ss->fcell(r,lengc);
				elevr = elevn_data[n] + (delelevn * len/len_total);
				cout << "row len len_total elevr "<<r<<' '<<len<<' '<<len_total<<' '<< elevr<<endl;
				ss->cell(r,el_calc,elevr);
			}
		}
		//provide a correct deltah 
		for (int r=sstart;r<send;r++) ss->cell(r,del_calc,ss->fcell(r+1,el_calc) - ss->fcell(r,el_calc));
		delelevn = elev_out - ss->fcell(send,el_calc);//last section only
		ss->cell(send,del_calc,delelevn);
		if (!ok) ok_overall = ok;
	}
	return ok_overall;
}

void errormessage(int n=0) {
	ss->temp=ss->temp + ss->itoa(n);
	fl_message("%s",ss->temp.c_str());
}
void place_runarray_titles() {
//below rest of data
	S0=carray0;
	Sid=carray0+1;//run array column assignments
	Sno=carray0+2;
	Stype=carray0+3;
	Sstart=carray0+4; 
	Send=carray0+5;
	Sfrom=carray0+6;
	Sto=carray0+7;
	Scon1=carray0+8;
	Scon2=carray0+9;
	Sits=carray0+10;
	Spracc=carray0+11;
	Sflacc=carray0+12;
	SOK=carray0+13;//current end
	//Spec1=carray0+12;
	//Spec2=carray0+13;
	//Spec3=carray0+14;
	//Sxl=carray0+15;
	ss->cells[ray0-3][S0-1].cval("RUN");
	ss->cells[ray0-3][Sid-1].cval("ARRAY");
	ss->cells[ray0-3][Sfrom-1].cval("Inlet");
	ss->cells[ray0-3][Sto-1].cval("Outlet");
	ss->cells[ray0-3][Scon1-1].cval("");
	ss->cells[ray0-3][Scon2-1].cval("");	
	ss->cells[ray0-2][S0-1].cval("Segment type");
	ss->cells[ray0-2][Sid-1].cval("Ident");
	ss->cells[ray0-2][Sno-1].cval("Segment No");
	ss->cells[ray0-2][Stype-1].cval("Type no");
	ss->cells[ray0-2][Sstart-1].cval("Start");
	ss->cells[ray0-2][Send-1].cval("End");
	ss->cells[ray0-2][Sfrom-1].cval("from");
	ss->cells[ray0-2][Sto-1].cval("to");
	ss->cells[ray0-2][Scon1-1].cval("conn 1");
	ss->cells[ray0-2][Scon2-1].cval("conn 2");
	ss->cells[ray0-2][Sits-1].cval("iterations");
	ss->cells[ray0-2][Spracc-1].cval("P diff");
	ss->cells[ray0-2][Sflacc-1].cval("Fl diff");
	ss->cells[ray0-2][SOK-1].cval("Run OK");
	//
	cout <<"place_runarray_titles\n";
}
void setup_segment_data(int _s) {
	//read in variables all rows in current segment
	for (int r=ss->icell(ray0+_s,Sstart);r<ss->icell(ray0+_s,Send)+1;r++) {
	setup_data(r);
	}
}
void setup_data(int thisrow) {
	//this fn set values to the variables from the sheet 
	//for the row of interest- it has to be run before a calculation to refresh data
	//as a prior run may have changed the internal static values
	//read in variables thisrow only
	if (ss->fcell(thisrow,diamc)>0.) _Diam = ss->fcell(thisrow,diamc)/1000.;
	else ss->cell(thisrow,diamc,_Diam *1000.);	//if (!_Diam ) return 0;
	if (ss->fcell(thisrow,dragfc)>0.) _Dragf   = ss->fcell(thisrow,dragfc);
	else ss->cell(thisrow,dragfc,_Dragf   );	//if (!_Diam ) return 0;
	if (ss->fcell(thisrow,roughc)>0.) _Rough = ss->fcell(thisrow,roughc)/1e6;
	else ss->cell(thisrow,roughc,_Rough *1e6);
	if (ss->fcell(thisrow,lengc)>0.) _Leng  = ss->fcell(thisrow,lengc)*1000.;
	else ss->cell(thisrow,lengc,_Leng  /1000.);	//if (!_Leng ) return pu;
	if (ss->fcell(thisrow,tabsc)>0.) _Tabs = ss->fcell(thisrow,tabsc)+TCONV  ;
	else ss->cell(thisrow,tabsc,_Tabs -TCONV  );
	_Deltah  = ss->fcell(thisrow,del_calc);// ; //can have 0 elevation changes
}
void show_data() {
	cout << " pu pd leng diam deltah tabs z  flow offfl\n";
	cout.width(5);
	cout<<_Pu  -PCONV  <<' ';
	cout<<_Pd -PCONV  <<' ';
	cout<<_Leng  /1000.<<' ';
	cout<<_Diam *1000.<<' ';
	cout<<_Deltah  <<' ';
	cout<<_Tabs -TCONV  <<' ';
	cout<<_Z<<' ';
	cout<<_Flow  <<' ';
	//cout<<brflow;
	cout<<endl;
}

int count_sections() { 
	nosectns=0;
	int row=rowoffset;
	while (!ss->cells[row-1][identc-1].empty()) {row++; nosectns++;}
	if (!nosectns) {
		ss->temp= "No sections found- check data - \nmust start in row ";
		ss->temp+=ss->itoa(rowoffset);
		fl_message("%s",ss->temp.c_str());return 0;
	}
	//really finished if further rows are empty, otherwise maybe an inadvertent gap
	//check for an empty space followed by further data
	if (!ss->cells[row][identc-1].empty()) {
		ss->temp="Gap in ident column at row ";
		ss->temp+=ss->itoa(row);
		fl_message("%s",ss->temp.c_str());
		return 0;
	}
	return 1;
	cout<<"count_sections\n";
}

void clean_secID() {
	string ID;
    for (int r=rowoffset;r<nosectns+rowoffset;r++) {
       	ID = ss->cell(r,identc); 
   		//drop any  blanks to avoid string comparison errors
		while (string::npos != ID.find(' ')) ID.erase(ID.find(' '),1);
		//ss->cell(r,identc+1,ID1);
     }
} 

int count_segments() {
	string IDseg; //this is the combination of alphabetic characters only
	int nosegs=0;
    int r=rowoffset;
   	IDseg=get_segment_ID(r);
    while (r<nosectns+rowoffset+1) {
    	if (IDseg==get_segment_ID(r)) r++;
		else  { 
			nosegs++;
			IDseg=get_segment_ID(r);
		}
	}
	cout <<"count_segments\n";
	return nosegs;
} 
int find_segments () {
	string IDseg; //this is the combination of alphabetic characters only in column 'ident' unique for each segment
	int nosegs=0;
    int r=rowoffset,rstart=0;
    //string temp=""; // msg="";
   	IDseg=get_segment_ID(r);
   	//if (IDseg!=".") temp=IDseg;
    rstart=r;
    while (r<nosectns+rowoffset+1) {// nb not +1
    //cout<<"allseg, this seg & no "<<temp<<' '<<IDseg<<' '<<nosegs+1<<endl;
    	if (IDseg==get_segment_ID(r)) r++; //{
    		//if (IDseg!=".") {
    			//if (r!=nosectns+rowoffset) ss->cells[r-1][flagc-1].cval("0");
    			//ss->cells[r-1][identc-1].colourx( (Fl_Color)nosegs);
    		//}
    		//r++;
    	//}
		else  { 
			ss->cell(ray0+nosegs,Sno,nosegs+1);
		    ss->cell(ray0+nosegs,Sstart,rstart); 
			ss->cell(ray0+nosegs,Send,r-1); //nos on sheet correspond to row nos
			ss->cell(ray0+nosegs,Sid,IDseg);	
			rstart=r;
			nosegs++;
			IDseg=get_segment_ID(r);
			//check this segment ID not already used
		/*	if (IDseg!="") {
				msg = "";
				if (temp.rfind(IDseg)!=string::npos) {
					msg= "non unique identifier: ";
					msg+=IDseg;
					msg+=" at row ";
					msg+= ss->itoa(r);
					fl_message(msg.c_str());
					return 0;
				} 
			}  */
			//if (IDseg!=".") temp+=IDseg;
		}
	}
	for (int r=ray0-2;r<ray0+nosegs+1;r++) {
		for (int c=S0;c<SOK+1;c++) {// format runarray area
			ss->cells[r-1][c-1].precisionx(0);
			ss->cells[r-1][c-1].colourx(FL_RED);
			ss->cells[r-1][c-1].sizex(10);
			ss->cells[r-1][c-1].format(0);//dont format (eliminates decimal points) 
			if (c==Spracc || c==Sflacc) ss->apply_C_format(r-1,c-1,"%5.2e");//20080429 change of index origin
		}
	}
	cout<<"find segments\n";
	return nosegs;
}

string &get_segment_ID (int row) {
	int j;
	string s1,s2;
	s1= ss->cell(row,identc);
	s2="";
	//bypass any special character at start > < / \ ~ etc ONLY ONE ALLOWED
	if (s1[0]=='.') return ss->temp=".";//treat the dot as a dummy segment; removed later
    if (isalpha(s1[0])) j=0;  else j=1;//start with alpha only (but this allows for additional markers as leadin
    while (isalpha(s1[j])) {s2=s2+s1[j]; j++;} //s2 contains segment identifier 
    return ss->temp=s2; //what happens if a null - returned string will be empty
}

int analyse_runarray(int ray0) {
/*//analyse runarray
---------------------------------------------------------------------------------------------------------------------------
 all segments on the sheet should have the following coding in the ident column - all intrasegment rows must start 
 with a unique alphabetic segID followed by a double if required eg ab1..........ab2...ab3  anything else
 is ignored unless it is in the first or last row of the segment (see following)
 this helps to keep segment definition clean and allows specification of a loop on a loop for instance
 doubles help to identify a particular row if a branch or junction occurs there
 Brtype   description                     	typical depiction in ident column
0   	dummy (bypassed for hydraulics) .	starts with '.' for spacing between segments
1     	normal or simple segment        	A1....A4.....A8.......A12
-1		reverse normal segment 	-   		-A1........A12 (normal but with outlet flow and outlet p as data inputs)	
2		pu,pd pipesegment		/	..		/a1.........a6  (iterate positive flow based on end pressures pu in & pd end)
-2		pu,pd pipesegment 		..     /	a1........../a6  as above but for negative flow (end to start on sheet)
3		reverse flow segment 	.... -		A1 .......-A12	flow proceeds from outlet (end of segment) to inlet
4 		continuation (all flow from ups 	ab1#A4..C1..........C8  takes flow and pressure from outlet of A4 												(not to be confused with -1)
8      	branch line in	         		 	ab .................ab>A4 (note > at end)
9		branch line out				 		ab<A4	 ............ab6 (note not an > at end) flow from start of A4

==================== now follow all connections=========================
21		loop or ringmain leadin				automatic (provided by process_rarray unless blocked by a set segmentflag (eg !ab1<A4)
22		loop without creating lead in/out	ab1<A4 ...etc
23   	distributor or ringmain loop <  <	ab1<A4.........ab4<A8 (flow in at both ends of looper)
25    	free flow connecting line  ~	~ 	ab1~A4......ab5~D7 (flow between two segments from A4 to D7 or D7 to A4
												depending on pressure diff (free ie no forced flow direction)
26		forced flow connecting line  :	 :	ab1:A4 .....ab5:D5 (flow can only be in from top to bottom of segment)
27		ringmain connection  *    *			ab1*A4 .....ab5*D5 (flow in at both ends from different segments)


	compressor, regulator, meterstns	show in kp column as c, s, m, f (flow control)
	
	placing a ! in front of the first ident for a segment sets a flag to enable different behaviour to be adopted here (eg see loop)


 implement to here
x       absolute ringmain               \>A4.........\>D7 (like a ringmain without the loopee segment
y   	absolute forced flow connection \-A4..........\-D7  (forced flow from D7 to A4 with compressor)
z    	flow control segment            ~A4............~  (flow into A4 at the pressure at A4
p   	older absolute segment          section no (takes offlow as first trial)
---------------------------------------------------------------------------------------------------------------------------
NB branch flows always assumed to occur at START of segment not at END
 thus flow upstream = flow downstream + branchflow (also for loop starts & returns when branchflow is shown negative) 
*/
	ss->textcolor(BRIGHT,RED,BLACK);
	cout <<"\nStarting analyse_runarray\n";
	ss->textcolor();
	
	//clean_secID();
	show_runarray(nosegs);
	unsigned segst,segend,segmentflag=0;
	unsigned long i;
	string IDst="",
		   IDend="",
		   IDseg="",
		   IDfr="",
		   IDto="",
		   temp="";
  
	for (int s=0;s<nosegs;s++) 
	{
		segst = ss->icell(ray0+s,Sstart);//in 1 origin
		segend = ss->icell(ray0+s,Send);
		IDseg = ss->cell(ray0+s,Sid);
		
		//look for a ! to set segment flag in first row of the segment : sets segment flag to provide for different behaviour
		if(ss->cell(segst,identc).find("!")!=string::npos) segmentflag=1; else segmentflag=0;
		
		if (IDseg==".") {//dummy segment
			for (int c=Sid;c<SOK+1;c++) ss->cell(ray0+s,c,"");
			ss->cell(ray0+s,Stype,0);
			continue;
		} 
		
		IDst=ss->cell(segst,identc);
		IDend=ss->cell(segend,identc);

		//look for < >
		i=IDst.find("<");
		if (i!=string::npos) {
			IDfr.assign (IDst,i+1,IDst.size());
			int rfr = find_ID_row(IDfr);
 			
 			if (ss->icell(ray0+find_which_segment(rfr),Sstart)==rfr) {
				temp = "can't have a branch takeoff at start of segment,\nuse dummy leadin if necessary\n";
				cout<<temp;
				fl_message("%s",temp.c_str());
				return 0;
				}
			//-------------------	
			ss->cell(ray0+s,Sfrom,rfr);//put take off row in Sfrom
			//-------------------
			
			//check end of segment to determine type
			i=IDend.find(">");
			if (i!=string::npos) {
				IDto.assign (IDend,i+1,IDend.size());
				if (!segmentflag) ss->cell(ray0+s,Stype,22);
				else {
					ss->cell(ray0+s,Stype,-22); //prevents loopee splitup into leadin and continuation line (some compound loops)
					segmentflag=0;
					}
				ss->cell(ray0+s,S0,"loop");
				ss->cell(ray0+s,Sto,find_ID_row(IDto));
				continue;
		    } //loop 
		    
		    i=IDend.find("<");
			if (i!=string::npos) {
				IDto.assign (IDend,i+1,IDend.size());
				if (!segmentflag) ss->cell(ray0+s,Stype,23);
				else {
					ss->cell(ray0+s,Stype,-23); //prevents loopee splitup into leadin and continuation line (some compound loops)
					segmentflag=0;
					}
				ss->cell(ray0+s,S0,"ring main");
				ss->cell(ray0+s,Sto,find_ID_row(IDto));
				continue;
		    } //a ringmain		 
		       
		    i=IDend.find("_");
			if (i!=string::npos) {
				IDto.assign (IDend,i+1,IDend.size());
				ss->cell(ray0+s,Stype,27);
				ss->cell(ray0+s,Sto,find_ID_row(IDto));
				ss->cell(ray0+s,S0,"ring main connectn");
				continue;
		    } //ringmain connection

		    
			else {
				ss->cell(ray0+s,Stype,9);
				ss->cell(ray0+s,S0,"branch line out");
				continue;
			} //a branch line out
		}
		i=IDend.find(">");
		if (i!=string::npos) {
			IDto.assign (IDend,i+1,IDend.size());
			ss->cell(ray0+s,Stype,8);
			ss->cell(ray0+s,Sto,find_ID_row(IDto));
			ss->cell(ray0+s,S0,"branch line in");
			continue;				
		} //branch line in
		
		i=IDst.find("#");
		if (i!=string::npos) {
			IDfr.assign (IDst,i+1,IDst.size());
			ss->cell(ray0+s,Stype,4);
			ss->cell(ray0+s,Sfrom,find_ID_row(IDfr));
			ss->cell(ray0+s,S0,"continuation line");
			continue;				
		} //continuation line (takes all flow)	
		
		i=IDst.find("/");// a pressure delimited segment for which +ve flow has to be determined
		if (i!=string::npos) {
			ss->cell(ray0+s,Stype,2);
			ss->cell(ray0+s,Sfrom,segst);
			ss->cell(ray0+s,S0,"+ve fl Pu/Pd");
			continue;				
		}
		i=IDend.find("/");// a pressure delimited segment for which negative flow has to be determined
		if (i!=string::npos) {
			ss->cell(ray0+s,Stype,-2);
			ss->cell(ray0+s,Sfrom,segst);
			ss->cell(ray0+s,S0,"-ve fl Pu/Pd");
			continue;				
		}
		i=IDend.find("-");// a segment where flow is from the outlet to the inlet
		if (i!=string::npos) {
			ss->cell(ray0+s,Stype,3);
			ss->cell(ray0+s,Sfrom,segst);
			ss->cell(ray0+s,S0,"reverse flow");
			continue;				
		}  
		i=IDst.find("~");
		if (i!=string::npos) {
			IDfr.assign (IDst,i+1,IDst.size());
			ss->cell(ray0+s,Sfrom,nosectns+rowoffset);//a fudge to ensure free connection is placed last by sorting on sfrom
			ss->cell(ray0+s,Scon1,find_ID_row(IDfr));
			i=IDend.find("~");
			if (i!=string::npos) {
				IDto.assign (IDend,i+1,IDend.size());
				ss->cell(ray0+s,Stype,25);
				ss->cell(ray0+s,Scon2,find_ID_row(IDto));
				ss->cell(ray0+s,S0,"free connectn");
				continue;
		    } //a freely flowing connection between 2 segments from one point to another in a direction determined by 
		      //pressure differences - usually means rerunning mainpressrun to iterate to solution			
		}
		
		i=IDst.find(":");
		if (i!=string::npos) {
			IDfr.assign (IDst,i+1,IDst.size());
			ss->cell(ray0+s,Sfrom,find_ID_row(IDfr));
			i=IDend.find(":");
			if (i!=string::npos) {
				IDto.assign (IDend,i+1,IDend.size());
				ss->cell(ray0+s,Stype,26);
				ss->cell(ray0+s,Sto,find_ID_row(IDto));
				ss->cell(ray0+s,S0,"forced fl connectn");
				continue;
		    } //a forced flow connection between 2 segments from inlet to outlet (if possible, else no flow)			
		}	
		
		i=IDst.find("*"); //not implemented yet
			if (i!=string::npos) {
			IDfr.assign (IDst,i+1,IDst.size());
			int rfr = find_ID_row(IDfr);
			ss->cell(ray0+s,Sfrom,rfr);//put take off row in Sfrom
			i=IDend.find("*");
			if (i!=string::npos) {
				IDto.assign (IDend,i+1,IDend.size());
				ss->cell(ray0+s,Stype,78);
				ss->cell(ray0+s,Sto,find_ID_row(IDto));
				cout <<" * option - not yet implemented\n";
				return 0; //not implemented
		    } //spare	
		}	
		
		i=IDst.find("-");// take flow and pressure as at end of segment
		if (i!=string::npos) {
			ss->cell(ray0+s,Stype,-1);
			ss->cell(ray0+s,Sfrom,segst);
			ss->cell(ray0+s,Sto,segend);
			ss->cell(ray0+s,S0,"P, flow at end");
			continue;				
		}
		else {
			ss->cell(ray0+s,Stype,1);
			ss->cell(ray0+s,Sfrom,segst); //can have multiple inlet pipes
			ss->cell(ray0+s,S0,"simple pipe");
			continue; //a normal segment with pu and flow contained in its first line
		}
		
	} 
	//remove dummy segments in runarray
	nosegs = nosegs - remove_dummy_runarray_segments(nosegs);//eliminate dummy segments and move others up
	for (int r=0;r<nosegs;r++) ss->cell(ray0+r,Sno,r+1);
	
	show_runarray(nosegs);
	
	//check open ended segments are followed by a dummy
	for (int s=0;s<nosegs;s++) 
	{
		segend = ss->icell(ray0+s,Send);
		if (ss->icell(ray0+s,Stype)<20 && (ss->cell(segend+1,identc)[0]!='.') && s!=nosegs-1) {
		temp = "no dummy cell after an open ended segment - \ninsert one to provide space for elevation";
		temp+= " at end of segment no ";
		temp+= ss->itoa(s+1);
		temp+=" or ignore";
		cout << temp<< endl;
		fl_message("%s",temp.c_str());
		}
	}
	cout <<"end analyse_runarray\n";
	return 1;
	
}
	
void show_runarray(int nosegs) {
	//on stdout
	cout << "   ID  Row  Type  St   End  From Sto  Con1 Con2  Its  OK \n";
	for (int r=ray0-1;r<nosegs+ray0-1;r++) {
		for (int c=1;c<SOK+1;c++)  {
			cout.width(5);
			cout<<ss->cells[r][c].cval();
			}
		cout<<endl;
		}
}

void process_rarray (int ray0) {
//retain loop segments together and create new sequences for inlet segment to loop, loopee, and following segment
	ss->textcolor(BRIGHT,YELLOW,BLACK);
	cout <<"\nStarting process_rarray\n";
	ss->textcolor();
	show_runarray(nosegs);	
	//take in any loops or ringmains (type 22  or 23 and rearrange to ensure looper and loopee
	//are calculated at the same time
	int leadin_seg1=0;// leadin_seg1_orig_end=0, leadin_seg2_orig_end=0, cont_seg_start=0;
	for (int s=0;s<nosegs;s++) { //LOOP or RINGMAIN PROCESSING
		switch (ss->icell(ray0+s,Stype)) {
		case 22: 
		case 23:
		 {
			//setup_elevations(s); 
		 	break; //a compound loop : a loop within an outer loop on the same segment - dont break up the segment so bypass processing
		//see comment below. denoted by a leading \ (segmentflag=1) in the first ident column
		}
		case -22:  //a loop or ringmain
		case -23: 
		{
    	int sfrom= ss->icell(ray0+s,Sfrom);
    	int sto= ss->icell(ray0+s,Sto);
    	leadin_seg1= find_which_segment(sfrom);//where looper is attached :provides pu
 
    	//copy the leadin segment to the end of the array for setting up leadout segment
    	if (ss->icell(ray0+leadin_seg1,Stype)!=22 && ss->icell(ray0+leadin_seg1,Stype)!=23) {
			copy_cells(ray0+leadin_seg1,ray0+leadin_seg1,S0,SOK);
     		paste_cells(ray0+nosegs,S0);
			ss->cell(ray0+leadin_seg1,Send,sfrom-1);//reset leadin segment end row
			ss->cell(ray0+leadin_seg1,Stype,21); //and type
			ss->cell(ray0+leadin_seg1,Sto,sfrom); //and where it's connected
			//leadout segment
			ss->cell(ray0+nosegs,Stype,5); //reset its data
    		ss->cell(ray0+nosegs,Sstart,sto); 
    		ss->cell(ray0+nosegs,Sfrom,sto); //but use sto -1 in mainrun below
     		nosegs++;
     	}
     	else continue; //;{nosegs++; continue; }//a loop on a looper don't break up looper or, for a loop on a loopee the loopee
    	//this won't necessarily iterate to the final solution, but rerunning  mainpressrun should converge to the solution
    	//setup_elevations(s);
		show_runarray(nosegs);
   		break; 
		}
		
		case 78: 
		{ 
	
   		return; 
		}
	}
	}
	ss->cell("b11",nosegs);	
	if (nosegs>1) runarray_sort();
	cout <<"end process_rarray\n\n";
}

void rebuild_runarrayrow_index(int s, int s1) {
//must have set up necessary arrays arrayindex & tempstr
//insert row s in the runarray below row s1
//by building index accordingly -  doesn't assume array data exists
//note 0 origin
	for (int n=0;n<nosegs;n++) {
		if (n<s) 				arrayindex[n]=n;
		if (s <= n && n < s1) 	arrayindex[n]=n+1;
		if (n==s1) 				arrayindex[n]=s;
		if (n>s1) 				arrayindex[n]=n;
	}
}
void rearrange_by_index() {
//must have set up necessary arrays arrayindex & tempstr
//rearrange runarray using indices given in arrayindex
	for (int n=0;n<nosegs;n++) {//copy existing layout into tempstr
		for (int c=S0;c<SOK+1;c++) tempstr[n][c] = ss->cell(ray0+n,c);
	}
	for (int n=0;n<nosegs;n++) {//relocate existing layout in accordance with arrayindex
		for (int c=S0;c<SOK+1;c++) ss->cell(ray0+n,c,tempstr[arrayindex[n]][c]);
	}
	//reinstate row doubleing
	for (int n=0;n<nosegs;n++) ss->cell(ray0+n,Sno,n+1);	
}

int find_which_segment(int r) {
//find which segment a row is in
	int s;
	cout <<"find_which_segment\n";
	for (s=0;s<nosegs;s++) {
		if (ss->icell(ray0+s,Sstart)<=r && r<= ss->icell(ray0+s,Send)) return s;
	}
	return 0;
}

int find_ID_row (string &IDnn) {
//find the row index in the array that has IDnn as its ident
	//ss->temp=IDnn;ss->temp="looking for " + ss->temp;
	//fl_message(ss->temp.c_str());
	cout <<"find_ID_row\n";
	for (int r=0;r<nosectns;r++) {
		//fl_message(ss->cell(r+rowoffset,identc).c_str());
		if (ss->cell(r+rowoffset,identc)==IDnn) {return r+rowoffset;}
	}
	return 0;
}	
inline void move_a_runarray_row(int rowfrom, int rowto) {
//move a whole row of runarray
	for (int c=0;c<SOK+1;c++) ss->cells[ray0+rowto-1][c].cval(ss->cells[ray0+rowfrom-1][c].cval());
	//cout <<"move_a_runarray_row\n";
}
void swap_runarray_rows(int first, int second) {
//swaps two nominated runarray rows
	cut_cells(ray0+first,ray0+first,S0,SOK);
	paste_cells(ray0+ss->rows(),S0);//place first in temp storage at bottom of sheet
	cut_cells(ray0+second,ray0+second,S0,SOK);
	paste_cells(ray0+first,S0);
	cut_cells(ray0+ss->rows(),ray0+ss->rows(),S0,SOK);
	paste_cells(ray0+second,S0);
	//cout <<"swap_runarray_rows\n";
} 

inline void erase_a_runarray_row(int row) {
	for (int c=S0;c<SOK+1;c++) ss->cells[ray0+row-1][c-1].cval("");
	//cout <<"erase_a_runarray_row\n";
}

int remove_dummy_runarray_segments(int nos) {
//removes dummy segments and returns new nosegs
	int count=0;
	for (int s=0;s<nos;s++) if (ss->icell(ray0+s,Stype)==0) count++;
	for (int s=0;s<nos;s++) {
		if (ss->icell(ray0+s,Stype)==0) for (int s0=s;s0<nos-1;s0++) move_a_runarray_row(s0+1,s0);
		}
	if (count) for (int s=nos-1;s>nos-count-1;s--) erase_a_runarray_row(s);
	return count;
}

void sort_runarray(int key) {
//must have set up necessary arrays arrayindex & tempstr
//using insertion sort
//sort runarray indices first based on ascending order of the key
	int i,j,value,l,tempindex;
 
	for (j=0;j<nosegs;j++) arrayindex[j] = j; //set up index array
	l = 0; 
	for (j = l+1;j<nosegs;j++) {
		tempindex = arrayindex[j];
		value = ss->icell(ray0+tempindex,key); 
		for (i=j-1;i>l-1;i--) { 
			if (ss->icell(ray0+arrayindex[i],key)<value) goto l2;   
			arrayindex[i+1] =arrayindex[i];
		}
		i = l-1;
		l2: arrayindex[i+1] = tempindex;
	}
//sort runarray using indices
	for (int s=0;s<nosegs;s++) {
	    for (int c=S0;c<SOK+1;c++) tempstr[s][c] = ss->cell(ray0+s,c);
  	}
	for (int s=0;s<nosegs;s++) {
  		for (int c=S0;c<SOK+1;c++) ss->cell(ray0+s,c,tempstr[arrayindex[s]][c]);
  	}
  	//restore sequence nos
  	for (int r=0;r<nosegs;r++) ss->cell(ray0+r,Sno,r+1);//redouble runarray segments for reference purposes
  	
	cout <<"sort runarray"; 
    return;
}

void cut_cells(int top,int bottom,int left,int right) {
//adapted from menu cut and paste
	ss->clipboard.sdrrowscols(bottom-top+1,right-left+1);
	int rr =0;
	//cut
	for (int r= top-1; r< bottom; r++)	{
		int cc = 0;
		for (int c= left-1; c< right; c++)	{
			ss->clipboard[rr][cc] = ss->cells[r][c];
			ss->cells[r][c].clearcell_value(); 
			cc++;
		}
		rr++;
	}
		//cout <<"cut_cells\n";
}
void copy_cells(int top,int bottom,int left,int right) {
//adapted from menu cut and paste
	ss->clipboard.sdrrowscols(bottom-top+1,right-left+1);
	int rr =0;
	//cut
	for (int r= top-1; r< bottom; r++)	{
		int cc = 0;
		for (int c= left-1; c< right; c++)	{
			ss->clipboard[rr][cc] = ss->cells[r][c];
			cc++;
		}
		rr++;
	}
		//cout <<"copy_cells\n";
}
void paste_cells(int top,int left)	 {
//paste clipboard from row, col designated
	int rr = 0;
	for (int r= top-1; r < top + ss->clipboard.sdrrows()-1; r++) {
		int cc = 0;
		for (int c= left-1; c < left + ss->clipboard.sdrcols()-1; c++)	{
			ss->cells[r][c] = ss->clipboard[rr][cc];  //pastes all
			cc++;
		}
		rr++;
	}
		//cout <<"paste_cells\n";
}

double bisection(double &xl,double &xr, double &fxl, double &fxm, double &tol) {
	//suggest the next trial to use for xm
	//ok how do we get fxl and fxm into this function???
	double xm;
	do {
		xm = (xl+xr)/2.;
		if (fxl*fxm>0.) xl = xm;
		else xr = xm;
	} while (fabs(xr-xl)>tol);
	return (xl+xr)/2.;
}
void showdata() {
	cout<<"pu "<< _Pu  -PCONV  <<" flow ";
	cout<<_Flow  <<" diam "<<_Diam *1000.<<" dragf "<<_Dragf   <<" rough "<<_Rough *1e6<<" length "<<_Leng  /1000.;
	cout <<" elev "<<_Deltah  <<" temp "<<_Tabs -TCONV  <<endl; 
}

void checkdata(double &pu , double &flow ) {
//checks pu and flow for mainpressrun NEEDS MORE !
	run_OK=1;
	if (pu <PCONV  ) run_OK=0;
	if (flow <=0.) run_OK=0;
	if (!run_OK) cout<<"error or rerun required\n";
	return;
}
int checkrun() {
//checks linked inlet and outlet pressures match to see if run was successful

	double pu,pd,pdiff,pdiffmax,flowup,flowdiff,flowdiffmax,stype;
	int r, sstart, sto, sfrom;
	int ok,ok_overall;
	ss->textcolor(BRIGHT,YELLOW,BLACK);
	cout<<"Checking run for pressure and flow consistency\n";
	ss->textcolor();
	ok_overall = 1;
	for (int s=0;s<nosegs;s++) {
		ok=1;
		pdiffmax = 0.;
		ss->cell(ray0+s,Spracc,"");
		r = sstart = ss->icell(ray0+s,Sstart);
		stype = fabs(ss->icell(ray0+s,Stype));
		sfrom = ss->icell(ray0+s,Sfrom);
		if (stype < 21) {
			if (stype != 4) pd = ss->fcell(ss->icell(ray0+s,Sfrom),pupc);
			else pd = ss->fcell(ss->icell(ray0+s,Sfrom),pdownc);//continuation line is an exception
			pu = ss->fcell(r,pupc);
			pdiff= fabs(pd-pu);
			if (pdiff>pdiffmax) pdiffmax = pdiff;
			if (pdiff>it_tol*10. || pd*pu==0.) {//relax tolerance a bit here to avoid to many error msgs
				ss->textcolor(BRIGHT,RED,BLACK);
				cout <<"ERROR in seg "<<s+1 <<" pu "<<pu<<" at row "<<r<<" pd "<<pd <<" at row "<<r-1<<endl;
				ok=0;
				ss->textcolor();
			}
		}
		else {
			
			sto = (ss->icell(ray0+s,Sto)) ? ss->icell(ray0+s,Sto): ss->icell(ray0+s,Scon2);
			pd = ss->fcell(ss->icell(ray0+s,Send),pdownc);
			pu = ss->fcell(sto,pupc);
			pdiff= fabs(pd-pu);
			if (pdiff>pdiffmax) pdiffmax = pdiff;
			if (pdiff>it_tol*10. || pd*pu==0.) {
				ss->textcolor(BRIGHT,RED,BLACK);
				cout <<"ERROR in seg "<<s+1 <<" pu "<<pu<<" at row ";
				cout<<ss->icell(ray0+s,Sto)<<" pd "<<pd <<" at row "<<ss->icell(ray0+s,Send)<<endl;
				ok=0;
				ss->textcolor();
			}
		}
	
		for (++r;r<ss->icell(ray0+s,Send)+1;r++) {	//this should normally be superfluous as these values are set
			pu=ss->fcell(r,pupc);
			pd=ss->fcell(r-1,pdownc);
			pdiff= fabs(pd-pu);
			if (pdiff>pdiffmax) pdiffmax = pdiff;
			if (pdiff>it_tol*10. || pd*pu==0.) {
				ss->textcolor(BRIGHT,RED,BLACK);
				cout <<"ERROR in seg "<<s+1 <<" pu "<<pu<<" at row "<<r<<" pd "<<pd <<" at row "<<r-1<<endl;
				ok=0;
				ss->textcolor();
			}
		}
		if (pdiffmax>0) ss->cell(ray0+s,Spracc,pdiffmax);
			
		// flow balance
		r = sstart;
		flowdiffmax = 0.;
		ss->cell(ray0+s,Sflacc,"");
		if (stype != 4) flowup = ss->fcell(r,flowc);
		else flowup = ss->fcell(sfrom,flowc); 
		for (++r;r<ss->icell(ray0+s,Send)+1;r++) {
			flowdiff = fabs(flowup - ss->fcell(r,flowc) -ss->fcell(r,brflowc));
			if (flowdiff>flowdiffmax) flowdiffmax = flowdiff;
			if (flowdiff>0.) {
				ss->textcolor(BRIGHT,YELLOW,BLACK);//note
				cout <<"WARNING : segment "<<s+1<<" row "<<r<<" flow balance difference "<<flowdiff<<endl;
				ss->textcolor();
			}
			if (flowdiff>0.001) {//accept
				ss->textcolor(BRIGHT,RED,BLACK);
				cout <<"FLOW ERROR in seg "<<ss->cell(ray0+s,Sid) <<" flowdiff "<<flowdiff<<" at row "<<r<<endl;
				ok=0;
				ss->textcolor();
			}
			flowup = ss->fcell(r,flowc);
		}
		if (flowdiffmax>0.) ss->cell(ray0+s,Sflacc,flowdiffmax);
		if (!ok) { ss->cell(ray0+s,SOK,"No"); ok_overall = 0;}
		else ss->cell(ray0+s,SOK,"Yes");
	}
	if (ok_overall) cout<<"Check run returns OK\n";
	else cout<<"Check run shows errors NOT OK\n";
	return ok_overall;
}

void mainpressrun() {
//this examines output of runarray and calculates pressures downstream
	ss->textcolor(BRIGHT,CYAN,BLACK);
	cout << "Main pressure run\n";
	ss->textcolor();
	//double 	tempd;
	int leadout_seg;
	int ok, number_of_runs=0;
	string msg="";
	do {
		for (current_seg=0;current_seg<nosegs;current_seg++) 
		{
			cout<< "segment type " << ss->icell(ray0+current_seg,Stype);
			cout<<" start "<<ss->icell(ray0+current_seg,Sstart)<<" end "<<ss->icell(ray0+current_seg,Send)<<endl;
			stype = ss->icell(ray0+current_seg,Stype);
			switch (stype) 
			{
				case 0: break; 		//dummy
				case 1: 
				{			//simple 
					segstart = ss->icell(ray0+current_seg,Sstart);
					segend = ss->icell(ray0+current_seg,Send);
					int r = segstart;	
					next_pressure_down(r,ss->fcell(r,pupc)+PCONV  ,ss->fcell(r,flowc));
					for (++r;r<segend+1;r++) {//prefix ++ to ensure updated r is used
						//flow remains the same but brflow comes off it at start of the current segment
						//_Pd becomes _Pup in next row
						next_pressure_down(r, _Pd  , _Flow -ss->fcell(r,brflowc));
						if (_Pd == PCONV) number_of_runs = RUNSMAX; //reached minimum pressure 	
					}							
					break;		
				}
				case -1: 
				{//simple segment but calculated backwards with pd and flow data at end of segment
						//rest of data as for case 1
					//setup_segment_data(s);//as we start at the end of the segment all data has to be set before as it is top down 
					segend = ss->icell(ray0+current_seg,Send);
					segstart = ss->icell(ray0+current_seg,Sstart);
					int r = segend;
					next_pressure_up(r, ss->fcell(r, pdownc)+PCONV, ss->fcell(r,flowc));
					for (--r;r>segstart-1;r--) {
						//brflow comes off at start of the previous segment
						next_pressure_up(r, _Pu , _Flow + ss->fcell(r+1, brflowc));
					}
					break;
				}
				case -2:
				case  2: 
				{ //pressure defined segment in +ve (+5) or -ve (-5) direction - iterate 
					// intrinsically stable given a reasonable press difference 
					// not very sensitive to elevation internally if the boundary point elevations are equal.
					// work backwards from lower to higher pressure as pu can increase practically without limit
					// and pressure/flow profile is more linear at higher pressures
					segstart= ss->icell(ray0+current_seg,Sstart);
					segend= ss->icell(ray0+current_seg,Send);
					if (stype==2) {
						p_in = ss->fcell(segstart,kpc)+PCONV  ; //pressure limits put in kp column
						p_out= ss->fcell(segend,kpc)+PCONV  ; //or the suction side of a compressor
					}
					else {
						p_out = ss->fcell(segstart,kpc)+PCONV  ; //pressure limits put in kp column
						p_in = ss->fcell(segend,kpc)+PCONV  ; //or the suction side of a compressor				
					}				
					setup_segment_data(current_seg);
					//
					if (p_in < p_out) {
						msg = "press in < press out -need to use a compressor in segment ";
						msg += ss->itoa(current_seg);
						cout <<msg <<endl;
						fl_message("%s",msg.c_str());
						//need an insert here to find the suction side of the compressor and set it p_out to
						//a pressure appropriate. Any flow is then possible subject to power and compression ratio 
						//for now
						break;
					}
					if (stype== -2) goto l_negativeflow;
					
					l_positiveflow:
					//flow_min = sum_of_segment_off_flows(segstart,segend); //min flow must equal sum of all branch offtakes
					_Leng  = diameter_adjusted_total_length(segstart,segend);
					// using this length determine flow across segment as if it consisted of only one section
					// this will be quite accurate unless diam changes provided _Deltah  across whole section is employed
					_Pu  = p_in;_Pd = p_out;
					_Deltah  = ss->fcell(segend,el_calc) - ss->fcell(segstart,el_calc);//change for a compressor
					local_flow= _Flowfn(); 
					cout<<"total leng " << total_length<<" fl first guess "<<local_flow<<" min flow "<<flow_min<<endl;
					xl=local_flow*1.5;
					xh=local_flow/1.5;
					xl_minimum=PCONV  +0.01, xh_maximum=1e10;//also an upper limit of ~18000kPa on z calculation
					//bracket the interval
					if(zbrac(&pressfn_positive_flow,&xl,&xh,&xl_minimum,&xh_maximum) != 1) {
						msg = "+ve flow pressure defined segment iteration failed - check data"; 
						fl_message("%s",msg.c_str() );
						cout<< msg << endl;
						number_of_runs = RUNSMAX;
					}//zbrac will have printed a message
					iterationcount=0;
					cout<< "running zbrent "<<endl;
					local_flow = zbrent(&pressfn_positive_flow,xl,xh,it_tol);
					cout<<"loop iterations completed - no iterations "<<iterationcount<<endl;
					ss->cell(ray0+current_seg,Sits,iterationcount);
					if (stype==25) { //a free connection - show flows
						ss->cell(rowfrom,brflowc,ss->fcell(segstart,flowc));//offtake flow
						ss->cell(rowto,brflowc,-ss->fcell(segend,flowc));//return flow
					}
					break; 
					
					l_negativeflow:
					
					//flow_min = sum_of_segment_off_flows(segstart,segend); //min flow must equal sum of all branch offtakes
					_Leng  = diameter_adjusted_total_length(segstart,segend);
					// using this length determine flow across segment as if it consisted of only one section
					_Pu = p_in; _Pd = p_out;
					_Deltah  = ss->fcell(segstart,el_calc) - ss->fcell(segend,el_calc);//change if compressor present
					local_flow = _Flowfn(); //only variable that needs to be reset is deltah
					cout<<"total leng " << total_length<<" fl first guess "<<local_flow<<" min flow "<<flow_min<<endl;
					xl=local_flow*1.5;
					xh=local_flow/1.5;
					xl_minimum=PCONV  +0.01, xh_maximum=1e10;//also an upper limit of ~18000kPa on z calculation
					//bracket the interval
					if(zbrac(&pressfn_negative_flow,&xl,&xh,&xl_minimum,&xh_maximum) != 1) {
						msg = "-ve flow pressure defined segment iteration failed - check data";
						fl_message("%s",msg.c_str() );
						cout<< msg <<endl;
						number_of_runs = RUNSMAX;
					}//zbrac will have printed a message
					iterationcount=0;
					cout<< "running zbrent "<<endl;
					local_flow = zbrent(&pressfn_negative_flow,xl,xh,it_tol);
					cout<<"loop iterations completed - no iterations "<<iterationcount<<endl;
					ss->cell(ray0+current_seg,Sits,iterationcount); 
					if (stype==25) { //a free connection - show flows
						ss->cell(rowfrom,brflowc,-ss->fcell(segstart,flowc));//offtake flow
						ss->cell(rowto,brflowc,ss->fcell(segend,flowc));//return flow
					}
					break; 				
				}
				case 3: //??PROBLEMS
				{ //reverse flow segment
					segstart = ss->icell(ray0+current_seg,Sstart);
					segend = ss->icell(ray0+current_seg,Send);			
				
					int r = segend;
					
					next_pressure_down_reverse_flow(r, ss->fcell(r, pdownc) + PCONV, fabs(ss->fcell(r,flowc)));
					for (--r;r>segstart-1;r--) {
						//brflow comes off at start of the previous segment
						next_pressure_down_reverse_flow(r, _Pd , _Flow - ss->fcell(r+1,brflowc));
					}
					break;
				}
				case 4: 
				{//continuation line- takes all flow and pressure from outlet of designated pipe section
					segstart = ss->icell(ray0+current_seg,Sstart);
					segend = ss->icell(ray0+current_seg,Send);
					rowfrom = ss->icell(ray0+current_seg,Sfrom);
					int r = segstart;
					next_pressure_down(r,ss->fcell(rowfrom,pdownc)+PCONV  ,ss->fcell(rowfrom,flowc));//only diff with 12
					for (++r;r<segend+1;r++) {
						next_pressure_down(r, _Pd  , _Flow - ss->fcell(r,brflowc));
						if (_Pd == PCONV)  	number_of_runs = RUNSMAX; //reached minimum pressure
					}
					break;								
				}
				case 9: 
				{ //branch flow out
					segstart = ss->icell(ray0+current_seg,Sstart);
					segend = ss->icell(ray0+current_seg,Send);
					rowfrom = ss->icell(ray0+current_seg,Sfrom);
					int r = segstart;
					next_pressure_down(r,ss->fcell(rowfrom,pupc)+PCONV  ,ss->fcell(rowfrom,brflowc)); //only diff with 11
					for (++r;r<segend+1;r++) {
						next_pressure_down(r, _Pd  , _Flow -ss->fcell(r,brflowc));
						if (_Pd ==PCONV ) number_of_runs = RUNSMAX; //reached minimum pressure
					}
					break;		
				}
				case -22:
				case 22: 
				{ //loop  
					//flows in each line sum to total but distribution is determined by trial and error
					
					rowfrom= ss->icell(ray0+current_seg,Sfrom); //loopee start	NEW
					rowto= ss->icell(ray0+current_seg,Sto); //looper end
					segstart= ss->icell(ray0+current_seg,Sstart);	//looper start	
					segend= ss->icell(ray0+current_seg,Send);
					
					loopee_pu = looper_pu= ss->fcell(rowfrom-1,pdownc)+PCONV  ; //grabs outlet pressure of the segment leadin
					ss->cell(rowfrom,pupc,loopee_pu-PCONV  ); 
					upstr_flow=ss->fcell(rowfrom-1,flowc);
					//==================== start iteration by finding suitable upper and lower limits
					//adjust flows in each branch to get equal end pressure in looper and loopee
					double sum_looper=0., sum_loopee=0.;
	
					for (int r=rowfrom+1;r<rowto;r++) sum_loopee+=ss->fcell(r,brflowc);//dont count brflow in/out associated with loop
					for (int r=segstart;r<segend+1;r++)sum_looper+=ss->fcell(r,brflowc);////////////
					if (upstr_flow<sum_loopee+sum_looper) {
						fl_message("insufficient flow upstream of loop -check data");
						number_of_runs = RUNSMAX;
					} 
					double xl,xh; 
					double diam0,eq_leng_looper=0, eq_leng_loopee=0;
					//determine approx equivalent lengths for looper and loopee and 
					//apportion starting flows accordingly flow ~ L^0.5  and L ~ D^5
					//looper
					//setup_segment_data(s);//sets all data rows for this segment (looper)
					diam0=ss->fcell(rowfrom,diamc);//use looper diam as base for both
					for (int r=segstart;r<segend+1;r++) {
						//cout<<"looper leng "<<ss->fcell(r,lengc)<<" looper diam "<< ss->fcell(r,diamc)<<endl;
						eq_leng_looper += ss->fcell(r,lengc)*pow(diam0/ss->fcell(r,diamc),5);
					}
					//loopee
					for (int r=rowfrom;r<rowto;r++) {
						setup_data(r);
						//cout<<"loopee leng "<<ss->fcell(r,lengc)<<" loopee diam "<< ss->fcell(r,diamc)<<endl;
						eq_leng_loopee += ss->fcell(r,lengc)*pow(diam0/ss->fcell(r,diamc),5);
					}
					double looper_flow0;
					looper_flow0= upstr_flow/(1+sqrt(eq_leng_looper/eq_leng_loopee));
					cout<<"equiv leng looper loopee "<<eq_leng_looper<<' '<<eq_leng_loopee<<endl;
					cout<<"first estimate looper flow "<<looper_flow0<<endl;
					xl=looper_flow0*0.1;
					xh=looper_flow0*1.1;//seems to work for a wide range of differences in capacities
					xh= (xh<sum_looper)?sum_looper:xh;
					//bracket the interval; 1 is returned if successful	
					double xl_minimum=0.1, xh_maximum=upstr_flow;
					if(zbrac(&loopfn,&xl,&xh,&xl_minimum,&xh_maximum)!=1) {//manual entry option if zbrac fails
						const char* lf = fl_input("loop iteration failed\nenter a starting value for looper flow ", 
						ss->ftoa(ss->rounddouble(looper_flow0,0)));
					if (lf)	{ 
						looper_flow0= atof(lf);
						xl=looper_flow0*0.1;
						xh=looper_flow0*1.1;	
						}				
					else {
						fl_message("loop iteration failed - check data");
						number_of_runs = RUNSMAX;
						}
					}
					//cout<<"initial limits iteration count "<<iterationcount<<endl;
					double result = zbrent(&loopfn,xl,xh,it_tol);
					//this iteration will break down for very large disparities between looper and loopee capacity
					//for example when diameters are very different as the accuracy of the pressure determination
					//causes loss of significance eg pout for loopee (say) becomes insensitive to small changes
					//in a very small looper flow associated with a much smaller capacity looper_flow
					result= (result>upstr_flow) ? upstr_flow:result;//limit output
					//iteration finished, show flows as offtake flows
					ss->cell(rowfrom,brflowc,ss->fcell(segstart,flowc));//offtake flow
					ss->cell(rowto,brflowc,-ss->fcell(segend,flowc));//return flow
					//================== end iteration
					
					cout<<"loop - iterations "<<iterationcount<<endl; //<< loopfn(result)<< endl;
					ss->cell(ray0+current_seg,Sits,iterationcount);
					//ss->cell(ray0+current_seg,SOK,fabs(looper_pd-loopee_pd));
					
					//finish off by running leadout section
					rowfrom = rowto;  
					segstart = rowto; 
					leadout_seg = find_which_segment(rowfrom);
					segend = ss->icell(ray0+leadout_seg,Send);
					setup_data(segstart);
					_Pu  = ss->fcell(rowfrom-1,pdownc)+PCONV  ;
					_Flow  = ss->fcell(rowfrom-1,flowc)-(ss->fcell(segstart,brflowc));
					_Pd = _Pdownfn();
					ss->cell(rowfrom,pupc,_Pu  -PCONV  );
					ss->cell(rowfrom,pdownc,_Pd -PCONV  );
					ss->cell(rowfrom,flowc,_Flow  );
					ss->cell(rowfrom,z_calc,_Z);
					if (_Pd == PCONV  ) break;
					for (int r=rowto+1;r<ss->icell(ray0+leadout_seg,Send)+1;r++) {
						next_pressure_down(r, _Pd  , _Flow - ss->fcell(r,brflowc));
						if (_Pd ==PCONV  ) break; //reached minimum pressure
					} 
					break;
				}
				case 23:
				case -23:	
				{//ringmain - segment with inflows at both ends equal to outflows along segment
				//pressure from either direction must match at one of the outflows
					//if (stype==27) flag=1; else flag=0;//test : if 27, ringmain connection ??consider move?
					flag = 0;
					rowfrom = ss->icell(ray0+current_seg,Sfrom); //main start
					rowto =	ss->icell(ray0+current_seg,Sto);
					segstart = ss->icell(ray0+current_seg,Sstart);	//ringmain start
					segend = ss->icell(ray0+current_seg,Send);
					
					main_pu = ss->fcell(rowfrom-1,pdownc)+PCONV  ;
					ss->cell(rowfrom,pupc,main_pu-PCONV  );
					cout<<"main pu "<<main_pu -PCONV  <<endl;

					sum_ringmain=0., sum_main=0.;
					upstr_flow=ss->fcell(rowfrom-1,flowc);
					for (int r=segstart;r<segend+1;r++) sum_ringmain+=ss->fcell(r,brflowc);
					for (int r=rowfrom+1;r<rowto+1;r++) sum_main+=ss->fcell(r,brflowc);
					cout<<"total oftake flows in ringmain and main are "<<sum_ringmain<<' '<<sum_main<<endl;
					if (sum_ringmain==0.) {fl_message("no flows out from ringmain, ignoring it!");number_of_runs = RUNSMAX;}
					if (upstr_flow<sum_ringmain+sum_main) {
						fl_message("insufficient flow upstream of ringmain -check data");number_of_runs = RUNSMAX;}
					//upstream flow into ringmain is somewhere in between 0 and sum_ringmain flows
					//this fixes downstream flow into ringmain and the two are correct when the calculated pressure 
					//for the flow coming from the upstream end equals the calculated pressure from the flow coming 
					//from the downstream end at (one of )the offtake(s) on the ringmain (the 'neutral' point)
					double xl,xh;
					xh=sum_ringmain/2.;
					xl=xh/2.;
					double xl_minimum=0.1, xh_maximum=upstr_flow;
					if(zbrac(&ringmainfn,&xl,&xh,&xl_minimum,&xh_maximum)!=1) {
						if (flag) {
						msg = "ringmain connection iteration failed - check data";
						fl_message("%s",msg.c_str() );
						cout << msg << endl;
						number_of_runs = RUNSMAX;
						}
						else {
						msg = "ringmain iteration failed - check data:\ntry reducing ringmain diameter or change it to a looper";
						fl_message("%s",msg.c_str() );
						cout << msg << endl;
						number_of_runs = RUNSMAX;
						}
					}
					double result = zbrent(&ringmainfn,xl,xh,it_tol);
					result= (result>upstr_flow) ? upstr_flow:result;
					//iteration finished show flows out from loopee
					ss->cell(rowfrom,brflowc,ss->fcell(segstart,flowc));//offtake flow
					ss->cell(rowto,brflowc,-ss->fcell(segend,flowc));//return flow
					cout<<"ringmain - iterations "<<iterationcount<<endl;
					ss->cell(ray0+current_seg,Sits,iterationcount);
	 
					//finish off by running leadout section
					rowfrom = rowto;  
					segstart = rowto; 
					leadout_seg = find_which_segment(rowfrom);
					segend = ss->icell(ray0+leadout_seg,Send);
					setup_data(segstart);
					_Pu  = ss->fcell(rowfrom-1,pdownc)+PCONV  ;
					_Flow  = ss->fcell(rowfrom-1,flowc)-(ss->fcell(segstart,brflowc));
					_Pd = _Pdownfn();
					ss->cell(rowfrom,pupc,_Pu  -PCONV  );
					ss->cell(rowfrom,pdownc,_Pd -PCONV  );
					ss->cell(rowfrom,flowc,_Flow  );
					ss->cell(rowfrom,z_calc,_Z);
					if (_Pd == PCONV  ) break;
					for (int r=rowto+1;r<ss->icell(ray0+leadout_seg,Send)+1;r++) {
						next_pressure_down(r, _Pd , _Flow - ss->fcell(r,brflowc));
						if (_Pd==PCONV  ) break; //reached minimum pressure
					} 
					break;
				}
				case 25: 
				{//free connecting line- flow between two segments; 
					//+ve flow goes from inlet to outlet (may need a compressor)
					//-ve flow in reverse direction (may also need a compressor
					rowfrom = ss->icell(ray0+current_seg,Scon1);
					rowto = ss->icell(ray0+current_seg,Scon2);
					segstart = ss->icell(ray0+current_seg,Sstart);
					segend = ss->icell(ray0+current_seg,Send);
					int temp;
					
					p_in = ss->fcell(rowfrom,pupc)+PCONV  ;
					p_out = ss->fcell(rowto,pupc)+PCONV  ;
				
					//ignoring elevation effects at the moment
					if (p_in > p_out) {
						cout<<"rowfrom p1 rowto p2 : "<<rowfrom<<' '<<p_in-PCONV  <<' '<<rowto<<' '<<p_out-PCONV  <<endl;
						goto l_positiveflow;
					}
					else if (p_in==p_out) {
						cout<<"inlet and outlet pressures equal - may need a compressor in segment "<<current_seg<<endl; break;
					}
					else {
						cout<<"rowfrom p1 rowto p2 : "<<rowfrom<<' '<<p_in-PCONV  <<' '<<rowto<<' '<<p_out-PCONV  <<endl;
						//swap rowfrom and rowto
						temp = rowfrom;
						rowfrom = rowto;
						rowto = temp;
						p_in = ss->fcell(rowfrom,pupc)+PCONV  ;
						p_out = ss->fcell(rowto,pupc)+PCONV  ;
						goto l_negativeflow;
						break;
					}
				}
				break;
			}
		}
		number_of_runs++;
		ok = checkrun();
	} 	while (number_of_runs < RUNSMAX && !ok);
	cout<<"number of runs "<<number_of_runs<<endl;
	ss->cell(ray0+nosegs,Scon2,"Runs reqd:");
	ss->cell(ray0+nosegs,Sits,number_of_runs);
	if (number_of_runs == RUNSMAX) ss->cell(ray0+nosegs,Spracc,"= MAX");
	if (!ok) { cout<<"errors found\n"; fl_message("errors found!"); }
	return;
}

double sum_of_segment_off_flows(int segstart, int segend) {
	double sum=0;
	for (int r=segstart;r<segend+1;r++) sum+=ss->fcell(r,brflowc);
	return sum;
}
double diameter_adjusted_total_length(int segstart, int segend) {
//used by i.a. pressure delimited segments
	diam0=ss->fcell(segstart,diamc);
	total_length = 0.;
	for (int r=segstart;r<segend+1;r++) {
		diamn=ss->fcell(r,diamc);
		//determine total equivalent length in case diam changes across segment
		total_length+=+ ss->fcell(r,lengc)*pow(diam0/diamn,5);//(d^5/l) is very approx constant
	}
	return total_length*1000.;
}
void next_pressure_down(int thisrow, double _pu, double _flow) {
//determines outlet pressure in a pipe segment for a given section
	setup_data(thisrow);
	_Pu  = _pu;
	_Flow= _flow;
	_Pd = _Pdownfn();
	ss->cell(thisrow, pdownc, _Pd - PCONV);
	ss->cell(thisrow, pupc, _Pu - PCONV); //necessary?
	ss->cell(thisrow, flowc, _Flow);
	//if (segend>(thisrow+1 )) ss->cell(thisrow+1,pupc,_Pd-PCONV  );
	ss->cell(thisrow, z_calc, _Z);
	//comment out if not wanted : 
	ss->cell(thisrow, densc, density((_Pu +_Pd )/2., _Tabs , _Z));
	ss->cell(thisrow, headc, ss->fcell(thisrow, densc) * 9.8 * _Deltah  /1000.);
	return;
}
void next_pressure_down_reverse_flow(int thisrow, double _pu, double _flow) {
//determines pressure profile for reverse flow in a segment
//this means that as shown, pup is less than pdown and the flow is -ve
//calculation proceeds backwards but with decreasing pressure going up
	setup_data(thisrow);
	_Pu  = _pu;
	_Flow = _flow;
	_Deltah = - _Deltah;//proceeding as if backwards so need to reverse sign
	
	_Pu  = _Pdownfn();

	ss->cell(thisrow, pdownc, _pu - PCONV);
	ss->cell(thisrow, pupc, _Pu - PCONV); 
	ss->cell(thisrow, flowc, - _Flow);//shown as a minus to show reverse flow
 
	ss->cell(thisrow, z_calc, _Z);
	
	ss->cell(thisrow, densc, density((_Pd +_pu )/2., _Tabs , _Z));
	ss->cell(thisrow, headc, ss->fcell(thisrow, densc) * 9.8 * _Deltah  /1000.);		
}
void next_pressure_up(int thisrow, double _pd, double _flow) {
//determines inlet pressure in a pipe segment for a given section 
//based on data at the downstream end (as shown on sheet)
//used also by pressfn_positive_flow
	setup_data(thisrow);
	_Pd = _pd;
	_Flow  = _flow;
	_Pu  = _Pupfn();	//sign of deltah is taken into account by _Pupfn
	ss->cell(thisrow, pupc, _Pu  - PCONV);
	ss->cell(thisrow, pdownc, _Pd - PCONV); 
	ss->cell(thisrow, flowc, _Flow);
	ss->cell(thisrow,z_calc,_Z);
	//comment out if not wanted : 
	ss->cell(thisrow,densc,density((_Pu + _pd)/2., _Tabs ,_Z));
	ss->cell(thisrow,headc,ss->fcell(thisrow, densc) * 9.8 * _Deltah  /1000.);
	return;	
}
double pressfn_positive_flow(double _flow) {
	//func for zbrent - returns difference between pd calc and pd actual for a 
	//segment for which only p_in and p_out are given
	int r = segend;
	setup_data(r);
	_Pd = p_out;

	next_pressure_up(r, _Pd, _flow ); //start from end
	for (--r;r>segstart-1;r--) {
		//brflow comes off at start of the previous segment
		next_pressure_up(r, _Pu , _Flow + ss->fcell(r+1,brflowc));
	}
	r = segstart;
	ss->cell(segstart, flowc, _Flow);
	ss->cell(segstart, pdownc, ss->fcell(segstart+1, pupc));
	return _Pu - p_in;
}
double pressfn_negative_flow(double _flow) {
	//func for zbrent - returns difference between pd calc and pd actual for a 
	//segment for which only p_in and p_out are given
	//
	int r = segstart;
	setup_data(r);
	_Pd = p_out;
	_Flow  = _flow;
	_Deltah = - _Deltah;//note sign change
	_Pu  = _Pupfn();
	ss->cell(r, pupc, _Pd - PCONV);
	ss->cell(r, pdownc, _Pu  - PCONV);
	ss->cell(r, flowc, - _Flow);
	ss->cell(r, z_calc, _Z);
	//comment out if not wanted : 
	ss->cell(r, densc, density((_Pu + _Pd )/2., _Tabs, _Z));
	ss->cell(r ,headc, ss->fcell(r, densc) * 9.8 * _Deltah /1000.);
	for (++r;r<segend+1;r++) {
		setup_data(r);
		_Pd = _Pu; //s->fcell(r, pupc) + PCONV  ;
		_Flow += ss->fcell(r,brflowc);//brflow comes off at start of the previous segment
_Deltah = - _Deltah;
		_Pu = _Pupfn();
		ss->cell(r, pupc , _Pd - PCONV);
		ss->cell(r, pdownc, _Pu - PCONV);
		ss->cell(r, flowc, - _Flow);
		ss->cell(r, z_calc, _Z);
		//comment out if not wanted : 
		ss->cell(r, densc, density((_Pu +_Pd )/2. , _Tabs , _Z));
		ss->cell(r, headc, ss->fcell(r, densc) * 9.8 * _Deltah  /1000.);
 	}
	r= segend; 
	ss->cell(segend, flowc, - _Flow);
	ss->cell(segend, pupc, ss->fcell(segend-1, pdownc));
	//comment out if not wanted : 
	ss->cell(r, densc, density((_Pu +_Pd )/2., _Tabs , _Z));
	ss->cell(r, headc, ss->fcell(r, densc) * 9.8 * _Deltah  /1000.);
	return _Pu - p_in;
}
double loopfn(double _flow) { 
	//func for zbrent - returns difference between looper and loopee outlet pressures

	int r1=rowfrom, r2=segstart; //localise rowfrom and segstart to preserve their values
	setup_data(r1);
	//loopee pressure run
	_Pu  = loopee_pu;
	_Flow = upstr_flow-_flow;
	_Pd = _Pdownfn();
	ss->cell(r1,pdownc,_Pd -PCONV  );
	ss->cell(r1+1,pupc,_Pd -PCONV  );
	ss->cell(r1,flowc,_Flow);
	ss->cell(r1,z_calc,_Z);
	for (++r1;r1<rowto;r1++) { 
		setup_data(r1);
		_Pu  = ss->fcell(r1,pupc)+PCONV  ; _Flow  = ss->fcell(r1-1,flowc)-ss->fcell(r1,brflowc);
		_Pd = _Pdownfn();
		ss->cell(r1,pdownc,_Pd -PCONV  );
		ss->cell(r1+1,pupc,_Pd -PCONV  );
		ss->cell(r1,flowc,_Flow  );
		ss->cell(r1,z_calc,_Z);
	//	cout<<' '<<flow<<' '<<pd-PCONV  <<endl;
		if (_Pd == PCONV  ) return 0.; //reached minimum pressure
	}
	loopee_pd = _Pd;
	//ss->cell(r1,pupc,pd-PCONV  );
	//looper pressure run
	next_pressure_down(r2,looper_pu,_flow);	//this calls setup_data itself			
	for (++r2;r2<segend+1;r2++) {
		next_pressure_down(r2, _Pd  , _Flow - ss->fcell(r2,brflowc));
		if (_Pd ==PCONV  ) return 0.; //reached minimum pressure
	}
	looper_pd = _Pd ;
	return	looper_pd - loopee_pd;
}
double ringmainfn(double _flow) {
//works so far for ringmain between 2 segments and flow from point to another
	//func for zbrent - returns difference between downstream and upstream pressures at neutral point
 	//main pressure run 
	int r1=rowfrom, r2=segstart; //localise 
	setup_data(r1);
	_Pu  = main_pu;//never change main_pu inside this function!!!
	_Flow  = upstr_flow-_flow;
	double downstream_inlet_pressure=_Pu; 
	if (!flag) {
	_Pd = _Pdownfn();
	ss->cell(r1,pdownc,_Pd -PCONV  );
	ss->cell(r1+1,pupc,_Pd -PCONV  );
	ss->cell(r1,flowc,_Flow  );
	ss->cell(r1,z_calc,_Z);
	for (++r1;r1<rowto;r1++) { 
		setup_data(r1);
		_Pu  = ss->fcell(r1,pupc)+PCONV  ; _Flow  = ss->fcell(r1-1,flowc)-ss->fcell(r1,brflowc);
		_Pd = _Pdownfn();
		ss->cell(r1,pdownc,_Pd -PCONV  );
		ss->cell(r1+1,pupc,_Pd -PCONV  );
		ss->cell(r1,flowc,_Flow  );
		ss->cell(r1,z_calc,_Z);
		if (_Pd ==PCONV  ) return 0.; //reached minimum pressure
	}
	downstream_inlet_pressure = _Pd ;
	}
	//else downstream_inlet_pressure=//whatever it is on the other segment 

	//determine neutral point in ringmain for this trial flow
	double _flowsum=0.;
	int jn;
	for (jn=r2;jn<segend+1;jn++) {
		_flowsum+=ss->fcell(jn,brflowc);
		if (_flow<_flowsum) break;
		}
	//jn; //should be jn-1 but 1 normally added in for loop below
	cout<<"neutral point in ringmain at row "<<jn-1<<endl;
	//determine P at neutral point from upstream end of ringmain or at end of segment
	next_pressure_down(r2,main_pu,_flow);				
	for (++r2;r2<jn;r2++) {
		next_pressure_down(r2, _Pd  , _Flow - ss->fcell(r2,brflowc));
		if (_Pd == PCONV  ) return _Pd; //reached minimum pressure
		}
	/*if (jn==segend+1) {	 
		//neutral point is past last offtake- no ringmain required treat as branch to neutral point
		//cout<<"genr loopfn returns "<<p1-pd<<endl;
		return ss->fcell(rowto,pdownc)+PCONV -pd;//difference between calc outlet press and other connection
	} */
	double p2=_Pd ;
	//determine P at neutral point from downstream end of ringmain
	int r = segend; 
	_Flow = sum_ringmain - _flow;
	next_pressure_down_reverse_flow(r, downstream_inlet_pressure, _Flow );
	for (--r;r>jn-1;r--) {//the neutral point is at the inlet of pipe at row jn
		//brflow comes off at start of the previous segment
		next_pressure_down_reverse_flow(r, _Pu , _Flow - ss->fcell(r+1,brflowc));
		}
	cout<<"ringmainfn returns "<<_Pu  -p2<<endl;
	return _Pu  -p2;	
}

/*********************************************************************
   Given a function func and its initial guessed range x1 to x2, the 
   routine expands the range geometrically until a root is bracketed
   by the returned values x1 and x2 (in which case zbrac returns 1)
   or until the range becomes unacceptably large (in which case zbrac
   returns 0).
   C.A. Bertulani        May/25/2000
*********************************************************************/
#define FACTOR 1.5
#define NTRY 50
int zbrac(double (*func)(double), double *x1, double *x2, double *x1_minimum, double *x2_maximum)
{
	int j;
	double f1,f2;

	if (*x1 == *x2) {cout << "bad initial range in zbrac\n"; return 0;}
	f1=(*func)(*x1);
	f2=(*func)(*x2);
	if (f1==0. || f2==0.) {cout<<"zbrac can't find interval\n"; return 0;}
	cout<<"zbrac start : fl "<<f1<<" fh "<<f2<<endl;
	for (j=1;j<=NTRY;j++) {
		if (f1*f2 < 0.0) {cout <<"zbrac success : fl "<<f1<<" fh "<<f2<< " -no of iterations "<<j<<endl;return 1;}
		if (fabs(f1) < fabs(f2)){
			*x1 += FACTOR*(*x1-*x2);
			if (*x1<*x1_minimum) *x1=*x1_minimum;
			else if (*x1>*x2_maximum) *x1=*x2_maximum;
			f1=(*func)(*x1);
			}
		else {
			*x2 += FACTOR*(*x2-*x1);
			if (*x2<*x1_minimum) *x2=*x1_minimum;
			else if (*x2>*x2_maximum) *x2=*x2_maximum;
			f2=(*func)(*x2);
			}
	}
	cout << "zbrac fail : max double of iterations exceeded "<<j<<endl;
	return 0;
}

double zbrent(double (*func) (double),double a,double b,double tol) {
//adapted from
//*********************************************************************
//   Finds the root of a function, f(x)=0, in an interval x1, x2, using 
//   Brent's method. The root, returned as zbrent, will be refined until
//   its accuracy is tol.
//   This is a robust program, which should be used for general purpose.
//   C.A. Bertulani        May/25/2000
//http://www.tamu-commerce.edu/physics/cab/computer/find_root.cpp
//*********************************************************************
	int ITMAX= 100;
	double EPS= 1.0e-16;//machine accuracy
	double c=b,d=0,e=0,min1,min2;
	double fa=(*func)(a),fb=(*func)(b),fc,p,q,r,s,tol1,xm;
	if ((fa > 0.0 && fb > 0.0) || (fa < 0.0 && fb < 0.0)) {
		cout << "Root must be bracketed in zbrent : "<< a<<' '<<b<<' '<<fa<<' '<<fb<<endl;
		return 0.0;//
		}
	fc=fb;
	for (iterationcount=1;iterationcount<=ITMAX;iterationcount++) {
		if ((fb > 0.0 && fc > 0.0) || (fb < 0.0 && fc < 0.0)) {
			c=a;		/* rename a, b, c, and adjust bounding interval d. */
			fc=fa;
			e=d=b-a;
		}
		if (fabs(fc) < fabs(fb)) {
			a=b;
			b=c;
			c=a;
			fa=fb;
			fb=fc;
			fc=fa;
		}
		tol1=2.0*EPS*fabs(b); //+0.5*tol;		/* Convergence check. Modified to isolate tol1 from tol */
		xm=0.5*(c-b);
		if (fabs(xm) <= tol1 || fabs(fb) < tol) { //fabs(fb) < tol) { // only going for a tolerance in pressure difference  //fabs(xm) <= tol1 || fabs(fb) < 1e-20) {
			//cout<<"xm tol fb "<<fabs(xm) <<' '<<tol<<' '<<fabs(fb)<<endl;
			return b;
		} // fb == 0.0) {  return b;} //
		if (fabs(e) >= tol1 && fabs(fa) > fabs(fb)) {
			s=fb/fa;		/* Attempt inverse quadratic interpolation. */
			if (a == c) {
				p=2.0*xm*s;
				q=1.0-s;
			} else {
				q=fa/fc;
				r=fb/fc;
				p=s*(2.0*xm*q*(q-r)-(b-a)*(r-1.0));
				q=(q-1.0)*(r-1.0)*(s-1.0);
			}
			if (p > 0.0) q = -q;		/* Check whether in bounds. */
			p=fabs(p);
			min1=3.0*xm*q-fabs(tol1*q);
			min2=fabs(e*q);
			if (2.0*p < (min1 < min2 ? min1 : min2)) {
				e=d;		/* Accept interpolation. */
				d=p/q;
			} else {
				d=xm;		/* Interpolation failed, use bisection. */
				e=d;
			}
		} else {		/* Bounds decreasing too slowly use bisection. */
			d=xm;
			e=d;
		}
		a=b;		/* Move last guess to a. */
		fa=fb;
		if (fabs(d) > tol1)		/* Evaluate new trial root. */
			b += d;
		else
			b+=tol1*fabs(xm)/xm;
			fb=(*func)(b);
		//fl_message("zbrent step");
	}
	cerr << "Maximum double of iterationcountations exceeded in zbrent\n";
	return 0.0;		/* Never get here. */
}

int	move_below_range() { 
//check that sfrom and sto are in a prior section either within sstart to send and sfrom to sto 
//or are below (after) one of those ranges in runarray.
//in the latter case move the row below that with the range found
	int count1=0;
	int sfrom1,sto1,sfrom,sto,sstart,send,stype;
	
	for (int s=0;s<nosegs;s++) {
		sfrom1 = ss->icell(ray0+s,Sfrom);
		sto1 = ss->icell(ray0+s,Sto);
		if (!sfrom1 ) {count1++; continue;} //don't bother with empty cells
		stype = abs(ss->icell(ray0+s,Stype)); //get type : abs to handle ! loops/ringmains which have -22,-23 types;
		if (stype==22 || stype==23 )  sfrom1--; //MAY NEED TO EXPAND FOR OTHER CONNECTORS
		//sfrom in loop type segments must be below the end row of its leadin, hence sfrom-1
		cout<<"in runarray row "<<s+1<<" looking for "<<sfrom1<<" in this row or a previous row"<<endl;
		int s1;
		for (s1=0;s1<nosegs;s1++) {  
			sstart = ss->icell(ray0+s1,Sstart);
			send = ss->icell(ray0+s1,Send);
			sfrom = ss->icell(ray0+s1,Sfrom);
			sto = ss->icell(ray0+s1,Sto);
			if (!send) { count1++; goto L0; } //not much use looking in this range as end is zero
			if ((sstart <= sfrom1 && sfrom1 <= send)) // || (sfrom <= sfrom1 && sfrom1 <= sto))
				{
				if (s1<s+1) { //downstream of range so OK
					cout<<"found that "<< sfrom1<<" is within a range ";
					cout<<"in runarray row "<<s1+1<<" OK"<<endl;
					count1++; //when all are found count1 = nosegs
					goto L0;
				}
				else {
					//found it upstream of range ; place in row below
					cout<<"found that "<< sfrom1<<" is within a range but ";
					cout<<"is upstream of that range in runarray row "<<s1+1<<endl;
					cout<<"MOVE DOWNSTREAM"<<endl;
					ss->textcolor(BRIGHT,RED,BLACK);
					cout<<"moving runarray row "<<s+1<<" below "<<s1+1<<endl;
					ss->textcolor(); 
					//insert runarray row after the range
					rebuild_runarrayrow_index(s,s1);
					//now rearrange runarray entries accordingly ready for next pass
					rearrange_by_index();
					
					show_runarray(nosegs);
					s--;//step back a row to make sure value after the swap is also examined!!
					count1--; //will need to check that this SWAP hasn't affected a previous OK
					goto L0;
				}
			}
			//if not found will eventually come here
		} 
		ss->textcolor(BRIGHT,YELLOW,BLACK);
		cout<<"NOT FOUND!\n";
		ss->textcolor();
		L0: if (count1==nosegs) { cout <<"count = "<<count1<<" - all found!\n"<<endl; return 1; }
	}
	return 0;
}	
void runarray_sort() {
/* runarray reordering to provide for proper sequence of solution
RULES:	1.	arrange Sfrom column in ascending order
		2.	ensure each sfrom is in Sstart to Send range of its own segment or one upstream
			if not move its runarray entry to below that of the range
*/
	
	arrayindex.length(nosegs); //set up arrayindex (SDRarray dynamically resizable)
	tempstr.sdrrowscols(nosegs,SOK+1);//set up temp strings for transfer

	//sort first on sfrom
	ss->textcolor(BRIGHT,MAGENTA,BLACK);	
	sort_runarray(Sfrom);
	cout<<" in ascending order of Sfrom\n";
	ss->textcolor();
	

	show_runarray(nosegs);
	ss->textcolor(BRIGHT,GREEN,BLACK);
	cout<<"\nchecking sfrom is in Sstart to Send range of its own segment (leadin) or one upstream\n";
	ss->textcolor();

	move_below_range();
	for (int r=0;r<nosegs;r++) ss->cell(ray0+r,Sno,r+1);
	show_runarray(nosegs);
}

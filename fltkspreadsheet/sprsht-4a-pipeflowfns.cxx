//sprsht v1.2.2
/*
copyright norman bakker (2007) */
//equations for determing flow in gas pipeline networks

static bool Metric_input = true; 

double _Flowfn();
double _Pdownfn();
double _Pupfn();
double _Diamfn();
double _Lengthfn();
//conversion factors multiply metric units by these to obtain english units - see British Gas Data Book vol 1(A) table 1.2
double mscf_per_day = 24. * 35.993/1000.;
double btu_per_cubic_ft = 26.8392;
double mmmbtu_per_day = 0.947817;
double pound_per_footsec = 0.671969;
double psi_per_kpa = 0.145038;
double foot_per_metre = 3.28084;
double in_per_mm = 0.0393701;
double thou_per_micron = 0.0393701;
double miles_per_km = 1./1.60934;
double zfn (double  &press , double &tabs , double &sg , double &n2 , double &co2 );
double ftspl(double &fx,double &nre); 
double _a0 = 1.737177928;
double _a1 = 1.413;
double _a2 = 0.0004336; // was 39.418 after including an assumed viscosity of 11 Pa.s x 1e6
double _a4 = 68081.0; 
double _a5 = 0.0683911;
double _a6 = 3.7;
double _a7 = 4635022561.;
double _a8 = 2.5;
double _a9 = 0.2;
double _a10= 0.3;
double _a11= 3.25;
double _a13= 1./3.;
double _a14= 2.;
//double a9 = 41666.6667; //divide by HHV to convert to TJ/d
double accuracy = 1e-7;
double PCONV = 101.325;
double TCONV = 273.15; 
double _Pu=PCONV ,_Pd =PCONV ,puo,pdo,_Diam=0.,_Leng=0.,_Deltah =0.,_Tabs  ,_SG ,_Eta  ,_Z =0.,_Rough=0.,_Dragf  ;
double _Flow  =0.,_TJpd =0., ftsplo, ftspln, nreo, nren, ft, ffc, nre, pressfvalue, nrefvalue, pav,
 	   a6dke, ftrpl, nrecrit, _tm, pdpd,pupu,  pfX, pfX1,pfY,pfY1, _N2, _CO2, _HHV =0.;
 	   
//takes into account slight difference in std conditions (35.3947

void setup_singlepipe() {
	//setup for a single pipe section calculation of flow given pressures, temperature, and gas
	//and pipe properties
	cout<<"cold start of pipeflow\n";
	ss->clearallcells();
	ss->rrow_height("1",4*ROWHEIGHT);
	ss->ccol_noformat("b");ss->ccol_noformat("f");
	ss->cellwidth("a","a",240); ss->cellwidth("b","d",100);ss->cellwidth("f","h",100);
	ss->ccol_align("a",FL_ALIGN_LEFT);
	ss->ccol_align("b",FL_ALIGN_CENTER);ss->ccol_align("e",FL_ALIGN_CENTER);ss->ccol_align("f",FL_ALIGN_CENTER);
	ss->cell("a1","Flow of natural gas in a pipe");ss->cellalign("a1",FL_ALIGN_WRAP);
	ss->cellfont("a1",FL_TIMES_BOLD);ss->cellcolour("a1",FL_DARK_GREEN);ss->cellfontsize("a1",16);
	ss->cellfont("d3",FL_TIMES_ITALIC);ss->cellfont("h3",FL_TIMES_ITALIC);
	ss->cellcolour("a3",FL_DARK_GREEN);
	ss->cellcolour("a6","a9",FL_DARK_GREEN);
	ss->cellfont("d6","d9",FL_TIMES_ITALIC);ss->cellfont("h6","h9",FL_TIMES_ITALIC);
	ss->cell("b1","based on American Gas Association equation");
	ss->cellalign("b1",FL_ALIGN_WRAP);ss->cellfont("b1",FL_TIMES_ITALIC);
	ss->cellcolour("b1",FL_DARK_GREEN);
	ss->cellcolour("d19",FL_RED);
	ss->cell("a2"," variable");
	ss->cellfont("a2","h2",FL_TIMES_BOLD);
	ss->cellprec("d3",2);ss->cellprec("h3",2);
	ss->cellprec("d3","f3",0);
	ss->cellprec("d4","f4",1);
	ss->cellprec("d5","f7",0);
	ss->cellprec("d8","f8",1);
	ss->cellprec("d9","f10",0);
	ss->cellprec("d12","f12",0);
	ss->cellprec("d19","f19",3);
	
	//unchanging cells
	ss->cell("a3"," flow");
	ss->cell("a4"," higher heating value" );
	ss->cell("a5"," energy flow");
	ss->cell("a6"," upstream pressure"); 
	ss->cell("a7"," downstream pressure");
	ss->cell("a8"," internal diameter ");	
	ss->cell("a9"," length");
	ss->cell("a10"," elevation difference");	
	ss->cell("a11"," pipe roughness");		
	ss->cell("a12"," temperature"); 
	ss->cell("a13"," dynamic viscosity" ); 			
	ss->cell("a15"," S.G."); 				
	ss->cell("a16"," N2" ); 					
	ss->cell("a17"," CO2" ); 				
	ss->cell("a18"," dragfactor" ); 			
	ss->cell("a19"," calculated z"); 
	ss->cell("b2","typical value");
	ss->cell("d2","actual value");
	ss->cell("f2","typical value");
	ss->cell("h2","actual value");
	
	ss->cell("g2","english units");
	ss->cell("f3","100-500,000");ss->cell("g3","thousnd std cf/d");
	ss->cell("f4","1,300-1,500");	ss->cell("g4","btu/std cf");
								ss->cell("g5","billion btu/day");
	ss->cell("f6","150-2,000");	ss->cell("g6","psi gauge");
	ss->cell("f7","150-1,500");	ss->cell("g7","psi gauge");
	ss->cell("f8","4-40");		ss->cell("g8","inch");
	ss->cell("f9","5-1,000");	ss->cell("g9","mile");
	ss->cell("f10","+1,000 -1,000");ss->cell("g10","ft");
	ss->cell("f11","0.5-5");	ss->cell("g11","inch x 1e3");
	ss->cell("f12","60-120");	ss->cell("g12", "deg F");
	ss->cell("f13","~7.4");		ss->cell("g13","lb/ft.s x1e6");
	
	ss->cell("c2","metric units");
	ss->cell("b3","100-500,000");ss->cell("c3","std m3/hr");
	ss->cell("b4","36-40");		ss->cell("c4","MJ/m3");
								ss->cell("c5","TJ/day");
	ss->cell("b6","1,000-15,000");	ss->cell("c6","kPa gauge");
	ss->cell("b7","1,000-10,000");	ss->cell("c7","kPa gauge");
	ss->cell("b8","100-1,000");	ss->cell("c8","mm");
	ss->cell("b9","10-1,000");	ss->cell("c9","km");
	ss->cell("b10","+250 -250");ss->cell("c10","m");
	ss->cell("b11","10-40");	ss->cell("c11","micron");
	ss->cell("b12", "15-50");	ss->cell("c12", "deg C");
	ss->cell("b13","~11");		ss->cell("c13","Pa.s x 1e6");
 
	ss->cell("c14","common");ss->cellfont("c14",FL_TIMES_BOLD);
	ss->cell("b15","0.6-0.7");	ss->cell("c15","-");
	ss->cell("b16","~1");		ss->cell("c16","%v/v");
	ss->cell("b17","~2");		ss->cell("c17","%v/v");
	ss->cell("b18","~0.95-1.0");ss->cell("c18","-");
	ss->cell("b19","0.8-0.9");	ss->cell("c19","-");

	ss->celllock("b1","c20");
	ss->celllock("f1","g20");
	//setup default values	
	const char *input_type = fl_input("Input units - metric/english ?", "m"); //set type of input units 
	if (input_type)  { 
		if (input_type[0] == 'E' || input_type[0] == 'e') { Metric_input = false; cout << input_type[0] << "  Using English units\n"; }
		else { Metric_input = true; cout << input_type[0] << "  Using Metric units\n"; }
	}
	_Flow  = 393038.; 			ss->cell("d3",_Flow); 		ss->cell("h3",_Flow * mscf_per_day);
	_HHV = 38.5;				ss->cell("d4",_HHV);		ss->cell("h4",_HHV * btu_per_cubic_ft);
	_TJpd = 0;					ss->cell("d5",_TJpd);		ss->cell("h5",_TJpd * mmmbtu_per_day);
	_Pu = 6895.+PCONV;			ss->cell("d6",_Pu-PCONV);	ss->cell("h6",(_Pu-PCONV) * psi_per_kpa);
	_Pd = 3700.+PCONV;			ss->cell("d7",_Pd-PCONV);	ss->cell("h7",(_Pd-PCONV) * psi_per_kpa);
	_Diam = 0.847; 				ss->cell("d8",_Diam*1000.);	ss->cell("h8",_Diam * in_per_mm * 1000.);
	_Leng = 1.3e6;				ss->cell("d9",_Leng/1000.); ss->cell("h9",_Leng * miles_per_km/1000.);
	_Deltah = 0.;				ss->cell("d10",_Deltah);	ss->cell("h10",_Deltah * foot_per_metre);		
	_Rough = 15e-6;				ss->cell("d11",_Rough*1e6);	ss->cell("h11",_Rough * thou_per_micron * 1e6);
	_Tabs = TCONV+20.; 			ss->cell("d12",_Tabs-TCONV);ss->cell("h12",(_Tabs - TCONV) * 9./5. + 32.);
	_Eta  = 11e-6;				ss->cell("d13",_Eta*1e6);	ss->cell("h13",_Eta * pound_per_footsec * 1e6);	
	_SG = 0.6; ss->cell("d15",_SG); 
	_N2 = 1.; ss->cell("d16",_N2);
	_CO2 = 2.; ss->cell("d17",_CO2);
	_Dragf = 0.97; ss->cell("d18",_Dragf); 
	_Z = 0.8936; ss->cell("d19",_Z);
	
	//add comment box with usage notes
	fl_color(250,255,191);
	string text="";
	text += " Guide:\n";
	text += " 1.Change data according to initial units choice\n" ;
	text += " 2.Choose a variable by clicking in row (3,6,7,8,or 9)\n" ;
	text += " 3.Press Run or Alt-Z to recalculate that variable";
	ss->fill_drawingmap(1,15,5,4,2,FLOATINGCOMMENT,text, FL_RSHADOW_BOX,0,1,fl_color(), FL_ALIGN_LEFT, FONTSIZE, FONT, FL_DARK_CYAN );
	ss->sheetname[0] = "pipe flow equations";
}

double pavf1(double &pu , double &pd ) {return (pu +pd -(pu *pd )/(pu +pd ))*2./3.;}
double pavf2(double &pu , double &pd ) {return ((pu *pu *pu -pd *pd *pd )/(pu *pu -pd *pd ))*2./3.;}

double density(double press, double tabs , double zf) {
//approximate density of the gas at given pressure, temperature and _Z assuming 
//density at std conditions is approx rho0 = 0.7636 kg/m3 
//rho1 = rh0 x (p1/p0) * (zf0/zf1) x (Tabs0/Tabs1)
	return 0.7636 * (press/PCONV ) * (0.998/zf) * (288.15/tabs );
}

double _Flowfn() {
	pav = pavf1(_Pu,_Pd );
	_Z= zfn(pav,_Tabs  ,_SG ,_N2  ,_CO2 );
	pav = pavf2(_Pu,_Pd );
	_tm = _a4 * pow(_Diam,_a8) * sqrt((_Pu*_Pu-_Pd *_Pd ) - (_a5 * _SG * _Deltah * pav * pav) / (_Tabs  * _Z));
	pressfvalue = _tm / sqrt(_SG * _Tabs  * _Leng* _Z);
	nrefvalue = (_a2/_Eta  ) * _SG / _Diam;
	a6dke = _a6 * _Diam / _Rough;
	ftrpl = _a0 * log(a6dke);
	double flown = ftrpl * pressfvalue;
	nrecrit = _a1 * pow(a6dke,1./_Dragf    ) * ftrpl;
	nren = nrefvalue * flown; 
	if (nren >= nrecrit) return flown;//rough pipe law
	else {
		ftspln = 10.;
    	do {
    		nreo = nren;
      		do {
        		ftsplo = ftspln;
        		ftspln = ftspl(ftsplo, nreo);
			} while (fabs(ftspln - ftsplo) >accuracy);
      		flown = ftspln * _Dragf    * pressfvalue;
      		nren =nrefvalue * flown;
		} while (fabs(nren - nreo) > accuracy);
	} 
	return flown;
}
double _Pupfn() {
	nrefvalue = (_a2/_Eta  ) * _SG / _Diam ;
	nre = nrefvalue * _Flow  ;
	_Flow  = _Flow  <0.1?0.1:_Flow  ; //ensure flow is not below a minimum to avoid overflows
	nre =nrefvalue * _Flow  ;
	nre = nre<10.?10.:nre;//ditto nre
 	double pun = _Pd + 0.1;
 	pdpd = _Pd *_Pd ;
	a6dke = _a6 * _Diam / _Rough;
	ftrpl = _a0 * log(a6dke);
	nrecrit = _a1 * pow(a6dke,1./_Dragf ) * ftrpl;
	pfX1 = _a5 * _SG * _Deltah / _Tabs  ; // sign of _Deltah is unchanged!!
 	pfY1 = _Flow  * _Flow  * _SG * _Tabs  * _Leng/ (_a7 * pow(_Diam ,5));
 	if (nre >= nrecrit) { ft = ftrpl; ffc =1.;}
	else {
		ftspln =1.;
  	 	do {
       		ftsplo = ftspln;
       		ftspln = _a0 * log(nre / (_a1 * ftsplo));
		} while (fabs(ftspln - ftsplo) > accuracy);
	    ft = ftspln;
    	ffc = _Dragf    ;
	}
	do {
    	puo = pun;
    	pav = pavf1(pun,_Pd );
    	_Z= zfn(pav,_Tabs  ,_SG ,_N2  ,_CO2 );
		pav = pavf2(pun,_Pd );
    	pfX = pfX1 * pav * pav / _Z;
    	pfY = pfY1 * _Z/ (ft * ft * ffc * ffc);
    	pun = sqrt((pdpd) + pfX + pfY);
	} while	(fabs(puo - pun) > accuracy);
	return pun;
}
double _Pdownfn() {
	double temp;
	pupu = _Pu*_Pu;
	//cout <<"flow "<<_Flow  <<endl; //no flow just adjust for elevation difference if any
	nrefvalue = (_a2/_Eta  ) * _SG / _Diam ;
	_Flow  = _Flow  <0.1?0.1:_Flow  ; //ensure flow is not below a minimum to avoid overflows
	nre =nrefvalue * _Flow  ;
	nre = nre<10.?10.:nre;//ditto nre
	//cout<<"nre "<<nre<<endl;
	double pdn = _Pu- 0.1;

	a6dke = _a6 * _Diam / _Rough;
	ftrpl = _a0 * log(a6dke);
	nrecrit = _a1 * pow(a6dke,1./_Dragf    ) * ftrpl;
	pfX1 = _a5 * _SG * _Deltah / _Tabs  ; // sign of _Deltah is unchanged
	pfY1 = _Flow  * _Flow  * _SG * _Tabs  * _Leng/ (_a7 * pow(_Diam ,5));
	if (nre >= nrecrit) { ft = ftrpl; ffc =1.;}
	else {
		ftspln =1.;
	 	do {
	   		ftsplo = ftspln;
	   		//cout<<"nre/(_a1 * ftsplo) "<< nre/(_a1 * ftsplo)<<endl;
       		ftspln = _a0 * log(nre / (_a1 * ftsplo));
		} while (fabs(ftspln - ftsplo) > accuracy);
	    ft = ftspln;
	    //cout<<"ft "<<ft<<endl;
    	ffc = _Dragf    ;
	}
	do {
    	pdo = pdn;
    	pav = pavf1(_Pu,pdn);
    	_Z= zfn(pav,_Tabs  ,_SG ,_N2  ,_CO2 );
		pav = pavf2(_Pu ,pdn);
    	pfX = pfX1 * pav * pav / _Z;
    	pfY = pfY1 * _Z/ (ft * ft * ffc * ffc);
    	temp= pupu - pfX - pfY;
    	if (temp<0) return PCONV ;
     	else pdn = sqrt(temp);
	} while	(fabs(pdo - pdn) > accuracy);
	//cout<<"_Deltah converted to del P "<< density(pav,_Tabs ,_Z) * 9.8 * _Deltah /1000.<<endl;
	return pdn;
}
double _Diamfn() {
	pav= pavf1(_Pu ,_Pd );
	_Z= zfn(pav,_Tabs  ,_SG ,_N2  ,_CO2 );
	pav = pavf2(_Pu ,_Pd );
	double diamn =0.001, diamo;
	do {
		diamo = diamn;
		nrefvalue = (_a2/_Eta  ) * _SG / diamn;
		nre = nrefvalue * _Flow  ;
		a6dke = _a6 * diamn / _Rough;
		ftrpl = _a0 * log(a6dke);
		nrecrit = _a1 * pow(a6dke,1./_Dragf    ) * ftrpl;
		if (nre >= nrecrit) { ft = ftrpl; ffc =1.;}
		else {
			ftspln =1.;
	 		do {
       			ftsplo = ftspln;
       			ftspln = _a0 * log(nre / (_a1 * ftsplo));
			} while (fabs(ftspln - ftsplo) > accuracy*accuracy);
	    	ft = ftspln;
   			ffc = _Dragf    ;
		}
	diamn = pow(_Leng/(_a7*ft*ft*ffc*ffc/(_SG *_Z*_Tabs  *_Flow  *_Flow  )*(_Pu *_Pu -_Pd *_Pd -(_a5*_SG *_Deltah *pav*pav)/(_Tabs  *_Z))),_a9);
	} while ((fabs(diamn-diamo)>accuracy) );
	return diamn;
}
double _Lengthfn() {
	pav= pavf1(_Pu ,_Pd );
	_Z= zfn(pav,_Tabs  ,_SG ,_N2  ,_CO2 );
	pav = pavf2(_Pu ,_Pd );
	nrefvalue = (_a2/_Eta  ) * _SG / _Diam ;
	nre = nrefvalue * _Flow  ;
	a6dke = _a6 * _Diam / _Rough;
	ftrpl = _a0 * log(a6dke);
	nrecrit = _a1 * pow(a6dke,1./_Dragf    ) * ftrpl;
	if (nre >= nrecrit) { ft = ftrpl; ffc =1.; }
		else {
		ftspln =1.;
  	 	do {
       		ftsplo = ftspln;
       		ftspln = _a0 * log(nre / (_a1 * ftsplo));
		} while (fabs(ftspln - ftsplo) > accuracy);
	    ft = ftspln;
    	ffc = _Dragf    ;
	}
	double lengn = _a7*pow(_Diam ,5)*ft*ft*ffc*ffc/(_SG *_Z*_Tabs  *_Flow  *_Flow  )*(_Pu *_Pu -_Pd *_Pd -(_a5*_SG *_Deltah *pav*pav)/(_Tabs  *_Z));
	return lengn;
}

int flow_of_gas_in_pipe() {
//determines flow in pipe based on inputs in cell column d
//calculations are in metric units, if necessary converted from/to english units depending on choice of units
//_Pu and _Pd set to show zero gauge pressure
//int    iterations=0;
//read in variables
	//find active cell
	string marker = "<--calc-->";
	if (!ss->_restart) cout<<"continuation run of pipeflow\n";
	ss->cell("e3","e9",""); //clear info column of any prior entry
	ss->cellcolour("d3","d18",ss->localtextcolor);
	ss->cellcolour("h3","h18",ss->localtextcolor);
	ss->cellcolour("a3",FL_DARK_GREEN);
	ss->cellcolour("a6","a9",FL_DARK_GREEN);
	ss->cellfont("a3",FL_TIMES_BOLD);
	ss->cellfont("a6","a9",FL_TIMES_BOLD);
	//ss->cellunlock("c2");
	//ss->cellunlock("g2");
	//calculate the quantity according to the row (Rcurrent) selected : eg only works for flow, pup, pdown, diam, length
	switch (ss->Rcurrent+1) {
		case 3: cout<<"flow";break;
		case 6: cout<<"pup";break;
		case 7: cout<<"pdown";break;
		case 8: cout<<"diam";break;
		case 9: cout<<"length";break;
	}
	//set input data depending on whether metric or english units region is has been selected cursor in col < column 5
	if ( Metric_input == false ) {
		 cout <<" english units input\n";
		 ss->cellcolour("g2",FL_RED);
		 ss->cellcolour("c2",FL_DARK_BLUE);
	}
	else {
		cout<<" metric input\n";
		ss->cellcolour("c2",FL_RED);
		ss->cellcolour("g2",FL_DARK_BLUE);
	}
	if (Metric_input == true) {

		_Flow = ss->fcell("d3"); 
		ss->cell("h3",_Flow * mscf_per_day);
		
		_HHV = ss->fcell("d4");
		ss->cell("h4",_HHV * btu_per_cubic_ft);
		
		_TJpd = ss->fcell("d5");
		ss->cell("h5",_TJpd * mmmbtu_per_day);
		
		_Pu = ss->fcell("d6")+PCONV;
		ss->cell("h6",(_Pu-PCONV) * psi_per_kpa);
		
		_Pd = ss->fcell("d7")+PCONV;
		ss->cell("h7",(_Pd-PCONV) * psi_per_kpa);
		
		_Diam = ss->fcell("d8")/1000.;
		ss->cell("h8",_Diam * in_per_mm * 1000.);
		
		_Leng = ss->fcell("d9") * 1000.;
		ss->cell("h9",_Leng * miles_per_km/1000.);
		
		_Deltah = ss->fcell("d10");
		ss->cell("h10",_Deltah * foot_per_metre);
		
		_Rough = ss->fcell("d11")/1e6;
		ss->cell("h11",_Rough * thou_per_micron * 1e6);
		
		_Tabs  = ss->fcell("d12") + TCONV;
		ss->cell("h12",(_Tabs - TCONV) * 9./5. + 32.);
		
		_Eta = ss->fcell("d13")/1e6;
		ss->cell("h13",_Eta * pound_per_footsec * 1e6);
		
	}
	else {

		_Flow  = ss->fcell("h3")/mscf_per_day; 
		ss->cell("d3", _Flow);
		
		_HHV = ss->fcell("h4")/btu_per_cubic_ft; 
		ss->cell("d4",_HHV);
		
		_TJpd = ss->fcell("h5")/mmmbtu_per_day;
		ss->cell("d5",_TJpd);
		
		_Pu = PCONV + ss->fcell("h6")/psi_per_kpa;
		ss->cell("d6",_Pu - PCONV);
		
		_Pd = PCONV + ss->fcell("h7")/psi_per_kpa;
		ss->cell("d7",_Pd - PCONV);
		
		_Diam = ss->fcell("h8")/in_per_mm/1000.;
		ss->cell("d8",_Diam * 1000.);
		
		_Leng = ss->fcell("h9")/miles_per_km*1000.;
		ss->cell("d9",_Leng /1000.);
		
		_Deltah = ss->fcell("h10")/foot_per_metre;
		ss->cell("d10",_Deltah);
		
		_Rough = ss->fcell("h11")/thou_per_micron/1e6;
		ss->cell("d11",_Rough *1e6);
		
		_Tabs = (ss->fcell("h12")-32.) * 5./9. + TCONV ;
		ss->cell("d12",_Tabs - TCONV);
		
		_Eta = ss->fcell("h13")/pound_per_footsec/1e6;
		ss->cell("d13",_Eta * 1e6);
	}
	
	if (_Pd >=_Pu) ss->cell("g7","error : P down > P up");
	
	_SG = ss->fcell("d15");
		
	_N2 = ss->fcell("d16");

	_CO2  = ss->fcell("d17");
	
	_Dragf = ss->fcell("d18");
			  
switch (ss->Rcurrent+1) {
	case 2: break; //adjusts values in non active units column for any changes in active units column
	default:
	case 3: {//calculation of flow of gas in std m3/hr
		ss->cell("e3",marker); //place marker in calculated value row
		ss->cellcolour("d3",FL_RED); //show calculated values in red
		ss->cellcolour("h3",FL_RED);
		//ss->cellcolour("a3",FL_RED);
		_Flow  = _Flowfn();
		ss->cell("d19",_Z);//show calculated z
		ss->cell("d3",_Flow  );//smooth pipe law
		ss->cell("h3",_Flow  *mscf_per_day);
		ss->cell("d5",_Flow  *_HHV *24/1e6);
		ss->cell("h5",ss->fcell("d5")*mmmbtu_per_day);
		break;
	}
	case 6: { //calculation of upstream pressure
		ss->cell("e6",marker);
		ss->cellcolour("d6",FL_RED);
		ss->cellcolour("h6",FL_RED);
		//ss->cellcolour("a6",FL_RED);
		_Pu= _Pupfn();
		ss->cell("d19",_Z);//show calculated z		
		ss->cell("d6",_Pu-PCONV ); 
		ss->cell("h6",(_Pu-PCONV )*psi_per_kpa);
		break;
	}
	case 7: { //calculation of downstream pressure
		ss->cell("e7",marker);
		ss->cellcolour("d7",FL_RED);
		ss->cellcolour("h7",FL_RED);
		//ss->cellcolour("a7",FL_RED);
		_Pd = _Pdownfn();
		ss->cell("d19",_Z);//show calculated z
		ss->cell("d7",_Pd -PCONV );
		ss->cell("h7",(_Pd -PCONV )*psi_per_kpa);  
		break;
	}
	case 8: { //calculation of diameter
		ss->cell("e8",marker);
		ss->cellcolour("d8",FL_RED);
		ss->cellcolour("h8",FL_RED);
		//ss->cellcolour("a8",FL_RED);
		_Diam = _Diamfn();
		ss->cell("d8",_Diam *1000.);
		ss->cell("h8",_Diam *in_per_mm*1000.);
		ss->cell("d19",_Z);//show calculated z
		//cout<<"iterations "<<iterations<<endl;
		break;
	}
	case 9: { //calculation of length
		ss->cell("e9",marker);
		ss->cellcolour("d9",FL_RED);
		ss->cellcolour("h9",FL_RED);
		//ss->cellcolour("a9",FL_RED);
		_Leng = _Lengthfn();
		ss->cell("d19",_Z);//show calculated z
		ss->cell("d9",_Leng /1000.);
	  	ss->cell("h9",_Leng *miles_per_km/1000.);
	  	break;
	}
	}
	printf("Flow %.0f HHV %.1f TJpd %f Pu %f\nPd %f Diam %f Leng %f Deltah %f\nRough %.0f Tabs %.1f Eta %.1f SG %.4f\nN2 %.1f CO2 %.1f  Dragf %.2f Z %.4f\n", _Flow,_HHV, _Flow  *_HHV *24/1e6, _Pu-PCONV, _Pd-PCONV, _Diam*1000., _Leng/1000., _Deltah, _Rough*1e6, _Tabs-TCONV, _Eta*1e6,
		   _SG, _N2, _CO2,  _Dragf, _Z); 
	return 1;
}

void pipeflow() {
	if (ss->_restart)	setup_singlepipe();
	flow_of_gas_in_pipe();
	//ss->redraw();

}

double ozfn(double &pav, double &tabs ,double &sg ){
	//determines a _Z factor if calculate=0, otherwise returns  zconst
	//from an equation provided by ??
	double x1=0.10008398,x2=0.55143885,xg,_tm;
	xg=x2-sg ;
	_tm=1.8/(348.-293.*xg);
	return 1.-x1/(674.+33.33*xg)*(0.55143885-0.29105342*_tm*tabs )*pav;
}

double zfn (double  &press , double &tabs  , double &sg , double &n2 , double &co2 ) {
// adapted and expanded from AGA equation : calculates _Z as fn of p,t,g,n2,co2
double e5 = 6.89476;
double ft,fp,pi,pi2 ,tau,tau2,tau3,tau4,tau5,n,x,y,b,sb,ex,taud,staud,e,d,f,l,m,h,i,j,k;

ft = 226.29 / (99.15 + (211.9 * sg ) - (co2 + 1.681 * n2));
fp = 156.47 / (160.8 - ((7.22 * sg ) - (co2 - 0.392 * n2)));
pi = 0.001 * press * fp / e5; //see zf
tau = 0.002 * tabs  * 1.8 * ft;

if ((pi <= 0.) || (pi >= 4.) || (tau <= 0.84) || (tau >= 1.4)) return 0.;  //pi was not to be >=2'but seems to work up to 4
pi2 = pi * pi;
y = pi2 * pow(pi,_a10);
tau2 = tau * tau;
tau3 = tau2 * tau;
tau4 = tau3 * tau;
tau5 = tau4 * tau;
m = (0.0330378 / tau2) - ((0.0221323 / tau3) - (0.0161353 / tau5));
n = ((0.265827 / tau2) + (0.0457697 / tau4) - (0.133185 / tau)) / m;
b = (3. - m * n * n) / (9. * m * pi2);
taud = 1.09 - tau;
ex = exp(-20. * taud);

if (tau >= 1.09) goto l1;
if (pi <= 1.3) goto l2;

f = 42.844 * pow(taud,4);
l = 18.028 * pow(taud,3);
h = 2.0167 * taud * taud;
i = 200. * pow(taud,6);
j = 0.03249 * taud;
k = 1. - 0.00075 * y * (2. - ex);

if (tau >= 0.88) goto l3;

x = (pi - 1.3) * ((1.69 * pow(_a14,(1.25 + 80. *(0.88 - tau) * (0.88 - tau))) ) - pi2);
goto l4;
l1:
taud = tau - 1.09;
ex = exp(-20. * taud);
staud = sqrt(taud);
x = 2.17 + (1.4 * staud) - pi;
e = (1. - 0.00075 * y * ex) - 0.0011 * staud * pi2 * x * x;
goto l5;
l2:
e = (1. - 0.00075 * y * (2. - ex)) - 1.317 * pow(taud,2.*_a14) * pi * (1.69 - pi2);
goto l5;
l3:
x = (pi - 1.3) * ((1.69 * pow(2.,1.25)) - pi2);
l4: e = k + 0.455 * (i - (j - (h - (l - f)))) * x;
l5:
x = pi2 * m;
sb = (((9. * n) - 2. * m * (n*n*n)) / (54. * x * pi)) - e / (2. * x);
d = pow( (sb + sqrt((sb * sb) + b * b * b)),_a13);
x = (sqrt((b / d) - (d - n / (3. * pi)))) / ((0.00132 / pow(tau,_a11)  ) + 1.);
return  1. / (x * x);
}

double ftspl(double &fx,double &nre) {return 1.737177928 * log(nre / (1.413 * fx));} //uses _a0,_a1

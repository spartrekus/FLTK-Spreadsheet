//sprsht v1.2.2
/*
copyright norman bakker (2007) */

//user function area

void homeloan() { //no arguments provided: prompt to get them from spreadsheet
//save existing view in clipboard TODO
//TODO increase spreadsheet rows if required after checking what's available
	double prin=10000,intr=5,pintrp,plnrp,dsp,totp,dep;
	double period_payment,valuation= 0,homevalueincrease=0,inflation=0,realvalue=0, mntce=0;
	int peri=12,term,yr0=2007;
	if (ss->_restart) {
	cout <<"starting homeloan for the first time\n";
	ss->clearallcells();
	//restart++;
	ss->cell("a2","=homeln()");    //ensures automatic updating providing homeln() is made a parserfn
	ss->celllock("a2");ss->cellhide("a2");//this cell should not be modified accidentally //hide
    //for (int c=1;c<13;c++) ss->numprecision[c]=6; //set number precision globally
	ss->ccol_precision("c",4);//override for year display
	for (int r=0;r<ss->rows();r++) ss->cells[r][2].thousands(0);//turn off thousands delimiter in year display
	//this lot is only written if restart=0, which only happens if called from the main menubar
	ss->cell("a3","Homeloan calculator ");	ss->ccol_width("a",190);ss->cellcolour(3,"a",FL_DARK_GREEN);
	ss->cell("b3","data "); 				ss->ccol_width("b",80);
	ss->cell("c3","year ");					ss->ccol_width("c",60);
	ss->cell("d3","debt-start yr ");		ss->ccol_width("d",100);
	ss->cell("e3","interest ");				ss->ccol_width("e",80);
	ss->cell("f3","principal ");			ss->ccol_width("f",80);
	ss->cell("g3","int+prin ");				ss->ccol_width("g",80);
	ss->cell("h3","debt-end yr ");		    ss->ccol_width("h",80);
	ss->cell("i2","real ");					ss->ccol_width("i",80);
	ss->cell("j2","real ");					ss->ccol_width("j",80);
	ss->cell("k2","real ");					ss->ccol_width("k",80);
	ss->cell("l2","real ");					ss->ccol_width("l",80);
	ss->cell("i3","valuation ");
	ss->cell("j3","interest ");
	ss->cell("k3","loan repayment");
	ss->cell("l3","maint,rates ");
	ss->cell("a4","enter principal ");
	ss->cell("a5","interest %/a ");
	ss->cell("a6","periods/a ");
	ss->cell("a7","loan term yrs ");
	ss->cell("a8","start year ");
	ss->cell("a11","repayment/period ");
	ss->cell("a13","initial cost ");
	ss->cell("a14","home value increase %/a ");
	ss->cell(15,"a","general inflation %/a ");//ont of the alternative cell addressing modes
	ss->ccol_font("a",ss->localfont+1);//bold font typically current font (default = ss->localfont) + 1
	ss->ccol_lock("a");
	ss->ccol_lock("c");
	ss->rrow_font("2",ss->localfont+1);
	ss->rrow_font("3",ss->localfont+1);
	ss->rrow_lock("3");
	ss->cell(16,"a","maint,rates %/value ");//ont of the alternative cell addressing modes
	ss->cell("a17","=homeln()");    //bracketting input data area seems to provide better updating
	ss->celllock("a17");ss->cellcolour("a17",ss->localbackgroundcolor);
	}
	if (!ss->_restart) cout << "continuation of homeloan\n";
	ss->cellprec("b4","z20",0);
	prin=ss->fcell("b4");
	peri=ss->icell("b6");
	intr=(ss->fcell("b5")/100.)/peri; //convert to per period interest rate
	term=ss->icell("b7");if (!term) term=10;
	yr0=ss->icell("b8");
	valuation=ss->fcell("b13");
	homevalueincrease=(ss->fcell("b14")/100.)/peri;//per period for house increase
	inflation=(ss->fcell(15,"b")/100.)/peri;//per period for inflation rate
	mntce=(ss->fcell(16,"b")/100.); //maintenance, rates ,taxes as a % of home valuation
 	if (prin>0) period_payment = prin * (intr/(1.-pow(1+intr,-term*peri))); else period_payment=0;
    ss->cell("b11",period_payment);
	//clean up in case of a change in term
	for (int r=13;r<ss->rows();r++) for (int c=2;c<11;c++) {
		ss->cells[r][c].cval("");
		ss->cells[r][c].fontx(ss->localfont);
		ss->row_height(r,ROWHEIGHT); //see row 9+inc
		ss->cells[r][c].alignx(FL_ALIGN_RIGHT);
		//if (r>19) ss->cells[r][0].cval("");
	}
	//per period calcn aggregate to per annum
	double  aintrp,alnrp,atotp,cum_atotp=0,cum_aintrp=0,
 			cum_alnrp=0,rpintrp=0,raintrp=0,cum_raintrp=0.,
			rplnrp=0,ralnrp=0,cum_ralnrp=0,amntce=0,cum_amntce=0;
	dep = prin;
	int n=0;
	for (int y=0;y<term;y++) {
		aintrp=0,alnrp=0,atotp=0,raintrp=0,ralnrp=0;
			//cout <<y<<' ';
		ss->cell(y+4,"c",yr0+y);//year 
		for (int p=0;p<peri;p++) {
			n++;
			dsp = dep;
			pintrp= dsp * intr;
			rpintrp=pintrp/pow(1+inflation,n);
			aintrp=aintrp+pintrp;
			raintrp=raintrp+rpintrp;
			plnrp  = period_payment - pintrp;
			rplnrp = plnrp/pow(1+inflation,n);
			alnrp = alnrp + plnrp;
			ralnrp=ralnrp+rplnrp;
			totp  = period_payment;
			atotp = atotp + totp;
			dep   = dsp - plnrp;
		}
		cum_aintrp=cum_aintrp+aintrp;
		cum_raintrp=cum_raintrp+raintrp;
		cum_alnrp=cum_alnrp+alnrp;
		cum_ralnrp=cum_ralnrp+ralnrp;
		cum_atotp =cum_atotp+atotp; //grand total all payments
  		realvalue=valuation*pow(1+homevalueincrease,n)/pow(1+inflation,n);
		amntce=mntce*realvalue;
		cum_amntce=cum_amntce+amntce;
		if (!y) ss->cell(y+4,"d",prin); else ss->cell(y+4,"d",ss->fcell(y+4,"h"));
		ss->cell(y+4,"e",aintrp);
		ss->cell(y+4,"f",alnrp);
		ss->cell(y+4,"g",atotp);
		ss->cell(y+4,"h",dep);
		ss->cell(y+4,"i",realvalue);
		ss->cell(y+4,"j",raintrp);
		ss->cell(y+4,"k",ralnrp);
		ss->cell(y+4,"l",amntce);
	}  	//cout<<endl;
		int inc=1;
		if(term<=12) inc = 10; else inc=10+ term-12;
		ss->cell(7+inc,"d","totals ");ss->cell(7+inc,"e",cum_aintrp);ss->cell(7+inc,"f",cum_alnrp);ss->cell(7+inc,"g",cum_atotp);
		ss->cell(8+inc,"d","real "); ss->cell(8+inc,"e",cum_raintrp);
		ss->cell(8+inc,"f",cum_ralnrp);
		ss->cell(8+inc,"g",cum_raintrp+cum_ralnrp);
		ss->cell(8+inc,"i",realvalue);
		ss->cell(8+inc,"j",cum_raintrp);
		ss->cell(8+inc,"k",cum_ralnrp);
		ss->cell(8+inc,"l",cum_amntce);
		ss->cell(9+inc,"j"," = real cost/period");
		ss->cell(9+inc,"k",((valuation-prin)+cum_amntce+cum_raintrp+cum_ralnrp-realvalue)/(peri*term));
		cout << 'd'<<valuation -prin<<'m'<<cum_amntce<<'i'<<cum_raintrp<<'l'<<cum_ralnrp<<'v'<<realvalue<<'p'<<peri*term<<endl;
		ss->cell(9+inc,"i","real costs - increase in real value");ss->cellalign(9+inc,"i",FL_ALIGN_WRAP);
		ss->rrow_height(ss->itoa(9+inc),4*ROWHEIGHT);
		ss->cellfont(9+inc,"j",ss->localfont);
		ss->cellfont(7+inc,"d",ss->localfont);ss->cellfont(8+inc,"d",ss->localfont);ss->cellfont(9+inc,"d",ss->localfont);
}

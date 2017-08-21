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


//newparser7 as at 20070108
//2002-2-24 Wu Yongwei using a short string to improve map access time
//http://www.geocities.com/yongweiwu

//and nab trial parser starting with Schildt ex and extending to fns,varnames and strings
//now with list for fn arguments
//and tokens struct

Parser::~Parser() { /*
	if (destructorcount==0) {
	map <mstring,string>::iterator v;
	//show what is defined on exit (console)
	//v= tmap.begin();
	//cout << "temp variables\n";
	//while (v!=tmap.end()) {  cout << v->first << ' ' << v->second << ';'; v++;}
	//cout << endl;
	v= vmap.begin();
	cout << "defined variables\n";
	while (v!=vmap.end()) { cout << v->first << ' ' << v->second << ';'; v++;}
	cout <<endl;
	map <mstring,int>::iterator f;
	f= fmap.begin();
	cout <<"defined functions\n";
	while (f!=fmap.end()) { cout << f->first << ' ' << f->second << ';'; f++;}
	cout <<endl;
	tmap.clear();
	vmap.clear();
	fmap.clear();
	destructorcount++;
	} */
}
Parser::Parser() {
	//set upper limit on no of tokens an equation can employ
	for (int i=0;i<MAXTOKENS;i++)  {token[i].val="";token[i].type=EMPTY;notoks=0; tokno=0;}
	//fn or constant name used for map must be <=8 characters to be unique
	//double type fns (lowercase) must be given a unique no
	//(its value is not important) in following list
	//string types start with uppercase also have their own numbering
	//all these numbers reappear in eval_a_fn & eval_a_sfn
		fmap["area"]=1;//area of a circle of radius arg[0]
		//====================================
		fmap["acos"]=20; //c++ fns defined in <cmath>
		fmap["asin"]=21;
		fmap["atan"]=22;
		fmap["atan2"]=23;
		fmap["ceil"]=24;
		fmap["cos"]=25;
		fmap["cosh"]=26;
		fmap["exp"]=27;
		fmap["abs"]=28;
		fmap["fabs"]=28;
		fmap["floor"]=29;
		fmap["fmod"]=30;
		//fmap["frexp"]=31;//not yet implemened (needs a pointer)
		//fmap["ldexp"]=32;
		fmap["log"]=33;
		fmap["log10"]=34;
		//fmap["modf"]=35;//not yet implemented (needs a pointer)
		fmap["pow"]=36;
		fmap["sin"]=37;
		fmap["sinh"]=38;
		fmap["sqrt"]=39;
		fmap["tan"]=40;
		fmap["tanh"]=41;
		fmap["randmax"]=42;
		fmap["srand"]=43;
		fmap["rand"]=44;
		fmap["rint"]=45;//round to nearest integer
		fmap["goal"]=100;//doesn't work
		fmap["homeln"]=800;
		fmap["mytestf"]=999;
		//==============================
		fmap["prod"]=5;
		fmap["fact"]=6;//factorial
		fmap["sum"]=7;
		fmap["Range"]=1; //does nothing at the moment
		fmap["Strcat"]=2;
		fmap["Tolower"]=3;//case
		fmap["Toupper"]=4;
		fmap["Spaces"]=5; //fn provides a variable no of spaces
		fmap["Left"]=6;
		fmap["Right"]=7;
		fmap["Mid"]=8;
		fmap["Substr"]=8;
		fmap["TIME"]=20;
		vmap["pi"]="3.14159265358979323846"; //defined constants go here
		vmap["e_"]="2.71828182845904523536";
		vmap["Str"]="hello ";
		vmap["Sp1"]=' '; //single space
}
template <class Ptype> Ptype &Numparser<Ptype>::eval_a_fn(int cfn) {
	//evaluate a real (double) function
	//arguments available are arg[0],arg[1],arg[2]....
	list<string>::iterator a;
	a = argtoken.begin();
	int i=0;
	int noargs=argtoken.size();
	double arg[noargs];
	presult = 0.0;
	//assign token values here to arg array
	while (a!= argtoken.end())	{arg[i]=atof((*a).c_str());i++;a++;}
	switch(cfn) {
		case 1: presult = 3.14159 * arg[0]*arg[0]; break;//area
		case 5: presult=1.;for (int n=0;n<noargs;n++)presult= presult*arg[n]; break;//prod
		case 6: presult=1.;for(int n=1;n<=arg[0];n++)presult=presult*n;break; //fact (eval as double)
		case 7: presult=0.;for(int n=0;n<noargs;n++)presult= presult+arg[n];break;//sum
		//======================= built in c++ fns
		case 20:presult=  acos(arg[0]);break;
		case 21:presult=  asin(arg[0]);break;
		case 22:presult=  atan(arg[0]);break;
		case 23: if (noargs==2){presult=atan2(arg[0],arg[1]);break;}//atan (arg[0]/arg[1])
				 else {cout<<"incorrect arguments\n";break;}
		case 24:presult=  ceil(arg[0]);break;
		case 25:presult=  cos(arg[0]);break;
		case 26:presult=  cosh(arg[0]);break;
		case 27:presult=  exp(arg[0]);break;
		case 28:presult=  fabs(arg[0]);break;
		case 29:presult=  floor(arg[0]);break;
		case 30: if (noargs==2){presult= fmod(arg[0],arg[1]);break;}
				 else {cout<<"incorrect arguments\n";break;}
		//case 31:frmod not implemented
		//case 32:presult=  ldexp(arg[0],arg[2]);break;//arg[0]* 2^arg[1]
		case 33:presult=  log(arg[0]);break;//nat log
		case 34:presult=  log10(arg[0]);break;
		//case 35: modf not implemented
		case 36: if (noargs==2){presult=pow(arg[0],arg[1]);break;} //arg[0] ^ arg[1]
				 else {cout<<"incorrect arguments\n";break;}
		case 37:presult=  sin(arg[0]);break;
		case 38:presult=  sinh(arg[0]);break;
		case 39:presult=  sqrt(arg[0]);break;
		case 40:presult=  tan(arg[0]);break;
		case 41:presult=  tanh(arg[0]);break;
		case 42:presult=  RAND_MAX;break;
		case 43:srand((unsigned) arg[0]); presult=arg[0]; break;
		case 44:presult=  rand();break;
		case 45:presult=  rint(arg[0]);break;//round to nearest integer
		case 100:
		{
		}
		case 800: {ss->_restart = 1; homeloan(); break;}//option 1 assumes this is a restart
		case 999: {
			//mytestfunction
			presult=0;
			noargs=1;
			for (int n=0;n<noargs;n++) {presult=123.456;break;}
		}
	}
	return presult;
}
string & Strparser::eval_a_sfn(int cfn) {
	//evaluate a string function
	list<string>::iterator a;
	a = argtoken.begin();
	int noargs=argtoken.size();
	sresult='"'; //ensures sresult is recognised by tokenise as a string type
	switch(cfn) {
		case 0: sresult=1;break;
		case 1: sresult = ss->range; break; //inserts range of cells eg c1,c2,d1,d3,...
		case 2: while(a!=argtoken.end()){sresult=sresult+*a;a++;}break;//Strcat
		case 3: for (unsigned n=0;n<(*a).size();n++) {(*a)[n]=tolower((*a)[n]);} sresult=sresult+*a;break;//lower
		case 4: for (unsigned n=0;n<(*a).size();n++) {(*a)[n]=toupper((*a)[n]);} sresult=sresult+*a;break;//UPPER
		case 5: for(int n=0;n<atoi((*a).c_str());n++) {sresult=sresult+' ';} break;//Spaces
		case 6: {if (noargs==2) {  //20070824
					sresult = *a; a++; //Left(str,n)
					int len = atoi((*a).c_str());
					sresult=sresult.substr(0,len);
					break;
					}
				else {
				sresult="Ans=args:2";//args:2 gets displayed if insufficient args
				break; }
				}
		case 7: {if (noargs==2) {//20070824
					sresult=*a; a++;  //Right(str,n)
					int sz=(int) sresult.size();
					int len = atoi((*a).c_str());
					if (len>sz) len=sz;
					sresult=sresult.substr(sz-len,len);
					break;
					}
				else {
				sresult="Ans=args:2";
				break; }
				}
		case 8: {if (noargs==3) {//20070824
			   		sresult=*a; a++;  //Substr(str,s,n)
					int s = atoi((*a).c_str());a++;//starting index 1 origin
					int e = atoi((*a).c_str());//no of chars
					if (s<1) s=1; if (s>(int) sresult.size()) s=(int) sresult.size();
					sresult=sresult.substr(s-1,e);
					break;
				}
				else {
				sresult="Ans=args:3";
				break; }
		}
		case 20: sresult='"'+ss->getlocaltime();break;
		default: sresult="=Ans=NoStrfnfound ";break;
	}
	return sresult;
}



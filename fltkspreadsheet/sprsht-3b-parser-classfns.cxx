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

string & Parser::findvar(mstring &name) {
	//cout << "in findvar : looking for "<<name<< " in following ";
	map<mstring,string>::iterator p;
	//p = tmap.begin();
	//while (p!=tmap.end()) { cout << p->first << ' ' << p->second << ';'; p++;}
	//cout <<endl;
	p = tmap.begin();
	p = tmap.find(name); //is it among temp variables
	if (p!=tmap.end()) {
		//cout<<name << " already defined, contains "<< p->second<<endl;
		return p->second; }
	p = vmap.begin();
	p = vmap.find(name);
	if (p!=vmap.end()) {
		//cout<<name << " predefined, contains "<< p->second<<endl;
		return p->second; }
	else {
		//var not defined see if its a valid cellname and copy cell's value;
		// << "var not defined create name based on cell reference and copy cell's value\n";
		int r,c,j=0;
		string temp, str;
		str = (*currenttoken).val;
		while(isalpha(str[j])) j++;
		c = ss->label_to_int(temp.assign(str,0,j));//0 origin
		r = atoi(temp.assign(str,j,str.size()).c_str())-1; //-1 as we are numbering rows from 1 and we want 0 origin below
		ss->temp = ss->cells[r][c].cval(); //a cell reference eg r12
		//cout << "cell " <<name << " contains " <<ss->temp<<endl;
		return ss->temp; //contents of cell[r][c]
	}
}
void Parser::findfnend(string &exp, unsigned i) {
	//find end of function in exp that starts at i
	int openers=0; //check for any inner ( brackets inside fn argument
	int closers=0;
		while (i<exp.size()) {
			if (exp[i]=='(') openers++;
			if (exp[i]==')') closers++;
			if (openers>0 && openers==closers){ en= i; return;} // ( and ) balance
			else i++;
	}
	en = -1;//return -1; //none found
	return;
}
void Parser::extractfn(string &exp) {
	if (en==0) return;
	//extracts fn from exp into fntoken
	fntoken.assign(exp,st,en-st+1);
	return;
}
void Parser::range_expander(string &t) {
	//called by evalfns to replace any range expressions within fn argument list with their expansion
	//check expression for presence of a range delimiter ( ':')
	//and replace this in the list with the expanded range eg c1:d2 ->c1 c2 d1 d2
	//assumes single line or rectangular selections
	stringstream st;
	unsigned s=0,e=0;
	string tok1,tok2,col,res="",temp;
	int r1,r2,j;
	int c1,c2;
	char stchar = 'a';
	if (isupper(t[0])) stchar='A';
	//if (stringflag==1) stchar='A'; //ssssssssssssssssssssssssssssssssss
	for (unsigned i=0;i<t.size();i++) {
		if (t[i]==':') {
		// test for valid range expression
			if (i<2) { serror(9); return;}
			if (!isdigit(t[i-1])) {serror(9); return;}
			//find adjacent , or ;  or ( or )
			s=e=i;
			while (t[s] !=',' && t[s]!=';' && t[s] !='(') s--;//there is at least a (
			if (!isalpha(t[i+1]))  {serror(9); return;}
			while (t[e] !=',' && t[e]!=';' && t[e] !=')') e++;//there is at least a )
			tok1.assign(t,s+1,i-s-1);
			tok2.assign(t,i+1,e-i-1);
			//convert cell references to integers
			j=0;
			while(isalpha(tok1[j])) j++;
			c1 = ss->label_to_int(temp=temp.assign(tok1,0,j).c_str()); //treat alpha part
			r1 = atoi(temp.assign(tok1,j,tok1.size()).c_str()); //int part
			j=0;
			while(isalpha(tok1[j])) j++;
			c2 = ss->label_to_int(temp=temp.assign(tok2,0,j).c_str());
			r2 = atoi(temp.assign(tok2,j,tok2.size()).c_str());
			if (c1>c2 || r1>r2) {serror(9);return; }//range sequence error
			//generate cell range
 			for (int c=c1;c<c2+1;c++) {
 				col = ss->int_to_col_label(c,stchar);
				for (int r=r1;r<r2+1;r++) {
				  	st << r;
					res+= col + st.str() +',';
					st.str("");
				}
			}
			//replace range reference with cell range
			t.replace(s+1,e-s-1,res.erase(res.size()-1,1));
			cout<<"found range expr "<<t<<" expanded to "<<res; //show range determined
			res="";
		}
	} cout<<endl;
}
void Parser::tokenise_arguments(string &t) {
	 //splits up fn arguments into separate ones based on ',' or ';' delim
	register unsigned n=0;					//this delim has to be added in findfnstart
	register unsigned i;
	unsigned ss = t.size(); // 20080319 see 2b too-1;
	int openers = 0;
	try{
	string temp;
	argtoken.clear();//clear list of any prior arguments
	for (i=0;i<ss;i++) if (t[i]=='(') {n=i+1; break;} //count past fn name and opening (
	openers++;
	while (t[n]=='(') {openers++; n++;} //any more (?
	ss = ss - openers +1; //ignore the closing brackets
	for (i=n;i<ss;i++) {
		if (t[i]==';' || t[i]==',') {
			if (i>n) {
				//save this token
				argtoken.push_back(temp.assign(t,n,i-n));
 				//cout << temp << endl;
				n=i+1; //save start new arg in n
			}
			else  {argtoken.push_back(" ");n++;}// a default argument - insert space
		}
	}
	//final argument, if there is one other than a delimiter
	if (n<ss) {
		argtoken.push_back(temp.assign(t,n,ss-n)); //dont want trailing )
		//cout << temp << endl;
	}
	//cout<<"tokenised arguments"<<endl;
	//list <string>::iterator a;
	//a = argtoken.begin();
	//while (a!=argtoken.end()) {cout <<*a<<endl; a++; }
	return;
	}
	catch(...) {cout<<"data error in tokenise called by getdata\n";
	return;}
}
string Parser::serror(int error) {
	//ss->errorflag=1;
	const char *e[] = {
		"syntax error",
		"unbalanced parentheses",
		"no expression present",
		"variable not defined",
		"string error",
		"function does not exist", // index 5 to here
		"error in token type in atom",
		"max number of expression tokens exceeds array size available",
		"max number of arguments exceeds array size available",
		"range error",
  		"incorrect number of arguments for function"
		};
		return e[error];
}
//===================================================================================================
void Strparser::satom(string &sresult) {
	//cout << "in satom " << stringflag << ' ' <<(*currenttoken).val <<  endl;//
	mstring tok((*currenttoken).val);
	switch ((*currenttoken).type) {
		case STRINGVAR: sresult = findvar(tok); gettoken(); return;
		case STRING: sresult =(*currenttoken).val; gettoken(); return;
		case DELIM:  return;
		default: serror(4);
	}
}
void Strparser::sevalexp6(string &sresult) { //parenthesized expression
	//cout << "in sevalexp6 "   << stringflag << ' ' <<(*currenttoken).val <<  endl;
	if((*currenttoken).val[0]=='(') {
		gettoken();
		sevalexp2(sresult);
		if((*currenttoken).val[0]!=')') serror(1);
		gettoken();
	}
	else satom(sresult);
}
void Strparser::sevalexp5(string &sresult) {
	//currently bypassed
	//unary
	//cout << "in sevalexp5 "   << stringflag << ' ' <<(*currenttoken).val <<  endl;
	register char op;
	char tok = (*currenttoken).val[0];
	if(((*currenttoken).type==DELIM) && ((tok=='+') || (tok=='-'))) {
		op = tok;
		gettoken();
	}
	sevalexp6(sresult);
//could do something here with unary + and - for strings
}
void Strparser::sevalexp2(string &sresult) { //concatenation and substring subtraction
	//cout << "in sevalexp2 "   << stringflag << ' ' <<(*currenttoken).val <<  endl;
	string temp;
	register char tok;
	sevalexp6(sresult); //bypass sevalexp5
	//the following is necessary to isolate delimiters from any returned string to avoid losing anything else
	while ((tok=(*currenttoken).val[0])=='+' || tok =='-') { // || tok=='/' || tok=='^' || tok=='%' ) {
		gettoken();
		sevalexp6(temp); //bypass sevalexp5
		switch (tok) {
		case '+': sresult=sresult+temp; break;
		case '-': {
   			unsigned long i = sresult.find(temp);
  			while (i!=string::npos) {//subtract all ocurrences of substr
  				sresult.erase(i,temp.size());
  				i = sresult.find(temp);
  				}
				break;
			}
		}
	}
}
void Strparser::sevalexp1(string &sresult) {
//	cout << "in sevalexp1 "   << stringflag << ' ' <<(*currenttoken).val <<  endl;
		mstring name((*currenttoken).val); //save name
		gettoken(); //find the next delimiter
		register char tok = (*currenttoken).val[0];
		if (tok == '=' ) { //assignment of a value to the variable at start of expression
			//avoid names which can be interpreted as a valid cell reference
			gettoken(); //get rhs
			sevalexp2(sresult); //evaluate rhs
			vmap[name] = sresult; //store name in vmap
			return;
		}
		else if (tok == '(') { //at start of expression- must be a string fn - see if it is defined in fmap
			map <mstring,int>::iterator p;
			p = fmap.find(name);
			if (p==fmap.end()) {serror(5);return;}//fn does not exist
			currentfn = p->second; //its index no
			gettoken(); // get argument
			sevalexp2(sresult); //evaluate argument
			sresult = eval_a_sfn(currentfn); //evaluate a string function
			return;
		}
		else {
			puttoken();//put token back as not an assignment
			sevalexp2(sresult);
			return;
		}
	sevalexp2(sresult);
}
string & Strparser::sevalexp(string &exp, int r, int c) {
	stringstream s;
	//cout << "in sevalexp " <<exp << endl;
	notoks = stokenise(exp); //setup tokens
	gettoken();
	if (notoks==0) {serror(2);return sresult=' ';}
	sevalexp1(sresult); //evaluate as a string expression
	//cout << sresult << endl;
	//cout << "evaluating as a string expression"<<endl;
	return sresult;
}
void Strparser::sevalfns(string &exp) {
	//printf("evaluating        :   %s\n",exp.c_str());
	nospace(exp); //get rid of spaces i
	int tst=-1;
	st=1;
	while (st>0) {
		sfindfnstart(exp); //makes st=-1 if no fn present
		if (st>=0) {
			findfnend(exp,st);//find the next delimiter
			if (en>st) {
				do {  //look for any inner function:
					tst=st; //save start of last fn found
					sfindfnstart(exp,arg); //look from start of fn argument for another fn
				} while (st>0);
				st = tst; //restore st
				//printf("fn found at %5d : ",st);
				findfnend(exp,st);
				extractfn(exp); //put fn(arg) in fntoken
				//if (islower(fntoken[0])) stringflag=0; /////////////
				range_expander(fntoken); //expand any ranges in fntoken
			}
			//printf("%20s\n",fntoken.c_str());
			//check for multiple arguments in fntoken-no should comply with fn definition
			tokenise_arguments(fntoken);
			list <string>::iterator a;
			a = argtoken.begin();
			//evaluate arguments separately
			while (a!=argtoken.end()) {sresult="";*a= sevalexp(*a); a++; } //gets bypassed if no arguments
			//20070824 added sresult="" otherwise further arguments starting with + or - are con(sub)catenated to preceeding one
			//printf("result is : %25s\n",sresult.c_str()); }
			//replace argument with its evaluated result
			exp.replace(st,en-st+1,sevalexp(fntoken));
			//printf("expr, flag  %s \n",exp.c_str());
		}
	}
}

int Strparser::stokenise(string &t) { //splits up expression into tokens and types them
	//types are based on leading character of a token :
	//" a string exp;   upper s string var or fn
	// upperalpha is a string variable
	// also used for cell references which can be manipulated by relative cell expressions
	// if followed by a number
	// an absolute cell reference is followed immediately by a !
	register unsigned token_no=0, n=0;
	register char ch;
	register unsigned i;
	register unsigned ss = t.size();//save string size in ss
	tokno = -1; //for gettoken() as it starts with tokno++
	for (i=0;i<ss;i++) {
		if (strchr(delimiters.c_str(),t[i])) {
			// if i=n we have adjacent delimiters
			if (i>n) { //if here, we have just passed a delimiter
				ch = t[n];
				if (ch=='"') {n++;token[token_no].type=STRING;}//is this now necessary????
				// " means treat rest as string (but drop " - hence n++)
				else if (isupper(ch)) {token[token_no].type=STRINGVAR;}//a string exp
				else token[token_no].type=STRING;
			token[token_no].val.assign(t,n,i-n); //save this token
			if(token_no<MAXTOKENS-1) token_no++;//dont exceed array size
			else {serror(7); return token_no;}
			}
			token[token_no].type=DELIM; //next token has to be a delim
			token[token_no].val.assign(t,i,1); //no different to token[c++\=t[i]
			if (token_no<MAXTOKENS-1) token_no++;
			else {serror(7); return token_no;}
			n=i+1; //save start new token in n
		}
	}
	//final token if there is one other than a delimiter
	if (n<ss) {
		ch = t[n];
		if (isupper(ch)) {token[token_no].type=STRINGVAR;}
		else if (ch=='"') {token[token_no].type=STRING; n++;}//n++ to not select "
		else {token[token_no].type=STRING;} //allows for numbers to be read as strings  //rev
		token[token_no].val.assign(t,n,ss-n);
		//cout << "inside tokeniser " <<token[token_no]<<' ' << n <<endl;
		if (token_no<MAXTOKENS-1) token_no++;
		else { serror(7); return token_no;}
	}
	return token_no; //delim at end of expression
}
void Strparser::sfindfnstart (string &exp, unsigned i) { //smallest fn is a(... )
	//find first function in exp
	char delim[]="+-=();,"; //;, added here to avoid a false fn find
	while (i<exp.size()) {
		if (exp[i]=='(' && !strchr(delim,exp[i-1])) { //found a fn
			arg=i+1;//this where argument starts
			while (i && !strchr(delim,exp[i-1])) i--; //find start of name
			st = i;
			return;
			}
		else i++;
	}
	st= -1; //none found
	return;
}
//======================================================================================
template <class Ptype> void Numparser<Ptype>::atom(Ptype &presult) {
	//cout << "in atom " << (*currenttoken).type << ' ' <<(*currenttoken).val <<  endl;
	mstring tok((*currenttoken).val);
	switch ((*currenttoken).type) {
		case VAR:
			presult=atof(findvar(tok).c_str());
			gettoken();
			return;
		case NUM:
			presult=atof((*currenttoken).val.c_str());
			gettoken();
			return;
		case DELIM:
			return;
		default:  serror(6);
	}
}
template <class Ptype> void Numparser<Ptype>::evalexp6(Ptype &presult) { //parenthesized expression
	//cout << "in evalexp6 "  <<  endl;
	if((*currenttoken).val[0]=='(') {
		gettoken();
		evalexp2(presult);
		if((*currenttoken).val[0]!=')') serror(1);
		gettoken();
	}
	else atom(presult);
}
template <class Ptype> void Numparser<Ptype>::evalexp5(Ptype &presult) { //unary + or -
	//cout << "in evalexp5 " << endl;
	register char op=0;
	char tok = (*currenttoken).val[0];
	if (( (*currenttoken).type==DELIM ) && (( tok=='+') || (tok=='-'))) {
		op=tok;
		gettoken();
	}
	evalexp6(presult);
	if (op=='-') presult= -presult;
}
template <class Ptype> void Numparser<Ptype>::evalexp4(Ptype &presult) {//integer exponent
	//cout << "in evalexp4 " <<  endl;
	Ptype temp,ex;
	register int t;
	evalexp5(presult);
	if ((*currenttoken).val[0]=='^') {
		gettoken();
		evalexp4(temp);
		ex = presult;
		if (temp==0.0) {
			presult = 1.0;
			return;
		}
		for (t=(int) temp-1;t>0;--t) presult=presult * (Ptype) ex;
	}
}
template <class Ptype> void Numparser<Ptype>::evalexp3(Ptype &presult) {//multiply or divide
	//cout << "in evalexp3" <<endl;
	Ptype temp;
	register char op;
	evalexp4(presult);
		while ((op=(*currenttoken).val[0]) =='*'|| op=='/' || op=='%') {
			gettoken();
			evalexp4(temp);
			switch (op) {
				case '*': presult=presult*temp; break;
				case '/': presult=presult/temp; break;
				case '%': presult=(int) presult % (int) temp; break; //modulus
			}
		}
}
template <class Ptype> void Numparser<Ptype>::evalexp2(Ptype &presult) { //addn or substraction
	//cout <<"in evalexp2 " <<endl;
	Ptype temp;
	register char op;
	evalexp3(presult);
	while ((op=(*currenttoken).val[0]) =='+'|| op=='-') {
		gettoken();
		evalexp3(temp);
		switch (op) {
			case '-': presult=presult-temp; break;
			case '+': presult=presult+temp; break;
		}
	}
}
template <class Ptype> void Numparser<Ptype>::evalexp1(Ptype &presult) {
	//cout << "in evalexp1 " << endl;
	if ((*currenttoken).type==VAR) { //for a var, a var assignment, or a fn
		mstring name((*currenttoken).val); //save name
		gettoken(); //find the next delimiter
		register char tok = (*currenttoken).val[0];
		if (tok == '=' ) { //assignment of a value to the variable at start of expression
			//avoid names which can be interpreted as a valid cell reference
			gettoken(); //get rhs
			evalexp2(presult); //evaluate rhs
			stringstream s;
			s << presult;
			vmap[name] = s.str(); //store in name in vmap
			//cout << "created " <<name <<" and assigned "<<s.str()<<endl;
			return;
		}
		else if (tok == '(') { //at start of expression- must be a fn - see if it is defined in fmap
			map <mstring,int>::iterator p;
			p = fmap.find(name);
			if (p==fmap.end()) {serror(5);return;}//fn does not exist
			currentfn = p->second; //its index no
			//cout << name << ' ' << name.index << endl; //actual LONGLONG value
			gettoken(); // get argument
			evalexp2(presult); //evaluate argument
			presult = eval_a_fn(currentfn); //evaluate function
			return;
		}
		else {
			puttoken();//put token back as not an assignment
			evalexp2(presult);
			return;
		}
	}
	evalexp2(presult);
}
template <class Ptype> int Numparser<Ptype>::tokenise(string &t) { //splits up expression into tokens and types them
	//types are based on leading character of a token :
	// lower a number variable or fn
	// (default) is a number (including scientific notation)
	// also used for cell references which can be manipulated by relative cell expressions
	// if followed by a number
	// an absolute cell reference is followed immediately by a !
	register unsigned token_no=0, n=0;
	register char ch;
	register unsigned i;
	register unsigned ss = t.size();//save string size in ss
	tokno = -1; //for gettoken() as it starts with tokno++
	for (i=0;i<ss;i++) {
		if (strchr(delimiters.c_str(),t[i])) {
			// if i=n we have adjacent delimiters
			if (i>n) { //if here, we have just passed a delimiter
				ch = t[n];
				if (islower(ch))
				{token[token_no].type=VAR;}	// if followed by (will be a fn)
				else
				{
					token[token_no].type=NUM;
					if (t[i-1]=='e' || t[i-1]=='E') {// scientific format eg e+04 or E+04
						i=i+1; //may be followed by sign
						while (isdigit(t[i])) i++; //collect exponent
					}
				}
			token[token_no].val.assign(t,n,i-n); //save this token
			if(token_no<MAXTOKENS-1) token_no++;//dont exceed array size
			else {serror(7); return token_no;}
			}
			token[token_no].type=DELIM; //next token has to be a delim
			token[token_no].val.assign(t,i,1); //no different to token[c++]=t[i]
			if (token_no<MAXTOKENS-1) token_no++;
			else {serror(7); return token_no;}
			n=i+1; //save start new token in n
		}
	}
	//final token if there is one other than a delimiter
	if (n<ss) {
		ch = t[n];
		if (islower(ch)) {token[token_no].type=VAR;}
		else token[token_no].type=NUM; //scientific filtered above
		token[token_no].val.assign(t,n,ss-n);
		//cout << "inside tokeniser " <<token[token_no]<<' ' << n <<endl;
		if (token_no<MAXTOKENS-1) token_no++;
		else { serror(7); return token_no;}
	}
	return token_no; //delim at end of expression
}
template <class Ptype> void Numparser<Ptype>::findfnstart (string &exp, unsigned i) { //smallest fn is a(... )
	//find first function in exp
	char delim[]="+-*/%^=();,"; //;, added here to avoid a false fn find
	while (i<exp.size()) {
		if (exp[i]=='(' && !strchr(delim,exp[i-1])) { //found a fn
			arg=i+1;//this where argument starts
			while (i && !strchr(delim,exp[i-1])) i--; //find start of name
			st = i;
			return;
			}
		else i++;
	}
	st= -1; //none found
	return;
}
template <class Ptype> void Numparser<Ptype>::evalfns(string &exp) {
	//printf("evaluating        :   %s\n",exp.c_str());
	nospace(exp); //get rid of spaces i
	int tst=-1;
	st=1;
	while (st>0) {
		findfnstart(exp); //makes st=-1 if no fn present
		if (st>=0) {
			findfnend(exp,st);//find the next delimiter
			if (en>st) {
				do {  //look for any inner function:
					tst=st; //save start of last fn found
					findfnstart(exp,arg); //look from start of fn argument for another fn
				} while (st>0);
				st = tst; //restore st
				//printf("fn found at %5d : ",st);
				findfnend(exp,st);
				extractfn(exp); //put fn(arg) in fntoken
				//if (islower(fntoken[0])) stringflag=0; /////////////
				range_expander(fntoken); //expand any ranges in fntoken
			}
			//printf("%20s\n",fntoken.c_str());
			//TODO check for multiple arguments in fntoken-no should comply with fn definition
			tokenise_arguments(fntoken);
			list <string>::iterator a;
			a = argtoken.begin();
			//evaluate arguments separately
			while (a!=argtoken.end()) {*a= evalexp(*a); a++; } //gets bypassed if no arguments
			//printf("presult is : %25s\n",temp.c_str());
			//replace argument with its evaluated presult
			exp.replace(st,en-st+1,evalexp(fntoken));
			//printf("expr, flag  %s %d \n",exp.c_str(),stringflag);
		}
	}
}
template <class Ptype> string & Numparser<Ptype>::evalexp(string &exp, int r, int c) {
	//called by
	stringstream s;
	//cout << "in evalexp " <<exp << endl;
	notoks = tokenise(exp); //setup tokens
	gettoken();
	if (notoks==0) {serror(2);return sresult=' ';}
	evalexp1(presult);
	s.precision(18); //to retain precision when converting to/from string format (default is 6)
	s <<presult;
	return sresult=s.str();
}









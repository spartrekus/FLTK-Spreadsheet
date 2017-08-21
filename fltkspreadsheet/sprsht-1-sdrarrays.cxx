//sprsht	v1.x.x
/*Copyright (C) 2007 norman a bakker*/

//zero only origin option
template <class Atype> class SDRarray 		//based on Schildt p479 and Bruno Preiss
{											//SDR = safe dynamically resizable (hopefully)
	protected:
		long 	len;						// 0 origin
		Atype* 	val;
	public:
		SDRarray() : len (0), val (new Atype [0]) {}
		SDRarray(long alength) :
				len(alength), val(new Atype [alength]) {}
		SDRarray (SDRarray const& array) :  //copy constructor
				len (array.len),
				val (new Atype [array.len])
		{
			for (long i = 0; i < len; ++i) val [i] = array.val [i];
		}
		~SDRarray() {delete [] val;}
		Atype & operator[](long r)
		{	// use cout statements only for development as they throw this function outline
			if (0 <= r && r < len) return val[r];
			//if (r < 0) 	{cout << "SDRarray index error < 0\n"; 			return val[0];}
			//else 	 	{cout << "SDRarray index error >= "<<len<<endl;	return val[len-1];}
			if (r < 0) 	return val[0];
			else 	 	return val[len-1];
		}
		long length() const {return len;}
		void length(long newlength)
		{
			Atype* const newarray = new Atype [newlength];
			long const minlength =	len < newlength ? len : newlength;
			for (long i = 0; i < minlength; ++i)	newarray [i] = val [i];
			delete [] val;
			val = newarray;
			len = newlength;
		}
};

template <class T> class SDRarray2D
{
	protected:
		long nrows;
		long ncols;
		SDRarray<T> array;
	public:
		class Row
		{
			SDRarray2D& array2D;
			long const row;
			public:
				Row () : array2D (0),row(0)	{}
				Row (SDRarray2D & _array2D, long _row) : array2D (_array2D),row (_row) {}
				~Row() {}
				T& operator [] (int column) const {return array2D.select(row,column);}
		};

		SDRarray2D () : nrows (0), ncols (0), array (0)	{}
		SDRarray2D (long m, long n) : nrows (m), ncols (n), array (m * n) {}
		~SDRarray2D() {}
		T& select (long r, long c)
		{	//use of cout statements throws this function outline
			if (r >= nrows) {r = nrows -1;} //	cout << "SDRarray2D  row index error >= " << nrows << endl;}
			if (c >= ncols) {c = ncols -1;} //	cout << "SDRarray2D  col index error >= " << ncols << endl;}
			return array [r * ncols + c];
		}
		Row  operator [] (long row) {return Row (*this, row);}
		long sdrrows() const {return nrows;}
		long sdrcols() const {return ncols;}
		void sdrrows(long newnrows) {array.length(newnrows * ncols);	nrows = newnrows;}
		void sdrcols(long newncols) {array.length(nrows * newncols);	ncols = newncols;}
		long sdrrowscols() const{return nrows * ncols;}
		void sdrrowscols(long newnrows, long newncols)
		{
			array.length(newnrows * newncols);
			nrows = newnrows;
			ncols = newncols;
		}
};

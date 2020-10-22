#include <fstream>
#include <sstream>
#include <string>

#include "ntk.hpp"

using namespace std;

int Rev_Ntk_t::WriteReal( const char * FileName ) const {
	ofstream ostr( FileName );
	WriteReal(ostr);
	ostr.close();
	return 1;
}

int Rev_Ntk_t::WriteReal( ostream& ostr ) const {
	const int nBit = width();
	ostr<<".version 2.0"<< endl;
	ostr<<".numvars "<<nBit<< endl;

	ostr<<".variables "; 
	for( int i=0; i<nBit; i++ ) ostr<<"x"<<i<<" ";
	ostr<< endl;

	ostr<<".inputs ";
	for( int i=0; i<nBit; i++ ) ostr<<"x"<<i<<" ";
	ostr<< endl;

	ostr<<".outputs ";
	for( int i=0; i<nBit; i++ ) ostr<<"y"<<i<<" ";
	ostr<< endl;

	ostr<<".constants ";
	for( int i=0; i<nBit; i++ ) ostr<<"-";
	ostr<< endl;

	ostr<<".garbage ";
	for( int i=0; i<nBit; i++ ) ostr<<"-";
	ostr<< endl;

	ostr<<".begin"<<endl;
	for(int i=0; i<_vLevel.size(); i++)
	{
		int flip = _vLevel[i]->getFlip();
		int icount = 0;
		Term_t Ctrls = _vLevel[i]->getCtrl();
		for(int j=0; j<Ctrls.ndata(); j++)
			if( Ctrls.val(j) )
				icount ++ ;
		ostr << "t" << icount + 1 <<" ";
		for(int j=0; j<Ctrls.ndata(); j++)
			if( Ctrls.val(j) )
				ostr<<"x"<< j <<" ";
		if( -1<flip )
			ostr<<"x"<< flip <<" "<<endl;
	}

	ostr<<".end"<< endl;
	return 1;
}

int verify(Rev_Ttb_t& ttb, const char * fNtk){
	Rev_Ntk_t Ntk(ttb.width());
	ifstream ifstr(fNtk);
	string line, word;

	while(getline(ifstr,line)){
		istringstream istr(line);
		int idx, ctrl;
		Rev_Gate_t Gate;
		if(!(istr>>ctrl)){
			cout << "missing control bit"<<endl;
			return 2;
		}

		Term_t Oper;
		Oper.resize(ttb.width());
		while( istr >> idx){
			if(ttb.width()  <= idx){
				cout << "extra ancilla detected"<<endl;
				return 2;
			}
			Oper.set(idx,1);
		}

		Gate.setCtrl(Oper);
		Gate.setFlip(ctrl);
		Ntk.push_back(Gate);
	}
	cout<<" Nkt: level= "<<Ntk.nLevel() <<" nCtrl= "<< Ntk.nCtrl() <<endl;
	cout << "QCost = " << Ntk.QCost() << endl;
	
	if( ! Ntk.Verify(ttb) ){
		cout<<" Verification: FAIL "<<endl;
		return 0;
	} else {
		cout<<" Verification: PASS "<<endl;
		return 1;
	}
	
}

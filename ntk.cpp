#include <fstream>
#include "ntk.hpp"

using namespace std;

int Rev_Ntk_t::WriteReal( const char * FileName ) const {
	ofstream ostr( FileName );
	WriteReal(ostr);
	ostr.close();
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
		Term_t Ctrls = _vLevel[i]->getCtrl();
		for(int j=0; j<Ctrls.ndata(); j++)
			if( Ctrls.val(j) )
				ostr<<"x"<< j <<" ";
		if( -1<flip )
			ostr<<"x"<< flip <<" "<<endl;
	}

	ostr<<".end"<< endl;
}

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

#include "ttb.hpp"

int Rev_Ttb_t::read( char * FileName ){
	ifstream istr( FileName );
	if( !istr.good() )
		return 0;

	string line, iWord, oWord;
	int nWidth;
	
	if( !getline(istr, line) )
		return 0;

	istringstream lstr(line);
	if( !(lstr >> nWidth) ){
		printf("Missing width indicator\n");
		return 0;
	}

	while( getline(istr, line) ){
		istringstream lstr(line);
		if( !(lstr >> iWord >> oWord ) ){
			printf("inconsistent I/O words. Current line: %s\n", line.c_str() );
			return 0;
		}
		push_back( iWord, oWord );
	}

	return 1;
}

void Rev_Ttb_t::print( std::ostream& ostr ) const {
	for(int i=0; i<_vEntry.size(); i++){
		_vEntry[i]->first .print( cout );
		_vEntry[i]->second.print( cout );
		cout << endl;
	}
}

void Rev_Ttb_t::dup( const Rev_Ttb_t * pTtb ){
	clear();
	for(int i=0; i<pTtb->_vEntry.size(); i++)
		push_back( pTtb->_vEntry[i] );
}

int Rev_TtbEquivalant( const Rev_Ttb_t& Ttb1, const Rev_Ttb_t& Ttb2 ){
	if( Ttb1.size() != Ttb2.size() )
		return 0;
	for( int i=0; i<Ttb1.size(); i++){
		if( Ttb1[i]->first != Ttb2[i]->first )
			return 0;
		if( Ttb1[i]->second!= Ttb2[i]->second)
			return 0;
	}
	return 1;
}

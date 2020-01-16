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

	for(int i=0; i<_vEntry.size(); i++){
		_vEntry[i]->first .print( cout );
		_vEntry[i]->second.print( cout );
		cout << endl;
	}

	return 1;
}

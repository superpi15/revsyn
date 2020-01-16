#include <iostream>
#include "ttb.hpp"
int main( int argc, char * argv[] ){
	Rev_Ttb_t ttb;
	if( !ttb.read(argv[1]) ){
		std::cout<<"Parsing Error"<<std::endl;
	}
}
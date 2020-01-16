#include <iostream>
#include <cstdlib>
#include "ttb.hpp"
#include "ntk.hpp"
#include "revsyn.hpp"

using namespace std;

int main( int argc, char * argv[] ){
	int fVerify = 1;
	int mode = 0;
	char * SpecName = NULL, * Output = NULL;
	if( argc <= 2 ){
		cout<<"./revsyn <SPEC_FILE> <MODE> [OUTPUT]"<<endl;
		return 1;
	}
	SpecName = argv[1];
	mode     = atoi(argv[2]);
	if( argc > 3 )
		Output  = argv[3];

	Rev_Ttb_t ttb;
	if( !ttb.read( SpecName ) ){
		std::cout<<"Parsing Error"<<std::endl;
		return 1;
	}
	cout<<" Spec: "<< SpecName <<" nEntry= "<< ttb.size() <<endl;
	
	if( ttb.size() < 16 )
		ttb.print( std::cout ); // print ttb 
	else
		cout << " Spec is too large. Skip printing "<<endl;
	
	Rev_Ntk_t * pNtk = NULL;
	if( 0 == mode )
		pNtk = Rev_GBD(ttb);
	else
		pNtk = Rev_qGBD(ttb);
	cout<<" Nkt: level= "<<pNtk->nLevel() <<" nCtrl= "<< pNtk->nCtrl() <<endl;

	if( pNtk->nLevel() < 40 )
		pNtk->print( std::cout ); // print ntk
	else
		cout<< " Result circuit is too large. Skip printing "<<endl;

	if( fVerify ){
		if( ! pNtk->Verify(ttb) )
			cout<<" Mismatch in implementation and spec. "<<endl;
		else
			cout<<" Ntk is correctly implemented the spec. "<<endl;
	}
	
	return 0;
}
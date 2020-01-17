#include <iostream>
#include <cstdlib>
#include "ttb.hpp"
#include "ntk.hpp"
#include "revsyn.hpp"

using namespace std;

int main( int argc, char * argv[] ){
	int fVerify = 1;
	int fDemo   = 0;

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


	///////////////////////
	// load specification
	///////////////////////
	Rev_Ttb_t ttb;
	if( !ttb.read( SpecName ) ){
		std::cout<<"Parsing Error"<<std::endl;
		return 1;
	}
	cout<<" Spec: "<< SpecName <<" nEntry= "<< ttb.size() <<endl;
	
	if( fDemo ){
		ttb.print( std::cout ); 
		cout<<endl;
	}
	

	///////////////////////
	// synthesize the circuit 
	///////////////////////
	Rev_Ntk_t * pNtk = NULL;
	if( 0 == mode )
		pNtk = Rev_GBD(ttb);
	else
		pNtk = Rev_qGBD(ttb);
	cout<<" Nkt: level= "<<pNtk->nLevel() <<" nCtrl= "<< pNtk->nCtrl() <<endl;

	if( fDemo ){
		pNtk->print( std::cout ); // print ntk
		cout<<endl;
	}


	///////////////////////
	// verification
	///////////////////////
	if( fVerify ){
		if( ! pNtk->Verify(ttb) )
			cout<<" Verification: FAIL "<<endl;
		else
			cout<<" Verification: PASS "<<endl;
	}


	///////////////////////
	// write output to file
	///////////////////////
	if( Output ){
		pNtk->WriteReal( Output );
		cout<<" Write result to \'"<< Output <<"\'"<< endl;
	}
	
	return 0;
}
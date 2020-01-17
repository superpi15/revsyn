#include <iostream>
#include <cstdlib>
#include "ttb.hpp"
#include "ntk.hpp"
#include "revsyn.hpp"

using namespace std;

void demo_bit();

int main( int argc, char * argv[] ){
	//demo_bit(); return 0;           // see demostration of usage of bitwise operation

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
		cout<<"Parsing Error"<<endl;
		return 1;
	}
	cout<<" Spec: "<< SpecName <<" nEntry= "<< ttb.size() <<endl;
	
	if( fDemo ){
		ttb.print( cout ); 
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
		pNtk->print( cout ); // print ntk
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
	
	delete pNtk;
	return 0;
}



void demo_bit(){
	Term_t word1, word2, word3;
	//const char * str1 = "01011010010110101001000001100111011010100100000110011101";\
	const char * str2 = "11010010110101001010101000101101010010000000011001101110";
	cout<< "Example on initialization "<<endl;
	const char * str1 = "00000000000000000000000000000000010000001";\
	const char * str2 = "10000000000000000000000000000000010000000";
	word1.setWord( str1 );
	word2.setWord( str2 );
	cout<<"str1 = "<< str1 <<endl;
	cout<<"str2 = "<< str2 <<endl;
	cout<<"word1= "<< word1 <<endl;
	cout<<"word2= "<< word2 <<endl;
	cout<< "Example on bit relation and operation "<<endl;
	cout<<" relation: word1 < word2 ? " << (word1 < word2? "Yes":"No") <<endl;
	cout<<" relation: word1 > word2 ? " << (word1 > word2? "Yes":"No") <<endl;
	cout<<" relation: word1 == word1 ? " << (word1 == word1? "Yes":"No") <<endl;
	cout<<" relation: word1 != word1 ? " << (word1 != word1? "Yes":"No") <<endl;
	cout<<" operation: word1 & word2 = " << (word1 & word2) <<endl;
	cout<<" operation: word1 | word2 = " << (word1 | word2) <<endl;
	cout<<" operation: word1 ^ word2 = " << (word1 ^ word2) <<endl;
	word3 = word1 ^ word2;
	cout<<" haming dist between word1 and word2 is " << word3.weight() <<" = weight of Boolean difference"<<endl;
	cout<<" Example of flipping "<< endl;
	cout<< word1 <<" total length = "<< word1.ndata() <<endl;
	word1.flip(31);
	cout<< word1 <<" flip "<< 31 <<endl;
	word1.flip(0);
	cout<< word1 <<" flip "<< 0 <<endl;
	word1.flip(32);
	cout<< word1 <<" flip "<< 32 <<endl;
	word1.flip(40);
	cout<< word1 <<" flip "<< 40 <<endl;
}
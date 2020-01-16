#include <iostream>
#include "ttb.hpp"
#include "ntk.hpp"
using namespace std;
int main( int argc, char * argv[] ){
//	vLembit_t<1> word1, word2, word3;
//	std::cout<< "Example on bit operation "<<std::endl;
//	//const char * str1 = "01011010010110101001000001100111011010100100000110011101";\
//	const char * str2 = "11010010110101001010101000101101010010000000011001101110";
//	const char * str1 = "00000000000000000000000000000000000000001";\
//	const char * str2 = "10000000000000000000000000000000000000000";
//	word1.setWord( str1 );
//	word2.setWord( str2 );
//	std::cout<<"str1 = "<< str1 <<std::endl;
//	std::cout<<"str2 = "<< str2 <<std::endl;
//	std::cout<<"word1= "; word1.print(std::cout); std::cout<<std::endl;
//	std::cout<<"word2= "; word2.print(std::cout); std::cout<<std::endl;
//	word3 = word1 ^ word2;
//	std::cout<<"word3= "; word3.print(std::cout); std::cout<<" = word1 ^ word2 (Boolean difference)"<<std::endl;
//	std::cout<<" haming dist between word1 & word2 = " << (word1 ^ word2).weight() <<" = weight of Boolean difference"<<std::endl;
//	std::cout<<" relation: word1 < word2 ? " << (word1 < word2? "Yes":"No") <<std::endl;
//	std::cout<<" relation: word1 > word2 ? " << (word1 > word2? "Yes":"No") <<std::endl;
//	std::cout<<" relation: word1 == word1 ? " << (word1 == word1? "Yes":"No") <<std::endl;
//	word1.print( std::cout ); std::cout<<" total length = "<< word1.ndata() <<std::endl;
//	word1.flip(31);
//	word1.print( std::cout ); std::cout<<" flip "<< 31 <<std::endl;
//	word1.flip(0);
//	word1.print( std::cout ); std::cout<<" flip "<< 0 <<std::endl;
//	word1.flip(32);
//	word1.print( std::cout ); std::cout<<" flip "<< 32 <<std::endl;
//	word1.flip(40);
//	word1.print( std::cout ); std::cout<<" flip "<< 40 <<std::endl;
//	return 0;
	Rev_Ttb_t ttb;
	if( !ttb.read(argv[1]) ){
		std::cout<<"Parsing Error"<<std::endl;
		return 1;
	}
	cout<<" Spec: "<< argv[1] <<endl;
	//ttb.print( std::cout );
	Rev_Ntk_t * pNtk = Rev_Gbd(ttb);
	cout<<" Nkt: level="<<pNtk->nLevel() <<endl;
	if( ! pNtk->Verify(ttb) )
		cout<<" mismatch in implementation and spec. "<<endl;
	else
		cout<<" Ntk is correctly implemented the spec. "<<endl;
	//pNtk->print( std::cout );
	return 0;
}
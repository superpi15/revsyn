#include <iostream>
#include "ttb.hpp"
#include "ntk.hpp"

int main( int argc, char * argv[] ){
	Rev_Ttb_t ttb;
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
//	return 0;
	if( !ttb.read(argv[1]) ){
		std::cout<<"Parsing Error"<<std::endl;
	}

	Rev_Gbd(ttb);
}
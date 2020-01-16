#ifndef ttb_hpp
#define ttb_hpp

#include <iostream>
#include <vector>
#include <utility>
#include <string>

#include "bitop.hpp"

typedef vLembit_t<1> Term_t;
typedef std::pair<Term_t , Term_t > BitPair_t;

class Rev_Ttb_t {
public:
	typedef std::vector<BitPair_t * > vEntry_t;
private:
	vEntry_t _vEntry;
public:
	Rev_Ttb_t(){}
	Rev_Ttb_t( const Rev_Ttb_t& Ttb ){ dup(&Ttb); }
	~Rev_Ttb_t(){ clear(); }
	BitPair_t*& operator[]( size_t idx ) { return _vEntry[idx]; }
	const BitPair_t* operator[]( size_t idx ) const { return _vEntry[idx]; }
	int size() const { return _vEntry.size(); }
	void clear(){
		for(int i=0; i<_vEntry.size(); i++)
			delete _vEntry[i];
		_vEntry.clear();
	}
	// add the pEntry to this ttb
	void push_back( BitPair_t * pEntry ){
		_vEntry.push_back( new BitPair_t );
		BitPair_t& Entry = *_vEntry.back();
		Entry.first. setWord( pEntry->first  );
		Entry.second.setWord( pEntry->second );
	}
	void push_back( const std::string& iWord, const std::string& oWord ){
		push_back( iWord.c_str(), oWord.c_str() );
	}
	void push_back( const char * iWord, const char * oWord ){
		_vEntry.push_back( new BitPair_t );
		BitPair_t& Entry = *_vEntry.back();
		Entry.first .setWord( iWord );
		Entry.second.setWord( oWord );
	}

	// read ttb from file
	int read( char * FileName );

	// dump ttb 
	void print( std::ostream& ) const ;

	// duplicate ttb from pTtb 
	void dup( const Rev_Ttb_t * pTtb );
};

extern int Rev_TtbEquivalant( const Rev_Ttb_t& Ttb1, const Rev_Ttb_t& Ttb2 );

#endif
#ifndef bitop_hpp
#define bitop_hpp

#include "lembit.hpp"
#include <cassert>

// less than 
template<int WIDTH>
static inline bool operator<( const vLembit_t<WIDTH>& b1, const vLembit_t<WIDTH>& b2 ){
	if( b1.ndata() != b2.ndata() )
		return b1.ndata() < b2.ndata();
	return b1.vec() < b2.vec();
}

// larger than 
template<int WIDTH>
static inline bool operator>( const vLembit_t<WIDTH>& b1, const vLembit_t<WIDTH>& b2 ){
	if( b1.ndata() != b2.ndata() )
		return b1.ndata() > b2.ndata();
	return b1.vec() > b2.vec();
}

// equal to
template<int WIDTH>
static inline bool operator==( const vLembit_t<WIDTH>& b1, const vLembit_t<WIDTH>& b2 ){
	if( b1.ndata() != b2.ndata() )
		return false;
	return b1.vec() == b2.vec();
}

// equal to
template<int WIDTH>
static inline bool operator!=( const vLembit_t<WIDTH>& b1, const vLembit_t<WIDTH>& b2 ){
	return !(b1==b2);
}

// boolean difference
template<int WIDTH>
static inline vLembit_t<WIDTH> operator^( const vLembit_t<WIDTH>& b1, const vLembit_t<WIDTH>& b2 ){
	assert( b1.ndata() == b2.ndata() );
	vLembit_t<WIDTH> ret;
	ret.resize( b1.ndata() );
	for(int i=0; i<b2.vec().size(); i++)
		ret.vec()[i] = b1.vec()[i] ^ b2.vec()[i];
	
	return ret;
}

// bitwise-and
template<int WIDTH>
static inline vLembit_t<WIDTH> operator&( const vLembit_t<WIDTH>& b1, const vLembit_t<WIDTH>& b2 ){
	assert( b1.ndata() == b2.ndata() );
	vLembit_t<WIDTH> ret;
	ret.resize( b1.ndata() );
	for(int i=0; i<b2.vec().size(); i++)
		ret.vec()[i] = b1.vec()[i] & b2.vec()[i];
	
	return ret;
}

// bitwise-or
template<int WIDTH>
static inline vLembit_t<WIDTH> operator|( const vLembit_t<WIDTH>& b1, const vLembit_t<WIDTH>& b2 ){
	assert( b1.ndata() == b2.ndata() );
	vLembit_t<WIDTH> ret;
	ret.resize( b1.ndata() );
	for(int i=0; i<b2.vec().size(); i++)
		ret.vec()[i] = b1.vec()[i] | b2.vec()[i];
	
	return ret;
}

// bitwise-complement
template<int WIDTH>
static inline vLembit_t<WIDTH> operator~( const vLembit_t<WIDTH>& b1 ){
	vLembit_t<WIDTH> ret;
	ret.resize( b1.ndata() );
	for(int i=0; i<b1.vec().size(); i++)
		ret.vec()[i] = ~b1.vec()[i];
	
	return ret;
}

#endif
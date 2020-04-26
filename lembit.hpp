#ifndef lembit_hpp
#define lembit_hpp

#include <iostream>
#include <algorithm>
#include <vector>
#include <utility>
#include <string>
#include <cstring>
#include <stdio.h>

#ifdef _MSC_VER
#  include <intrin.h>
#  define __builtin_popcount __popcnt
#endif

template<int WIDTH>
class vLembit_t: private std::vector<unsigned int> {
	unsigned int weight( unsigned int val ) const { return __builtin_popcount(val); }
public:
	vLembit_t(){
		unsigned int remain = sizeof(unsigned int)*8 - WIDTH;
		MASK = FULL() >> remain;
		_ndata = 0;
	}
	unsigned int unit_volume()          const { return sizeof(unsigned int)*8/WIDTH; }
	const unsigned int size ()          const { return vec().size() * unit_volume(); }
	unsigned int ndata()                const { return _ndata; }
	void clear(){ vec().clear(); }
	void resize( unsigned int nsize ){
		_ndata = nsize;
		int nunit = _unit(nsize) + ( _rank(nsize) ?1:0);
		vec().resize( nunit );
		memset( data(), 0, sizeof(int)*nunit );
	}
	void set( unsigned int index, unsigned int val ){
		vec()[ _index(index) ] &= ~_mask(index);
		vec()[ _index(index) ] |= ( val<< _offset(index) );
	}
	void flip( unsigned int index ){
		vec()[ _index(index) ] ^= ( 1<< _offset(index) );
	}
	const unsigned int val( unsigned int index ) const {
		unsigned int ret = ( vec()[_index(index)] & _mask(index) )>> _offset(index);
		return ret;
	}
	int weight() const {
		int k = 0, wt = 0;
		for(int i=0; i<vec().size(); i++){
			wt += weight( vec()[i] );
		}
		return wt;
	}
	int leading1() const {
		int idx = -1;
		for(int i=ndata()-1; i>=0; i--)
			if(val(i)) return i;
		return -1;
	}
	void setWord( const char * Word ){
		const size_t len = strlen(Word);
		clear();
		resize(len);
		for(int i=0; i<len; i++)
			set(i, Word[len-1-i]=='1'? 1: 0);
	}
	void setWord( const vLembit_t<1>& vBitSrc ){
		const size_t len = vBitSrc.ndata();
		clear();
		resize(len);
		for(int i=0; i<len; i++)
			set(i, vBitSrc.val(i) );
	}
	const std::vector<unsigned int>& vec() const { return dynamic_cast<const std::vector<unsigned int>&>(*this);}
	std::vector<unsigned int>& vec(){ return dynamic_cast<std::vector<unsigned int>&>(*this);}
	void print(std::ostream& ostr) const {
		int k = 0;
		std::string line;
		for(int i=vec().size()-1; i>=0; -- i){
			for(int j=0; j<sizeof(int)*8 && k<_ndata; ++j, ++k )
				line.push_back(((vec()[i])&(1<<j))? '1': '0');
			//ostr<<" ";
		}
		std::reverse(line.begin(),line.end());
		ostr<<line;
	}
private:
	int _ndata;
	unsigned int MASK;
	unsigned int _unit       (unsigned int index) const { return index/unit_volume(); }
	unsigned int _index      (unsigned int index) const { return vec().size()-1-_unit(index); }
	unsigned int _rank       (unsigned int index) const { return index%unit_volume(); }
	unsigned int _offset     (unsigned int index) const { return _rank(index)*WIDTH; }
	unsigned int _mask       (unsigned int index) const { return MASK<<_offset(index);}
	unsigned int FULL       () const { return ~0;}
};

template<int WIDTH>
static inline std::ostream& operator<<( std::ostream& ostr, const vLembit_t<WIDTH>& vBit ){
	vBit.print( ostr );
	return ostr;
}

#endif
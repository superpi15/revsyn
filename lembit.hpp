#ifndef lembit_hpp
#define lembit_hpp

#include <iostream>
#include <algorithm>
#include <vector>
#include <utility>
#include <string>
#include <cstring>

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
		vec()[ _unit(index) ] &= ~_mask(index);
		vec()[ _unit(index) ] |= ( val<< _offset(index) );
	}
	const unsigned int val( unsigned int index ) const {
		unsigned int ret = ( vec()[_unit(index)] & _mask(index) )>> _offset(index);
		return ret;
	}
	int weight() const {
		int k = 0, wt = 0;
		for(int i=0; i<vec().size(); i++){
			wt += weight( vec()[i] );
		}
		return wt;
	}
	void setWord( const char * Word ){
		const size_t len = strlen(Word);
		clear();
		resize(len);
		for(int i=0; i<len; i++)
			set(i, Word[i]=='1'? 1: 0);
	}
	void setWord( const vLembit_t<1>& vBitSrc ){
		const size_t len = vBitSrc.ndata();
		clear();
		resize(len);
		for(int i=0; i<len; i++)
			set(i, vBitSrc.val(i) );
	}
	void  fill( unsigned int from, unsigned int to, bool fOne = true ) {
		int funit = _unit(from);
		int tunit = _unit(  to);
		if( funit == tunit ){
			unsigned int mask = FULL();
			mask <<= sizeof(int)*8 - _offset(to);
			mask >>= sizeof(int)*8 - _offset(to);
			mask >>= _offset(from);
			mask <<= _offset(from);
			fill_unit(funit, mask, fOne );
			return;
		}

		{ // fill head 
			unsigned int mask = FULL();
			mask >>= _offset(from);
			mask <<= _offset(from);
			fill_unit(funit, mask, fOne );
		}
		
		size_t dist = tunit - funit - 1;
		if( dist > 0 )
			memset( &vec()[funit+1], fOne? FULL(): 0, dist * sizeof(int) );
		{ // fill tail
			if( 0 == _offset(to) )
				return;
			unsigned int mask = FULL();
			mask <<= sizeof(int)*8 - _offset(to);
			mask >>= sizeof(int)*8 - _offset(to);
			fill_unit(tunit, mask, fOne );
		}
	}
	const std::vector<unsigned int>& vec() const { return dynamic_cast<const std::vector<unsigned int>&>(*this);}
	std::vector<unsigned int>& vec(){ return dynamic_cast<std::vector<unsigned int>&>(*this);}
	void print(std::ostream& ostr) const {
		int k = 0;
		for(int i=0; i<vec().size(); i++){
			for(int j=0; j<sizeof(int)*8 && k<_ndata; ++j, ++k )
				ostr<<(((vec()[i])&(1<<j))? '1': '0');
			//ostr<<" ";
		}
	}
private:
	int _ndata;
	unsigned int MASK;
	unsigned int _unit       (unsigned int index) const { return index/unit_volume(); }
	unsigned int _rank       (unsigned int index) const { return index%unit_volume(); }
	unsigned int _offset     (unsigned int index) const { return _rank(index)*WIDTH; }
	unsigned int _mask       (unsigned int index) const { return MASK<<_offset(index);}
	unsigned int FULL       () const { return ~0;}
	void  fill_unit( unsigned int index_unit, unsigned int mask, bool fOne ){
		if( fOne )
			vec()[index_unit] |= mask;
		else
			vec()[index_unit] = vec()[index_unit] & ~mask;
	}
};

#endif
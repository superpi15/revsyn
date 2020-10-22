#ifndef ntk_hpp
#define ntk_hpp

#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <cstring>
#include <algorithm>
#include "ttb.hpp"

class Rev_Gate_t: vLembit_t<2> {
public:
	typedef enum {
		None = 0,
		Ctrl , 
		Flip
	} Oper_t;
	int width() const { return ndata(); }
	int nCtrl() const {
		int ret = 0;
		for( int i=0; i<ndata(); i++ )
			if( Ctrl == val(i) )
				ret ++ ;
		return ret;
	}
	void setCtrl( const Term_t& term ){
		if( ndata() < term.ndata() )
			resize( term.ndata() );
		for( int i=0; i<ndata(); i++ )
			if( term.val(i) )
				set(i, Ctrl);
	}
	void setFlip( unsigned int index ){
		set(index, Flip);
	}
	Term_t getCtrl(){
		Term_t ret;
		ret.resize(ndata());
		for(int i=0; i<ndata(); i++)
			if( val(i)==Ctrl )
				ret.set(i, 1);
		return ret;
	}
	int getFlip(){
		for(int i=0; i<ndata(); i++)
			if( val(i)==Flip )
				return i;
		return -1;
	}
	char symbol( int idx ) const {
		switch( val(idx) ){
			case Ctrl: return '+';
			case Flip: return 'X';
			case None: return '-';
		}
		return '?';
	}
	void print( std::ostream& ostr ) const {
		for(int i=ndata()-1; i>=0; i--)
			ostr << symbol(i);
	}
};

class Rev_Ntk_t {
	typedef std::vector<Rev_Gate_t * > vLevel_t;
	vLevel_t _vLevel;
	int _width;
public:
	Rev_Ntk_t( int nwidth=0 ):_width(nwidth){}
	~Rev_Ntk_t(){
		clear();
	}
	void reset(int nwidth){
		clear();
		_width = nwidth;
	}
	int nLevel() const { return _vLevel.size(); }
	int width () const { return _width; }
	int nCtrl() const {
		int ret = 0;
		for(int i=0; i<_vLevel.size(); i++)
			ret += _vLevel[i]->nCtrl();
		return ret;
	}
	void clear(){
		for(int i=0; i<_vLevel.size(); i++)
			delete _vLevel[i];
		_vLevel.clear();
	}
	void reverse(){
		std::reverse( _vLevel.begin(), _vLevel.end() );
	}
	void push_back( const Rev_Gate_t& Gate ){
		_vLevel.push_back( new Rev_Gate_t(Gate) );
	}
	void Apply( Term_t& term ) const {
		assert( width() == term.ndata() );
		int Flip;
		Term_t Ctrls;
		for(int i=0; i<_vLevel.size(); i++){
			Flip = _vLevel[i]->getFlip();
			if( -1 == Flip )
				continue;
			Ctrls = _vLevel[i]->getCtrl();
			if( Ctrls != (Ctrls & term) )
				continue;
			term.flip(Flip);
		}
	}

	void Apply( Rev_Ttb_t& Ttb ) const {
		assert( width() == Ttb.width() );
		int Flip;
		Term_t Ctrls;
		for(int i=0; i<_vLevel.size(); i++){
			Flip = _vLevel[i]->getFlip();
			if( -1 == Flip )
				continue;
			Ctrls = _vLevel[i]->getCtrl();
			for(int j=0; j<Ttb.size(); j++){
				Term_t& iterm = Ttb[j]->first;
				if( Ctrls != (Ctrls & iterm) )
					continue;
				iterm.flip(Flip);
			}
		}
	}

	void Append( const Rev_Ntk_t& Ntk ){
		for(int i=0; i<Ntk._vLevel.size(); i++ )
			push_back( *Ntk._vLevel[i] );
	}

	void print( std::ostream& ostr ) const {
		if( _vLevel.empty() )
			return ;
		const int width = _vLevel.front()->width();
		for(int i=width-1; i>=0; i--){
			for(int j=0; j<_vLevel.size(); j++){
				ostr<<_vLevel[j]->symbol(i);
			}
			ostr<<std::endl;
		}
	}

	int Verify( const Rev_Ttb_t& ttb ) const {
		Rev_Ttb_t ttb2(ttb);
		Apply(ttb2);
		//ttb2.print(std::cout);
		for(int i=0; i<ttb2.size(); i++)
			if( ttb2[i]->first != ttb2[i]->second )
				return 0;
		return 1;
	}

	int QCost() const {
		int ret = 0;
		/**
		const int TableSz = 11;
		const int CostTable[] = {0,1,1,5,13,29,61,125,253,509,1021};
		for(int i=0; i<_vLevel.size(); i++){
			int nOper = _vLevel[i]->nCtrl() + ( -1 < _vLevel[i]->getFlip() );
			ret += nOper < TableSz? CostTable[ nOper ]: (1<<nOper) - 3;
		}
		/**/
		for(int i=0; i<_vLevel.size(); i++){
			int nOper = _vLevel[i]->nCtrl();
			switch( nOper ){
				case 0:; case 1: break;
				case 2: ret += 7; break;
				default:
					if( width() - nOper - 1 >= (nOper-1)/2 )
						ret += 8 * (nOper - 1);
					else
						ret += 16 * (nOper - 1);
				;
			}
		}
		return ret;
	}

	int WriteReal( const char * FileName ) const ;
	int WriteReal( std::ostream& ostr ) const ;
};

#endif
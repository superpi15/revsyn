#ifndef revsynUtil_hpp
#define revsynUtil_hpp
#include <cassert>

class Syn_Obj_t {
public:
	Syn_Obj_t( BitPair_t * npB=NULL ): pB(npB), _small(-1){}
	BitPair_t * pB;
	const Term_t& small() const { assert(_small>=0); return _small==0? pB->first: pB->second; }
	const Term_t& large() const { assert(_small>=0); return _small==1? pB->first: pB->second; }
	const Term_t& first () const { return pB->first ; }
	const Term_t& second() const { return pB->second; }
	bool isForward() const { return 0 == _small; }
	void setLegal( int val ) { assert( 0 <= val && val < 2); _small = val; }
	bool isForwardLegal() const { return 0 == _small; }
	Term_t& small() { assert(_small>=0); return _small==0? pB->first: pB->second; }
	Term_t& large() { assert(_small>=0); return _small==1? pB->first: pB->second; }
	Term_t& first (){ return pB->first ; }
	Term_t& second(){ return pB->second; }
	Term_t& another(const Term_t& cur) const { return cur==pB->first? pB->second: pB->first; }
	int hamdist() const { return (pB->first ^ pB->second).weight(); }
	int maxWeight() const { return std::max(pB->first.weight(), pB->second.weight());}
	void update_small(int force=-1){
		if(0 <= force){
			_small = force;
			return; }
		if( pB->first < pB->second )
			_small = 0;
		else
			_small = 1;
	}
	struct Cmptor_t	{
		bool operator()( const Syn_Obj_t& o1, const Syn_Obj_t& o2 ) const {
			return o1.small() < o2.small();
		}
	};
private:
	int _small;
};

typedef std::vector<Syn_Obj_t> vSynObj_t;

class Var_Inf_t {
public:
	Var_Inf_t():nCtrAbl(0),VarId(0){}
	int VarId;
	int nCtrAbl; // controllability
	struct Cmptor_t {
		bool operator()( const Var_Inf_t& v1, const Var_Inf_t& v2 ) const {
			return v1.nCtrAbl != v2.nCtrAbl? v1.nCtrAbl < v2.nCtrAbl: v1.VarId > v2.VarId;
			//return v1.VarId < v2.VarId;
		}
		bool operator()( const Var_Inf_t * pv1, const Var_Inf_t* pv2 ) const {
			return (*this)(*pv1,*pv2); }
	};
};

typedef std::vector<Var_Inf_t> vVarInf_t;
typedef std::vector<Var_Inf_t*> vVarInfPtr_t;


#include <set>
class TermObj_t {
public:
	TermObj_t( const Term_t * npTerm=NULL, const Syn_Obj_t * npSrc=NULL ):pTerm(npTerm), pSrc(npSrc){}
	const Term_t * pTerm;
	const Syn_Obj_t * pSrc;
	bool isLegal() const { return NULL != pSrc; }
	const Syn_Obj_t * src() const { return pSrc; }
	struct Cmptor_t {
		bool operator()( const TermObj_t& Obj1, const TermObj_t& Obj2 ) const {
			return Obj1.pSrc!=Obj2.pSrc? Obj1.pSrc < Obj2.pSrc: Obj1.pTerm < Obj2.pTerm;
			//return (Obj1.isLegal()!=Obj2.isLegal()) ? Obj1.isLegal() > Obj2.isLegal(): Obj1.pTerm < Obj2.pTerm; 
		}
	};
	static const TermObj_t null;
};

inline bool operator==(const TermObj_t& a, const TermObj_t& b){
	return a.pTerm == b.pTerm && a.pSrc == b.pSrc;
}
inline bool operator!=(const TermObj_t& a, const TermObj_t& b){
	return !(a==b);
}
typedef std::set<TermObj_t, TermObj_t::Cmptor_t> TermObjSet_t;
#endif
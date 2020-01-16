#ifndef revsynUtil_hpp
#define revsynUtil_hpp

class Syn_Obj_t {
public:
	Syn_Obj_t( BitPair_t * npB=NULL ): pB(npB), _small(-1){}
	BitPair_t * pB;
	const Term_t& small() const { assert(_small>=0); return _small==0? pB->first: pB->second; }
	const Term_t& large() const { assert(_small>=0); return _small==1? pB->first: pB->second; }
	const Term_t& first () const { return pB->first ; }
	const Term_t& second() const { return pB->second; }
	bool isForward() const { return _small==0; }
	Term_t& small() { assert(_small>=0); return _small==0? pB->first: pB->second; }
	Term_t& large() { assert(_small>=0); return _small==1? pB->first: pB->second; }
	Term_t& first (){ return pB->first ; }
	Term_t& second(){ return pB->second; }
	int hamdist() const { return (pB->first ^ pB->second).weight(); }
	void update_small(){
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
			return v1.nCtrAbl < v2.nCtrAbl;
		}
		bool operator()( const Var_Inf_t * pv1, const Var_Inf_t* pv2 ) const {
			return pv1->nCtrAbl < pv2->nCtrAbl;
		}
	};
};

typedef std::vector<Var_Inf_t> vVarInf_t;
typedef std::vector<Var_Inf_t*> vVarInfPtr_t;
#endif
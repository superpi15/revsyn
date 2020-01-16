#include "ttb.hpp"
#include "ntk.hpp"

using namespace std;

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

static void Rev_EntrySimpleSyn( Syn_Obj_t * pObj, Rev_Ntk_t& Ntk ){
	const Term_t& small = pObj->small();
	const Term_t& large = pObj->large();
	Term_t Prog = large;   // progress
	Term_t Diff = small ^ large;
	for(int i=0; i<Diff.ndata(); i++){
		if( !Diff.val(i) )
			continue;
		Term_t Oper = Prog;
		Oper.set(i,0);
		
		Rev_Gate_t Gate;
		Gate.setCtrl( Oper );
		Gate.setFlip( i );
		Ntk.push_back( Gate );
		Prog.flip(i);
	}
}

Rev_Ntk_t * Rev_Gbd( const Rev_Ttb_t& ttb ){
	Rev_Ttb_t ttb2(ttb); // create a truth table same as ttb
	Rev_Ntk_t * pNtk;
	pNtk = new Rev_Ntk_t;
	// prepare array for sorting 
	vSynObj_t vSynObj( ttb2.size() );
	for(int i=0; i<vSynObj.size(); i++){
		vSynObj[i] = Syn_Obj_t( ttb2[i] );
		vSynObj[i].update_small();
	}
	
	//pick min-minterm, tie-break by hamdist
	Rev_Ntk_t NtkFront, NtkBack;
	for(int i=0; i<vSynObj.size(); i++){
		std::sort( vSynObj.begin()+i, vSynObj.end(), Syn_Obj_t::Cmptor_t() );
		// check tie-breack condition
		if( i+1<vSynObj.size() )
			if( vSynObj[i].small()==vSynObj[i+1].small() ){
				if( vSynObj[i].hamdist()>vSynObj[i+1].hamdist() ) // select smaller hamming distance
					swap( vSynObj[i], vSynObj[i+1] );
			}
		bool fForward = vSynObj[i].isForward();
		Rev_Ntk_t SubNtk;
		Rev_EntrySimpleSyn( &vSynObj[i], SubNtk );
		if( 0==SubNtk.nLevel() )
			continue;

		for(int j=i; j<vSynObj.size(); j++){
			SubNtk.Apply( fForward? vSynObj[j].second(): vSynObj[j].first () );
			vSynObj[j].update_small();
		}


		if( fForward )
			NtkBack .Append( SubNtk );
		else
			NtkFront.Append( SubNtk );
	}
	pNtk->Append( NtkFront );
	NtkBack.reverse();
	pNtk->Append( NtkBack  );
	return pNtk;
}
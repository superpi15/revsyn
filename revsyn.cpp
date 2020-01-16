#include "ttb.hpp"
#include "ntk.hpp"

using namespace std;

class Syn_Obj_t {
public:
	Syn_Obj_t( BitPair_t * npB=NULL ): pB(npB), _small(-1){}
	BitPair_t * pB;
	const vLembit_t<1>& small() const { assert(_small>=0); return _small==0? pB->first: pB->second; }
	const vLembit_t<1>& large() const { assert(_small>=0); return _small==1? pB->first: pB->second; }
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

Rev_Ntk_t * Rev_Gbd( const Rev_Ttb_t& ttb ){
	Rev_Ttb_t ttb2(ttb); // create a truth table same as ttb

	vSynObj_t vSynObj( ttb2.size() );
	for(int i=0; i<vSynObj.size(); i++){
		vSynObj[i] = Syn_Obj_t(ttb2[i]);
		vSynObj[i].update_small();
	}
	std::sort( vSynObj.begin(), vSynObj.end(), Syn_Obj_t::Cmptor_t() );

	for(int i=0; i<vSynObj.size(); i++){
		vSynObj[i].small().print( cout ); cout<<" ";
		vSynObj[i].large().print( cout ); cout<<endl;
	}
	//find min-minterm, tie-break by hamdist
}
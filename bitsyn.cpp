#include "ttb.hpp"
#include "ntk.hpp"
#include "revsyn.hpp"
#include "revsynUtil.hpp"
#include <vector>

template<typename A>
class Lin_Obj_t {
public:
	Lin_Obj_t(){
		prev = NULL;
		next = NULL;
	}
	Lin_Obj_t * prev, * next;
	A data;
};

template<typename A>
class Lin_Lst_t {
public:
	Lin_Obj_t<A> root;
	;
};

Rev_Ntk_t * Rev_BitSyn(const Rev_Ttb_t& ttb){
	std::vector< Lin_Lst_t<TermObjSet_t> > v;
}
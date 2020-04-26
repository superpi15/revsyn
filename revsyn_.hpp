#ifndef revsyn__hpp
#define revsyn__hpp

#include <vector>

#include "ttb.hpp"
#include "ntk.hpp"
#include "revsynUtil.hpp"
#include "minisat/simp/SimpSolver.h"

class Rev_Syn_t {
public:
	Rev_Syn_t();
	~Rev_Syn_t();
	int ctrlMode;  // 0=all, 1=zero-controllability, 2=trace dominance
	int legalPath; // 0=lexicalgraphic, 1=Boolean lattice
	int costMode;  // 0=hamming distance, 1=exact cost
	vSynObj_t vSynObj;
	Rev_Ntk_t * perform(const Rev_Ttb_t& ttb);
private:
	Minisat::SimpSolver * pSol;

	int _nEntry;               // count synthesized entry in current synthesize round
	Rev_Ntk_t _SubNtk;          // the last synthesized sub-circuit
	Rev_Ttb_t ttb2;            // operation field 
	vVarInf_t vVarInf;         // for 1 == ctrlMode
	vVarInfPtr_t vVarInfPtr;   // for 1 == ctrlMode
	std::vector< TermObjSet_t > mW2Term; // for 1 == legalPath
	int selectEntry(int idx);
	int entryCost(int idx, int fixedTop);
	void synthesizeEntry(int idx, Rev_Ntk_t& Ntk);
	void dataInitialize(const Rev_Ttb_t& ttb);
	void dataUpdate(int idx);

	int OneGateAssumeCost(Rev_Ntk_t& tmpGate, TermObj_t * pTermObj, vSynObj_t::iterator fixedBegin, vSynObj_t::iterator fixedEnd, vVarInfPtr_t& vVarInfPtr);
};

#endif
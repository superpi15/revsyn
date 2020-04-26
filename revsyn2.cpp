#include "revsyn_.hpp"
#include <time.h>

using namespace std;
using namespace Minisat;

Rev_Syn_t::Rev_Syn_t(){
	ctrlMode = 0;
	legalPath= 0;
	_nEntry  = 0;
	pSol = NULL;
	vSynObj.clear();
}

Rev_Syn_t::~Rev_Syn_t(){
	pSol = NULL;
}

// test whether space contains prime 
static bool Contain( const Term_t& prime, const Term_t& space ){
	return ( prime == ( prime & space ) ) && ( prime != space );
}

static void FillOneFirstOrder( const Term_t& Diff, const Term_t& large, vector<int>& vOrder ){
	for(int i=0; i<Diff.ndata(); i++){
		int var = i;
		if( !Diff.val(var) )
			continue;
		if( 1==large.val(var) )
			continue;
		vOrder.push_back(var);
	}

	for(int i=0; i<Diff.ndata(); i++){
		int var = i;
		if( !Diff.val(var) )
			continue;
		if( 0==large.val(var) )
			continue;
		vOrder.push_back(var);
	}
}

static void Rev_EntrySimpleSyn( Syn_Obj_t * pObj, Rev_Ntk_t& Ntk ){
	const Term_t& small = pObj->small();
	const Term_t& large = pObj->large();
	Term_t Prog = large;   // progress
	Term_t Diff = small ^ large;
	vector<int> vOrder;

	FillOneFirstOrder( Diff, large, vOrder);

	for(int i=0; i<vOrder.size(); i++){
		int idx = vOrder[i];
		Term_t Oper = Prog;
		Oper.set(idx,0);
		
		Rev_Gate_t Gate;
		Gate.setCtrl( Oper );
		Gate.setFlip( idx );
		Ntk.push_back( Gate );
		Prog.flip( idx );

		//cout <<" working on "<< small <<" vs "<< large <<"  Oper "<< Oper <<" prog="<< Prog << endl;
	}
}

static Term_t Rev_QcostMinOper(int fill01, const Term_t& OperCand, vVarInfPtr_t& vVarInfPtr, const vSynObj_t::iterator fixedBegin, const vSynObj_t::iterator fixedEnd ){
	Term_t ret;
	vector<vSynObj_t::iterator> vUnchecked;

	ret.resize( OperCand.ndata() );
	if( 1 == fixedEnd - fixedBegin ){
		if( ret == fixedBegin->small() ) // zero 
			return ret;
	}
	
	for( vSynObj_t::iterator itr = fixedBegin; itr != fixedEnd; itr ++ )
		vUnchecked.push_back(itr);

	for(int i=0; i<vVarInfPtr.size(); i++){\
		int var = vVarInfPtr[i]->VarId;
	//for(int i=0; i<OperCand.ndata(); i++){\
		int var = fill01? i: (OperCand.ndata()-i-1);
		//int var = fill01? OperCand.ndata()-i-1: i;
	//for(int i=OperCand.ndata()-1; i>=0; i--){\
		int var = i;
		if( 0==OperCand.val( var ) )
			continue;
		ret.set( var, 1 );

		vector<vSynObj_t::iterator> vUncheckedNew;
		for(int j=0; j<vUnchecked.size(); j++){
			const Term_t& target = vUnchecked[j]->small();
			//if(Contain(OperCand,target)){\
				cout<<"OperCand is dominated "<< OperCand <<" by "<< target <<"  its pair is "<< vUnchecked[j]->large() <<std::endl;\
			}
			assert(!Contain(OperCand,target));

			if( (ret != OperCand)? target == (ret | target) : false )
				vUncheckedNew.push_back( vUnchecked[j] );
		}
		vUnchecked = vUncheckedNew;
		if( vUncheckedNew.empty() )
			break;
	}
	//cout<<"OperCand "<< OperCand <<" ret ="<< ret<<endl;
//	if( !vUnchecked.empty() ){
//		cout<<"OperCand "<< OperCand <<endl;
//		for(int i=0; i<vUnchecked.size(); i++){
//			cout<<"["<<i<<"] "<< vUnchecked[i]->first() <<" vs "<< vUnchecked[i]->second()  <<endl;
//		}
//		std::cout<<"fixed count: "<< fixedEnd - fixedBegin <<std::endl;
//		std::cout<<ret <<" vs "<< fixedBegin->small()<<std::endl;
//	}
	assert( vUnchecked.empty() );
	return ret;
}

static void Rev_EntryQcostSyn( Syn_Obj_t * pObj, Rev_Ntk_t& Ntk, vVarInfPtr_t& vVarInfPtr, const vSynObj_t::iterator fixedBegin, const vSynObj_t::iterator fixedEnd ){
	sort( vVarInfPtr.begin(), vVarInfPtr.end(), Var_Inf_t::Cmptor_t() );
	reverse( vVarInfPtr.begin(), vVarInfPtr.end() );
	
	const Term_t& small = pObj->small();
	const Term_t& large = pObj->large();
	Term_t Prog = large;   // progress
	Term_t Diff = small ^ large;
	vector<int> vOrder;

	FillOneFirstOrder(Diff, large, vOrder);

	//std::cout<<"synthesize for \n";\
	std::cout<< small <<" "<<large<<"\n";\
	std::cout<<"diff = "<< Diff <<"\n";
	for(int i=0; i<vOrder.size(); i++){
	//for(int i=0; i<Diff.ndata(); i++){
		//std::cout<< i <<" prog at "<< Prog <<std::endl;
		int idx = vOrder[i];
		if( !Diff.val(idx) )
			continue;
		Term_t Oper = Prog;
		Oper.set(idx,0);
		Oper = Rev_QcostMinOper(small.val(idx), Oper, vVarInfPtr, fixedBegin, fixedEnd );
		
		Rev_Gate_t Gate;
		Gate.setCtrl( Oper );
		Gate.setFlip( idx );
		Ntk.push_back( Gate );
		Prog.flip( idx );

		//std::cout<<"target bit= "<<idx<<" prog = "<< Prog<<" get gate = "; Gate.print(std::cout); std::cout<<"\n";
		//cout <<" working on "<< small <<" vs "<< large <<"  Oper "<< Oper <<" prog="<< Prog <<" get gate = "; Gate.print(std::cout); std::cout<<"\n";
	}
	//std::cout<<"prog vs. small= "<< Prog <<" "<< small<<std::endl;
	assert( Prog == small );
}

void blockSwap(vec<Lit>& v, int pos1, int pos2, int sz){
	for(int i=0; i<sz; i++)
		std::swap(v[pos1+i], v[pos2+i]);
}

void leftRotate(vec<Lit>& v, int p, int q, int r){
	if(p==q || q==r)
		return;
	if(q-p < r-q){
		int sz = q-p;
		blockSwap(v,p,r-sz,sz);
		leftRotate(v,p,q,r-sz);
	} else {
		int sz = r-q;
		blockSwap(v,p,r-sz,sz);
		leftRotate(v,p+sz,r-sz,r);
	}
}

int Lex_SatVarSel(SimpSolver * pSol, vec<Lit>& vAssume, int p, int q){
	lbool status = l_Undef;
	int r = (p+q)/2;
	int nLarge, nSmall;

	if(0 == q-p){
		cout<<"empty: "<<p<<" "<<q<<endl;
		return 0;
	}

	vec<Lit> vAssumeLocal;

	vAssumeLocal.clear();
	for(int i=0; i<r; i++)
		vAssumeLocal.push(vAssume[i]);

	for(int i=r; i<q; i++) // negating undecided variables 
		vAssumeLocal.push(~vAssume[i]);

	for(int i=q; i<vAssume.size(); i++) // negated variables 
		vAssumeLocal.push(vAssume[i]);

	if(1 == q-p){
		//vAssumeLocal[0] = ~vAssumeLocal[0];
		if(l_True == pSol->solveLimited(vAssumeLocal)){
			assert(!sign(vAssume[p]));
			vAssume[p] = ~vAssume[p];
//			cout << "unit SAT: \n\t";
//			for(int i=0; i<vAssume.size(); i++)
//				cout<< vAssume[i].x <<" ";
//			cout << endl<<"\t";
//			for(int i=0; i<vAssumeLocal.size(); i++)
//				cout<< vAssumeLocal[i].x <<" ";
//			cout << endl;
			return 0;
		} else {
//			cout <<"unit UNSAT ~~~ :"<<p<<" "<<r<<" "<<q<<": ";
//			for(int i=0; i<vAssumeLocal.size(); i++)
//				cout <<(sign(vAssumeLocal[i])? "-":"")<< var(vAssumeLocal[i]) << " ";
//			cout << endl;
			return 1;
		}
	}

	if(l_True == (status = pSol->solveLimited(vAssumeLocal))){
		//cout << " stage 205: "<<endl;
		//for(int i=0; i<vAssumeLocal.size(); i++)
		//	cout<< vAssumeLocal[i].x <<" ";
		//cout << endl;

		//cout << " stage 205 origin: "<<endl;
		//for(int i=0; i<vAssume.size(); i++)
		//	cout<< vAssume[i].x <<" ";
		//cout << endl;
		for(int i=r; i<q; i++){
			assert(!sign(vAssume[i]));
			vAssume[i] = ~vAssume[i];
		}
//		cout << " SAT: ";
//		for(int i=0; i<vAssume.size(); i++)
//			cout<< vAssume[i].x <<" ";
//		cout << endl;
		return Lex_SatVarSel(pSol, vAssume, p, r);
	} else {
//		cout <<" UNSAT ~~~ :";
//		for(int i=0; i<vAssumeLocal.size(); i++)
//			cout <<(sign(vAssumeLocal[i])? "-":"")<< var(vAssumeLocal[i]) << " ";
//		cout << endl;
	}
	
	// search [r:q), and obtain x lits
	nLarge = Lex_SatVarSel(pSol, vAssume, r, q);
	// move lit 
	leftRotate(vAssume, p, r, r+nLarge);

	// assume x lits from [r:q], and search [p:r), then obtain y lits. 
	nSmall = Lex_SatVarSel(pSol, vAssume, p+nLarge, p+nLarge+r-p);
//	printf("(%2d,%2d)\n",nSmall,nLarge);
	return nLarge + nSmall;
}

static Term_t Lex_QcostMinOper(SimpSolver * pSol, int fill01, const Term_t& OperCand, vVarInfPtr_t& vVarInfPtr, const vSynObj_t::iterator fixedBegin, const vSynObj_t::iterator fixedEnd ){
	Term_t ret;
	int nCtrl, nCand;
	vector<vSynObj_t::iterator> vUnchecked;
	vec<Lit> vAssume;

	ret.resize( OperCand.ndata() );
	if( 1 == fixedEnd - fixedBegin ){
		if( ret == fixedBegin->small() ) // zero 
			return ret;
	}

	for(int i=0; i<vVarInfPtr.size(); i++){
		int var = vVarInfPtr[i]->VarId;
		if( OperCand.val( var ) )
			vAssume.push(mkLit(var));
	}
	nCand = vAssume.size();
	for(int i=0; i<vVarInfPtr.size(); i++){
		int var = vVarInfPtr[i]->VarId;
		if(!OperCand.val( var ) )
			vAssume.push(mkLit(var,1));
	}
//	cout << "nCand= "<<OperCand << endl;\
	for(int i=0; i<vAssume.size(); i++)\
		cout << vAssume[i].x <<" ";\
	cout<<endl;
	nCtrl = Lex_SatVarSel(pSol, vAssume, 0, nCand);
//	cout << nCtrl << endl;\
	cout<<endl;

//	cout<<"selected: ";\
	for(int i=0; i<vAssume.size(); i++)\
		cout<<vAssume[i].x<< (i<nCtrl?"*":"") <<" ";\
	cout << endl;
	for(int i=0; i<nCtrl; i++)
		ret.set(var(vAssume[i]), 1);

	return ret;
}

static void Rev_EntryLexSyn(SimpSolver * pSol, Syn_Obj_t * pObj, Rev_Ntk_t& Ntk, vVarInfPtr_t& vVarInfPtr, const vSynObj_t::iterator fixedBegin, const vSynObj_t::iterator fixedEnd ){
	sort( vVarInfPtr.begin(), vVarInfPtr.end(), Var_Inf_t::Cmptor_t() );
	reverse( vVarInfPtr.begin(), vVarInfPtr.end() );
	
	const Term_t& small = pObj->small();
	const Term_t& large = pObj->large();
	Term_t Prog = large;   // progress
	Term_t Diff = small ^ large;
	vector<int> vOrder;

	// add clause 
	assert(pSol);
	vec<Lit> cla;
	for(int j=0; j<small.ndata(); j++)
		if(0 == small.val(j))
			cla.push(mkLit(j, 0));
	if(cla.size()){
		pSol->addClause(cla);
		//cout<<"add clause: ";\
		for(int j=0; j<cla.size(); j++)\
			cout<< var(cla[j]) <<" ";\
		cout << endl;
	}

	FillOneFirstOrder(Diff, large, vOrder);

	//std::cout<<"synthesize for \n";\
	std::cout<< small <<" "<<large<<"\n";\
	std::cout<<"diff = "<< Diff <<"\n";
	for(int i=0; i<vOrder.size(); i++){
	//for(int i=0; i<Diff.ndata(); i++){
		//std::cout<< i <<" prog at "<< Prog <<std::endl;
		int idx = vOrder[i];
		if( !Diff.val(idx) )
			continue;
		Term_t Oper = Prog;
		Oper.set(idx,0);
		Oper = Lex_QcostMinOper(pSol, small.val(idx), Oper, vVarInfPtr, fixedBegin, fixedEnd );
		//cout << "oper = "<< Oper <<endl;
		Rev_Gate_t Gate;
		Gate.setCtrl( Oper );
		Gate.setFlip( idx );
		Ntk.push_back( Gate );
		Prog.flip( idx );

		//std::cout<<"target bit= "<<idx<<" prog = "<< Prog<<" get gate = "; Gate.print(std::cout); std::cout<<"\n";
		//cout <<" working on "<< small <<" vs "<< large <<"  Oper "<< Oper <<" prog="<< Prog <<" get gate = "; Gate.print(std::cout); std::cout<<"\n";
	}
	//std::cout<<"prog vs. small= "<< Prog <<" "<< small<<std::endl;
	assert( Prog == small );
}

void Rev_Syn_t::synthesizeEntry(int idx, Rev_Ntk_t& Ntk){
	if(0 == ctrlMode){
		Rev_EntrySimpleSyn(&vSynObj[idx], Ntk);
		return;
	} else
	if(1 == ctrlMode){
		Rev_EntryQcostSyn( &vSynObj[idx], Ntk, vVarInfPtr, vSynObj.begin(), vSynObj.begin() + idx + 1 );
		return;
	} else
	if(2 == ctrlMode){
		//Rev_EntryQcostSyn( &vSynObj[idx], Ntk, vVarInfPtr, vSynObj.begin(), vSynObj.begin() + idx + 1 );
		Rev_EntryLexSyn(pSol, &vSynObj[idx], Ntk, vVarInfPtr, vSynObj.begin(), vSynObj.begin() + idx + 1 );
		return;
	}
	assert(false);
}

int Rev_Syn_t::OneGateAssumeCost(Rev_Ntk_t& tmpGate, TermObj_t * pTermObj, vSynObj_t::iterator fixedBegin, vSynObj_t::iterator fixedEnd, vVarInfPtr_t& vVarInfPtr){
	tmpGate.reset(pTermObj->pTerm->ndata());
	Syn_Obj_t * pObj = (Syn_Obj_t*) pTermObj->pSrc;
	
	pObj->update_small(*pTermObj->pTerm==pTermObj->pSrc->first()? 0: 1); // assume the direction
	std::swap( *fixedEnd, *pObj );	// assume
	
	synthesizeEntry(fixedEnd- fixedBegin, tmpGate);
	
	pObj->update_small(); 			// restore 
	std::swap( *fixedEnd, *pObj );	// restore 
	return tmpGate.QCost();
}


int Rev_Syn_t::entryCost(int idx, int fixedTop){
	// hamming distance
	if(0 == costMode){
		return vSynObj[idx].hamdist();
	} else
	if(1 == costMode){
		TermObj_t obj1(&vSynObj[idx].small(), &vSynObj[idx]);
		swap( vSynObj[fixedTop], vSynObj[idx] );
		int ret = OneGateAssumeCost(_SubNtk, &obj1,vSynObj.begin(),vSynObj.begin()+fixedTop,vVarInfPtr);
		swap( vSynObj[fixedTop], vSynObj[idx] );
		return ret;
	}
	return -1;
}


static void tryInsertTermObj( vector< TermObjSet_t >& mW2Term, const Syn_Obj_t * pSrc, const Term_t& term )
{
	const int weight = term.weight();
	const Term_t& CounterPart =  term == pSrc->first()? pSrc->second(): pSrc->first();
	//bool IOillegal = Contain( CounterPart, term );
	TermObj_t Obj( &term, pSrc );
	TermObjSet_t& TermObjSet = mW2Term[ weight ];
//	TermObjSet_t::iterator itr = TermObjSet.find( Obj );
//	if( TermObjSet.end() != itr ){
//		return;
//		//if( itr->src() == pSrc )\
//			return;
//		//if( itr->src()->hamdist() < pSrc->hamdist() )\
//			return;
//		//if(!itr->isLegal() )  // the term has been denoted as illegal fix\
//			return;
//		TermObjSet.erase( itr );
//	}
	TermObjSet.insert(Obj);
}

static void LegalHamWeightGraph( vSynObj_t& vSynObj, vSynObj_t::iterator unfixedBegin, vector< TermObjSet_t >& Ret )
{
	if( vSynObj.empty() )
		return;
	vector< TermObjSet_t >& mW2Term = Ret;    // weight to terms 
	mW2Term.clear();
	mW2Term.resize( vSynObj.front().first().ndata() + 1 );
	for( vSynObj_t::iterator itr = unfixedBegin; itr != vSynObj.end(); itr ++ ){
		tryInsertTermObj( mW2Term, &*itr, itr->first () );
		tryInsertTermObj( mW2Term, &*itr, itr->second() );
	}
//	int nWeight = 0;
//	for(int i=0; i< mW2Term.size(); i++)
//		if( ! mW2Term[i].empty() )
//			nWeight ++;
//	Ret.resize( nWeight );
//	nWeight = 0;
//	for(int i=0; i< mW2Term.size(); i++)
//		if( ! mW2Term[i].empty() )
//			swap( Ret[ nWeight ++ ], mW2Term[i] );
}


int Rev_Syn_t::selectEntry(int idx){
	int i, j, ret;

	// lexicographic ordering
	if(0 == legalPath){
		int retCost, curCost;
		ret = idx;
		retCost = entryCost(idx,idx);
		for(i=idx + 1; i<vSynObj.size(); i++)
			if(vSynObj[i].small() != vSynObj[ret].small()
				? vSynObj[i].small() < vSynObj[ret].small()
				: (curCost = entryCost(i,idx)) < retCost)
				ret = i, retCost = curCost;
		return ret;
	} else
	if(1 == legalPath){
		mW2Term.clear();
		LegalHamWeightGraph( vSynObj, vSynObj.begin() + idx, mW2Term );
	}
	assert(false);
	return -1;
}

void Rev_Syn_t::dataInitialize(const Rev_Ttb_t& ttb){
	ttb2.dup(&ttb); // create a truth table same as ttb
	// prepare array for sorting 
	vSynObj.clear();
	vSynObj.resize( ttb2.size() );
	for(int i=0; i<vSynObj.size(); i++){
		vSynObj[i] = Syn_Obj_t( ttb2[i] );
		vSynObj[i].update_small();
	}
	
	if(1 <= ctrlMode){
		vVarInf.clear();
		vVarInfPtr.clear();
		vVarInf.resize( ttb.width() );
		vVarInfPtr.resize( ttb.width() );
		for(int i=0; i<vVarInf.size(); i++){
			vVarInf[i].VarId = i;
			vVarInfPtr[i] = &vVarInf[i];
		}
	}

	if(2 == ctrlMode){
		pSol = new SimpSolver;
		for(int i=0; i<ttb.width(); i++)
			pSol->newVar();
	}
}

void Rev_Syn_t::dataUpdate(int idx){
	if(1 <= ctrlMode){
		Term_t& term = vSynObj[idx].first();
		for(int j=0; j<term.ndata(); j++)
			if(0 == term.val(j))
				vVarInf[j].nCtrAbl ++ ;
	}
}

Rev_Ntk_t * Rev_Syn_t::perform(const Rev_Ttb_t& ttb){	
	Rev_Ntk_t * pNtk;
	int width = ttb.width();
	_nEntry = 0;

	pNtk = new Rev_Ntk_t( width );
	
	dataInitialize(ttb);

	int nTiebreak = 0;
	int target = -1;

	Rev_Ntk_t NtkFront(width), NtkBack(width);
	for(int i=0; i<vSynObj.size(); i++){
		//cout<<"syn@"<<i<<": "<<endl;\
		ttb2.print(cout);\
		cout<<endl;

		//pick min-minterm, tie-break by hamdist
		if(i != (target = selectEntry(i)))
			swap(vSynObj[i], vSynObj[target]);

		bool fForward = vSynObj[i].isForward();
		_SubNtk.reset(width);
		Rev_Ntk_t& SubNtk = _SubNtk;

		synthesizeEntry(i, SubNtk);

//		if( fQGBD ){
//			Rev_EntryQcostSyn( &vSynObj[i], SubNtk, vVarInfPtr, vSynObj.begin(), vSynObj.begin() + i + 1 );
//		} else {
//			Rev_EntrySimpleSyn( &vSynObj[i], SubNtk );            // synthesis for one IO pair
//		}
		
		//SubNtk.print(cout); cout<<endl;
		if(SubNtk.nLevel())
		for(int j=i; j<vSynObj.size(); j++){
			SubNtk.Apply( fForward? vSynObj[j].second(): vSynObj[j].first () );
			
			//cout<<"fForward? "<< fForward<<": " << vSynObj[j].second() <<" vs. "<< vSynObj[j].first () <<endl; 
			//assert(vSynObj[j].second() == vSynObj[j].first () );
			vSynObj[j].update_small();
		}

		dataUpdate(i);

		if( 0==SubNtk.nLevel() )
			continue;

		if( fForward )
			NtkBack .Append( SubNtk );
		else
			NtkFront.Append( SubNtk );
	}
	pNtk->Append( NtkFront );
	NtkBack.reverse();
	pNtk->Append( NtkBack  );
	printf("tie-break # %d\n", nTiebreak);
	return pNtk;
}
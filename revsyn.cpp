#include "ttb.hpp"
#include "ntk.hpp"
#include "revsynUtil.hpp"
#include "revsyn_.hpp"
#include <time.h>

using namespace std;
const TermObj_t TermObj_t::null = TermObj_t(NULL,NULL);;

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


// find min weight over unfixed terms
static int MinWeight( const vSynObj_t& vSynObj, const vSynObj_t::iterator unfixedBegin ){
	int ret = -1;
	int weight = 0;
	for(vSynObj_t::const_iterator uitr = unfixedBegin; uitr != vSynObj.end(); uitr++)
	{
		const Term_t& iTerm = uitr->first ();
		const Term_t& oTerm = uitr->second();
		if( (weight= iTerm.weight()) < ret ? true: (-1 == ret) )
			ret = weight;
		if( (weight= oTerm.weight()) < ret ? true: (-1 == ret) )
			ret = weight;
	}
	assert( -1 != ret );
	return ret;
}

static clock_t LHDclk = 0;
// find upper-bound of legal fixing hamming distance
static int LegalHammingDistUBound( const vSynObj_t& vSynObj, const vSynObj_t::iterator unfixedBegin ){
	int minWeight = MinWeight( vSynObj, unfixedBegin );
	int ret = -1;
	clock_t clk = clock();
	for(vSynObj_t::const_iterator uitr = unfixedBegin; uitr != vSynObj.end(); uitr++)
	{
		const Term_t& iTerm = uitr->first ();
		const Term_t& oTerm = uitr->second();
		int iWeight = iTerm.weight();
		int oWeight = oTerm.weight();
		//cout << iTerm <<" -> "<< oTerm << " minWeight = "<< minWeight <<endl;
		if( iWeight > minWeight && oWeight > minWeight )
			continue;
		int Dist = ( iTerm ^ oTerm ).weight();
		if( -1 == ret || Dist < ret )
			ret = Dist;
	}
	//cout<< " ret = "<< ret <<endl;
	assert( -1 != ret );
	LHDclk += clock() - clk;
	return ret;
}



static clock_t ISLclk = 0;
static int IsLegalFix( const Term_t& prime, const vSynObj_t::iterator unfixedBegin, const vSynObj_t::iterator unfixedEnd ){
	clock_t clk = clock();
	for(vSynObj_t::iterator itr = unfixedBegin; itr != unfixedEnd; itr++)
	{
		const Term_t& iTerm = itr->first ();
		const Term_t& oTerm = itr->second();
		if( Contain( iTerm, prime ) || Contain( oTerm, prime ) )
			return 0;
	}
	ISLclk += clock() - clk;
	return 1;
}

static clock_t DIFclk = 0, CTNclk = 0;
static vSynObj_t::iterator SelectFix( vSynObj_t& vSynObj, const vSynObj_t::iterator unfixedBegin )
{
	clock_t clk;
	int LegalDistUBound = LegalHammingDistUBound( vSynObj, unfixedBegin );

	vSynObj_t::iterator ret = vSynObj.end();
	for(vSynObj_t::iterator itr = unfixedBegin; itr != vSynObj.end(); itr++)
	{
		const Term_t& iTerm = itr->first ();
		const Term_t& oTerm = itr->second();
		clk = clock();
		int Dist = ( iTerm ^ oTerm ).weight();
		DIFclk += clock() - clk;
		if( Dist > LegalDistUBound )
			continue;

		// check legality 
		clk = clock();
		int nIllegal_i  = Contain( oTerm, iTerm );  // check if iTerm is illegal at beginning
		int nIllegal_o  = Contain( iTerm, oTerm );  // check if oTerm is illegal at beginning
		CTNclk += clock() - clk;

		// test legality for all care-terms
		if( 0 == nIllegal_i )
			nIllegal_i += ! IsLegalFix( iTerm, unfixedBegin, vSynObj.end() );
		if( 0 == nIllegal_o )
			nIllegal_o += ! IsLegalFix( oTerm, unfixedBegin, vSynObj.end() );
		
		if( Dist < LegalDistUBound || vSynObj.end() == ret ){
			if( 0 == nIllegal_i )
				itr->setLegal(0);
			else
			if( 0 == nIllegal_o )
				itr->setLegal(1);
			else 
				continue;          // both iTerm & oTerm are illegal 

			ret = itr;
			LegalDistUBound = Dist;
			//cout<<" is legal fix = "<< ret->first()<< " -> "<< ret->second() <<" LegalDistUBound= "<< LegalDistUBound << endl;
		}
	}
	assert( vSynObj.end() != ret );
	//cout<<" selected fix = "<< ret->first()<< " -> "<< ret->second() <<" LegalDistUBound= "<< LegalDistUBound << endl;
	return ret;
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


static int IsLegalFix2( const Term_t& prime, int GroupId, vector< TermObjSet_t >& mW2Term ){
	clock_t clk = clock();
	for(int i = GroupId-1; i >= 0; i-- )
	{
		for(TermObjSet_t::iterator itr = mW2Term[i].begin(); itr != mW2Term[i].end(); itr++)
		{
			const Term_t& term = * itr->pTerm;
			if( Contain( term, prime ) )
				return 0;
		}
	}
	ISLclk += clock() - clk;
	return 1;
}

inline static int countOneNum(vVarInf_t& vOneInf, const Term_t& term){
	int ret = 0;
	for(int i=0; i<term.ndata(); i++)
		if(term.val(i))
			ret += vOneInf[i].nCtrAbl;
	return ret;
}

static inline int OneGateAssumeCost(TermObj_t * pTermObj, vSynObj_t::iterator fixedBegin, vSynObj_t::iterator fixedEnd, vVarInfPtr_t& vVarInfPtr){
	Rev_Ntk_t tmpGate(pTermObj->pTerm->ndata());
	Syn_Obj_t * pObj = (Syn_Obj_t*) pTermObj->pSrc;
	
	pObj->update_small(*pTermObj->pTerm==pTermObj->pSrc->first()? 0: 1); // assume the direction
	std::swap( *fixedEnd, *pObj );	// assume
	
	Rev_EntryQcostSyn( &*pObj, tmpGate, vVarInfPtr, fixedBegin, fixedEnd + 1 );
	
	pObj->update_small(); 			// restore 
	std::swap( *fixedEnd, *pObj );	// restore 
	return tmpGate.QCost();
}

//static TermObj_t SelectFix2( vector< TermObjSet_t >& mW2Term, const vSynObj_t::iterator unfixedBegin, const vSynObj_t::iterator unfixedEnd ){
static TermObj_t SelectFix2( vector< TermObjSet_t >& mW2Term, vVarInf_t& vOneInf, vVarInfPtr_t& vVarInfPtr, vSynObj_t::iterator fixedBegin, vSynObj_t::iterator fixedEnd ){
//	for(int i=0; i< mW2Term.size(); i++){
//		for(TermObjSet_t::iterator itr = mW2Term[i].begin(); itr != mW2Term[i].end(); itr ++ )
//			cout << *itr->pTerm << " ";
//		cout << endl;
//	}
//	cout<<endl;
	bool fExactCost = true;
	TermObj_t ret = TermObj_t::null;
	int retWeight, curWeight;
	int retOneNum, curOneNum;
	int retCost, curCost;
	int retLead1, curLead1;
	int retLead12, curLead12;
	int retDst, curDst;
	//Rev_EntryQcostSyn( &ret, SubNtk, vVarInfPtr, vSynObj.begin(), vSynObj.begin() + i + 1 );
	for(int i=0; i< mW2Term.size(); i++){
		for(TermObjSet_t::iterator itr = mW2Term[i].begin(); itr != mW2Term[i].end(); itr ++ ){
			if( !IsLegalFix2(* itr->pTerm, i, mW2Term))\
				continue;
			//if( !itr->isLegal() )\
				break;
			//if( !IsLegalFix( *itr->pTerm, unfixedBegin, unfixedEnd ))\
				continue;
			curLead1 = itr->pTerm->leading1();
			curLead12= itr->pSrc->another(*itr->pTerm).leading1();
			curDst   = itr->pSrc->hamdist();

			curWeight = itr->pTerm->weight();
			// select larger hamming weight
			if(ret != TermObj_t::null && retWeight > curWeight)\
				continue;
			// select larger hamming distance
			//if(ret != TermObj_t::null && retDst > curDst)\
				continue;
			//if(ret != TermObj_t::null && retLead1 > curLead1)\
				continue;\
			if(ret != TermObj_t::null && retLead12< curLead12)\
				continue;


			curOneNum = countOneNum(vOneInf, itr->pSrc->first ());
			curOneNum+= countOneNum(vOneInf, itr->pSrc->second());
			if( fExactCost ){
				curCost   = OneGateAssumeCost((TermObj_t*)&*itr,fixedBegin,fixedEnd,vVarInfPtr);
				if(ret == TermObj_t::null? true: (curCost != retCost? curCost < retCost: (*itr->pTerm < *ret.pTerm)))     // exact cost
					ret = * itr, retCost = curCost, retWeight = curWeight, retLead1 = curLead1, retLead12 = curLead12, retDst = curDst;
				continue;
			}
//			Rev_Ntk_t tmpGate(itr->pTerm->ndata());
//			Syn_Obj_t * pObj = (Syn_Obj_t*) itr->pSrc;
//			pObj->update_small(itr->pTerm==itr->pSrc->first()? 0: 1); // assume the direction
//			Rev_EntryQcostSyn( &*pObj, tmpGate, vVarInfPtr, vSynObj.begin(), vSynObj.begin() + i + 1 );
//			pObj->update_small(); // restore 
			//if(ret == TermObj_t::null? true: itr->pSrc->maxWeight() > ret.pSrc->maxWeight()) // 
			//if(ret == TermObj_t::null? true: (itr->pTerm->weight() > ret.pTerm->weight()))
			//if(ret == TermObj_t::null? true: (itr->pSrc->another(*itr->pTerm).weight() > ret.pSrc->another(*ret.pTerm).weight()))
			//if(ret == TermObj_t::null? true: (*itr->pTerm > *ret.pTerm))                       // lexical greatest first
			//if(ret == TermObj_t::null? true: (itr->pSrc->hamdist() < ret.pSrc->hamdist()))     // smallest cost 
			//if(ret == TermObj_t::null? true: (curCost != retCost? curCost < retCost: (*itr->pTerm < *ret.pTerm)))     // exact cost
			if(ret == TermObj_t::null? true: (*itr->pTerm < *ret.pTerm))                       // lexical smallest first 
			//if(ret == TermObj_t::null? true: (curOneNum != retOneNum? curOneNum < retOneNum: *itr->pTerm < *ret.pTerm))
			//if(ret == TermObj_t::null? true: (itr->pSrc->hamdist() != ret.pSrc->hamdist()? itr->pSrc->hamdist() < ret.pSrc->hamdist(): (*itr->pTerm < *ret.pTerm)))     // smallest cost 
				ret = * itr, retCost = curCost, retWeight = curWeight, retLead1 = curLead1, retLead12 = curLead12;
		}
	}
	return ret;
}

inline static void addOneInf(vVarInf_t& vOneInf, Term_t& term){
	for(int i=0; i<term.ndata(); i++)
		if(term.val(i))
			vOneInf[i].nCtrAbl ++ ;
}
inline static void eraseOneInf(vVarInf_t& vOneInf, Term_t& term){
	for(int i=0; i<term.ndata(); i++)
		if(term.val(i))
			vOneInf[i].nCtrAbl -- ;
}

Rev_Ntk_t * _Rev_GBDL( const Rev_Ttb_t& ttb, bool fQGBD ){
	
	Rev_Ttb_t ttb2(ttb); // create a truth table same as ttb
	Rev_Ntk_t * pNtk;
	vVarInf_t vVarInf;         // for qGBD only 
	vVarInfPtr_t vVarInfPtr;   // for qGBD only 

	vVarInf_t vOneInf;         // for qGBD only 
	int width = ttb.width();
	clock_t clk, clk1;
	clk = 0;

	// prepare array for sorting 
	vSynObj_t vSynObj( ttb2.size() );
	for(int i=0; i<vSynObj.size(); i++){
		vSynObj[i] = Syn_Obj_t( ttb2[i] );
		vSynObj[i].update_small();
	}

	if( fQGBD || true){
		vVarInf.resize( ttb.width() );
		vVarInfPtr.resize( ttb.width() );
		for(int i=0; i<vVarInf.size(); i++){
			vVarInf[i].VarId = i;
			vVarInfPtr[i] = &vVarInf[i];
		}


		vOneInf.resize( ttb.width() );
		for(int i=0; i<vOneInf.size(); i++)
			vOneInf[i].VarId = i;
		for(int i=0; i<vSynObj.size(); i++){
			addOneInf(vOneInf, ttb2[i]->first );
			addOneInf(vOneInf, ttb2[i]->second);
		}
		printf("one ctrl:\n");
		for(int i=0; i<vOneInf.size(); i++)
			printf("%d ", vOneInf[i].nCtrAbl);
		printf("\n");
	}

	pNtk = new Rev_Ntk_t( width );
	Rev_Ntk_t NtkFront(width), NtkBack(width);
	for(int i=0; i<ttb2.size(); i++){
		vector< TermObjSet_t > mW2Term;
		TermObj_t tar;
		clk1 = clock();
		LegalHamWeightGraph( vSynObj, vSynObj.begin() + i, mW2Term );
//		std::cout<<"legal hw graph:\n";
//		for(int j=0; j<mW2Term.size(); j++){
//			std::cout<<j<<" ";
//			for(TermObjSet_t::iterator itr=mW2Term[j].begin(); itr!=mW2Term[j].end(); itr++)
//				std::cout<< *itr->pTerm<< " ";
//			std::cout<<"\n";
//		}
//		std::cout<<"\n";
		tar = SelectFix2( mW2Term, vOneInf, vVarInfPtr, vSynObj.begin(), vSynObj.begin() + i );
		//tar = SelectFix2( mW2Term, vSynObj.begin() + i, vSynObj.end() );
		assert(tar!=TermObj_t::null);
		assert(tar.isLegal());
		clk += clock() - clk1;

		Rev_Ntk_t SubNtk(width);
		Syn_Obj_t ret(tar.pSrc->pB);
		bool fForward = *tar.pTerm==tar.pSrc->first();

		//std::cout<<"fix " << *tar.pTerm<<" in the pair: ";\
		tar.pSrc->pB->first.print(std::cout); std::cout <<" "; tar.pSrc->pB->second.print(std::cout);  std::cout<<"\n";

		ret.update_small(fForward? 0: 1);

		Syn_Obj_t * addr = (Syn_Obj_t*)tar.pSrc;
		addr->update_small(fForward? 0: 1);
		int prev_idx = addr - &*vSynObj.begin();
		mW2Term.clear();
		swap( *(vSynObj.begin()+i), *addr );


		for(int j=0; j<=i; j++){
			if(Contain(*tar.pTerm,vSynObj[j].small())){
				std::cout<<"!!! "<< * tar.pTerm <<" "<<vSynObj[j].small()<<std::endl;
			}
			assert(!Contain(*tar.pTerm,vSynObj[j].small()));
		}
		//std::cout<<"current: \n";\
		for(int j=0; j<vSynObj.size(); j++){\
			std::cout<<j<<" "<<vSynObj[j].first()<<" "<<vSynObj[j].second() <<std::endl;\
		}

		if( fQGBD ){
			Rev_EntryQcostSyn( &ret, SubNtk, vVarInfPtr, vSynObj.begin(), vSynObj.begin() + i + 1 );
		} else {
			Rev_EntrySimpleSyn( &ret, SubNtk );            // synthesis for one IO pair
		}

		if(SubNtk.nLevel())
		for(int j=i; j<vSynObj.size(); j++){
			if( fQGBD )
				eraseOneInf(vOneInf, fForward? vSynObj[j].second(): vSynObj[j].first ());
			SubNtk.Apply( fForward? vSynObj[j].second(): vSynObj[j].first () );
			if( fQGBD )
				addOneInf(vOneInf, fForward? vSynObj[j].second(): vSynObj[j].first ());
			vSynObj[j].update_small();
		}

		if( fQGBD ){
			Term_t& term = vSynObj[i].first();
			for(int j=0; j<term.ndata(); j++)
				if( 0==term.val(j) )
					vVarInf[j].nCtrAbl ++ ;
		}

		if(0 == SubNtk.nLevel())
			continue;		
		if( fForward )
			NtkBack .Append( SubNtk );
		else
			NtkFront.Append( SubNtk );
	}

	printf("one ctrl:\n");
	for(int i=0; i<vOneInf.size(); i++)
		printf("%d ", vOneInf[i].nCtrAbl);
	printf("\n");

	pNtk->Append( NtkFront );
	NtkBack.reverse();
	pNtk->Append( NtkBack  );
	//cout<<" Select Time = " << clk/CLOCKS_PER_SEC <<endl;
	//cout<<" LHD Time = " << LHDclk/CLOCKS_PER_SEC <<endl;
	//cout<<" ISL Time = " << ISLclk/CLOCKS_PER_SEC <<endl;
	//cout<<" DIF Time = " << DIFclk/CLOCKS_PER_SEC <<endl;
	//cout<<" CTN Time = " << CTNclk/CLOCKS_PER_SEC <<endl;
	return pNtk;
}


Rev_Ntk_t * Rev_GBDL( const Rev_Ttb_t& ttb ){
	return _Rev_GBDL(ttb, false);
}

Rev_Ntk_t * Rev_qGBDL( const Rev_Ttb_t& ttb ){
	return _Rev_GBDL(ttb, true);
}



static Rev_Ntk_t * _Rev_GBD( const Rev_Ttb_t& ttb, bool fQGBD ){
	Rev_Ttb_t ttb2(ttb); // create a truth table same as ttb
	Rev_Ntk_t * pNtk;
	vVarInf_t vVarInf;         // for qGBD only 
	vVarInfPtr_t vVarInfPtr;   // for qGBD only 
	int width = ttb.width();

	pNtk = new Rev_Ntk_t( width );
	// prepare array for sorting 
	vSynObj_t vSynObj( ttb2.size() );
	for(int i=0; i<vSynObj.size(); i++){
		vSynObj[i] = Syn_Obj_t( ttb2[i] );
		vSynObj[i].update_small();
	}
	
	if( fQGBD ){
		vVarInf.resize( ttb.width() );
		vVarInfPtr.resize( ttb.width() );
		for(int i=0; i<vVarInf.size(); i++){
			vVarInf[i].VarId = i;
			vVarInfPtr[i] = &vVarInf[i];
		}
	}
	int nTiebreak = 0;

	Rev_Ntk_t NtkFront(width), NtkBack(width);
	for(int i=0; i<vSynObj.size(); i++){

		//pick min-minterm, tie-break by hamdist
		std::sort( vSynObj.begin()+i, vSynObj.end(), Syn_Obj_t::Cmptor_t() );
		
		if( i+1<vSynObj.size() )
			if( vSynObj[i].small()==vSynObj[i+1].small() ){       // check tie-breack condition
				if(fQGBD){
					TermObj_t obj1(&vSynObj[i].small(), &vSynObj[i]);
					int Cost1   = OneGateAssumeCost(&obj1,vSynObj.begin(),vSynObj.begin()+i,vVarInfPtr);
					swap( vSynObj[i], vSynObj[i+1] );
					TermObj_t obj2(&vSynObj[i].small(), &vSynObj[i]);
					int Cost2   = OneGateAssumeCost(&obj2,vSynObj.begin(),vSynObj.begin()+i,vVarInfPtr);
					swap( vSynObj[i], vSynObj[i+1] );

					if(Cost1 > Cost2)
						swap( vSynObj[i], vSynObj[i+1] ), nTiebreak ++;
				} else
				if( vSynObj[i].hamdist()>vSynObj[i+1].hamdist() ) // select smaller hamming distance
					swap( vSynObj[i], vSynObj[i+1] ), nTiebreak ++;
			}

		bool fForward = vSynObj[i].isForward();
		Rev_Ntk_t SubNtk(width);

		if( fQGBD ){
			Rev_EntryQcostSyn( &vSynObj[i], SubNtk, vVarInfPtr, vSynObj.begin(), vSynObj.begin() + i + 1 );
		} else {
			Rev_EntrySimpleSyn( &vSynObj[i], SubNtk );            // synthesis for one IO pair
		}
		
		if(SubNtk.nLevel())
		for(int j=i; j<vSynObj.size(); j++){
			SubNtk.Apply( fForward? vSynObj[j].second(): vSynObj[j].first () );
			vSynObj[j].update_small();
		}


		if( fQGBD ){
			Term_t& term = vSynObj[i].first();
			for(int j=0; j<term.ndata(); j++)
				if(0 == term.val(j))
					vVarInf[j].nCtrAbl ++ ;
		}

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


Rev_Ntk_t * Rev_GBD( const Rev_Ttb_t& ttb ){
	Rev_Syn_t core;
	core.ctrlMode = 0;
	core.legalPath= 0;
	core.costMode = 0;
	return core.perform(ttb);
	return _Rev_GBD( ttb, false );
}

Rev_Ntk_t * Rev_qGBD( const Rev_Ttb_t& ttb ){
	Rev_Syn_t core;
	core.ctrlMode = 1;
	core.legalPath= 0;
	core.costMode = 0;
	return core.perform(ttb);
	return _Rev_GBD( ttb, true );
}

Rev_Ntk_t * Rev_sGBD( const Rev_Ttb_t& ttb ){
	Rev_Syn_t core;
	core.ctrlMode = 2;
	core.legalPath= 0;
	core.costMode = 0;
	return core.perform(ttb);
	return _Rev_GBD( ttb, true );
}

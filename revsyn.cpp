#include "ttb.hpp"
#include "ntk.hpp"
#include "revsynUtil.hpp"
#include <time.h>

using namespace std;

static Term_t Rev_QcostMinOper( const Term_t& OperCand, vVarInfPtr_t& vVarInfPtr, const vSynObj_t::iterator fixedBegin, const vSynObj_t::iterator fixedEnd ){
	Term_t ret;
	vector<vSynObj_t::iterator> vUnchecked;

	ret.resize( OperCand.ndata() );
	if( 1 == fixedEnd - fixedBegin ){
		if( ret == fixedBegin->small() ) // zero 
			return ret;
	}
	
	for( vSynObj_t::iterator itr = fixedBegin; itr != fixedEnd; itr ++ )
		vUnchecked.push_back(itr);

	for(int i=0; i<vVarInfPtr.size(); i++){
		int var = vVarInfPtr[i]->VarId;
		if( 0==OperCand.val( var ) )
			continue;
		ret.set( var, 1 );

		vector<vSynObj_t::iterator> vUncheckedNew;
		for(int j=0; j<vUnchecked.size(); j++){
			const Term_t& target = vUnchecked[j]->small();
			if( (ret != OperCand)? target == (ret | target) : false )
				vUncheckedNew.push_back( vUnchecked[j] );
		}
		vUnchecked = vUncheckedNew;
		if( vUncheckedNew.empty() )
			break;
	}
//	if( !vUnchecked.empty() ){
//		cout<<"OperCand "<< OperCand <<endl;
//		for(int i=0; i<vUnchecked.size(); i++){
//			cout<<"["<<i<<"] "<< vUnchecked[i]->first() <<" vs "<< vUnchecked[i]->second()  <<endl;
//		}
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

	for(int i=0; i<Diff.ndata(); i++){
		if( !Diff.val(i) )
			continue;
		int idx = i;
		Term_t Oper = Prog;
		Oper.set(idx,0);
		Oper = Rev_QcostMinOper( Oper, vVarInfPtr, fixedBegin, fixedEnd );
		
		Rev_Gate_t Gate;
		Gate.setCtrl( Oper );
		Gate.setFlip( idx );
		Ntk.push_back( Gate );
		Prog.flip( idx );

		//cout <<" working on "<< small <<" vs "<< large <<"  Oper "<< Oper <<" prog="<< Prog << endl;
	}
	assert( Prog == small );
}

static void FillOneFirstOrder( const Term_t& Diff, const Term_t& large, vector<int>& vOrder ){
	for(int i=0; i<Diff.ndata(); i++){
		if( !Diff.val(i) )
			continue;
		if( 1==large.val(i) )
			continue;
		vOrder.push_back( i );
	}

	for(int i=0; i<Diff.ndata(); i++){
		if( !Diff.val(i) )
			continue;
		if( 0==large.val(i) )
			continue;
		vOrder.push_back( i );
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

	Rev_Ntk_t NtkFront(width), NtkBack(width);
	for(int i=0; i<vSynObj.size(); i++){

		//pick min-minterm, tie-break by hamdist
		std::sort( vSynObj.begin()+i, vSynObj.end(), Syn_Obj_t::Cmptor_t() );
		
		if( i+1<vSynObj.size() )
			if( vSynObj[i].small()==vSynObj[i+1].small() ){       // check tie-breack condition
				if( vSynObj[i].hamdist()>vSynObj[i+1].hamdist() ) // select smaller hamming distance
					swap( vSynObj[i], vSynObj[i+1] );
			}

		bool fForward = vSynObj[i].isForward();
		Rev_Ntk_t SubNtk(width);

		if( fQGBD ){
			Rev_EntryQcostSyn( &vSynObj[i], SubNtk, vVarInfPtr, vSynObj.begin(), vSynObj.begin() + i + 1 );
		} else {
			Rev_EntrySimpleSyn( &vSynObj[i], SubNtk );            // synthesis for one IO pair
		}
		
		if( 0==SubNtk.nLevel() )
			continue;

		for(int j=i; j<vSynObj.size(); j++){
			SubNtk.Apply( fForward? vSynObj[j].second(): vSynObj[j].first () );
			vSynObj[j].update_small();
		}


		if( fQGBD ){
			Term_t& term = vSynObj[i].first();
			for(int j=0; j<term.ndata(); j++)
				if( 0==term.val(j) )
					vVarInf[j].nCtrAbl ++ ;
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


Rev_Ntk_t * Rev_GBD( const Rev_Ttb_t& ttb ){
	return _Rev_GBD( ttb, false );
}

Rev_Ntk_t * Rev_qGBD( const Rev_Ttb_t& ttb ){
	return _Rev_GBD( ttb, true );
}


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

// find upper-bound of legal fixing hamming distance
static int LegalHammingDistUBound( const vSynObj_t& vSynObj, const vSynObj_t::iterator unfixedBegin ){
	int minWeight = MinWeight( vSynObj, unfixedBegin );
	int ret = -1;
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
	return ret;
}


// test whether space contains prime 
static bool Contain( const Term_t& prime, const Term_t& space ){
	return ( prime == ( prime & space ) ) && ( prime != space );
}

static int IsLegalFix( const Term_t& prime, const vSynObj_t::iterator unfixedBegin, const vSynObj_t::iterator unfixedEnd ){
	for(vSynObj_t::iterator itr = unfixedBegin; itr != unfixedEnd; itr++)
	{
		const Term_t& iTerm = itr->first ();
		const Term_t& oTerm = itr->second();
		if( Contain( iTerm, prime ) || Contain( oTerm, prime ) )
			return 0;
	}
	return 1;
}

static vSynObj_t::iterator SelectFix( vSynObj_t& vSynObj, const vSynObj_t::iterator unfixedBegin ){
	int LegalDistUBound = LegalHammingDistUBound( vSynObj, unfixedBegin );

	vSynObj_t::iterator ret = vSynObj.end();
	for(vSynObj_t::iterator itr = unfixedBegin; itr != vSynObj.end(); itr++)
	{
		const Term_t& iTerm = itr->first ();
		const Term_t& oTerm = itr->second();
		int Dist = ( iTerm ^ oTerm ).weight();
		if( Dist > LegalDistUBound )
			continue;

		// check legality 
		int nIllegal_i  = Contain( oTerm, iTerm );  // check if iTerm is illegal at beginning
		int nIllegal_o  = Contain( iTerm, oTerm );  // check if oTerm is illegal at beginning

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

Rev_Ntk_t * Rev_GBDL( const Rev_Ttb_t& ttb ){
	Rev_Ttb_t ttb2(ttb); // create a truth table same as ttb
	Rev_Ntk_t * pNtk;
	vVarInf_t vVarInf;         // for qGBD only 
	vVarInfPtr_t vVarInfPtr;   // for qGBD only 
	int width = ttb.width();
	clock_t clk, clk1;
	clk = 0;

	// prepare array for sorting 
	vSynObj_t vSynObj( ttb2.size() );
	for(int i=0; i<vSynObj.size(); i++)
		vSynObj[i] = Syn_Obj_t( ttb2[i] );

	pNtk = new Rev_Ntk_t( width );
	Rev_Ntk_t NtkFront(width), NtkBack(width);
	for(int i=0; i<ttb2.size(); i++){

		clk1 = clock();
		vSynObj_t::iterator ret = SelectFix( vSynObj, vSynObj.begin() + i );
		clk += clock() - clk1;

		bool fForward = ret->isForwardLegal();
		Rev_Ntk_t SubNtk(width);
		Rev_EntrySimpleSyn( &*ret, SubNtk );            // synthesis for one IO pair
		swap( *(vSynObj.begin()+i), *ret );
		if( 0==SubNtk.nLevel() )
			continue;


		for(int j=i; j<vSynObj.size(); j++)
			SubNtk.Apply( fForward? vSynObj[j].second(): vSynObj[j].first () );

		
		if( fForward )
			NtkBack .Append( SubNtk );
		else
			NtkFront.Append( SubNtk );
	}
	pNtk->Append( NtkFront );
	NtkBack.reverse();
	pNtk->Append( NtkBack  );
	cout<<" Select Time = " << clk/CLOCKS_PER_SEC <<endl;
	return pNtk;
}




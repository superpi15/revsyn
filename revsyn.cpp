#include "ttb.hpp"
#include "ntk.hpp"
#include "revsynUtil.hpp"

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
	vector<int> vOrder;

	//for(int i=0; i<Diff.ndata(); i++){
	for(int i=Diff.ndata(); i>=0; i--){
		if( !Diff.val(i) )
			continue;
		if( 1==large.val(i) )
			continue;
		vOrder.push_back(i);
	}
	for(int i=0; i<Diff.ndata(); i++){
		if( !Diff.val(i) )
			continue;
		if( 0==large.val(i) )
			continue;
		vOrder.push_back(i);
	}

	for(int i=0; i<vOrder.size(); i++){
		int idx = vOrder[i];
		Term_t Oper = Prog;
		Oper.set(idx,0);
		Oper = Rev_QcostMinOper( Oper, vVarInfPtr, fixedBegin, fixedEnd );
		
		Rev_Gate_t Gate;
		Gate.setCtrl( Oper );
		Gate.setFlip( idx );
		Ntk.push_back( Gate );
		Prog.flip( idx );

		//cout <<" working on "<< small <<" vs "<< large <<"  \nOper "<< Oper <<" prog="<< Prog << endl;
	}
	assert( Prog == small );
}

static void Rev_EntrySimpleSyn( Syn_Obj_t * pObj, Rev_Ntk_t& Ntk ){
	const Term_t& small = pObj->small();
	const Term_t& large = pObj->large();
	Term_t Prog = large;   // progress
	Term_t Diff = small ^ large;
	vector<int> vOrder;

	//for(int i=0; i<Diff.ndata(); i++){
	for(int i=Diff.ndata(); i>=0; i--){
		if( !Diff.val(i) )
			continue;
		if( 1==large.val(i) )
			continue;
		vOrder.push_back(i);
	}
	for(int i=0; i<Diff.ndata(); i++){
		if( !Diff.val(i) )
			continue;
		if( 0==large.val(i) )
			continue;
		vOrder.push_back(i);
	}

	for(int i=0; i<vOrder.size(); i++){
		int idx = vOrder[i];
		Term_t Oper = Prog;
		Oper.set(idx,0);
		
		Rev_Gate_t Gate;
		Gate.setCtrl( Oper );
		Gate.setFlip( idx );
		Ntk.push_back( Gate );
		Prog.flip( idx );
	}
}

static Rev_Ntk_t * _Rev_GBD( const Rev_Ttb_t& ttb, bool fQGBD ){
	Rev_Ttb_t ttb2(ttb); // create a truth table same as ttb
	Rev_Ntk_t * pNtk;
	vVarInf_t vVarInf;         // for qGBD only 
	vVarInfPtr_t vVarInfPtr;   // for qGBD only 

	pNtk = new Rev_Ntk_t;
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

	Rev_Ntk_t NtkFront, NtkBack;
	for(int i=0; i<vSynObj.size(); i++){

		//pick min-minterm, tie-break by hamdist
		std::sort( vSynObj.begin()+i, vSynObj.end(), Syn_Obj_t::Cmptor_t() );
		
		if( i+1<vSynObj.size() )
			if( vSynObj[i].small()==vSynObj[i+1].small() ){       // check tie-breack condition
				if( vSynObj[i].hamdist()>vSynObj[i+1].hamdist() ) // select smaller hamming distance
					swap( vSynObj[i], vSynObj[i+1] );
			}

		bool fForward = vSynObj[i].isForward();
		Rev_Ntk_t SubNtk;

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

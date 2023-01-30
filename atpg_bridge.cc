#include <iostream>
#include "circuit.h"
#include "GetLongOpt.h"
#include <algorithm>   
using namespace std;

extern GetLongOpt option;

void CIRCUIT::Atpg_bridge()
{
    cout << "Run stuck-at fault ATPG" << endl;
    unsigned i, total_backtrack_num(0), pattern_num(0);
    ATPG_STATUS status;
    Bridging_FAULT* fptr;
    list<Bridging_FAULT*>::iterator fite;
    
    //Prepare the output files
    ofstream OutputStrm;
    if (option.retrieve("output")){
        OutputStrm.open((char*)option.retrieve("output"),ios::out);
        if(!OutputStrm){
              cout << "Unable to open output file: "
                   << option.retrieve("output") << endl;
              cout << "Unsaved output!\n";
              exit(-1);
        }
    }

    if (option.retrieve("output")){
	    for (i = 0;i<PIlist.size();++i) {
		OutputStrm << "PI " << PIlist[i]->GetName() << " ";
	    }
	    OutputStrm << endl;
    }
    for (fite = BFlist.begin(); fite != BFlist.end();++fite) {
        fptr = *fite;
        
        if (fptr->GetStatus() == DETECTED) { continue;}
        //run podem algorithm
        
        status = Bridging_Podem(fptr, total_backtrack_num);
        switch (status) {
            case TRUE:
                fptr->SetStatus(DETECTED);
                ++pattern_num;
                //run fault simulation for fault dropping
                for (i = 0;i < PIlist.size();++i) { 
					ScheduleFanout(PIlist[i]); 
                        if (option.retrieve("output")){ OutputStrm << PIlist[i]->GetValue();}
		}
                if (option.retrieve("output")){ OutputStrm << endl;}
                for (i = PIlist.size();i<Netlist.size();++i) { Netlist[i]->SetValue(X); }
                LogicSim();
                BridgeFaultSim();
                break;
            case CONFLICT:
                fptr->SetStatus(REDUNDANT);
                break;
            case FALSE:
                fptr->SetStatus(ABORT);
                break;
        }
        
    } //end all faults

    //compute fault coverage
    unsigned total_num(0);
    unsigned abort_num(0), redundant_num(0), detected_num(0);
    unsigned eqv_abort_num(0), eqv_redundant_num(0), eqv_detected_num(0);
    for (fite = BFlist.begin();fite!=BFlist.end();++fite) {
        fptr = *fite;
        switch (fptr->GetStatus()) {
            case DETECTED:
                ++eqv_detected_num;
                detected_num += fptr->GetEqvFaultNum();
                break;
            case REDUNDANT:
                ++eqv_redundant_num;
                redundant_num += fptr->GetEqvFaultNum();
                break;
            case ABORT:
                ++eqv_abort_num;
                abort_num += fptr->GetEqvFaultNum();
                break;
            default:
                cerr << "Unknown fault type exists" << endl;
                break;
        }
    }
    total_num = detected_num + abort_num + redundant_num;

    cout.setf(ios::fixed);
    cout.precision(2);
    cout << "---------------------------------------" << endl;
    cout << "Test pattern number = " << pattern_num << endl;
    cout << "Total backtrack number = " << total_backtrack_num << endl;
    cout << "---------------------------------------" << endl;
    cout << "Total fault number = " << total_num << endl;
    cout << "Detected fault number = " << detected_num << endl;
    cout << "Undetected fault number = " << abort_num + redundant_num << endl;
    cout << "Abort fault number = " << abort_num << endl;
    cout << "Redundant fault number = " << redundant_num << endl;
    cout << "---------------------------------------" << endl;
    cout << "Total equivalent fault number = " << BFlist.size() << endl;
    cout << "Equivalent detected fault number = " << eqv_detected_num << endl;
    cout << "Equivalent undetected fault number = " << eqv_abort_num + eqv_redundant_num << endl;
    cout << "Equivalent abort fault number = " << eqv_abort_num << endl;
    cout << "Equivalent redundant fault number = " << eqv_redundant_num << endl;
    cout << "---------------------------------------" << endl;
    cout << "Fault Coverge = " << 100*detected_num/double(total_num) << "%" << endl;
    cout << "Equivalent FC = " << 100*eqv_detected_num/double(BFlist.size()) << "%" << endl;
    cout << "Fault Efficiency = " << 100*detected_num/double(total_num - redundant_num) << "%" << endl;
    cout << "---------------------------------------" << endl;
    return;
}

ATPG_STATUS CIRCUIT::Bridging_Podem(Bridging_FAULT* fptr, unsigned &total_backtrack_num)
{
    unsigned i, backtrack_num(0);
    GATEPTR pi_gptr(0), decision_gptr(0);
    ATPG_STATUS status;

    //set all values as unknown
    for (i = 0;i<Netlist.size();++i) { Netlist[i]->SetValue(X); }
    //mark propagate paths
    
    //MarkPropagateTree(fptr->GetOutputGate());
    //propagate fault free value
    GATEPTR gptr1(fptr->GetNet1());
    GATEPTR gptr2(fptr->GetNet2());
    //store input value
    BRIDGING Btype(fptr->GetBridging());
    
    GATEPTR FGate;
    VALUE FValue;
    //FOR 01CONDITION
    if (Btype == AND){
    	FGate = gptr2;
    	FValue = D;
    }
    else{
    	FGate = gptr1;
    	FValue = B;
    }
    
    status = Bridging_SetUniqueImpliedValue(fptr,false);
    switch (status) {
        case TRUE:
            LogicSim();
            //inject faulty value
            if (Bridging_FaultEvaluate(fptr)) {
                //forward implication
                ScheduleFanout(FGate);
                LogicSim();
            }
            //check if the fault has propagated to PO
            if (!CheckTest()) { status = FALSE; }
            break;
        case CONFLICT:
            status = CONFLICT;
            break;
        case FALSE: break;
    }
    
    
    MarkPropagateTree(FGate);
    while(backtrack_num < BackTrackLimit && status == FALSE) {
        //search possible PI decision
        pi_gptr = Set_Bcondition_01(fptr);
        if (pi_gptr) { //decision found
            ScheduleFanout(pi_gptr);
            //push to decision tree
            GateStack.push_back(pi_gptr);
            decision_gptr = pi_gptr;
        }
        else { //backtrack previous decision
            while (!GateStack.empty() && !pi_gptr) {
                //all decision tried (1 and 0)
                if (decision_gptr->GetFlag(ALL_ASSIGNED)) {
                    decision_gptr->ResetFlag(ALL_ASSIGNED);
                    decision_gptr->SetValue(X);
                    ScheduleFanout(decision_gptr);
                    
                    //remove decision from decision tree
                    GateStack.pop_back();
                    decision_gptr = GateStack.back();
                    
                }
                //inverse current decision value
                else {
                    decision_gptr->InverseValue();
                    
                    ScheduleFanout(decision_gptr);
                    decision_gptr->SetFlag(ALL_ASSIGNED);
                    ++backtrack_num;
                    pi_gptr = decision_gptr;
                }
            }
            //no other decision
            if (!pi_gptr) { 
            	status = CONFLICT; 
            }
        }
        if (pi_gptr) {
            LogicSim();
            //fault injection
            if(Bridging_FaultEvaluate(fptr)) {
                //forward implication
                ScheduleFanout(FGate);/////////
                LogicSim();
            }
            if (CheckTest()) { 
            	status = TRUE;
            }
        }
    } //end while loop
    
    ///FOR 10 CONDITION 
    if (status != TRUE){
    	for (i = 0;i<Netlist.size();++i) { Netlist[i]->SetValue(X); }
    	pi_gptr = 0;
    	decision_gptr = 0;
    	status = FALSE;
    	backtrack_num = 0;
    	if (Btype == AND){
    		FGate = gptr2;
    		FValue = D;
    	}
    	else{
    		FGate = gptr1;
    		FValue = B;
    	}
    	list<GATEPTR>::iterator gite;
    	for (gite = GateStack.begin();gite != GateStack.end();++gite) {
        	(*gite)->ResetFlag(ALL_ASSIGNED);
    	}
    	for (gite = PropagateTree.begin();gite != PropagateTree.end();++gite) {
        	(*gite)->ResetFlag(MARKED);
    	}
    	//clear stacks
    	GateStack.clear(); PropagateTree.clear();
    	
    	status = Bridging_SetUniqueImpliedValue(fptr,true);
    	switch (status) {
        	case TRUE:
            	LogicSim();
            	//inject faulty value
            	if (Bridging_FaultEvaluate(fptr)) {
                	//forward implication
                	ScheduleFanout((FGate));
                	LogicSim();
            	}
            //check if the fault has propagated to PO
            	if (!CheckTest()) { status = FALSE; }
            	break;
        	case CONFLICT:
            	status = CONFLICT;
            	break;
        	case FALSE: break;
    	}
    	
    	MarkPropagateTree(FGate);
    }
    
    while(backtrack_num < BackTrackLimit && status == FALSE) {
        //search possible PI decision
        pi_gptr = Set_Bcondition_10(fptr);
        if (pi_gptr) { //decision found
            ScheduleFanout(pi_gptr);
            //push to decision tree
            GateStack.push_back(pi_gptr);
            decision_gptr = pi_gptr;
        }
        else { //backtrack previous decision
            while (!GateStack.empty() && !pi_gptr) {
                //all decision tried (1 and 0)
                if (decision_gptr->GetFlag(ALL_ASSIGNED)) {
                    decision_gptr->ResetFlag(ALL_ASSIGNED);
                    decision_gptr->SetValue(X);
                    ScheduleFanout(decision_gptr);
                    
                    //remove decision from decision tree
                    GateStack.pop_back();
                    decision_gptr = GateStack.back();
                    
                }
                //inverse current decision value
                else {
                    decision_gptr->InverseValue();
                    
                    ScheduleFanout(decision_gptr);
                    decision_gptr->SetFlag(ALL_ASSIGNED);
                    ++backtrack_num;
                    pi_gptr = decision_gptr;
                }
            }
            //no other decision
            if (!pi_gptr) { 
            	status = CONFLICT; 
            }
        }
        if (pi_gptr) {
            LogicSim();
            //fault injection
            if(Bridging_FaultEvaluate(fptr)) {
                //forward implication
                ScheduleFanout(FGate);/////////
                LogicSim();
            }
            if (CheckTest()) { 
            	status = TRUE;
            }
        }
    } //end while loop
	
    //clean ALL_ASSIGNED and MARKED flags
    list<GATEPTR>::iterator gite;
    for (gite = GateStack.begin();gite != GateStack.end();++gite) {
        (*gite)->ResetFlag(ALL_ASSIGNED);
    }
    for (gite = PropagateTree.begin();gite != PropagateTree.end();++gite) {
        (*gite)->ResetFlag(MARKED);
    }

    //clear stacks
    GateStack.clear(); PropagateTree.clear();
    
    //assign true values to PIs
    if (status ==  TRUE) {
		for (i = 0;i<PIlist.size();++i) {
		    switch (PIlist[i]->GetValue()) {
			case S1: break;
			case S0: break;
			case D: PIlist[i]->SetValue(S1); break;
			case B: PIlist[i]->SetValue(S0); break;
			case X: PIlist[i]->SetValue(VALUE(2.0 * rand()/(RAND_MAX + 1.0))); break;
			default: cerr << "Illigal value" << endl; break;
		    }
		}//end for all PI
    } //end status == TRUE

    total_backtrack_num += backtrack_num;
    return status;
}

ATPG_STATUS CIRCUIT::Bridging_SetUniqueImpliedValue(Bridging_FAULT* fptr, bool type)
{
    register ATPG_STATUS status_1(FALSE), status_2(FALSE);
    GATEPTR gptr1(fptr->GetNet1());
     GATEPTR gptr2(fptr->GetNet2());
    VALUE value1;
    VALUE value2;
    if (type){
    	value1 = S0;
    	value2 = S1;
    }
    else{
    	value1 = S1;
    	value2 = S0;
    }
    //backward implication fault-free value
    switch (BackwardImply(gptr1, value1)) {
        case TRUE: status_1 = TRUE; break;
        case CONFLICT: return CONFLICT; break;
        case FALSE: break;
    }
    switch (BackwardImply(gptr2, value2)) {
        case TRUE: status_2 = TRUE; break;
        case CONFLICT: return CONFLICT; break;
        case FALSE: break;
    }
    if (status_1 == TRUE && status_2 == TRUE){
    	return TRUE;
    }
    else {return FALSE;}
}

//return possible PI decision
GATEPTR CIRCUIT::Set_Bcondition_01(Bridging_FAULT* fptr)
{
	GATEPTR gptr1(fptr->GetNet1());
    GATEPTR gptr2(fptr->GetNet2());
    //store input value
    VALUE bvalue1(gptr1->GetValue());
    VALUE bvalue2(gptr2->GetValue());
    //can not do fault injection
    BRIDGING Btype(fptr->GetBridging());
    
    GATEPTR FGate;
    VALUE FValue;
    
    GATEPTR decision_gptr;
    VALUE decision_value;
    if (Btype == AND){
    	FGate = gptr2;
    	FValue = D;
    }
    else{
    	FGate = gptr1;
    	FValue = B;
    }
    
    if (!FGate->GetFlag(OUTPUT)){
    	if (gptr1->GetValue() != X && gptr2->GetValue() != X){
    		if (FGate->GetValue() != FValue) { return 0; }
            //search D-frontier
            decision_gptr = FindPropagateGate();
            if (!decision_gptr) { return 0;}
            switch (decision_gptr->GetFunction()) {
                case G_AND:
                case G_NOR: decision_value = S1; break;
                case G_NAND:
                case G_OR: decision_value = S0; break;
                default: return 0;
            }
    	}
    	else{
    		if (!TraceUnknownPath(FGate)) { return 0; }
    		if (gptr1->GetValue() != S0){
    			if (gptr2->GetValue() == S1){
    				decision_gptr = gptr1;
    				decision_value = S0;
    			}
    			else if(gptr2->GetValue() == S0){
    				return 0;
    			}
    			else {
    				decision_gptr = gptr1;
    				decision_value = S0;
    			}
    		}
    		else if (gptr2->GetValue() != S1){
    			decision_gptr = gptr2;
    			decision_value = S1;
    		}
    	}
    	///////////////
    }
    else{
    	if (bvalue1 == X) {
            decision_gptr = gptr1;
    		decision_value = S0;
        }
        else if (bvalue1 == S0 && bvalue2 == X){
        	decision_gptr = gptr2;
    		decision_value = S1;
        }
        else{
        	return 0;
        }
    }
	return FindPIAssignment(decision_gptr, decision_value);
}

GATEPTR CIRCUIT::Set_Bcondition_10(Bridging_FAULT* fptr)
{
	GATEPTR gptr1(fptr->GetNet1());
    GATEPTR gptr2(fptr->GetNet2());
    //store input value
    VALUE bvalue1(gptr1->GetValue());
    VALUE bvalue2(gptr2->GetValue());
    //can not do fault injection
    BRIDGING Btype(fptr->GetBridging());
    
    GATEPTR FGate;
    VALUE FValue;
    
    GATEPTR decision_gptr;
    VALUE decision_value;
    if (Btype == AND){
    	FGate = gptr1;
    	FValue = D;
    }
    else{
    	FGate = gptr2;
    	FValue = B;
    }
    
    if (!FGate->GetFlag(OUTPUT)){
    	if (gptr1->GetValue() != X && gptr2->GetValue() != X){
    		if (FGate->GetValue() != FValue) { return 0; }
            //search D-frontier
            decision_gptr = FindPropagateGate();
            if (!decision_gptr) { return 0;}
            switch (decision_gptr->GetFunction()) {
                case G_AND:
                case G_NOR: decision_value = S1; break;
                case G_NAND:
                case G_OR: decision_value = S0; break;
                default: return 0;
            }
    	}
    	else{
    		if (!TraceUnknownPath(FGate)) { return 0; }
    		if (gptr1->GetValue() != S1){
    			if (gptr2->GetValue() == S0){
    				decision_gptr = gptr1;
    				decision_value = S1;
    			}
    			else if(gptr2->GetValue() == S1){
    				return 0;
    			}
    			else {
    				decision_gptr = gptr1;
    				decision_value = S1;
    			}
    		}
    		else if (gptr2->GetValue() != S0){
    			decision_gptr = gptr2;
    			decision_value = S0;
    		}
    	}
    	///////////////
    }
    else{
    	if (bvalue1 == X) {
            decision_gptr = gptr1;
    		decision_value = S1;
        }
        else if (bvalue1 == S1 && bvalue2 == X){
        	decision_gptr = gptr2;
    		decision_value = S0;
        }
        else{
        	return 0;
        }
    }
	return FindPIAssignment(decision_gptr, decision_value);
}


bool CIRCUIT::Bridging_FaultEvaluate(Bridging_FAULT* fptr)
{
    GATEPTR gptr1(fptr->GetNet1());
    GATEPTR gptr2(fptr->GetNet2());
    //store input value
    VALUE bvalue1(gptr1->GetValue());
    VALUE bvalue2(gptr2->GetValue());
    //can not do fault injection
    BRIDGING Btype(fptr->GetBridging());
    //int fselect(0);
    
    if (bvalue1 == bvalue2) { return false; }
    switch(Btype){
    	case AND:
    		if (bvalue1 == S1){
    			gptr1->SetValue(B);
    			//fselect = 1;
    		}
    		else{
    			gptr2->SetValue(B);
    			//fselect = 2;
    		}
    		break;
    	case OR:
    		if (bvalue1 == S0){
    			gptr1->SetValue(D);
    			//fselect = 1;
    		}
    		else{
    			gptr2->SetValue(D);
    			//fselect = 2;
    		}
    		break;
    	default:return false;
    }	

    return true;
}


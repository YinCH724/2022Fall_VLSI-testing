#include <iostream>
#include "circuit.h"
#include "GetLongOpt.h"
#include "ass4.h"
using namespace std;

extern GetLongOpt option;

void CIRCUIT::Faultlist_ASS6(){
	GATE* fanout;
	FAULT* fptr;
	for(unsigned i = 0; i<No_Gate();++i){
		//cout<<Gate(i)->GetName()<<endl;
		if (Gate(i)->GetName()=="net17"){
			fanout = Gate(i)->Fanout(0);
    		fptr = new FAULT(Gate(i), Gate(i), S0);
    		fptr->SetBranch(false);
    		Flist.push_front(fptr);
		}
		else if (Gate(i)->GetName()=="n60"){
			fanout = Gate(i)->Fanout(0);
    		fptr = new FAULT(Gate(i), Gate(i), S1);
    		fptr->SetBranch(false);
    		Flist.push_front(fptr);
		}
	}
	UFlist = Flist;
}


void CIRCUIT::Atpg_ASS6()
{
    cout << "Run stuck-at fault ATPG" << endl;
    unsigned i, total_backtrack_num(0), pattern_num(0);
    ATPG_STATUS status;
    FAULT* fptr;
    list<FAULT*>::iterator fite;
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
    
    //////////////////
    srand(time(NULL));
    double fault_coverage;
    unsigned ramNum; 
	for (ramNum = 0 ; ramNum<PreSimNum ; ++ramNum){
		fault_coverage = Fault_Coverage_ASS6(OutputStrm);
		if (fault_coverage>90){
			cout<<"saturation"<<endl;
			break;
		}
	}
	pattern_num = ramNum;
	cout<<"Random Pattern Fault Coverge = " <<fault_coverage<<"%"<<endl;
	cout<<"random pattern end"<<endl;
	cout<<"random pattern = "<<ramNum<<endl;
    //////////////////
    for (fite = Flist.begin(); fite != Flist.end();++fite) {
        fptr = *fite;
        if (fptr->GetStatus() == DETECTED) { continue; }
        //run podem algorithm
        //
        status = Podem(fptr, total_backtrack_num);
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
                FaultSim();
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
    for (fite = Flist.begin();fite!=Flist.end();++fite) {
        fptr = *fite;
        
        /*cout<<fptr->GetInputGate()->GetName()<<" ";
        cout<<fptr->GetOutputGate()->GetName()<<endl;*/
        
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
    cout << "Total equivalent fault number = " << Flist.size() << endl;
    cout << "Equivalent detected fault number = " << eqv_detected_num << endl;
    cout << "Equivalent undetected fault number = " << eqv_abort_num + eqv_redundant_num << endl;
    cout << "Equivalent abort fault number = " << eqv_abort_num << endl;
    cout << "Equivalent redundant fault number = " << eqv_redundant_num << endl;
    cout << "---------------------------------------" << endl;
    cout << "Fault Coverge = " << 100*detected_num/double(total_num) << "%" << endl;
    cout << "Equivalent FC = " << 100*eqv_detected_num/double(Flist.size()) << "%" << endl;
    cout << "Fault Efficiency = " << 100*detected_num/double(total_num - redundant_num) << "%" << endl;
    cout << "---------------------------------------" << endl;
    return;
}


double CIRCUIT::Fault_Coverage_ASS6(ofstream &File)
{
    double fault_coverage;
    int rand_value;
    //cout << "Run stuck-at fault simulation" << endl;
	////
    
	for (unsigned i =0 ; i<PIlist.size() ; ++i){
		rand_value = int(2*(float(rand())/(RAND_MAX)));
		if (rand_value == 0){
			if (PIlist[i]->GetValue() != S0){
				PIlist[i]->SetFlag(SCHEDULED);
				PIlist[i]->SetValue(S0);
			}
		}
		else if (rand_value == 1){
			if (PIlist[i]->GetValue() != S1){
				PIlist[i]->SetFlag(SCHEDULED);
				PIlist[i]->SetValue(S1);
			}
		}
		File<<rand_value;
	}
	File<<endl;
	
	SchedulePI();
	LogicSim();
    FaultSim();
    //compute fault coverage
    unsigned total_num(0);
    unsigned undetected_num(0), detected_num(0);
    unsigned eqv_undetected_num(0), eqv_detected_num(0);
    FAULT* fptr;
    list<FAULT*>::iterator fite;
    for (fite = Flist.begin();fite!=Flist.end();++fite) {
        fptr = *fite;
        switch (fptr->GetStatus()) {
            case DETECTED:
                ++eqv_detected_num;
                detected_num += fptr->GetEqvFaultNum();
                break;
            default:
                ++eqv_undetected_num;
                undetected_num += fptr->GetEqvFaultNum();
                break;
        }
    }
    total_num = detected_num + undetected_num;
    fault_coverage = 100*detected_num/double(total_num);
    
    return fault_coverage;
}

/*
void CIRCUIT::Atpg_BridgeFault()
{
    cout << "Run Bridge fault ATPG" << endl;
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
        if (fptr->GetStatus() == DETECTED) { continue; }
        //run podem algorithm
        status = Bridge_Podem(fptr, total_backtrack_num);
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
                FaultSim();
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
    cout << "Total equivalent fault number = " << Flist.size() << endl;
    cout << "Equivalent detected fault number = " << eqv_detected_num << endl;
    cout << "Equivalent undetected fault number = " << eqv_abort_num + eqv_redundant_num << endl;
    cout << "Equivalent abort fault number = " << eqv_abort_num << endl;
    cout << "Equivalent redundant fault number = " << eqv_redundant_num << endl;
    cout << "---------------------------------------" << endl;
    cout << "Fault Coverge = " << 100*detected_num/double(total_num) << "%" << endl;
    cout << "Equivalent FC = " << 100*eqv_detected_num/double(Flist.size()) << "%" << endl;
    cout << "Fault Efficiency = " << 100*detected_num/double(total_num - redundant_num) << "%" << endl;
    cout << "---------------------------------------" << endl;
    return;
}

ATPG_STATUS CIRCUIT::Bridge_Podem(Bridging_FAULT* fptr, unsigned &total_backtrack_num)
{
    unsigned i, backtrack_num(0);
    GATEPTR pi_gptr(0), decision_gptr(0);
    ATPG_STATUS status;

    //set all values as unknown
    for (i = 0;i<Netlist.size();++i) { Netlist[i]->SetValue(X); }
    //mark propagate paths
    MarkPropagateTree(fptr->GetOutputGate());
    //propagate fault free value
    status = BridgeSetUniqueImpliedValue(fptr);
    switch (status) {
        case TRUE:
            LogicSim();
            //inject faulty value
            if (FaultEvaluate(fptr)) {
                //forward implication
                ScheduleFanout(fptr->GetOutputGate());
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

    while(backtrack_num < BackTrackLimit && status == FALSE) {
        //search possible PI decision
        pi_gptr = TestPossible(fptr);
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
            if (!pi_gptr) { status = CONFLICT; }
        }
        if (pi_gptr) {
            LogicSim();
            //fault injection
            if(FaultEvaluate(fptr)) {
                //forward implication
                ScheduleFanout(fptr->GetOutputGate());
                LogicSim();
            }
            if (CheckTest()) { status = TRUE; }
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

ATPG_STATUS CIRCUIT::BridgeSetUniqueImpliedValue(Bridge_FAULT* fptr, VALUE value)
{
    register ATPG_STATUS status(FALSE);
    GATEPTR gptr1(fptr->GetNet1());
    GATEPTR gptr2(fptr->GetNet2());
    
    BRIDGING bridge;
    
    switch (BackwardImply(gptr1, value)) {
        case TRUE: status = TRUE; break;
        case CONFLICT: return CONFLICT; break;
        case FALSE: break;
    }
	
    
    if (!fptr->Is_Branch()) { return status; }
    //if branch, need to check side inputs of the output gate
    GATEPTR ogptr(fptr->GetOutputGate());
    VALUE ncv(NCV[ogptr->GetFunction()]);
    //set side inputs as non-controlling value
    for (unsigned i = 0;i < ogptr->No_Fanin();++i) {
        if (igptr == ogptr->Fanin(i)) { continue; }
        switch (BackwardImply(ogptr->Fanin(i), ncv)) {
            case TRUE: status = TRUE; break;
            case CONFLICT: return CONFLICT; break;
            case FALSE: break;
        }
    }
    return status;
}
*/  

#include <iostream>
#include "circuit.h"
#include "GetLongOpt.h"
#include "ass4.h"
using namespace std;

extern GetLongOpt option;


void CIRCUIT::BridgeFaultSimVectors()
{
    cout << "Run Bridge fault simulation" << endl;
    unsigned pattern_num(0);
    if(!Pattern.eof()){ // Readin the first vector
        while(!Pattern.eof()){
            ++pattern_num;
            Pattern.ReadNextPattern();
            //fault-free simulation
            SchedulePI();
            LogicSim();
            //single pattern parallel fault simulation
            BridgeFaultSim();
        }
    }

    //compute fault coverage
    unsigned total_num(0);
    unsigned undetected_num(0), detected_num(0);
    unsigned eqv_undetected_num(0), eqv_detected_num(0);
    Bridging_FAULT* fptr;
    list<Bridging_FAULT*>::iterator fite;
    for (fite = BFlist.begin();fite!=BFlist.end();++fite) {
        fptr = *fite;
        /*
        cout<< fptr->GetNet1()->GetName()<<" ";
        cout<< fptr->GetNet2()->GetName()<<" ";
        if(fptr->GetBridging()==AND)
		cout<<"AND"<<endl;
	else if(fptr->GetBridging()==OR)
		cout<<"OR"<<endl;
	*/
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
    cout.setf(ios::fixed);
    cout.precision(2);
    cout << "---------------------------------------" << endl;
    cout << "Test pattern number = " << pattern_num << endl;
    cout << "---------------------------------------" << endl;
    cout << "Total fault number = " << total_num << endl;
    cout << "Detected fault number = " << detected_num << endl;
    cout << "Undetected fault number = " << undetected_num << endl;
    cout << "---------------------------------------" << endl;
    cout << "Equivalent fault number = " << BFlist.size() << endl;
    cout << "Equivalent detected fault number = " << eqv_detected_num << endl; 
    cout << "Equivalent undetected fault number = " << eqv_undetected_num << endl; 
    cout << "---------------------------------------" << endl;
    cout << "Fault Coverge = " << 100*detected_num/double(total_num) << "%" << endl;
    cout << "Equivalent FC = " << 100*eqv_detected_num/double(BFlist.size()) << "%" << endl;
    cout << "---------------------------------------" << endl;
    return;
}


void CIRCUIT::BridgeFaultSim()
{
    register unsigned i, fault_idx(0);
    GATEPTR gptr, gptr1,gptr2;
    Bridging_FAULT *fptr;
    Bridging_FAULT *simulate_flist[PatternNum];
    list<GATEPTR>::iterator gite;
    //initial all gates
    for (i = 0; i < Netlist.size(); ++i) {
        Netlist[i]->SetFaultFreeValue();
    }

    //for all undetected faults
    list<Bridging_FAULT*>::iterator fite;
    for (fite = UBFlist.begin();fite!=UBFlist.end();++fite) {
        fptr = *fite;
        //skip redundant and detected faults
        if (fptr->GetStatus() == REDUNDANT || fptr->GetStatus() == DETECTED) { continue; }
        //the fault is not active
        if (fptr->GetNet2()->GetValue() == fptr->GetNet1()->GetValue()) { continue; }
        //the fault can be directly seen
        gptr1 = fptr->GetNet1();
        gptr2 = fptr->GetNet2();
        if ((gptr1->GetFlag(OUTPUT)||gptr2->GetFlag(OUTPUT) )) {
            fptr->SetStatus(DETECTED);
            continue;
        }
        
        if (!gptr1->GetFlag(FAULTY)&&!gptr2->GetFlag(FAULTY)) {
            gptr1->SetFlag(FAULTY); GateStack.push_front(gptr1);
            gptr2->SetFlag(FAULTY); GateStack.push_front(gptr2);
        }
        InjectBFaultValue(gptr1, gptr2, fault_idx, fptr->GetBridging());
        gptr1->SetFlag(FAULT_INJECTED);
        gptr2->SetFlag(FAULT_INJECTED);
        ScheduleFanout(gptr1);
        ScheduleFanout(gptr2);
        simulate_flist[fault_idx++] = fptr;
       


        //collect PatternNum fault, do fault simulation
        if (fault_idx == PatternNum) {
            //do parallel fault simulation
            for (i = 0;i<= MaxLevel; ++i) {
                while (!Queue[i].empty()) {
                    gptr = Queue[i].front(); Queue[i].pop_front();
                    gptr->ResetFlag(SCHEDULED);
                    FaultSimEvaluate(gptr);
                }
            }

            // check detection and reset wires' faulty values
            // back to fault-free values
            for (gite = GateStack.begin();gite != GateStack.end();++gite) {
                gptr = *gite;
                //clean flags
                gptr->ResetFlag(FAULTY);
                gptr->ResetFlag(FAULT_INJECTED);
                gptr->ResetFaultFlag();
                if (gptr->GetFlag(OUTPUT)) {
                    for (i = 0; i < fault_idx; ++i) {
                        if (simulate_flist[i]->GetStatus() == DETECTED) { continue; }
                        //faulty value != fault-free value && fault-free != X &&
                        //faulty value != X (WireValue1[i] == WireValue2[i])
                        if (gptr->GetValue() != VALUE(gptr->GetValue1(i)) && gptr->GetValue() != X 
                                && gptr->GetValue1(i) == gptr->GetValue2(i)) {
                            simulate_flist[i]->SetStatus(DETECTED);
                        }
                    }
                }
                gptr->SetFaultFreeValue();    
            } //end for GateStack
            GateStack.clear();
            fault_idx = 0;
        } //end fault simulation
    } //end for all undetected faults

    //fault sim rest faults
    if (fault_idx) {
        //do parallel fault simulation
        for (i = 0;i<= MaxLevel; ++i) {
            while (!Queue[i].empty()) {
                gptr = Queue[i].front(); Queue[i].pop_front();
                gptr->ResetFlag(SCHEDULED);
                FaultSimEvaluate(gptr);
            }
        }

        // check detection and reset wires' faulty values
        // back to fault-free values
        for (gite = GateStack.begin();gite != GateStack.end();++gite) {
            gptr = *gite;
            //clean flags
            gptr->ResetFlag(FAULTY);
            gptr->ResetFlag(FAULT_INJECTED);
            gptr->ResetFaultFlag();
            if (gptr->GetFlag(OUTPUT)) {
                for (i = 0; i < fault_idx; ++i) {
                    if (simulate_flist[i]->GetStatus() == DETECTED) { continue; }
                    //faulty value != fault-free value && fault-free != X &&
                    //faulty value != X (WireValue1[i] == WireValue2[i])
                    if (gptr->GetValue() != VALUE(gptr->GetValue1(i)) && gptr->GetValue() != X 
                            && gptr->GetValue1(i) == gptr->GetValue2(i)) {
                        simulate_flist[i]->SetStatus(DETECTED);
                    }
                }
            }
            gptr->SetFaultFreeValue();    
        } //end for GateStack
        GateStack.clear();
        fault_idx = 0;
    } //end fault simulation

    // remove detected faults
    for (fite = UBFlist.begin();fite != UBFlist.end();) {
        fptr = *fite;
        if (fptr->GetStatus() == DETECTED || fptr->GetStatus() == REDUNDANT) {
            fite = UBFlist.erase(fite);
        }
        else { ++fite; }
    }
    return;
}


void CIRCUIT::InjectBFaultValue(GATEPTR gptr1, GATEPTR gptr2, unsigned idx, BRIDGING bridge)
{
    if (bridge == AND) {
        gptr1->ResetValue1(idx);
        gptr1->ResetValue2(idx);
        gptr2->ResetValue1(idx);
        gptr2->ResetValue2(idx);
    }
    else if (bridge == OR) {
        gptr1->SetValue1(idx);
        gptr1->SetValue2(idx);
        gptr2->SetValue1(idx);
        gptr2->SetValue2(idx);
    }
    gptr1->SetFaultFlag(idx);
    gptr1->SetFaultFlag(idx);
    return;
}





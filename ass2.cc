#include <iostream>
#include "gate.h"
#include "circuit.h"
#include "ReadPattern.h"
#include "GetLongOpt.h"
using namespace std;

extern GetLongOpt option;

void CIRCUIT::LogicSimVectors_CPU()
{
    cout << "Run logic simulation" << endl;
    //read test patterns
    ofstream file("result_ass2.output");
    while (!Pattern.eof()) {
        Pattern.ReadNextPattern();
        SchedulePI();
        LogicSim_CPU();
        
        file << "PI: ";
        for (unsigned i = 0;i<No_PI();++i) { file << PIGate(i)->GetValue(); }
		file << " PO: ";
		for (unsigned i = 0;i<No_PO();++i) { file << POGate(i)->GetValue(); }
		file << endl;
		
        PrintIO();
    }
    file.close();
    return;
}
void CIRCUIT::LogicSim_CPU()
{
    GATE* gptr;
    VALUE new_value;
    for (unsigned i = 0;i <= MaxLevel;i++) {
        while (!Queue[i].empty()) {
            gptr = Queue[i].front();
            Queue[i].pop_front();
            gptr->ResetFlag(SCHEDULED);
            new_value = Evaluate_CPU(gptr);
            if (new_value != gptr->GetValue()) {
                gptr->SetValue(new_value);
                ScheduleFanout(gptr);
            }
        }
    }
    return;
}

VALUE CIRCUIT::Evaluate_CPU(GATEPTR gptr)
{
	int val;
	int FI;
    GATEFUNC fun(gptr->GetFunction());
    VALUE value(gptr->Fanin(0)->GetValue());
    val = value;
    if (val==1){
		val = 3;
	}
    switch (fun) {
        case G_AND:
        case G_NAND:
            for (unsigned i = 1;i<gptr->No_Fanin() ;++i) {
				FI = gptr->Fanin(i)->GetValue();
				if (FI==1){
					FI = 3;
				}
				val = val & FI;
            }
            break;
        case G_OR:
        case G_NOR:
            for (unsigned i = 1;i<gptr->No_Fanin() ;++i) {
            	FI = gptr->Fanin(i)->GetValue();
            	if (FI==1){
					FI = 3;
				}
				val = val | FI;
            }
            break;
        default: break;
    }
    //NAND, NOR and NOT
    if (gptr->Is_Inversion()) { val=~val; }
    	val = val & 3;
    
    if(val == 0){
    	value = S0;
    }
    else if(val == 3){
    	value = S1;
    }
    else{
    	value = X;
    }
    
    return value;
}





#include <iostream>
#include "circuit.h"
#include "GetLongOpt.h"
using namespace std;

extern GetLongOpt option;

void CIRCUIT::FaultList(){
	cout << "Generate check point" << endl;
	register unsigned i, j;
	GATEFUNC fun;
	GATEPTR gptr, fanout;
	TFAULT *fptr;
	for(unsigned i=1; i<No_Gate();++i){
		gptr = Netlist[i]; 
		fun = gptr->GetFunction();
        	if (fun == G_PO) { continue; } 
        	
        	fptr = new TFAULT(gptr, gptr, S0);
        	TFlist.push_front(fptr);
        	fptr = new TFAULT(gptr, gptr, S1);
        	TFlist.push_front(fptr);
	}


}

#include <iostream>
#include "circuit.h"
#include "GetLongOpt.h"

using namespace std;

extern GetLongOpt option;
void CIRCUIT::ASS0_Print(){
	GATE *gptr;
	GATEFUNC GateType;
	string GName;
	
	unsigned NoInv = 0;
	unsigned NoNand = 0;
	unsigned NoNor= 0;
	unsigned NoAnd = 0;
	unsigned NoOr = 0;
	unsigned NoDFF = 0;
	
	unsigned TotalGate = 0;
	unsigned NumFO = 0;
	unsigned TotalFO =0;
	
	unsigned TotalNet = 0;
	unsigned BranchNet = 0;
	unsigned StemNet = 0;
	
	float AvgFO = 0;
	cout<<"-----------------------------------"<<endl;
	cout<<"Number of input = "<<No_PI()<<endl;
	cout<<"Number of output = "<<No_PO()<<endl;
	cout<<"-----------------------------------"<<endl;
	
	for (unsigned i=0;i<No_Gate();i++){
        	gptr = Gate(i);
        	GateType=gptr->GetFunction();
        	NumFO=gptr->No_Fanout();
        	TotalFO=TotalFO+NumFO;
        	if (GateType==G_NAND){
        		NoNand=NoNand+1;
        	}
        	else if (GateType==G_NOT){
        		NoInv=NoInv+1;
        	}
        	else if (GateType==G_AND){
        		NoAnd=NoAnd+1;
        	}
        	else if (GateType==G_NOR){
        		NoNor=NoNor+1;
        	}
        	else if (GateType==G_OR){
        		NoOr=NoOr+1;
        	}
        	else if (GateType==G_PPI){
        		NoDFF=NoDFF+1;
        	}
        	else if (GateType==G_DFF){
        		NoDFF=NoDFF+1;
        	}
        	
        	if (gptr->No_Fanout()==1){
        		StemNet=StemNet+1;
        	}
        	else if (gptr->No_Fanout()>1){
        		BranchNet=BranchNet+1;
        		BranchNet=BranchNet+(gptr->No_Fanout());	
        	}
	}
	TotalNet=StemNet+BranchNet;
	TotalGate=NoNor+NoOr+NoNand+NoAnd+NoInv;
	
	cout<<"Total number of gates ="<<TotalGate<<endl;
	cout<<"Number of Inverter = "<<NoInv<<endl;
	cout<<"Number of And = "<<NoAnd<<endl;
	cout<<"Number of Nand = "<<NoNand<<endl;
	cout<<"Number of Or = "<<NoOr<<endl;
	cout<<"Number of Nor = "<<NoNor<<endl;
	cout<<"-----------------------------------"<<endl;
	cout<<"Number of DFF = "<<NoDFF<<endl;
	cout<<"-----------------------------------"<<endl;
	AvgFO=float(TotalFO)/float(TotalGate);
	cout<<"Total number of net ="<<TotalNet<<endl;
	cout<<"Number of Branchnet ="<<BranchNet<<endl;
	cout<<"Number of Stemnet ="<<StemNet<<endl;
	cout<<"Average Fanout = "<<AvgFO<<endl;
	cout<<"-----------------------------------"<<endl;
}

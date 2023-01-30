#ifndef ASS4_H
#define ASS4_H
#include "gate.h"

class Bridging_FAULT
{
	private:
        BRIDGING Bridging;
        GATE* Net1;
        GATE* Net2;
        unsigned level;
        bool Branch;
        unsigned EqvFaultNum;
        FAULT_STATUS Status;
   public:
   		Bridging_FAULT(GATE* gptr, GATE* ogptr, BRIDGING btype, unsigned L): Bridging(btype), Net1(gptr),Net2(ogptr),level(L) ,Branch(false), EqvFaultNum(1), Status(UNKNOWN) {}
        BRIDGING GetBridging() { return Bridging; }
        GATE* GetNet1() { return Net1; }
        GATE* GetNet2() { return Net2; }
        unsigned GetLevel() { return level;}
        void SetBranch(bool b) { Branch = b; }
        bool Is_Branch() { return Branch; }
        void SetEqvFaultNum(unsigned n) { EqvFaultNum = n; }
        void IncEqvFaultNum() { ++EqvFaultNum; }
        unsigned GetEqvFaultNum() { return EqvFaultNum; }
        void SetStatus(FAULT_STATUS status) { Status = status; }
        FAULT_STATUS GetStatus() { return Status; }
   
	
};



#endif

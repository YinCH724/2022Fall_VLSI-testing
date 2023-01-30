#include <iostream>
#include "gate.h"
#include "circuit.h"
#include "ReadPattern.h"
#include "GetLongOpt.h"
#include <bitset>
using namespace std;

extern GetLongOpt option;

int ASS3_counter = 0;

int BitsToInt(bitset<PatternNum> bits){
	int power = 2;
	int Intform = bits[0];
	for (unsigned i = 1; i<PatternNum ; ++i){
		Intform = Intform + bits[i]*power;
		power *= 2;
	}
	return Intform;
}
void ass3_DFS(GATE* gptr,ofstream &File , vector<bool> &visited){
	GATE* FIgptr;
	string FIname, name;
	int FIid, id;
	GATEFUNC Func;
	 
	id = gptr->GetID();
	name = gptr->GetName();
	visited[id] = TRUE;
	Func = gptr->GetFunction();
	for (unsigned i = 0; i<gptr->No_Fanin();++i){
		FIgptr = gptr->Fanin(i);
		FIid = FIgptr->GetID();
		if(!visited[FIid]){
			ass3_DFS(FIgptr,File,visited);
		}
	}
	
	if (gptr->No_Fanin() > 0){
		FIgptr = gptr->Fanin(0);
		FIname = FIgptr->GetName();
    	File<<char(9)<<"G_"<<name<<"[0] = "<<"G_"<<FIname<<"[0];"<<endl;
    	File<<char(9)<<"G_"<<name<<"[1] = "<<"G_"<<FIname<<"[1];"<<endl;
    	switch (Func) {
        	case G_AND:
        	case G_NAND:
            	for (unsigned i = 1;i<gptr->No_Fanin() ;++i) {
            		FIgptr = gptr->Fanin(i);
					FIname = FIgptr->GetName();
					File<<char(9)<<"G_"<<name<<"[0] &= "<<"G_"<<FIname<<"[0];"<<endl;
    				File<<char(9)<<"G_"<<name<<"[1] &= "<<"G_"<<FIname<<"[1];"<<endl;
            	}
            	break;
        	case G_OR:
        	case G_NOR:
            	for (unsigned i = 1;i<gptr->No_Fanin() ;++i) {
            		FIgptr = gptr->Fanin(i);
					FIname = FIgptr->GetName();
            		File<<char(9)<<"G_"<<name<<"[0] |= "<<"G_"<<FIname<<"[0];"<<endl;
    				File<<char(9)<<"G_"<<name<<"[1] |= "<<"G_"<<FIname<<"[1];"<<endl;
            	}
            	break;
        	default: break;
    	}
    }
    
    if (gptr->Is_Inversion()){
    	File<<char(9)<<"temp = "<<"G_"<<name<<"[0];"<<endl;
    	File<<char(9)<<"G_"<<name<<"[0] = ~"<<"G_"<<name<<"[1];"<<endl;
    	File<<char(9)<<"G_"<<name<<"[1] = "<<"~temp;"<<endl;
    }
}

void CIRCUIT::CreateLogicSim(){
	GATE* gptr;
	string GateName;
	//const unsigned PatternNum = 16;
	bitset<PatternNum> BitCov;
	
	
	cout<<"start generate simulator code"<<endl;
	const char *In = option.retrieve("input"); 
	const char *Out = option.retrieve("simulator"); 
	
	int current = 0;
	string InputFile(In);
	string CCFile;
	string OutputFile;
	int DotPos = InputFile.find_first_of(".",current);
	/*	
	CCFile = InputFile.substr(current,DotPos-current);
	CCFile.append(".cc");
	*/
	OutputFile = InputFile.substr(current,DotPos-current);
	OutputFile.append(".out");
	
	
	ofstream File(Out);
	
	File<<"#include <iostream>"<<endl;
	File<<"#include <ctime>"<<endl;
	File<<"#include <bitset>"<<endl;
	File<<"#include <string>"<<endl;
	File<<"#include <fstream>"<<endl;
	
	File<<"using namespace std;"<<endl;
	
	File<<"const unsigned PatternNum = "<<PatternNum<<";"<<endl;
	File<<"void evaluate();"<<endl;
	File<<"void printIO(unsigned idx);"<<endl<<endl;
	File<<"bitset<PatternNum> temp;"<<endl;

	for (unsigned i = 0; i<No_Gate() ; ++i){
		File<<"bitset<PatternNum> ";
		gptr = Gate(i);
		GateName = gptr->GetName();
		File<<"G_"<<GateName<<"[2];"<<endl;
	}
	
	File<<"ofstream fout("<<char(34)<<OutputFile<<char(34)<<",ios::out);"<<endl;
	File<<endl;
	File<<endl;
	File<<"int main(){"<<endl;
	File<<char(9)<<"clock_t time_init, time_end;"<<endl;
	File<<char(9)<<"time_init = clock();"<<endl;
	File<<endl;
	
	unsigned pattern_num(0);
    unsigned pattern_idx(0);
    
	while(!Pattern.eof()){ 
		pattern_num = 0;
		for(pattern_idx=0; pattern_idx<PatternNum; pattern_idx++){
	    	if(!Pattern.eof()){ 
	        	++pattern_num;
	       	 	Pattern.ReadNextPattern(pattern_idx);
	   		}
	    	else break;
		}
		for (unsigned i = 0; i<No_PI() ; ++i){
			gptr = PIGate(i);
			GateName = gptr->GetName();
			int intValue;
			BitCov = gptr->GetWireValue(0);
			intValue = BitsToInt(BitCov);
			File<<char(9)<<"G_"<<GateName<<"[0] = "<<intValue<<";"<<endl;
			BitCov = gptr->GetWireValue(1);
			intValue = BitsToInt(BitCov);
			File<<char(9)<<"G_"<<GateName<<"[1] = "<<intValue<<";"<<endl;
		}
		File<<endl;
		File<<char(9)<<"evaluate();"<<endl;
		File<<char(9)<<"printIO("<< pattern_num <<");"<<endl;
		File<<endl;
		File<<endl;
	}
	
	
	File<<char(9)<<"time_end = clock();"<<endl;
	File<<char(9)<<"cout << "<<char(34)<<"Total CPU Time ="<<char(34)<<"<< double(time_end - time_init)/CLOCKS_PER_SEC << endl;"<<endl;
	File<<char(9)<<"system("<<char(34)<<"ps aux | grep a.out"<<char(34)<<");"<<endl;
	File<<char(9)<<"return 0;"<<endl;
	File<<"}"<<endl;
	
	File<<"void evaluate(){"<<endl;
	vector<bool> visited(No_Gate());
	
	for(unsigned i = 0; i<No_PI(); ++i){
		gptr = PIGate(i);
		int PI_id = gptr->GetID();
		visited[PI_id] = TRUE;
	}
	
	for(unsigned i = 0; i<No_PO(); ++i){
		gptr = POGate(i);
		ass3_DFS(gptr,File,visited);
	}
	
	File<<char(9)<<endl;
	
	File<<"}"<<endl;
	
	File<<"void printIO(unsigned idx){"<<endl;
	File<<char(9)<<"for (unsigned j=0; j<idx; j++){"<<endl;
	for (unsigned i = 0; i<No_PI(); ++i){
		gptr = PIGate(i);
		File<<char(9)<<char(9)<<"if ("<<"G_"<<gptr->GetName()<<"[0][j]==0){"<<endl;
		
		File<<char(9)<<char(9)<<char(9)<<"if ("<<"G_"<<gptr->GetName()<<"[1][j]==1)"<<endl;
		File<<char(9)<<char(9)<<char(9)<<char(9)<<"fout<<"<<char(34)<<"F"<<char(34)<<";"<<endl;
		File<<char(9)<<char(9)<<char(9)<<"else"<<endl;
		File<<char(9)<<char(9)<<char(9)<<char(9)<<"fout<<"<<char(34)<<"0"<<char(34)<<";"<<endl;
		
		File<<char(9)<<char(9)<<"}"<<endl;
		
		File<<char(9)<<char(9)<<"else{"<<endl;
		
		File<<char(9)<<char(9)<<char(9)<<"if ("<<"G_"<<gptr->GetName()<<"[1][j]==1)"<<endl;
		File<<char(9)<<char(9)<<char(9)<<char(9)<<"fout<<"<<char(34)<<"1"<<char(34)<<";"<<endl;
		File<<char(9)<<char(9)<<char(9)<<"else"<<endl;
		File<<char(9)<<char(9)<<char(9)<<char(9)<<"fout<<"<<char(34)<<"2"<<char(34)<<";"<<endl;
		
		File<<char(9)<<char(9)<<"}"<<endl;
		
	}
	File<<char(9)<<char(9)<<"fout<<"<<char(34)<<" "<<char(34)<<";"<<endl;
	
	for (unsigned i = 0; i<No_PO(); ++i){
		gptr = POGate(i);
		File<<char(9)<<char(9)<<"if ("<<"G_"<<gptr->GetName()<<"[0][j]==0){"<<endl;
		
		File<<char(9)<<char(9)<<char(9)<<"if ("<<"G_"<<gptr->GetName()<<"[1][j]==1)"<<endl;
		File<<char(9)<<char(9)<<char(9)<<char(9)<<"fout<<"<<char(34)<<"F"<<char(34)<<";"<<endl;
		File<<char(9)<<char(9)<<char(9)<<"else"<<endl;
		File<<char(9)<<char(9)<<char(9)<<char(9)<<"fout<<"<<char(34)<<"0"<<char(34)<<";"<<endl;
		
		File<<char(9)<<char(9)<<"}"<<endl;
		
		File<<char(9)<<char(9)<<"else{"<<endl;
		
		File<<char(9)<<char(9)<<char(9)<<"if ("<<"G_"<<gptr->GetName()<<"[1][j]==1)"<<endl;
		File<<char(9)<<char(9)<<char(9)<<char(9)<<"fout<<"<<char(34)<<"1"<<char(34)<<";"<<endl;
		File<<char(9)<<char(9)<<char(9)<<"else"<<endl;
		File<<char(9)<<char(9)<<char(9)<<char(9)<<"fout<<"<<char(34)<<"2"<<char(34)<<";"<<endl;
		
		File<<char(9)<<char(9)<<"}"<<endl;
	
	}
	File<<char(9)<<char(9)<<"fout<<"<<"endl"<<";"<<endl;
	File<<char(9)<<"}"<<endl;
	File<<"}"<<endl;
	File.close();
}











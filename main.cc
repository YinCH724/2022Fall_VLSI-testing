#include <iostream>
#include <ctime>
#include "circuit.h"
#include "GetLongOpt.h"
#include "ReadPattern.h"
#include<sys/types.h>
#include<unistd.h>
using namespace std;

// All defined in readcircuit.l
extern char* yytext;
extern FILE *yyin;
extern CIRCUIT Circuit;
extern int yyparse (void);
extern bool ParseError;
extern void Interactive();

GetLongOpt option;



int SetupOption(int argc, char ** argv)
{
    option.usage("[options] input_circuit_file");
    option.enroll("help", GetLongOpt::NoValue,
            "print this help summary", 0);
    option.enroll("logicsim", GetLongOpt::NoValue,
            "run logic simulation", 0);
    option.enroll("plogicsim", GetLongOpt::NoValue,
            "run parallel logic simulation", 0);
    option.enroll("fsim", GetLongOpt::NoValue,
            "run stuck-at fault simulation", 0);
    option.enroll("stfsim", GetLongOpt::NoValue,
            "run single pattern single transition-fault simulation", 0);
    option.enroll("transition", GetLongOpt::NoValue,
            "run transition-fault ATPG", 0);
    option.enroll("input", GetLongOpt::MandatoryValue,
            "set the input pattern file", 0);
    option.enroll("output", GetLongOpt::MandatoryValue,
            "set the output pattern file", 0);
    option.enroll("bt", GetLongOpt::OptionalValue,
            "set the backtrack limit", 0);
            
    //----------------------------------------------------
    option.enroll("ass0",GetLongOpt::NoValue,
    	    "Assigment0 310512050 -09/14",0);

	//----------------------------------------------------
    option.enroll("path",GetLongOpt::NoValue,
    	    "Assigment1 310512050 -10/06",0);
	option.enroll("start",GetLongOpt::MandatoryValue,
    	    "Assigment1 input 310512050 -10/06",0);
	option.enroll("end",GetLongOpt::MandatoryValue,
    	    "Assigment1 output 310512050 -10/06",0);
    	    
    //----------------------------------------------------     
    option.enroll("pattern",GetLongOpt::NoValue,
    	    "Assigment2 2-a 310512050 -10/06",0);
    option.enroll("num",GetLongOpt::MandatoryValue,
    	    "Assigment2 2-a 310512050 -10/06",0);
   	option.enroll("unknown",GetLongOpt::NoValue,
    	    "Assigment2 2-a 310512050 -10/06",0);
	option.enroll("mod_logicsim",GetLongOpt::NoValue,
    	    "Assigment2 2-b 310512050 -10/06",0);
    	    
   	
    //-----------------------------------------------------
    option.enroll("simulator",GetLongOpt::MandatoryValue,
    	    "Assigment2 3 -10/06",0);
    	    
    //-----------------------------------------------------
    option.enroll("check_point",GetLongOpt::NoValue,
    	    "Assigment4 a -11/20",0);
    option.enroll("bridging",GetLongOpt::NoValue,
    	    "Assigment4 b -11/20",0);
    	    
    option.enroll("bridging_fsim",GetLongOpt::NoValue,
    	    "Assigment5 1-c -12/1",0);
	option.enroll("CheckPoint_fsim",GetLongOpt::NoValue,
    	    "Assigment5 1-a -12/1",0);
    	    
    option.enroll("random_pattern",GetLongOpt::NoValue,
    	    "Assigment6 1-d -2023/01/06",0);
	option.enroll("ASS6_test",GetLongOpt::NoValue,
    	    "Assigment6 1-d -2023/01/06",0);
   	option.enroll("bridging_atpg",GetLongOpt::NoValue,
    	    "Assigment6 1-d -2023/01/06",0);
	
    	    
    int optind = option.parse(argc, argv);
    if ( optind < 1 ) { exit(0); }
    if ( option.retrieve("help") ) {
        option.usage();
        exit(0);
    }
    return optind;
}


// ass1
void checkIO(const char* start, const char* end){
	if (!start || !end){
		cout<<"no input!";
		exit(-1);
	}
}

GATE* FindPI(string str_SGate){
	GATE* gptr;
	for (unsigned i=0; i<Circuit.No_PI() ;++i){
		gptr = Circuit.PIGate(i);
		if (gptr->GetName() == str_SGate){
			cout<<"find out start gate!"<<endl;
			return gptr;
		}
	}
	cout<<"can't find start gate"<<endl;
	exit(-1);
}

GATE* FindPO(string str_EGate){
	GATE* gptr;
	for (unsigned i=0; i<Circuit.No_PO() ;++i){
		gptr = Circuit.POGate(i);
		if (gptr->GetName() == str_EGate){
			cout<<"find out end gate!"<<endl;
			return gptr;
		}
	}
	cout<<"can't find end gate"<<endl;
	exit(-1);
}

void DFS(GATE* SGate, GATE* EGate, vector< vector < vector < string > > > &Path, vector<bool> &visited){
	GATE* gptr;
	string CurName;
	string NextName;
	int id;
	int Nid;
	
	id = SGate->GetID();
	CurName = SGate->GetName();
	vector<string> arry1d;
	vector<vector<string> > arry2d;	
	visited[id]=true;
	
	for (unsigned i=0; i<(SGate->No_Fanout()) ; ++i){
		gptr = SGate->Fanout(i);
		Nid = gptr->GetID();
		NextName = gptr->GetName();
		if (gptr==EGate){
			arry1d.push_back(NextName);
			arry2d.push_back(arry1d);
			Path[id]=arry2d;
			return;
		}
		else if(!visited[Nid]){
			DFS(gptr,EGate,Path,visited);
		}
		Path[id].insert(Path[id].end(),Path[Nid].begin(),Path[Nid].end());
	}
	
	for (size_t i = 0; i<Path[id].size();++i){
		Path[id][i].push_back(CurName);
	}
}

int string2int(string ss){
	int L = ss.length();
	char ch[L+1];
	strcpy(ch,ss.c_str());
	int Num;
	int temp;
	
	Num=0;
	temp = 0;
	for (int i=0 ; i<L ; ++i){
		temp=ch[i]-48;
		for (int j=0 ; j<L-i-1 ;++j){
			temp = temp*10;
		}
		Num = Num+temp;
	}
	return Num;
}

void CreatePattern(){
	GATE* gptr;
	string GName;
	const char *Pat_N = option.retrieve("num"); 
    const char *File_N = option.retrieve("output");
    string NumP(Pat_N);
    if (File_N==NULL){
    	cout<<"*****NO OUPUT DIRCECT*****"<<endl;
    	exit(-1);
    }
    
    ofstream file(option.retrieve("output"));
	
	cout<<"start create"<<endl;
	
	for (unsigned i = 0; i<Circuit.No_PI();++i){
		file<<"PI ";
		gptr = Circuit.PIGate(i);
		GName = gptr->GetName();
		file<<GName<<" ";
	}
	file<<endl;

	srand(time(NULL));
	for (int i=0; i<string2int(NumP) ;++i){
		for (unsigned j =0 ;j<Circuit.No_PI() ; ++j){
			file<<int(2*(float(rand())/(RAND_MAX)))<<" ";
		}
		file<<endl;
	}
	file.close();
}

void CreatePattern_Unknown(){
	GATE* gptr;
	string GName;
	int state;
	const char *Pat_N = option.retrieve("num"); 
    const char *File_N = option.retrieve("output");
    string NumP(Pat_N);
    if (File_N==NULL){
    	cout<<"*****NO OUPUT DIRCECT*****"<<endl;
    	exit(-1);
    }
    
    ofstream file(option.retrieve("output"));
	
	cout<<"start create"<<endl;
	
	for (unsigned i = 0; i<Circuit.No_PI();++i){
		file<<"PI ";
		gptr = Circuit.PIGate(i);
		GName = gptr->GetName();
		file<<GName<<" ";
	}
	file<<endl;

	srand(time(NULL));
	for (int i=0; i<string2int(NumP) ;++i){
		for (unsigned j =0 ;j<Circuit.No_PI() ; ++j){
			state = int(3*(float(rand())/(RAND_MAX)));
			if (state==2){
				file<<"X"<<" ";
			}
			else{
				file<<state<<" ";
			}
		}
		file<<endl;
	}
	file.close();
}



int main(int argc, char ** argv)
{
    int optind = SetupOption(argc, argv);
    clock_t time_init, time_end;
    time_init = clock();
    //Setup File
    if (optind < argc) {
        if ((yyin = fopen(argv[optind], "r")) == NULL) {
            cout << "Can't open circuit file: " << argv[optind] << endl;
            exit( -1);
        }
        else {
            string circuit_name = argv[optind];
            string::size_type idx = circuit_name.rfind('/');
            if (idx != string::npos) { circuit_name = circuit_name.substr(idx+1); }
            idx = circuit_name.find(".bench");
            if (idx != string::npos) { circuit_name = circuit_name.substr(0,idx); }
            Circuit.SetName(circuit_name);
        }
    }
    else {
        cout << "Input circuit file missing" << endl;
        option.usage();
        return -1;
    }
    cout << "Start parsing input file\n";
    yyparse();
    if (ParseError) {
        cerr << "Please correct error and try Again.\n";
        return -1;
    }
    fclose(yyin);
    Circuit.FanoutList();
    Circuit.SetupIO_ID();
    Circuit.Levelize();
    Circuit.Check_Levelization();
    Circuit.InitializeQueue();

    if (option.retrieve("logicsim")) {
        //logic simulator
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.LogicSimVectors();
    }
    else if (option.retrieve("plogicsim")) {
        //parallel logic simulator
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.ParallelLogicSimVectors();
    }
    else if (option.retrieve("stfsim")) {
        //single pattern single transition-fault simulation
        Circuit.MarkOutputGate();
        Circuit.GenerateAllTFaultList();
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.TFaultSimVectors();
    }
    else if (option.retrieve("transition")) {
        Circuit.MarkOutputGate();
        Circuit.GenerateAllTFaultList();
        Circuit.SortFaninByLevel();
        if (option.retrieve("bt")) {
            Circuit.SetBackTrackLimit(atoi(option.retrieve("bt")));
        }
        Circuit.TFAtpg();
    }
	//ASS0
    //------------------------------------
    else if (option.retrieve("ass0")){
    	Circuit.ASS0_Print();
    }
    //------------------------------------
	
	//ASS1
	//------------------------------------
	else if (option.retrieve("path")){
		int startID;
        const char *start = option.retrieve("start"); 
        const char *end = option.retrieve("end");
        checkIO(start, end);
        string startString(start), endString(end);
        cout<<endl;
        GATE *startGate = FindPI(startString);
        GATE *endGate = FindPO(endString);
        cout<<endl;
        vector< vector < vector < string > > > Path(Circuit.No_Gate());
        vector<bool> visited(Circuit.No_Gate());
        
        DFS(startGate, endGate, Path, visited);
        
		startID = startGate->GetID();
		cout<<"PATH "<<endl;
		cout<<"-----------------------------------------------"<<endl;
		for (unsigned i = 0; i<Path[startID].size() ;++i){
			
			for (vector<string>::reverse_iterator it = Path[startID][i].rbegin() ; it != Path[startID][i].rend() ; ++it){
				cout<<*it<<" ";
			}
			cout<<endl<<endl;
		}
		
		cout<<endl<<"--------------------------------------------------"<<endl;

        cout << "The paths from " << startString << " to " << endString << ": " << Path[startID].size() << endl;
    }
	
	//------------------------------------
	else if (option.retrieve("pattern")){
		if (option.retrieve("unknown")){
			CreatePattern_Unknown();
		}
		else{
			CreatePattern();
		}
	}
	else if(option.retrieve("mod_logicsim")){
		Circuit.InitPattern(option.retrieve("input"));
		Circuit.LogicSimVectors_CPU();
	}
	else if(option.retrieve("simulator")){
		Circuit.InitPattern(option.retrieve("input"));
		Circuit.CreateLogicSim();
	}
	//-----------------------------------------
	else if(option.retrieve("check_point")){
		Circuit.SortFaninByLevel();
		Circuit.CompareFaultList();
		/*
		Circuit.GenerateCheckPointFaultList();
		Circuit.PrintFaultList();
		*/
	}
	else if(option.retrieve("bridging")){
		//Circuit.SortFaninByLevel();
		Circuit.GenerateBridgingFaultList();
	}
	else if (option.retrieve("CheckPoint_fsim")){
		Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.GenerateCheckPointFaultList();
        Circuit.ass5_1a();
        Circuit.FaultSimVectors();
    }
	else if (option.retrieve("bridging_fsim")){
		Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
		Circuit.GenerateBridgingFaultList();
		Circuit.InitPattern(option.retrieve("input"));
			
        Circuit.BridgeFaultSimVectors();
    }
    else if (option.retrieve("random_pattern")){
    	Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
        Circuit.GenerateCheckPointFaultList();
        Circuit.ass5_1a();
    	Circuit.Atpg_ASS6();
    }
    else if(option.retrieve("ASS6_test")){
    	Circuit.Faultlist_ASS6();
    	Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
        Circuit.Atpg();
    }
    else if(option.retrieve("bridging_atpg")){
    	Circuit.GenerateBridgingFaultList();
    	Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
        Circuit.Atpg_bridge();
    }
	else {
        Circuit.GenerateAllFaultList();
        Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
        /*
        Circuit.GenerateCheckPointFaultList();
        Circuit.ass5_1a();*/
        if (option.retrieve("fsim")) {
            //stuck-at fault simulator
        	Circuit.InitPattern(option.retrieve("input"));
        	Circuit.FaultSimVectors();
        }
        else {
            if (option.retrieve("bt")) {
                Circuit.SetBackTrackLimit(atoi(option.retrieve("bt")));
            }
            //stuck-at fualt ATPG
            
            Circuit.Atpg();
        }
    }
    time_end = clock();
    /*
	int gatenumber = Circuit.No_Gate();
	cout<<endl<<"Pattern Package Number: "<<PatternNum<<endl;
	cout<<"Total Pattern Number: "<<ASS3_patternNum<<endl;
    cout<<"Gate Evluation Number:"<<ASS3_counter<<endl;
    cout<<"Average Gate Evluation Number:"<<float(ASS3_counter)/float(ASS3_patternNum)<<endl;
    cout<<"Percentage of Average Gate Evaluations Over the Total Number of Gates: "<<float(ASS3_counter)/float(ASS3_patternNum)/float(gatenumber)<<endl<<endl;
    */
    
    
    /*
    int pid=(int) getpid();
	char buf[1024];
	sprintf(buf, "cat /proc/%d/statm",pid);
	system(buf); 
	*/
	system("ps aux | grep atpg"); 
	
	
    cout << "total CPU time = " << double(time_end - time_init)/CLOCKS_PER_SEC << endl;
    cout << endl;
    return 0;
}

#include<iostream>
#include<fstream>
#include<string.h>
#include<stdlib.h>

using namespace std;
bool exec_mem_stall = false;
bool decode_depend = false;
bool exec_stall = false;
bool decode_stall = false;
bool branch_setter = false;
bool branch_negative = false;
bool jump_neg = false;
int branch_pc;
bool branch_execute;
bool jump_execute;
bool halt_execute;
int jal_target;
bool prog_end = false;
bool stop = false;
int mem_counter = 0;
int branch_source;
bool branch_offset = false;
int branch_dispatch_cc;
bool branch_rob = false;
/*bool m1_stall = false;
bool m2_stall = false;
bool md_stall = false;
bool div_stall = false;*/
//bool exec_output_stall = false;

struct Code_Line {
	int file_line_number;
	int address;
	string instruction_string;
};

struct Code_Memory {
	Code_Line codememory[100];
}Cd_Obj;

struct Data_Memory{
	int base_address;
	int data[4000];
};

struct Register{
	int value;
	bool status = true;
	//int status_counter = 0;
	bool allocate = false;	
	bool rename = false;
	bool cc = false;//conditional counter
};


struct Register_File{
       Register reg[16];
       Register	phy[32];	
};


struct Rename{
	bool phyarch = false;
	int address;	
};


struct Rename_Table{
	Rename rtable[16] ;//= {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};	

};

struct rob_e{
	int dest_add;
	int destar_add;
	int destresult;
	bool status = false;
	int prev_dest_add;
	int Dispatchclk;
	bool phyarch = false;
	int prev_destresult;
	string opcode;
	string instruction_string;
};

struct rob{
	rob_e robentry[32];
	int start_pt; // head
	int end_pt = 0;   //tail	
};


struct lsqe{
	string lsqstring;	
	bool lsq1_valid = false;
	int lsq1_value;
	int lsq1_address;
	bool lsq2_valid = false;
	int lsq2_value;
	int lsq2_address;
	int ldst_value;
	int ldst_address;
	string lsqdest;
	int mem_address;
	bool mem_addressvalid;
	int tail;
	bool valid;
	string lopcode;
	int ldispatch;
	int rename_destination;
	bool validcheck = false ;
};

struct lsq{
	lsqe temp;
	lsqe lsqentry[32];
	int startpt;
	int endpt = 0;	
}lsq_name;



struct Instruction_info{
	int PC;
	string instruction_string = "nop";
	int source1_register;
	int source1_val;
	string source1name_register;
	int source2_register;
	int source2_val;
	string source2name_register;
	string destname_register; //string rename reg
	int rename_destination;  //number rename value		
	int dest_register;
	int prev_dest_add;
	int prev_destresult;	
	int phyarch; 
	int literal;
	int tail;
	string opcode1 = "nop";
	string target_memory_addr;
	string target_memory_data;
	int lsq_entry;
	int dispatch;
	int computed_val;
  	bool stalled = false;
	bool src1_valid = false;
	bool src2_valid = false;
};

struct stage{
	Instruction_info* Input_Instruction;
	Instruction_info* output_Instruction;
	bool Stalled = false;
	bool next_stage = false;
}F,D,I,E,EM1,EM2,ED1,ED2,ED3,ED4,L,R,M,W,M_temp,M2_temp;

struct Issue{
	bool Qs1_valid = false;
	int Qs1_value;
	int Qs1_address;
	bool Qs2_valid = false;
	int Qs2_value;
	int Qs2_address;
	string Qdest;
	int Qd;
	string Qinstruction_string;
	string Qopcode;
	int Qliteral;
	int QPC;
	int Qdispatch_clk;
	bool Q_valid = false;
	int Qtail;
	int Qlsqtail;
	int Qrename_destination;
};

struct Issue_Q{
	Issue entry[16];	
}IS;



struct Flags{
	bool zero = false;
	bool carry = false;
	bool negative = false;
};

struct Stats{
	int cycle = 0;
}C;




struct fwd{
	bool valid;
	int reg_address;
	int reg_value;
};

struct bus{
	fwd busarray[5];
	/*
	0:LOAD MEM
	1:LOAD LSQ
	2:IFU
	3:MUL
	4:DIV	*/
				
}fwd_b;

void Fetch(stage* fetch, stage* decode){
	
	if (fetch -> Stalled == true){
		return;	
	}
	else{
		if(fetch -> output_Instruction -> stalled && decode -> Input_Instruction -> stalled){
			fetch -> Input_Instruction -> stalled = true;	
			//cout << "Testing" << endl;		
		}
		else{
			fetch -> Input_Instruction -> stalled = false;		
		}		
		
		
		if (decode -> output_Instruction -> stalled)
		{
			if(!decode -> Input_Instruction ->stalled){			
				fetch -> output_Instruction -> instruction_string = fetch -> Input_Instruction -> instruction_string;
				fetch -> output_Instruction -> PC = fetch -> Input_Instruction -> PC;
				
				decode -> Input_Instruction -> instruction_string = fetch -> output_Instruction -> instruction_string;
				decode -> Input_Instruction -> PC = fetch -> output_Instruction -> PC; 
				

				fetch -> output_Instruction -> stalled = true;
				decode -> Input_Instruction -> stalled = true;
				
			}
			else
			{
				return;
			}					
		}
		else if(!decode -> output_Instruction -> stalled){
			fetch -> output_Instruction -> instruction_string = fetch -> Input_Instruction -> instruction_string;
			fetch -> output_Instruction -> PC = fetch -> Input_Instruction -> PC;
				
			decode -> Input_Instruction -> instruction_string = fetch -> output_Instruction -> instruction_string;
			decode -> Input_Instruction -> PC = fetch -> output_Instruction -> PC; 

			fetch -> Input_Instruction -> stalled = false;			
			fetch -> output_Instruction -> stalled = false;
			decode -> Input_Instruction -> stalled = false;
			
		}
			
		decode -> next_stage = true;
	}
}

void Decode(stage* decode,stage* iq, stage* exec,stage* exec_m1,stage* exec_d1,Register_File* r_f,Rename_Table* r_t,bus* f_b){
	if (decode -> Stalled == true){
		return;	
	} 
	else{
		if (decode -> next_stage){
			if(!decode -> output_Instruction -> stalled){	
				decode -> output_Instruction -> instruction_string = decode -> Input_Instruction -> instruction_string;	
				//cout << endl << "Decode output is:" << endl<< decode -> output_Instruction -> instruction_string << endl;
				decode -> output_Instruction -> PC = decode -> Input_Instruction -> PC;
				string s = decode -> output_Instruction -> instruction_string;
				string opcode;
				//cout << "Test" << endl;
				if (decode -> output_Instruction -> instruction_string != "nop")
				{
					opcode = s.substr(0, s.find(','));
					
				}
				else
				{
					opcode = "nop";
				}

				decode -> output_Instruction -> opcode1 = opcode;

				int src1;
				int src2;
				int ltrl;
				int dest;
				string d1;
				

				if((opcode == "BZ")||(opcode =="BNZ")){
					string litr;
					decode -> output_Instruction -> source1name_register = "R" + to_string(branch_source);
					decode -> output_Instruction -> source1_register = branch_source;
					decode -> output_Instruction -> destname_register = "";
					decode -> output_Instruction -> source2name_register = "";
					if(s.find("-") == -1){
						litr = s.substr(s.find('#')+1);
						ltrl = atoi(litr.c_str());
						//cout << "ltrl value " << ltrl << endl;
						decode -> output_Instruction -> literal = ltrl;
						branch_offset = false;
					}
					else
					{
						litr = s.substr(s.find('-')+1);
						ltrl = atoi(litr.c_str());
						//cout << "ltrl value " << ltrl << endl;
						decode -> output_Instruction -> literal = ltrl;
						branch_offset = true;
					}
					/*if(r_f -> reg[16].status){
						decode_depend = false;
					}
					else{
						//cout << "BZ status chk";
						decode -> output_Instruction -> stalled = true;	
						decode_depend = true;				
					}	*/

					src1 = decode -> output_Instruction -> source1_register;
					//cout << "src1 - " << src1 << endl;

					decode -> output_Instruction -> src1_valid = false;
					decode -> output_Instruction -> src2_valid = true;
					if(r_f -> phy[src1].status){					
						decode -> output_Instruction -> source1_val = r_f -> phy[src1].cc;
						decode -> output_Instruction -> src1_valid = true;	
						//cout << "??1" << endl;
					}
					else{
						for(int i=0;i<=5;i++){
							if(f_b -> busarray[i].valid){
								if(decode -> output_Instruction -> source1_register == f_b -> busarray[i].reg_address){
									if (f_b -> busarray[i].reg_value == 0)
									{
										decode -> output_Instruction -> source1_val = 1;
									}
									else
									{
										decode -> output_Instruction -> source1_val = 0;
									}
									decode -> output_Instruction -> src1_valid = true;
									//cout << "??2" << endl;
								}		
							}
						}		
					}
					//cout << "decode -> output_Instruction -> src1_valid" << decode -> output_Instruction -> src1_valid << endl;
				}

				else if(opcode == "JUMP"){
					string sr1,litr;					
					
					if(s.find("-") == -1){
						s = s.substr(s.find(',')+1);
						sr1 = s.substr(0, s.find(','));	
						src1 = atoi(sr1.substr(1).c_str());
						litr = s.substr(s.find('#')+1);
						ltrl = atoi(litr.c_str());
						jump_neg = false;
					}
					else{
						s = s.substr(s.find(',')+1);
						sr1 = s.substr(0, s.find(','));	
						src1 = atoi(sr1.substr(1).c_str());
						litr = s.substr(s.find('-')+1);
						ltrl = atoi(litr.c_str());
						jump_neg = true;
					}

					/*if(r_t -> rtable[src1].phyarch = false){
						decode -> output_Instruction -> source1_register = src1;
										
					}

					else{
						decode -> output_Instruction -> source1_register = r_t ->rtable[src1].address;
						sr1 = "R"+to_string(r_t ->rtable[src1].address);
						src1 = 	r_t ->rtable[src1].address ;	  
						
					}
					
					decode -> output_Instruction -> source1name_register = sr1;
					
					//decode -> output_Instruction -> source1_register = src1;	
					decode -> output_Instruction -> destname_register = "";
					decode -> output_Instruction -> source2name_register = "";
					decode -> output_Instruction -> literal = ltrl;
										
					if(r_f -> reg[atoi(sr1.substr(1).c_str())].status){
						decode -> output_Instruction -> source1_val = r_f -> reg[atoi(sr1.substr(1).c_str())].value;	
						decode -> output_Instruction -> src1_valid = true;					
					}
					else{
						decode -> output_Instruction -> src1_valid = false;					
					}
					*/
					decode -> output_Instruction -> src1_valid = false;

					if(r_t -> rtable[src1].phyarch == false){
						decode -> output_Instruction -> source1_register = src1;

						if(r_f -> reg[src1].status){					
							decode -> output_Instruction -> source1_val = r_f -> reg[atoi(sr1.substr(1).c_str())].value;
							decode -> output_Instruction -> src1_valid = true;
							//cout << "ADD S1 - " << decode -> output_Instruction -> source1_val << endl;
						}
										
					}

					else{
						decode -> output_Instruction -> source1_register = r_t ->rtable[src1].address;
						sr1 = "R"+to_string(r_t ->rtable[src1].address);
						src1 = 	r_t ->rtable[src1].address ;	  
						//cout<<"step1 src1"<<endl;


						decode -> output_Instruction -> source1_register = src1;

						if(r_f -> phy[src1].status){					
							decode -> output_Instruction -> source1_val = r_f -> phy[atoi(sr1.substr(1).c_str())].value;
							decode -> output_Instruction -> src1_valid = true;	
						}
						else{
							for(int i=0;i<=5;i++){
								if(f_b -> busarray[i].valid){
									if(decode -> output_Instruction -> source1_register == f_b -> busarray[i].reg_address){
										decode -> output_Instruction -> source1_val = f_b -> busarray[i].reg_value;
										decode -> output_Instruction -> src1_valid = true;	
									}		
								}
							}

							
						}
					}
																	
				}
				else if(opcode == "HALT"){
					decode -> output_Instruction -> destname_register = "";
					decode -> output_Instruction -> source1name_register = "";
					decode -> output_Instruction -> source2name_register = "";
				}

				else if((opcode == "ADD")||(opcode =="SUB")||(opcode =="MUL")||(opcode =="EXOR")||(opcode =="OR")||(opcode =="AND")||(opcode =="DIV")){
					s = s.substr(s.find(',')+1);
 					d1 = s.substr(0, s.find(','));
					//cout << "sub string s: " << s << endl<<d1<<endl;
					dest = atoi(d1.substr(1).c_str());
					string s1;
					s1 = s.substr(s.find(',')+1);
					//cout << "s1" << s1 << endl;
					string sr1 = s1.substr(0, s.find(',')); 				
					string sr2 = s1.substr(s1.find(',')+1);
					//cout << "sr " << sr1 << "-" <<sr2 <<endl;
					src1 = atoi(sr1.substr(1).c_str());
					src2 = atoi(sr2.substr(1).c_str()); 
					//cout << "src " << src1 << endl <<src2 <<endl;
					
									 			
					/*if(r_t -> rtable[src1].phyarch == false){
						decode -> output_Instruction -> source1_register = src1;
										
					}

					else{
						decode -> output_Instruction -> source1_register = r_t ->rtable[src1].address;
						//cout << "R ----------- " << r_t ->rtable[src1].address << endl;
						sr1 = "R"+to_string(r_t ->rtable[src1].address);
						src1 = 	r_t ->rtable[src1].address ;  
						
					}
		
					if(r_t -> rtable[src2].phyarch == false){
						decode -> output_Instruction -> source2_register = src2;
					}

					else{
						 
						decode -> output_Instruction -> source2_register =  r_t ->rtable[src2].address;	
						sr2 = "R"+to_string(r_t ->rtable[src2].address);
						src2 = 	r_t ->rtable[src2].address ;  
					}
					
					//decode -> output_Instruction -> destname_register = d1;
					decode -> output_Instruction -> source1name_register = sr1;
					decode -> output_Instruction -> source2name_register = sr2;*/
					decode -> output_Instruction -> src1_valid = false;

					if(r_t -> rtable[src1].phyarch == false){
						decode -> output_Instruction -> source1_register = src1;

						if(r_f -> reg[src1].status){					
							decode -> output_Instruction -> source1_val = r_f -> reg[atoi(sr1.substr(1).c_str())].value;
							decode -> output_Instruction -> src1_valid = true;
							//cout << "ADD S1 - " << decode -> output_Instruction -> source1_val << endl;
						}
										
					}

					else{
						decode -> output_Instruction -> source1_register = r_t ->rtable[src1].address;
						sr1 = "R"+to_string(r_t ->rtable[src1].address);
						src1 = 	r_t ->rtable[src1].address ;	  
						//cout<<"step1 src1"<<endl;


						decode -> output_Instruction -> source1_register = src1;

						if(r_f -> phy[src1].status){					
							decode -> output_Instruction -> source1_val = r_f -> phy[atoi(sr1.substr(1).c_str())].value;
							decode -> output_Instruction -> src1_valid = true;	
						}
						else{
							for(int i=0;i<=5;i++){
								if(f_b -> busarray[i].valid){
									if(decode -> output_Instruction -> source1_register == f_b -> busarray[i].reg_address){
										decode -> output_Instruction -> source1_val = f_b -> busarray[i].reg_value;
										decode -> output_Instruction -> src1_valid = true;	
									}		
								}
							}

							
						}
					}

					decode -> output_Instruction -> src2_valid = false;
					
					if(r_t -> rtable[src2].phyarch == false){
						decode -> output_Instruction -> source2_register = src2;

						if(r_f -> reg[src2].status){
							decode -> output_Instruction -> source2_val = r_f -> reg[atoi(sr2.substr(1).c_str())].value;
							decode -> output_Instruction -> src2_valid = true;
							//cout << "ADD S2 - " << decode -> output_Instruction -> source2_val << endl;
						}
					
					}

					else{
						 
						decode -> output_Instruction -> source2_register =  r_t ->rtable[src2].address;	
						sr2 = "R"+to_string(r_t ->rtable[src2].address);
						src2 = 	r_t ->rtable[src2].address;
						//cout<<"step1 src2"<<endl;

						decode -> output_Instruction -> source2_register = src2;

						if(r_f -> phy[src2].status){
							decode -> output_Instruction -> source2_val = r_f -> phy[atoi(sr2.substr(1).c_str())].value;
							decode -> output_Instruction -> src2_valid = true;
						}
						else{
							for(int i=0;i<=5;i++){
								if(f_b -> busarray[i].valid){
									if(decode -> output_Instruction -> source2_register == f_b -> busarray[i].reg_address){
										decode -> output_Instruction -> source2_val = f_b -> busarray[i].reg_value;
										decode -> output_Instruction -> src2_valid = true;	
									}		
								}
							}

							
						}
//////recieving fwd
					}	
									
					if((opcode == "ADD")||(opcode =="SUB")||(opcode =="MUL")||(opcode =="DIV")){
						branch_setter = true;
						branch_pc = decode -> output_Instruction -> PC;
					}
					else{
						branch_setter = false;
					}

					//cout << "SRC 1 - " << decode -> output_Instruction -> src1_valid << endl;
					//cout << "SRC 2 - " << decode -> output_Instruction -> src2_valid << endl;


					//for rob
					decode -> output_Instruction -> dest_register = dest;
					decode -> output_Instruction-> prev_dest_add = r_t -> rtable[dest].address;
					decode -> output_Instruction-> phyarch = r_t -> rtable[dest].phyarch;
					if (r_t -> rtable[dest].phyarch == true){	 					
					      decode -> output_Instruction -> prev_destresult = r_f -> phy[r_t -> rtable[dest].address].value ;}
					else{
					      decode -> output_Instruction-> prev_destresult = r_f -> reg[r_t -> rtable[dest].address].value ;}	

													
					if(r_t -> rtable[dest].phyarch == true){
						r_f->phy[r_t -> rtable[dest].address].rename = true;
					}
					r_f -> reg[dest].status = false;					
										       
					for(int e = 0;e<32;e++){
								
						if(r_f -> phy[e].allocate == false){	
							r_t -> rtable[dest].address = e;
							r_f -> phy[e].allocate = true;
							r_f -> phy[e].status = false;
							r_t -> rtable[dest].phyarch = true;
							decode -> output_Instruction -> rename_destination = e;
						 	decode -> output_Instruction -> destname_register =  "R"+to_string(e);
							//cout << "rename dest:" << decode -> output_Instruction -> destname_register << endl;
							break;				
						}
					}	
					branch_source = decode -> output_Instruction -> rename_destination ;
					/*decode -> output_Instruction -> src1_valid = false;
					if(r_f -> reg[src1].status){
						
						decode -> output_Instruction -> source1_val = r_f -> reg[atoi(sr1.substr(1).c_str())].value;
						decode -> output_Instruction -> src1_valid = true;
						//cout << "src1 value " << decode -> output_Instruction -> source1_val << endl;
					}
					else{
						for(int i=0;i<5;i++){
							if(f_b -> busarray[i].valid){
								if(decode -> output_Instruction -> source1_register == f_b -> busarray[i].reg_address){
									decode -> output_Instruction -> source1_val = f_b -> busarray[i].reg_value;
									decode -> output_Instruction -> src1_valid = true;	
								}		
							}
						}								
					}

					decode -> output_Instruction -> src2_valid = false;
					if (r_f -> reg[src2].status){
						decode -> output_Instruction -> source2_val = r_f -> reg[atoi(sr2.substr(1).c_str())].value;
						decode -> output_Instruction -> src2_valid = true;
						//cout << "src2 value " << decode -> output_Instruction -> source2_val << endl;
						
					}
					else{
						for(int i=0;i<5;i++){
							if(f_b -> busarray[i].valid){
								if(decode -> output_Instruction -> source2_register == f_b -> busarray[i].reg_address){
									decode -> output_Instruction -> source2_val = f_b -> busarray[i].reg_value;
									decode -> output_Instruction -> src2_valid = true;	
								}		
							}
						}
							
					}*/
				
				}
				else if(opcode == "MOVC"){
					s = s.substr(s.find(',')+1);
					
					d1 = s.substr(0,s.find(','));
					dest = atoi(d1.substr(1).c_str());
				
					decode -> output_Instruction -> dest_register = dest;
					decode -> output_Instruction-> prev_dest_add = r_t -> rtable[dest].address;
					decode -> output_Instruction-> phyarch = r_t -> rtable[dest].phyarch;

					if(r_t -> rtable[dest].phyarch == true){
						r_f->phy[r_t -> rtable[dest].address].rename = true;
					}
					r_f -> reg[dest].status = false;
					       
					for(int e = 0;e<32;e++){						
						if(r_f -> phy[e].allocate == false){	
							r_t -> rtable[dest].address = e;
							r_f -> phy[e].allocate = true;
							r_f -> phy[e].status = false;
							r_t -> rtable[dest].phyarch = true;
							decode -> output_Instruction -> rename_destination = e;
							decode -> output_Instruction -> destname_register =  "R"+to_string(e);
							//cout << "rename dest:" << decode -> output_Instruction -> destname_register << endl;
							break;				
						}
					}	
					//decode -> output_Instruction -> destname_register = d1;
					decode -> output_Instruction -> dest_register =dest;
					decode -> output_Instruction -> source1name_register = "";
					decode -> output_Instruction -> source2name_register = "";
					string litr = s.substr(s.find(',')+2);
					ltrl = atoi(litr.c_str());
					decode -> output_Instruction -> src1_valid = true;
					decode -> output_Instruction -> src2_valid = true;
					
				}
				else if((opcode =="SUBL")||(opcode =="ADDL")||(opcode =="LOAD") ||(opcode == "JAL")){
					s = s.substr(s.find(',')+1);
					//cout << "s is " << s << endl;
					d1 = s.substr(0,s.find(','));
					//cout << "d1 is " << d1 << endl;
					dest = atoi(d1.substr(1).c_str());
					//cout << "dest is " << dest << endl;
					string s1;
					s1 = s.substr(s.find(',')+1);
					//cout << "s1 is " << s1 << endl;
					string sr1 = s1.substr(0,s.find(',')+1);
					//cout << "sr1 is " << sr1 << endl;
					
					string litr;

					decode -> output_Instruction -> dest_register = dest;
					decode -> output_Instruction-> prev_dest_add = r_t -> rtable[dest].address;
					decode -> output_Instruction-> phyarch = r_t -> rtable[dest].phyarch;

					if (opcode != "JAL")
					{ 
						litr = s1.substr(s1.find('#')+1);
						//cout << "litr is " << litr << endl;
						ltrl = atoi(litr.c_str());
						//cout << "ltrl is " << ltrl << endl; 
						src1 = atoi(sr1.substr(1).c_str());
						//cout << "src1 is " << src1 << endl;
					}
					else if (opcode == "JAL")
					{
						if(s.find("-") == -1){
							litr = s1.substr(s.find(',')+2);
							ltrl = atoi(litr.c_str()); 
							src1 = atoi(sr1.substr(1).c_str());
							jump_neg = false;
							//cout << endl << litr << endl << src1 << endl;
						}
				
						else{
							litr = s.substr(s.find('-')+1);
							ltrl = atoi(litr.c_str());
							src1 = atoi(sr1.substr(1).c_str());
							jump_neg = true;
							//cout << endl << litr << endl << src1 << endl;
						}
					}

					if(r_t -> rtable[dest].phyarch == true){
						r_f->phy[r_t -> rtable[dest].address].rename = true;
					}

					r_f -> reg[dest].status = false;					       
					for(int e = 0;e<32;e++){
								
						if(r_f -> phy[e].allocate == false){	
							r_t -> rtable[dest].address = e;
							r_f -> phy[e].allocate = true;
							r_f -> phy[e].status = false;
							r_t -> rtable[dest].phyarch = true;
							decode -> output_Instruction -> rename_destination = e;
						 	decode -> output_Instruction -> destname_register =  "R"+to_string(e);
							//cout << "rename dest:" << decode -> output_Instruction -> destname_register << endl;
							break;				
						}
					}	
					
					decode -> output_Instruction -> src1_valid = false;
					if(r_t -> rtable[src1].phyarch == false){
						decode -> output_Instruction -> source1_register = src1;
						//cout << "dssd" << endl;	
						if(r_f -> reg[src1].status){

							decode -> output_Instruction -> source1_val = r_f -> reg[atoi(sr1.substr(1).c_str())].value;
							decode -> output_Instruction -> src1_valid = true;	
						}
										
					}

					else{
						decode -> output_Instruction -> source1_register = r_t ->rtable[src1].address;
						sr1 = "R"+to_string(r_t ->rtable[src1].address);
						src1 = 	r_t ->rtable[src1].address ;	  
						//cout<<"step1 src1 - " << src1 <<endl;


						decode -> output_Instruction -> source1_register = src1;

						if(r_f -> phy[src1].status){					
							decode -> output_Instruction -> source1_val = r_f -> phy[atoi(sr1.substr(1).c_str())].value;
							decode -> output_Instruction -> src1_valid = true;	
						}
						else{
							for(int i=0;i<=5;i++){
								if(f_b -> busarray[i].valid){
									if(decode -> output_Instruction -> source1_register == f_b -> busarray[i].reg_address){
										decode -> output_Instruction -> source1_val = f_b -> busarray[i].reg_value;
										decode -> output_Instruction -> src1_valid = true;	
									}		
								}
							}

							
						}
					}

					//cout << "LOAD SRC1 Status - " << decode -> output_Instruction -> src1_valid << endl;
					//cout << "LOAD SRC1 Value - " << decode -> output_Instruction -> source1_val << endl;
					/*if(r_t -> rtable[src1].phyarch == false){
						decode -> output_Instruction -> source1_register = src1;
										
					}

					else{
						decode -> output_Instruction -> source1_register = r_t ->rtable[src1].address;
						sr1 = "R"+to_string(r_t ->rtable[src1].address);
						src1 = 	r_t ->rtable[src1].address ;	  
						
					}*/
					//decode -> output_Instruction -> destname_register = d1;
					decode -> output_Instruction -> dest_register =dest;
					//decode -> output_Instruction -> source1name_register = sr1;
					decode -> output_Instruction -> source2name_register = "";
					decode -> output_Instruction -> src2_valid = true;
					//decode -> output_Instruction -> source1_register = src1;
						
					/*
					if(r_f -> reg[src1].status){
						
						decode -> output_Instruction -> source1_val = r_f -> reg[atoi(sr1.substr(1).c_str())].value;
						decode -> output_Instruction -> src1_valid = true;
				
					}
					else{
						for(int i=0;i<=5;i++){
							if(f_b -> busarray[i].valid){
								if(decode -> output_Instruction -> source1_register == f_b -> busarray[i].reg_address){
									decode -> output_Instruction -> source1_val = f_b -> busarray[i].reg_value;
									decode -> output_Instruction -> src1_valid = true;	
								}		
							}
						}

						//decode -> output_Instruction -> src1_valid = false;
					}*/
												
				}
			
				else if (opcode == "STORE"){
					s = s.substr(s.find(',')+1);
					string sr1 = s.substr(0,s.find(','));	
					string s1;
					s1 = s.substr(s.find(',')+1);
					string sr2 = s1.substr(0,s.find(','));	
					string litr = s1.substr(s1.find('#')+1);				
					ltrl =  atoi(litr.c_str());
					//cout<<"step1 - " << sr1 << " " << sr2 <<endl;
					src1 = atoi(sr1.substr(1).c_str());
					src2 = atoi(sr2.substr(1).c_str());

					decode -> output_Instruction -> src1_valid = false;
					
					if(r_t -> rtable[src1].phyarch == false){
						decode -> output_Instruction -> source1_register = src1;

						if(r_f -> reg[src1].status){					
							decode -> output_Instruction -> source1_val = r_f -> reg[atoi(sr1.substr(1).c_str())].value;
							decode -> output_Instruction -> src1_valid = true;	
						}
						/*else{
							for(int i=0;i<=5;i++){
								if(f_b -> busarray[i].valid){
									if(decode -> output_Instruction -> source1_register == f_b -> busarray[i].reg_address){
										decode -> output_Instruction -> source1_val = f_b -> busarray[i].reg_value;
										decode -> output_Instruction -> src1_valid = true;	
									}		
								}
							}

							//decode -> output_Instruction -> src1_valid = false;
						}*/
										
					}

					else{
						decode -> output_Instruction -> source1_register = r_t ->rtable[src1].address;
						sr1 = "R"+to_string(r_t ->rtable[src1].address);
						src1 = 	r_t ->rtable[src1].address ;	  
						//cout<<"step1 src1"<<endl;


						decode -> output_Instruction -> source1_register = src1;

						if(r_f -> phy[src1].status){					
							decode -> output_Instruction -> source1_val = r_f -> phy[atoi(sr1.substr(1).c_str())].value;
							decode -> output_Instruction -> src1_valid = true;
							//cout << "SRC 1 - " << decode -> output_Instruction -> source1_val << endl;
						}
						else{
							for(int i=0;i<=5;i++){
								if(f_b -> busarray[i].valid){
									if(decode -> output_Instruction -> source1_register == f_b -> busarray[i].reg_address){
										decode -> output_Instruction -> source1_val = f_b -> busarray[i].reg_value;
										decode -> output_Instruction -> src1_valid = true;	
									}		
								}
							}

							//decode -> output_Instruction -> src1_valid = false;
						}
					}
					//cout<<"step1"<<endl;

					decode -> output_Instruction -> src2_valid = false;
					if(r_t -> rtable[src2].phyarch == false){
						decode -> output_Instruction -> source2_register = src2;

						if(r_f -> reg[src2].status){
							decode -> output_Instruction -> source2_val = r_f -> reg[atoi(sr2.substr(1).c_str())].value;
							decode -> output_Instruction -> src2_valid = true;
						}
						/*else{
							for(int i=0;i<=5;i++){
								if(f_b -> busarray[i].valid){
									if(decode -> output_Instruction -> source2_register = f_b -> busarray[i].reg_address){
										decode -> output_Instruction -> source2_val = f_b -> busarray[i].reg_value;
										decode -> output_Instruction -> src2_valid = true;	
									}		
								}
							}

							//decode -> output_Instruction -> src2_valid = false;
						}*/
					}

					else{
						 
						decode -> output_Instruction -> source2_register =  r_t ->rtable[src2].address;	
						sr2 = "R"+to_string(r_t ->rtable[src2].address);
						src2 = 	r_t ->rtable[src2].address;
						//cout<<"step1 src2 ---- " << src2 <<endl;

						decode -> output_Instruction -> source2_register = src2;

						if(r_f -> phy[src2].status){
							decode -> output_Instruction -> source2_val = r_f -> phy[atoi(sr2.substr(1).c_str())].value;
							decode -> output_Instruction -> src2_valid = true;
						}
						else{
							for(int i=0;i<=5;i++){
								if(f_b -> busarray[i].valid){
									if(decode -> output_Instruction -> source2_register == f_b -> busarray[i].reg_address){
										decode -> output_Instruction -> source2_val = f_b -> busarray[i].reg_value;
										decode -> output_Instruction -> src2_valid = true;	
									}		
								}
							}

							//decode -> output_Instruction -> src2_valid = false;
						}
					}
					//cout<<"step1"<<endl;
					decode -> output_Instruction -> destname_register = "";
					decode -> output_Instruction -> source1name_register = sr1;
					decode -> output_Instruction -> source2name_register = sr2;
					

					//src1 = atoi(sr1.substr(1).c_str());
					//src2 = atoi(sr2.substr(1).c_str());

					//decode -> output_Instruction -> source1_register = src1;
					//decode -> output_Instruction -> source2_register = src2;

					//cout << "STORE - " << decode -> output_Instruction -> src2_valid << endl;
					//cout<<"step1"<<endl;
					/*if(r_f -> reg[src2].status){
						decode -> output_Instruction -> source2_val = r_f -> reg[atoi(sr2.substr(1).c_str())].value;
						decode -> output_Instruction -> src2_valid = true;
					}
					else{
						for(int i=0;i<=5;i++){
							if(f_b -> busarray[i].valid){
								if(decode -> output_Instruction -> source2_register = f_b -> busarray[i].reg_address){
									decode -> output_Instruction -> source2_val = f_b -> busarray[i].reg_value;
									decode -> output_Instruction -> src2_valid = true;	
								}		
							}
						}

						//decode -> output_Instruction -> src2_valid = false;
					}*/
					
				}
				//cout<<"step1"<<endl;
				decode -> output_Instruction -> dest_register = dest;
				decode -> output_Instruction -> opcode1 = opcode;
				decode -> output_Instruction -> literal = ltrl;
				
				//cout << "Testing Seg Fault - dest - " << decode -> output_Instruction -> literal << endl;
				//cout << "Testing Seg Fault - opcode1 - " << decode -> output_Instruction -> opcode1 << endl;
				//cout << "Testing Seg Fault - literal - " << decode -> output_Instruction -> PC << endl;				

				iq -> Input_Instruction -> instruction_string = decode -> output_Instruction -> instruction_string;
				iq -> Input_Instruction -> source1_register = decode -> output_Instruction -> source1_register;
				iq -> Input_Instruction -> source1_val = decode -> output_Instruction -> source1_val;
				iq -> Input_Instruction -> src1_valid = decode -> output_Instruction -> src1_valid;
				iq -> Input_Instruction -> source2_register = decode -> output_Instruction -> source2_register;
				iq -> Input_Instruction -> source2_val = decode -> output_Instruction -> source2_val;
				iq -> Input_Instruction -> src2_valid = decode -> output_Instruction -> src2_valid;
				iq -> Input_Instruction -> dest_register = decode -> output_Instruction -> dest_register;
				iq -> Input_Instruction -> source1name_register = decode -> output_Instruction -> source1name_register;
				iq -> Input_Instruction -> source2name_register = decode -> output_Instruction -> source2name_register;
				iq -> Input_Instruction -> destname_register = decode -> output_Instruction -> destname_register;
				iq -> Input_Instruction -> rename_destination = decode -> output_Instruction -> rename_destination;
				iq -> Input_Instruction -> literal = decode -> output_Instruction -> literal;
				iq -> Input_Instruction -> opcode1 = decode -> output_Instruction -> opcode1;
				iq -> Input_Instruction -> PC = decode -> output_Instruction -> PC;
				iq -> Input_Instruction -> dest_register = decode -> output_Instruction -> dest_register ;
				iq -> Input_Instruction -> prev_dest_add = decode -> output_Instruction-> prev_dest_add;
				iq -> Input_Instruction -> phyarch = decode -> output_Instruction-> phyarch ;
				iq -> Input_Instruction-> prev_destresult = decode -> output_Instruction-> prev_destresult;
 				iq -> next_stage = true;
				//cout << "Testing Seg Fault" << endl;
			}
		}
	}
}







//void IssueQ(stage* iq, stage* exec,stage* exec_m1,stage* exec_d1,Issue_Q* is,Stats * cyc,rob *ROB){
void IssueQ(stage* iq, stage* exec,stage* exec_m1,stage* exec_d1,Issue_Q* is,Stats * cyc,Register_File* r_f,Rename_Table* r_t,bus* f_b, rob* rob_array,lsq * LSQ){
		
	if (iq -> next_stage){
		
		iq -> output_Instruction -> instruction_string = iq -> Input_Instruction -> instruction_string;
		iq -> output_Instruction -> source1_register = iq -> Input_Instruction -> source1_register;
		iq -> output_Instruction -> source1_val = iq -> Input_Instruction -> source1_val;
		iq -> output_Instruction -> src1_valid = iq -> Input_Instruction -> src1_valid;
		iq -> output_Instruction -> source2_register = iq -> Input_Instruction -> source2_register;
		iq -> output_Instruction -> source2_val = iq -> Input_Instruction -> source2_val;
		iq -> output_Instruction -> src2_valid = iq -> Input_Instruction -> src2_valid;
		iq -> output_Instruction -> dest_register = iq -> Input_Instruction -> dest_register;
		
		iq -> output_Instruction -> source1name_register = iq -> Input_Instruction -> source1name_register;
		iq -> output_Instruction -> source2name_register = iq -> Input_Instruction -> source2name_register;
		iq -> output_Instruction -> rename_destination = iq -> Input_Instruction -> rename_destination;
		
		iq -> output_Instruction -> destname_register = iq -> Input_Instruction -> destname_register;
		iq -> output_Instruction -> literal = iq -> Input_Instruction -> literal;
		iq -> output_Instruction -> opcode1 = iq -> Input_Instruction -> opcode1;
		iq -> output_Instruction -> PC = iq -> Input_Instruction -> PC;

		iq -> output_Instruction-> prev_destresult = iq -> Input_Instruction-> prev_destresult;
		iq -> output_Instruction -> prev_dest_add = iq -> Input_Instruction-> prev_dest_add;
		iq -> output_Instruction -> phyarch = iq -> Input_Instruction-> phyarch ;

		
	
	
		//insertion in rob
		if (iq -> output_Instruction -> instruction_string != "nop" && iq -> output_Instruction -> instruction_string != "" && iq -> output_Instruction -> opcode1 != "nop")
		{
			//cout << "iq -> output_Instruction -> instruction_string - " << iq -> output_Instruction -> instruction_string << endl;
			//cout << "iq -> output_Instruction -> opcode1 - " << iq -> output_Instruction -> opcode1 << endl;
			//cout << "iq -> output_Instruction -> instruction_string - " << iq -> output_Instruction -> instruction_string << endl;
			rob_array -> robentry[rob_array -> end_pt].dest_add      = iq -> output_Instruction -> rename_destination ; 
			rob_array -> robentry[rob_array -> end_pt].destar_add    =iq -> output_Instruction -> dest_register  ; 
			rob_array -> robentry[rob_array -> end_pt].prev_dest_add = iq -> output_Instruction -> prev_dest_add;
			rob_array -> robentry[rob_array -> end_pt].phyarch       =iq -> output_Instruction -> phyarch;
			rob_array -> robentry[rob_array -> end_pt].Dispatchclk   = cyc -> cycle + 1;
			rob_array -> robentry[rob_array -> end_pt].prev_destresult	= iq -> output_Instruction-> prev_destresult ;
			rob_array -> robentry[rob_array -> end_pt].opcode = iq -> output_Instruction -> opcode1;
			rob_array -> robentry[rob_array -> end_pt].instruction_string = iq -> output_Instruction -> instruction_string;
			//cout << "Rob Tail - " << iq -> output_Instruction -> instruction_string << endl;
			//iq -> output_Instruction -> tail = rob_array -> end_pt;	
		}

		//insertion in LSQ
		if( iq -> output_Instruction -> opcode1 == "LOAD" || iq -> output_Instruction -> opcode1 == "STORE"){
				
			
			
			LSQ -> lsqentry[LSQ -> endpt].lsqstring = iq -> output_Instruction -> instruction_string ; 
			LSQ -> lsqentry[LSQ -> endpt].lsq1_address = iq -> output_Instruction -> source1_register ;
			LSQ -> lsqentry[LSQ -> endpt].lsq1_value = iq -> output_Instruction -> source1_val ;
			LSQ -> lsqentry[LSQ -> endpt].lsq1_valid = iq -> output_Instruction -> src1_valid ;
			LSQ -> lsqentry[LSQ -> endpt].lsq2_address = iq -> output_Instruction -> source2_register; 
			LSQ -> lsqentry[LSQ -> endpt].lsq2_value = iq -> output_Instruction -> source2_val; 
			LSQ -> lsqentry[LSQ -> endpt].lsq2_valid = iq -> output_Instruction -> src2_valid; 
			LSQ -> lsqentry[LSQ -> endpt].lsqdest = iq -> output_Instruction -> dest_register;
			LSQ -> lsqentry[LSQ -> endpt].lopcode =	iq -> output_Instruction -> opcode1;
			LSQ -> lsqentry[LSQ -> endpt].rename_destination = iq -> output_Instruction -> rename_destination;
			LSQ -> lsqentry[LSQ -> endpt].ldispatch	= cyc -> cycle + 1;
			LSQ -> lsqentry[LSQ -> endpt].tail = rob_array -> end_pt;
			LSQ -> lsqentry[LSQ -> endpt].valid = true;
			LSQ -> lsqentry[LSQ -> endpt].mem_addressvalid = false;
		 	if(LSQ -> endpt == 31){
				LSQ -> endpt = 0 ;
			}
			else{
				LSQ -> endpt = LSQ -> endpt + 1;
			}
		 
		}
			
		
				
						//insertion
		for(int w = 0;w<16;w++){
			if( is ->entry[w].Q_valid == false  && iq -> output_Instruction -> instruction_string != "nop" && iq -> output_Instruction -> opcode1 != "nop"){
				branch_rob = false;
				is -> entry[w].Qs1_valid = iq -> output_Instruction -> src1_valid;
				is -> entry[w].Qs1_value = iq -> output_Instruction -> source1_val; 
				is -> entry[w].Qs1_address = iq -> output_Instruction -> source1_register;
				is -> entry[w].Qs2_valid = iq -> output_Instruction -> src2_valid;
				is -> entry[w].Qs2_value = iq -> output_Instruction -> source2_val;
				is -> entry[w].Qs2_address = iq -> output_Instruction -> source2_register;
				is -> entry[w].Qdest = iq -> output_Instruction -> destname_register; // name as string
				//is -> entry[w].Qrename = iq -> output_Instruction -> rename_destination; 		
				is -> entry[w].Qd = iq -> output_Instruction -> rename_destination;// name as int
				is -> entry[w].Qinstruction_string = iq -> output_Instruction -> instruction_string;
				is -> entry[w].Qopcode = iq -> output_Instruction -> opcode1;
				is -> entry[w].Qliteral = iq -> output_Instruction -> literal;  
				is -> entry[w].QPC = iq -> output_Instruction -> PC;
				is -> entry[w].Qrename_destination = iq -> output_Instruction -> rename_destination;
				is -> entry[w].Qdispatch_clk = cyc -> cycle + 1;
				is -> entry[w].Qtail = rob_array -> end_pt;
				is -> entry[w].Qlsqtail = LSQ -> endpt - 1;
				is -> entry[w].Q_valid = true;	
				break;						
			}
		}

		if (iq -> output_Instruction -> instruction_string != "nop" && iq -> output_Instruction -> instruction_string != "" && iq -> output_Instruction -> opcode1 != "nop")
		{

			if(rob_array -> end_pt == 31){
				rob_array -> end_pt = 0;
			}
			else {
				rob_array ->end_pt = rob_array -> end_pt + 1;
			}
		}

		for(int w = 0; w < 16; w++)
		{
			if (is ->entry[w].Q_valid)
			{
				if(!is -> entry[w].Qs1_valid){
					for(int i=0;i<5;i++){
						//cout << "Quora 1 - " << i << endl;
						if(f_b -> busarray[i].valid){
							//cout << "Quora 2 - " << is -> entry[w].Qs1_address << "; " << f_b -> busarray[i].reg_address << endl;
							if(is -> entry[w].Qs1_address == f_b -> busarray[i].reg_address){
								//cout << "Quora 3 - " << w << endl;
								if (is -> entry[w].Qopcode == "BZ")
								{
									if (f_b -> busarray[i].reg_value == 0)
									{
										is -> entry[w].Qs1_value = 1;
									}
									else
									{
										is -> entry[w].Qs1_value = 0;
									}
									
								}
								else if (is -> entry[w].Qopcode == "BNZ")
								{
									if (f_b -> busarray[i].reg_value == 1)
									{
										is -> entry[w].Qs1_value = 1;
									}
									else
									{
										is -> entry[w].Qs1_value = 0;
									}
								}
								else
								{
									is -> entry[w].Qs1_value = f_b -> busarray[i].reg_value;
								}
								
								is -> entry[w].Qs1_valid = true;
							}
						}
					}
					//if (!is -> entry[w].Qs1_valid)
					//is -> entry[w].Qs1_valid = false;
				}

				if(!is -> entry[w].Qs2_valid){
					for(int i=0;i<5;i++){
						if(f_b -> busarray[i].valid){
							//cout << "BUS ADDRESS - " << f_b -> busarray[i].reg_address << endl << "IQ ADDRESS - " << is -> entry[w].Qs2_address << endl;
							if(is -> entry[w].Qs2_address == f_b -> busarray[i].reg_address){
								is -> entry[w].Qs2_value = f_b -> busarray[i].reg_value;
								is -> entry[w].Qs2_valid = true;	
							}		
						}
					}
					//is -> entry[w].Qs2_valid = false;
				}
			}

		}
		
		/*int tempmin_index  = 100;
		int tempmin_index1  = 100;
		int tempmin_index2  = 100;
		int temp; 
		int temp1;*/
		bool mul = false;
		bool div = false;
		bool intfu = false;
		int mul_small = 100000;
		int div_small = 100000;
		int int_small = 100000;
		int mul_index;
		int div_index;
		int int_index;

				
		for(int w =0; w < 16; w++){
			if( is -> entry[w].Qopcode == "MUL" && is -> entry[w].Q_valid && is -> entry[w].Qs1_valid && is -> entry[w].Qs2_valid){
				//cout << "issue chk MUL" << endl;
				if((is -> entry[w].Qdispatch_clk) < mul_small){
					mul_small = is -> entry[w].Qdispatch_clk;
					mul_index = w;
							
				}
				mul = true;						
			}

			else if ( is -> entry[w].Qopcode == "DIV" && is -> entry[w].Q_valid && is -> entry[w].Qs1_valid && is -> entry[w].Qs2_valid){
				//cout << "issue chk DIV" << endl;
				if((is -> entry[w].Qdispatch_clk) < div_small){
					div_small = is -> entry[w].Qdispatch_clk;
					div_index = w;
				}
				div = true;
			}
			else if ( is -> entry[w].Qopcode != "DIV" && is -> entry[w].Qopcode != "MUL" && is -> entry[w].Q_valid && is -> entry[w].Qs1_valid && is -> entry[w].Qs2_valid){
				//cout << "issue chk " << endl;
				if((is -> entry[w].Qdispatch_clk) < int_small){
					int_small = is -> entry[w].Qdispatch_clk;
					int_index = w;
				}
				intfu = true;
			}
		}

		if(!intfu){
			//	cout << "issue chk " << endl;
			exec -> Input_Instruction -> instruction_string = "nop";
 			exec -> Input_Instruction -> opcode1 = "nop";
			//exec -> Input_Instruction -> tail = iq -> output_Instruction -> tail;	
		}

		else if(intfu){
			//	cout << "issue chk " << endl;
			exec -> Input_Instruction -> instruction_string = is -> entry[int_index].Qinstruction_string;
 			exec -> Input_Instruction -> source1_register = is -> entry[int_index].Qs1_address;
			exec -> Input_Instruction -> source1_val = is -> entry[int_index].Qs1_value;
			exec -> Input_Instruction -> source2_register = is -> entry[int_index].Qs2_address;
			exec -> Input_Instruction -> source2_val = is -> entry[int_index].Qs2_value;
			exec -> Input_Instruction -> destname_register = is -> entry[int_index].Qdest;
			exec -> Input_Instruction -> dest_register = is -> entry[int_index].Qd;
			exec -> Input_Instruction -> literal = is -> entry[int_index].Qliteral;
			exec -> Input_Instruction -> opcode1 = is -> entry[int_index].Qopcode;
			exec -> Input_Instruction -> PC = is -> entry[int_index].QPC;
			exec -> Input_Instruction -> tail = is -> entry[int_index].Qtail;
			exec -> Input_Instruction -> lsq_entry = is -> entry[int_index].Qlsqtail;
			exec -> Input_Instruction -> rename_destination = is -> entry[int_index].Qrename_destination;
			exec -> Input_Instruction -> dispatch = is -> entry[int_index].Qdispatch_clk;

					
			is -> entry[int_index].Q_valid = false;
				
		}
		
		if(!mul){
			exec_m1 -> Input_Instruction -> instruction_string = "nop";
			exec_m1 -> Input_Instruction -> opcode1 = "nop";	
		}

		else if (mul){
 			exec_m1 -> Input_Instruction -> instruction_string = is -> entry[mul_index].Qinstruction_string;
 			exec_m1 -> Input_Instruction -> source1_register = is -> entry[mul_index].Qs1_address;
			exec_m1 -> Input_Instruction -> source1_val = is -> entry[mul_index].Qs1_value;
			exec_m1 -> Input_Instruction -> source2_register = is -> entry[mul_index].Qs2_address;
			exec_m1 -> Input_Instruction -> source2_val = is -> entry[mul_index].Qs2_value;
			exec_m1 -> Input_Instruction -> destname_register = is -> entry[mul_index].Qdest;
			exec_m1 -> Input_Instruction -> dest_register = is -> entry[mul_index].Qd;
			exec_m1 -> Input_Instruction -> literal = is -> entry[mul_index].Qliteral;
			exec_m1 -> Input_Instruction -> opcode1 = is -> entry[mul_index].Qopcode;
			exec_m1 -> Input_Instruction -> PC = is -> entry[mul_index].QPC;
			exec_m1 -> Input_Instruction -> rename_destination = iq -> output_Instruction -> rename_destination;
			exec_m1 -> Input_Instruction -> tail = is -> entry[mul_index].Qtail;
			is -> entry[mul_index].Q_valid = false;			
		}
	
		if(!div){

			exec_d1 -> Input_Instruction -> instruction_string = "nop";
			exec_d1 -> Input_Instruction -> opcode1 = "nop";
			//exec_d1 -> Input_Instruction -> tail = iq -> output_Instruction -> tail;	
		}

		else if(div){

			exec_d1 -> Input_Instruction -> instruction_string = is -> entry[div_index].Qinstruction_string;
 			exec_d1 -> Input_Instruction -> source1_register = is -> entry[div_index].Qs1_address;
			exec_d1 -> Input_Instruction -> source1_val = is -> entry[div_index].Qs1_value;
			exec_d1 -> Input_Instruction -> source2_register = is -> entry[div_index].Qs2_address;
			exec_d1 -> Input_Instruction -> source2_val = is -> entry[div_index].Qs2_value;
			exec_d1 -> Input_Instruction -> destname_register = is -> entry[div_index].Qdest;
			exec_d1 -> Input_Instruction -> dest_register = is -> entry[div_index].Qd;
			exec_d1 -> Input_Instruction -> literal = is -> entry[div_index].Qliteral;
			exec_d1 -> Input_Instruction -> opcode1 = is -> entry[div_index].Qopcode;
			exec_d1 -> Input_Instruction -> PC = is -> entry[div_index].QPC;
			exec_d1 -> Input_Instruction -> rename_destination = iq -> output_Instruction -> rename_destination;
			exec_d1 -> Input_Instruction -> tail = is -> entry[div_index].Qtail;
			is -> entry[div_index].Q_valid = false;
		
		}
		exec -> next_stage = true;
	}
}



//void Execute(stage* exec, stage* mem,stage* exec_m1, stage* exec_m2,stage* exec_d1,stage* exec_d2,stage* exec_d3,stage* exec_d4,stage* mem_temp,stage* mem2_temp,Flags* flg,bus* f_b){
/*void Execute(stage* exec, stage* mem,stage* exec_m1, stage* exec_m2,stage* exec_d1,stage* exec_d2,stage* exec_d3,stage* exec_d4,stage* mem_temp,stage* mem2_temp,Flags* flg,Stats * cyc,rob *ROB){*/
void Execute(stage* exec, stage* mem,stage* exec_m1, stage* exec_m2,stage* exec_d1,stage* exec_d2,stage* exec_d3,stage* exec_d4,stage* mem_temp,stage* mem2_temp,Flags* flg,Stats * cyc,Register_File* r_f,Rename_Table* r_t,bus* f_b,stage* ROB, rob* rob_array, lsq* LSQ){
	string opcode_exec;
	int src1;
	int src2;
	int ltrl;
	if (exec -> next_stage)
	{
		
//div 4

		if (exec_d4 -> Input_Instruction -> opcode1 == "DIV" || exec_d4 -> Input_Instruction -> opcode1 == "HALT"){
			exec_d4 -> output_Instruction -> instruction_string = exec_d4 -> Input_Instruction -> instruction_string;
			exec_d4 -> output_Instruction -> source1_register = exec_d4 -> Input_Instruction -> source1_register;
			exec_d4 -> output_Instruction -> source2_register = exec_d4 -> Input_Instruction -> source2_register;
			exec_d4 -> output_Instruction -> dest_register = exec_d4 -> Input_Instruction -> dest_register;
			exec_d4 -> output_Instruction -> computed_val = exec_d4 -> Input_Instruction -> computed_val;
			exec_d4 -> output_Instruction -> literal = exec_d4 -> Input_Instruction -> literal;
			exec_d4 -> output_Instruction -> opcode1 = exec_d4 -> Input_Instruction -> opcode1;
			exec_d4 -> output_Instruction -> PC = exec_d4 -> Input_Instruction -> PC;
			exec_d4 -> output_Instruction -> rename_destination = exec_d4 -> Input_Instruction -> rename_destination;
			exec_d4 -> output_Instruction -> tail = exec_d4 -> Input_Instruction -> tail;

			r_f -> phy[exec_d4 -> output_Instruction -> dest_register].status = true ;//rename table
			r_f -> phy[exec_d4 -> output_Instruction -> dest_register].value = exec_d4 -> output_Instruction -> computed_val;
			if(exec_d4 -> output_Instruction -> computed_val == 0){
				r_f -> phy[exec_d4 -> output_Instruction -> dest_register].cc = true ;}
			else {
				r_f -> phy[exec_d4 -> output_Instruction -> dest_register].cc = false ;
			}	


			f_b -> busarray[4].reg_value = exec_d4 -> output_Instruction -> computed_val;
			f_b -> busarray[4].reg_address = exec_d4 -> output_Instruction -> dest_register;

			f_b -> busarray[4].valid = true;
			rob_array -> robentry[exec_d4 -> output_Instruction -> tail].status = true; //rob status
			rob_array -> robentry[exec_d4 -> output_Instruction -> tail].destresult = exec_d4 -> output_Instruction -> computed_val; //rob dest value
			
			ROB -> next_stage = true;

		}
		
//div 4 nop part

		if(exec_d4 -> Input_Instruction -> opcode1 != "DIV" && exec_d4 -> Input_Instruction -> opcode1 != "HALT"){
      			exec_d4 -> output_Instruction -> instruction_string = exec_d4 -> Input_Instruction -> instruction_string;
			exec_d4 -> output_Instruction -> source1_register = exec_d4 -> Input_Instruction -> source1_register;
			exec_d4 -> output_Instruction -> source2_register = exec_d4 -> Input_Instruction -> source2_register;
			exec_d4 -> output_Instruction -> dest_register = exec_d4 -> Input_Instruction -> dest_register;
			exec_d4 -> output_Instruction -> literal = exec_d4 -> Input_Instruction -> literal;
			exec_d4 -> output_Instruction -> opcode1 = exec_d4 -> Input_Instruction -> opcode1;
			exec_d4 -> output_Instruction -> PC = exec_d4 -> Input_Instruction -> PC;
			
			f_b -> busarray[4].valid = false;
			exec_d4 -> output_Instruction -> tail = exec_d4 -> Input_Instruction -> tail;


    		}	
		
//div 3

		if (exec_d3 -> Input_Instruction -> opcode1 == "DIV" || exec_d3 -> Input_Instruction -> opcode1 == "HALT"){
			exec_d3 -> output_Instruction -> instruction_string = exec_d3 -> Input_Instruction -> instruction_string;
			exec_d3 -> output_Instruction -> source1_register = exec_d3 -> Input_Instruction -> source1_register;
			exec_d3 -> output_Instruction -> source2_register = exec_d3 -> Input_Instruction -> source2_register;
			exec_d3 -> output_Instruction -> dest_register = exec_d3 -> Input_Instruction -> dest_register;
			exec_d3 -> output_Instruction -> computed_val = exec_d3 -> Input_Instruction -> computed_val;
			exec_d3 -> output_Instruction -> literal = exec_d3 -> Input_Instruction -> literal;
			exec_d3 -> output_Instruction -> opcode1 = exec_d3 -> Input_Instruction -> opcode1;
			exec_d3 -> output_Instruction -> PC = exec_d3 -> Input_Instruction -> PC;
			exec_d3 -> output_Instruction -> tail = exec_d3 -> Input_Instruction -> tail;

			exec_d4 -> Input_Instruction -> instruction_string = exec_d3 -> output_Instruction -> instruction_string;
			exec_d4 -> Input_Instruction -> source1_register = exec_d3 -> output_Instruction -> source1_register;
			exec_d4 -> Input_Instruction -> source2_register = exec_d3 -> output_Instruction -> source2_register;
			exec_d4 -> Input_Instruction -> dest_register = exec_d3 -> output_Instruction -> dest_register;
			exec_d4 -> Input_Instruction -> rename_destination = exec_d3 -> output_Instruction -> rename_destination;        
			exec_d4 -> Input_Instruction -> computed_val = exec_d3 -> output_Instruction -> computed_val;
			exec_d4 -> Input_Instruction -> literal = exec_d3 -> output_Instruction -> literal;
			exec_d4 -> Input_Instruction -> opcode1 = exec_d3 -> output_Instruction -> opcode1;
			exec_d4 -> Input_Instruction -> PC = exec_d3 -> output_Instruction -> PC;
			exec_d4 -> Input_Instruction -> tail = exec_d3 -> output_Instruction -> tail;
		}


//div3 nop part

		
		if(exec_d3 -> Input_Instruction -> opcode1 != "DIV" && exec_d3 -> Input_Instruction -> opcode1 != "HALT"){
			exec_d3 -> output_Instruction -> instruction_string = exec_d3 -> Input_Instruction -> instruction_string;
			exec_d3 -> output_Instruction -> source1_register = exec_d3 -> Input_Instruction -> source1_register;
			exec_d3 -> output_Instruction -> source2_register = exec_d3 -> Input_Instruction -> source2_register;
			exec_d3 -> output_Instruction -> dest_register = exec_d3 -> Input_Instruction -> dest_register;
			exec_d3 -> output_Instruction -> literal = exec_d3 -> Input_Instruction -> literal;
			exec_d3 -> output_Instruction -> opcode1 = exec_d3 -> Input_Instruction -> opcode1;
			exec_d3 -> output_Instruction -> PC = exec_d3 -> Input_Instruction -> PC;
			exec_d3 -> output_Instruction -> tail = exec_d3 -> Input_Instruction -> tail;
			
			exec_d4 -> Input_Instruction -> instruction_string = exec_d3 -> output_Instruction -> instruction_string;
			exec_d4 -> Input_Instruction -> source1_register = exec_d3 -> output_Instruction -> source1_register;
			exec_d4 -> Input_Instruction -> source2_register = exec_d3 -> output_Instruction -> source2_register;
			exec_d4 -> Input_Instruction -> dest_register = exec_d3 -> output_Instruction -> dest_register;
			exec_d4 -> Input_Instruction -> rename_destination = exec_d3 -> output_Instruction -> rename_destination;
			exec_d4 -> Input_Instruction -> literal = exec_d3 -> output_Instruction -> literal;
			exec_d4 -> Input_Instruction -> opcode1 = exec_d3 -> output_Instruction -> opcode1;
			exec_d4 -> Input_Instruction -> PC = exec_d3 -> output_Instruction -> PC;
			exec_d4 -> Input_Instruction -> tail = exec_d3 -> output_Instruction -> tail;
			
    		}

//div 2
		if (exec_d2 -> Input_Instruction -> opcode1 == "DIV" || exec_d2 -> Input_Instruction -> opcode1 == "HALT"){
			exec_d2 -> output_Instruction -> instruction_string = exec_d2 -> Input_Instruction -> instruction_string;
			exec_d2 -> output_Instruction -> source1_register = exec_d2 -> Input_Instruction -> source1_register;
			exec_d2 -> output_Instruction -> source2_register = exec_d2 -> Input_Instruction -> source2_register;
			exec_d2 -> output_Instruction -> dest_register = exec_d2 -> Input_Instruction -> dest_register;
			exec_d2 -> output_Instruction -> computed_val = exec_d2 -> Input_Instruction -> computed_val;
			exec_d2 -> output_Instruction -> literal = exec_d2 -> Input_Instruction -> literal;
			exec_d2 -> output_Instruction -> opcode1 = exec_d2 -> Input_Instruction -> opcode1;
			exec_d2 -> output_Instruction -> PC = exec_d2 -> Input_Instruction -> PC;
			exec_d2 -> output_Instruction -> tail = exec_d2 -> Input_Instruction -> tail;

			exec_d3 -> Input_Instruction -> instruction_string = exec_d2 -> output_Instruction -> instruction_string;
			exec_d3 -> Input_Instruction -> source1_register = exec_d2 -> output_Instruction -> source1_register;
			exec_d3 -> Input_Instruction -> source2_register = exec_d2 -> output_Instruction -> source2_register;
			exec_d3 -> Input_Instruction -> dest_register = exec_d2 -> output_Instruction -> dest_register; 
			exec_d3 -> Input_Instruction -> rename_destination = exec_d2 -> output_Instruction -> rename_destination;       
			exec_d3 -> Input_Instruction -> computed_val = exec_d2 -> output_Instruction -> computed_val;
			exec_d3 -> Input_Instruction -> literal = exec_d2 -> output_Instruction -> literal;
			exec_d3 -> Input_Instruction -> opcode1 = exec_d2 -> output_Instruction -> opcode1;
			exec_d3 -> Input_Instruction -> PC = exec_d2 -> output_Instruction -> PC;
			exec_d3 -> Input_Instruction -> tail = exec_d2 -> output_Instruction -> tail;	
		}

//div2 nop part

		
		if(exec_d2 -> Input_Instruction -> opcode1 != "DIV" && exec_d2 -> Input_Instruction -> opcode1 != "HALT"){
	      		exec_d2 -> output_Instruction -> instruction_string = exec_d2 -> Input_Instruction -> instruction_string;
			exec_d2 -> output_Instruction -> source1_register = exec_d2 -> Input_Instruction -> source1_register;
			exec_d2 -> output_Instruction -> source2_register = exec_d2 -> Input_Instruction -> source2_register;
			exec_d2 -> output_Instruction -> dest_register = exec_d2 -> Input_Instruction -> dest_register;
			exec_d2 -> output_Instruction -> literal = exec_d2 -> Input_Instruction -> literal;
			exec_d2 -> output_Instruction -> opcode1 = exec_d2 -> Input_Instruction -> opcode1;
			exec_d2 -> output_Instruction -> PC = exec_d2 -> Input_Instruction -> PC;
			exec_d2 -> output_Instruction -> tail = exec_d2 -> Input_Instruction -> tail;
			
			exec_d3 -> Input_Instruction -> instruction_string = exec_d2 -> output_Instruction -> instruction_string;
			exec_d3 -> Input_Instruction -> source1_register = exec_d2 -> output_Instruction -> source1_register;
			exec_d3 -> Input_Instruction -> source2_register = exec_d2 -> output_Instruction -> source2_register;
			exec_d3 -> Input_Instruction -> dest_register = exec_d2 -> output_Instruction -> dest_register;
			exec_d3 -> Input_Instruction -> rename_destination = exec_d2 -> output_Instruction -> rename_destination; 
			exec_d3 -> Input_Instruction -> literal = exec_d2 -> output_Instruction -> literal;
			exec_d3 -> Input_Instruction -> opcode1 = exec_d2 -> output_Instruction -> opcode1;
			exec_d3 -> Input_Instruction -> PC = exec_d2 -> output_Instruction -> PC;
			exec_d3 -> Input_Instruction -> tail = exec_d2 -> output_Instruction -> tail;
    		}

//mul 2
		if (exec_m2 -> Input_Instruction -> opcode1 == "MUL" && !exec_m2 -> Input_Instruction -> stalled){
			exec_m2 -> output_Instruction -> instruction_string = exec_m2 -> Input_Instruction -> instruction_string;
			exec_m2 -> output_Instruction -> source1_register = exec_m2 -> Input_Instruction -> source1_register;
			exec_m2 -> output_Instruction -> source2_register = exec_m2 -> Input_Instruction -> source2_register;
			exec_m2 -> output_Instruction -> dest_register = exec_m2 -> Input_Instruction -> dest_register;
			exec_m2 -> output_Instruction -> computed_val = exec_m2 -> Input_Instruction -> computed_val;
			exec_m2 -> output_Instruction -> literal = exec_m2 -> Input_Instruction -> literal;
			exec_m2 -> output_Instruction -> rename_destination = exec_m2 -> Input_Instruction -> rename_destination; 
			exec_m2 -> output_Instruction -> opcode1 = exec_m2 -> Input_Instruction -> opcode1;
			exec_m2 -> output_Instruction -> PC = exec_m2 -> Input_Instruction -> PC;
			exec_m2 -> output_Instruction -> tail = exec_m2 -> Input_Instruction -> tail;
			r_f -> phy[exec_m2 -> output_Instruction -> dest_register].status = true ;//rename table
			r_f -> phy[exec_m2 -> output_Instruction -> dest_register].value = exec_m2 -> output_Instruction -> computed_val;
			if(exec_m2 -> output_Instruction -> computed_val == 0){
				r_f -> phy[exec_m2 -> output_Instruction -> dest_register].cc = true ;}
			else {
				r_f -> phy[exec_m2 -> output_Instruction -> dest_register].cc = false ;
			}	


			f_b -> busarray[3].reg_value = exec_m2 -> output_Instruction -> computed_val;
			f_b -> busarray[3].reg_address = exec_m2 -> output_Instruction -> dest_register;
			f_b -> busarray[3].valid = true;

			rob_array -> robentry[exec_m2 -> output_Instruction -> tail].status = true; //rob status
			rob_array -> robentry[exec_m2 -> output_Instruction -> tail].destresult = exec_m2 -> output_Instruction -> computed_val; //rob dest value
		 		
			ROB -> next_stage = true;		


		}
//mul 2 nop part
		if(exec_m2 -> output_Instruction -> instruction_string != "nop" && exec_m2 -> Input_Instruction -> instruction_string == "nop" && !exec_m2 -> Input_Instruction -> stalled){
      			exec_m2 -> output_Instruction -> instruction_string = exec_m2 -> Input_Instruction -> instruction_string;
			
			exec_m2 -> output_Instruction -> source1_register = exec_m2 -> Input_Instruction -> source1_register;
			exec_m2 -> output_Instruction -> source2_register = exec_m2 -> Input_Instruction -> source2_register;
			exec_m2 -> output_Instruction -> dest_register = exec_m2 -> Input_Instruction -> dest_register;
			exec_m2 -> output_Instruction -> rename_destination = exec_m2 -> Input_Instruction -> rename_destination;	
			exec_m2 -> output_Instruction -> literal = exec_m2 -> Input_Instruction -> literal;
			exec_m2 -> output_Instruction -> opcode1 = exec_m2 -> Input_Instruction -> opcode1;
			exec_m2 -> output_Instruction -> PC = exec_m2 -> Input_Instruction -> PC;
			f_b -> busarray[3].valid = false;
			exec_m2 -> output_Instruction -> tail = exec_m2 -> Input_Instruction -> tail;


    		}	


//div 1

		if (!exec_d1 -> Input_Instruction -> stalled){
			exec_d1 -> output_Instruction -> instruction_string = exec_d1 -> Input_Instruction -> instruction_string;
			exec_d1 -> output_Instruction -> source1_register = exec_d1-> Input_Instruction -> source1_register;
			exec_d1 -> output_Instruction -> source1_val = exec_d1 -> Input_Instruction -> source1_val;
			exec_d1 -> output_Instruction -> source2_register = exec_d1 -> Input_Instruction -> source2_register;
			exec_d1 -> output_Instruction -> source2_val = exec_d1 -> Input_Instruction -> source2_val;
			exec_d1 -> output_Instruction -> dest_register = exec_d1 -> Input_Instruction -> dest_register;
			exec_d1 -> output_Instruction -> rename_destination = exec_d1 -> Input_Instruction -> rename_destination;
			exec_d1 -> output_Instruction -> literal = exec_d1 -> Input_Instruction -> literal;
			exec_d1 -> output_Instruction -> opcode1 = exec_d1 -> Input_Instruction -> opcode1;
			exec_d1 -> output_Instruction -> PC = exec_d1 -> Input_Instruction -> PC;
			exec_d1 -> output_Instruction -> tail = exec_d1 -> Input_Instruction -> tail;
			

			opcode_exec = exec_d1 -> output_Instruction -> opcode1;	
			if (opcode_exec == "DIV"){
				if (exec_d1 -> output_Instruction -> source2_val != 0)
				{
					exec_d1 -> output_Instruction -> computed_val = (exec_d1 -> output_Instruction -> source1_val / exec_d1 -> output_Instruction -> source2_val);
				}
				else
				{
					cout << "Floating point Exception" << endl;
				}
								
				
				
				
			}
			else if(opcode_exec == "HALT"){
				halt_execute = true;
			}

			exec_d2 -> Input_Instruction -> instruction_string = exec_d1 -> output_Instruction -> instruction_string;
			exec_d2 -> Input_Instruction -> source1_register = exec_d1 -> output_Instruction -> source1_register;
			exec_d2 -> Input_Instruction -> source2_register = exec_d1 -> output_Instruction -> source2_register;
			exec_d2 -> Input_Instruction -> dest_register = exec_d1 -> output_Instruction -> dest_register;
			exec_d2 -> Input_Instruction -> rename_destination = exec_d1 -> output_Instruction -> rename_destination;
			exec_d2 -> Input_Instruction -> literal = exec_d1 -> output_Instruction -> literal;
			exec_d2 -> Input_Instruction -> opcode1 = exec_d1 -> output_Instruction -> opcode1;
			exec_d2 -> Input_Instruction -> PC = exec_d1 -> output_Instruction -> PC;
			exec_d2 -> Input_Instruction -> computed_val = exec_d1 -> output_Instruction -> computed_val;
			exec_d2 -> Input_Instruction -> tail = exec_d1 -> output_Instruction -> tail;
		}

//div1 nop part

		else{
	      	exec_d1 -> output_Instruction -> instruction_string = "nop";
			exec_d1 -> output_Instruction -> opcode1 = "nop";
		
			exec_d2 -> Input_Instruction -> instruction_string = exec_d1 -> output_Instruction -> instruction_string;
			exec_d2 -> Input_Instruction -> opcode1 = exec_d1 -> output_Instruction -> opcode1;
    		}


//mul1 part      
   		if (exec_m1 -> Input_Instruction -> opcode1 == "MUL" && !exec_m1 -> Input_Instruction -> stalled){
			exec_m1 -> output_Instruction -> instruction_string = exec_m1 -> Input_Instruction -> instruction_string;
			exec_m1 -> output_Instruction -> source1_register = exec_m1-> Input_Instruction -> source1_register;
			exec_m1 -> output_Instruction -> source1_val = exec_m1 -> Input_Instruction -> source1_val;
			exec_m1 -> output_Instruction -> source2_register = exec_m1 -> Input_Instruction -> source2_register;
			exec_m1 -> output_Instruction -> source2_val = exec_m1 -> Input_Instruction -> source2_val;
			exec_m1 -> output_Instruction -> dest_register = exec_m1 -> Input_Instruction -> dest_register;
			exec_m1 -> output_Instruction -> rename_destination = exec_m1 -> Input_Instruction -> rename_destination;
			exec_m1 -> output_Instruction -> literal = exec_m1 -> Input_Instruction -> literal;
			exec_m1 -> output_Instruction -> opcode1 = exec_m1 -> Input_Instruction -> opcode1;
			exec_m1 -> output_Instruction -> PC = exec_m1 -> Input_Instruction -> PC;
			exec_m1 -> output_Instruction -> tail = exec_m1 -> Input_Instruction -> tail;
			

			opcode_exec = exec_m1 -> output_Instruction -> opcode1;	
			if (opcode_exec == "MUL"){
				exec_m1 -> output_Instruction -> computed_val = (exec_m1 -> output_Instruction -> source1_val * exec_m1 -> output_Instruction -> source2_val);
			}

			exec_m2 -> Input_Instruction -> instruction_string = exec_m1 -> output_Instruction -> instruction_string;
			exec_m2 -> Input_Instruction -> source1_register = exec_m1 -> output_Instruction -> source1_register;
			exec_m2 -> Input_Instruction -> source2_register = exec_m1 -> output_Instruction -> source2_register;
			exec_m2 -> Input_Instruction -> dest_register = exec_m1 -> output_Instruction -> dest_register;
			exec_m2 -> Input_Instruction -> rename_destination = exec_m1 -> output_Instruction -> rename_destination;
			exec_m2 -> Input_Instruction -> literal = exec_m1 -> output_Instruction -> literal;
			exec_m2 -> Input_Instruction -> opcode1 = exec_m1 -> output_Instruction -> opcode1;
			exec_m2 -> Input_Instruction -> PC = exec_m1 -> output_Instruction -> PC;
			exec_m2 -> Input_Instruction -> computed_val = exec_m1 -> output_Instruction -> computed_val;
			exec_m2 -> Input_Instruction -> tail = exec_m1 -> output_Instruction -> tail;
			
		}

		else
		{
			exec_m1 -> output_Instruction -> instruction_string = "nop";
			exec_m1 -> output_Instruction -> opcode1 = "nop";
			
		
			exec_m2 -> Input_Instruction -> instruction_string = exec_m1 -> output_Instruction -> instruction_string;
			exec_m2 -> Input_Instruction -> opcode1 = exec_m1 -> output_Instruction -> opcode1;	
		}

//intfu part






		if (exec_m1 -> Input_Instruction -> instruction_string == "nop" && exec_m2 -> Input_Instruction -> instruction_string == "nop"  && exec -> Input_Instruction -> stalled && exec -> output_Instruction -> instruction_string != "nop" && mem -> Input_Instruction -> instruction_string == "nop" && exec -> output_Instruction -> stalled)
		{
			exec -> output_Instruction -> stalled = false;
			exec_stall = false;
		}
		else if (!exec -> Input_Instruction -> stalled && !exec -> output_Instruction -> stalled){
			exec -> output_Instruction -> instruction_string = exec -> Input_Instruction -> instruction_string;
			exec -> output_Instruction -> source1_register = exec-> Input_Instruction -> source1_register;
			exec -> output_Instruction -> source1_val = exec -> Input_Instruction -> source1_val;
			exec -> output_Instruction -> source2_register = exec -> Input_Instruction -> source2_register;
			exec -> output_Instruction -> source2_val = exec -> Input_Instruction -> source2_val;
			exec -> output_Instruction -> dest_register = exec -> Input_Instruction -> dest_register;
			exec -> output_Instruction -> rename_destination = exec -> Input_Instruction -> rename_destination;
			exec -> output_Instruction -> literal = exec -> Input_Instruction -> literal;
			exec -> output_Instruction -> opcode1 = exec -> Input_Instruction -> opcode1;
			exec -> output_Instruction -> PC = exec -> Input_Instruction -> PC;
			exec -> output_Instruction -> tail = exec -> Input_Instruction -> tail;
			exec -> output_Instruction -> lsq_entry = exec -> Input_Instruction -> lsq_entry;
			exec -> output_Instruction -> dispatch = exec -> Input_Instruction -> dispatch;
			
			
			opcode_exec = exec -> output_Instruction -> opcode1;
			ltrl = exec -> output_Instruction -> literal;
	
			if (opcode_exec == "ADD"){
					
				exec -> output_Instruction -> computed_val = exec -> output_Instruction -> source1_val + exec -> output_Instruction -> source2_val;
				//cout << "addition output " << exec -> output_Instruction -> computed_val << endl;
				r_f -> phy[exec -> output_Instruction -> dest_register].status = true ;//rename table
				r_f -> phy[exec -> output_Instruction -> dest_register].value = exec -> output_Instruction -> computed_val;
				if(exec -> output_Instruction -> computed_val == 0){
				r_f -> phy[exec -> output_Instruction -> dest_register].cc = true ;}
				else {
				r_f -> phy[exec -> output_Instruction -> dest_register].cc = false ;
				}	



				f_b -> busarray[2].reg_value = exec -> output_Instruction -> computed_val;
				f_b -> busarray[2].reg_address = exec -> output_Instruction -> rename_destination;
				f_b -> busarray[2].valid = true;
				//cout << "int branch val :" << f_b -> busarray[2].reg_value << endl; 
				//cout << "Tail :" << exec -> output_Instruction -> tail << endl;
				//cout << "ADD EX 1 - " << exec -> output_Instruction -> source1_val << endl;
				//cout << "ADD EX 2 - " << exec -> output_Instruction -> source2_val << endl;
				rob_array -> robentry[exec -> output_Instruction -> tail].status = true; //rob status
				rob_array -> robentry[exec -> output_Instruction -> tail].destresult = exec -> output_Instruction -> computed_val; //rob dest value
					
			}

			else if (opcode_exec == "SUB"){
					
				exec -> output_Instruction -> computed_val = exec -> output_Instruction -> source1_val - exec -> output_Instruction -> source2_val;
				r_f -> phy[exec -> output_Instruction -> dest_register].status = true ;//rename table
				r_f -> phy[exec -> output_Instruction -> dest_register].value = exec -> output_Instruction -> computed_val;
				if(exec -> output_Instruction -> computed_val == 0){
				r_f -> phy[exec -> output_Instruction -> dest_register].cc = true ;}
				else {
				r_f -> phy[exec -> output_Instruction -> dest_register].cc = false ;
				}	


				f_b -> busarray[2].reg_value = exec -> output_Instruction -> computed_val;
				f_b -> busarray[2].reg_address = exec -> output_Instruction -> rename_destination;
				f_b -> busarray[2].valid = true;
				rob_array -> robentry[exec -> output_Instruction -> tail].status = true; //rob status
				rob_array -> robentry[exec -> output_Instruction -> tail].destresult = exec -> output_Instruction -> computed_val; //rob dest value
				
			}

			

			else if (opcode_exec == "EXOR"){
					
				exec -> output_Instruction -> computed_val = (exec -> output_Instruction -> source1_val) ^ exec -> output_Instruction -> source2_val;
				r_f -> phy[exec -> output_Instruction -> dest_register].status = true ;//rename table
				r_f -> phy[exec -> output_Instruction -> dest_register].value = exec -> output_Instruction -> computed_val;
				f_b -> busarray[2].reg_value = exec -> output_Instruction -> computed_val;
				f_b -> busarray[2].reg_address = exec -> output_Instruction -> rename_destination;
				f_b -> busarray[2].valid = true;
				rob_array -> robentry[exec -> output_Instruction -> tail].status = true; //rob status
				rob_array -> robentry[exec -> output_Instruction -> tail].destresult = exec -> output_Instruction -> computed_val; //rob dest value

					
			}

			else if (opcode_exec == "OR"){
					
				exec -> output_Instruction -> computed_val = (exec -> output_Instruction -> source1_val) || exec -> output_Instruction -> source2_val;
				r_f -> phy[exec -> output_Instruction -> dest_register].status = true ;//rename table
				r_f -> phy[exec -> output_Instruction -> dest_register].value = exec -> output_Instruction -> computed_val;
				f_b -> busarray[2].reg_value = exec -> output_Instruction -> computed_val;
				f_b -> busarray[2].reg_address = exec -> output_Instruction -> rename_destination;
				f_b -> busarray[2].valid = true;
				rob_array -> robentry[exec -> output_Instruction -> tail].status = true; //rob status
				rob_array -> robentry[exec -> output_Instruction -> tail].destresult = exec -> output_Instruction -> computed_val; //rob dest value
			}

			else if (opcode_exec == "AND"){
					
				exec -> output_Instruction -> computed_val = (exec -> output_Instruction -> source1_val) && exec -> output_Instruction -> source2_val;
				r_f -> phy[exec -> output_Instruction -> dest_register].status = true ;//rename table
				r_f -> phy[exec -> output_Instruction -> dest_register].value = exec -> output_Instruction -> computed_val;
				f_b -> busarray[2].reg_value = exec -> output_Instruction -> computed_val;
				f_b -> busarray[2].reg_address = exec -> output_Instruction -> rename_destination;
				f_b -> busarray[2].valid = true;
				rob_array -> robentry[exec -> output_Instruction -> tail].status = true; //rob status
				rob_array -> robentry[exec -> output_Instruction -> tail].destresult = exec -> output_Instruction -> computed_val; //rob dest value
			}

			else if ((opcode_exec == "ADDL")||(opcode_exec == "LOAD")){
					
				exec -> output_Instruction -> computed_val = exec -> output_Instruction -> source1_val + ltrl;
				//cout << endl << exec -> output_Instruction -> source1_val <<" sds" <<ltrl <<"print"<< endl;
				//r_f -> phy[exec -> output_Instruction -> dest_register].status = true ;//rename table
				//r_f -> phy[exec -> output_Instruction -> dest_register].value = exec -> output_Instruction -> computed_val;
				//f_b -> busarray[2].reg_value = exec -> output_Instruction -> computed_val;
				//f_b -> busarray[2].reg_address = exec -> output_Instruction -> rename_destination;
				f_b -> busarray[2].valid = false;
				//rob_array -> robentry[exec -> output_Instruction -> tail].status = false; //rob status
				//rob_array -> robentry[exec -> output_Instruction -> tail].destresult = exec -> output_Instruction -> computed_val; //rob dest value
				//cout << exec -> output_Instruction -> lsq_entry;
				//LSQ -> lsqentry[exec -> output_Instruction -> lsq_entry].mem_addressvalid = true;
				//LSQ -> lsqentry[exec -> output_Instruction -> lsq_entry].mem_address = exec -> output_Instruction -> computed_val;
				for(int i = 0;i< LSQ -> endpt;i++){
					if(LSQ -> lsqentry[i].ldispatch == exec -> output_Instruction -> dispatch){
						LSQ -> lsqentry[i].mem_addressvalid = true;
						LSQ -> lsqentry[i].mem_address = exec -> output_Instruction -> computed_val;
					}
				}

			}

			else if (opcode_exec == "SUBL"){
					
				exec -> output_Instruction -> computed_val = exec -> output_Instruction -> source1_val - ltrl;
				r_f -> phy[exec -> output_Instruction -> dest_register].value = exec -> output_Instruction -> computed_val;
				r_f -> phy[exec -> output_Instruction -> dest_register].status = true ;//rename table
				f_b -> busarray[2].reg_value = exec -> output_Instruction -> computed_val;
				f_b -> busarray[2].reg_address = exec -> output_Instruction -> rename_destination;
				f_b -> busarray[2].valid = true;
				rob_array -> robentry[exec -> output_Instruction -> tail].status = true; //rob status
				rob_array -> robentry[exec -> output_Instruction -> tail].destresult = exec -> output_Instruction -> computed_val; //rob dest value
			}

			else if (opcode_exec == "STORE"){
					
				exec -> output_Instruction -> computed_val = exec -> output_Instruction -> source2_val + ltrl;
				r_f -> phy[exec -> output_Instruction -> dest_register].status = true ;//rename table
				r_f -> phy[exec -> output_Instruction -> dest_register].value = exec -> output_Instruction -> computed_val;
				f_b -> busarray[2].valid = false;
				//rob_array -> robentry[exec -> output_Instruction -> tail].status = true; //rob status
				//rob_array -> robentry[exec -> output_Instruction -> tail].destresult = exec -> output_Instruction -> computed_val; //rob dest value

				//LSQ -> lsqentry[exec -> output_Instruction -> lsq_entry].mem_addressvalid = true;
				//LSQ -> lsqentry[exec -> output_Instruction -> lsq_entry].mem_address = exec -> output_Instruction -> computed_val;
				//cout << exec -> output_Instruction -> lsq_entry << "sd" << endl;
				//cout << "STORE - "<< exec -> output_Instruction -> source1_val <<" sds" <<ltrl <<"print"<< endl;
				for(int i = 0;i< LSQ -> endpt;i++){
					if(LSQ -> lsqentry[i].ldispatch == exec -> output_Instruction -> dispatch){
						LSQ -> lsqentry[i].mem_addressvalid = true;
						LSQ -> lsqentry[i].mem_address = exec -> output_Instruction -> computed_val;
					}
				}
					
			}

			else if (opcode_exec == "MOVC"){
					
				exec -> output_Instruction -> computed_val = ltrl;
				r_f -> phy[exec -> output_Instruction -> dest_register].status = true ;//rename table
				r_f -> phy[exec -> output_Instruction -> dest_register].value = exec -> output_Instruction -> computed_val;
				f_b -> busarray[2].reg_value = exec -> output_Instruction -> computed_val;
				f_b -> busarray[2].reg_address = exec -> output_Instruction -> rename_destination;
				f_b -> busarray[2].valid = true;
				rob_array -> robentry[exec -> output_Instruction -> tail].status = true; //rob status
				rob_array -> robentry[exec -> output_Instruction -> tail].destresult = exec -> output_Instruction -> computed_val; //rob dest value
			}

			else if ((opcode_exec == "BZ") || (opcode_exec == "BNZ")){
				if(opcode_exec == "BZ"){
					if(exec -> output_Instruction -> source1_val == 1){
						//cout << "TRUE" << endl;
						branch_execute = true;
						if(branch_offset){
							exec -> output_Instruction -> computed_val = exec -> output_Instruction -> PC - ltrl;
							//r_f -> phy[exec -> output_Instruction -> dest_register].value = exec -> output_Instruction -> computed_val;
							//rob_array -> robentry[exec -> output_Instruction -> tail].status = true; //rob status
							//rob_array -> robentry[exec -> output_Instruction -> tail].destresult = exec -> output_Instruction -> computed_val; //rob dest value
						}

						else{
							exec -> output_Instruction -> computed_val = exec -> output_Instruction -> PC + ltrl;
							//cout << "exec -> output_Instruction -> computed_val - " << exec -> output_Instruction -> computed_val << endl;
							//r_f -> phy[exec -> output_Instruction -> dest_register].value = exec -> output_Instruction -> computed_val;
							//rob_array -> robentry[exec -> output_Instruction -> tail].status = true; //rob status
							//rob_array -> robentry[exec -> output_Instruction -> tail].destresult = exec -> output_Instruction -> computed_val; //rob dest value
						}
						branch_dispatch_cc = exec -> output_Instruction -> dispatch;
								
					}
					else{
						//cout << "False" << endl;
						branch_execute = false;
					}
				}	
				else{
	
					if(exec -> output_Instruction -> source1_val == 0){
						branch_execute = true;	
						if(branch_offset){
							exec -> output_Instruction -> computed_val = exec -> output_Instruction -> PC - ltrl;
							//r_f -> phy[exec -> output_Instruction -> dest_register].value = exec -> output_Instruction -> computed_val;
							//rob_array -> robentry[exec -> output_Instruction -> tail].status = true; //rob status
							//rob_array -> robentry[exec -> output_Instruction -> tail].destresult = exec -> output_Instruction -> computed_val; //rob dest value
						}

						else{
							exec -> output_Instruction -> computed_val = exec -> output_Instruction -> PC + ltrl;
							//r_f -> phy[exec -> output_Instruction -> dest_register].value = exec -> output_Instruction -> computed_val;
							//rob_array -> robentry[exec -> output_Instruction -> tail].destresult = exec -> output_Instruction -> computed_val; //rob dest value
						}
						branch_dispatch_cc = exec -> output_Instruction -> dispatch;
						
					}
					else{
						branch_execute = false;
					}					
				}
				rob_array -> robentry[exec -> output_Instruction -> tail].status = true; //rob status
				f_b -> busarray[2].valid = false;
					
			}

			else if(opcode_exec == "JUMP"){
				if(jump_neg){
					exec -> output_Instruction -> computed_val =  exec -> output_Instruction -> source1_val - ltrl;
					jump_execute = true;
					//r_f -> phy[exec -> output_Instruction -> dest_register].value = exec -> output_Instruction -> computed_val;
					//r_f -> phy[exec -> output_Instruction -> dest_register].status = true ;//rename table
					rob_array -> robentry[exec -> output_Instruction -> tail].status = true; //rob status
					//rob_array -> robentry[exec -> output_Instruction -> tail].destresult = exec -> output_Instruction -> computed_val; //rob dest value
				}
				else{
					exec -> output_Instruction -> computed_val =  exec -> output_Instruction -> source1_val + ltrl;
					jump_execute = true;
					//r_f -> phy[exec -> output_Instruction -> dest_register].value = exec -> output_Instruction -> computed_val;
					//r_f -> phy[exec -> output_Instruction -> dest_register].status = true ;//rename table
					rob_array -> robentry[exec -> output_Instruction -> tail].status = true; //rob status
					//rob_array -> robentry[exec -> output_Instruction -> tail].destresult = exec -> output_Instruction -> computed_val; //rob dest value
				}
				branch_dispatch_cc = exec -> output_Instruction -> dispatch;
				f_b -> busarray[2].valid = false;
			}

			else if(opcode_exec == "JAL"){
					
				if(jump_neg){
					exec -> output_Instruction -> computed_val = exec -> output_Instruction -> source1_val - ltrl;
					jal_target = exec -> output_Instruction -> PC + 4;
					jump_execute = true;
					r_f -> phy[exec -> output_Instruction -> dest_register].value = exec -> output_Instruction -> computed_val;
					r_f -> phy[exec -> output_Instruction -> dest_register].status = true ;//rename table
					f_b -> busarray[2].reg_value = exec -> output_Instruction -> computed_val;
					f_b -> busarray[2].reg_address = exec -> output_Instruction -> rename_destination;
					f_b -> busarray[2].valid = true;
					rob_array -> robentry[exec -> output_Instruction -> tail].status = true; //rob status
					rob_array -> robentry[exec -> output_Instruction -> tail].destresult = exec -> output_Instruction -> computed_val; //rob dest value
						
				}
				else{
					exec -> output_Instruction -> computed_val = exec -> output_Instruction -> source1_val + ltrl;
					jal_target = exec -> output_Instruction -> PC + 4;
					jump_execute = true;	
					r_f -> phy[exec -> output_Instruction -> dest_register].value = exec -> output_Instruction -> computed_val;
					r_f -> phy[exec -> output_Instruction -> dest_register].status = true ;//rename table
					f_b -> busarray[2].reg_value = exec -> output_Instruction -> computed_val;
					f_b -> busarray[2].reg_address = exec -> output_Instruction -> rename_destination;
					f_b -> busarray[2].valid = true;
					rob_array -> robentry[exec -> output_Instruction -> tail].status = true; //rob status
					rob_array -> robentry[exec -> output_Instruction -> tail].destresult = exec -> output_Instruction -> computed_val; //rob dest value
				}
			}
		

			
			else if (exec -> Input_Instruction -> instruction_string == "nop" && !exec -> output_Instruction -> stalled){
						
				exec -> output_Instruction -> instruction_string = "nop";
				f_b -> busarray[2].valid = false;
			}
		
			//mem -> next_stage = true;    
			ROB -> next_stage = true;  	
			
		}	
	}			
}


void lsq_fn(stage * mem, lsq* LSQ, rob* rob_array, Register_File* r_f,bus* f_b){


	int localstatus = false;
	int k;

	for (int i = 0 ; i < LSQ -> endpt ; i++){
		//cout <<endl<<i<<":"<<LSQ -> lsqentry[i].mem_addressvalid<<LSQ -> lsqentry[i].validcheck<<LSQ -> lsqentry[i].valid<<LSQ -> lsqentry[i].lopcode<<endl;
		if(LSQ -> lsqentry[i].mem_addressvalid && LSQ -> lsqentry[i].lopcode == "LOAD" && !LSQ -> lsqentry[i].validcheck && LSQ -> lsqentry[i].valid  ){
			//cout << "check123" << endl;
			for(int j = i-1 ; j>-1 ;j--){
				//cout << "check12" << endl;
				if( LSQ -> lsqentry[j].mem_addressvalid && LSQ -> lsqentry[j].lopcode == "STORE"){
					if( LSQ -> lsqentry[j].mem_address == LSQ -> lsqentry[i].mem_address)
					{	
						f_b -> busarray[1].reg_value =  LSQ -> lsqentry[j].lsq1_value;
						f_b -> busarray[1].reg_address = LSQ -> lsqentry[j].ldst_value;
						f_b -> busarray[1].valid = true;	
						r_f -> phy[LSQ -> lsqentry[j].ldst_value].value = LSQ -> lsqentry[j].lsq1_value;
						r_f -> phy[LSQ -> lsqentry[j].ldst_value].status = true;
						rob_array -> robentry[LSQ -> lsqentry[i].tail].status = true;
						LSQ -> lsqentry[i].valid = false;
						//cout << LSQ -> endpt<<"SD"<<i<<endl;
						for(int a = i;a <= LSQ ->endpt;a++  ){	
							if(a == LSQ -> endpt){	
								LSQ -> lsqentry[a].valid = false;
							}
							else{
								LSQ -> lsqentry[a].lsqstring = LSQ -> lsqentry[a+1].lsqstring  ; 
								LSQ -> lsqentry[a].lsq1_address = LSQ -> lsqentry[a+1].lsq1_address ;
								LSQ -> lsqentry[a].lsq1_value = LSQ -> lsqentry[a+1].lsq1_value ;
								LSQ -> lsqentry[a].lsq1_valid = LSQ -> lsqentry[a+1].lsq1_valid ;
								LSQ -> lsqentry[a].lsq2_address = LSQ -> lsqentry[a+1].lsq2_address; 
								LSQ -> lsqentry[a].lsq2_value = LSQ -> lsqentry[a+1].lsq2_value; 
								LSQ -> lsqentry[a].lsq2_valid = LSQ -> lsqentry[a+1].lsq2_valid; 
								LSQ -> lsqentry[a].lsqdest = LSQ -> lsqentry[a+1].lsqdest;
								LSQ -> lsqentry[a].ldst_value = LSQ -> lsqentry[a+1].ldst_value;
								LSQ -> lsqentry[a].ldst_address = LSQ -> lsqentry[a+1].ldst_address;
								LSQ -> lsqentry[a].mem_address = LSQ -> lsqentry[a+1].mem_address;
								LSQ -> lsqentry[a].mem_addressvalid = LSQ -> lsqentry[a+1].mem_addressvalid;
								LSQ -> lsqentry[a].tail = LSQ -> lsqentry[a+1].tail;
								LSQ -> lsqentry[a].lopcode = LSQ -> lsqentry[a+1].lopcode;
								LSQ -> lsqentry[a].ldispatch = LSQ -> lsqentry[a+1].ldispatch;
								LSQ -> lsqentry[a].rename_destination = LSQ -> lsqentry[a+1].rename_destination;
								LSQ -> lsqentry[a].validcheck = LSQ -> lsqentry[a+1].validcheck;
								
								LSQ -> lsqentry[a].valid = LSQ -> lsqentry[a+1].valid;
	


							}
						}
						LSQ -> endpt--;
						//cout << "check12" << endl;
						break;	
					}
					else { 
						if(j == 0){
							localstatus = true;
							k = i;

						}				
					}
				}	
									
				else if(LSQ -> lsqentry[j].mem_addressvalid == false && LSQ -> lsqentry[j].lopcode == "STORE"){
					LSQ -> lsqentry[i].validcheck = true;
					break;
				}	
			}
		}



						
	}

			
	if(localstatus){
		//cout << "stepp" << endl;
		LSQ -> temp.lsqstring = LSQ -> lsqentry[k].lsqstring ; 
				 LSQ -> temp.lsq1_address = LSQ -> lsqentry[k].lsq1_address;
				 LSQ -> temp.lsq1_value = LSQ -> lsqentry[k].lsq1_value ;
				 LSQ -> temp.lsq1_valid = LSQ -> lsqentry[k].lsq1_valid;
				 LSQ -> temp.lsq2_address = LSQ -> lsqentry[k].lsq2_address; 
				 LSQ -> temp.lsq2_value = LSQ -> lsqentry[k].lsq2_value; 
				 LSQ -> temp.lsq2_valid = LSQ -> lsqentry[k].lsq2_valid; 
				 LSQ -> temp.lsqdest = LSQ -> lsqentry[k].lsqdest;
				 LSQ -> temp.ldst_address = LSQ -> lsqentry[k].ldst_address;
				 LSQ -> temp.ldst_value = LSQ -> lsqentry[k].ldst_value;
				 LSQ -> temp.mem_address = LSQ -> lsqentry[k].mem_address;
				 LSQ -> temp.mem_addressvalid = LSQ -> lsqentry[k].mem_addressvalid;
				 LSQ -> temp.tail = LSQ -> lsqentry[k].tail;
				 LSQ -> temp.lopcode = LSQ -> lsqentry[k].lopcode;
				 LSQ -> temp.ldispatch = LSQ -> lsqentry[k].ldispatch;
				 LSQ -> temp.rename_destination = LSQ -> lsqentry[k].rename_destination;
				 LSQ -> temp.validcheck = LSQ -> lsqentry[k].validcheck;	
				 LSQ -> temp.valid = LSQ -> lsqentry[k].valid;	
		for( int w = k - 1 ; w >= 0;w--){						
				LSQ -> lsqentry[w+1].lsqstring = LSQ -> lsqentry[w].lsqstring  ; 
				LSQ -> lsqentry[w+1].lsq1_address = LSQ -> lsqentry[w].lsq1_address ;
				LSQ -> lsqentry[w+1].lsq1_value = LSQ -> lsqentry[w].lsq1_value ;
				LSQ -> lsqentry[w+1].lsq1_valid = LSQ -> lsqentry[w].lsq1_valid ;
				LSQ -> lsqentry[w+1].lsq2_address = LSQ -> lsqentry[w].lsq2_address; 
				LSQ -> lsqentry[w+1].lsq2_value = LSQ -> lsqentry[w].lsq2_value; 
				LSQ -> lsqentry[w+1].lsq2_valid = LSQ -> lsqentry[w].lsq2_valid; 
				LSQ -> lsqentry[w+1].lsqdest = LSQ -> lsqentry[w].lsqdest;
				LSQ -> lsqentry[w+1].ldst_value = LSQ -> lsqentry[w].ldst_value;
				LSQ -> lsqentry[w+1].ldst_address = LSQ -> lsqentry[w].ldst_address;
				LSQ -> lsqentry[w+1].mem_address = LSQ -> lsqentry[w].mem_address;
				LSQ -> lsqentry[w+1].mem_addressvalid = LSQ -> lsqentry[w].mem_addressvalid;
				LSQ -> lsqentry[w+1].tail = LSQ -> lsqentry[w].tail;
				LSQ -> lsqentry[w+1].lopcode = LSQ -> lsqentry[w].lopcode;
				LSQ -> lsqentry[w+1].ldispatch = LSQ -> lsqentry[w].ldispatch;
				LSQ -> lsqentry[w+1].rename_destination = LSQ -> lsqentry[w].rename_destination;
				LSQ -> lsqentry[w+1].validcheck = LSQ -> lsqentry[w].validcheck;
				LSQ -> lsqentry[w+1].valid = LSQ -> lsqentry[w].valid;
		}
		
		LSQ -> lsqentry[0].lsqstring = LSQ -> temp.lsqstring ; 
		LSQ -> lsqentry[0].lsq1_address = LSQ -> temp.lsq1_address ;
		LSQ -> lsqentry[0].lsq1_value = LSQ -> temp.lsq1_value ;
		LSQ -> lsqentry[0].lsq1_valid = LSQ -> temp.lsq1_valid ;
		LSQ -> lsqentry[0].lsq2_address = LSQ -> temp.lsq2_address; 
		LSQ -> lsqentry[0].lsq2_value = LSQ -> temp.lsq2_value; 
		LSQ -> lsqentry[0].lsq2_valid = LSQ -> temp.lsq2_valid; 
		LSQ -> lsqentry[0].ldst_address = LSQ -> temp.ldst_address;
		LSQ -> lsqentry[0].ldst_value = LSQ -> temp.ldst_value;
		LSQ -> lsqentry[0].lsqdest = LSQ -> temp.lsqdest;
		LSQ -> lsqentry[0].mem_address = LSQ -> temp.mem_address;
		LSQ -> lsqentry[0].mem_addressvalid = LSQ -> temp.mem_addressvalid;
		LSQ -> lsqentry[0].tail = LSQ -> temp.tail;
		LSQ -> lsqentry[0].lopcode = LSQ -> temp.lopcode;
		LSQ -> lsqentry[0].ldispatch = LSQ -> temp.ldispatch;
		LSQ -> lsqentry[0].rename_destination = LSQ -> temp.rename_destination;
		LSQ -> lsqentry[0].validcheck = true;

	}	





	//cout << LSQ -> lsqentry[0].valid << mem -> output_Instruction -> stalled << LSQ -> lsqentry[0].mem_addressvalid << LSQ -> lsqentry[0].lsq1_valid << endl;

	if(LSQ -> lsqentry[0].valid && !mem -> output_Instruction -> stalled && LSQ -> lsqentry[0].mem_addressvalid && LSQ -> lsqentry[0].lsq1_valid){
		//cout << "stepp" << endl;
		if(LSQ -> lsqentry[0].lopcode == "LOAD"){
			
			mem -> Input_Instruction -> instruction_string = LSQ -> lsqentry[0].lsqstring;
			mem -> Input_Instruction -> source1_register =  LSQ -> lsqentry[0].lsq1_address;
			mem -> Input_Instruction -> source1_val =  LSQ -> lsqentry[0].lsq1_value;
			mem -> Input_Instruction -> source2_register =  LSQ -> lsqentry[0].lsq2_address;
			mem -> Input_Instruction -> source2_val =  LSQ -> lsqentry[0].lsq2_value;
			mem -> Input_Instruction -> dest_register =  LSQ -> lsqentry[0].ldst_address;
			mem -> Input_Instruction -> computed_val =  LSQ -> lsqentry[0].mem_address;
			mem -> Input_Instruction -> opcode1 =  LSQ -> lsqentry[0].lopcode;
			mem -> Input_Instruction -> tail = LSQ -> lsqentry[0].tail;
			mem -> Input_Instruction -> rename_destination = LSQ -> lsqentry[0].rename_destination;
			LSQ -> lsqentry[0].valid = false;

			for(int i = 1;i <= LSQ -> endpt;i++){
				if ( i == LSQ -> endpt){
				
					LSQ -> lsqentry[i].valid = false;
				}
				else{	
					LSQ -> lsqentry[i-1].lsqstring = LSQ -> lsqentry[i].lsqstring  ; 
					LSQ -> lsqentry[i-1].lsq1_address = LSQ -> lsqentry[i].lsq1_address ;
					LSQ -> lsqentry[i-1].lsq1_value = LSQ -> lsqentry[i].lsq1_value ;
					LSQ -> lsqentry[i-1].lsq1_valid = LSQ -> lsqentry[i].lsq1_valid ;
					LSQ -> lsqentry[i-1].lsq2_address = LSQ -> lsqentry[i].lsq2_address; 
					LSQ -> lsqentry[i-1].lsq2_value = LSQ -> lsqentry[i].lsq2_value; 
					LSQ -> lsqentry[i-1].lsq2_valid = LSQ -> lsqentry[i].lsq2_valid; 
					LSQ -> lsqentry[i-1].lsqdest = LSQ -> lsqentry[i].lsqdest;
					LSQ -> lsqentry[i-1].valid = LSQ -> lsqentry[i].valid;
					LSQ -> lsqentry[i-1].mem_address = LSQ -> lsqentry[i].mem_address;
					LSQ -> lsqentry[i-1].mem_addressvalid = LSQ -> lsqentry[i].mem_addressvalid;
					LSQ -> lsqentry[i-1].tail = LSQ -> lsqentry[i].tail;
					LSQ -> lsqentry[i-1].lopcode = LSQ -> lsqentry[i].lopcode;
					LSQ -> lsqentry[i-1].ldispatch = LSQ -> lsqentry[i].ldispatch;
					LSQ -> lsqentry[i-1].rename_destination = LSQ -> lsqentry[i].rename_destination;
					LSQ -> lsqentry[i-1].validcheck = LSQ -> lsqentry[i].validcheck;
				}
			}
			//cout << "LSQ -> endpt - " << LSQ -> endpt << endl;
			LSQ -> lsqentry[LSQ -> endpt - 1].valid = false;
			LSQ -> endpt--;
			mem -> next_stage = true;

		}
		else if(LSQ -> lsqentry[0].lopcode == "STORE"){
			if(rob_array -> robentry[rob_array -> start_pt].Dispatchclk == 	LSQ -> lsqentry[0].ldispatch){
				mem -> Input_Instruction -> instruction_string = LSQ -> lsqentry[0].lsqstring;
				mem -> Input_Instruction -> source1_register =  LSQ -> lsqentry[0].lsq1_address;
				mem -> Input_Instruction -> source1_val =  LSQ -> lsqentry[0].lsq1_value;
				mem -> Input_Instruction -> source2_register =  LSQ -> lsqentry[0].lsq2_address;
				mem -> Input_Instruction -> source2_val =  LSQ -> lsqentry[0].lsq2_value;
				mem -> Input_Instruction -> computed_val =  LSQ -> lsqentry[0].mem_address;
				mem -> Input_Instruction -> opcode1 =  LSQ -> lsqentry[0].lopcode;
				mem -> Input_Instruction -> tail = LSQ -> lsqentry[0].tail;
				mem -> Input_Instruction -> rename_destination = LSQ -> lsqentry[0].rename_destination;
				LSQ -> lsqentry[0].valid = false;
				rob_array -> robentry[rob_array -> start_pt].status = true;
				rob_array -> start_pt++;
				//cout << "check1 " << endl;
				for(int i = 1;i <= LSQ -> endpt;i++){
					if ( i == LSQ -> endpt){
				
						LSQ -> lsqentry[i].valid = false;
					}
					else{	
						LSQ -> lsqentry[i-1].lsqstring = LSQ -> lsqentry[i].lsqstring  ; 
						LSQ -> lsqentry[i-1].lsq1_address = LSQ -> lsqentry[i].lsq1_address ;
						LSQ -> lsqentry[i-1].lsq1_value = LSQ -> lsqentry[i].lsq1_value ;
						LSQ -> lsqentry[i-1].lsq1_valid = LSQ -> lsqentry[i].lsq1_valid ;
						LSQ -> lsqentry[i-1].lsq2_address = LSQ -> lsqentry[i].lsq2_address; 
						LSQ -> lsqentry[i-1].lsq2_value = LSQ -> lsqentry[i].lsq2_value; 
						LSQ -> lsqentry[i-1].lsq2_valid = LSQ -> lsqentry[i].lsq2_valid; 
						LSQ -> lsqentry[i-1].ldst_value = LSQ -> lsqentry[i].ldst_value;
						LSQ -> lsqentry[i-1].ldst_address = LSQ -> lsqentry[i].ldst_address;
						LSQ -> lsqentry[i-1].lsqdest = LSQ -> lsqentry[i].lsqdest;
						LSQ -> lsqentry[i-1].valid = LSQ -> lsqentry[i].valid;
						LSQ -> lsqentry[i-1].mem_address = LSQ -> lsqentry[i].mem_address;
						LSQ -> lsqentry[i-1].mem_addressvalid = LSQ -> lsqentry[i].mem_addressvalid;
						LSQ -> lsqentry[i-1].tail = LSQ -> lsqentry[i].tail;
						LSQ -> lsqentry[i-1].lopcode = LSQ -> lsqentry[i].lopcode;
						LSQ -> lsqentry[i-1].ldispatch = LSQ -> lsqentry[i].ldispatch;
						LSQ -> lsqentry[i-1].rename_destination = LSQ -> lsqentry[i].rename_destination;
						LSQ -> lsqentry[i-1].validcheck = LSQ -> lsqentry[i].validcheck;			
					}
				}
				LSQ -> lsqentry[LSQ -> endpt - 1].valid = false;
				LSQ -> endpt--;
				mem -> next_stage = true;
			}	
		}
	}
	else
	{
		mem -> Input_Instruction -> instruction_string = "nop";
		mem -> Input_Instruction -> opcode1 = "nop";
	}
}



void ROB_commit(stage* ROB, Register_File* r_f, Rename_Table* r_t, rob* ROB_array, stage* mem){
	if (ROB -> next_stage){
		int counter = 0;
		int destar_add;
		int dest_add;
//recheck head condition
			while(ROB_array -> robentry[ROB_array -> start_pt].status){
				if( ROB_array -> robentry[ROB_array -> start_pt].opcode != "STORE" &&  ROB_array -> robentry[ROB_array -> start_pt].opcode != "nop" && ROB_array -> robentry[ROB_array -> start_pt].opcode != "BZ" && ROB_array -> robentry[ROB_array -> start_pt].opcode != "BNZ" && ROB_array -> robentry[ROB_array -> start_pt].opcode != "JUMP"){				
					destar_add = ROB_array -> robentry[ROB_array -> start_pt].destar_add;
					dest_add = ROB_array -> robentry[ROB_array -> start_pt].dest_add;
					r_f -> reg[destar_add].value = ROB_array -> robentry[ROB_array -> start_pt].destresult;
					r_f -> reg[destar_add].status = true;
					//cout << r_t -> rtable[destar_add].address << endl << "i " <<ROB_array -> robentry[ROB_array -> start_pt].dest_add << endl;
					if (r_t -> rtable[destar_add].address == ROB_array -> robentry[ROB_array -> start_pt].dest_add){/////recheck //////////
						r_f -> phy[dest_add].status = false;
						r_f -> phy[dest_add].allocate = false;
						r_f -> phy[dest_add].rename = false;
						r_t -> rtable[destar_add].phyarch = false;
						r_t -> rtable[destar_add].address = destar_add;
					}	
					if (ROB_array -> start_pt < 31){
						ROB_array -> start_pt ++;
					}
					else {
						ROB_array -> start_pt = 0;
					}
					
					counter++;
					if (counter == 2){
						break;
					}
				}
				else if (ROB_array -> robentry[ROB_array -> start_pt].opcode == "STORE" && ROB_array -> robentry[ROB_array -> start_pt].status){
					if (mem_counter == 0)
					{
						if (ROB_array -> start_pt < 31){
							ROB_array -> start_pt ++;
						}
						else {
							ROB_array -> start_pt = 0;
						}

						counter++;
					}
				}
				else if (ROB_array -> robentry[ROB_array -> start_pt].opcode == "BZ" || ROB_array -> robentry[ROB_array -> start_pt].opcode == "BNZ" || ROB_array -> robentry[ROB_array -> start_pt].opcode == "JUMP")
				{
					if (ROB_array -> start_pt < 31){
						ROB_array -> start_pt ++;
					}
					else {
						ROB_array -> start_pt = 0;
					}

					counter++;
					if (counter == 2){
						break;
					}
				}
			}
			

			if (ROB_array -> start_pt == ROB_array -> end_pt)
			{
				 //mem_counter == 2
				if(!mem -> output_Instruction -> stalled && !branch_rob)
				{
					stop = true;
				}
			}
	}
}


	
	
void Memory(stage* mem, Data_Memory* d_m, rob* rob_array, bus* f_b,Register_File* r_f){
	if (mem -> Stalled){
		return;	
	}
	else{
		if (mem -> next_stage){
	
      		if(mem -> Input_Instruction -> opcode1 != "nop" && !mem -> output_Instruction -> stalled){
				mem -> output_Instruction -> instruction_string = mem -> Input_Instruction -> instruction_string;
	  			mem -> output_Instruction -> source1_register = mem-> Input_Instruction -> source1_register;
				mem -> output_Instruction -> source1_val = mem -> Input_Instruction -> source1_val;
				mem -> output_Instruction -> source2_register = mem -> Input_Instruction -> source2_register;
				mem -> output_Instruction -> source2_val = mem -> Input_Instruction -> source2_val;
	  			mem -> output_Instruction -> dest_register = mem -> Input_Instruction -> dest_register;
	  			mem -> output_Instruction -> computed_val = mem -> Input_Instruction -> computed_val;
				//cout << " addition memory output value " << mem -> output_Instruction -> computed_val << endl;
	  			mem -> output_Instruction -> literal = mem -> Input_Instruction -> literal;
	  			mem -> output_Instruction -> opcode1 = mem -> Input_Instruction -> opcode1;
	  			mem -> output_Instruction -> PC = mem -> Input_Instruction -> PC;
				mem -> output_Instruction -> tail = mem -> Input_Instruction -> tail;
				mem -> output_Instruction -> rename_destination = mem -> Input_Instruction -> rename_destination;
				mem -> output_Instruction -> stalled = true;
				//mem_counter = 0;
	      	}
			else if (mem -> Input_Instruction -> opcode1 == "nop" && mem_counter == 0 && !mem -> output_Instruction -> stalled)
			{
				mem -> output_Instruction -> instruction_string = "nop";
				mem -> output_Instruction -> opcode1 = "nop";
			}

			if (mem_counter < 3)
			{
				if (mem_counter == 2)
				{
					mem_counter = -1;
					string opcode_exec;
					int src1 = mem -> output_Instruction -> source1_val;	
					int compute;
					int memdata;
					int ltrl;
			
					opcode_exec = mem -> output_Instruction -> opcode1;
					compute = mem -> output_Instruction -> computed_val;

					if (mem -> output_Instruction -> opcode1 == "LOAD")
					{
						src1 = mem -> output_Instruction -> source1_register;
						ltrl = mem -> output_Instruction -> literal;   
					    mem -> output_Instruction -> computed_val = d_m -> data[compute];

					    r_f -> phy[mem -> output_Instruction -> dest_register].status = true;
					    r_f -> phy[mem -> output_Instruction -> dest_register].value = mem -> output_Instruction -> computed_val;
						
						//cout << "mem -> output_Instruction -> tail - " << mem -> output_Instruction -> tail << endl;
						rob_array -> robentry[mem -> output_Instruction -> tail].status = true;
						rob_array -> robentry[mem -> output_Instruction -> tail].destresult = mem -> output_Instruction -> computed_val;
						f_b -> busarray[0].reg_value = mem -> output_Instruction -> computed_val;
						f_b -> busarray[0].reg_address = mem -> output_Instruction -> rename_destination;
						f_b -> busarray[0].valid = true;
					}
					else if (mem -> output_Instruction -> opcode1 == "STORE")
					{
						src1 = mem -> output_Instruction -> source1_register;
						d_m -> data[compute] = mem -> output_Instruction -> source1_val;
					}

					mem -> output_Instruction -> stalled = false;
				}
				else
				{
					f_b -> busarray[0].valid = false;	
				}
				mem_counter++;
				//cout << "Mem counter - " << mem_counter << endl;
				//cout << "ROB==========" << endl;
				/*if (mem_counter == 3)
				{
					mem_counter = 0;
					if (mem -> output_Instruction -> opcode1 == "LOAD")
					{
						// Set ROB entry status bit to true;
						//cout << "ROB==========" << endl;
						rob_array -> robentry[mem -> output_Instruction -> tail].status = true;
						rob_array -> robentry[mem -> output_Instruction -> tail].destresult = mem -> output_Instruction -> computed_val;
						f_b -> busarray[0].reg_value = mem -> output_Instruction -> computed_val;
						f_b -> busarray[0].reg_address = mem -> output_Instruction -> rename_destination;
						f_b -> busarray[0].valid = true;
					}
					else
					{
						f_b -> busarray[0].valid = false;
					}
				}
				else
				{
					f_b -> busarray[0].valid = false;
				}*/
			}
			/*else if (mem2_temp -> Input_Instruction -> instruction_string != "nop"){

				mem -> output_Instruction -> instruction_string = mem2_temp -> Input_Instruction -> instruction_string;
	  			mem -> output_Instruction -> source1_register = mem2_temp-> Input_Instruction -> source1_register;
				mem -> output_Instruction -> source1_val = mem2_temp -> Input_Instruction -> source1_val;
				mem -> output_Instruction -> source2_register = mem2_temp -> Input_Instruction -> source2_register;
				mem -> output_Instruction -> source2_val = mem2_temp -> Input_Instruction -> source2_val;
	  			mem -> output_Instruction -> dest_register = mem2_temp -> Input_Instruction -> dest_register;
	  			mem -> output_Instruction -> computed_val = mem2_temp -> Input_Instruction -> computed_val;
				cout << " addition memory output value " << mem -> output_Instruction -> computed_val << endl;
	  			mem -> output_Instruction -> literal = mem2_temp -> Input_Instruction -> literal;
	  			mem -> output_Instruction -> opcode1 = mem2_temp -> Input_Instruction -> opcode1;
	  			mem -> output_Instruction -> PC = mem2_temp -> Input_Instruction -> PC;
				mem2_temp -> Input_Instruction -> stalled = true;
				mem2_temp -> Input_Instruction -> instruction_string = "nop";
				mem2_temp -> Input_Instruction -> opcode1 = "nop";
				
			}

			else{
				mem -> output_Instruction -> instruction_string = mem_temp -> Input_Instruction -> instruction_string;
	  			mem -> output_Instruction -> source1_register = mem_temp-> Input_Instruction -> source1_register;
				mem -> output_Instruction -> source1_val = mem_temp -> Input_Instruction -> source1_val;
				mem -> output_Instruction -> source2_register = mem_temp -> Input_Instruction -> source2_register;
				mem -> output_Instruction -> source2_val = mem_temp -> Input_Instruction -> source2_val;
	  			mem -> output_Instruction -> dest_register = mem_temp -> Input_Instruction -> dest_register;
	  			mem -> output_Instruction -> computed_val = mem_temp -> Input_Instruction -> computed_val;
				cout << " addition memory output value " << mem -> output_Instruction -> computed_val << endl;
	  			mem -> output_Instruction -> literal = mem_temp -> Input_Instruction -> literal;
	  			mem -> output_Instruction -> opcode1 = mem_temp -> Input_Instruction -> opcode1;
	  			mem -> output_Instruction -> PC = mem_temp -> Input_Instruction -> PC;
	       			mem_temp -> Input_Instruction -> stalled = true;
				mem_temp -> Input_Instruction -> instruction_string = "nop";
				mem_temp -> Input_Instruction -> opcode1 = "nop";
				exec_mem_stall = true;
				
      			}
			//cout << "addition input to wback " << mem -> output_Instruction -> computed_val << endl;
			string opcode_exec;
			int src1 = mem -> output_Instruction -> source1_val;	
			int compute;
			int memdata;
			int ltrl;
			
			opcode_exec = mem -> output_Instruction -> opcode1;
			compute = mem -> output_Instruction -> computed_val;
			
			if (opcode_exec == "STORE"){
				
				src1 = mem -> output_Instruction -> source1_register;
				d_m -> data[compute] = r_f -> reg[src1].value;
			}	
			else if (opcode_exec == "LOAD"){

				
				src1 = mem -> output_Instruction -> source1_register;
				ltrl = mem -> output_Instruction -> literal;   
			       	mem -> output_Instruction -> computed_val = d_m -> data[compute];			
			}		
			wback -> Input_Instruction -> instruction_string = mem -> output_Instruction -> instruction_string;
			wback -> Input_Instruction -> source1_register = mem -> output_Instruction -> source1_register;
			wback -> Input_Instruction -> source2_register = mem -> output_Instruction -> source2_register;
			wback -> Input_Instruction -> dest_register = mem -> output_Instruction -> dest_register;        
			wback -> Input_Instruction -> computed_val = mem -> output_Instruction -> computed_val;
			//cout << "addition input to wback " << mem -> output_Instruction -> computed_val << endl;
			wback -> Input_Instruction -> literal = mem -> output_Instruction -> literal;
			wback -> Input_Instruction -> opcode1 = mem -> output_Instruction -> opcode1;
			wback -> Input_Instruction -> PC = mem -> output_Instruction -> PC;
			wback -> next_stage = true;*/
		}
	}
}

void WriteBack(stage* wback,Register_File* r_f, Rename_Table* r_t){
	if (wback -> Stalled == true){
		return;	
	}
	else{
		if (wback -> next_stage){
			wback -> output_Instruction -> instruction_string = wback -> Input_Instruction -> instruction_string;
			wback -> output_Instruction -> source1_register = wback -> Input_Instruction -> source1_register;
			wback -> output_Instruction -> source2_register = wback -> Input_Instruction -> source2_register;
			wback -> output_Instruction -> dest_register = wback -> Input_Instruction -> dest_register;
			wback -> output_Instruction -> computed_val = wback -> Input_Instruction -> computed_val;
			//cout << " addition wback output value " << wback -> output_Instruction -> computed_val << endl;
			wback -> output_Instruction -> literal = wback -> Input_Instruction -> literal;
			wback -> output_Instruction -> opcode1 = wback -> Input_Instruction -> opcode1;
			wback -> output_Instruction -> PC = wback -> Input_Instruction -> PC;	
			string opcode_wback;
			int dest;
			
			opcode_wback = wback -> output_Instruction -> opcode1;
			dest = wback -> output_Instruction -> dest_register;
			//cout << " wback dest " << dest << endl;  
		
			if (opcode_wback == "JAL")
			{
				r_f -> phy[dest].value = jal_target;
			}			
			else if (opcode_wback != "STORE" && opcode_wback != "nop" && opcode_wback != "" && opcode_wback != "BZ" && opcode_wback != "BNZ" && opcode_wback != "JUMP" && opcode_wback != "HALT"  ){	
				
				if ( r_f->phy[dest].status == true ){
					r_f -> phy[dest].allocate = false;
					r_f -> phy[dest].status = false;
					r_f -> phy[dest].rename = false;
				}
				if ( dest == r_t -> rtable[dest].address ){
					r_t -> rtable[dest].address = dest ;
				}
	
				
				r_f -> reg[dest].value = wback -> output_Instruction -> computed_val;
			}
		}
	}
}

void flush(rob* rob_array,lsq* LSQ,Issue_Q* IS,stage* exec_m1, stage* exec_m2,stage* exec_d1,stage* exec_d2,stage* exec_d3,stage* exec_d4, stage* decode, Register_File* r_f, Rename_Table* r_t){
			//i = rob_array -> end_pt
		//cout << "i - " << rob_array -> end_pt - 1 << endl;
		for(int i = rob_array -> end_pt - 1; rob_array -> robentry[i].Dispatchclk > branch_dispatch_cc; i--){
			//cout << "i - " << rob_array -> end_pt - 1 << endl;
			if(rob_array -> robentry[i].phyarch == true){
				r_f -> phy[rob_array -> robentry[i].dest_add].status = true;
				r_f -> phy[rob_array -> robentry[i].dest_add].allocate = true;
				r_f -> phy[rob_array -> robentry[i].dest_add].value = rob_array -> robentry[i].destresult;
			}
			else{
				r_f -> reg[rob_array -> robentry[i].dest_add].status = true;
				r_f -> reg[rob_array -> robentry[i].dest_add].value = rob_array -> robentry[i].destresult;
			}	
			rob_array -> robentry[i].status = false;
			rob_array -> end_pt--;
		}

		for(int z=LSQ -> endpt - 1; LSQ ->lsqentry[z].ldispatch > branch_dispatch_cc; z--){
			LSQ ->lsqentry[z].valid = false;
		}

		//cout << "LSQ COMPLETE" << endl;

		for(int k=0;k<16;k++){
			if (IS -> entry[k].Q_valid && IS -> entry[k].Qdispatch_clk > branch_dispatch_cc)
			{
				IS -> entry[k].Q_valid = false;
			}
			
		}

		//cout << "IQ COMPLETE" << endl;


			if(exec_m2 -> output_Instruction -> dispatch > branch_dispatch_cc){
				exec_m2 -> Input_Instruction -> opcode1 = "nop";
				exec_m2 -> output_Instruction -> instruction_string = "nop";
			}
			if(exec_m1 -> output_Instruction -> dispatch > branch_dispatch_cc){
				exec_m1 -> Input_Instruction -> opcode1 = "nop";
				exec_m1 -> output_Instruction -> instruction_string = "nop";
			}
			if(exec_d4 -> output_Instruction -> dispatch > branch_dispatch_cc){
				exec_d4 -> Input_Instruction -> opcode1 = "nop";
				exec_d4 -> output_Instruction -> instruction_string = "nop";
			}
			if(exec_d3 -> output_Instruction -> dispatch > branch_dispatch_cc){
				exec_d3 -> Input_Instruction -> opcode1 = "nop";
				exec_d3 -> output_Instruction -> instruction_string = "nop";
			}
			if(exec_d2 -> output_Instruction -> dispatch > branch_dispatch_cc){
				exec_d2 -> Input_Instruction -> opcode1 = "nop";
				exec_d2 -> output_Instruction -> instruction_string = "nop";
			}
			if(exec_d1 -> output_Instruction -> dispatch > branch_dispatch_cc){
				exec_d1 -> Input_Instruction -> opcode1 = "nop";
				exec_d1 -> output_Instruction -> instruction_string = "nop";
			}

		if (decode -> output_Instruction -> phyarch)
		{
			r_f -> phy[atoi(decode -> output_Instruction -> destname_register.substr(1).c_str())].allocate = false;
			r_t -> rtable[decode -> output_Instruction -> dest_register].phyarch = false;
			r_t -> rtable[decode -> output_Instruction -> dest_register].address = decode -> output_Instruction -> dest_register;
			r_f -> reg[decode -> output_Instruction -> dest_register].status = true;
		}
		else
		{
			r_t -> rtable[decode -> output_Instruction -> dest_register].phyarch = false;
			r_t -> rtable[decode -> output_Instruction -> dest_register].address = decode -> output_Instruction -> dest_register;
			r_f -> reg[decode -> output_Instruction -> dest_register].status = true;
		}
}
int main(int argc, char *argv[]) {
	int i,j,p,r;
	int PC = 4000;
	string stage;
	int clock_cycle;
	int clk_it = 0;
	int k = 0;
	int l=0;
	fstream fIn;
	string line;
	//lsq lsq_name;
	Register_File Reg_Obj;
	Data_Memory Data_Obj;
	string filename;
	Rename_Table R_name;
	rob reorder_buffer;
	int start_pt = 0;
	int end_pt = 0;
	//bus fwd_b;
	
	
	//bus bus_obj;
	//cout << W.Stalled;
	//fstream myfile ("input.txt");
	cout<<"Enter the filename: "<<endl;
	
        //getline(cin,filename);
        cin >> filename;
        fIn.open(filename, ios::in );
	int init = 0;
	int index =0;
	int num_lines;
	int total_clock_cycles = 0;
	bool initialize = false;
	Flags fg;
	fg.zero = false;

	F.Input_Instruction = new Instruction_info;
	F.output_Instruction = new Instruction_info;

	D.Input_Instruction = new Instruction_info;
	D.output_Instruction = new Instruction_info;

	I.Input_Instruction = new Instruction_info;
	I.output_Instruction = new Instruction_info;

	E.Input_Instruction = new Instruction_info;
	E.output_Instruction = new Instruction_info;

	EM1.Input_Instruction = new Instruction_info;
	EM1.Input_Instruction -> instruction_string = "nop";
	EM1.output_Instruction = new Instruction_info;
	EM1.output_Instruction -> instruction_string = "nop";

	EM2.Input_Instruction = new Instruction_info;
	EM2.Input_Instruction -> instruction_string = "nop";
	EM2.output_Instruction = new Instruction_info;	
	EM2.output_Instruction -> instruction_string = "nop";

	ED1.Input_Instruction = new Instruction_info;
	ED1.Input_Instruction -> instruction_string = "nop";
	ED1.output_Instruction = new Instruction_info;
	ED1.output_Instruction -> instruction_string = "nop";

	ED2.Input_Instruction = new Instruction_info;
	ED2.Input_Instruction -> instruction_string = "nop";
	ED2.output_Instruction = new Instruction_info;
	ED2.output_Instruction -> instruction_string = "nop";

	ED3.Input_Instruction = new Instruction_info;
	ED3.Input_Instruction -> instruction_string = "nop";
	ED3.output_Instruction = new Instruction_info;
	ED3.output_Instruction -> instruction_string = "nop";

	ED4.Input_Instruction = new Instruction_info;
	ED4.Input_Instruction -> instruction_string = "nop";
	ED4.output_Instruction = new Instruction_info;
	ED4.output_Instruction -> instruction_string = "nop";
	
	M_temp.Input_Instruction = new Instruction_info;
	M_temp.Input_Instruction -> instruction_string = "nop";
	M_temp.output_Instruction = new Instruction_info;

	M2_temp.Input_Instruction = new Instruction_info;
	M2_temp.Input_Instruction -> instruction_string = "nop";
	M2_temp.output_Instruction = new Instruction_info;
	M2_temp.output_Instruction -> stalled = true;

	
	M.Input_Instruction = new Instruction_info;
	M.output_Instruction = new Instruction_info;

	W.Input_Instruction = new Instruction_info;
	W.output_Instruction = new Instruction_info;

	R.Input_Instruction = new Instruction_info;
	R.output_Instruction = new Instruction_info;


//	if(myfile.is_open())
	if(fIn.is_open())	
	{
//		while(getline(myfile,line))
		while(getline(fIn,line))
		{
		//	cout << line <<'\n';
			Cd_Obj.codememory[init].instruction_string = line;
		//	cout << Cd_Obj.codememory[index].instruction_string << endl;
			init++;
		}
		num_lines = init;
//		myfile.close();
		fIn.close();
	}
	else cout << "Unable to open file" <<endl;


	while(true){

	
		cout << "Enter a stage" << endl << "1.Initialization" << endl << "2.Simulator" << endl << "3.Display." << endl << "4.EXIT" << endl <<"ENTER: ";
		cin >> stage;

		if (stage == "1"){
			if (!initialize)
			{
				F.next_stage = true;

				for(i=0;i<16;i++){
					Reg_Obj.reg[i].value = 0;	
				}
				for(j=0;j<4000;j++){
					Data_Obj.data[j]=0;
				}
				for (p=0;p<16;p++){
					R_name.rtable[p].address = p;
					R_name.rtable[p].phyarch = false;		
				}
				for(i=0;i<32;i++){
					Reg_Obj.phy[i].value = 0;
				}
					
				cout << "initialised the memory values" << endl;
				F.Input_Instruction -> instruction_string = Cd_Obj.codememory[index].instruction_string;
				cout << Cd_Obj.codememory[index].instruction_string;				
				F.Input_Instruction -> PC = 4000;
				index++;
				initialize = true;
			}
		}
		else if (stage == "2"){
			if (initialize && !prog_end)
//			if (initialize)
			{	
				cout << "Enter the number of clock cycles:" << endl;
				cin >> clock_cycle;
				clk_it = 0;
				while (clk_it < clock_cycle && !stop){
					cout << endl << endl;
				
					
					
					ROB_commit(&R,&Reg_Obj,&R_name,&reorder_buffer, &M);

					//cout << "ROB_commit" << endl;
					lsq_fn(&M, &lsq_name, &reorder_buffer,&Reg_Obj,&fwd_b);
					//cout << "lsq_name" << endl;
					Memory(&M, &Data_Obj, &reorder_buffer, &fwd_b,&Reg_Obj);
					//cout << "Mwmory" << endl;
					if(exec_mem_stall){
						E.output_Instruction ->  stalled = false;
						exec_mem_stall = false;	
						exec_stall = false;
						decode_stall = false;	

						if(!decode_depend && !exec_stall){				
							E.Input_Instruction ->  stalled = false;
							D.output_Instruction ->  stalled = false;
							D.Input_Instruction ->  stalled = false;
		
						}					
					}
//					Execute(&E,&M,&EM1,&EM2,&ED1,&ED2,&ED3,&ED4,&M_temp,&M2_temp,&fg,&bus_obj);
					Execute(&E,&M,&EM1,&EM2,&ED1,&ED2,&ED3,&ED4,&M_temp,&M2_temp,&fg,&C,&Reg_Obj,&R_name,&fwd_b,&R,&reorder_buffer,&lsq_name);
					//cout << "execute" << endl;		
					if(branch_execute){
						branch_rob = true;
						flush(&reorder_buffer, &lsq_name, &IS,&EM1,&EM2,&ED1,&ED2,&ED3,&ED4, &D,&Reg_Obj,&R_name);
						EM1.Input_Instruction -> instruction_string = "nop";
						EM1.Input_Instruction -> opcode1 = "nop";
						EM1.Input_Instruction -> stalled = false;
						E.Input_Instruction -> instruction_string = "nop";
						E.Input_Instruction -> opcode1 = "nop";
						E.Input_Instruction -> stalled = false;
						I.Input_Instruction -> instruction_string = "nop";
						I.Input_Instruction -> opcode1 = "nop";
						I.Input_Instruction -> stalled = false;	
						I.output_Instruction -> instruction_string = "nop";
						I.output_Instruction -> opcode1 = "nop";
						I.output_Instruction -> stalled = false;
						D.Input_Instruction -> instruction_string = "nop";
						D.Input_Instruction -> opcode1 = "nop";
						D.Input_Instruction -> stalled = false;	
						D.output_Instruction -> instruction_string = "nop";
						D.output_Instruction -> opcode1 = "nop";
						D.output_Instruction -> stalled = false;
						F.Input_Instruction -> instruction_string = "nop";
						F.Input_Instruction -> opcode1 = "nop";
						F.Input_Instruction -> stalled = false;	
						F.output_Instruction -> instruction_string = "nop";
						F.output_Instruction -> opcode1 = "nop";
						F.output_Instruction -> stalled = false;
						branch_execute = false;
						PC = E.output_Instruction -> computed_val - 4;					

					}
					else if(jump_execute){
						branch_rob = true;
						flush(&reorder_buffer, &lsq_name, &IS,&EM1,&EM2,&ED1,&ED2,&ED3,&ED4, &D,&Reg_Obj,&R_name);
						EM1.Input_Instruction -> instruction_string = "nop";
						EM1.Input_Instruction -> opcode1 = "nop";
						EM1.Input_Instruction -> stalled = false;
						E.Input_Instruction -> instruction_string = "nop";
						E.Input_Instruction -> opcode1 = "nop";
						E.Input_Instruction -> stalled = false;
						I.Input_Instruction -> instruction_string = "nop";
						I.Input_Instruction -> opcode1 = "nop";
						I.Input_Instruction -> stalled = false;	
						I.output_Instruction -> instruction_string = "nop";
						I.output_Instruction -> opcode1 = "nop";
						I.output_Instruction -> stalled = false;
						D.Input_Instruction -> instruction_string = "nop";
						D.Input_Instruction -> opcode1 = "nop";
						D.Input_Instruction -> stalled = false;	
						D.output_Instruction -> instruction_string = "nop";
						D.output_Instruction -> opcode1 = "nop";
						D.output_Instruction -> stalled = false;
						F.Input_Instruction -> instruction_string = "nop";
						F.Input_Instruction -> opcode1 = "nop";
						F.Input_Instruction -> stalled = false;	
						F.output_Instruction -> instruction_string = "nop";
						F.output_Instruction -> opcode1 = "nop";
						F.output_Instruction -> stalled = false;
						jump_execute = false;
						PC = E.output_Instruction -> computed_val - 4;			

					}
					else if(halt_execute){
						EM1.Input_Instruction -> instruction_string = "nop";
						EM1.Input_Instruction -> opcode1 = "nop";
						EM1.Input_Instruction -> stalled = false;
						E.Input_Instruction -> instruction_string = "nop";
						E.Input_Instruction -> opcode1 = "nop";
						E.Input_Instruction -> stalled = false;
						D.Input_Instruction -> instruction_string = "nop";
						D.Input_Instruction -> opcode1 = "nop";
						D.Input_Instruction -> stalled = false;	
						D.output_Instruction -> instruction_string = "nop";
						D.output_Instruction -> opcode1 = "nop";
						D.output_Instruction -> stalled = false;
						F.Input_Instruction -> instruction_string = "nop";
						F.Input_Instruction -> opcode1 = "nop";
						F.Input_Instruction -> stalled = false;	
						F.output_Instruction -> instruction_string = "nop";
						F.output_Instruction -> opcode1 = "nop";
						F.output_Instruction -> stalled = false;					
					}
					
					IssueQ(&I,&E,&EM1,&ED1,&IS,&C,&Reg_Obj,&R_name,&fwd_b,&reorder_buffer,&lsq_name);	
					//cout << "isuseq" << endl;
					Decode(&D,&I,&E,&EM1,&ED1,&Reg_Obj,&R_name,&fwd_b);
					//cout << "Decode" << endl;
					Fetch(&F,&D);	
					//cout << "Fectch" << endl;
					
					if(W.output_Instruction -> opcode1 != "nop" && W.output_Instruction -> opcode1 != "STORE" && W.output_Instruction -> opcode1 != "BZ" && W.output_Instruction -> opcode1 != "BNZ" && W.output_Instruction -> opcode1 != "JUMP" && W.output_Instruction -> opcode1 != "HALT"){
						Reg_Obj.reg[W.output_Instruction -> dest_register].status = true;

					}

					if(W.output_Instruction -> computed_val == 0 && W.output_Instruction -> PC == branch_pc){
						fg.zero = true;
						Reg_Obj.reg[16].status = true;
					}
					else if (W.output_Instruction -> computed_val != 0 && W.output_Instruction -> PC == branch_pc){
						fg.zero = false;
						Reg_Obj.reg[16].status = true;					
					}
						
					if (PC < ((num_lines-1)*4+4000)){
						
						if(!F.Input_Instruction -> stalled && !halt_execute){						
							PC = PC+4;
							//cout << "PC - " << PC << endl;
							F.Input_Instruction -> instruction_string = Cd_Obj.codememory[(PC - 4000)/4].instruction_string;
							F.Input_Instruction -> PC = PC;
							//cout << "PC - " << PC;
						}
					}
					else
					{
						if (F.Input_Instruction -> instruction_string == F.output_Instruction -> instruction_string)
						{
							F.Input_Instruction -> instruction_string = "nop";
							F.Input_Instruction -> opcode1 = "nop";
						}
					}
					clk_it++;
					C.cycle++;
					total_clock_cycles++;
					cout << endl;

					

					/*				
					cout << "Fetch input:" << F.Input_Instruction -> instruction_string << ";	FI stalled " << F.Input_Instruction -> stalled << endl;
					cout << "Fetch output:" << F.output_Instruction -> instruction_string << ";	FO stalled " << F.output_Instruction -> stalled << endl;
					cout << "Decode input:" << D.Input_Instruction -> instruction_string << ";	DI stalled " << D.Input_Instruction -> stalled << endl;
					cout << "Decode output:" << D.output_Instruction -> instruction_string << ";	DO stalled " << D.output_Instruction -> stalled << endl;
					*/
				

					cout << endl << endl;
					//cout << "Issue Queue" << endl;
					for (int i = 0; i < 16; i++)
					{
						if (IS.entry[i].Q_valid) {
						//	cout << "Entry " << i << " - " << IS.entry[i].Qinstruction_string << "; Opcode - " << IS.entry[i].Qs1_valid << IS.entry[i].Qs2_valid << endl;
						}
					}
					cout << endl << endl;


					cout << endl << endl;
					/*
					cout << "LSQ Entry" << endl;
					for (int j= 0 ; j <32 ; j++ ){
 						
						if (lsq_name.lsqentry[j].valid){
							cout << "Entry " << i << " - " << lsq_name.lsqentry[j].lsqstring << endl;
						}
					}
					cout << endl << endl;

					
					/*cout << "ROB Entry" << endl;
					for (int i = reorder_buffer.start_pt; i < reorder_buffer.end_pt; i++)
					{
						cout << "Entry " << i << " - " << reorder_buffer.robentry[i].instruction_string << "; " << reorder_buffer.robentry[i].status << endl;
					}
					cout << endl << endl;
					*/

					cout << "Cycle " << total_clock_cycles << ":" << endl << endl;
					cout << "Fetch \t :" << F.output_Instruction -> instruction_string  <<endl;
					cout << "DRF \t :" << D.output_Instruction -> instruction_string  << endl << endl;
					cout << "\t<RENAME TABLE>\t" << endl;
					for (int i = 0 ;i < 16 ; i++ ){
						if ( R_name.rtable[i].phyarch == true ){
							cout << "    * R"<< i <<": P"<< R_name.rtable[i].address << endl; //for loop
							}}
					cout << "\t<IQ>:" << endl;
					for (int i = 0 ;i < 16 ; i++ ){
						if ( IS.entry[i].Q_valid == true ){
							cout << "*" << IS.entry[i].Qinstruction_string << endl;}}
						
					//cout << //print for iq
					cout << "\t<ROB>:" << endl;
					for (int i = reorder_buffer.start_pt; i < reorder_buffer.end_pt; i++)
					{
						cout << " * " << reorder_buffer.robentry[i].instruction_string << endl;
					}
					cout << endl << endl;

					//cout << "Head - " << reorder_buffer.start_pt << endl;
					//cout << "Tail - " << reorder_buffer.end_pt << endl;

					//cout << endl << endl;
					
					//cout << // print for rob
					cout << "\tCommit :" << endl;
					//for(int r =0;r<16;){}
					cout << "\t<LSQ> :" << endl;
					//lsq display
					for (int j= 0 ; j <32 ; j++ ){
 						
						if (lsq_name.lsqentry[j].valid){
							
							cout << " * " << j << ":" << lsq_name.lsqentry[j].lsqstring<< endl;

						}

					}
					//cout << lsq_name.endpt << endl;
					cout << endl << endl;
					//cout << endl << mem_counter << endl;
					
					cout << "INTFU \t :" << E.output_Instruction -> instruction_string << endl;
					cout << "MUL1  \t :" << EM1.output_Instruction -> instruction_string << endl;
					cout << "MUL2  \t :" << EM2.output_Instruction -> instruction_string << endl;
					cout << "DIV1  \t :" << ED1.output_Instruction -> instruction_string << endl;
					cout << "DIV2  \t :" << ED2.output_Instruction -> instruction_string << endl;
					cout << "DIV3  \t :" << ED3.output_Instruction -> instruction_string << endl;
					cout << "DIV4  \t :" << ED4.output_Instruction -> instruction_string << endl;
					cout << "MEM   \t :" << M.output_Instruction -> instruction_string << endl; 
					
					/*for (int j= 0 ; j <32 ; j++ ){
 						
						if (lsq_name.lsqentry[j].valid = true){
							cout << endl << lsq_name.lsqentry[j].lsqstring << "output" << endl;}}

					/*cout << "Execute input:" << E.Input_Instruction -> instruction_string << ";	EI stalled " << E.Input_Instruction -> stalled << endl;
					cout << "Execute output:" << E.output_Instruction -> instruction_string << ";	EO stalled " << E.output_Instruction -> stalled << endl;
					cout << "Mul 1 input:" << EM1.Input_Instruction -> instruction_string << ";	EM1I stalled " << EM1.Input_Instruction -> stalled << endl;
					cout << "Mul 1 output:" << EM1.output_Instruction -> instruction_string << ";	EM1O stalled " << EM1.output_Instruction -> stalled << endl;
					cout << "Mul 2 input:" << EM2.Input_Instruction -> instruction_string << ";	EM2I stalled " << EM2.Input_Instruction -> stalled << endl;
					cout << "Mul 2 output:" << EM2.output_Instruction -> instruction_string << ";	EM2O stalled " << EM2.output_Instruction -> stalled << endl;
					cout << "DIV 1 input:" << ED1.Input_Instruction -> instruction_string << ";	ED1I stalled " << ED1.Input_Instruction -> stalled << endl;
					cout << "DIV 1 output:" << ED1.output_Instruction -> instruction_string << ";	ED1O stalled " << ED1.output_Instruction -> stalled << endl;
					cout << "DIV 2 input:" << ED2.Input_Instruction -> instruction_string << ";	ED2I stalled " << ED2.Input_Instruction -> stalled << endl;
					cout << "DIV 2 output:" << ED2.output_Instruction -> instruction_string << ";	ED2O stalled " << ED2.output_Instruction -> stalled << endl;
					cout << "DIV 3 input:" << ED3.Input_Instruction -> instruction_string << ";	ED3I stalled " << ED3.Input_Instruction -> stalled << endl;
					cout << "DIV 3 output:" << ED3.output_Instruction -> instruction_string << ";	ED3O stalled " << ED3.output_Instruction -> stalled << endl;
					cout << "DIV 4 input:" << ED4.Input_Instruction -> instruction_string << ";	ED4I stalled " << ED4.Input_Instruction -> stalled << endl;
					cout << "DIV 4 output:" << ED4.output_Instruction -> instruction_string << ";	ED4O stalled " << ED4.output_Instruction -> stalled << endl;
					cout << "Memory temp2 input:" << M2_temp.Input_Instruction -> instruction_string << ";	M2_temp stalled " << M2_temp.Input_Instruction -> stalled << endl;					
					cout << "Memory temp input:" << M_temp.Input_Instruction -> instruction_string << ";	M_temp stalled " << M_temp.Input_Instruction -> stalled << endl;
					cout << "Memory input:" << M.Input_Instruction -> instruction_string << ";	MI stalled " << M.Input_Instruction -> stalled << endl;
					cout << "Memory output:" << M.output_Instruction -> instruction_string << ";	MO stalled " << M.output_Instruction -> stalled << endl;
					cout << "WriteBack input:" << W.Input_Instruction -> instruction_string << ";	WI stalled " << W.Input_Instruction -> stalled << endl;
					cout << "WriteBack output:" << W.output_Instruction -> instruction_string << ";	WO stalled " << W.output_Instruction -> stalled << endl;
					cout << "Total clock cycles: " << total_clock_cycles << endl;	
					cout << "Zero Flag: " << fg.zero << endl;
					

						

					cout << "Rob HEAD - " << reorder_buffer.start_pt << endl;
					cout << "Rob Tail - " << reorder_buffer.end_pt << endl; 


					*/

					if (stop){
						prog_end = true;
						cout << "Simulation complete - " << total_clock_cycles << endl;
						break;			
					}		
				}
			
			}
			else
			{
			 	cout << "Initialize before Simulating" << endl;
			}
		}
	
	
		else if (stage == "3"){
			if(initialize){
				
				for(int z = 0;z<16;z++){
						
						cout << "value bit" << Reg_Obj.phy[z].value << endl;
						cout << " value " << Reg_Obj.phy[z].value << endl << endl;
						cout <<"cc value" << endl << Reg_Obj.phy[z].cc<<endl;
					}				
					cout << "Rename:" <<   ";	DO stalled " << D.output_Instruction -> stalled << endl;
				for(k=0;k<=16;k++){
					cout << "register value of R"<< k <<" "<< Reg_Obj.reg[k].value << " - " << Reg_Obj.reg[k].status << endl;
				}
				for(l=0;l<400;l+=4){
					cout << "value present in the memory location - " << l << " "<< Data_Obj.data[l]<< endl;			
				}
			}	
			else{
				cout << "Initialize the registers and memory before  displaying" << endl;	
			}			
			
		}

		else if (stage == "4"){
			cout << "Exited Successfully" << endl;
		
			break;
		}


	}	
	
	
	return 0;
}

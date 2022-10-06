#include <string.h>
#include <iostream>
#include <stdio.h>
#include <fstream> 
#include <ctype.h>
#include <algorithm>
#include <regex>
#include <map>
#include <iterator>
#include <vector>
#include <set>
#include <iomanip>

using namespace std;

// function declarations
void Pass1();
void Pass2();
int readInt(bool optional = false); 
struct Symbol readSymbol(bool optional = false);
void CreateSymbol(string s, int value); 
char readAEIR(bool optional = false);
bool validCountAndOp(int num);
bool validSymbol(string s);
void getToken(); 
void __parseerror(int errcode);
void checkWarnings(vector<string> definitions, int moduleSize, int modBase, int modCount);
bool checkIllegalOpCode(int opcode);
string initialize(int mapOrder);
bool exceedsModuleSize(int operand);
bool entryTooLarge(int operand, int size);
string __instructionerror(int errorcode);
void relocateToAbsoluteAddressing(int modBase);

// global variable declarations 
string file="", strLine="";  
char *token = NULL; 
char *charLine = NULL; 
char delimiters[] = " \t\n"; 
map <string,int> Symbol_Table; 
ifstream input;
int parseErrorFlag = -1; 
vector<string> errorSymbols;
int mapOrder = 0;
//set<string> program_uselist;
int line_num = 0, line_offset = 0;
int prevLine = 0;
int flag = -1;


// struct declarations  
struct Symbol {
    string sym; 
}; 


// ======================= main function ========================
int main(int argc, char* argv[]) {
    
    file = argv[1];
    Pass1();
    input.close();
    map<string, int>::iterator it; 
    
    if(parseErrorFlag == -1) { 
        cout << "Symbol Table\n";
        for(it = Symbol_Table.begin(); it != Symbol_Table.end(); ++it)
        {
        
            cout << it->first << "=" << it->second;
            if(find(errorSymbols.begin(), errorSymbols.end(), it->first) != errorSymbols.end()) {
                cout << " Error: This variable is multiple times defined; first value used";
            }
            cout << '\n';
        }
    Pass2();
    Symbol_Table.clear();
    errorSymbols.clear();
    input.close();
    }
}

void Pass1() {
     
    int modCount = 1; int modBase = 0; 
    input.open(file);

    if(input.is_open()) {
        while (true)
        {
            getToken();  
            int defCount = readInt(true); 
            struct Symbol symbol;
            int val; 

            if(defCount == -1) break;
            
            //if(!validCountAndOp(defCount)) break;
            if(defCount > 16) {
                __parseerror(4);
                break; 
            }  
  
            vector<string> definition_list;  
            for(int i = 0; i < defCount; ++i) {  
                getToken();
                symbol = readSymbol();
                if(!validSymbol(symbol.sym)) return; 
                
                getToken();
                int value = readInt();
                if(!validCountAndOp(value)) return;
                
                definition_list.push_back(symbol.sym);
                CreateSymbol(symbol.sym, modBase+value);
            }
                

            getToken();

            int useCount = readInt();

            if(!validCountAndOp(useCount)) return; 
            if(useCount > 16) {
                __parseerror(5);
                return; 
            } 

            for(int i = 0; i < useCount; ++i) {  
                getToken();
                symbol = readSymbol();
                if(!validSymbol(symbol.sym)) return; 

            }

            getToken(); 

            int instCount = readInt();
            if(instCount + modBase > 512) {
                __parseerror(6); 
                return; 
            } 

            for(int i = 0; i < instCount; ++i) {  

                getToken();
                char addressmode = readAEIR();
                if(addressmode=='\0') return;

                getToken(); 
                int op = readInt();
                if(!validCountAndOp(op)) return ; 
                
            }
            checkWarnings(definition_list, instCount, modBase, modCount);
            modCount++; 
            modBase = modBase + instCount;
            definition_list.clear();      
        }
    }

    input.close();
    return;
} 

void Pass2() {
    cout << "\nMemory Map\n";
    int modCount = 1; int modBase = 0; 
    set<string> program_uselist;
    input.open(file);
    map<string, int> definitionMap = Symbol_Table; 
    if(input.is_open()) {
        while (true)
        {
            getToken();  
            int defCount = readInt(true); 
            struct Symbol symbol;
            int val; 

            if(defCount == -1) break;   
            
             
            for(int i = 0; i < defCount; ++i) {  
                getToken();
                symbol = readSymbol();
                getToken();
                int value = readInt();
                definitionMap[symbol.sym] = modCount;
            }

            getToken();   
            int useCount = readInt();


            vector<string> use_list; 
            vector<bool> used_flag;  
            for(int i = 0; i < useCount; ++i) {  
                getToken();
                symbol = readSymbol();
                program_uselist.insert(symbol.sym);
                use_list.push_back(symbol.sym);
                used_flag.push_back(false);
            }
            getToken(); 

            int instCount = readInt();
            for(int i = 0; i < instCount; ++i) {  

                getToken();
                char addressmode = readAEIR();
                if(addressmode=='\0') return;
                getToken(); 
                int op = readInt();
                if(!validCountAndOp(op)) return ; 
                int operand = op%1000, opcode = op/1000; 

                
                string message=""; 

                if(checkIllegalOpCode(opcode) && addressmode != 'I'){
                    operand = 999; opcode = 9;
                    message = __instructionerror(4); 
                }
                else { 
                    switch (addressmode) {
                        case 'A': 
                            if(exceedsModuleSize(operand)) {
                                message = __instructionerror(0);
                                operand=0;
                            }
                            break;
                        case 'I': 
                            if(op >= 10000) {
                                operand = 999; opcode = 9;
                                message = __instructionerror(3);
                            }
                            break;
                        case 'R': 
                            if(entryTooLarge(operand, instCount)) {
                                message = __instructionerror(1);
                                operand=0;
                            }
                            operand = modBase + operand;
                            break;
                        case 'E': 
                            used_flag[operand] = true;
                            if(entryTooLarge(operand, use_list.size())) {
                                message = __instructionerror(2);
                            }
                            else if(Symbol_Table.find(use_list[operand]) == Symbol_Table.end()) {
                                message = "Error: " + use_list[operand] + " is not defined; zero used";
                                operand = 0;
                            }
                            else {
                                operand = Symbol_Table[use_list[operand]];
                            }
                            break;                         
                    }
                }
                op = operand + opcode*1000; 
                cout << setw(3) << setfill('0') << mapOrder <<  ": ";
                cout << setw(4) << setfill('0') << op; 
                cout << " " << message << "\n";
                mapOrder++;
            }
            vector<bool>::iterator it; 
            if(used_flag.empty() == false) {
                for(it = used_flag.begin(); it != used_flag.end(); ++it) {
                    if(*it == false) {
                        cout << "Warning: Module "<<to_string(modCount)<<": "<<use_list[it-used_flag.begin()]<<" appeared in the uselist but was not actually used\n"; 
                    }
                }
            } 
            used_flag.clear();
            use_list.clear();
            modCount++; 
            modBase = modBase + instCount;
        }
        map<string, int>::iterator it2;
        cout << "\n"; 
        if(definitionMap.empty() == false) {
            for(it2 = definitionMap.begin(); it2 != definitionMap.end(); it2++) {
                if(program_uselist.find(it2->first) == program_uselist.end()) 
                    cout << "Warning: Module "<< it2->second << ": "<< it2->first <<" was defined but never used\n";
            }
        }

    } 
    input.close();
    definitionMap.clear();
    program_uselist.clear();
}

int readInt(bool optional) { 
    
    if(token == NULL) { 
        if(optional == true) 
           return -1;
        else {
            __parseerror(0);
            return -1;
        }
    } 

    char c = token[0]; 
    for(int i=1; c!= '\0'; ++i) {
        if(isdigit(c)); 
        else {
            __parseerror(0);
            return -1; 
        }
        c = token[i]; 
        //cout << c; 
    }
    return stoi(token); 
}

struct Symbol readSymbol(bool optional) {
    
    struct Symbol s;
    if(token == NULL) {
        s.sym = "\0";
        if(optional == true)
           return s;
        else {
            __parseerror(1);
            return s;
        }
    }
      
    s.sym = token;
    if(regex_match(token, regex("[a-zA-z][a-zA-Z0-9]*"))); 
    else if(s.sym.length() > 17) {
        __parseerror(3);
        s.sym = "\0"; 
    }
    else {
         __parseerror(1);
         
         s.sym = "\0"; 
    }

    return s;  
}

void CreateSymbol(string s, int value) {
     if((Symbol_Table.find(s))==Symbol_Table.cend()) {
        Symbol_Table.insert(make_pair(s, value));
     }
     else {
        errorSymbols.push_back(s);
     }
}

char readAEIR(bool optional) {
    if(token == NULL) {
        if(optional == true)
           return '\0';
        else {
            __parseerror(2);
            return '\0';
        }
    }
    
    char c = token[0]; 

    if(c == 'A' || c == 'E' || c == 'I' || c == 'R');
    else {
        
        c = '\0';
        __parseerror(2); 
    }
    return c; 
}

bool validCountAndOp(int num) {
    if (num == -1) {
        return false; 
    }
    return true; 
}

bool validSymbol(string s) {
    if (s == "\0") {
        return false; 
    }

    return true; 
}

void getToken() {
    static bool firsttime = true; 
    char *next;
    
    if(firsttime) {
        next = NULL; 
        firsttime = false; 
    }
    else {
        next = strtok(NULL, delimiters);
    }
    if(next == NULL) {
        while(true) {
            if(input.eof()) {
                line_offset = prevLine+1;
                //cout << prevLine << "+" << line_offset;
                line_num--;
                token = NULL;
                return ;
            }
            ++line_num;
            getline(input, strLine);
            if(strLine.empty() && !input.eof())  {
                prevLine = 0;
                line_offset = 1;
                continue; 
            }
           char* charLine = (char *) strLine.c_str();
           token = strtok(charLine, delimiters);
           if(token != NULL) {
              prevLine = strLine.length();
              line_offset = 1;
              break;
           }
        }
    }
    else {
        int add = strlen(token);
        token = next; 
        line_offset = line_offset + add +  1;
    }
}
void __parseerror(int errcode) {
    parseErrorFlag = 0;
    static string errstr[] = { "NUM_EXPECTED","SYM_EXPECTED","ADDR_EXPECTED","SYM_TOO_LONG","TOO_MANY_DEF_IN_MODULE","TOO_MANY_USE_IN_MODULE", "TOO_MANY_INSTR"};
        cout << "Parse Error line "<< line_num <<" offset " << line_offset << ": " << errstr[errcode] << "\n";
        exit(0);
}

void checkWarnings(vector<string> definitions, int moduleSize, int modBase, int modCount) {
   vector<string>::iterator it;
   for(it = definitions.begin(); it!=definitions.end(); it++) {
      if(Symbol_Table[*it]-modBase >= moduleSize) {
          cout << "Warning: Module " << modCount <<": "<<*it<<" too big "<<Symbol_Table[*it]-modBase<<" (max="<<moduleSize-1<<") assume zero relative\n";
          Symbol_Table[*it] = modBase + 0; 
      }
   }
}

bool checkIllegalOpCode(int opcode) {
    if(opcode >= 10) {
        return true;
    }
    return false; 
}

string initialize(int mapOrder) {
    if(mapOrder >= 0 && mapOrder <= 9) {
        return string(2, '0')+(to_string(mapOrder));
    }

    else if(mapOrder >= 10 && mapOrder <= 99) {
        return string(1, '0')+(to_string(mapOrder));
    }

    else if(mapOrder >= 100 && mapOrder <= 256) {
        return to_string(mapOrder); 
    }
    return "";
}

bool exceedsModuleSize(int operand) {
    if(operand >= 512) {
        return true;
    }
    else {
        return false;
    } 
}

bool entryTooLarge(int operand, int size) {
    if(operand >= size) {
        __instructionerror(2);
        return true;
    }
    return false; 
}

string __instructionerror(int errorcode) {
    static string ierrstr[5] = { "Absolute address exceeds machine size; zero used","Relative address exceeds module size; zero used","External address exceeds length of uselist; treated as immediate","Illegal immediate value; treated as 9999","Illegal opcode; treated as 9999"}; 
    return "Error: "+ierrstr[errorcode];
}

void relocateToAbsoluteAddressing(int modBase) {
    map<string, int>::iterator i; 
    for(i = Symbol_Table.begin(); i!=Symbol_Table.end(); ++i) {
        Symbol_Table[i->first] = modBase + i->second;  
    }
}

#include <string.h>
#include <iostream>
#include <stdio.h>
#include <fstream> 
#include <unistd.h>
#include <cstring>
#include <vector>
#include <iterator>
#include <queue>
#include <deque>
#include <vector>
#include <set>
#include <limits>
#include <climits>

// remember vma is dynamically allocated 

using namespace std; 

struct pte_t; 
struct frame_t;
struct memory_area;
class Process; 
class Pager;
class FIFO; 
class Clock;
class ESC; 
class Aging; 
class WorkingSet; 
class Random; 

int num_frames = 0; 
char algo='\0'; 
int O_flag=0, P_flag=0, F_flag=0, S_flag=0; 
string inputFile, randFile;
fstream input, rfile;
const int MAX_VPAGES = 64; 
int MAX_FRAMES = 128; 
vector <Process *> processes;
int inst_count = 0; 
char operation = '\0'; 
int vpage = 0; 
string line = "";
Process* currentproc = nullptr; 
int fpos = 0;
int reset_flag = 0; 
int icount = 0; 
int ofs = -1, size = 0;
vector<int> randvals;

unsigned long ctx_switches = 0, process_exits = 0; 
unsigned long long cost = 0; 

void setOptionsFlag(string options); 
struct frame_t *get_frame();
bool get_next_instruction(char* operation, int* vpage);
bool page_fault_handler(int* vpage); 
struct frame_t* allocate_frame_from_free_list();
void print_page_table(Process* p); 
int myrandom(int burst);
void readRandNums();

void setOptionsFlag(string options) {
    if (options.find('O') != string::npos) {
        O_flag = 1;
    }
    if(options.find('P') != string::npos) {
        P_flag = 1; 
    }
    if(options.find('F') != string::npos) {
        F_flag = 1;
    }
    if(options.find('S') != string::npos) {
        S_flag = 1; 
    }

    return ; 
}

struct pte_t {

    // 5 bits for flags and 7 bits for max=128 frames
    unsigned int present:1; 
    unsigned int referenced:1; 
    unsigned int modified:1;  
    unsigned int write_protect:1; 
    unsigned int pagedout:1; 
    unsigned int frame_number:7; 
    unsigned int file_mapped:1;
    unsigned int visited:1; 

    pte_t() {
        present = 0; 
        referenced = 0; 
        modified = 0; 
        write_protect = 0; 
        pagedout = 0; 
        frame_number = -1; 
        file_mapped = 0; 
        visited = 0;
    } 

    // free bits, can be used for filemapped or not and checking if vpage belongs to any of the vmas 

}; 

struct frame_t {
    // use pid or process object? if process object pointer then I can just access the pagetable or do I need to 
    // have a separate field for it?
    int pid; 
    pte_t* pte;
    int vpage; 
    int frameid; 
    unsigned int allocated:1; 
    static int fcount; 
    unsigned int age; 
    long long last_used; 

    frame_t() {
        allocated = 0; 
        pte = nullptr;    
        frameid = fcount; 
        cout << "frame count: " << fcount; 
        ++fcount; 
        age = 0; 
        last_used = 0; 
    }
}; 

struct frame_t* frame_table = nullptr;
int frame_t::fcount = 0;
vector<struct frame_t*> free_frames; 
struct frame_t* free_frame = nullptr;

struct memory_area {
    unsigned int start_vpage; 
    unsigned int end_vpage; 
    unsigned int write_protected; 
    unsigned int file_mapped; 
}; 

class Process {
    public: 
        static int count; 
        int vcount; 
        int pid;
        vector<struct memory_area> VMA; 
        struct pte_t page_table[MAX_VPAGES];
        unsigned long U, M, I, O, FI, FO, SV, SP, Z; 
        

        Process(int num_of_vma) {
            pid = Process::count; 
            Process::count++;
            vcount = 0;
            VMA.resize(num_of_vma);
            U = 0;
            M = 0;
            I = 0; 
            O = 0; 
            FI = 0; 
            FO = 0; 
            SV = 0; 
            SP = 0; 
            Z = 0; 
        }

        void print_vma() {
            for(auto it = VMA.begin(); it != VMA.end(); ++it) {


                cout << (*it).start_vpage << " "; 
                cout << (*it).end_vpage << " "; 
                cout << (*it).write_protected << " "; 
                cout << (*it).file_mapped << " "; 
            }
        }

        void initializeVMA(int start, int end, int write, int filemap) {
            VMA[vcount].start_vpage = start; 
            VMA[vcount].end_vpage = end;
            VMA[vcount].write_protected = write; 
            VMA[vcount].file_mapped= filemap;
            vcount++; 
            int len = 0; 
            /*for(struct memory_area mem: VMA) {
                len++;
            }
            cout << "Size of VMA: " << len << endl; */
        }

        struct pte_t* getPageTable(int vpage) {
            return page_table + vpage;
        }
}; 

int Process::count = 0; 

class Pager {
    public: 
        frame_t* hand; 
        static int pos; 
        virtual frame_t* select_victim_frame() = 0;
        virtual void algo_name() = 0; // virtual base class
}; 

//struct frame_t* victim_frame = nullptr; 
class FIFO: public Pager {
    public:
        struct frame_t* hand; 
        static int pos; 
        
        struct frame_t* select_victim_frame() {  
            if(pos >= 0 && pos < MAX_FRAMES) {
                struct frame_t* victim_frame = frame_table + FIFO::pos;
                FIFO::pos++; 
                if(FIFO::pos >= MAX_FRAMES) {
                    FIFO::pos = 0; 
                }    
                return victim_frame;
            }
            // return nullptr;
        } // virtual base class

        FIFO() { 
            //cout << "FIFO Algoritm\n";
            hand = frame_table;
            FIFO::pos = 0; 
        }

        void algo_name() {
            cout << "FIFO Algorithm set\n"; 
        }
};

class Clock: public Pager {
    public:
        struct frame_t* hand; 
        static int pos; 
        
        struct frame_t* select_victim_frame() { 
            struct frame_t* victim_frame = frame_table + Clock::pos; 
            //cout << "page considered: " << victim_frame->vpage << endl; 
            //struct pte_p* p = victim_frame->pte;
            if(victim_frame->pte->referenced == 0) {
                //cout << "page evicted: " << victim_frame->vpage <<endl;
                Clock::pos++;
                if(Clock::pos >= MAX_FRAMES) {
                    //cout << "wrapping around!\n";
                    Clock::pos = 0; 
                } 
                return victim_frame;
            }

            else if(victim_frame->pte->referenced == 1) {
                //cout << "page is being pushed to the tail\n"; 
                victim_frame->pte->referenced = 0;
                //cout << "page " << victim_frame->vpage << " reference is reset\n";
                Clock::pos++;
                //cout << "advancing the position of hand\n";
                if(Clock::pos >= MAX_FRAMES) {
                    //cout << "wrapping around!\n";
                    Clock::pos = 0; 
                } 
                return select_victim_frame();
            }
        } // virtual base class

        Clock() { 
            //cout << "FIFO Algoritm\n";
            hand = frame_table;
            Clock::pos = 0; 
        }

        void algo_name() {
            cout << "Clock Algorithm set\n"; 
        }
};

class ESC: public Pager {
    public:
        struct frame_t* hand; 
        static int pos; 
        struct frame_t* class_0;
        struct frame_t* class_1; 
        struct frame_t* class_2; 
        struct frame_t* class_3; 
    

        ESC() { 
            //cout << "ESC Algoritm\n";
            hand = frame_table;
            icount = 0;
        }

        struct frame_t* select_victim_frame() {
            struct frame_t* victim_frame; 
            int i;
            class_0 = nullptr; 
            class_1 = nullptr; 
            class_2 = nullptr; 
            class_3 = nullptr;

            /*if(current_reset == 1) {
                struct frame_t* p = frame_table + (ESC::pos - 1); 
                p->pte->referenced = 0; 
                current_reset = 0; 
            }*/

            //cout << "Instruction: " << inst_count << endl;
            //cout << "Position is: " << ESC::pos << endl;
            for(i = 0; i < MAX_FRAMES; ++i) {
                
                //cout << "Entering for loop beginning: " << i << endl;
                struct frame_t* f = frame_table + ESC::pos; 
                if(f->pte->referenced == 0 && f->pte->modified == 0 && class_0 == nullptr) {
                    //cout << "Class 0\n"; 
                    class_0 = f; 
                    if(icount < 50) 
                        break;
                }
                else if(f->pte->referenced == 0 && f->pte->modified == 1 && class_1 == nullptr) {  
                    //cout << "Class 1 set!\n"; 
                    class_1 = f; 

                }
                else if(f->pte->referenced == 1 && f->pte->modified == 0 && class_2 == nullptr) {   
                    //cout << "Class 2 set!\n";
                    class_2 = f; 
                }
                else if(f->pte->referenced == 1 && f->pte->modified == 1 && class_3 == nullptr) {  
                    //cout << "Class 3 set!\n";  
                    class_3 = f; 
                }

                ESC::pos = (ESC::pos + 1)%MAX_FRAMES;
            }

            //cout << "Reference bits???\n"; 
            /*
            if(reset_flag == 1) {
                //cout << "Reset reference bits at " << inst_count << "\n"; 
                for(int i = 0; i < MAX_FRAMES; ++i) {
                    struct frame_t* f = frame_table + i; 
                    f->pte->referenced = 0; 
                }
                reset_flag = 0; 

            }*/ 

            if(icount >= 50) {
                for(int i = 0; i < MAX_FRAMES; ++i) {
                    (frame_table+i)->pte->referenced = 0;
                }
                icount = 0; 
            }

            if(class_0 != nullptr) {
                //cout << "Evicting frame of class 0\n";
                ESC::pos = class_0 - &frame_table[0] + 1; 
                victim_frame = class_0;
                class_0 = nullptr; 
            }
            else if(class_1 != nullptr) {
                //cout << "Evicting frame of class 1\n"; 
                ESC::pos = class_1 - &frame_table[0]  + 1;
                victim_frame =  class_1;
                class_1 = nullptr; 
            }
            else if(class_2 != nullptr) {
                //cout << "Evicting frame of class 2\n"; 
                ESC::pos = class_2 - &frame_table[0]  + 1; 
                victim_frame = class_2;
                class_2 = nullptr; 
            }
            else if(class_3 != nullptr) {
                //cout << "Evicting frame of class 3\n"; 
                ESC::pos = class_3 - &frame_table[0]  + 1; 
                victim_frame = class_3; 
                class_3 = nullptr; 
            }

            ESC::pos = ESC::pos % MAX_FRAMES;

            //cout << ESC::pos << " " << MAX_FRAMES << "\n";
            /*
            struct frame_t* f; 
            for(int i = 0; i < MAX_FRAMES; ++i) {
                    f = frame_table + i; 
                    cout << f->vpage << ":"; 
                    cout << "" << (f->pte->referenced*2 + f->pte->modified) << " ";
            }
            cout << "\n";*/
            

            return victim_frame;
            }

        void algo_name() {
            cout << "ESC Algorithm set\n"; 
        }

};

class Aging: public Pager {
    public:
        struct frame_t* hand; 
        static int pos; 

        void modifyAge() {
            for(int i = 0; i < MAX_FRAMES; ++i) {
                struct frame_t* f = frame_table + i; 
                f->age = f->age >> 1; 
                if(f->pte->referenced == 1) {
                    f->age = (f->age | 0x80000000); 
                    f->pte->referenced = 0; 
                }
            }
        }
        
        struct frame_t* select_victim_frame() {  
           struct frame_t* victim_frame; 
           modifyAge();
           long lowest_age = 5000000000;
           for(int i = 0; i < MAX_FRAMES; ++i) {
                struct frame_t* f = frame_table + ESC::pos; 
                if(f->age < lowest_age) {
                    lowest_age = f->age;
                    victim_frame = f; 
                }
                ESC::pos = (ESC::pos + 1)%MAX_FRAMES;
           }
           ESC::pos = (victim_frame - frame_table) + 1;
           ESC::pos = ESC::pos % MAX_FRAMES;

           return victim_frame; 
        } // virtual base class

        Aging() { 
            //cout << "FIFO Algoritm\n";
            hand = frame_table;
            Aging::pos = 0; 
        }

        void algo_name() {
            cout << "Aging Algorithm set\n"; 
        }
};

class WorkingSet: public Pager {

    public:

        int tau;
        static int pos;

        struct frame_t * select_victim_frame() {    
            unsigned long long lowest_used = ULLONG_MAX;
            unsigned int lowest_unused = ULLONG_MAX;    
            struct frame_t* u = nullptr; 
            struct frame_t* un = nullptr; 

            for(int i =0; i< MAX_FRAMES; i++){
                
                struct frame_t *victim_frame = frame_table + WorkingSet::pos;

                if(victim_frame->last_used < lowest_used) {
                        lowest_used = victim_frame->last_used;
                        u = victim_frame;
                } 
                if(victim_frame->pte->referenced==1){
                    victim_frame->last_used = icount;
                    victim_frame->pte->referenced = 0;
                } 
                else {
                    if(victim_frame->last_used < lowest_unused){
                            lowest_unused = victim_frame->last_used;
                            un = victim_frame; 
                    }  
                    if((icount-victim_frame->last_used) >= 50) {         
                            WorkingSet::pos = ((victim_frame - frame_table)+1)%MAX_FRAMES;
                            return &frame_table[(victim_frame - frame_table)];
                    }
                }
                WorkingSet::pos = (WorkingSet::pos+1)%MAX_FRAMES;
            }

            if(un != nullptr) {
                WorkingSet::pos = (un - frame_table+1)%MAX_FRAMES;
                return un;
            } 
            else {
                WorkingSet::pos = (u - frame_table+1)%MAX_FRAMES;
                return u;

            }
        }   

        WorkingSet(){
            pos = 0;
            icount = 0;
            tau = 0;
        }

        void algo_name() {
            cout << "WS Algorithm set\n"; 
        }

};

class Random: public Pager {

    public:
        struct frame_t* hand; 
        static int pos; 
        
        struct frame_t* select_victim_frame() {  
            int random = myrandom(MAX_FRAMES);
            struct frame_t* victim_frame = frame_table + (random - 1); 
            return victim_frame;
            // return nullptr;
            // virtual base class
        }

        Random() { 
            //cout << "FIFO Algoritm\n";
            hand = frame_table;
            Random::pos = 0; 
        }

        void algo_name() {
            cout << "Random Algorithm set\n"; 
        }
};

int FIFO::pos = 0;
int Clock::pos = 0;
int ESC::pos = 0;
int Aging::pos = 0;
int WorkingSet::pos = 0; 
int Random::pos = 0; 

struct frame_t* allocate_frame_from_free_list() {

    if(free_frames.empty()) {
        return nullptr; 
    }

    free_frame = free_frames.front(); 
    free_frames.erase(free_frames.begin());
    return free_frame; 
} 

struct frame_t *get_frame(Pager* THE_PAGER) {
    struct frame_t *frame = allocate_frame_from_free_list();

    if (frame == nullptr) {
        //THE_PAGER->algo_name();
        frame = THE_PAGER->select_victim_frame();
        cout << " UNMAP " << frame->pid << ":" << frame->vpage << endl;
        Process* temp = processes[frame->pid];
        temp->U++; 
        frame->pte->present = 0;
        if(frame->pte->modified == 1) {
            frame->pte->referenced = 0; 
            frame->pte->modified = 0; 
            frame->pte->frame_number = -1;
            if(frame->pte->file_mapped == 1) {
                cout << " FOUT\n";
                temp->FO++; 
            }
            else {
                frame->pte->pagedout = 1;
                cout << " OUT\n";
                temp->O++; 
            }
        }
    }
    return frame;
    //return nullptr;
} 

bool get_next_instruction(char* operation, int* vpage) {
    if(input.eof()) {
        return false; 
    }
    getline(input, line); 
    if(line[0] == '#') {
        return false;
    }
    //cout << "Instruction parsed: " << line << endl; 
    char* str = (char*) line.c_str();
    sscanf(str, "%c %d", operation, vpage);
    cout << inst_count << ": ==> ";
    cout << *operation << " " << *vpage << endl;
    if((inst_count + 1)%50 == 0) {
        reset_flag = 1; 
    }
    inst_count++; 
    return true; 
}

bool page_fault_handler(int vpage) { 
    for(auto it = currentproc->VMA.begin(); it!= currentproc->VMA.end(); ++it) {
        if(vpage >= (*it).start_vpage && vpage <= (*it).end_vpage) {
            if(currentproc->page_table[vpage].visited == 0) {
                pte_t *p = &(currentproc->page_table[vpage]);
                p->write_protect = (*it).write_protected; 
                p->file_mapped = (*it).file_mapped;
                p->visited = 1; 
            }
            return true; 
        }
    }
    cout << " SEGV\n"; 
    currentproc->SV++;
    return false; 
}

void print_frame_table() {
    cout << "FT:"; 
    for(int i = 0; i < MAX_FRAMES; ++i) {
        cout << " ";
        struct frame_t* f = frame_table + i; 
        if(f->pte != nullptr)
            cout << f->pid << ":" << f->vpage;
        else 
            cout << "*";
    }
}

void print_page_table(Process* p) {
    printf("PT[%d]:", p->pid);
    for(int i = 0; i < MAX_VPAGES; ++i) {
        cout << " ";
        struct pte_t* page = p->page_table + i; 
        if(page->present == 1) {
            cout << i << ":"; 
            if(page->referenced == 1) {
                cout << "R"; 
            }
            else {
                cout << "-"; 
            }
            if(page->modified == 1) {
                cout << "M";
            }
            else {
                cout << "-"; 
            }
            if(page->pagedout == 1) {
                cout << "S"; 
            }
            else {
                cout << "-";
            }
        }

        else if(page->present == 0) {
            if(page->pagedout == 1) {
                cout << "#"; 
            }
            else if(page->pagedout == 0) {
                cout << "*"; 
            }
        }
    }
}

int myrandom(int burst) {
    ofs++;
    return 1 + (randvals[ofs%size] % burst);
}

void readRandNums() {
    string num = "\0";
    rfile.open(randFile);
    if(rfile.is_open()) {
        getline(rfile, num);
        size = stoi(num);
        //randvals = new int(stoi(num));
        for(int i = 0; getline(rfile, num); i++) {
                randvals.push_back(stoi(num));
        }
    }
    rfile.close();
}


int main(int argc, char** argv) {

    // PARSE COMMAND AND ITS OPTIONS
    string frames="", algos="", options=""; 
    char opt_f, opt_a; 
    int c; 
    while((c = getopt(argc, argv, "f:a:o:")) != -1) {
        switch(c) {
            case 'f': 
                frames = optarg; 
                sscanf(frames.c_str(), "%d",&num_frames);
                MAX_FRAMES = num_frames;
                break;
            case 'a':   
                algos = optarg; 
                sscanf(algos.c_str(), "%c", &algo);
                break; 
            case 'o':
                options = optarg; 
                setOptionsFlag(optarg);
                break; 
            case '?': 
                cout << "Unexpected!\n";
                break;
        }
        inputFile = argv[optind];
        randFile = argv[optind+1];
    }

    if(algo == 'r') {
        readRandNums();
    }

    // PARSE PROCESS AND ITS VMAs 
    string line = "";
    int num_of_proc = -1, num_of_vm = -1; 
    char* str; 
    char *token = nullptr;
    input.open(inputFile);
    Process *p;

    if(input.is_open()) {
        while(true) {
            getline(input, line);
            //cout << "Line parsed: " << line << "\n";
            if(input.eof()) {
               break; 
            }

            if(line[0] != '#') {
                str = (char*) line.c_str();
                if(num_of_proc == -1) {    
                    token = strtok(str, " ");
                    num_of_proc = stoi(token); 
                    continue; 
                }
                
                //cout << "Number of processes: " << num_of_proc << endl; 
                for(int i = 0; i < num_of_proc; i++) {
                    token = strtok(str, " ");
                    num_of_vm = stoi(token);
                    p = new Process(num_of_vm); 
                    int var1, var2, var3, var4; 
                    //cout << "Number of vmas: " << num_of_vm << endl; 
                    for(int j = 0; j < num_of_vm; j++) {
                        getline(input, line);
                        str = (char*) line.c_str();
                        sscanf(str, "%d %d %d %d", &var1, &var2, &var3, &var4);
                        p->initializeVMA(var1, var2, var3, var4); 
                        //cout << "proc: " << i << endl; 
                        //cout << "vma: "  << j << endl; 
                        //p->print_vma();
                        //cout << line << endl; 
                    }
                    processes.push_back(p);
                    p = nullptr;
                    if(i+1 < num_of_proc) {
                        getline(input, line);
                        getline(input, line); 
                        getline(input, line);
                    }
                    else {
                        getline(input, line);
                    }
                }
                //cout << "Parsing instructions\n";

                Pager* THE_PAGER;

                if(algo == 'f') {
                    THE_PAGER = new FIFO(); 
                }
                else if(algo == 'c') {
                    THE_PAGER = new Clock();
                }

                else if(algo == 'e') {
                    THE_PAGER = new ESC();
                }

                else if(algo == 'a') {
                    THE_PAGER = new Aging();
                }
                else if(algo == 'w') {
                    THE_PAGER = new WorkingSet();
                }
                else if(algo == 'r') {
                    THE_PAGER = new Random();
                }

                frame_table = (struct frame_t*) malloc(sizeof(struct frame_t)*MAX_FRAMES);
                //memset(frame_table, nullptr, sizeof frame_table);

                for(int i = 0; i < MAX_FRAMES; ++i) {
                    free_frames.push_back(frame_table+i); 
                    frame_table[i].pte = nullptr;
                }
                
                while(get_next_instruction(&operation, &vpage)) {
                    icount++; 
                    // handle special case of “c” and “e” instruction
                    if(operation == 'c') {
                        int proc_num = vpage; 
                        currentproc = processes[proc_num];
                        ++ctx_switches; 
                    }

                    else if(operation == 'e') {
                        cout << "EXIT current process " << currentproc->pid << endl;
                        for(int i = 0; i < MAX_VPAGES; ++i) {
                            struct pte_t* p = &currentproc->page_table[i]; 
                            if(p->present == 1) {
                                struct frame_t* f = frame_table + p->frame_number;
                                free_frames.push_back(f);
                                cout << " UNMAP " << currentproc->pid << ":" << i << endl;
                                f->pte = nullptr;
                                currentproc->U++;
                                if(p->modified == 1) {
                                    p->referenced = 0; 
                                    p->modified = 0;
                                    p->frame_number = -1;
                                    if(p->file_mapped == 1) {
                                        cout << " FOUT\n";
                                        currentproc->FO++; 
                                    }
                                }
                            }
                            p->present = 0;
                            p->pagedout = 0;
                        }
                        ++process_exits; 
                    }

                     // now the real instructions for read and write 
                    if(operation == 'r' || operation == 'w') {

                        struct pte_t *current_pte = &currentproc->page_table[vpage];
 
                        if (current_pte->present == 0) {
                            if(page_fault_handler(vpage) == false) {
                                continue;  
                            }
                            struct frame_t* allocated_frame = get_frame(THE_PAGER); 

                            // frame is allocated to new pte 
                            allocated_frame->pte = current_pte;
                            allocated_frame->pid = currentproc->pid; 
                            allocated_frame->allocated = 1; 
                            allocated_frame->vpage = vpage; 

                            // pte is set to allocated frame number and valid bit is set
                            current_pte->frame_number = allocated_frame - frame_table;
                            current_pte->present = 1; 
                            
                            if(current_pte->pagedout == 1) {
                                
                                if(current_pte->file_mapped == 1) {
                                    cout<< " FIN\n"; 
                                    currentproc->FI++; 
                                }
                                else {
                                    cout << " IN\n"; 
                                    currentproc->I++;    
                                }
                            }

                            else {
                                if(current_pte->file_mapped == 1) {
                                    cout<< " FIN\n"; 
                                    currentproc->FI++; 
                                }
                                else {
                                    cout << " ZERO\n";
                                    currentproc->Z++; 
                                }
                               
                            }
                            cout << " MAP " << allocated_frame - frame_table << endl; 
                            allocated_frame->age = 0;
                            allocated_frame->last_used = icount; 
                            currentproc->M++;
                        
                        }
                        
                        if(operation == 'r' ) {
                            current_pte->referenced = 1; 
                            
                        }

                        if(operation == 'w') {
                            if(current_pte->write_protect == 1) {
                                cout << " SEGPROT\n";
                                currentproc->SP++;
                                current_pte->referenced = 1;
                            }
                            else {
                                current_pte->modified = 1; 
                                current_pte->referenced = 1;
                            }
                        }
                        /*
                        if(algo == 'a') {
                            struct pte_t *current = &currentproc->page_table[vpage];
                            for(int i = 0; i < MAX_FRAMES; ++i) {
                                frame_table[i].age = frame_table[i].age >> 1;
                                if(i == current->frame_number) {
                                    //cout << "entered at " << current_pte->frame_number << "\n";
                                    frame_table[current->frame_number].age = (frame_table[current->frame_number].age | 0x80000000);
                                }
                            }
                        }*/
                        //cout << "Page #: " << vpage << "\n"; 
                        //cout << current_pte->present << " " << current_pte->referenced << " " << current_pte->modified << " "; 
                        //cout << current_pte->pagedout << " " << current_pte->write_protect << " " << current_pte->file_mapped; 
                        //cout << "\n"; 
                    }
                }

            }
                
        }
        for(auto it = processes.begin(); it != processes.end(); ++it) {
            print_page_table(*it);
            cout << "\n";
        }
    
        print_frame_table();
        cout << "\n";
        cost = (inst_count - ctx_switches - process_exits)*1 + ctx_switches*130 + process_exits*1250; 
        for(auto it = processes.begin(); it != processes.end(); ++it) {
            printf("PROC[%d]: U=%lu M=%lu I=%lu O=%lu FI=%lu FO=%lu Z=%lu SV=%lu SP=%lu\n", (*it)->pid, (*it)->U, (*it)->M, (*it)->I, (*it)->O, (*it)->FI, (*it)->FO, (*it)->Z, (*it)->SV, (*it)->SP); 
            cost = cost + (*it)->M*300 + (*it)->U*400 + (*it)->I*3100 + (*it)->O*2700 + (*it)->FI*2800 + (*it)->FO*2400 + (*it)->Z*140 + (*it)->SV*340 + (*it)->SP*420; 
        }
        printf("TOTALCOST %lu %lu %lu %llu %lu\n", inst_count, ctx_switches, process_exits, cost, sizeof(pte_t)); 
        
    }
    
    /*cout << "processes: \n"; 

    for(auto it = processes.begin(); it!= processes.end(); ++it) {
        cout << (*it)->pid << endl;
        (*it)->print_vma();
        cout << "\n"; 
    }*/
    
}

// ${INDIR}/

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

using namespace std;

enum state {CREATED, READY, RUNNING, BLOCKED, COMPLETE};
enum trans {TRANS_TO_READY, TRANS_TO_PREEMPT, TRANS_TO_RUN, TRANS_TO_BLOCK, TRANS_TO_COMPLETE};

// Classes
class Process; 
class Event; 
class DESLayer; 
class Scheduler; 
class SchedulerFCFS;
class SchedulerLCFS;
class SchedulerSRTF;
class SchedulerRR;
class SchedulerPriority;
class SchedulerPrePriority;

// FUNCTION DECLARATIONS
int myrandom(int burst);
void parseCommand(int argc, char** argv);
void readRandNums();
void readInputProcesses(); 
void Simulation();

// PROCESS CLASS 
class Process {  
    public:
        static int count;
        int pid;  
        int AT;
        int TC;  
        int CB; 
        int IO; 
        int static_prio; 
        int dynamic_prio; 
        int ft; 
        int tt; 
        int it; 
        int cw;
        int cb_rem;  
        state p_state; 
        int state_ts; 
        int premflag; 
        int remaining_burst; 
        int cb; 
        int ib; 
        int finishFlag; 
        int timeInPrevState;
        
        // constructor
        Process() {
            //cout << "Process created!\n";
            this->pid = count; 
            this->p_state = CREATED;
            //this->cb_rem = this->CB;
            //this->state_ts = this->AT;  
            count++;
        }
        // setter functions 
        void setStaticPrio(int static_prio) {
            this->static_prio = static_prio; 
        }
        void setDynamicPrio(int dynamic_prio) {
            this->dynamic_prio = dynamic_prio; 
        }
        void setState(state p_state) {
            this->p_state = p_state; 
        }
        void setParams(int AT, int TC, int CB, int IO) {
            this->AT = AT;
            this->CB = CB; 
            this->TC = TC; 
            this->IO = IO; 
        }
        void setDerivedParams(int ft, int tt, int it, int cw) {
            this->ft = ft;
            this->tt = tt; 
            this->it = it; 
            this->cw = cw; 
        }
        // getter functions 
        int getAT() {
            return this->AT;  
        }
        int getTC() {
            return this->TC;  
        }
        int getCB() {
            return this->CB;  
        }
        int getIO() {
            return this->IO;  
        }
        int getStaticPrio() {
            return this->static_prio; 
        }
        int getDynamicPrio() {
            return this->dynamic_prio;
        }
        state getState() {
            return this->p_state; 
        }

};

// EVENT CLASS
class Event { 
    public:
        static int ecount; 
        int eid;  
        int evtTimeStamp; 
        Process *evtProcess; 
        trans transition;

        //void Event(int ts, Process* p, int t, state old_state, state new_state) {
               // this->evtTimeStamp = ts;
               // this->transition = t; 
              //  this->old_state = old_state;
              //  this->new_state = new_state; 
             //   this->evtProcess = p; 
        //}

        Event() {
            //cout << "Event created!\n"; 
            this->eid = ecount;
            //this->transition = 0;
            //this->evtTimeStamp = this->evtProcess->AT;
            ecount++;
        }
};

struct  cmp {
    bool operator()(const Event* e, const Event* f) {
        if(e->evtTimeStamp == f->evtTimeStamp) {
            return e->eid < f->eid;
        }
        return e->evtTimeStamp < f->evtTimeStamp;
    }
};

class DESLayer {
     public: 
         static set<Event*, cmp> EventQueue; 

     public:

        static void put_event(Event *e) {
            //cout << "Entered put event function\n";
            EventQueue.insert(e);
        }

        static Event* get_event() {
            //cout << "Entered get event function\n";
            if(!EventQueue.empty()) {
                Event *e = *EventQueue.begin(); 
                return e;
            }
            else {
                return nullptr;
            }
        }

        static void rm_event(Event* e) {
            //cout << "Entered remove event function\n";
            EventQueue.erase(e);
        }

        static int get_next_event_time() {
            if(!EventQueue.empty()) {
                //cout << "get next event time entered!\n";
                Event* e = get_event();
                return e->evtTimeStamp;
                //return 0;
            }
            else {
                return -1; 
            }
        }
        
        DESLayer() {
            //cout << "DESLayer created!\n"; 
        }
}; 

// Base class
class Scheduler {
   public:
      // pure virtual function providing interface framework.
      virtual void add_process(Process* p) = 0;
      virtual Process* get_next_process() = 0;  
   
};
 
// Derived classes
class SchedulerFCFS: public Scheduler {
   public:
      void add_process(Process* p) {
          //cout << "Process added\n";
          RUNQUEUE.push(p);
          //cout << "Success!\n";
      }

      Process* get_next_process() {
          Process* p; 
          if(!RUNQUEUE.empty()) {
              p = RUNQUEUE.front();
              RUNQUEUE.pop();
              return p;
          }
          else {
              return nullptr;
          }

      }
    protected:
      queue<Process*> RUNQUEUE; 
};

class SchedulerLCFS: public Scheduler {
   public:
      void add_process(Process* p) {
          //cout << "Process added\n";
          RUNQUEUE.push_back(p);
          //cout << "Success!\n";
      }

      Process* get_next_process() {
          Process* p; 
          if(!RUNQUEUE.empty()) {
              p = RUNQUEUE.back(); 
              RUNQUEUE.pop_back();
              return p;
          }
          else {
              return nullptr;
          }

      }
    protected:
      deque<Process*> RUNQUEUE; 
};

struct rqcmp {
    bool operator()(const Process* p1, Process* p2) {
        if(p1->cb_rem == p2->cb_rem) {
            return p1->state_ts < p2->state_ts;
        }
        return p1->cb_rem < p2->cb_rem;
    }
};

class SchedulerSRTF: public Scheduler {
   public:
      void add_process(Process* p) {

        if(RUNQUEUE.empty()){
            //cout <<"EMPYTY INSERT \n";
            RUNQUEUE.push_back(p);
            /*for(auto it = RUNQUEUE.begin(); it != RUNQUEUE.end(); ++it) {
                //cout << "RUNQUEUE: "; 
                //cout << (*it)->pid << " " << (*it)->cb_rem << "\n";*/
            return;
            }
        else {
            auto it = RUNQUEUE.begin();
            for(;it!=RUNQUEUE.end() && (*it)->cb_rem <= p->cb_rem;) {
                it++; 
            }
            RUNQUEUE.insert(it, p);
           } 
            /*for(auto it = RUNQUEUE.begin(); it != RUNQUEUE.end(); ++it) {
            cout << "RUNQUEUE: "; 
            cout << (*it)->pid << " " << (*it)->cb_rem << "\n";
            }*/
       }            
          //cout << "Success!\n";

      Process* get_next_process() {
          Process* p; 
          if(!RUNQUEUE.empty()) {
              p = RUNQUEUE.front();
              RUNQUEUE.pop_front();
              return p;
          }
          else {
              return nullptr;
          }

      }

      bool does_preempt() {
          return false; 
      }

      void print_rq() {
         if(!RUNQUEUE.empty()) {
            for(auto it = RUNQUEUE.begin(); it!=RUNQUEUE.end(); ++it) {
                cout << (*it)->pid;
            }
         }
      }

    protected:
      deque<Process*> RUNQUEUE; 
};


class SchedulerRR: public Scheduler {
   public:
      void add_process(Process* p) {
          //cout << "Process added\n";
          RUNQUEUE.push(p);
          //cout << "Success!\n";
      }

      Process* get_next_process() {
          Process* p; 
          if(!RUNQUEUE.empty()) {
              p = RUNQUEUE.front();
              RUNQUEUE.pop();
              return p;
          }
          else {
              return nullptr;
          }

      }

    protected:
      queue<Process*> RUNQUEUE; 
};

class SchedulerPriority: public Scheduler {
   public:
      
      SchedulerPriority(int max) {
        maxprio = max; 
        active_queue.resize(max);
        expired_queue.resize(max);
      }

      void add_process(Process* p) {
          if(p->dynamic_prio == -1) {
            p->dynamic_prio = p->static_prio-1;
            expired_queue[p->dynamic_prio].push_back(p);
          }
          else {
            active_queue[p->dynamic_prio].push_back(p);
          }
      }

      int isEmpty(vector<deque<Process *> > p) {
        for(int i=0; i< maxprio; ++i) {
            if(p[i].empty()) {
                continue; 
            }
            else {
                return 0; 
            }
        }
        return 1; 
      }

      Process* get_next_process() {
        if(isEmpty(active_queue))
        {
            if(!(isEmpty(expired_queue)))
            {
                //cout << "non empty GNP\n";
                if(isEmpty(active_queue)) {
                    active_queue.swap(expired_queue);
                }
            }
        }

                for(int i=maxprio-1; i>=0; i--) {
                    if(!active_queue[i].empty()){
                        Process* p = active_queue[i].front(); 
                        active_queue[i].pop_front();
                        return p; 
                    }
                    else
                        continue;
                }
        return nullptr;
      }

    protected:
      vector<deque<Process*> > active_queue; 
      vector<deque<Process*> > expired_queue; 
      int maxprio; 
};

class SchedulerPrePriority: public Scheduler {
   public:
      
      SchedulerPrePriority(int max) {
        maxprio = max; 
        active_queue.resize(max);
        expired_queue.resize(max);
      }

      void add_process(Process* p) {
          if(p->dynamic_prio == -1) {
            p->dynamic_prio = p->static_prio-1;
            expired_queue[p->dynamic_prio].push_back(p);
          }
          else {
            active_queue[p->dynamic_prio].push_back(p);
          }
      }

      int isEmpty(vector<deque<Process *> > p) {
        for(int i=0; i< maxprio; ++i) {
            if(p[i].empty()) {
                continue; 
            }
            else {
                return 0; 
            }
        }
        return 1; 
      }

      Process* get_next_process() {
        if(isEmpty(active_queue))
        {
            if(!(isEmpty(expired_queue)))
            {
                //cout << "non empty GNP\n";
                if(isEmpty(active_queue)) {
                    active_queue.swap(expired_queue);
                }
            }
        }

                for(int i=maxprio-1; i>=0; i--) {
                    if(!active_queue[i].empty()){
                        Process* p = active_queue[i].front(); 
                        active_queue[i].pop_front();
                        return p; 
                    }
                    else
                        continue;
                }
        return nullptr;
      }

    protected:
      vector<deque<Process*> > active_queue; 
      vector<deque<Process*> > expired_queue; 
      int maxprio; 
};



// GLOBAL VARIABLES 
int vflag = 0;
int tflag = 0;
int eflag = 0; 
int pflag = 0; 
string sOption = "";
char s; 
string inputFile, randFile;
fstream input, rfile;
vector<int> randvals;
int ofs = 0; 
int Process::count = 0; 
int Event::ecount = 0;
int CURRENT_TIME = 0;
Process* CURRENT_RUNNING_PROCESS = nullptr;
bool CALL_SCHEDULER = false;
set <Event*, cmp> DESLayer::EventQueue;
int size = 0; 
int quant = 10000, maxprio = 4; 
vector<Process *> Processes; 
DESLayer* SimulationLayer = new DESLayer();
Scheduler* scheduler = nullptr;
int finish_event_time = 0;
int total_io = 0;



// MAIN FUNCTION
int main(int argc, char** argv) {
    parseCommand(argc, argv); 
    readRandNums();
    string procParams;
    char *token = nullptr;
    int i; 
    int* param = (int *) malloc(sizeof(int)*5);
    
    input.open(inputFile);
    if(input.is_open()) {
        while(true) {
            getline(input, procParams);
            if(input.eof()) {
               break; 
            }
            i = 0;
            char *params = (char *) procParams.c_str(); 
            token = strtok(params, " ");
            while(token!=nullptr) {
                param[i++] = stoi(token);
                token = strtok(nullptr, " ");
            }
            Process* p = new Process();
            Processes.push_back(p);
            Event* e = new Event();
            param[i] = myrandom(maxprio); 
            //cout << "My maxprio: " << maxprio <<endl;
            p->setParams(param[0], param[1], param[2], param[3]);
            p->setStaticPrio(param[4]);
            p->setDynamicPrio(param[4]-1);
            p->cb_rem = p->TC;
            p->state_ts = p->AT;
            p->ft = 0;
            p->tt = 0;
            p->cw = 0;
            p->it = 0;
            p->premflag = 0;
            e->evtProcess = p;
            e->evtTimeStamp = p->AT;
            e->transition = TRANS_TO_READY; 
            SimulationLayer->put_event(e);
            // event transition??????
            //cout << Process::count << " " <<   Event::ecount; 
        }
    }
    input.close();
    /*
    for(auto it = SimulationLayer.EventQueue.begin(); it!=SimulationLayer.EventQueue.end(); ++it) {
        cout << (*it)->evtProcess->pid << " " << (*it)->evtProcess->static_prio << endl; 
    }
    */

	if (s=='F') {
		scheduler = new SchedulerFCFS();
	}
	else if (s == 'L') {
		scheduler = new SchedulerLCFS();
	}
	else if (s == 'S') {
		scheduler = new SchedulerSRTF();
	}
	else if (s =='R') {
		scheduler = new SchedulerRR();
	}
	else if (s =='P') {
		scheduler = new SchedulerPriority(maxprio);
	}
    else if (s =='E') {
		scheduler = new SchedulerPrePriority(maxprio);
	}

    Simulation();
    //cout << *(SimulationLayer.EventQueue.begin());
    int total_tt = 0, total_cw = 0, total_tc = 0;

    if (s=='F') {
		cout<<"FCFS\n";
	}
	else if (s == 'L') {
		cout<<"LCFS\n";
	}
	else if (s == 'S') {
		cout<<"SRTF\n";
	}
	else if (s =='R') {
		cout<<"RR " <<quant<<endl;
	}
	else if (s =='P') {
		cout<<"PRIO "<<quant<<endl;
	}
    else if (s =='E') {
		cout<<"PREPRIO "<<quant<<endl;
	} 

    for(auto it= Processes.begin(); it!= Processes.end(); ++it) {
        Process *p = *it; 
        printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n",
                p->pid, p->AT, p->TC, p->CB, p->IO, p->static_prio, p->ft, p->tt, p->it, p->cw);
        
        total_tt = total_tt + p->ft - p->AT;
        total_tc = total_tc + p->TC;
        total_cw = total_cw + p->cw;
    }
    double cpu_util = (double)(total_tc*100.0/finish_event_time);
    double io_util = (double)(total_io*100.0/finish_event_time);
    double avg_tt = (double)(total_tt*1.0/Processes.size());
    double avg_cw = (double)(total_cw*1.0/Processes.size());
    double throughput = (double)(Processes.size()*100.0/finish_event_time);

    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n", finish_event_time, cpu_util, io_util, avg_tt, avg_cw, throughput);
    randvals.clear();
 }

void parseCommand(int argc, char** argv) { 
    int c;  
    while((c = getopt(argc, argv, "vtpes:")) != -1) {
        switch(c) {
            case 'v':
                    vflag = 1;
                    break;
            case 't':
                    tflag = 1;
                    break; 
            case 'p':
                    pflag = 1;
                    break;
            case 'e':
                    eflag = 1;
                    break;
            case 's': 
                    sOption = optarg;
                    //cout << "Optarg: " << sOption <<endl;
                    /*if(s == 'E' || s == 'R' || s == 'P') {
                        size_t pos = sOption.find(":", 1);
                        quant = stoi(sOption.substr(1, pos));
                        if(s == 'E' && sOption[pos+1] != '\0') {
                            maxprio = stoi(sOption.substr(pos+1, sOption.length()-1));
                        }
                        else if(s == 'P' && sOption[pos+1] != '\0') {
                            maxprio = stoi(sOption.substr(pos+1, sOption.length()-1));
                        }
                    }*/
                    break;
        }
    }
    
    sscanf(sOption.c_str(), "%c%d:%d", &s, &quant, &maxprio);

    inputFile = argv[optind];
    randFile = argv[optind+1];

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


int myrandom(int burst) { 
    if(ofs == size) {
        ofs = 0;
    }
    return 1 + (randvals[ofs++] % burst);
}

void Simulation() {
         Event* evt;
         int processes_blocked = 0, block_time = 0;
         while( (evt = SimulationLayer->get_event()) ) {
                /*cout << "==========" << endl;
                cout << "Event Queue State:\n";

                for(auto it = SimulationLayer->EventQueue.begin(); it!=SimulationLayer->EventQueue.end(); ++it) {
                    cout << (*it)->evtProcess->pid << " " << (*it)->evtTimeStamp << endl;
                }*/
                 Event* new_event;
                 Process *proc = evt->evtProcess;  
                 CURRENT_TIME = evt->evtTimeStamp;
                 trans transition = evt->transition;
                 proc->timeInPrevState = CURRENT_TIME-proc->state_ts;
                 proc->state_ts = evt->evtTimeStamp;
                 //cout << "---------"<< endl; 
                 //cout << "current event: " << evt->evtProcess->pid << " " << evt->evtTimeStamp << " " << evt->transition << endl; 
                 //cout << "---------"<< endl; 

                 SimulationLayer->rm_event(evt); 
                 delete evt;                  
                 evt = nullptr;

                 //cout << CURRENT_TIME << " " << proc->pid << " " << proc->timeInPrevState << ": ";
                 //cout << proc->p_state << " " << transition; 
                 //cout << "\n";

                 switch(transition) {  
                    case TRANS_TO_READY: 
                        if(CURRENT_RUNNING_PROCESS != nullptr && s == 'E') {
                            if(proc->dynamic_prio > CURRENT_RUNNING_PROCESS->dynamic_prio) {
                                for(auto it = SimulationLayer->EventQueue.begin(); it != SimulationLayer->EventQueue.end(); ++it) {
                                    if((*it)->evtProcess->pid == CURRENT_RUNNING_PROCESS->pid && (*it)->evtTimeStamp != CURRENT_TIME) {
                                        CURRENT_RUNNING_PROCESS->remaining_burst = CURRENT_RUNNING_PROCESS->remaining_burst + (*it)->evtTimeStamp - CURRENT_TIME;
                                        SimulationLayer->rm_event(*it); 
                                        Event* preprio = new Event(); 
                                        preprio->evtProcess = CURRENT_RUNNING_PROCESS; 
                                        preprio->evtTimeStamp = CURRENT_TIME; 
                                        preprio->transition = TRANS_TO_PREEMPT; 
                                        SimulationLayer->put_event(preprio); 
                                        CURRENT_RUNNING_PROCESS = nullptr; 
                                        break; 

                                    }
                                     
                                }
                            }
                        }

                        if(proc->p_state == BLOCKED) {
                        //proc->dynamic_prio = proc->static_prio-1;
                            proc->it = proc->it + proc->timeInPrevState;
                            processes_blocked--;
                            if(processes_blocked==0)
                                total_io = total_io+ CURRENT_TIME - block_time;
                        }
                        scheduler->add_process(proc);
                        CALL_SCHEDULER = true;
                        proc->p_state = READY;
                        break;

                    case TRANS_TO_PREEMPT: 
                 // similar to TRANS_TO_READY

                        CURRENT_RUNNING_PROCESS = nullptr;
                        proc->dynamic_prio = proc->dynamic_prio-1;
                        scheduler->add_process(proc);
                        proc->p_state = READY;
                        CALL_SCHEDULER = true;
                        proc->cb_rem = proc->cb_rem - proc->timeInPrevState;
                        proc->cb = proc->cb - min(quant, proc->timeInPrevState);

                        
                        /*cout << " cb= " << proc->cb << " "; 
                        cout << "rem= " << proc->cb_rem << " ";
                        cout << "prio= " << proc->dynamic_prio << " "; 
                        cout << "quant= " << quant; 
                        cout << "\n";*/
                        break;

                    case TRANS_TO_RUN:
                         CURRENT_RUNNING_PROCESS = proc;
                         proc->cw = proc->cw + proc->timeInPrevState;

                        if(proc->remaining_burst == 0)
                            proc->cb = myrandom(proc->CB);
                        else
                            proc->cb = proc->remaining_burst;
                        
                        proc->cb = min(proc->cb_rem, proc->cb);
                        
                        //cout << " cb= " << proc->cb << " "; 
                        //cout << "rem= " << proc->cb_rem << " ";
                        //cout << "prio= " << proc->dynamic_prio; 
                        //cout << "quant= " << quant; 
                        //cout << "\n";

                        if(proc->cb > quant) {
                            proc->remaining_burst = proc->cb - quant; 
                            new_event = new Event();
                            new_event->evtTimeStamp = CURRENT_TIME + quant;
                            new_event->transition = TRANS_TO_PREEMPT; 

                        }
                        else
                        {
                            if(proc->cb == proc->cb_rem) {
                                new_event = new Event(); 
                                new_event->evtTimeStamp = CURRENT_TIME + proc->cb_rem;
                                new_event->transition = TRANS_TO_COMPLETE;  
    
                            }
                            else 
                            {
                                new_event = new Event(); 
                                new_event->evtTimeStamp = CURRENT_TIME + proc->cb;
                                new_event->transition = TRANS_TO_BLOCK;
                                proc->remaining_burst = 0;                           
                            }
                        }
                        proc->p_state = RUNNING;
                        new_event->evtProcess = proc;
                        SimulationLayer->put_event(new_event);
                        break;

                    case TRANS_TO_BLOCK:
                        processes_blocked = processes_blocked + 1; 
                        if(processes_blocked == 1) {
                            block_time = CURRENT_TIME;
                        }
                        proc->cb_rem = proc->cb_rem - proc->timeInPrevState;
                        new_event = new Event();                 
                        new_event->evtProcess = proc;
                        //proc->cb -= proc->timeInPrevState;
                        proc->ib = myrandom(proc->IO);   
                        //cout << " ib= " << proc->ib << " "; 
                        //cout << "rem= " << proc->cb_rem << " ";
                        //cout << "\n"; 
                        new_event->transition = TRANS_TO_READY;
                        new_event->evtTimeStamp = CURRENT_TIME + proc->ib;
                        proc->p_state = BLOCKED;    
                        SimulationLayer->put_event(new_event);
                        proc->dynamic_prio = proc->static_prio-1;
                        CURRENT_RUNNING_PROCESS = nullptr;
                        CALL_SCHEDULER = true;
                        break;

                    case TRANS_TO_COMPLETE: 
                        proc->ft = CURRENT_TIME; 
                        proc->tt = proc->ft - proc->AT;
                        proc->p_state = COMPLETE;
                        //cout << "Process " << proc->pid << " finished!\n"; 
                        //cout << proc->pid << " "; 
                        //cout << proc->ft << " " << proc->tt << " " << proc->it << " " << proc->cw << "\n";
                        CALL_SCHEDULER = true; 
                        CURRENT_RUNNING_PROCESS = nullptr;
                        finish_event_time = CURRENT_TIME;
                        break;
                }
                 if(CALL_SCHEDULER) {
                         if (SimulationLayer->get_next_event_time() == CURRENT_TIME) {
                                continue; //process next event from Event queue       
                         }
                         CALL_SCHEDULER = false; // reset global flag
                         if (CURRENT_RUNNING_PROCESS == nullptr) {
                            CURRENT_RUNNING_PROCESS = scheduler->get_next_process();
                                   if (CURRENT_RUNNING_PROCESS == nullptr) {  
                                       continue;
                                   }
                                   //cout << "\njust starting "<<CURRENT_RUNNING_PROCESS->pid<<endl; 
                                    new_event = new Event(); 
                                    //proc->p_state = READY;
                                    new_event->evtTimeStamp = CURRENT_TIME;
                                    new_event->evtProcess = CURRENT_RUNNING_PROCESS; 
                                    new_event->transition = TRANS_TO_RUN;
                                    proc->state_ts = CURRENT_TIME;
                                    SimulationLayer->put_event(new_event); 
                                   // Create event to make this process runnable for same time  
                            }
                 }
                 new_event = nullptr;
         }
}

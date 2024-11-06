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

using namespace std;

string inputFile; 
fstream input; 
char algo='\0'; 
int vflag = 0, qflag = 0, fflag = 0; 
string line = "";
int CURRENT_TIME = 0, CURRENT_HEAD = 0, PREV_HEAD = -1;
int TOTAL_MOVEMENT = 0, TOTAL_TIME = 0, max_waittime = 0; 
double avg_turnaround = 0.0F, avg_waittime = 0.0F; 
int completed_requests = 0; 
enum DIRECTION {UP, DOWN, NO_MOVE};

class Request; 
class Strategy; 

void setOptionsFlag(string options); 

void setOptionsFlag(string options) {
    if (options.find('v') != string::npos) {
        vflag = 1;
    }
    if(options.find('q') != string::npos) {
        qflag = 1; 
    }
    if(options.find('f') != string::npos) {
        fflag = 1;
    }
    return ; 
}

class Request {
    public:
        static int req_count; 
        int req_id; 
        int head_seek; 
        int arrival_time; 
        int disk_start_time; 
        int disk_end_time; 
        int turnaround_time; 
        int wait_time; 
        int added; 

    Request() {
        added = 0;
        req_id = req_count; 
        req_count++;
    }

    void print_requests() {
        printf("%5d: %5d %5d %5d\n", req_id, arrival_time, disk_start_time, disk_end_time);
    }
};

int Request::req_count = 0; 
vector<Request* > Requests;
Request* current_io = nullptr; 

class Strategy {
    public:
        virtual Request* next_request() = 0;
        virtual void add_request(Request*) = 0;
        virtual void algo_name() = 0;
        deque <Request*> IOQueue;  
};

class FIFO: public Strategy {
    public:
        Request* next_request() {
            if(IOQueue.empty()) {
                return nullptr;
            }
            Request* next = IOQueue.front();
            IOQueue.pop_front();
            return next;
        }

        void add_request(Request* req) {
            IOQueue.push_back(req);
            //print_io_queue();
        }
        void algo_name() {
            cout << "FIFO Algorithm\n";
        }

        void print_io_queue() {
            for(auto it = IOQueue.begin(); it != IOQueue.end(); ++it) {
                if(IOQueue.empty()) {
                    "queue is empty!\n";
                }
                cout << "Queue request id: " << (*it)->req_id << endl;
            }
        }
        
        deque <Request*> IOQueue;  
};

class SSTF: public Strategy {
    public:
        Request* next_request() {
            //cout << "Entered next request!\n";

            Request* next = nullptr; 

            if(IOQueue.empty()) {
                return next;
            }

            //print_io_queue();

            if(IOQueue.size() == 1) {
                next = IOQueue.front();
                IOQueue.pop_front();
                return next;
            }

            int min_seek = abs(IOQueue.front()->head_seek - PREV_HEAD);
            next = IOQueue.front();
            auto minItr = IOQueue.begin(); 

            //cout << min_seek << endl;
            auto it = IOQueue.begin()+1;
            
            for(; it != IOQueue.end(); ++it) {
                int mov = abs((*it)->head_seek - PREV_HEAD); 
                if(mov < min_seek) {
                    //cout << min_seek << " < " << mov << "\n";
                    min_seek = mov;
                    next = *it; 
                    minItr = it; 
                }
            }
            //cout << "selected request: " << next->req_id <<endl;
            IOQueue.erase(minItr); 
            //print_io_queue();
            return next; 
        }

        void add_request(Request* req) {
            IOQueue.push_back(req);
            //print_io_queue();
        }
        void algo_name() {
            cout << "SSTF Algorithm\n";
        }

        void print_io_queue() {
            for(auto it = IOQueue.begin(); it != IOQueue.end(); ++it) {
                if(IOQueue.empty()) {
                    "queue is empty!\n";
                }
                cout << "IO Queue " << (*it)->req_id << " ";
            }
            cout << "\n";
        }
        
        deque <Request*> IOQueue;  
};

class LOOK: public Strategy {
    public:
        Request* next_request() {

            Request* next = nullptr; 
            Request *nextUP = nullptr; 
            Request *nextDOWN = nullptr; 

            if(IOQueue.empty()) {
                return next;
            }

            //print_io_queue();

            if(IOQueue.size() == 1) {
                next = IOQueue.front();
                IOQueue.pop_front();
                if(PREV_HEAD < next->head_seek) {
                    direction = UP; 
                }
                else if(PREV_HEAD > next->head_seek) {
                    direction = DOWN; 
                }
                return next;
            }

            //cout << "Entered next request!\n"; 

            
            int min_head = -1, max_head = -1;
            deque<Request *>::iterator minItr; 
            deque<Request *>::iterator maxItr; 

            if(IOQueue.front()->head_seek > PREV_HEAD) {
                //cout << "Set MIN head to: " << IOQueue.front()->head_seek << endl;
                min_head = IOQueue.front()->head_seek; 
                nextUP = IOQueue.front(); 
                minItr = IOQueue.begin();
            }

            else if(IOQueue.front()->head_seek < PREV_HEAD) {
                //cout << "Set MAX head to: " << IOQueue.front()->head_seek << endl;
                max_head = IOQueue.front()->head_seek; 
                nextDOWN = IOQueue.front(); 
                maxItr = IOQueue.begin();
            }

            else if(IOQueue.front()->head_seek == PREV_HEAD) {
                    if(direction == UP) {
                            min_head = IOQueue.front()->head_seek;
                            nextUP = IOQueue.front(); 
                            minItr = IOQueue.begin();
                    }
                    else if(direction == DOWN) {
                            max_head = IOQueue.front()->head_seek;
                            nextDOWN = IOQueue.front(); 
                            maxItr = IOQueue.begin();
                    }
            }
            //cout << "direction is: " << direction << endl;

            auto it = IOQueue.begin()+1;
            for(;it!=IOQueue.end(); ++it) {

                    if((*it)->head_seek > PREV_HEAD) {
                        if((min_head == -1) || ((*it)->head_seek < min_head)) {
                            min_head = (*it)->head_seek;
                            nextUP = *it; 
                            minItr = it; 
                        }
                    }
                    else if((*it)->head_seek < PREV_HEAD){
                        if((max_head == -1) || ((*it)->head_seek > max_head)) {
                            max_head = (*it)->head_seek;
                            nextDOWN = *it; 
                            maxItr = it; 
                        }
                    }
                    else if((*it)->head_seek == PREV_HEAD) {
                        if(direction == UP) {
                            min_head = (*it)->head_seek;
                            nextUP = *it; 
                            minItr = it;
                        }
                        else if(direction == DOWN) {
                            max_head = (*it)->head_seek;
                            nextDOWN = *it; 
                            maxItr = it;
                        }
                    }
            }

            if(direction == UP && nextUP != nullptr) {
                //cout << "flow 1\n";
                next = nextUP; 
                IOQueue.erase(minItr);
            }
            else if(direction == UP && nextUP == nullptr) { 
                //cout << "flow 2\n";
                next = nextDOWN; 
                IOQueue.erase(maxItr);
                direction = DOWN;
            }

            else if(direction == DOWN && nextDOWN != nullptr) {
                //cout << "flow 3\n";
                next = nextDOWN;
                IOQueue.erase(maxItr);
            }
            else if(direction == DOWN && nextDOWN == nullptr) {
                //cout << "flow 4\n";
                next = nextUP; 
                IOQueue.erase(minItr);
                direction = UP; 
            }
            //cout << "direction is: " << direction << endl;
            //cout << "selected request: " << next->req_id << " " << next->head_seek <<endl;
            return next; 
        }

        void add_request(Request* req) {
            IOQueue.push_back(req);
            //cout << "Adding!\n";
            //print_io_queue();
        }
        void algo_name() {
            cout << "LOOK Algorithm\n";
        }

        void print_io_queue() {
            for(auto it = IOQueue.begin(); it != IOQueue.end(); ++it) {
                if(IOQueue.empty()) {
                    "queue is empty!\n";
                }
                cout << "Queue request id: " << (*it)->req_id << " " << (*it)->head_seek << endl;
            }
        }

        LOOK() {
            direction = UP;
        }
        
        deque <Request*> IOQueue;  
        DIRECTION direction; 
};

class CLOOK: public Strategy {
    public:
        Request* next_request() {

            Request *next = nullptr;
            Request *nextW = nullptr;

            if(IOQueue.empty()) {
                return next;
            }

            //print_io_queue();

            if(IOQueue.size() == 1) {
                next = IOQueue.front();
                IOQueue.pop_front();
                return next;
            }
            
            int min_seek = -1, wrap_seek = -1;
            auto itr = IOQueue.begin(); 
            deque<Request *>::iterator minItr; 
            deque<Request *>::iterator wrapItr;

            for(; itr != IOQueue.end(); ++itr) {
                if((*itr)->head_seek >= PREV_HEAD) {
                    if((min_seek == -1) || ((*itr)->head_seek < min_seek)) {
                        min_seek = (*itr)->head_seek;
                        next = *itr; 
                        minItr = itr; 
                    }
                }
                else if((*itr)->head_seek < PREV_HEAD) {
                    if((wrap_seek == -1) || ((*itr)->head_seek < wrap_seek)) {
                        wrap_seek = (*itr)->head_seek;
                        nextW = *itr; 
                        wrapItr = itr;
                    }
                }
            }

            if(next != nullptr) {
                //cout << "flow 1\n"; 
                IOQueue.erase(minItr);
            }

            else if(next == nullptr) {
                //cout << "flow 2\n";
                next = nextW; 
                IOQueue.erase(wrapItr);
            }

            //cout << "selected request: " << next->req_id << " " << next->head_seek <<endl;

            return next; 

            
        }

        void add_request(Request* req) {
            IOQueue.push_back(req);
            //cout << "Adding!\n";
            //print_io_queue();
        }
        void algo_name() {
            cout << "CLOOK Algorithm\n";
        }

        void print_io_queue() {
            for(auto it = IOQueue.begin(); it != IOQueue.end(); ++it) {
                if(IOQueue.empty()) {
                    "queue is empty!\n";
                }
                cout << "Queue request id: " << (*it)->req_id << " " << (*it)->head_seek << endl;
            }
        }

        CLOOK() {
            direction = UP;
        }
        
        deque <Request*> IOQueue;  
        DIRECTION direction; 
};

class FLOOK: public Strategy {
    public:
        Request* next_request() {

            Request* next = nullptr; 
            Request *nextUP = nullptr; 
            Request *nextDOWN = nullptr; 

            if(IOQueue.empty()) {
                if(!add_queue.empty())
                    add_queue.swap(IOQueue);
                else 
                    return next;
            }

            //print_io_queue();

            if(IOQueue.size() == 1) {
                next = IOQueue.front();
                IOQueue.pop_front();
                if(PREV_HEAD < next->head_seek) {
                    direction = UP; 
                }
                else if(PREV_HEAD > next->head_seek) {
                    direction = DOWN; 
                }
                return next;
            }

            //cout << "Entered next request!\n"; 

            
            int min_head = -1, max_head = -1;
            deque<Request *>::iterator minItr; 
            deque<Request *>::iterator maxItr; 

            if(IOQueue.front()->head_seek > PREV_HEAD) {
                //cout << "Set MIN head to: " << IOQueue.front()->head_seek << endl;
                min_head = IOQueue.front()->head_seek; 
                nextUP = IOQueue.front(); 
                minItr = IOQueue.begin();
            }

            else if(IOQueue.front()->head_seek < PREV_HEAD) {
                //cout << "Set MAX head to: " << IOQueue.front()->head_seek << endl;
                max_head = IOQueue.front()->head_seek; 
                nextDOWN = IOQueue.front(); 
                maxItr = IOQueue.begin();
            }

            else if(IOQueue.front()->head_seek == PREV_HEAD) {
                    if(direction == UP) {
                            min_head = IOQueue.front()->head_seek;
                            nextUP = IOQueue.front(); 
                            minItr = IOQueue.begin();
                    }
                    else if(direction == DOWN) {
                            max_head = IOQueue.front()->head_seek;
                            nextDOWN = IOQueue.front(); 
                            maxItr = IOQueue.begin();
                    }
            }
            //cout << "direction is: " << direction << endl;

            auto it = IOQueue.begin()+1;
            for(;it!=IOQueue.end(); ++it) {

                    if((*it)->head_seek > PREV_HEAD) {
                        if((min_head == -1) || ((*it)->head_seek < min_head)) {
                            min_head = (*it)->head_seek;
                            nextUP = *it; 
                            minItr = it; 
                        }
                    }
                    else if((*it)->head_seek < PREV_HEAD){
                        if((max_head == -1) || ((*it)->head_seek > max_head)) {
                            max_head = (*it)->head_seek;
                            nextDOWN = *it; 
                            maxItr = it; 
                        }
                    }
                    else if((*it)->head_seek == PREV_HEAD) {
                        if(direction == UP) {
                            min_head = (*it)->head_seek;
                            nextUP = *it; 
                            minItr = it;
                        }
                        else if(direction == DOWN) {
                            max_head = (*it)->head_seek;
                            nextDOWN = *it; 
                            maxItr = it;
                        }
                    }
            }

            if(direction == UP && nextUP != nullptr) {
                //cout << "flow 1\n";
                next = nextUP; 
                IOQueue.erase(minItr);
            }
            else if(direction == UP && nextUP == nullptr) { 
                //cout << "flow 2\n";
                next = nextDOWN; 
                IOQueue.erase(maxItr);
                direction = DOWN;
            }

            else if(direction == DOWN && nextDOWN != nullptr) {
                //cout << "flow 3\n";
                next = nextDOWN;
                IOQueue.erase(maxItr);
            }
            else if(direction == DOWN && nextDOWN == nullptr) {
                //cout << "flow 4\n";
                next = nextUP; 
                IOQueue.erase(minItr);
                direction = UP; 
            }
            //cout << "direction is: " << direction << endl;
            //cout << "selected request: " << next->req_id << " " << next->head_seek <<endl;
            return next; 
        }

        void add_request(Request* req) {
            add_queue.push_back(req);
            //cout << "Adding!\n";
            //print_io_queue();
        }
        void algo_name() {
            cout << "FLOOK Algorithm\n";
        }

        void print_io_queue() {
            for(auto it = IOQueue.begin(); it != IOQueue.end(); ++it) {
                if(IOQueue.empty()) {
                    "queue is empty!\n";
                }
                cout << "Queue request id: " << (*it)->req_id << " " << (*it)->head_seek << endl;
            }
        }

        FLOOK() {
            direction = UP;
        }
        
        deque <Request*> add_queue;  
        deque <Request*> IOQueue; 
        DIRECTION direction; 
};

int main(int argc, char** argv) {
    string frames="", algos="", options=""; 
    char opt_f, opt_a; 
    int c; 
    Strategy* THE_STRATEGY; 
    DIRECTION d = UP;

    while((c = getopt(argc, argv, "s:vqf")) != -1) {
        switch(c) {
            case 's':   
                algos = optarg; 
                sscanf(algos.c_str(), "%c", &algo);
                break; 
            case 'q':
                options = optarg; 
                setOptionsFlag(optarg);
                break; 
            case 'v':
                options = optarg; 
                setOptionsFlag(optarg);
                break; 
            case 'f':
                options = optarg; 
                setOptionsFlag(optarg);
                break; 
            case '?': 
                cout << "Unexpected!\n";
                break;
        }
        inputFile = argv[optind];
    }

    int parse_flag = 0, num_of_ios = 0; 
    input.open(inputFile);
    char* str = nullptr;
    char* token = nullptr;
    Request* r; 

    if(input.is_open()) {
        while(true) {
            getline(input, line);
            if(input.eof()) {
               break; 
            }   

            if(line[0] == '#') {
                continue; 
            }

            if(line[0] != '#') {
                str = (char*) line.c_str();
                while(!line.empty()) {
                    for(int i = 0; i < 1; ++i) {
                        token = strtok(str, " ");
                        r = new Request(); 
                        r->arrival_time = stoi(token);
                        token = strtok(NULL, " ");
                        r->head_seek = stoi(token);
                        Requests.push_back(r);
                        r = nullptr;
                    }
                    getline(input, line);
                }
            }
        }
    }

    if(algo == 'i') {
        THE_STRATEGY = new FIFO(); 
    }
    else if(algo == 'j') {
        THE_STRATEGY = new SSTF(); 
    }
    else if(algo == 's'){
        THE_STRATEGY = new LOOK(); 
    }
    else if(algo == 'c'){
        THE_STRATEGY = new CLOOK(); 
    }
    else if(algo == 'f') {
        THE_STRATEGY = new FLOOK(); 
    }


    //cout << "Starting simulation: \n";

    while(true) {

        //cout << "TIME IS : " << CURRENT_TIME << "s" << endl; 
        //if(current_io != nullptr)
            //cout << "current io is: " << current_io->req_id << endl; 
        //cout << "HEAD IS : " << CURRENT_HEAD << endl;
        
        for(auto it = Requests.begin(); it!= Requests.end(); ++it) {
            //cout << "entered flow #1\n"; 
            //cout << "arrival time " << (*it)->req_id << " is: " << (*it)->arrival_time <<endl;
            if((*it)->arrival_time == CURRENT_TIME && (*it)->added != 1) {
                //cout << "arrived!\n";
                THE_STRATEGY->add_request(*it); 
                (*it)->added = 1; 
                break;
            }
            
        }

        if((current_io != nullptr) && (current_io->head_seek == CURRENT_HEAD)) {
            //cout << "entered flow #2\n";
            current_io->disk_end_time = CURRENT_TIME;
            current_io->turnaround_time = current_io->disk_end_time - current_io->arrival_time;
            current_io->wait_time = current_io->disk_start_time - current_io->arrival_time;
            completed_requests++;
            PREV_HEAD = current_io->head_seek;
            //cout << "COMPLETED REQUESTS: " << current_io->req_id << endl;
            current_io = nullptr;   
        }

        if(current_io == nullptr) {
            //cout << "entered flow #3\n";
            current_io = THE_STRATEGY->next_request();
            if(current_io == nullptr) {
                //cout << "empty queue!\n";
            }
            else {
                //cout << "Head moves from " << CURRENT_HEAD << " "; 
                //cout <<  "to " << current_io->head_seek; 
                current_io->disk_start_time = CURRENT_TIME;
                if(current_io->head_seek < PREV_HEAD) {
                    //cout << " down"; 
                    d = DOWN;
                }

                else if(current_io->head_seek > PREV_HEAD) {
                    //cout << " up"; 
                    d = UP;
                }
                else if(current_io->head_seek == PREV_HEAD) {
                    d = NO_MOVE;
                    //cout << " or not at all.."; 
                }
                //cout << "\n";
            }

            if(completed_requests == Requests.size()) {
                //cout << "Exit simulation\n";
                break;
            }
        }

        if(current_io != nullptr) {
        //cout << "entered flow #4\n";
            if(d == UP) {
                CURRENT_HEAD = CURRENT_HEAD + 1;
                TOTAL_MOVEMENT++;
            }
            else if(d == DOWN) {
                CURRENT_HEAD = CURRENT_HEAD - 1; 
                TOTAL_MOVEMENT++;
            }         
        }
        if(d != NO_MOVE)
            CURRENT_TIME = CURRENT_TIME + 1; 
    }

    for(auto it = Requests.begin(); it!= Requests.end(); ++it) {
        (*it)->print_requests();
        avg_turnaround = avg_turnaround + (*it)->turnaround_time;
        avg_waittime = avg_waittime + (*it)->wait_time;
        if((*it)->wait_time > max_waittime) {
            max_waittime = (*it)->wait_time;
        }
    }

    num_of_ios = (int) Requests.size(); 
    avg_turnaround = avg_turnaround/num_of_ios;
    avg_waittime = avg_waittime/num_of_ios;

    printf("SUM: %d %d %.2lf %.2lf %d\n", CURRENT_TIME, TOTAL_MOVEMENT, avg_turnaround, avg_waittime, max_waittime); 
    return 0;
}
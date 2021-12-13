#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>

//Queue implementation
struct Node{
    int tid;
    int size;
    int* si;
    struct Node* next;
};

struct Queue {
    struct Node *front, *rear;
};

struct Node* create_node(int tid, int size, int* si){
    struct Node* temp = (struct Node*) malloc(sizeof(struct Node));
    temp->tid = tid;
    temp->size = size;
    temp->si = si;
    temp->next = NULL;
    return temp;
}

void display(struct Queue* q){
    struct Node* temp = q->front;
    printf("Queue: ");
    while(temp!=NULL){
        printf("%d ", temp->tid);
        temp = temp->next;
    }
    printf("\n");
}

void enqueue(struct Queue* q, int tid, int size, int* si){
    struct Node* new_node = create_node(tid, size, si);
    if (q->rear == NULL){
        q->front = new_node;
        q->rear = new_node;
        return;
    }
    q->rear->next = new_node;
    q->rear = new_node;
}

void dequeue(struct Queue* q){
    if (q->front == NULL) return;
    struct Node* delete_node = q->front;
    q->front = q->front->next;
    if (q->front == NULL) q->rear = NULL;
    free(delete_node);
}

struct Queue* create_Queue(){
    struct Queue* q = (struct Queue*) malloc(sizeof(struct Queue));
    q->front = NULL;
    q->rear = NULL;
    return q;
}
// Queue implementation ends

//global variables
time_t st;
struct thread_data{
    int tid;
    int size;
    int duration;
};
int memory_size;
int np = 0;
struct Queue* q;
int* mem;
int tc;
pthread_mutex_t lock;
int T;
int process_number;
int process_time;
int fit;
//global variables end

int floor2(float value){
    int res = (int) value;
    return res;
}

int ceil2(float value){
    int temp = (int) value;
    float temp2 = temp;
    if (value == temp2) return temp;
    else return temp+1; 
}

float memory_utilization(int *arr, int size){
    int ch = 0;
    pthread_mutex_lock(&lock);
    for(int i=0;i<size;i++)
        if (arr[i] == 1) ch++;
    pthread_mutex_unlock(&lock);
    return ((float)ch)/size;
}

int memory_allocation(int size, int pa, int* s_i){
    //first_fit
    if (pa == 1){
        //printf("First fit starts\n");
        int eb = 0; //numer of 0 bytes after most recent occupied byte
        int si = -1; //start index of memory allocation
        bool ch = true; //boolean vaiable to set start index appropriately
        bool is_available = false;
        for(int i=0;i<memory_size;i++){ //access the memory
            if (mem[i] == 0){   //if available memory allocation
                eb++;
                if (ch == true){    //if just after an occupied location
                    si = i;     //set start index of this block
                    ch = false;
                }
                if (eb == size){    //if a block of size >= required size found
                    is_available = true;
                    for(int j=si;j<si+size;j++){    //allocate this block to process
                        mem[j] = 1;     
                    }
                    break;
                }

            }
            else{       // if occupied memory location
                ch = true;  
                eb = 0;
            }
        }
        //printf("First fit ends\n");
        if (is_available == false) si = -1;
        else *s_i = si;
        return si;
    }
    //best fit
    if (pa == 2){
        int eb = 0;
        int si = -1;
        int ans = 0;
        int min = INT_MAX;
        bool ch = true;
        bool is_available = false;
        for(int i=0;i<memory_size;i++){
            if (mem[i] == 0){
                eb++;
                if (ch == true){
                    si = i;
                    ch = false;
                }
            }
            else{
                ch = true;
                if (eb - size >= 0 && eb - size < min){
                    is_available = true;
                    min = eb-size;
                    ans = si;
                }
                eb = 0;
            }
        }
        if (eb - size >= 0 && eb - size < min){
            is_available = true;
            min = eb-size;
            ans = si;
        }
        if (is_available == false) ans = -1;
        else {
            *s_i = ans;
            for(int i=ans;i<ans+size;i++){
                mem[i] = 1;
            }
        }
        return ans;
    }
    //next fit
    if (pa == 3){
        int eb = 0;
        int si = -1;
        bool ch = true;
        bool is_available = false;
        for(int i=np;i<memory_size;i++){
            if (mem[i] == 0){
                eb++;
                if (ch == true){
                    si = i;
                    ch = false;
                }
                if (eb == size){
                    for(int j=si;j<si+size;j++){
                        mem[j] = 1;
                    }
                    is_available = true;
                    break;
                }
            }
            else{
                ch = true;
                eb = 0;
            }
        }
        if (is_available == false){
            si = -1;
            np = 0;
        }
        else {*s_i = si;np = si + size;}
        return si;
    }
}

void memory_deallocation(int *arr, int size, int* si, int tid){
    pthread_mutex_lock(&lock);
    printf("*****************\nMemory deallocation began\nProcess %d is being deallocated\n*******************\n", tid);
    printf("Memory:\n");
    for(int i=0;i<memory_size;i++) printf("%d ", mem[i]);
    printf("\n");
    for(int i=(*si);i<(*si)+size;i++){
        arr[i] = 0;
    }
    printf("Memory:\n");
    for(int i=0;i<memory_size;i++) printf("%d ", mem[i]);
    printf("\n");
    pthread_mutex_unlock(&lock);
}

//thread which keeps allocation memory by popping from queue
void* mem_allocation(void *tdata){  
    while(true){
        //check if queue is empty or not
        if (q->front == NULL){
            sleep(1);
            continue;
        }
        if (time(NULL)-st >= T){    //if time has exceeded time of execution
            break;
        }
        /*if (q->front == NULL && ch == true){
            break;
        }*/
        //take rear elememt of queue
        struct Node* top = q->front;
        
        //memory is displayed before allocation
        pthread_mutex_lock(&lock);  //mutex lock so that allocation and deallocation do not happen at the same time
        printf("Memory:\n");
        for(int i=0;i<memory_size;i++) printf("%d ", mem[i]);
        printf("\n");

        //printf("********************\nMemory Allocation\nProcess number %d is being allocated memory.Size = %d\n", top->tid, top->size);
        //fulfill memory allocation request if possible
        int si = memory_allocation(q->front->size, fit, q->front->si);
        //check if successful
        //if not possible, push it to the back of the queue and return
        if (si == -1){
            printf("Unsuccessful memory allocation\n******************************\n");
            //enqueue(q, q->front->tid, q->front->size, q->front->si);
            //dequeue(q);
            //display(q);
            pthread_mutex_unlock(&lock);
            sleep(1);
            continue;
        }
        //if possible, dequeue
        dequeue(q);
        printf("Successful memory allocation\n********************\n");
        display(q);

        //memory is displayed after allocation
        printf("Memory:\n");
        for(int i=0;i<memory_size;i++) printf("%d ", mem[i]);
        printf("\n");
        pthread_mutex_unlock(&lock);
        //sleep(1);
    }
    pthread_exit(NULL);
}


void* thread_function(void *tdata){
    struct thread_data* td = (struct thread_data*) tdata;
    /*int si; //start index of allocated memory
    si = memory_allocation(td->mem, td->size, 1);
    sleep(td->duration);
    memory_deallocation(td->mem, td->size, si);*/
    //TODO: push request for memory allocation to queue
    int* si = (int*)malloc(sizeof(int));    //si is status variable, to check if allocation successful or not
    *si = -1;
    time_t start_mem = time(NULL);
    //push request for memory allocation to queue
    //printf("Enqueue process %d\n", td->tid);
    enqueue(q, td->tid, td->size, si);
    //keep checking for return value from memory allocation request
    while((*si) == -1);
    //calculate time taken for allocation
    int diff = time(NULL) - start_mem;
    //updating average turnaround time
    process_number++;
    process_time+=diff;
    // sleep for required duration
    sleep(td->duration);
    //printf("hehe*********************************************\n");
    //then deallocate
    memory_deallocation(mem, td->size, si, td->tid);
    free(si);   //free status variable
    //destroy the thread
    pthread_exit(NULL); //delete the thread
}

int main(){

    //defining input parameters
    int p;  //total size of physical memory
    int q_value;  //size reserved for os
    int n;  //used in defining process arival rate
    int m;  //used in defining sizes of the processes
    int t;  //used in defining duration of each process
      //total duration of process is T
    printf("Enter (p, q, n, m, t, T, placement): ");
    scanf("%d %d %d %d %d %d %d", &p, &q_value, &n, &m, &t, &T, &fit);   //accepting all values in variables
    // defining process arrival rate
    int r;  //process arrival rate
    
    int s;  //size of processes in multiple of 10
    int d;  //duration of processes in multiple of 5

    //memory in form of array
    mem = malloc(sizeof(int)*p);    //allocating memory
    for(int i=0;i<p;i++) mem[i] = 0;
    for(int i=0;i<q_value;i++) mem[i] = 1;
    /*printf("Memory is now ready for allocation\n");
    for(int i=0;i<p;i++) printf("%d ", mem[i]);
    printf("\n");*/

    //start time
    st = time(NULL);

    //initialization of global variables
    memory_size = p;
    tc = 0;
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return 1;
    }
    process_number = 0; //variables to help in turnaround time calculation
    process_time = 0;

    q = create_Queue();
    //printf("Queue is created for memory allocation process\n");

    //keep running algorithm till execution time
    //at every r interval of time, create new processes 
    //printf("%d\n", time(NULL)-st);
    int count = 0;

    // create memory allocation thread
    pthread_t thread_index = tc;
    tc++;
    pthread_create(&thread_index, NULL, mem_allocation, NULL);
    //pthread_join(thread_index, NULL);

    //processes created according to r-- process arrival rate
    while (time(NULL)-st <= T){
        if (time(NULL)-st >= count){    //this line ensures that every sec the following code runs
            count++; 
            r = rand()%((int)(floor2(1.2*n) - ceil2(0.1*n) + 1)) + ceil2(0.1*n);    //random process arrival rate
            printf("Process arrival rate is: %d\n", r);
            for(int i=0;i<r;i++){
                pthread_t thread_id = tc;
                tc++;
                s = rand()%((int)(floor2(0.3*m) - ceil2(0.05*m) + 1)) + ceil2(0.05*m);  //defining random sizes
                s*=10;
                d = rand()%((int)(floor2(1.2*t) - ceil2(0.1*t) + 1)) + ceil2(0.1*t);    //defining random durations
                d*=5;
                struct thread_data *tdata = malloc(sizeof(struct thread_data));
                tdata->size = s;
                tdata->duration = d;
                tdata->tid = thread_id;
                int check;
                printf("Time:\t%ld\tProcess %ld is created\tsize: %d\tduration: %d\n", time(NULL)-st,thread_id, s, d);
                //display(q);
                check = pthread_create(&thread_id, NULL, thread_function, (void *)tdata);
                //pthread_join(thread_id, NULL);
                if (check){
                    printf("\n ERROR: thread creation failed \n");
                    exit(1);
                }
            }
        }
    }
    printf("Turnaround time = %f\n", ((float)process_time)/process_number);
    printf("Memory utilization = %f\n", memory_utilization(mem, memory_size));
}
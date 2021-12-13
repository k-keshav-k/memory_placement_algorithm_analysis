README

1.	What does this program do ?
This program simulates dynamic partioning based memory placement. 
It uses first-fit, best-fit, next-fit for placing of process memory requests in memory and reports the relative effectiveness of the three placement algorithms. 
It uses a set of parameters and for different combination of values of these parameters it decides which placement algorithm is most effective by comparing memory utilization percentage and average turnaround time.

2.	Description of how this program works
This program works by first taking values of parameters as input from the user and then the program is run with the respective values of the parameters. 
According to the process arrival rate, the program creates ‘r’ threads every second to simulate processes. These r threads have random sizes and durations, according to the specifications. 
These threads then are enqueued in a queue. 
Another thread ‘memory allocation’ is created, with the purpose to allocate memory to the processes. This thread accesses the queue, then uses one of first-fit, best-fit or next-fit to allocate memory. If allocation is successful, then the requesting thread is dequeued, else the requesting thread is removed from the queue and added at the end. 
Memory allocation thread, to allocate meomory, calls a function. This function allocates memory, if possible, and then sets a memory location to be equal to the start position of allocated memory, if allocation was successful. 
The individual threads, each have a memory location assigned to them, which they regularly check. If the value at that memory location, for a particular memory requesting thread changes value from original value of -1, then the requesting thread sleeps for the process duration and then calls a function to deallocate memory. This deallocation function, deallocates memory starting from the start index of the allocated memory for the process.  
Mutex lock is used to ensure that only memory allocation or deallocation happens at a particular time.
When time of execution of the program exceedes ‘T’, then the program exits after printing memory utilization and average turnaround time of allocated processes.

3.	How to compile and run this program
gcc memory_placement.c -o a -lpthread
./a

4.  Sample Output
Sample output is shown in out_readme.txt file.
Input values: p=1000 q=200 n=10 m=10 t=10 T=300 memory_alogorithm=1

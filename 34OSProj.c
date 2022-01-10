/*
Group 34
Deepti Kumar 2018B5A70790H
Aswath Vinayak K 2018B4A70713H
Sankalamaddy Ruthvik Reddy 2018B4A70701H
Kartheek Sivavarma Nadimpalli 2018B4A70922H
Anirudh A 2018B4A70936H
Sanath Salil 2018B4A70812H
Eva Tiwari 2018B5A70816H
Srirampur Shreya 2018B4A70886H
*/

#include<stdio.h>
#include<errno.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include<string.h>
#include<pthread.h>
#include<signal.h>
#define NUM_THREADS 2
//Global Variables for shared memory resources
//Shared memory variable, '0' by default, '1' when thread finishes execution. This is to control the parallel execution of the threads.
int child1=0 , child2=0, child3=0 ; //0 = default, 1 = stop/pause. 
char *shmchild1 ;
char *shmchild2 ;
char *shmchild3 ;
int *shmflag1 ; //Monitoring thread. 
int *shmflag2 ;
int *shmflag3 ;
int shmid1 , shmid2, shmid3 ;  //Status of execution thread. 
int shmid11 , shmid21, shmid31 ; //For receiving/sending thread pid.
int waste ;
long double execution[3] ;
//Stores wait time and turnaround time for the three processes
long double waitt[3] , turnaround[3] ;
long double averagewait, averageturnaround ;
//To decide between FCFS and Round Robin
char algo[40] ;
//Threads
pthread_t threadid1,threadid2,threadid3;
pid_t threadpid[3];
//Time quantum for Round Robin Scheduling 
long quanta ;
//Keys for Shared memory
key_t key1 , key2, key3 , key11, key21, key31;
//Pipes to communicate between child processes and process M - pdfs = pipe descriptor (-1 means error)
int pdfsc1_m[2];
int pdfsc2_m[2];
int pdfsc3_m[2];

//Code for Child process 1 - Add numbers (Compute Intensive)
void * taskC1(void * arg){
    if((shmid11 = shmget(2041,32,0666 | IPC_CREAT)) == -1)exit(1) ; //Stores PID. Attributes: key, size, user permissions | IPC- creates a new msg queue if there doesnt exist one corresponding to the "key". Returns msg queue ID / -1 for error
    shmflag1 = (int*)shmat(shmid11,(void*)0,0) ; //Pointer to shared memory. (shmat =>shm attach). 0 if ok, -1 if error. Attributes: shmid, const void *addr, flag 
    if (shmflag1 == (int *) -1)exit(2);
    int id = getpid() ;
    shmflag1[0] = id ; 
    pthread_kill(pthread_self(),SIGSTOP); //Returns pid to M and waits to get "go" signal from M to start executing. This makes sure that the processes dont run at the same time. 
    printf("\n");
    //Loading value of n1
    long n1 ;
    n1 = (long)arg ;
    long long int sum=0;
    //Adding numbers from 1 to n1
    for(int i=1;i<=n1;i++)
        sum+=i;
    close(pdfsc1_m[0]); //Close read end. 
    //Output sent to M
    write(pdfsc1_m[1], &sum, sizeof(sum)); //Write or send value of sum to Master process. Attributes: file desc(where to write), what to write, size
    //Shared variable
    printf("\nC1 complete \n") ;
    if((shmid1 = shmget(2044,32,0666 | IPC_CREAT)) == -1)exit(1) ;
    shmchild1 = (char*)shmat(shmid1,(void*)0,0) ; 
    if (shmchild1 == (char *) -1)exit(2);
    shmchild1[0] = '1'; //stop c1
    //Shared variable written and ID closed
    pthread_exit(NULL);
}
//Code for Child process 2 - IO intensive. (read and print)
void * taskC2(void * arg){
    if((shmid21 = shmget(2042,32,0666 | IPC_CREAT)) == -1)exit(1) ; //Stores PID. Attributes:key, size, user permissions | IPC- creates a new msg queue if there doesnt exist one corresponding to the "key". Returns msgg queue ID / -1 for error
    shmflag2 = (int*)shmat(shmid21,(void*)0,0) ; //Pointer to shared memory. (shmat =>shm attach). 0 if ok, -1 if error. Attributes: shmid, const void *addr, flag 
    if (shmflag2 == (int *) -1)exit(2);
    int id = getpid() ;
    shmflag2[0] = id ;
    pthread_kill(pthread_self(),SIGSTOP); //Returns pid to M and waits to get "go" signal from M to start executing. This makes sure that the processes dont run at the same time. 
    printf("\n");
    //Loading value of n2
    long n2 ;
    n2 = (long)arg ;
    //Opening text file to take input from
    FILE *rfile = fopen("sample_large_1mill.txt", "r") ;
    long num ; 
    int i = 0 ; 
    //reading n2 numbers from the file and printing them out
    while ( i<n2)  
    { 
           fscanf(rfile, "%ld", &num ); 
           printf("%ld\n", num); 
           i++;
    } 
    char s[20]="Done Printing!";
    //Shared Variable
    close(pdfsc2_m[0]); //Close read end. 
    write(pdfsc2_m[1],s,strlen(s)+1); //Write or send Completed msg to Master process. Attributes: file desc(where to write), what to write, size
    fclose(rfile);
    //Shared Variable written and ID closed
    printf("\nC2 complete\n"); 
    if((shmid2 = shmget(2045,32,0666 | IPC_CREAT)) == -1)exit(1) ;
    shmchild2 = (char*)shmat(shmid2,(void*)0,0) ; 
    if (shmchild2 == (char *) -1)exit(2);
    shmchild2[0] = '1'; //stop c2
    pthread_exit(NULL);	
}
//Code for Child process 3
void * taskC3(void * arg){
    if((shmid31 = shmget(2043,32,0666 | IPC_CREAT)) == -1)exit(1) ; //Stores PID. Attributes: key, size, user permissions | IPC- creates a new msg queue if there doesnt exist one corresponding to the "key". Returns msgg queue ID / -1 for error
    shmflag3 = (int*)shmat(shmid31,(void*)0,0) ;  //Pointer to shared memory. (shmat =>shm attach). 0 if ok, -1 if error. Attributes: shmid, const void *addr, flag 
    if (shmflag3 == (int *) -1)exit(2);
    int id = getpid() ;
    shmflag3[0] = id ;
    pthread_kill(pthread_self(),SIGSTOP); //Returns pid to M and waits to get "go" signal from M to start executing. This makes sure that the processes dont run at the same time. 
    printf("\n");
    //Loading value of n3
    long n3 ;
    n3 = (long)arg ;
    long num1, num[1000000];
    int i = 0 ;
    //Opening file to read input from
    FILE *rfile = fopen("sample_large_1mill.txt", "r") ;
    //Reading each number and adding it to total
    while ( i<n3 )  
    { 
           fscanf(rfile, "%ld", &num1 );
           num[i] = num1 ; i = i+1 ;
    } 
    unsigned long long int sum=0;
    for(int i=0;i<n3;i++)
        sum+=num[i];
    //Shared variable
        close(pdfsc3_m[0]);
    write(pdfsc3_m[1],&sum,sizeof(sum)); //Write or send value of sum to Master process. Attributes: file desc(where to write), what to write, size
    if((shmid3 = shmget(2046,32,0666 | IPC_CREAT)) == -1)exit(1) ;
    shmchild3 = (char*)shmat(shmid3,(void*)0,0) ; 
    if (shmchild3 == (char *) -1)exit(2);
    fclose(rfile);
    //Shared variable written and ID closed
    printf("\nC3 complete\n") ;
    shmchild3[0] = '1'; //stop c3
    pthread_exit(NULL);
}


int main()
{
    //Process M
    long n1,n2,n3;
    printf("\nEnter n1, n2 and n3: ") ;
    scanf("%ld %ld %ld",&n1,&n2,&n3);
    printf("Enter scheduling algorithm (R/r for round robin, F/f for FCFS): ") ;
    scanf("%s" , algo) ; //choose between FCFS, RR. 
    if(algo[0] == 'R' || algo[0] == 'r'){
            printf("Enter time quanta (in microseconds, values between 1-10000): ") ;
            scanf("%ld", &quanta) ;
        }
    pid_t pid , p , p1;
    //Pipe creation - Pipes to communicate between child processes and process M - pdfs = pipe descriptor (-1 means error)
    if(pipe(pdfsc1_m)==-1)
        exit(1);
    if(pipe(pdfsc2_m)==-1)
        exit(1);
    if(pipe(pdfsc3_m)==-1)
        exit(1);
    pid=fork();
    //Fork failure condition
    if(pid<0 )
        exit(1);
    //Process C1
    if(pid==0 )
    {
        int rc ;
        pthread_attr_t attr;
        pthread_attr_init(&attr); //Initialises thread attributes. So, implemented before thread creation. 0 on success. $attr = data st holding thread attributes.
        rc = pthread_create(&threadid1,NULL,taskC1, (void*)n1) ; //0 on success. Attributes: pointer to threadID, pointer to thread attribute, user defined function, void & ard for the function. 
        if(rc == -1 ){
            exit(1) ;
        }        
        pthread_join(threadid1,NULL); //Parameters: ID, return val. Will make C1 wait till the thread with "threadid1" has completed the task.
    }
    //Child processes C2,C3 
    else if(pid >0 )
    {
       p = fork() ;  
       if(p<0 )exit(1);
       //Process C2
       else if (p == 0 )
        { 
            int rc ;
            pthread_attr_t attr;
            pthread_attr_init(&attr); //Initialises thread attributes. So, implemented before thread creation. 0 on success. $attr = data st holding thread attributes.
            rc = pthread_create(&threadid2,NULL,taskC2, (void*)n2) ; //0 on success. Attributes: pointer to threadID, pointer to thread attribute, user defined function, void & ard for the function. 
            if(rc == -1 ){
                exit(1) ;
            }
            pthread_join(threadid2,NULL); //Parameters: ID, return val. Will make C1 wait till the thread with "threadid1" has completed the task.
            
        }
        else if(p > 0)
        {
            p1 = fork() ;  
            if(p1<0 )exit(1);
            //Process C3
            else if (p1 == 0 )
            {       
                    int rc ;
                    pthread_attr_t attr;
                    pthread_attr_init(&attr); //Initialises thread attributes. So, implemented before thread creation. 0 on success. $attr = data st holding thread attributes.
                    rc = pthread_create(&threadid3,NULL,taskC3, (void*)n3) ; //0 on success. Attributes: pointer to threadID, pointer to thread attribute, user defined function, void & ard for the function. 
                    if(rc == -1 ){
                        exit(1) ;
                    }
                    pthread_join(threadid3,NULL); //Parameters: ID, return val. Will make C1 wait till the thread with "threadid1" has completed the task.

            }
            else if (p1 > 0 ) 
            {
                    
                    //Implementing Scheduling Algorithms
                    
                    if(algo[0] == 'F' || algo[0] == 'f'){
                        //FCFS Scheduling Algorithm
                        
                        //Getting Thread PIDs
                        sleep(5);
                        if((shmid11 = shmget(2041,32,0666 | IPC_CREAT)) == -1)exit(1) ; //Stores PID. Attributes: key, size, user permissions | IPC- creates a new msg queue if there doesnt exist one corresponding to the "key". Returns msgg queue ID / -1 for error
                        if((shmid21 = shmget(2042,32,0666 | IPC_CREAT)) == -1)exit(1) ;
                        if((shmid31 = shmget(2043,32,0666 | IPC_CREAT)) == -1)exit(1) ;
                        shmflag1 =shmat(shmid11,(void*)0,0) ; //Pointer to shared memory. (shmat =>shm attach). 0 if ok, -1 if error. Attributes: shmid, const void *addr, flag 
                        shmflag2 =shmat(shmid21,(void*)0,0) ;
                        shmflag3 =shmat(shmid31,(void*)0,0) ; 
                        threadpid[0] = shmflag1[0]; //opening shared mem to get pid. 
                        threadpid[1] = shmflag2[0];
                        threadpid[2] = shmflag3[0];
                        shmdt((void*)shmflag1) ; //shm dettach(const void * addr) - 0 if ok, -1 if error
                        shmdt((void*)shmflag2) ;
                        shmdt((void*)shmflag3) ;
                        
                        //Getting status
                        if((shmid1 = shmget(2044,32,0666 | IPC_CREAT)) == -1)exit(1) ; 
                        if((shmid2 = shmget(2045,32,0666 | IPC_CREAT)) == -1)exit(1) ;
                        if((shmid3 = shmget(2046,32,0666 | IPC_CREAT)) == -1)exit(1) ;
                        shmchild1 =shmat(shmid1,(void*)0,0) ;
                        shmchild2 =shmat(shmid1,(void*)0,0) ;
                        shmchild3 =shmat(shmid1,(void*)0,0) ;
                        if (shmchild1 == (char *) -1)exit(2);
                        shmchild1[0] = '0'; //written '0' to shared memory. 
                        if (shmchild2 == (char *) -1)exit(2);
                        shmchild2[0] = '0';
                        if (shmchild3 == (char *) -1)exit(2);
                        shmchild3[0] = '0';
                        child1 = 0 ;
                        child2 = 0 ;
                        child3 = 0 ;
                        execution[0] = 0 ;
                        execution[1] = 0 ;
                        execution[2] = 0 ;
                        //Scheduling
                        sleep(5);
                        long double ti = (long double)clock(); //Stores number of clock ticks.
                        execution[0] = -1*ti ; //Final + (-initial)
                        printf("C1 at clock time: %Lf\n", ti  );
                        kill(threadpid[0],SIGCONT); //asks threadpid[0] to continue. 
                        ti = (long double)clock()  ;
                        execution[0] += ti; //Add time
                        execution[0] /= CLOCKS_PER_SEC ; //Divide to get time from clock ticks. 
                        printf("C1 ends at clock time: %Lf\n", ti );
                        printf("\n");
                        pthread_join(threadid1,NULL); //Wait for C1
                        while(!child1)
                        	child1 = (shmchild1[0] - '0') ; //Until c1 has completed just keep checking status. 
                        sleep(3);
                        ti = (long double)clock();
                        execution[1] = -1 *ti ;
                        printf("C2 at clock time: %Lf\n", ti );
                        kill(threadpid[1],SIGCONT);
                        ti = (long double)clock()  ;
                        execution[1] += ti;
                        execution[1] /= CLOCKS_PER_SEC ;
                        printf("C2 ends at clock time: %Lf\n",ti );
                        printf("\n");
                        pthread_join(threadid2,NULL);
                        while(!child2)
                        	child2 = (shmchild2[0] - '0') ;
                        sleep(3);

                        ti = (long double)clock();
                        execution[2] = -1 *ti ;
                        printf("C3 at clock time: %Lf\n", ti );
                        kill(threadpid[2],SIGCONT);
                        ti = (long double)clock()  ;
                        execution[2] += ti;
                        execution[2] /= CLOCKS_PER_SEC ;

                        printf("C3 ends at clock time: %Lf\n",ti  );
                        printf("\n");
                        pthread_join(threadid3,NULL);
                        waitt[0] =  0 ;
                        waitt[1] = (execution[0]) + waitt[0] ; //C2 has to wait for C1 to complete
                        waitt[2] = (execution[1]) + waitt[1] ; //C3 waits for C2 to complete
                        //Turnaround time = Waiting time + Execution time
                        turnaround[0] = waitt[0] + (execution[0]) ;
                        turnaround[1] = waitt[1] + (execution[1]) ;
                        turnaround[2] = waitt[2] + (execution[2]) ;
                        //Calculating average waiting time and turnaround time
                        averagewait= ((waitt[0] + waitt[1] + waitt[2])) / 3.0 ;
                        averageturnaround =((turnaround[0]+turnaround[1]+turnaround[2])) /3.0 ;
                        //Printing details
                        printf("\nFirst Come First Serve scheduling details: ") ;
                        printf("\nProcess\tWaiting Time\tTurn Around Time") ;
                        for(int i = 0 ; i < 3 ; i++){
                            printf("\nC%d\t%Lf\t%Lf" , i+1 , waitt[i] , turnaround[i]) ;
                        }
                        printf("\nAverage waiting time is: %Lf" , averagewait) ;
                        printf("\nAverage Turn Around time is: %Lf" , averageturnaround) ;
                    }
                    else{
                        //Round Robin Scheduling Algorithm
                        //
                        sleep(5);
                        if((shmid11 = shmget(2041,32,0666 | IPC_CREAT)) == -1)exit(1) ; //Stores PID. Attributes: key, size, user permissions | IPC- creates a new msg queue if there doesnt exist one corresponding to the "key". Returns msgg queue ID / -1 for error
                        if((shmid21 = shmget(2042,32,0666 | IPC_CREAT)) == -1)exit(1) ;
                        if((shmid31 = shmget(2043,32,0666 | IPC_CREAT)) == -1)exit(1) ;
                        shmflag1 =shmat(shmid11,(void*)0,0) ;
                        shmflag2 =shmat(shmid21,(void*)0,0) ;
                        shmflag3 =shmat(shmid31,(void*)0,0) ; 
                        threadpid[0] = shmflag1[0];
                        threadpid[1] = shmflag2[0];
                        threadpid[2] = shmflag3[0];
                        shmdt((void*)shmflag1) ;
                        shmdt((void*)shmflag2) ;
                        shmdt((void*)shmflag3) ;
                        sleep(10) ;
                        if((shmid1 = shmget(2044,32,0666 | IPC_CREAT)) == -1)exit(1) ;
                        if((shmid2 = shmget(2045,32,0666 | IPC_CREAT)) == -1)exit(1) ;
                        if((shmid3 = shmget(2046,32,0666 | IPC_CREAT)) == -1)exit(1) ;
                        //These variables contain '1' or '0'. '0' by default, '1' when the thread is to be executed.
                        //This is done to control the parallel execution of threads in round robin.
                        shmchild1 =(char*)shmat(shmid1,(void*)0,0) ;
                        shmchild2 =(char*)shmat(shmid1,(void*)0,0) ;
                        shmchild3 =(char*)shmat(shmid1,(void*)0,0) ; // contains '1' or '0'
                        if (shmchild1 == (char *) -1)exit(2);
                        shmchild1[0] = '0'; //written '0' to shared memory
                        if (shmchild2 == (char *) -1)exit(2);
                        shmchild2[0] = '0';
                        if (shmchild3 == (char *) -1)exit(2);
                        shmchild3[0] = '0';
                        child1 = 0 ;
                        child2 = 0 ;
                        child3 = 0 ;
                        long double ti ;
                        execution[0] = 0 ;
                        execution[1] = 0 ;
                        execution[2] = 0 ;
                        while(!child1 || !child2 || !child3){
                            if(!child1){
                            ti = (long double)clock() ;
                            execution[0] += -1 * ti ;
                            printf("Starting C1 in RR: %Lf\n", ti);
                            sleep(1); 
                            kill(threadpid[0],SIGCONT);
                            usleep(quanta); //usleep => microsecs for sleep. For monitor thread. 
                            kill(threadpid[0],SIGSTOP);
                            ti = (long double)clock() ;
                            execution[0] += ti ;
                            printf("Stopping C1 in RR: %Lf\n",ti);
                            child1 = (int)(shmchild1[0] - '0') ;
                            }
                            if(!child2)
                            {
                            ti = (long double)clock() ;
                            execution[1] += -1 * ti ;
                            printf("Starting C2 in RR at clock reading: %Lf\n",ti);
                            sleep(1);
                            kill(threadpid[1],SIGCONT);
                            usleep(quanta);
                            kill(threadpid[1],SIGSTOP);
                            ti = (long double)clock() ;
                            execution[1] += ti ;
                            printf("Stopping C2 in RR at clock reading: %Lf\n",ti);
                            child2 = (int)(shmchild2[0] - '0') ;
                            }
                            if(!child3)
                            {
                            ti = (long double)clock() ;
                            execution[2] += -1 * ti ;
                            printf("Starting C3 in RR at clock reading: %Lf\n",ti);
                            sleep(1);
                            kill(threadpid[2],SIGCONT);
                            usleep(quanta);
                            kill(threadpid[2],SIGSTOP);
                            ti = (long double)clock() ;
                            execution[2] += ti ;
                            printf("Stopping C3 in RR at clock reading: %Lf\n",ti);
                            child3 = (int)(shmchild3[0] - '0') ;
                            }


                        }
                        shmdt(shmchild1) ;
                        shmdt(shmchild2) ;
                        shmdt(shmchild3) ;
                        shmctl(shmchild1,IPC_RMID,NULL);
                        shmctl(shmchild2,IPC_RMID,NULL);
                        shmctl(shmchild3,IPC_RMID,NULL);
                        
                       
                        for(int i = 0 ; i < 3 ; i++){
                            execution[i] = execution[i] / CLOCKS_PER_SEC * 1000000 ;
                            printf("\nExec time of %d is: %Lf" , i+1 , execution[i]) ;
                        }
                        long double remaining_exe[3] ;
                        for(int i = 0 ; i < 3 ; i++){
                            remaining_exe[i] = (execution[i]) ;
                        }
                        int t = 0 ; // Current time
                        while(1){
                            int done = 1;
                            for(int i = 0 ; i < 3 ; i++){
                                if(remaining_exe[i] >0){
                                    done = 0 ;
                                    //Time Quanta condition, if remaining execution time is more than the value of quanta, the execution time is decreased by the value of quanta
                                    if(remaining_exe[i] > quanta){
                                        t+= quanta ;
                                        remaining_exe[i] -= quanta ;
                                    }
                                    else{
                                      
                                       t+= quanta ;
                                       remaining_exe[i] = 0 ;
                                       waitt[i] = t - (execution[i]) ;
                                    }
                                }
                            }
                            if(done == 1)break;
                        }
                        //Changing waiting time and turnaround time to seconds
                        for(int i = 0 ; i < 3 ; i++){
                        		
                            turnaround[i] = (execution[i]) + waitt[i] ; //TAT = total execution time. So, FT-AT or Exec time+ wait time. 
                            waitt[i] /= 1000000 ;
                            turnaround[i] /=1000000 ;
                        }
                        //Calculating average waiting time and average turnaround time
                        averagewait = ((waitt[0] + waitt[1] + waitt[2])) / 3.0 ;
                        averageturnaround = ((turnaround[0]+turnaround[1]+turnaround[2])) /3.0 ;

                        printf("\nRound Robin scheduling details: ") ;
                        printf("\nProcess\tWaiting Time\tTurn Around Time") ;
                        for(int i = 0 ; i < 3 ; i++){
                            printf("\nC%d\t%Lf\t%Lf" , i+1 , waitt[i] , turnaround[i]) ;
                        }
                        printf("\nAverage Waiting time is: %Lf" , averagewait) ;
                        printf("\nAverage Turn Around time is: %Lf" , averageturnaround) ;
                    }
                    close(pdfsc1_m[1]);
                    close(pdfsc2_m[1]);
                    close(pdfsc3_m[1]);
                    long long int c1res;
                    unsigned long long int c3res;
                    char buf[20];
                    read(pdfsc1_m[0],&c1res,sizeof(c1res));
                    read(pdfsc2_m[0],buf,30);
                    read(pdfsc3_m[0],&c3res,sizeof(c3res));
                    printf("\nC1: %lld \n",c1res);
                    printf("C2: %s \n",buf);
                    printf("C3: %llu \n",c3res);
            }
        }      
    }
    
    exit(1);



    return 0 ;
        
    
}

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>

extern int timeslice, switchmode;

typedef struct TCB_NODE *TCB_ptr;
typedef struct TCB_NODE{
    jmp_buf  Environment;
    int      Thread_id;
    TCB_ptr  Next;
    TCB_ptr  Prev;
    int i, N;
    int w, x, y, z;
} TCB;

extern jmp_buf MAIN, SCHEDULER;
extern TCB_ptr Head;
extern TCB_ptr Current;
extern TCB_ptr Work;
extern sigset_t base_mask, waiting_mask, tstp_mask, alrm_mask;

void sighandler(int signo);
void scheduler();

// Call function in the argument that is passed in
#define ThreadCreate(function, thread_id, number)                                         \
{   \
    if(setjmp(MAIN)==0)                                                                      \
	    function(thread_id, number);									                      \
}

// Build up TCB_NODE for each function, insert it into circular linked-list
#define ThreadInit(thread_id, number)                                                     \
{                                                                                         \
    if(Head==NULL){\
        Head=(TCB_ptr) malloc(sizeof(TCB));\
        Current=Head;\
        Current->Next=Current;\
        Current->Prev=Current;\
    }\
    else{\
        Work=(TCB_ptr)malloc(sizeof(TCB));\
        Current->Next->Prev=Work;\
        Work->Next=Current->Next;\
        Work->Prev=Current;\
        Current->Next=Work;\
        Current=Work;\
    }\
    Current->Thread_id=thread_id;\
}

// Call this while a thread is terminated
#define ThreadExit()                                                                      \
{                                                                                         \
    longjmp(SCHEDULER,2);\
}

// Decided whether to "context switch" based on the switchmode argument passed in main.c
#define ThreadYield()                                                                     \
{                                                                                           \
    if(setjmp(Current->Environment)==0){\
        if(switchmode==0){\
            longjmp(SCHEDULER,1);\
        }\
        else if(switchmode==1){\
            sigpending(&waiting_mask);\
            if(sigismember(&waiting_mask,SIGTSTP)){\
                sigprocmask(SIG_UNBLOCK,&tstp_mask,NULL);\
            }\
            else if(sigismember(&waiting_mask,SIGALRM)){\
                sigprocmask(SIG_UNBLOCK,&alrm_mask,NULL);\
            }\
        }\
    }\
}

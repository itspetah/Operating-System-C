#ifndef SIMULATOR_H
#define SIMULATOR_H

// header files
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "StringUtils.h"
#include "configops.h"

typedef enum command_list_s
{
	SYS = 10,
	APP,
	DEVIN = 0,
	DEVOUT = 1,
	CPU,
	MEM
} command_t;

typedef enum string_args_list_s
{
	START = 10,
	END,
	PROCESS,
	ALLOCATE,
	ACCESS,
	ETHERNET = 0,
	HDD = 1,
	KEYBOARD = 2,
	MONITOR = 3,
	SERIAL = 4,
	SOUND_SIGNAL = 5,
	USB = 6,
	VIDEO_SIGNAL = 7
} string_args_t;

typedef struct executable_s
{
	command_t command;
	string_args_t strArgs1;
	int intArg2, intArg3;
	OpCodeType *origin;
	int time;
} executable_t;
//wil be on the final, how many status for a process
#define PROCESS_STATE_NEW 0
#define PROCESS_STATE_READY 1
#define PROCESS_STATE_RUN 2
#define PROCESS_STATE_BLOCKED 3
#define PROCESS_STATE_END 4

#define SMALL_QUANTUM 1e-3

typedef struct process_s
{
	int pid;
    int cur_exe;
	int exe_size;
	executable_t *execution_flow;
	int total_time;
	int start_flag, end_flag;
	int state;
	struct process_s* next;
} process_t;

typedef struct ready_queue_s
{
    process_t* head;
    process_t* tail;
    int size;
} ready_queue_t;

typedef struct sim_s
{
    int num_ended;
    Boolean interrupt;
    ready_queue_t* interrupt_q;
    ready_queue_t* ready_q;
    process_t* cur_proc;
    pthread_mutex_t mutex;
} sim_t;

typedef struct block_for_io_args_s
{
    sim_t* sim;
    process_t* proc;
    ConfigDataType* config;
    
} block_for_io_args_t;


//typedef struct memory
//{
//	int base;
//	int offset;
//	int pid;
//	struct memory *next;
//} memory_t;

// function prototypes


void sim_init(sim_t* sim, ready_queue_t* ready_q);
void sim_destroy(sim_t* sim);


void *msleep(void *args);
void timer(double millisecond);
void initial_process(int n, ConfigDataType *configPtr);
void memset_usr(char *ptr, int count, char value);

ready_queue_t* rq_new();
void rq_delete(ready_queue_t* rq);
void rq_push_back(ready_queue_t* rq, process_t* proc);
void rq_push_front(ready_queue_t* rq, process_t* proc);
void rq_insert_cmp_by_remaining_time(ready_queue_t* rq, process_t* proc);
void rq_insert_cmp_by_pid(ready_queue_t* rq, process_t* proc);
process_t* rq_pop_front(ready_queue_t* ready_q);
int rq_size(ready_queue_t* rq);

void runSim(ConfigDataType *configPtr, OpCodeType *metaDataMsterPtr);

#endif

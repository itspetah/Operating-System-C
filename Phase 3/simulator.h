#ifndef SIMULATOR_H
#define SIMULATOR_H

// header files
#include <stdio.h>
#include <stdlib.h>
#include "StringUtils.h"
#include "configops.h"

typedef enum command_list
{
	SYS,
	APP,
	DEVIN,
	DEVOUT,
	CPU,
	MEM
} command_t;

typedef enum string_arg_list
{
	START,
	END,
	PROCESS,
	ALLOCATE,
	ACCESS,
	ETHERNET,
	HDD,
	KEYBOARD,
	MONITOR,
	SERIAL,
	SOUND_SIGNAL,
	USB,
	VIDEO_SIGNAL
} string_args_t;

typedef struct executable
{
	command_t command;
	string_args_t strArgs1;
	int intArg2, intArg3;
	OpCodeType *origin;
	int time;
} executable_t;

#define PROCESS_STATE_NEW 0
#define PROCESS_STATE_READY 1
#define PROCESS_STATE_RUN 2
#define PROCESS_STATE_END 3

typedef struct process
{
	int pid;
	int exe_size;
	executable_t *execution_flow;
	int total_time;
	int start_flag, end_flat;
	int state;
	struct process *next;
} process_t;

typedef struct memory
{
	int base;
	int offset;
	int pid;
	struct memory *next;
} memory_t;

// function prototypes
void runSim(ConfigDataType *configPtr, OpCodeType *metaDataMsterPtr);

#endif

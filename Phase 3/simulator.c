#include "configops.h"
#include "metadataops.h"
#include "simulator.h"
#include "StringUtils.h"
#include "simtimer.h"

// use posix thread
#include <pthread.h>

/*
 Function Name: msleep
 Algorithm: Sleep some time in milliseconds,
            Only used by POSIX thread
 Precondition: The pointer to the time interval
 Postcondition: Block the thread for some time
 Exception: none
 Notes: none
 */
void *msleep(void *args)
{
    double millisecond = *((double *)args);
    char dummy[100];
    double start_time = accessTimer(LAP_TIMER, dummy);
    while (1000 * (accessTimer(LAP_TIMER, dummy) - start_time) <= millisecond)
        ;
    return NULL;
}

/*
 Function Name: timer
 Algorithm: Sleep some time in millisecond
 Precondition: Given the time for sleeping
 Postcondition: Block the thread for some time
 Exception: none
 Notes: none
 */
void timer(double millisecond)
{
    if (millisecond <= 1e-3)
        return;
    pthread_t pid;
    double *ptr = (double *)malloc(sizeof(double));
    *ptr = millisecond;
    // create POSIX thread for simulating the IO operation
    pthread_create(&pid, NULL, msleep, (void *)ptr);
    // join the thread and block it
    pthread_join(pid, NULL);
    free(ptr);
}

/*
 Function Name: mem_alloca_func
 Algorithm: Simulate the memory allocation operation
 Precondition: Given the address range
 Postcondition: currently, none
 Exception: none
 Notes: none
 */
Boolean mem_alloca_func(memory_t **mem_list, ConfigDataType *config, int base, int offset, int pid)
{
    if (base + offset > config->memAvailable)
    {
        //? base start from 0?
        return False;
    }
    memory_t *mem = *mem_list;
    while (mem)
    {
        // go through the memory request list and check if this request overlap woth any existing request
        if (!((mem->base < base && mem->base + mem->offset < base + offset) || (base < mem->base && base + offset < mem->base + mem->offset)))
        {
            return False;
            // overlap, can't allocate
        }
        mem = mem->next;
        // next node
    }

    // not overlap, so allocate it, store the request at the head of the list
    mem = (memory_t *)malloc(sizeof(memory_t));
    mem->base = base;
    mem->offset = offset;
    mem->pid = pid;
    mem->next = *mem_list;
    *mem_list = mem;
    // update the list head
    return True;
}

/*
 Function Name: mem_access_func
 Algorithm: Simulate the memory allocation operation
 Precondition: Given the address
 Postcondition: currently, none
 Exception: none
 Notes: none
 */
Boolean mem_access_func(memory_t *mem_list, int base, int offset, int pid)
{
    memory_t *mem = mem_list;
    while (mem)
    {
        // go though the memory request list and check if this request is inside of any allocated request of the process
        if (mem->base <= base && base + offset <= mem->base + mem->offset)
        {
            return pid == mem->pid;
        }
        mem = mem->next;
        // next node
    }
    return False;
    // not allocated
}

/*
 Function Name: runProcess
 Algorithm: master the process and execute the process one by one
 Precondition: given the head pointer of the process configuration and the config data type
 Postcondition: Provide whether should go further running
 Exception: none
 Notes: none
 */
void runProcess(process_t *current_process, memory_t **mem_list, ConfigDataType *config)
{
    int i;
    char value[1000];

    // app start
    sprintf(value, "OS: Process %d set from READY to RUNNING\n", current_process->pid);
    output_with_time(value, config);
    // set process state
    current_process->state = PROCESS_STATE_RUN;

    Boolean mem_fault = False;
    Boolean result;
    executable_t *exec;
    for (i = 0; !mem_fault && i < current_process->exe_size; ++i)
    {
        exec = &current_process->execution_flow[i];

        // for each process, running from the first execution flow to the end
        switch (exec->command)
        {
        case APP:
        {
            break;
        }
        case DEVIN:
        {
            sprintf(value, "Process: %d, %s intput operation start", current_process->pid, exec->origin->strArg1);
            output_with_time(value, config);

            timer(exec->time);
            sprintf(value, "Process: %d, %s input operation end", current_process->pid, exec->origin->strArg1);
            output_with_time(value, config);
            break;
        }
        case DEVOUT:
        {
            sprintf(value, "Process: %d, %s output operation start", current_process->pid, exec->origin->strArg1);
            output_with_time(value, config);

            timer(exec->time);
            sprintf(value, "Process: %d, %s output operation end", current_process->pid, exec->origin->strArg1);
            output_with_time(value, config);
            break;
        }
        case CPU:
        {
            sprintf(value, "Process: %d, cpu operation start", current_process->pid);
            output_with_time(value, config);

            timer(exec->time);
            sprintf(value, "Process: %d, cpu operation end", current_process->pid);
            output_with_time(value, config);
            break;
        }
        case MEM:
        {
            if (exec->strArgs1 == ALLOCATE)
            {
                sprintf(value, "Process: %d, attempting mem allocate request", current_process->pid);
                output_with_time(value, config);
                // check whether allocation area can be allocated
                result = mem_alloca_func(mem_list, config, exec->intArg2, exec->intArg3, current_process->pid);
                if (result)
                {
                    sprintf(value, "Process: %d, successful mem allocate request", current_process->pid);
                }
                else
                {
                    sprintf(value, "Process: %d, failed mem allocate request", current_process->pid);
                    mem_fault = True;
                }
                output_with_time(value, config);
                break;
            }
            else if (exec->strArgs1 == ACCESS)
            {
                sprintf(value, "Process: %d, attempting mem access access request", current_process->pid);
                output_with_time(value, config);
                // check whether the position can be accessed
                result = mem_access_func(*mem_list, exec->intArg2, exec->intArg3, current_process->pid);
                if (result)
                {
                    sprintf(value, "Process: %d, successful mem allocate request", current_process->pid);
                }
                else
                {
                    sprintf(value, "Process: %d, failed mem allocate request", current_process->pid);
                    mem_fault = True;
                }
                output_with_time(value, config);
                break;
            }
        }
        case SYS:
            break;
        }
    }

    // app end
    if (mem_fault)
    {
        output("", config);
        sprintf(value, "OS: Segmentation fault, Process %d ended", current_process->pid);
        output_with_time(value, config);
    }
    else
    {
        output("", config);
        sprintf(value, "OS: Process %d ended", current_process->pid);
        output_with_time(value, config);
    }
    sprintf(value, "OS: Process %d set to EXIT", current_process->pid);
    output_with_time(value, config);
    current_process->state = PROCESS_STATE_END;
}

/*
 Function Name: initial_process
 Algorithm: Initial the process status and output the message, place holder
 Precondition: The process count and the config
 Postcondition: Output the message
 Exception: none
 Notes: none
 */
void initial_process(int n, ConfigDataType *configPtr)
{
    char value[1007];
    int i;
    for (i = 0; i < n; ++i)
    {
        sprintf(value, "OS: Process %d set to READY state from NEW state.", i);
        output_with_time(value, configPtr);
    }
}

/*
 Function Name: memset_usr
 Algorithm: set the memory area with the same value
 Precondition: The memory pointer and the count with value
 Postcondition: The filled memory area
 Exception: none
 Notes: none
 */
void memset_usr(char *ptr, int count, char value)
{
    int i;
    for (i = 0; i < count; ++i)
    {
        ptr[i] = value;
    }
}

/*
 Function Name: schedule
 Algorithm: Pick up the next process to run based on the schedule alforithm
 Precondition: The process list and the config
 Postcondition: The next process to run
 Exception: none
 Notes: The process picked up will be removed from process list
 */
process_t *schedule(process_t **process_list, ConfigDataType *configPtr)
{
    if (*process_list == NULL)
    {
        return NULL;
    }

    process_t *selected = NULL;
    if (configPtr->cpuSchedCode == CPU_SCHED_FCFS_N_CODE)
    {
        // FCFS-N: first come first serve
        selected = *process_list;
        *process_list = selected->next;
    }
    else if (configPtr->cpuSchedCode == CPU_SCHED_SJF_N_CODE)
    {
        // SJN-N: Shortest job first
        process_t *selected_prev = NULL;
        // previous node of the selected node
        process_t *prev = NULL;
        // previous node of proc
        process_t *proc = *process_list;
        while (proc)
        {
            if (proc->state == PROCESS_STATE_NEW)
            {
                if (!selected || proc->total_time < selected->total_time)
                {
                    selected = proc;
                    selected_prev = prev;
                }
            }
            prev = proc;
            proc = proc->next;
        }
        // remove it from the list
        if (selected_prev)
        {
            selected_prev->next = selected->next;
        }
        else
        {
            *process_list = selected->next;
        }
    }
    return selected; // return the process
}

/*
 Function Name: runSims
 Algorithm: master driver for simulator operations;
            conducts OS simulation with varyin sceduling strategies
            and vary numbers of processes
 Precondition: Given head pointer to config data and metadata
 Postcondition: simulator is provided, file output is provided as configured
 Exception: none
 Notes: none
 */
void runSim(ConfigDataType *configPtr, OpCodeType *metaDataMsterPtr)
{
    if (configPtr->logToCode == LOGTO_BOTH_CODE || configPtr->logToCode == LOGTO_FILE_CODE)
    {
        FILE *fp = fopen(configPtr->logToFileName, "w+");
        char displayString[STD_STR_LEN];
        fprintf(fp, "\n==================================================\n");
        fprintf(fp, "Simulator Log File Header\n\n");
        fprintf(fp, "File Name                       : %s\n", configPtr->metaDataFileName);
        configCodeToString(configPtr->cpuSchedCode, displayString);
        fprintf(fp, "CPU Scheduling                  : %s\n", displayString);
        fprintf(fp, "Quantum Cycles                  : %d\n", configPtr->quantumCycles);
        fprintf(fp, "Memory Available (KB)           : %d\n", configPtr->memAvailable);
        fprintf(fp, "Processor Cycle Rate (ms/cycle) : %d\n", configPtr->procCycleRate);
        fprintf(fp, "I/O Cycle Rate (ms/cycle)       : %d\n", configPtr->ioCycleRate);
        fprintf(fp, "\n================\n");
        fprintf(fp, "Begin Simulation\n\n");
        fclose(fp);
    }

    printf("Simulation Run\n------------\n\n");

    // Make the timer enabled
    runTimer(0);
    // make the dummy char array for timer
    char value[100];
    // reset the timer here
    accessTimer(ZERO_TIMER, value);
    // output the simulation start message
    output_with_time("OS: Simulator start", configPtr);

    OpCodeType *ptr = metaDataMsterPtr;
    process_t *root_process_list = NULL;
    process_t *prevProcess = NULL;
    process_t *process_list;
    OpCodeType *app_start = NULL, *app_end;
    int range = 0;
    int process_cnt = 0;
    // visit all the config pointer and make it into process information
    while (ptr != NULL)
    {
        // sys start and end situation
        if (compareString(ptr->command, "sys") == 0)
        {
            // Do nothing!!!
        }
        else if (compareString(ptr->command, "app") == 0)
        {
            // app start and end
            if (compareString(ptr->strArg1, "start") == 0)
            {
                // set the range size and the pointer
                range = 1;
                app_start = ptr;
            }
            else if (compareString(ptr->strArg1, "end") == 0)
            {
                // set range and the end pointer
                range++;
                app_end = ptr;
                // malloc the initial execution list object and initialze the value to zero
                process_list = (process_t *)malloc(sizeof(process_t));
                memset_usr((char *)process_list, (sizeof(process_t)) / sizeof(char), 0);
                process_list->execution_flow = (executable_t *)malloc(sizeof(executable_t) * (range));
                memset_usr((char *)process_list->execution_flow, (sizeof(executable_t) * (range)) / sizeof(char), 0);
                // set up the size
                process_list->exe_size = range;
                process_list->total_time = 0;
                process_list->pid = process_cnt;

                int command_idx = 0;
                struct OpCodeType *app_ptr;
                // make the original string type into defined type
                // visit from the first pointer to the end(application range)
                for (app_ptr = app_start; app_ptr != app_end; app_ptr = app_ptr->nextNode)
                {
                    executable_t *exec = &process_list->execution_flow[command_idx];

                    // int args
                    exec->intArg2 = app_ptr->intArg2;
                    exec->intArg3 = app_ptr->intArg3;
                    exec->origin = app_ptr;
                    process_list->state = PROCESS_STATE_NEW;

                    if (compareString(app_ptr->command, "app") == 0)
                    {
                        // app start
                        if (compareString(app_ptr->strArg1, "start") == 0)
                        {
                            exec->command = APP;
                            exec->strArgs1 = START;
                        }
                        else if (compareString(app_ptr->strArg1, "end") == 0)
                        {
                            // app end
                            exec->command = APP;
                            exec->strArgs1 = START;
                        }
                        exec->time = exec->intArg2 * configPtr->procCycleRate;
                    }
                    else if (compareString(app_ptr->command, "cpu") == 0)
                    {
                        // cpu command
                        if (compareString(app_ptr->strArg1, "process") == 0)
                        {
                            exec->command = CPU;
                            exec->strArgs1 = PROCESS;
                        }
                        exec->time = exec->intArg2 * configPtr->procCycleRate;
                    }
                    else if (compareString(app_ptr->command, "dev") == 0)
                    {
                        if (compareString(app_ptr->inOutArg, "in") == 0)
                        {
                            // device in
                            exec->command = DEVIN;
                        }
                        else if (compareString(app_ptr->inOutArg, "out") == 0)
                        {
                            // device out
                            exec->command = DEVOUT;
                        }
                        // device type
                        if (compareString(app_ptr->strArg1, "monitor") == 0)
                            exec->strArgs1 = MONITOR;
                        if (compareString(app_ptr->strArg1, "sound signal") == 0)
                            exec->strArgs1 = SOUND_SIGNAL;
                        if (compareString(app_ptr->strArg1, "ethernet") == 0)
                            exec->strArgs1 = ETHERNET;
                        if (compareString(app_ptr->strArg1, "hard drive") == 0)
                            exec->strArgs1 = HDD;
                        if (compareString(app_ptr->strArg1, "keyboard") == 0)
                            exec->strArgs1 = KEYBOARD;
                        if (compareString(app_ptr->strArg1, "serial") == 0)
                            exec->strArgs1 = SERIAL;
                        if (compareString(app_ptr->strArg1, "video signal") == 0)
                            exec->strArgs1 = VIDEO_SIGNAL;
                        if (compareString(app_ptr->strArg1, "usb") == 0)
                            exec->strArgs1 = USB;

                        exec->time = exec->intArg2 * configPtr->ioCycleRate;
                    }
                    else if (compareString(app_ptr->command, "mem") == 0)
                    {
                        exec->command = MEM;

                        if (compareString(app_ptr->strArg1, "access") == 0)
                            // memory access
                            exec->strArgs1 = ACCESS;
                        else if (compareString(app_ptr->strArg1, "allocate") == 0)
                            // memory allocation
                            exec->strArgs1 = ALLOCATE;

                        exec->time = 0;
                    }
                    // int args
                    process_list->total_time += exec->time;
                    command_idx += 1;
                }
                // if this is not the first process, then add to the link
                if (prevProcess != NULL)
                    prevProcess->next = process_list;
                else
                { // else it's the very first process
                    root_process_list = process_list;
                }
                // update the previous process
                // malloc the initial process list object and initialize the value to zero
                prevProcess = process_list;
                process_cnt++;
            }
        }
        else
        {
            range++;
        }
        // set connection with next node
        ptr = ptr->nextNode;
    }
    // output_with_time("OS: System start", configPtr);
    initial_process(process_cnt, configPtr);
    process_list = root_process_list;

    memory_t *mem_list = NULL;
    process_t *selected_proc;
    while ((selected_proc = schedule(&process_list, configPtr)) != False)
    {
        sprintf(value, "OS: Process %d selected with %d ms remaining", selected_proc->pid, selected_proc->total_time);
        output_with_time(value, configPtr);
        // run from the first process
        runProcess(selected_proc, &mem_list, configPtr);

        // free the memory since this process is EXITED
        memory_t *mem = mem_list;
        while (mem)
        {
            mem_list = mem->next;
            free(mem);
            mem = mem_list;
        }
        free(selected_proc->execution_flow);
        free(selected_proc);
    }
    output_with_time("OS: System stop", configPtr);
    output_with_time("OS: Simulation end", configPtr);

    if (configPtr->logToCode == LOGTO_BOTH_CODE || configPtr->logToCode == LOGTO_FILE_CODE)
    {
        FILE *fp = fopen(configPtr->logToFileName, "a+");
        fprintf(fp, "\nEnd Simulation - Complete\n");
        fprintf(fp, "=========================\n\n");
        fclose(fp);
    }
}

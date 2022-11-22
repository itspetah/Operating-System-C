#include "configops.h"
#include "metadataops.h"
#include "simulator.h"
#include "StringUtils.h"
#include "simtimer.h"

#include <assert.h>

// use posix thread
//#include <pthread.h>


process_t* schedule(ready_queue_t* ready_q, ConfigDataType* configPtr)
{
    if (rq_size(ready_q) == 0)
    {
        return NULL;
    }
    
    process_t* selected = rq_pop_front(ready_q);
    
    selected-> next = NULL;
    
    return selected;
}

void enqueue(ready_queue_t* ready_q, process_t* proc, ConfigDataType* configPtr, Boolean init)
{
    assert(proc->cur_exe < proc-> exe_size);
    if (configPtr-> cpuSchedCode == CPU_SCHED_FCFS_N_CODE)
    {
        if (init)
        {
            rq_push_back(ready_q, proc);
        }
        else
        {
            rq_push_front(ready_q, proc);
        }
    }
    else if (configPtr-> cpuSchedCode == CPU_SCHED_SJF_N_CODE)
    {
        if (init)
        {
            rq_insert_cmp_by_pid(ready_q, proc);
        }
        else
        {
            rq_push_front(ready_q, proc);
        }
    }
    else if(configPtr-> cpuSchedCode == CPU_SCHED_FCFS_P_CODE)
    {
        rq_insert_cmp_by_pid(ready_q, proc);
    }
    else if(configPtr-> cpuSchedCode == CPU_SCHED_SRTF_P_CODE)
    {
        rq_insert_cmp_by_remaining_time(ready_q, proc);
    }
    else if(configPtr-> cpuSchedCode == CPU_SCHED_RR_P_CODE)
    {
        rq_push_back(ready_q, proc);
    }
}


void process_end(process_t* proc, ConfigDataType* config)
{
    assert(proc->execution_flow);
    
    char value[BUFSIZ];
    
    
    sprintf(value, "OS: Process %d ended", proc->pid);
    
    output_with_time(value, config);
    
    sprintf(value, "OS: Process %d set to EXIT", proc->pid);
    
    output_with_time(value, config);
    
    free(proc-> execution_flow);
    
    free(proc);
}


Boolean has_interrupt(sim_t* sim)
{
    pthread_mutex_lock(&sim-> mutex);
    
    Boolean ret = rq_size(sim-> interrupt_q) > 0;
    
    pthread_mutex_unlock(&sim->mutex);
    
    return ret;
    
}


void print_interrupt(sim_t* sim, ConfigDataType* config)
{
    char value[BUFSIZ];
    
    process_t* current_process;
    
    pthread_mutex_lock(&sim-> mutex);
    
    if(rq_size(sim-> interrupt_q) > 0)
    {
        current_process = sim-> interrupt_q->tail;
        
        sprintf(value, "OS: Interrupted by process %d, %s %sput operation", current_process->pid,
                current_process-> execution_flow[current_process->cur_exe].origin->strArg1,
                current_process-> execution_flow[current_process->cur_exe].origin->inOutArg);
        
        output_with_time(value, config);
    }
    
    pthread_mutex_unlock(&sim->mutex);
    
}



void handle_interrupt(sim_t* sim, ConfigDataType* config)
{
    char value[BUFSIZ];
    
    process_t* current_process;
    
    pthread_mutex_lock(&sim-> mutex);
    
    while(rq_size(sim-> interrupt_q) > 0)
    {
        current_process = rq_pop_front(sim->interrupt_q);
        
        current_process-> cur_exe++;
        
        if (current_process-> cur_exe < current_process -> exe_size)
        {
            current_process-> state = PROCESS_STATE_READY;
            
            sprintf(value, "OS: Process %d set from BLOCKED to READY", current_process->pid);
            
            output_with_time(value, config);
            
            enqueue(sim->ready_q, current_process, config, False);
            
            sim-> interrupt = True;
            
        }
        else
        {
            sim->num_ended++;
            
            sim->cur_proc = NULL;
            
            process_end(current_process, config);
        }
    }
    
    sim-> interrupt = False;
    
    pthread_mutex_unlock(&sim->mutex);
}


void* block_for_io(void* args)
{
    char value[BUFSIZ];
    
    block_for_io_args_t* arg = args;
    
    ConfigDataType* config = arg->config;
    
    process_t* current_process = arg->proc;
    
    executable_t* exec = &arg-> proc-> execution_flow[arg->proc-> cur_exe];
    
    timer(exec-> time);
    
    current_process-> total_time -= exec-> time;
    
    if (exec->command == DEVIN)
    {
        sprintf(value, "Process: %d, %s input operation end", current_process->pid, exec->origin->strArg1);
        
        output_with_time(value, config);
    }
    else
    {
        sprintf(value, "Process: %d, %s output operation end", current_process->pid, exec->origin->strArg1);
        
        output_with_time(value, config);
    }
    
    pthread_mutex_lock(&arg->sim->mutex);
    
    arg->sim->interrupt = True;
    
    rq_push_back(arg->sim-> interrupt_q, current_process);
    
    pthread_mutex_unlock(&arg->sim->mutex);
    
    free(args);
    
    return NULL;
}

Boolean runProcess(process_t* current_process, sim_t* sim, ConfigDataType* config)
{
    char value[BUFSIZ];
    
    assert(current_process-> cur_exe < current_process->exe_size);
    
    executable_t* exec = &current_process-> execution_flow[current_process->cur_exe];
    
    if (current_process-> state != PROCESS_STATE_RUN)
    {
        sprintf(value, "Process: %d set from READY to RUNNING\n", current_process->pid);
        
        output_with_time(value, config);
        
        current_process-> state = PROCESS_STATE_RUN;
    }
    
    if (exec-> command == CPU)
    {
        sprintf(value, "Process: %d, cpu process operation start", current_process->pid);
        
        output_with_time(value, config);
        
        int quantum = config-> quantumCycles;
        
        Boolean interrupted = False;
        
        while (exec-> time > 0 && quantum > 0)
        {
            timer(config-> procCycleRate);
            
            exec-> time -= config -> procCycleRate;
            current_process->total_time -= config->procCycleRate;
            
            pthread_mutex_lock(&sim->mutex);
            
            if (sim-> interrupt) {
                interrupted = True;
                
                pthread_mutex_unlock(&sim->mutex);
                
                break;
            }
            pthread_mutex_unlock(&sim->mutex);
            
            quantum--;
        }
        
        if (exec-> time == 0)
        {
            current_process->cur_exe++;
        }
        
        if (!interrupted && quantum == 0 && exec-> time > 0)
        {
            output("", config);
            
            sprintf(value, "OS: Process %d quantum time out, cpu process operation end\n", current_process->pid);
            
            output_with_time(value, config);
        }
        else
        {
            sprintf(value, "Process: %d, cpu process operation end\n", current_process->pid);
            
            output_with_time(value, config);
        }
        
        if(interrupted)
        {
            print_interrupt(sim, config);
        }
    }
    
    else if(exec-> command  == DEVIN || exec->command == DEVOUT )
    {
        if (exec-> command == DEVIN)
        {
            sprintf(value, "Process: %d, %s input operation start\n", current_process->pid, exec->origin->strArg1);
            
            output_with_time(value, config);
            
            sprintf(value, "OS: Process %d blocked for input operation", current_process->pid);
            
            output_with_time(value, config);
        }
        else
        {
            sprintf(value, "Process: %d, %s output operation start\n", current_process->pid, exec->origin->strArg1);
            
            output_with_time(value, config);
            
            sprintf(value, "OS: Process %d blocked for output operation", current_process->pid);
            
            output_with_time(value, config);
            
        }
        sprintf(value, "OS: Process %d set from RUNNING to BLOCKED", current_process->pid);
        
        output_with_time(value, config);
        
        current_process -> state = PROCESS_STATE_BLOCKED;
        
        pthread_t pid;
        
        block_for_io_args_t* args = (block_for_io_args_t*)malloc(sizeof(block_for_io_args_t));
        
        assert(args);
        
        args-> sim = sim;
        
        args-> proc = current_process;
        
        args-> config = config;
        
        if (config-> cpuSchedCode == CPU_SCHED_FCFS_N_CODE || config-> cpuSchedCode == CPU_SCHED_SJF_N_CODE)
        {
            block_for_io((void*)args);
        }
        else
        {
            pthread_create(&pid, NULL, block_for_io, (void*)args);
            
            pthread_detach(pid);
        }
        return False;
    }
    else if (exec-> command == MEM)
    {
        if (exec -> strArgs1 == ALLOCATE)
        {
            sprintf(value,"Process: %d, attempting mem allocate request", current_process->pid);
            
            output_with_time(value, config);
        }
        else if(exec->strArgs1 == ACCESS)
        {
            sprintf(value, "Process: %d, attempting mem access request", current_process->pid);
            
            output_with_time(value, config);
        }
        current_process-> cur_exe++;
     }
    else
    {
        current_process->cur_exe++;
    }
    
    pthread_mutex_lock(&sim-> mutex);
     
    sim-> cur_proc = current_process;
    
    pthread_mutex_unlock(&sim-> mutex);
    
    return True;
 }





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
    {
        
    }
    free(args);
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
    if (millisecond > SMALL_QUANTUM)
    {
    pthread_t pid;
    double *ptr = (double *)malloc(sizeof(double));
    *ptr = millisecond;
    // create POSIX thread for simulating the IO operation
    pthread_create(&pid, NULL, msleep, (void *)ptr);
    // join the thread and block it
    pthread_join(pid, NULL);
    }
}

/*
 Function Name: mem_alloca_func
 Algorithm: Simulate the memory allocation operation
 Precondition: Given the address range
 Postcondition: currently, none
 Exception: none
 Notes: none
 */
//Boolean mem_alloca_func(memory_t **mem_list, ConfigDataType *config, int base, int offset, int pid)
//{
//    if (base + offset > config->memAvailable)
//    {
//        //? base start from 0?
//        return False;
//    }
//    memory_t *mem = *mem_list;
//    while (mem)
//    {
//        // go through the memory request list and check if this request overlap woth any existing request
//        if (!((mem->base < base && mem->base + mem->offset < base + offset) || (base < mem->base && base + offset < mem->base + mem->offset)))
//        {
//            return False;
//            // overlap, can't allocate
//        }
//        mem = mem->next;
//        // next node
//    }
//
//    // not overlap, so allocate it, store the request at the head of the list
//    mem = (memory_t *)malloc(sizeof(memory_t));
//    mem->base = base;
//    mem->offset = offset;
//    mem->pid = pid;
//    mem->next = *mem_list;
//    *mem_list = mem;
//    // update the list head
//    return True;
//}

/*
 Function Name: mem_access_func
 Algorithm: Simulate the memory allocation operation
 Precondition: Given the address
 Postcondition: currently, none
 Exception: none
 Notes: none
 */
//Boolean mem_access_func(memory_t *mem_list, int base, int offset, int pid)
//{
//    memory_t *mem = mem_list;
//    while (mem)
//    {
//        // go though the memory request list and check if this request is inside of any allocated request of the process
//        if (mem->base <= base && base + offset <= mem->base + mem->offset)
//        {
//            return pid == mem->pid;
//        }
//        mem = mem->next;
//        // next node
//    }
//    return False;
//    // not allocated
//}
//
/*
 Function Name: runProcess
 Algorithm: master the process and execute the process one by one
 Precondition: given the head pointer of the process configuration and the config data type
 Postcondition: Provide whether should go further running
 Exception: none
 Notes: none
 */
//void runProcess(process_t *current_process, memory_t **mem_list, ConfigDataType *config)
//{
//    int i;
//    char value[1000];
//
//    // app start
//    sprintf(value, "OS: Process %d set from READY to RUNNING\n", current_process->pid);
//    output_with_time(value, config);
//    // set process state
//    current_process->state = PROCESS_STATE_RUN;
//
//    Boolean mem_fault = False;
//    Boolean result;
//    executable_t *exec;
//    for (i = 0; !mem_fault && i < current_process->exe_size; ++i)
//    {
//        exec = &current_process->execution_flow[i];
//
//        // for each process, running from the first execution flow to the end
//        switch (exec->command)
//        {
//        case APP:
//        {
//            break;
//        }
//        case DEVIN:
//        {
//            sprintf(value, "Process: %d, %s intput operation start", current_process->pid, exec->origin->strArg1);
//            output_with_time(value, config);
//
//            timer(exec->time);
//            sprintf(value, "Process: %d, %s input operation end", current_process->pid, exec->origin->strArg1);
//            output_with_time(value, config);
//            break;
//        }
//        case DEVOUT:
//        {
//            sprintf(value, "Process: %d, %s output operation start", current_process->pid, exec->origin->strArg1);
//            output_with_time(value, config);
//
//            timer(exec->time);
//            sprintf(value, "Process: %d, %s output operation end", current_process->pid, exec->origin->strArg1);
//            output_with_time(value, config);
//            break;
//        }
//        case CPU:
//        {
//            sprintf(value, "Process: %d, cpu operation start", current_process->pid);
//            output_with_time(value, config);
//
//            timer(exec->time);
//            sprintf(value, "Process: %d, cpu operation end", current_process->pid);
//            output_with_time(value, config);
//            break;
//        }
//        case MEM:
//        {
//            if (exec->strArgs1 == ALLOCATE)
//            {
//                sprintf(value, "Process: %d, attempting mem allocate request", current_process->pid);
//                output_with_time(value, config);
//                // check whether allocation area can be allocated
//                result = mem_alloca_func(mem_list, config, exec->intArg2, exec->intArg3, current_process->pid);
//                if (result)
//                {
//                    sprintf(value, "Process: %d, successful mem allocate request", current_process->pid);
//                }
//                else
//                {
//                    sprintf(value, "Process: %d, failed mem allocate request", current_process->pid);
//                    mem_fault = True;
//                }
//                output_with_time(value, config);
//                break;
//            }
//            else if (exec->strArgs1 == ACCESS)
//            {
//                sprintf(value, "Process: %d, attempting mem access access request", current_process->pid);
//                output_with_time(value, config);
//                // check whether the position can be accessed
//                result = mem_access_func(*mem_list, exec->intArg2, exec->intArg3, current_process->pid);
//                if (result)
//                {
//                    sprintf(value, "Process: %d, successful mem allocate request", current_process->pid);
//                }
//                else
//                {
//                    sprintf(value, "Process: %d, failed mem allocate request", current_process->pid);
//                    mem_fault = True;
//                }
//                output_with_time(value, config);
//                break;
//            }
//        }
//        case SYS:
//            break;
//        }
//    }
//
//    // app end
//    if (mem_fault)
//    {
//        output("", config);
//        sprintf(value, "OS: Segmentation fault, Process %d ended", current_process->pid);
//        output_with_time(value, config);
//    }
//    else
//    {
//        output("", config);
//        sprintf(value, "OS: Process %d ended", current_process->pid);
//        output_with_time(value, config);
//    }
//    sprintf(value, "OS: Process %d set to EXIT", current_process->pid);
//    output_with_time(value, config);
//    current_process->state = PROCESS_STATE_END;
//}

/*
 Function Name: initial_process
 Algorithm: Initial the process status and output the message, place holder
 Precondition: The process count and the config
 Postcondition: Output the message
 Exception: none
 Notes: none
 */
void initial_process(int non, ConfigDataType *configPtr)
{
    char value[BUFSIZ];
    int index;
    for (index = 0; index < non; ++index)
    {
        sprintf(value, "OS: Process %d set to READY state from NEW state.", index);
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
    int index;
    for (index = 0; index < count; ++index)
    {
        ptr[index] = value;
    }
}


int process_cmp_by_remaining_time(process_t* pro1, process_t* pro2)
{
    return pro1-> total_time - pro2-> total_time;
}

int process_cmp_by_pid(process_t* pro1, process_t* pro2)
{
    return pro1-> pid - pro2-> pid;
}

ready_queue_t* rq_new()
{
    ready_queue_t* ready_q = (ready_queue_t*)malloc(sizeof(ready_queue_t));
    
    ready_q -> head = ready_q -> tail = NULL;
    
    ready_q-> size = 0;
    
    return ready_q;

}

void rq_delete(ready_queue_t* ready_q)
{
    process_t* proc = ready_q->head;
    
    while (proc)
    {
        ready_q->head = proc-> next;
        
        free(proc-> execution_flow);
        
        free(proc);
        
        proc = ready_q-> head;
    }
    
    free(ready_q);
}

void rq_push_back(ready_queue_t* ready_q, process_t* proc)
{
    assert(proc);
    
    proc-> next = NULL;
    
    if (ready_q-> head)
    {
        ready_q-> tail-> next = proc;
        
        ready_q-> tail = proc;
    }
    
    else
    {
        ready_q-> head = ready_q-> tail = proc;
    }
    
    ++ready_q->size;
        }


void rq_push_front(ready_queue_t* ready_q, process_t* proc)
{
    assert(proc);
    
    proc-> next = ready_q->head;
    
    if (ready_q-> head)
    {
        
        ready_q-> head = proc;
    }
    
    else
    {
        ready_q-> head = ready_q-> tail = proc;
    }
    
    ++ready_q->size;
        }



void rq_insert_cmp_by_remaining_time(ready_queue_t* ready_q, process_t* proc)
{

    assert(proc);
    
    if (ready_q->head == NULL || process_cmp_by_remaining_time(proc, ready_q-> head) < 0)
    {
        rq_push_front(ready_q, proc);
    }
    
    else if(process_cmp_by_remaining_time(ready_q-> tail, proc) <= 0)
    {
        rq_push_back(ready_q, proc);
    }
    else
    {
        process_t* proc_node = ready_q->head;
        
        while (proc_node->next)
        {
            if (process_cmp_by_remaining_time(proc, proc_node->next) < 0)
            {
                proc-> next = proc_node-> next;
                
                proc_node-> next = proc;
                
                break;
                
            }
            
            proc_node = proc_node-> next;
        }
        ++ready_q-> size;
    }
}


void rq_insert_cmp_by_pid(ready_queue_t* ready_q, process_t* proc)
{

    assert(proc);
    
    if (ready_q->head == NULL || process_cmp_by_pid(proc, ready_q-> head) < 0)
    {
        rq_push_front(ready_q, proc);
    }
    
    else if(process_cmp_by_pid(ready_q-> tail, proc) <= 0)
    {
        rq_push_back(ready_q, proc);
    }
    else
    {
        process_t* proc_node = ready_q->head;
        
        while (proc_node->next)
        {
            if (process_cmp_by_pid(proc, proc_node->next) < 0)
            {
                proc-> next = proc_node-> next;
                
                proc_node-> next = proc;
                
                break;
                
            }
            
            proc_node = proc_node-> next;
        }
        ++ready_q-> size;
    }
}



process_t* rq_pop_front(ready_queue_t* ready_q)
{
    assert(ready_q->head);
    
    process_t* head = ready_q-> head;
    
    ready_q-> head = head-> next;
    
    if (ready_q-> head == NULL)
    {
        ready_q ->tail = NULL;
    }
    
    --ready_q-> size;
    
    return head;
}

int rq_size(ready_queue_t* ready_q)
{
    
    return ready_q-> size;
}
/*
 Function Name: schedule
 Algorithm: Pick up the next process to run based on the schedule alforithm
 Precondition: The process list and the config
 Postcondition: The next process to run
 Exception: none
 Notes: The process picked up will be removed from process list
 */
//process_t *schedule(process_t **process_list, ConfigDataType *configPtr)
//{
//    if (*process_list == NULL)
//    {
//        return NULL;
//    }
//
//    process_t *selected = NULL;
//    if (configPtr->cpuSchedCode == CPU_SCHED_FCFS_N_CODE)
//    {
//        // FCFS-N: first come first serve
//        selected = *process_list;
//        *process_list = selected->next;
//    }
//    else if (configPtr->cpuSchedCode == CPU_SCHED_SJF_N_CODE)
//    {
//        // SJN-N: Shortest job first
//        process_t *selected_prev = NULL;
//        // previous node of the selected node
//        process_t *prev = NULL;
//        // previous node of proc
//        process_t *proc = *process_list;
//        while (proc)
//        {
//            if (proc->state == PROCESS_STATE_NEW)
//            {
//                if (!selected || proc->total_time < selected->total_time)
//                {
//                    selected = proc;
//                    selected_prev = prev;
//                }
//            }
//            prev = proc;
//            proc = proc->next;
//        }
//        // remove it from the list
//        if (selected_prev)
//        {
//            selected_prev->next = selected->next;
//        }
//        else
//        {
//            *process_list = selected->next;
//        }
//    }
//    return selected; // return the process
//}

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
    char value[BUFSIZ];
    // reset the timer here
    accessTimer(ZERO_TIMER, value);
    // output the simulation start message
    output_with_time("OS: Simulator start", configPtr);

    process_t* proc;
    OpCodeType *ptr = metaDataMsterPtr;
    ready_queue_t* ready_q = rq_new();
    OpCodeType *app_start, *app_end;
    int range = 0;
    int process_cnt = 0;
    int command_index;
    struct OpCodeType* app_ptr;
    executable_t* exec;
    
//    process_t *root_process_list = NULL;
//    process_t *prevProcess = NULL;
//    process_t *process_list;
 
   
    
    // visit all the config pointer and make it into process information
    while (ptr != NULL)
    {
        // sys start and end situation
        if (compareString(ptr->command, "app") == 0)
        {
            // Do nothing!!!
            
            // app start and end
            if (compareString(ptr->strArg1, "start") == 0)
            {
                // set the range size and the pointer
                range = 0;
                app_start = ptr;
            }
            else if (compareString(ptr->strArg1, "end") == 0)
            {
                // set range and the end pointer
    
                app_end = ptr;
                // malloc the initial execution list object and initialze the value to zero
                proc = (process_t *)malloc(sizeof(process_t));
                memset_usr((char *)proc, sizeof(process_t), 0);
                proc->execution_flow = (executable_t *)malloc(sizeof(executable_t) * (range));
                memset_usr((char *)proc->execution_flow, sizeof(executable_t) * (range), 0);
                // set up the size
                proc->exe_size = range;
                proc->cur_exe = 0;
                proc->total_time = 0;
                proc->pid = process_cnt;
                proc-> state = PROCESS_STATE_NEW;

                command_index = 0;
                
//                struct OpCodeType *app_ptr;
                // make the original string type into defined type
                // visit from the first pointer to the end(application range)
                for (app_ptr = app_start->nextNode; app_ptr != app_end; app_ptr = app_ptr->nextNode)
                {
                   exec = &proc->execution_flow[command_index];

                    // int args
                    exec->intArg2 = app_ptr->intArg2;
                    exec->intArg3 = app_ptr->intArg3;
                    exec->origin = app_ptr;
                    
//                    process_list->state = PROCESS_STATE_NEW;

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
                            exec->strArgs1 = END;
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
                        {
                            // memory access
                            exec->strArgs1 = ACCESS;
                        }
                        else if (compareString(app_ptr->strArg1, "allocate") == 0)
                        {
                            // memory allocation
                            exec->strArgs1 = ALLOCATE;
                        }
                        exec->time = 0;
                    }
                    // int args
                    proc->total_time += exec->time;
                    command_index++;
                    
                }
                enqueue(ready_q, proc, configPtr, True);
                
                process_cnt++;
            }
        }
        
        else if (compareString(ptr->command, "sys") != 0)
        //{
//            range++;
//        }
//                // if this is not the first process, then add to the link
//                if (prevProcess != NULL)
//                    prevProcess->next = process_list;
//                else
//                { // else it's the very first process
//                    root_process_list = process_list;
//                }
//                // update the previous process
//                // malloc the initial process list object and initialize the value to zero
//                prevProcess = process_list;
//                process_cnt++;
//            }
//        }
//        else
        {
            range++;
        }
        // set connection with next node
        ptr = ptr->nextNode;
    }
    // output_with_time("OS: System start", configPtr);
    initial_process(process_cnt, configPtr);
    
    sim_t sim;
    sim_init(&sim, ready_q);
    
    Boolean result;
    process_t* selected_proc;
    Boolean is_idle = False;
    
    while(1)
    {
        pthread_mutex_lock(&sim.mutex);
        
        sim.interrupt = False;
        
        if (sim.num_ended == process_cnt)
        {
            pthread_mutex_unlock(&sim.mutex);
            break;
        }
        
        selected_proc = schedule(ready_q, configPtr);
        
        if (selected_proc)
        {
            if (sim.cur_proc != selected_proc)
            {
                sprintf(value, "OS: Process %d selected with %d ms remaining", selected_proc->pid, selected_proc->total_time);
                output_with_time(value, configPtr);
            }
            pthread_mutex_unlock(&sim.mutex);
            
            result = runProcess(selected_proc, &sim, configPtr);
            
            if (result == True)
            {
                if (selected_proc -> cur_exe < selected_proc -> exe_size)
                {
                    pthread_mutex_lock(&sim.mutex);
                    
                    enqueue(sim.ready_q, selected_proc, configPtr, False);
                    pthread_mutex_unlock(&sim.mutex);
                }
                else
                {
                    sim.cur_proc = NULL;
                    
                    sim.num_ended++;
                    
                    process_end(selected_proc, configPtr);
                }
            }
            is_idle = False;
        }
        else
        {
            if (!is_idle) {
                output_with_time("OS: CPU idle, all active processes blocked", configPtr);
                timer(1);
                
                is_idle = True;
            }
            
            pthread_mutex_unlock(&sim.mutex);
        }
        if (is_idle && has_interrupt(&sim))
        {
            output_with_time("OS: CPU interrupt, end idle", configPtr);
            
            print_interrupt(&sim, configPtr);
        }
        
        handle_interrupt(&sim, configPtr);
    }
    assert(sim.num_ended == process_cnt);
    
    output_with_time("OS: System stop", configPtr);
    
    output_with_time("OS: System end", configPtr);
    
    if (configPtr-> logToCode == LOGTO_BOTH_CODE || configPtr-> logToCode == LOGTO_FILE_CODE )
    {
        FILE *fp = fopen(configPtr->logToFileName, "a+");
        fprintf(fp, "\nEnd Simulation - Complete\n");
        fprintf(fp, "=========================\n\n");
        fclose(fp);
    }
    
    sim_destroy(&sim);
}
     
    
void sim_init(sim_t* sim, ready_queue_t* ready_q)
{
    
    sim-> num_ended = 0;
    
    sim-> cur_proc = NULL;
    
    sim->ready_q = ready_q;
    
    sim->interrupt_q = rq_new();
    
    sim->interrupt = False;
    
    pthread_mutex_init(&sim->mutex, NULL);
}

void sim_destroy(sim_t* sim)
{
    rq_delete(sim->ready_q);
    rq_delete(sim->interrupt_q);
    pthread_mutex_destroy(&sim->mutex);
}

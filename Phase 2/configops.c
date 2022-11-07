// header file
#include "configops.h"

/*
Function Name: clearConfigData
Algorithim: frees allocated memory for config fata
Precondition: configData hold allocated data set
Postcondition: memory is freed, pointer is set to NULL
Exception: none
Notes: none
*/

ConfigDataType *clearConfigData(ConfigDataType *configData)
{
  // check that config data pointer is not null
  if (configData != NULL)
  {
    // free data structure memory
    // function: free
    free(configData);
    // set config data to null
    configData = NULL;
  }
  // set config data pointer to null
  return NULL;
}

/*
Function name: configCodeToString
Algorithim: Utitliy function to support display
            of CPU scheduling or Log to code strings
Precondition: code variable holds constant value from Config Data Codes for item
Postcondition: string parameter holds correct string associated with the given contant
Exception: none
Notes: none
*/

void configCodeToString(int code, char *outString)
{
  char displayStrings[8][10] = {"SJF-N", "SRTF-P", "FCFS-P",
                                "RR-P", "FCFS-N", "Monitor",
                                "File", "Both"};

  // copy string to return parameter
  // function copy string
  copyString(outString, displayStrings[code]);
}

/*
  function Name: displayConfigData
  Algorithim: diagnostic function to show config data output
  Precondition: configData holds allocated data set
  Postcondition:data is printed to screen
  Execeptions: none
  Notes: none

*/
void displayConfigData(ConfigDataType *configData)
{
  // initialzie function/variables
  char displayString[STD_STR_LEN];

  // print lines of display
  // function: printf configCode TOString

  printf("Config File Display\n");
  printf("-----------------\n");
  printf("Version           : %3.2f\n", configData->version);
  printf("Program file name : %s\n", configData->metaDataFileName);
  configCodeToString(configData->cpuSchedCode, displayString);
  printf("CPU schedule selecetion: %s\n", displayString);
  printf("Quantum time       : %d\n", configData->quantumCycles);
  printf("Memory Available  : %d\n", configData->memAvailable);
  printf("Process cycle rate: %d\n", configData->procCycleRate);
  printf("I/O cycle rate    : %d\n", configData->ioCycleRate);
  configCodeToString(configData->logToCode, displayString);
  printf("Log to selection  : %s\n", displayString);
  printf("Log file name     : %s\n\n", configData->logToFileName);
}

/*
  Function Name; getConfigData
  Algorithm: opens file, acquires configuration data,
  returns poiunter to data structure
  Precondition: For correct operation, file is availble, is formatted correctly
  and has all configuration lines and data although
  the configuration lines are not required to be in specific order
  Postcondition: in correct operation, returns pointer to correct configuration data structure
  Exceptions: correctly and appropriately(without program failure)
  response to and reports file access failure

*/

Boolean getConfigData(char *fileName, ConfigDataType **configData, char *endStateMsg)
{
  // initialize function / variables

  // set constant number of data lines
  const int NUM_DATA_LINES = 9;

  // set read only constant
  const char READ_ONLY_FLAG[] = "r";

  // create pointer for data input
  ConfigDataType *tempData;

  // declare other vairables
  FILE *fileAccessPtr;
  char dataBuffer[MAX_STR_LEN];
  int intData, dataLineCode, lineCtr = 0;
  double doubleData;
  Boolean dontStopAtNonPrintable = False;

  // set sendStateMsg to success
  //  function: copyString

  copyString(endStateMsg, "Configuration file upload successful");

  // initalize config data pointer in case of return error
  *configData = NULL;

  // open file
  //  function: fopen
  fileAccessPtr = fopen(fileName, READ_ONLY_FLAG);

  // check for file open failure

  if (fileAccessPtr == NULL)
  {
    // set end state message to config file access error
    // function: copyString
    copyString(endStateMsg, "Configuration file access error");

    return False;
  }

  // get first line, check for failure
  // function: getLineTo

  if (getLineTo(fileAccessPtr, MAX_STR_LEN, COLON, dataBuffer, IGNORE_LEADING_WS, dontStopAtNonPrintable) != NO_ERR || compareString(dataBuffer, "Start Simulator Configuration File") != STR_EQ)
  {
    // close file access
    // function: fclsoe
    fclose(fileAccessPtr);

    // set end state message to corrup leader line error
    //  function: copyString

    copyString(endStateMsg, "Corrupt configuration leader liner error");

    return False;
  }

  // create temp pointer to config data struct

  tempData = (ConfigDataType *)malloc(sizeof(ConfigDataType));

  while (lineCtr < NUM_DATA_LINES)
  {
    // get line leader, check for failure
    // function: getLineTo

    if (getLineTo(fileAccessPtr, MAX_STR_LEN, COLON, dataBuffer, IGNORE_LEADING_WS, dontStopAtNonPrintable) != NO_ERR)
    {
      // free temp struct memory
      // function: free
      free(tempData);

      // close file access
      // function: fclose
      fclose(fileAccessPtr);

      // set end state message to line capture failure
      // function: copyString
      copyString(endStateMsg, "Configuration start line capture error");

      // return incomplete file error
      return False;
    }

    // find correct data line by number
    // function: getDataLineCode
    dataLineCode = getDataLineCode(dataBuffer);

    // check for data line found
    if (dataLineCode != CFG_CORRUPT_PROMT_ERR)
    {
      // get data value
      // check for cersion number (double value)
      if (dataLineCode == CFG_VERSION_CODE)
      {
        // get version number
        // function: fscanf
        fscanf(fileAccessPtr, "%lf", &doubleData);
      }
      // otherwise, check for metaData or LogTo file names
      // or CPU scheduling names (strings)
      else if (dataLineCode == CFG_MD_FILE_NAME_CODE ||
               dataLineCode == CFG_LOG_FILE_NAME_CODE ||
               dataLineCode == CFG_CPU_SCHED_CODE ||
               dataLineCode == CFG_LOG_TO_CODE)
      {
        fscanf(fileAccessPtr, "%s", dataBuffer);
      }
      else // otherwise assume integer data
      {
        // get integer input
        // function: fscanf
        fscanf(fileAccessPtr, "%d", &intData);
      }
      if (valueInRange(dataLineCode, intData, doubleData, dataBuffer) == True)
      {
        // assign to data pointer depending on config claue
        // function: getCpuSchedCode, getLogToCode
        switch (dataLineCode)
        {
        case CFG_VERSION_CODE:
          tempData->version = doubleData;
          break;
        case CFG_MD_FILE_NAME_CODE:
          copyString(tempData->metaDataFileName, dataBuffer);
          break;
        case CFG_CPU_SCHED_CODE:
          tempData->cpuSchedCode = getCpuSchedCode(dataBuffer);
          break;
        case CFG_QUANT_CYCLES_CODE:
          tempData->quantumCycles = intData;
          break;
        case CFG_MEM_AVAILABLE_CODE:
          tempData->memAvailable = intData;
          break;
        case CFG_PROC_CYCLES_CODE:
          tempData->procCycleRate = intData;
          break;
        case CFG_IO_CYCLES_CODE:
          tempData->ioCycleRate = intData;
          break;
        case CFG_LOG_TO_CODE:
          tempData->logToCode = getLogToCode(dataBuffer);
          break;
        case CFG_LOG_FILE_NAME_CODE:
          copyString(tempData->logToFileName, dataBuffer);
          break;
        }
      }
      else
      {
        // free temp struct memory
        // function : free
        free(tempData);

        // close file
        fclose(fileAccessPtr);

        // set end state message to config data
        // function copyStirng
        copyString(endStateMsg, "Configuration item out of range");

        // Return data out of range LINE 309
        return False;
      }
    }
    // adding
    else
    {
      // free temp struct memory
      // function: free
      free(tempData);
      // close file access
      // function: fclose
      fclose(fileAccessPtr);

      // set end state message to configuration corrupt prompt error
      // function: copyString
      copyString(endStateMsg, "Corrupted congiuration prompt");

      // return corrupt config file name
      return False;
    }
    // increment line counter
    lineCtr++;
  }
  // end master loop.
  // acquire end of sim config string
  // function: getLineTo
  if (getLineTo(fileAccessPtr, MAX_STR_LEN, PERIOD,
                dataBuffer, IGNORE_LEADING_WS, dontStopAtNonPrintable) != NO_ERR ||
      compareString(dataBuffer, "End Simulator Configuration File") != STR_EQ)
  {

    // Free temp struct memory
    // function: free
    free(tempData);
    // Close file access.
    // function: fclose
    fclose(fileAccessPtr);
    // set end state message to corrupt configuration end line
    // function: copyString
    copyString(endStateMsg, "Configuration end line capture error");
    // return correct file data
    return False;
  }

  // assign temporary pointer to parameter return pointer
  *configData = tempData;
  // close file access.
  // function: fclose.
  fclose(fileAccessPtr);
  // return no error code
  return True;
}

/*
Function Name: getCpuSchedCode
Algorithm: converts string data (e.g., "SJF-N ", "SRTF-P")
to constant code number to be stored as integer
Precondition: codeStr is a C-Style string with one.
of the specified cpu scheduling operations
Postcondition: returns code representing scheduling actions
Exceptions: defaults to fcfs code.
Notes: none
*/

ConfigDataCodes getCpuSchedCode(char *codeStr)
{
  // initialize function/variables
  // set up temporary string for testing
  // fucntion: getString Length, malloc
  int strlen = getStringLength(codeStr);
  char *tempStr = (char *)malloc(strlen + 1);
  // set default return to FCFS-N
  int returnVal = CPU_SCHED_FCFS_N_CODE;
  // set string to lower case for testing
  // function: setStrToLowerCase
  setStrToLowerCase(tempStr, codeStr);
  // check for SJF-N
  // function: compareString
  if (compareString(tempStr, "sjf-n") == STR_EQ)
  {
    // set return code to sjf
    returnVal = CPU_SCHED_SJF_N_CODE;
  }
  // check for SRTF-P
  // function: compareString
  if (compareString(tempStr, "srtf-p") == STR_EQ)
  {
    // set return code to srtf
    returnVal = CPU_SCHED_SRTF_P_CODE;
  }
  // check for FCFS-P
  // function: compareString
  if (compareString(tempStr, "fcfs-p") == STR_EQ)
  {
    // set return codd to fcfs.
    returnVal = CPU_SCHED_FCFS_P_CODE;
  }
  // check for RR-P
  // function: compareString
  if (compareString(tempStr, "rr-p") == STR_EQ)
  {
    // set return code to rr
    returnVal = CPU_SCHED_RR_P_CODE;
  }
  // free temp string memory
  // function: free
  free(tempStr);
  // return resulting value
  return returnVal;
}

int getDataLineCode(char *dataBuffer)
{
  // return appropriate code depending on prompt string provided
  // function: compareString
  if (compareString(dataBuffer, "Version/Phase") == STR_EQ)
  {
    return CFG_VERSION_CODE;
  }
  if (compareString(dataBuffer, "File Path") == STR_EQ)
  {
    return CFG_MD_FILE_NAME_CODE;
  }
  if (compareString(dataBuffer, "CPU Scheduling Code") == STR_EQ)
  {
    return CFG_CPU_SCHED_CODE;
  }
  if (compareString(dataBuffer, "Quantum Time (cycles)") == STR_EQ)
  {
    return CFG_QUANT_CYCLES_CODE;
  }
  if (compareString(dataBuffer, "Memory Available (KB)") == STR_EQ)
  {
    return CFG_MEM_AVAILABLE_CODE;
  }
  if (compareString(dataBuffer, "Processor Cycle Time (msec)") == STR_EQ)
  {
    return CFG_PROC_CYCLES_CODE;
  }
  if (compareString(dataBuffer, "I/O Cycle Time (msec)") == STR_EQ)
  {
    return CFG_IO_CYCLES_CODE;
  }
  if (compareString(dataBuffer, "Log To") == STR_EQ)
  {
    return CFG_LOG_TO_CODE;
  }
  if (compareString(dataBuffer, "Log File Path") == STR_EQ)
  {
    return CFG_LOG_FILE_NAME_CODE;
  }

  return CFG_CORRUPT_PROMT_ERR;
}

ConfigDataCodes getLogToCode(char *logToStr)
{
  // initialize function/variables.
  // create temporary string
  // function: getStringLength, malloc.
  int strlen = getStringLength(logToStr);
  char *tempStr = (char *)malloc(strlen + 1);
  // set default to log to monitor
  int returnVal = LOGTO_MONITOR_CODE;
  // set temp string to lower case.
  // function: setStrToLowerCase
  setStrToLowerCase(tempStr, logToStr);
  // Check for BOTH
  // function: compareString
  if (compareString(tempStr, "both") == STR_EQ)
  {
    // set return calue to both code
    returnVal = LOGTO_BOTH_CODE;
  }
  // Check for FILE
  // function: compareString
  if (compareString(tempStr, "file") == STR_EQ)
  {
    // set return value to file code
    returnVal = LOGTO_FILE_CODE;
  }
  // Free temp string memory
  // function: free
  free(tempStr);
  // return found code
  return returnVal;
}

Boolean valueInRange(int lineCode, int intVal, double doubleVal, char *stringVal)
{
  // initialize function/ variables.
  Boolean result = True;
  char *tempStr;
  int strlen;
  // use line code to identify prompt line
  switch (lineCode)
  {
  // for version code.
  case CFG_VERSION_CODE:
    // check if limits of version code are exceeded
    if (doubleVal < 0.00 || doubleVal > 10.0)
    {
      // set Boolean result to false
      result = False;
    }
    break;
    // for cpu scheduling code
  case CFG_CPU_SCHED_CODE:
    // create temporary string and set to lower case
    // function: getStringLength, malloc, setStrToLowerCase
    strlen = getStringLength(stringVal);
    tempStr = (char *)malloc(strlen + 1);
    setStrToLowerCase(tempStr, stringVal);
    // check for not finding one of the scheduling strings.
    // function: compareString
    if (compareString(tempStr, "fcfs-n") != STR_EQ && compareString(tempStr, "sjf-n") != STR_EQ && compareString(tempStr, "srtf-p") != STR_EQ && compareString(tempStr, "fcfs-p") != STR_EQ && compareString(tempStr, "rr-p") != STR_EQ)
    {
      // set boolean result to false
      result = False;
    }
    // free temp string memory
    // function: free
    free(tempStr);

    break;
    // for quantum cycles.
  case CFG_QUANT_CYCLES_CODE:
    // check for quantum cycles limits exceeded
    if (intVal < 0 || intVal > 100)
    {
      // set boolean result to false.
      result = False;
    }
    break;
    // for memory avaible
  case CFG_MEM_AVAILABLE_CODE:
    // check for available memory limits exceeded
    if (intVal < 1024 || intVal > 102400)
    {
      // set Boolean result to false
      result = False;
    }
    break;
    // check for process cycles
  case CFG_PROC_CYCLES_CODE:
    // check for process cycles limits exceeded
    if (intVal < 1 || intVal > 100)
    {

      // set boolean result to False
      result = False;
    }
    break;
    // check for I/0 cycles.
  case CFG_IO_CYCLES_CODE:
    // check for I/O cycles limits exceeded
    if (intVal < 1 || intVal > 1000)
    {
      // set boolean result to False
      result = False;
    }
    break;
    // check for Log to operation
  case CFG_LOG_TO_CODE:
    // create temporary string and set to lower set to lower case.
    // function: getStringLength, malloc, setStrToLowerCase
    strlen = getStringLength(stringVal);
    tempStr = (char *)malloc(strlen + 1);
    setStrToLowerCase(tempStr, stringVal);
    // check for not finding one of the log to strings
    // function: compareString
    if (compareString(tempStr, "both") != STR_EQ && compareString(tempStr, "monitor") != STR_EQ && compareString(tempStr, "file") != STR_EQ)
    {
      // set Boolean result to false
      result = False;
    }
    // free temp string memory
    // function: free
    free(tempStr);
    break;
  }
  // return result of limits analysis.
  return result;
}

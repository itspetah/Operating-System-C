
#ifndef StringUtils_h
#define StringUtils_h

// Header files
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

// GLOBAL CONSTANTS - may be used in other files
typedef enum
{
	False,
	True
} Boolean;

// Config data structure
typedef struct ConfigDataType
{
	double version;
	char metaDataFileName[100];
	int cpuSchedCode;
	int quantumCycles;
	int memAvailable;
	int procCycleRate;
	int ioCycleRate;
	int logToCode;
	char logToFileName[100];
} ConfigDataType;

typedef struct OpCodeType
{
	int pid;
	char command[5];
	char inOutArg[5];
	char strArg1[15];
	int intArg2;
	int intArg3;

	double opEndTime;
	struct OpCodeType *nextNode;
} OpCodeType;

typedef enum
{
	NO_ERR,
	INCOMPLETE_FILE_ERR,
	INPUT_BUFFER_OVERRUN_ERR,
	NON_PRINTABLE_CHAR,
} StringManipCode;

extern const char NULL_CHAR;
extern const char SPACE;
extern const char COLON;
extern const char COMMA;
extern const char SEMICOLON;
extern const char PERIOD;
extern const int STD_STR_LEN;
extern const int MAX_STR_LEN;
extern const int STR_EQ;
extern const int SUBSTRING_NOT_FOUND;
extern const Boolean IGNORE_LEADING_WS;
extern const Boolean ACCEPT_LEADING_WS;

// Function prototypes
int compareString(const char *oneStr, const char *otherStr);
void concatenateString(char *destStr, const char *sourceStr);
void copyString(char *destStr, const char *sourceStr);
int findSubString(const char *testStr, const char *searchSubStr);
int getStringLength(const char *testStr);
void getSubString(char *destStr, const char *sourceStr, int startIndex, int endIndex);
void setStrToLowerCase(char *destStr, const char *sourceStr);
char toLowerCase(char testChar);
Boolean isEndOfFile(FILE *filePtr);
int getLineTo(FILE *filePtr, int bufferSize, char stopChar,
		    char *buffer, Boolean omitLeadingWhiteSpace, Boolean stopAtNonPrintable);
bool getStringConstrained(
    FILE *inStream,
    char clearLeadingNonPrintable,
    bool clearLeadingSpace,
    bool stopAtNonPrintable,
    char delimiter,
    char *capturedString);
bool getStringToDelimiter(
    FILE *inStream,
    char delimiter,
    char *capturedString);
bool getStringToLineEnd(
    FILE *inStream,
    char *capturedString);

#endif /* StringUtils_h */
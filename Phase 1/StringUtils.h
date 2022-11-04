#ifndef STRING_UTILS_H
#define STRING_UTILS_H

// Header files
#include <stdio.h> //for malloc
#include <stdbool.h>
#include <stdlib.h> //for file operations
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

typedef struct OpCOdeType
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
int getLineTo(FILE *filePtr, int bufferSize, char stopChar, char *buffer, Boolean omitLeadingWhiteSpace, Boolean stopAtNonPrintable);
Boolean isEndOfFile(FILE *filePtr);
int getStringLength(const char *teststr);
void copyString(char *destStr, const char *sourceStr);
void concatenateString(char *destStr, const char *sourceStr);
int compareString(const char *oneStr, const char *otherStr);
void getSubString(char *destStr, const char *sourceStr, int startIndex, int endIndex);

int findSubString(const char *testStr, const char *searchSubStr);
void setStrToLowerCase(char *destStr, const char *sourceStr);
char toLowerCase(char testChar);
bool getStringConstrained(
    FILE *inStream,
    bool clearLeadingNonPrintable,
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

#endif // STRING_UTILS_H
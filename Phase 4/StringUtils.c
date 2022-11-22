#include "StringUtils.h"
#include <stdbool.h>
#include "configops.h"
#include "simtimer.h"

// constant delaration
const char NULL_CHAR = '\0';
const char SPACE = ' ';
const char COLON = ':';
const char COMMA = ',';
const char SEMICOLON = ';';
const char PERIOD = '.';
const char LEFT_PAREN = '(';
const char RIGHT_PAREN = ')';
const int SMALL_STR_LEN = 30;
const int STD_STR_LEN = 60;
const int MAX_STR_LEN = 200;
const int STR_EQ = 0;
const int SUBSTRING_NOT_FOUND = -1001;
const Boolean IGNORE_LEADING_WS = True;
const Boolean ACCEPT_LEADING_WS = False;

/*
Function Name: compareSTring
Process: compares two string with the following results
            if left string less than right string, return less than zero
            if left string greater than right string, return greter than zero
            if left string equals right string, return zero
            - equals text includes length
Function Input/Parameters: c-style left and right string (char *)
Function Output/Parameters: none
Finction Output/Returned: result as specified (int)
Device Input/Keyboard: none
Device Output/Montior: none
Dependencies: getStringLength
*/
int compareString(char *oneStr, char *otherStr)
{
    // initialize function/variables
    int difference, index = 0;

    // loop to end of one of the two strings
    // loop limited to MAX_STR_LEN
    while (index < MAX_STR_LEN && oneStr[index] != NULL_CHAR && otherStr[index] != NULL_CHAR)
    {
        // find the difference between the currently aligned characters
        difference = oneStr[index] - otherStr[index];

        // check for non-zero difference
        if (difference != 0)
        {
            // return non-zero difference
            return difference;
        }

        // increment index
        index++;
    }

    // assume strings are equal to this point, return string length difference
    // function: getStringLength
    return getStringLength(oneStr) - getStringLength(otherStr);
}

/*
Function Name: concatenateString
Process: appedns one string onto another
Function Input/Parameters: c-style source string (char *)
Function Output/Parameters: c-stule destination string (char *)
Finction Output/Returned: none
Device Input/Keyboard: none
Device Output/Montior: none
Dependencies: getStringLength
*/
void concatenateString(char *destination, char *source)
{
    // initialize function/variables

    // set destination index
    // funciton: getStringLength
    int destIndex = getStringLength(destination);

    // set source index to zero
    int sourceIndex = 0;

    // loop to end of source index
    // loop limited to MAX_STR_LEN
    while (destIndex < MAX_STR_LEN && source[sourceIndex] != NULL_CHAR)
    {
        // assign source character to destination at destination index
        destination[destIndex] = source[sourceIndex];

        // increment source and destination indices
        sourceIndex++;
        destIndex++;

        // assign null character to next destination element
        destination[destIndex] = NULL_CHAR;
    }
}

/*
Function Name: copyString
Process: compares copies one string into another,
         overwriting data in teh destination string
Function Input/Parameters: c-style source string (char *)
Function Output/Parameters: c-style destination string (char *)
Finction Output/Returned: none
Device Input/Keyboard: none
Device Output/Montior: none
Dependencies: getStringLength
*/
void copyString(char *destination, char *source)
{
    // initialize function/variables
    int index = 0;

    // loop until null character is found in source string
    // loop limited to MAX_StR_LEN
    while (index < MAX_STR_LEN && source[index] != NULL_CHAR)
    {
        // ASSIGN SOURCE CHARACTER TO DESTINATION ELEMENT
        destination[index] = source[index];

        // increment index
        index++;

        // assign null character to next destination element
        destination[index] = NULL_CHAR;
    }
    // end loop
}

/*
Function Name: findSubString
Process: compares linear search for given substring within another string
Function Input/Parameters: c-style source test string (char *)
                           c-style source search string (char *)
Function Output/Parameters: none
Finction Output/Returned: index of found substring, or SUBSTRING_NOT_FOUND constant if string not found
Device Input/Keyboard: none
Device Output/Montior: none
Dependencies: getStringLength
*/
int findSubString(char *testStr, char *searchSubStr)
{
    // initialize function/variables

    // intitialize test string length
    // function: getStringLength
    int testStrLen = getStringLength(testStr);

    // INITIALIZE OTHER VARIABLES
    int searchIndex, internalIndex, masterIndex = 0;

    // loop across test string
    while (masterIndex < testStrLen)
    {
        // set internal loop index to current test string index
        internalIndex = masterIndex;

        // set internal search index to zero
        searchIndex = 0;

        // loop to end of test string
        // while test string/sub string characters are the same
        while (internalIndex <= testStrLen && testStr[internalIndex] == searchSubStr[searchIndex])
        {
            // increment test string, substring indices
            internalIndex++;
            searchIndex++;

            // check for end of substring (search completed)
            if (searchSubStr[searchIndex] == NULL_CHAR)
            {
                // return beginning location of sub string
                return masterIndex;
            }
        }
        // end internal comparison loop

        // increment current beginning location index
        masterIndex++;
    }
    // end loop acrross test string

    // assume test have failed at this point, return SUBSTRING_NOT_FOUND
    return SUBSTRING_NOT_FOUND;
}

/*
Function Name: getStringConstrained
Process: capture a string from teh input stream
        to a specified delimiter
        Note: consumes delimiter
Function Input/Parameters: input stream( FILE *)
                            stops at specified delimiter (char),
Function Output/Parameters: string returned (char *)
Finction Output/Returned: success of operation (bool)
Device Input/Keyboard: none
Device Output/Montior: none
Dependencies: fgetc
*/
bool getStringConstrained(
    FILE *inStream,
    char clearLeadingNonPrintable,
    bool clearLeadingSpace,
    bool stopAtNonPrintable,
    char delimiter,
    char *capturedString)
{
    // initialize variables
    int intChar = EOF, index = 0;

    // initialize output string
    capturedString[index] = NULL_CHAR;

    // capture first value in stream
    // function: fgetc
    intChar = fgetc(inStream);

    // loop to clear non printable or space, if indicated
    while ((intChar != EOF) && ((clearLeadingNonPrintable && intChar == (int)SPACE) || (clearLeadingNonPrintable && intChar == (int)SPACE)))
    {
        // get next character
        // function: fgetc
        intChar = fgetc(inStream);
    }
    // end clear non printable/space loop

    // check for end of file found
    if (intChar == EOF)
    {
        // return failed operation
        return false;
    }

    // loop to capture input
    while (
        // continues if not at end of file and max string length not reached
        (intChar != EOF && index < MAX_STR_LEN - 1)

        // AND
        // continues if not printable flag set and characters are printable
        // OR continues if not printable flag not set
        && ((stopAtNonPrintable && intChar >= (int)SPACE) || (!stopAtNonPrintable))

        // AND
        // continues if specified delimiter is not found
        && (intChar != (char)delimiter))
    {

        // place character in array element
        capturedString[index] = (char)intChar;

        // increment array index
        index++;

        // set next element to null character / end of c-string
        capturedString[index] = NULL_CHAR;

        // get next character as integer
        // function: fgetc
        intChar = fgetc(inStream);
    }
    // end loop

    // return successful operation
    return true; // temporary stub return
}

/*
Name: getStringLength
Process: finds the length of a string by counting character up to the NULL_CHAR character
Function Input/Parameters: c-style string (char *)
Function Output/Parameters: none
Function Output/Returned: length of string
Device Input/Keyboard: none
Device Output/Monitor: none
Dependencies: none
*/
int getStringLength(char *str)
{
    // initialize function/variables
    int index = 0;

    // loop to end of string, protect from overflow
    while (str[index] != NULL_CHAR)
    {
        // increment index
        index++;
    }
    // end loop

    // return index/length
    return index;
}

/*
Name: getStringToDelimiter
Process: captures a string from the input stream
        to a specified delimiter;
        Note: consumes delimiter
Function Input/Parameters: int stream(FILE *)
                           stops at specified delimiter (char),
Function Output/Parameters: string returned (char *)
Function Output/Returned: success of operation (bool)
Device Input/Keyboard: none
Device Output/Monitor: none
Dependencies: getStringConstrained
*/
bool getStringToDelimiter(
    FILE *inStream,
    char delimiter,
    char *capturedString)
{
    // call engine function with delimiter
    // function: getStringConstrained
    return getStringConstrained(
        inStream,      // file stream pointer
        true,          // clears leading non printable character
        true,          // bool clearLeadingSpace,
        true,          // stops at non printable
        delimiter,     // stops at delimiter
        capturedString // returns string
    );
}

/*
Name: getSubString
Process: captures sub string within larger string
         between two inclusive indices.
         returns empty string if either index is out of range,
         assumes enough memory in destination string
Function Input/Parameters: c-style source string (char *), start and end indices (int)
Function Output/Parameters: c-style destination string (char *)
Function Output/Returned: none
Device Input/Keyboard: none
Device Output/Monitor: none
Dependencies: getStringLength, malloc, copyString, free
*/
void getSubString(char *destStr, char *sourceStr, int startIndex, int endIndex)
{
    // initialize function/variables

    // set length of source string
    // function: getStringLength
    int sourceStrLen = getStringLength(sourceStr);

    // initialize the destination index to zero
    int destIndex = 0;

    // initialize source index to start index (parameter)
    int sourceIndex = startIndex;

    // create pointer for temp string
    char *tempSourceStr;

    // check for indices within limits
    if (startIndex >= 0 && endIndex >= startIndex && endIndex < sourceStrLen)
    {
        // creat temporary string
        // function: malloc, copyString
        tempSourceStr = (char *)malloc(sourceStrLen + 1);
        copyString(tempSourceStr, sourceStr);

        // loop across requested substring (indices)
        // loop limited to MAX_STR_LEN
        while (destIndex < MAX_STR_LEN && sourceIndex <= endIndex)
        {
            // assign source character to destination element
            destStr[destIndex] = tempSourceStr[sourceIndex];

            // increment indices
            destIndex++;
            sourceIndex++;

            // add null charactter to next destination string element
            destStr[destIndex] = NULL_CHAR;
        }
        // end loop

        // return memory for temporary string
        // function: free
        free(tempSourceStr);
    }
    // otherwise, assuem indices outside limits
    else
    {
        // create empty string with null character
        destStr[0] = NULL_CHAR;
    }
}

/*
Name: setStrToLowerCase
Process: iterates through string, sets any upper case letter to lower case;
         no effect on other letter
Function Input/Parameters: c-style source string (char *)
Function Output/Parameters: c-style destination string (char *)
Function Output/Returned: none
Device Input/Keyboard: none
Device Output/Monitor: none
Dependencies: toLowerCase
*/
void setStrToLowerCase(char *destStr, char *sourceStr)
{
    // initializer function/variables

    // create temporary source string
    // function: getStringLength, malloc
    int strLen = getStringLength(sourceStr);
    char *tempStr = (char *)malloc(strLen + 1);

    // initialize source string index to zero
    int index = 0;

    // copy source string to temp
    // function: copyString
    copyString(tempStr, sourceStr);

    // loop to end of temp/source string
    // loop limited to MAX_STR_LEN
    while (index < MAX_STR_LEN && tempStr[index] != NULL_CHAR)
    {
        // change letter to lower case as need ans assign
        // to destination string
        // function: setCharTOlowercase
        destStr[index] = setCharToLowerCase(tempStr[index]);

        // increment index
        index++;

        // add null character to next destination string element
        destStr[index] = NULL_CHAR;
    }

    // release temp string memory to OS
    // function: free
    free(tempStr);
}

/*
Name: setCharToLowerCase
Process: if character is upper case, sets it to lower case;
    otherwise returns character unchanged.
Function Input/Parameters: test character (char)
Function Output/Parameters: none
Function Output/Returned: character to set to lowercase, if appropriate
Device Input/Keyboard: none
Device Output/Monitor: none
Dependencies: none
*/
char setCharToLowerCase(char inputChar)
{
    // check for upper case letter
    if (inputChar >= 'A' && inputChar <= 'Z')
    {
        // return lower case letter
        inputChar = inputChar - 'A' + 'a';
    }

    // otherwise, assume no upper case letter,
    // return character unchanged
    return inputChar;
}

int getLineTo(FILE *filePtr, int bufferSize, char stopChar,
              char *buffer, Boolean omitLeadingWhiteSpace, Boolean stopAtNonPrintable)
{
    // initialize function/variables
    int charAsInt, charIndex = 0;
    int statusReturn = NO_ERR;
    Boolean bufferSizeAvailable = True;
    Boolean nonPrintableFound = False;

    // get first character( as integer)
    // function: fgetc
    charAsInt = fgetc(filePtr);

    // consum leading white space, if flagged
    while (omitLeadingWhiteSpace == True && charAsInt != (int)stopChar && charIndex < bufferSize && charAsInt <= (int)SPACE)
    {
        // get next character (as integer)
        // function: fgetc
        charAsInt = fgetc(filePtr);
    }

    // check for stop at non printable if needed
    if (stopAtNonPrintable == True && charAsInt < (int)SPACE)
    {
        // set non printable flag to true
        nonPrintableFound = True;
    }

    // capture string
    // lopp while character is not stop character and buffer size available
    while (charAsInt != (int)stopChar && nonPrintableFound == False && bufferSizeAvailable == True)
    {
        // check for input failure
        // function: isEndOfFile
        if (isEndOfFile(filePtr) == True)
        {
            // return incomplete file error
            return INCOMPLETE_FILE_ERR;
        }

        // check for usable (printable) character
        if (charAsInt >= (int)SPACE)
        {
            // assign input character to buffer string
            buffer[charIndex] = (char)charAsInt;

            // increment index
            charIndex++;
        }

        // set next buffer element to null character
        buffer[charIndex] = NULL_CHAR;

        // check for not at end of buffer size
        if (charIndex < bufferSize - 1)
        {
            // get a new character
            charAsInt = fgetc(filePtr);

            // check for stop at non printable if needed
            if (stopAtNonPrintable == True && charAsInt < (int)SPACE)
            {
                // set non printable flag to true
                nonPrintableFound = True;
            }
        }

        // otherwise, assume end of buffer size
        else
        {
            // set buffer size Boolean to false to end loop
            bufferSizeAvailable = False;

            // set status return to buffer overrun
            statusReturn = INPUT_BUFFER_OVERRUN_ERR;
        }
    }

    // return operation status
    return statusReturn;
}

Boolean isEndOfFile(FILE *filePtr)
{
    // check for feof end of file response
    if (feof(filePtr) != 0)
    {
        // return true
        return True;
    }

    // return false
    return False;
}

/*
Function Name: output
Algorithm: Output the message with given config
Precondition: given pointer to config data
Postcondition: Output to the monitor, file or both.
Exceptions: none
Notes: none
*/
void output(char *message, ConfigDataType *configPtr)
{
    // if log to file, then write file
    if (configPtr->logToCode == LOGTO_FILE_CODE)
    {
        FILE *fp = fopen(configPtr->logToFileName, "a+");
        fprintf(fp, "%s\n", message);
        fclose(fp);
    }

    // if both, then write to file and to screen
    if (configPtr->logToCode == LOGTO_BOTH_CODE)
    {
        FILE *fp = fopen(configPtr->logToFileName, "a+");
        fprintf(fp, "%s\n", message);
        fclose(fp);
        printf("%s\n", message);
    }

    // if screen only, then write to screen
    if (configPtr->logToCode == LOGTO_MONITOR_CODE)
    {
        printf("%s\n", message);
    }
}

/*
Function Name: output_with_time
Algorithm: Output the message and current timestamp with given config
Precondition: given pointer to config data
Postcondition: Output to the monitor, file or both.
Exceptions: none
Notes: none
*/
void output_with_time(char *message, ConfigDataType *configPtr)
{
    char value[100], buf[200];
    accessTimer(LAP_TIMER, value);
    sprintf(buf, " %s, %s", value, message);
    output(buf, configPtr);
}

// header files
#include "StringUtils.h"

// constant declaraation
const char NULL_CHAR = '\0';
const char SPACE = ' ';
const char COLON = ':';
const char COMMA = ',';
const char SEMICOLON = ';';
const char PERIOD = '.';
const int STD_STR_LEN = 64;
const int MAX_STR_LEN = 128;
const int STR_EQ = 0;
const int SUBSTRING_NOT_FOUND = -1001;
const Boolean IGNORE_LEADING_WS = True;
const Boolean ACCEPT_LEADING_WS = False;

/*
 Name: compareString
 Process: compares two strings with the following results
		if left string less than right string, returns less than zero
		if left string greater than right string, returns grater than zero
		if left string equals right/string, returns zero
		- equals test includes length
 Function Iutput/Parameters: c-style left and right strings (char *)
 Function Output/Parameters: none
 Function Output/Returned: result as specified (int)
 Device Input/Keyboard: none
 Device Output/Monitor: none
 Dependencies: getStringLength
 */
int compareString(const char *oneStr, const char *otherStr)
{
	// initialize function/variables
	int diff, index = 0;

	// loop to end of the shrotest string
	// with overrun protection
	while (oneStr[index] != NULL_CHAR && otherStr[index] != NULL_CHAR && index < MAX_STR_LEN)
	{
		// get difference in characters
		diff = oneStr[index] - otherStr[index];

		// check for difference between characters
		if (diff != 0)
		{
			// return difference
			return diff;
		}

		// increment index
		index++;
	}
	// end loop

	// return difference in lengths, if any
	// function: getStringLength
	return getStringLength(oneStr) - getStringLength(otherStr);
}
/*
	Name: concatenateString
	Process: appends one string onto another
	Function Input/Parameters: c-style source strings (char*)
	Function Output/Parameters: c-style destination string (char*)
	Function Output/Returned: none
	Device Input/Keyboard: none
	Device Output/Monitor: none
	Dependencies: getStringLength
*/
void concatenateString(char *destStr, const char *sourceStr)
{
	// initialize function/variables

	// set destination index
	// function: getStringLength
	int destIndex = getStringLength(destStr);

	// set source string index
	// function: getStringLength
	int sourceStrLen = getStringLength(sourceStr);

	// create temporary string pointer
	char *tempStr;

	// set other variables
	int sourceIndex = 0;

	// copy source in case of aliasing
	// function: malloc copyString
	tempStr = (char *)malloc(sizeof(sourceStrLen + 1));
	copyString(tempStr, sourceStr);

	// loop to end of source string
	while (tempStr[sourceIndex] != NULL_CHAR && destIndex < MAX_STR_LEN)
	{
		// asssign characters to end of destination string
		destStr[destIndex] = tempStr[sourceIndex];

		// update indices
		destIndex++;
		sourceIndex++;

		// set temporary end of destination string
		destStr[destIndex] = NULL_CHAR;
	}
	// end loop

	// release memory used for temp string
	// function: free
	free(tempStr);
}

/*
	Name: copyString
	Process: copies one string into another,
			overwriting data in the destination string
	Function Input/Parameters: c-style source string (char*)
	Function Output/Parameters: c-style destination string (char*)
	Function Output/Returned: none
	Device Input/Keybard: none
	Device Output/monitor: none
	Dependencies: getStringLength
*/
void copyString(char *destStr, const char *sourceStr)
{
	// initialize function/variables
	int index = 0;

	// check for source/destination not the same(aliasing)
	if (destStr != sourceStr)
	{
		// loop to end of source string
		while (sourceStr[index] != NULL_CHAR && index < MAX_STR_LEN)
		{
			// assign character to end of destination string
			destStr[index] = sourceStr[index];

			// update index
			index++;

			// set temporary end of destination string
			destStr[index] = NULL_CHAR;
		}
		// end loop
	}
}

/*
	Name: findSubString
	Process: linear search for given substring within another string
	Function Input/Parameters: c-style source test string (char*)
							c-style source search string (char*)
	Function Output/Parameters: none
	Function Output/Returned: index of found substring, or
						SUBSTRING_NOT_FOUND constant
						if string not found
	Device Input/Keyboard: none
	Device Output/Monitor: none
	Dependencies: getStringLength
*/
int findSubString(const char *testStr, const char *searchSubStr)
{
	// initialize function/variables

	// initialize test string length
	// function: getStringLength
	int testStrLen = getStringLength(testStr);

	// initialize master index - location of sub string start point
	int masterIndex = 0;
	// initialize other variables
	int searchIndex, internalIndex;

	// loop accross test string
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
	// end loop across test string

	// assume test have failed at this point, return SUBSTRING_NOT_FOUND;
	return SUBSTRING_NOT_FOUND;
}

/*
	Name: getStringConstrained
	Process: capture a string from the input stream
			to a specified delimiter
			Note: consumes delimiter
	Function Input/Parameters: input stream (FILE*)
							stops at specified delimiter (char),
	Function Output/Parameters: string returned (char*)
	Function Output/Returned: success of operation (bool)
	Device Input/Keyboard: none
	Device Output/Monitor: none
	Dependencies: fgetc
*/
bool getStringConstrained(
    FILE *inStream,
    bool clearLeadingNonPrintable,
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
	while ((intChar != EOF) && ((clearLeadingNonPrintable && intChar == (int)SPACE)) || (clearLeadingSpace && intChar == (int)SPACE))
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
	    // OR continues if not printable flag is not set
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
	return true;
}
/*
	Name: getStringLength
	Process: find the length of a string by counting characters up to the NULL_CHAR character
	Function Input/Parameters: c-style string (char*)
	Function Output/Parameters: none
	Function Output/Returned: length of string
	Device Input/Keyboard: none
	Device Output/Monitor: none
	Dependencies: none
*/
int getStringLength(const char *testStr)
{
	// initialize functio/variables
	int index = 0;

	// loop to end of stirng, protect from overflow
	while (index < STD_STR_LEN && testStr[index] != NULL_CHAR)
	{
		// update index
		index++;
	}
	// end loop

	// return index/length
	return index;
}
/*
	Name: getStringToDelimiter
	Process: captre a string from the input strem to a specified delimiter;
			Note: consumes delimiter
	Function Input/Parameters: input sream (FILE*)
							stops at specified delimiter (char),
	Function Output/Parameters: string returned (char*)
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
	    inStream,	    // file stream pointer
	    true,		    // clears leading non printable character
	    true,		    // bool clearLeadingSpace
	    true,		    // stops at non printable
	    delimiter,	    // stops at delimiter
	    capturedString // returns string
	);
}
/*
	Name: getStringToLineEnd
	Process: capture a string from the input stream to the end of the line
	Function Input/Parameters: input stream (FILE*)
	Function Output/Parameters: string returned (char*)
	Function Output/Returned: success of operation (bool)
	Device Input/Keyboard: none
	Device Output/Monitor: none
	Dependencies: getStringConstrained
*/
bool getStringToLineEnd(
    FILE *inStream,
    char *capturedString)
{
	// call engine function with delimiter
	// function: getStringConstrained
	return getStringConstrained(
	    inStream,		    // file stream pointer
	    true,			    // clears leading non printable character
	    true,			    // bool clearLeadingSpace
	    true,			    // stops ar non printable
	    NON_PRINTABLE_CHAR, // non printable delimiter
	    capturedString	    // returns string
	);
}
/*
	Name: getSubString
	Process: captures sub string within larger string between two inclusive indices
			returns empty string if either index is out of range,
			assumes enough memory in destination string
	Function Input/Parameters: c-style source string (char*),
							start and end indices (int)
	Function Output/Parameters: c-style detsination string (char*)
	Function Output/Returned: none
	Device Input/Keyboard: none
	Device Output/Monitor: none
	Dependencies:getStringLength, malloc, copyString, free
*/
void getSubString(char *destStr, const char *sourceStr, int startIndex, int endIndex)
{
	// initialize function/varibles
	// set length of source string
	// function getStringLength

	int sourceStrLen = getStringLength(sourceStr);

	// initialize destination index to zero
	int destIndex = 0;

	// initialize source index to stat index (parameter)
	int sourceIndex = startIndex;

	// create pointer to temp string
	char *tempStr;

	// check for indices within limits
	if (startIndex >= 0 && startIndex <= endIndex && endIndex < sourceStrLen)
	{
		// create temporary string
		// function: malloc, copyString
		tempStr = (char *)malloc(sourceStrLen + 1);
		copyString(tempStr, sourceStr);

		// loop across requested substring (indices)
		while (sourceIndex <= endIndex)
		{
			// assign source character to destination element
			destStr[destIndex] = tempStr[sourceIndex];

			// increment indices
			destIndex++;
			sourceIndex++;

			// set temporary end of destination strng
			destStr[destIndex] = NULL_CHAR;
		}
		// end loop

		// release memory for temporary string
		// function: free
		free(tempStr);
	}
}

/*
	Name: setStrToLowerCase
	Process: iterates through string, sets an upper case letter
			to lower case; no effect on other letters
	Function Input/Parameters: c-style source string (char*)
	Function Output/Parameters: c-style destination string (char*)
	Function Output/Returned: none
	Device Input/Keyboard: none
	Device Output/Monitor: none
	Dependencies: toLowerCase
*/
void setStrToLowerCase(char *destStr, const char *sourceStr)
{
	// initialize function/variables
	// get soruce string elngth
	// function: getStringLength
	int sourceStrLen = getStringLength(sourceStr);

	// create temporary string pointer
	char *tempStr;

	// create other variables
	int index = 0;

	// copy source string in case of aliasing
	// function: mallo, copyString
	tempStr = (char *)malloc(sizeof(sourceStrLen + 1));
	copyString(tempStr, sourceStr);

	// loop across source string
	while (tempStr[index] != NULL_CHAR && index < MAX_STR_LEN)
	{
		// set individual character to lower case as needed
		// assign to destination string
		destStr[index] = toLowerCase(tempStr[index]);

		// update index
		index++;

		// set temporary end of destination strng
		destStr[index] = NULL_CHAR;
	}
	// release memory used for temp string
	// function: free
	free(tempStr);
}

/*
	Name: toLowerCase
	Process: if character is uppercase, sets it to lowercase;
			otherwise returns character unchanged
	Function Input/Parameters: text character (char)
	Function Output/Parameters: none
	Function Output/Returned: character to set to lower case, if appropraite
	Device Input/Keyboard: none
	Device Output/Monitor: none
	Dependencies: none
*/
char toLowerCase(char testChar)
{
	// check for uppercase letter
	if (testChar >= 'A' && testChar <= 'Z')
	{
		// return ower case letter
		testChar = testChar - 'A' + 'a';
	}
	// otherwise, assume no uppercase letter,
	// return character unchanged
	return testChar;
}

int getLineTo(FILE *filePtr, int bufferSize, char stopChar,
		    char *buffer, Boolean omitLeadingWhiteSpace, Boolean stopAtNonPrintable)
{
	// initialize function/variables
	int charAsInt, charIndex = 0;
	int statusReturn = NO_ERR;
	Boolean bufferSizeAvailable = True;
	Boolean nonPrintableFound = False;
	// get first character (as integer)
	// function: fgetc
	charAsInt = fgetc(filePtr);

	// consume leading white space, fi flagged
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
	// loop while charcter is not stop character and buffer size available
	while (charAsInt != (int)stopChar && nonPrintableFound == False && bufferSizeAvailable == True)
	{
		// check for input failure
		// function:isEndOfFile
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
			// set buffer size Boolean to fale to end loop
			bufferSizeAvailable = False;

			// set staus return to buffer overrun
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
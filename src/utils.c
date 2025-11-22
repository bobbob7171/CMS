/*
 * utils.c - Utility Functions Implementation
 * Provides string manipulation and command parsing helpers
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"

// Trim leading and trailing whitespace characters from a string (in place)
void trimWhitespace(char* str) {
    if (!str) return;
    
    // Skip leading whitespace 
    char* start = str;
    while (isspace((unsigned char)*start)) start++;
    
    if (*start == 0) {
        *str = '\0';
        return;
    }
    
    // Trim trailing whitespace
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    
    // Move trimmed string to beginning and null terminate
    size_t len = (end - start) + 1;
    if (start != str) {
        memmove(str, start, len);
    }
    str[len] = '\0';
}

// Convert a string to uppercase (ASCII only)
void toUpperCase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

// Convert a string to lowercase (ASCII only)
void toLowerCase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

// Return 1 if the string is NULL, empty, or only whitespace; otherwise 0
int isEmpty(const char* str) {
    if (!str || strlen(str) == 0) return 1;
    
    for (int i = 0; str[i]; i++) {
        if (!isspace((unsigned char)str[i])) return 0;
    }
    return 1;
}

// Print a prompt and read a single line of input, stripping the trailing '\n'
void getInput(const char* prompt, char* buffer, int size) {
    printf("%s", prompt);
    if (fgets(buffer, size, stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0';
    }
}

/* 
 * Parse key-value pairs from command parameters
 * Format: KEY=value or KEY="value with spaces"
 * Returns: 1 if found, 0 if not found
 */
int parseKeyValue(const char* params, const char* key, char* value) {
    // Create uppercase copies so we can search for keys without case issues
    char upperParams[1024];
    strncpy(upperParams, params, 1023);
    upperParams[1023] = '\0';
    toUpperCase(upperParams);
    
    char upperKey[50];
    strncpy(upperKey, key, 49);
    upperKey[49] = '\0';
    toUpperCase(upperKey);
    
    // Search for KEY= pattern in the uppercased command
    char searchPattern[52];
    sprintf(searchPattern, "%s=", upperKey);
    
    const char* keyPos = strstr(upperParams, searchPattern);
    if (!keyPos) return 0;
    
    // Translate match position back into the original string
    int offset = keyPos - upperParams;
    const char* valueStart = params + offset + strlen(searchPattern);
    
    // Skip any whitespace before the actual value
    while (*valueStart == ' ' || *valueStart == '\t') {
        valueStart++;
    }
    
    // Find end of value by looking for the next known key or end of string
    const char* valueEnd = NULL;
    const char* searchStart = upperParams + offset + strlen(searchPattern);
    
    // List of possible keys to detect the start of the next pair
    const char* possibleKeys[] = {"ID=", "NAME=", "PROGRAMME=", "PROGRAM=", "MARK="};
    int minOffset = 1024;
    
    for (int i = 0; i < 5; i++) {
        // Look for the next key starting after a space (e.g. " NAME=")
        char pattern[60];
        sprintf(pattern, " %s", possibleKeys[i]);
        const char* found = strstr(searchStart, pattern);
        
        if (found) {
            int foundOffset = found - searchStart;
            if (foundOffset < minOffset) {
                minOffset = foundOffset;
                valueEnd = params + (found - upperParams);
            }
        }
    }
    
    // Calculate value length and trim trailing spaces
    int valueLen;
    if (valueEnd) {
        valueLen = valueEnd - valueStart;
        // Trim trailing whitespace
        while (valueLen > 0 && (valueStart[valueLen-1] == ' ' || valueStart[valueLen-1] == '\t')) {
            valueLen--;
        }
    } else {
        valueLen = strlen(valueStart);
    }
    
    // Copy the final value into the output buffer
    if (valueLen > 0 && valueLen < 512) {
        strncpy(value, valueStart, valueLen);
        value[valueLen] = '\0';
        trimWhitespace(value);
    } else {
        value[0] = '\0';
    }
    
    return 1;
}

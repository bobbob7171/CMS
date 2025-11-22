/*
 * utils.h - Utility Functions
 * String manipulation and command parsing utilities
 */

#ifndef UTILS_H
#define UTILS_H

// String manipulation
void trimWhitespace(char* str);
void toUpperCase(char* str);
void toLowerCase(char* str);

// Command parsing
int parseKeyValue(const char* params, const char* key, char* value);
int isEmpty(const char* str);
void getInput(const char* prompt, char* buffer, int size);

#endif
/*
 * fileio.c - Database File I/O Implementation
 * Handles file operations, validation, and data integrity
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "fileio.h"
#include "utils.h"

// Build a path inside the data folder, rejecting any user-supplied directories
static int buildDataPath(const char* filename, char* buffer, size_t bufferSize) {
    if (!filename || !buffer || bufferSize == 0) {
        return 0;
    }
    
    // Disallow directory traversal or explicit paths; we only read/write under data/
    if (strchr(filename, '/') || strchr(filename, '\\')) {
        return 0;
    }
    
    int written = snprintf(buffer, bufferSize, "data/%s", filename);
    return written > 0 && (size_t)written < bufferSize;
}

/* ============================================================================
 * Database Management
 * ============================================================================ */

// Initialise an empty in-memory database structure
void initDatabase(Database* db) {
    db->count = 0;
    db->isOpen = 0;
    db->isDirty = 0;
    db->filename[0] = '\0';
}

// Close the currently open database, prompting to save unsaved changes
void closeDatabase(Database* db) {
    if (!db->isOpen) {
        printf("CMS: No database open.\n");
        return;
    }
    
    // Prompt the user to save if there are unsaved changes
    if (db->isDirty) {
        char response[16];
        while (1) {
            printf("CMS: Unsaved changes in \"%s\". Save? (Y/N/Cancel): ", db->filename);
            if (!fgets(response, sizeof(response), stdin)) {
                printf("CMS: Close cancelled.\n");
                return;
            }
            trimWhitespace(response);
            if (isEmpty(response)) {
                printf("CMS: Please enter Y, N, or C.\n");
                continue;
            }
            if (response[0] == 'Y' || response[0] == 'y') {
                saveDatabase(db);
                break;
            }
            if (response[0] == 'N' || response[0] == 'n') {
                break;
            }
            if (response[0] == 'C' || response[0] == 'c') {
                printf("CMS: Close cancelled.\n");
                return;
            }
            printf("CMS: Please enter Y, N, or C.\n");
        }
    }
    
    printf("CMS: Closed \"%s\".\n", db->filename);
    initDatabase(db);
}

// Close the current file (if any) and open a new one, with a save prompt
int switchDatabase(Database* db, const char* newFilename) {
    // Give the user a chance to save or cancel before switching files
    if (db->isOpen && db->isDirty) {
        char response[16];
        while (1) {
            printf("CMS: Unsaved changes in \"%s\". Save? (Y/N/Cancel): ", db->filename);
            if (!fgets(response, sizeof(response), stdin)) {
                printf("CMS: Switch cancelled.\n");
                return 0;
            }
            trimWhitespace(response);
            if (isEmpty(response)) {
                printf("CMS: Please enter Y, N, or C.\n");
                continue;
            }
            if (response[0] == 'C' || response[0] == 'c') {
                printf("CMS: Switch cancelled.\n");
                return 0;
            }
            if (response[0] == 'Y' || response[0] == 'y') {
                saveDatabase(db);
                break;
            }
            if (response[0] == 'N' || response[0] == 'n') {
                break;
            }
            printf("CMS: Please enter Y, N, or C.\n");
        }
    }
    
    if (db->isOpen) {
        initDatabase(db);
    }
    
    return openDatabase(db, newFilename);
}

/* ============================================================================
 * Validation Functions
 * ============================================================================ */

int validateStudentID(int id, char* errorMsg) {
    if (id <= 0) {
        sprintf(errorMsg, "Student ID must be positive");
        return 0;
    }
    
    // Count digits to ensure the ID fits the YYXXXXX pattern
    int digitCount = 0;
    int tempId = id;
    while (tempId > 0) {
        digitCount++;
        tempId /= 10;
    }
    
    if (digitCount != 7) {
        sprintf(errorMsg, "Student ID must be 7 digits (Format: YYXXXXX)");
        return 0;
    }
    
    // Validate year prefix (first 2 digits of the ID)
    int year = id / 100000;
    if (year > 25) {
        sprintf(errorMsg, "Year cannot exceed 25 (currently 2025)");
        return 0;
    }
    if (year == 0) {
        sprintf(errorMsg, "Year cannot be 00");
        return 0;
    }
    
    return 1;
}

int validateName(const char* name, char* errorMsg) {
    if (!name || strlen(name) == 0) {
        sprintf(errorMsg, "Name cannot be empty");
        return 0;
    }
    
    int len = strlen(name);
    if (len < 2) {
        sprintf(errorMsg, "Name must be at least 2 characters");
        return 0;
    }
    if (len > 50) {
        sprintf(errorMsg, "Name cannot exceed 50 characters");
        return 0;
    }
    
    // Validate characters and check for at least one letter
    int hasLetter = 0;
    for (int i = 0; i < len; i++) {
        char c = name[i];
        if (isalpha(c)) {
            hasLetter = 1;
        } else if (c != ' ' && c != '-' && c != '\'' && c != '.') {
            sprintf(errorMsg, "Name can only contain letters, spaces, hyphens, apostrophes, periods");
            return 0;
        }
    }
    
    if (!hasLetter) {
        sprintf(errorMsg, "Name must contain at least one letter");
        return 0;
    }
    
    return 1;
}

int validateProgramme(const char* programme, char* errorMsg) {
    if (!programme || strlen(programme) == 0) {
        sprintf(errorMsg, "Programme cannot be empty");
        return 0;
    }
    
    int len = strlen(programme);
    if (len < 2) {
        sprintf(errorMsg, "Programme must be at least 2 characters");
        return 0;
    }
    
    // Validate characters and confirm there is at least one letter
    int hasLetter = 0;
    for (int i = 0; i < len; i++) {
        char c = programme[i];
        if (isalpha(c)) {
            hasLetter = 1;
        } else if (c != ' ' && c != '-' && c != '&') {
            sprintf(errorMsg, "Programme can only contain letters, spaces, hyphens, and &");
            return 0;
        }
    }
    
    if (!hasLetter) {
        sprintf(errorMsg, "Programme must contain at least one letter");
        return 0;
    }
    
    return 1;
}

int validateMark(const char* markStr, float* outMark, char* errorMsg) {
    if (!markStr || strlen(markStr) == 0) {
        sprintf(errorMsg, "Mark cannot be empty");
        return 0;
    }
    
    // Validate numeric format (digits, optional decimal point, optional sign)
    int hasDigit = 0;
    int decimalCount = 0;
    
    for (int i = 0; markStr[i] != '\0'; i++) {
        if (isdigit(markStr[i])) {
            hasDigit = 1;
        } else if (markStr[i] == '.') {
            decimalCount++;
            if (decimalCount > 1) {
                sprintf(errorMsg, "Mark can only have one decimal point");
                return 0;
            }
        } else if (markStr[i] == '-' && i == 0) {
            continue; // Allow minus at start
        } else if (markStr[i] == ' ') {
            continue; // Skip spaces
        } else {
            sprintf(errorMsg, "Mark must be a number (digits and decimal only)");
            return 0;
        }
    }
    
    if (!hasDigit) {
        sprintf(errorMsg, "Mark must contain at least one digit");
        return 0;
    }
    
    // Validate resulting numeric value is within allowed range
    float mark = atof(markStr);
    if (mark < 0.0 || mark > 100.0) {
        sprintf(errorMsg, "Mark must be between 0.0 and 100.0");
        return 0;
    }
    
    *outMark = mark;
    return 1;
}

/* ============================================================================
 * File Operations - Helper Functions
 * ============================================================================ */

// Check for duplicate IDs in database
int checkDuplicateIDs(Database* db) {
    int duplicates = 0;
    
    for (int i = 0; i < db->count; i++) {
        for (int j = i + 1; j < db->count; j++) {
            if (db->records[i].id == db->records[j].id) {
                duplicates++;
                printf("  WARNING: Duplicate ID found: %d (positions %d and %d)\n", 
                       db->records[i].id, i+1, j+1);
            }
        }
    }
    
    return duplicates;
}

// Validate file header format
static int validateFileHeader(FILE* file, char* errorMsg) {
    char line[MAX_LINE_LEN];
    
    // Read and validate header lines (1-4)
    for (int i = 0; i < 4; i++) {
        if (!fgets(line, sizeof(line), file)) {
            sprintf(errorMsg, "File format error: Missing header line %d", i + 1);
            return 0;
        }
    }
    
    // Read and validate column headers (line 5)
    if (!fgets(line, sizeof(line), file)) {
        sprintf(errorMsg, "File format error: Missing column headers");
        return 0;
    }
    
    // Check for required column names in a case-insensitive way
    char upperLine[MAX_LINE_LEN];
    strncpy(upperLine, line, MAX_LINE_LEN - 1);
    upperLine[MAX_LINE_LEN - 1] = '\0';
    toUpperCase(upperLine);
    
    if (!strstr(upperLine, "ID") || !strstr(upperLine, "NAME") || 
        !strstr(upperLine, "PROGRAMME") || !strstr(upperLine, "MARK")) {
        sprintf(errorMsg, "File format error: Column headers must contain ID, Name, Programme, Mark");
        return 0;
    }
    
    return 1;
}

// Parse a single record line from file
static int parseRecordLine(const char* line, StudentRecord* record, int lineNum, char* errorMsg) {
    char lineCopy[MAX_LINE_LEN];
    strncpy(lineCopy, line, MAX_LINE_LEN - 1);
    lineCopy[MAX_LINE_LEN - 1] = '\0';
    
    // Parse ID column (first field)
    char* token = strtok(lineCopy, "\t");
    if (!token) {
        sprintf(errorMsg, "Missing ID field");
        return 0;
    }
    
    trimWhitespace(token);
    
    // Validate ID is numeric before converting to integer
    for (int i = 0; token[i]; i++) {
        if (!isdigit(token[i])) {
            sprintf(errorMsg, "ID must be numeric, got '%s'", token);
            return 0;
        }
    }
    
    int id = atoi(token);
    char idError[256];
    if (!validateStudentID(id, idError)) {
        sprintf(errorMsg, "%s", idError);
        return 0;
    }
    record->id = id;
    
    // Parse Name column (second field)
    token = strtok(NULL, "\t");
    if (!token || strlen(token) == 0) {
        sprintf(errorMsg, "Missing Name field");
        return 0;
    }
    
    trimWhitespace(token);
    char nameError[256];
    if (!validateName(token, nameError)) {
        sprintf(errorMsg, "%s", nameError);
        return 0;
    }
    strncpy(record->name, token, MAX_NAME_LEN - 1);
    record->name[MAX_NAME_LEN - 1] = '\0';
    record->hasName = 1;
    
    // Parse Programme column (third field)
    token = strtok(NULL, "\t");
    if (!token || strlen(token) == 0) {
        sprintf(errorMsg, "Missing Programme field");
        return 0;
    }
    
    trimWhitespace(token);
    char progError[256];
    if (!validateProgramme(token, progError)) {
        sprintf(errorMsg, "%s", progError);
        return 0;
    }
    strncpy(record->programme, token, MAX_PROGRAMME_LEN - 1);
    record->programme[MAX_PROGRAMME_LEN - 1] = '\0';
    record->hasProgramme = 1;
    
    // Parse Mark column (fourth field)
    token = strtok(NULL, "\t\n\r");
    if (!token || strlen(token) == 0) {
        sprintf(errorMsg, "Missing Mark field");
        return 0;
    }
    
    trimWhitespace(token);
    float mark;
    char markError[256];
    if (!validateMark(token, &mark, markError)) {
        sprintf(errorMsg, "%s", markError);
        return 0;
    }
    record->mark = mark;
    record->hasMark = 1;
    
    return 1;
}

/* ============================================================================
 * File Operations - Main Functions
 * ============================================================================ */

int openDatabase(Database* db, const char* filename) {
    if (db->isOpen) {
        printf("CMS: Database \"%s\" is already open. Use SWITCH to change files.\n", db->filename);
        return 0;
    }
    
    char cleanName[MAX_FILENAME_LEN];
    strncpy(cleanName, filename, sizeof(cleanName) - 1);
    cleanName[sizeof(cleanName) - 1] = '\0';
    trimWhitespace(cleanName);
    
    if (isEmpty(cleanName)) {
        printf("CMS: Filename cannot be empty.\n");
        return 0;
    }
    
    char resolvedPath[MAX_FILENAME_LEN];
    if (!buildDataPath(cleanName, resolvedPath, sizeof(resolvedPath))) {
        printf("CMS: Invalid filename \"%s\". Use a file inside the data folder.\n", cleanName);
        return 0;
    }
    
    FILE* file = fopen(resolvedPath, "r");
    if (!file) {
        printf("CMS: Cannot open file \"%s\" in data/. File may not exist.\n", cleanName);
        return 0;
    }
    
    // Validate basic file structure and column headers
    char errorMsg[256];
    if (!validateFileHeader(file, errorMsg)) {
        printf("CMS: %s\n", errorMsg);
        printf("CMS: Expected format:\n");
        printf("     Line 1: Database Name: ...\n");
        printf("     Line 2: Authors: ...\n");
        printf("     Line 3: (blank or info)\n");
        printf("     Line 4: Table Name: ...\n");
        printf("     Line 5: ID    Name    Programme    Mark\n");
        printf("     Line 6+: Data records (tab-separated)\n");
        fclose(file);
        return 0;
    }
    
    // Read data records line by line, stopping at MAX_RECORDS
    db->count = 0;
    int lineNum = 5;
    int recordsLoaded = 0;
    int recordsSkipped = 0;
    char line[MAX_LINE_LEN];
    
    while (fgets(line, sizeof(line), file) && db->count < MAX_RECORDS) {
        lineNum++;
        
        // Skip empty or whitespace-only lines
        if (isEmpty(line)) continue;
        
        // Parse record and apply validation on each field
        StudentRecord tempRecord;
        char recordError[256];
        
        if (parseRecordLine(line, &tempRecord, lineNum, recordError)) {
            db->records[db->count++] = tempRecord;
            recordsLoaded++;
        } else {
            recordsSkipped++;
            printf("  WARNING: Skipping invalid record at line %d: %s\n", lineNum, recordError);
        }
    }
    
    fclose(file);
    
    // Check if in-memory table has reached its maximum capacity
    if (db->count >= MAX_RECORDS) {
        printf("  WARNING: Database is full. Maximum %d records loaded.\n", MAX_RECORDS);
        printf("           Additional records in file were ignored.\n");
    }
    
    // Update database state after successful load
    db->isOpen = 1;
    db->isDirty = 0;
    strncpy(db->filename, cleanName, MAX_FILENAME_LEN - 1);
    db->filename[MAX_FILENAME_LEN - 1] = '\0';
    
    printf("CMS: The database file \"%s\" is successfully opened.\n", cleanName);
    printf("     Loaded %d valid record(s).\n", recordsLoaded);
    
    if (recordsSkipped > 0) {
        printf("     Skipped %d invalid record(s). See warnings above.\n", recordsSkipped);
    }
    
    // Data quality report to give a quick overview of loaded data
    if (db->count > 0) {
        printf("===============================================================================\n");
        printf("Data Quality Report\n");
        printf("===============================================================================\n");
        
        int duplicates = checkDuplicateIDs(db);
        if (duplicates > 0) {
            printf("  ALERT: Found %d duplicate ID(s)!\n", duplicates);
        } else {
            printf("  All IDs are unique.\n");
        }
        
        printf("  All loaded records have complete data.\n");
        printf("===============================================================================\n\n");
    }
    
    return 1;
}

int saveDatabase(Database* db) {
    if (!db->isOpen) {
        printf("CMS: No database open.\n");
        return 0;
    }
    
    char resolvedPath[MAX_FILENAME_LEN];
    if (!buildDataPath(db->filename, resolvedPath, sizeof(resolvedPath))) {
        printf("CMS: Cannot save. Invalid filename \"%s\".\n", db->filename);
        return 0;
    }
    
    FILE* file = fopen(resolvedPath, "w");
    if (!file) {
        printf("CMS: Cannot save to \"%s\" in data/.\n", db->filename);
        return 0;
    }
    
    // Write file header and column names
    fprintf(file, "Database Name: P1_2-CMS\n");
    fprintf(file, "Authors: Assistant Prof Oran Zane Devilly\n\n");
    fprintf(file, "Table Name: StudentRecords\n");
    fprintf(file, "ID\tName\tProgramme\tMark\n");
    
    // Write each record in tab-separated format
    for (int i = 0; i < db->count; i++) {
        fprintf(file, "%d\t%s\t%s\t%.1f\n",
                db->records[i].id,
                db->records[i].name,
                db->records[i].programme,
                db->records[i].mark);
    }
    
    fclose(file);
    db->isDirty = 0;
    printf("CMS: The database file \"%s\" is successfully saved.\n", db->filename);
    return 1;
}

/*
 * main.c - Class Management System Entry Point
 * Command processing and main application loop
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fileio.h"
#include "operations.h"
#include "enhance.h"
#include "unique.h"
#include "utils.h"

#define MAX_COMMAND_LEN 512

/* ============================================================================
 * Command Processing
 * ============================================================================ */

/* Interpret a single user command string and dispatch it to the right handler */
static int processCommand(Database* db, const char* input) {
    char command[MAX_COMMAND_LEN];
    strcpy(command, input);
    trimWhitespace(command);
    
    if (isEmpty(command)) return OP_SUCCESS;
    
    /* Uppercase copy is used only for command keyword matching */
    char upperCmd[MAX_COMMAND_LEN];
    strcpy(upperCmd, command);
    toUpperCase(upperCmd);
    
    /* EXIT */
    if (strcmp(upperCmd, "EXIT") == 0) {
        if (db->isOpen && db->isDirty) {
            char response[16];
            while (1) {
                printf("CMS: Unsaved changes. Save? (Y/N/Cancel): ");
                if (!fgets(response, sizeof(response), stdin)) {
                    printf("CMS: Exit cancelled.\n");
                    return OP_SUCCESS;
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
                    printf("CMS: Exit cancelled.\n");
                    return OP_SUCCESS;
                }
                printf("CMS: Please enter Y, N, or C.\n");
            }
        }
        printf("CMS: Goodbye!");
        return OP_EXIT;
    }
    
    /* OPEN */
    if (strncmp(upperCmd, "OPEN", 4) == 0) {
        char filename[MAX_FILENAME_LEN];
        const char* fileStart = command + 4;
        while (*fileStart == ' ') fileStart++;
        
        if (strlen(fileStart) > 0) {
            strncpy(filename, fileStart, MAX_FILENAME_LEN - 1);
            filename[MAX_FILENAME_LEN - 1] = '\0';
            trimWhitespace(filename);
        } else {
            strcpy(filename, "P1_2-CMS.txt");
        }
        
        openDatabase(db, filename);
        saveStateForUndo(db);
        return OP_SUCCESS;
    }
    
    /* SWITCH */
    if (strncmp(upperCmd, "SWITCH", 6) == 0) {
        char filename[MAX_FILENAME_LEN];
        const char* fileStart = command + 6;
        while (*fileStart == ' ') fileStart++;
        
        if (strlen(fileStart) == 0) {
            printf("CMS: Usage: SWITCH filename.txt\n");
            return OP_SUCCESS;
        }
        
        strncpy(filename, fileStart, MAX_FILENAME_LEN - 1);
        filename[MAX_FILENAME_LEN - 1] = '\0';
        trimWhitespace(filename);
        
        switchDatabase(db, filename);
        saveStateForUndo(db);
        return OP_SUCCESS;
    }
    
    /* CLOSE */
    if (strcmp(upperCmd, "CLOSE") == 0) {
        closeDatabase(db);
        return OP_SUCCESS;
    }
    
    /* SAVE */
    if (strcmp(upperCmd, "SAVE") == 0) {
        if (!db->isOpen) {
            printf("CMS: No database open.\n");
            return OP_SUCCESS;
        }
        saveDatabase(db);
        return OP_SUCCESS;
    }
    
    /* AUTOSAVE */
    if (strcmp(upperCmd, "AUTOSAVE") == 0) {
        printf("CMS: Autosave is currently %s.\n", 
               isAutosaveEnabled() ? "ON" : "OFF");
        return OP_SUCCESS;
    }
    
    if (strncmp(upperCmd, "AUTOSAVE ", 9) == 0) {
        const char* param = upperCmd + 9;
        while (*param == ' ') param++;
        
        if (strcmp(param, "ON") == 0) {
            setAutosaveEnabled(1);
            printf("CMS: Autosave is now ON.\n");
        } else if (strcmp(param, "OFF") == 0) {
            setAutosaveEnabled(0);
            printf("CMS: Autosave is now OFF.\n");
        } else {
            printf("CMS: Usage: AUTOSAVE [ON|OFF]\n");
            printf("     Type AUTOSAVE alone to check current status.\n");
        }
        return OP_SUCCESS;
    }
    
    /* HELP */
    if (strcmp(upperCmd, "HELP") == 0) {
        displayHelp();
        return OP_SUCCESS;
    }
    
    /* Commands below this point require a database to be open */
    if (!db->isOpen) {
        printf("CMS: No database open. Use OPEN command first.\n");
        return OP_SUCCESS;
    }
    
    /* SHOW commands */
    if (strcmp(upperCmd, "SHOW") == 0) {
        showMenu(db);
        return OP_SUCCESS;
    }
    
    if (strcmp(upperCmd, "SHOW ALL") == 0) {
        showAll(db);
        return OP_SUCCESS;
    }
    
    /* SHOW ALL SORT BY - delegate parsing to enhancement module */
    int sortResult = parseAndExecuteSort(db, upperCmd);
    if (sortResult >= 0) return OP_SUCCESS;
    
    /* SHOW ALL WHERE MARK - delegate parsing and validation */
    int filterResult = parseAndExecuteMarkFilter(db, input);
    if (filterResult >= 0) return OP_SUCCESS;
    
    /* SHOW SUMMARY */
    if (strcmp(upperCmd, "SHOW SUMMARY") == 0) {
        showSummary(db);
        return OP_SUCCESS;
    }
    
    /* INSERT - supports both direct parameters and interactive prompts */
    if (strncmp(upperCmd, "INSERT", 6) == 0) {
        char idStr[50];
        if (parseKeyValue(command, "ID", idStr)) {
            int id = atoi(idStr);
            if (id == 0 && strcmp(idStr, "0") != 0) {
                printf("CMS: ID must be numeric.\n");
                return OP_SUCCESS;
            }
            
            char errorMsg[256];
            if (!validateStudentID(id, errorMsg)) {
                printf("CMS: %s\n", errorMsg);
                return OP_SUCCESS;
            }
            
            /* Check if all required parameters are provided for direct insert */
            char temp[MAX_NAME_LEN];
            int hasAll = parseKeyValue(command, "Name", temp) && !isEmpty(temp) &&
                        parseKeyValue(command, "Programme", temp) && !isEmpty(temp) &&
                        parseKeyValue(command, "Mark", temp) && !isEmpty(temp);
            
            if (hasAll) {
                insertRecordDirect(db, command + 6);
            } else {
                insertRecordInteractive(db, id);
            }
        } else {
            printf("CMS: Usage: INSERT ID=xxx [Name=xxx Programme=xxx Mark=xxx]\n");
            printf("CMS: If only ID is provided, interactive mode will be used.\n");
        }
        saveStateForUndo(db);
        return OP_SUCCESS;
    }
    
    /* QUERY */
    if (strncmp(upperCmd, "QUERY", 5) == 0) {
        char idStr[50];
        if (parseKeyValue(command, "ID", idStr)) {
            int id = atoi(idStr);
            if (id == 0 && strcmp(idStr, "0") != 0) {
                printf("CMS: ID must be numeric.\n");
                return OP_SUCCESS;
            }
            queryRecord(db, id);
        } else {
            printf("CMS: Usage: QUERY ID=xxx\n");
        }
        return OP_SUCCESS;
    }
    
    /* UPDATE - supports partial updates and interactive mode */
    if (strncmp(upperCmd, "UPDATE", 6) == 0) {
        char idStr[50];
        if (parseKeyValue(command, "ID", idStr)) {
            int id = atoi(idStr);
            if (id == 0 && strcmp(idStr, "0") != 0) {
                printf("CMS: ID must be numeric.\n");
                return OP_SUCCESS;
            }
            
            /* Check if any field parameter is provided for direct update */
            char temp[MAX_NAME_LEN];
            int hasParams = parseKeyValue(command, "Name", temp) ||
                           parseKeyValue(command, "Programme", temp) ||
                           parseKeyValue(command, "Mark", temp);
            
            if (hasParams) {
                updateRecordDirect(db, id, command + 6);
            } else {
                updateRecordInteractive(db, id);
            }
            saveStateForUndo(db);
        } else {
            printf("CMS: Usage: UPDATE ID=xxx [Name=xxx] [Programme=xxx] [Mark=xxx]\n");
            printf("CMS: If only ID is provided, interactive mode will be used.\n");
        }
        return OP_SUCCESS;
    }
    
    /* DELETE - require confirmation before removing a record */
    if (strncmp(upperCmd, "DELETE", 6) == 0) {
        char idStr[50];
        if (parseKeyValue(command, "ID", idStr)) {
            int id = atoi(idStr);
            if (id == 0 && strcmp(idStr, "0") != 0) {
                printf("CMS: ID must be numeric.\n");
                return OP_SUCCESS;
            }
            deleteRecord(db, id);
            saveStateForUndo(db);
        } else {
            printf("CMS: Usage: DELETE ID=xxx\n");
        }
        return OP_SUCCESS;
    }
    
    /* SEARCH */
    if (strncmp(upperCmd, "SEARCH", 6) == 0) {
        char searchTerm[MAX_NAME_LEN];
        const char* searchStart = command + 6;
        while (*searchStart == ' ') searchStart++;
        
        if (strlen(searchStart) > 0) {
            strncpy(searchTerm, searchStart, MAX_NAME_LEN - 1);
            searchTerm[MAX_NAME_LEN - 1] = '\0';
            trimWhitespace(searchTerm);
            searchByKeyword(db, searchTerm);
        } else {
            printf("CMS: Usage: SEARCH <keyword>\n");
        }
        return OP_SUCCESS;
    }
    
    /* CHECK DUPLICATES */
    if (strcmp(upperCmd, "CHECK DUPLICATES") == 0 || 
        strcmp(upperCmd, "CHECK DUPLICATE") == 0) {
        checkDuplicates(db);
        return OP_SUCCESS;
    }
    
    /* EXPORT */
    if (strncmp(upperCmd, "EXPORT", 6) == 0) {
        exportToCSV(db, command);
        return OP_SUCCESS;
    }
    
    /* UNDO */
    if (strcmp(upperCmd, "UNDO") == 0) {
        undoLastOperation(db);
        return OP_SUCCESS;
    }
    
    /* REDO */
    if (strcmp(upperCmd, "REDO") == 0) {
        redoLastOperation(db);
        return OP_SUCCESS;
    }
    
    /* Unknown command */
    printf("CMS: Unknown command. Type HELP for commands.\n");
    return OP_SUCCESS;
}

/* ============================================================================
 * Main Program
 * ============================================================================ */

int main() {
    Database db;
    initDatabase(&db);
    
    /* Initialize systems */
    initializeUndoHistory();
    initializeAutosave(&db);
    
    /* Display declaration */
    displayDeclaration();
    
    /* Display welcome banner */
    printf("\n");
    printf("===============================================================================\n");
    printf("               CLASS MANAGEMENT SYSTEM - Group P1_2\n");
    printf("===============================================================================\n");
    printf("Data folder:  data/   (place all database .txt files here)\n");
    printf("Default file: P1_2-CMS.txt (use OPEN to load)\n");
    printf("Quick Start:  OPEN | INSERT | SHOW | SAVE | HELP | EXIT\n");
    printf("Advanced:     UNDO | REDO | EXPORT | SEARCH | AUTOSAVE | CHECK DUPLICATES\n");
    printf("Autosave:     OFF by default (type 'AUTOSAVE ON' to enable)\n\n");
    printf("Need help? Type HELP for the full command list and rules.\n");
    printf("===============================================================================\n");
    
    /* Main command loop */
    char input[MAX_COMMAND_LEN];
    int running = 1;
    
    while (running) {
        /* Display prompt with status indicator */
        if (db.isOpen) {
            /* Show '*' when dirty, otherwise show nothing (no space) */
            printf("P1_2%s: ", db.isDirty ? "*" : "");
        } else {
            printf("P1_2: ");
        }
        
        if (!fgets(input, sizeof(input), stdin)) break;
        
        input[strcspn(input, "\n")] = '\0';
        
        int result = processCommand(&db, input);
        if (result == OP_EXIT) running = 0;
        
        printf("\n");
    }
    
    /* Cleanup */
    printf("CMS: Cleaning up resources...\n");
    cleanupAutosave();
    cleanupUndoHistory();
    printf("CMS: Shutdown complete.\n");
    
    return 0;
}

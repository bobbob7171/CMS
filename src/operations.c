/*
 * operations.c - CRUD Operations Implementation
 * Handles Create, Read, Update, Delete operations in both interactive and direct modes
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "operations.h"
#include "enhance.h"
#include "unique.h"
#include "utils.h"

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/* Returns the index of the record with the given ID, or -1 if not found */
int findRecordIndex(Database* db, int id) {
    for (int i = 0; i < db->count; i++) {
        if (db->records[i].id == id) {
            return i;
        }
    }
    return -1;
}

/* Print a single student record in a compact, readable format */
void displayRecordSummary(StudentRecord* rec) {
    printf("-------------------------------------------------------------------------------\n");
    printf("Record Summary\n");
    printf("-------------------------------------------------------------------------------\n");
    printf("ID:         %d\n", rec->id);
    printf("Name:       %s\n", rec->name);
    printf("Programme:  %s\n", rec->programme);
    printf("Mark:       %.1f\n", rec->mark);
    printf("-------------------------------------------------------------------------------\n");
}

/* ============================================================================
 * Display Operations
 * ============================================================================ */

/* Print all records in a simple table with paging to avoid flooding the screen */
void showAll(Database* db) {
    if (db == NULL || db->count == 0) {
        printf("CMS: No records found.\n");
        return;
    }
    
    printf("CMS: Here are all the records found in the table \"StudentRecords\".\n");

    /* Show results in small chunks so very large datasets stay readable */
    const int PAGE_SIZE = 20;
    char input[32];
    int displayed = 0;

    printf("%-10s %-25s %-30s %-10s\n", "ID", "Name", "Programme", "Mark");
    printf("-------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < db->count; i++) {
        printf("%-10d %-25s %-30s %-10.1f\n",
               db->records[i].id,
               db->records[i].name,
               db->records[i].programme,
               db->records[i].mark);
        displayed++;

        if ((i + 1) % PAGE_SIZE == 0 && (i + 1) < db->count) {
            /* After each page, give the user a chance to stop */
            printf("-------------------------------------------------------------------------------\n");
            printf("-- Showing %d of %d record(s). Press Enter to continue, or type 'cancel' to stop --\n", 
                   i + 1, db->count);
            if (!fgets(input, sizeof(input), stdin)) {
                break;
            }
            trimWhitespace(input);
            if (strcasecmp(input, "cancel") == 0) {
                printf("CMS: Listing cancelled by user.\n");
                printf("CMS: Displayed %d of %d record(s).\n", displayed, db->count);
                return;
            }

            printf("\n%-10s %-25s %-30s %-10s\n", "ID", "Name", "Programme", "Mark");
            printf("-------------------------------------------------------------------------------\n");
        }
    }
    
    printf("-------------------------------------------------------------------------------\n");
    printf("CMS: Displayed %d of %d record(s).\n", displayed, db->count);
}

/* Show an interactive menu for the SHOW command (sorting, filtering, summary) */
void showMenu(Database* db) {
    printf("-------------------------------------------------------------------------------\n");
    printf("SHOW Menu\n");
    printf("-------------------------------------------------------------------------------\n");
    printf("1. Show all records\n");
    printf("2. Sort by ID (ascending)\n");
    printf("3. Sort by ID (descending)\n");
    printf("4. Sort by Mark (ascending)\n");
    printf("5. Sort by Mark (descending)\n");
    printf("6. Filter by mark (WHERE MARK)\n");
    printf("7. Show summary statistics\n");
    printf("0. Cancel\n");
    printf("-------------------------------------------------------------------------------\n");
    
    /* Loop until the user enters a valid option or cancels */
    while (1) {
        printf("Choice: ");
        char input[10];
        if (!fgets(input, sizeof(input), stdin)) continue;
        
        trimWhitespace(input);
        if (isEmpty(input)) {
            printf("CMS: Please enter a valid choice (0-7).\n");
            continue;
        }
        
        int choice = atoi(input);
        if (choice < 0 || choice > 7) {
            printf("CMS: Invalid choice. Please enter a number between 0 and 7.\n");
            continue;
        }
        
        switch(choice) {
            case 1: showAll(db); return;
            case 2: showAllSorted(db, "ID"); return;
            case 3: showAllSorted(db, "ID_DESC"); return;
            case 4: showAllSorted(db, "MARK"); return;
            case 5: showAllSorted(db, "MARK_DESC"); return;
            case 6: {
                /* Interactive filter by mark */
                char op[10], valueStr[20];
                
                while (1) {
                    printf("Enter operator (=, !=, <, >, <=, >=) or 'cancel' to exit: ");
                    if (!fgets(op, sizeof(op), stdin)) continue;
                    trimWhitespace(op);
                    
                    if (strcasecmp(op, "cancel") == 0) {
                        printf("CMS: Cancelled.\n");
                        return;
                    }
                    
                    /* Validation of operator string */
                    if (strcmp(op, "=") != 0 && strcmp(op, "!=") != 0 && 
                        strcmp(op, "<") != 0 && strcmp(op, ">") != 0 && 
                        strcmp(op, "<=") != 0 && strcmp(op, ">=") != 0) {
                        printf("CMS: Invalid operator. Please use: =, !=, <, >, <=, or >=\n");
                        continue;
                    }
                    break;
                }
                
                while (1) {
                    printf("Enter mark value (0-100) or 'cancel' to exit: ");
                    if (!fgets(valueStr, sizeof(valueStr), stdin)) continue;
                    trimWhitespace(valueStr);
                    
                    if (strcasecmp(valueStr, "cancel") == 0) {
                        printf("CMS: Cancelled.\n");
                        return;
                    }
                    
                    /* Detects non-numeric characters */
                    char* endptr;
                    double val = strtod(valueStr, &endptr);
                    if (valueStr == endptr || *endptr != '\0' || val < 0 || val > 100) {
                        printf("CMS: Invalid mark value. Please enter a number between 0 and 100.\n");
                        continue;
                    }
                    
                    showAllFilteredByMark(db, op, (float)val);
                    return;
                }
            }
            case 7: showSummary(db); return;
            case 0: printf("CMS: Cancelled.\n"); return;
        }
    }
}

/* ============================================================================
 * Query Operation
 * ============================================================================ */

void queryRecord(Database* db, int id) {
    int pos = findRecordIndex(db, id);
    
    if (pos == -1) {
        printf("CMS: The record with ID=%d does not exist.\n", id);
        return;
    }
    
    printf("CMS: The record with ID=%d is found in the data table.\n", id);
    printf("%-10s %-25s %-30s %-10s\n", "ID", "Name", "Programme", "Mark");
    printf("-------------------------------------------------------------------------------\n");
    printf("%-10d %-25s %-30s %-10.1f\n", 
           db->records[pos].id,
           db->records[pos].name,
           db->records[pos].programme,
           db->records[pos].mark);
    printf("-------------------------------------------------------------------------------\n");
}

/* ============================================================================
 * Insert Operations
 * ============================================================================ */

int insertRecordDirect(Database* db, const char* params) {
    if (db->count >= MAX_RECORDS) {
        printf("CMS: Database full (max %d records).\n", MAX_RECORDS);
        return OP_ERROR;
    }
    
    /* Parse required parameters from the command string */
    char idStr[50], name[MAX_NAME_LEN], programme[MAX_PROGRAMME_LEN], markStr[50];
    char errorMsg[256];
    
    if (!parseKeyValue(params, "ID", idStr)) {
        printf("CMS: ID parameter is required.\n");
        return OP_ERROR;
    }
    
    if (!parseKeyValue(params, "Name", name) || isEmpty(name)) {
        printf("CMS: Name parameter is required.\n");
        return OP_ERROR;
    }
    
    if (!parseKeyValue(params, "Programme", programme) || isEmpty(programme)) {
        printf("CMS: Programme parameter is required.\n");
        return OP_ERROR;
    }
    
    if (!parseKeyValue(params, "Mark", markStr) || isEmpty(markStr)) {
        printf("CMS: Mark parameter is required.\n");
        return OP_ERROR;
    }
    
    /* Validate ID value and logical rules */
    int id = atoi(idStr);
    if (id == 0 && strcmp(idStr, "0") != 0) {
        printf("CMS: ID must be numeric.\n");
        return OP_ERROR;
    }
    
    if (!validateStudentID(id, errorMsg)) {
        printf("CMS: %s\n", errorMsg);
        return OP_ERROR;
    }
    
    if (findRecordIndex(db, id) != -1) {
        printf("CMS: The record with ID=%d already exists.\n", id);
        return OP_ERROR;
    }
    
    /* Validate other fields using shared validation helpers */
    if (!validateName(name, errorMsg)) {
        printf("CMS: %s\n", errorMsg);
        return OP_ERROR;
    }
    
    if (!validateProgramme(programme, errorMsg)) {
        printf("CMS: %s\n", errorMsg);
        return OP_ERROR;
    }
    
    float mark;
    if (!validateMark(markStr, &mark, errorMsg)) {
        printf("CMS: %s\n", errorMsg);
        return OP_ERROR;
    }
    
    /* Create record */
    StudentRecord* rec = &db->records[db->count];
    rec->id = id;
    strncpy(rec->name, name, MAX_NAME_LEN - 1);
    rec->name[MAX_NAME_LEN - 1] = '\0';
    rec->hasName = 1;
    strncpy(rec->programme, programme, MAX_PROGRAMME_LEN - 1);
    rec->programme[MAX_PROGRAMME_LEN - 1] = '\0';
    rec->hasProgramme = 1;
    rec->mark = mark;
    rec->hasMark = 1;
    
    db->count++;
    db->isDirty = 1;
    
    printf("CMS: A new record with ID=%d is successfully inserted.\n", id);
    displayRecordSummary(rec);
    
    markDatabaseModified(db);
    return OP_SUCCESS;
}

int insertRecordInteractive(Database* db, int id) {
    if (db->count >= MAX_RECORDS) {
        printf("CMS: Database full (max %d records).\n", MAX_RECORDS);
        return OP_ERROR;
    }
    
    char input[MAX_NAME_LEN];
    char errorMsg[256];
    
    /* Get ID if not provided */
    if (id < 0) {
        while (1) {
            getInput("Student ID (7 digits, YYXXXXX): ", input, sizeof(input));
            
            if (strcasecmp(input, "cancel") == 0) {
                printf("CMS: Cancelled.\n");
                return OP_CANCELLED;
            }
            
            if (isEmpty(input)) {
                printf("CMS: ID is required.\n");
                continue;
            }
            
            /* Check if numeric */
            int isNumeric = 1;
            for (int i = 0; input[i]; i++) {
                if (!isdigit(input[i])) {
                    isNumeric = 0;
                    break;
                }
            }
            
            if (!isNumeric) {
                printf("CMS: ID must be numeric.\n");
                continue;
            }
            
            id = atoi(input);
            
            if (!validateStudentID(id, errorMsg)) {
                printf("CMS: %s\n", errorMsg);
                continue;
            }
            
            if (findRecordIndex(db, id) != -1) {
                printf("CMS: The record with ID=%d already exists.\n", id);
                continue;
            }
            
            break;
        }
    } else {
        if (findRecordIndex(db, id) != -1) {
            printf("CMS: The record with ID=%d already exists.\n", id);
            return OP_ERROR;
        }
    }
    
    StudentRecord* rec = &db->records[db->count];
    rec->id = id;
    rec->hasName = 1;
    rec->hasProgramme = 1;
    rec->hasMark = 1;
    
    /* Get Name */
    while (1) {
        getInput("Name: ", input, sizeof(input));
        
        if (strcasecmp(input, "cancel") == 0) {
            printf("CMS: Cancelled.\n");
            return OP_CANCELLED;
        }
        
        if (isEmpty(input)) {
            printf("CMS: Name is required.\n");
            continue;
        }
        
        if (!validateName(input, errorMsg)) {
            printf("CMS: %s\n", errorMsg);
            continue;
        }
        
        strncpy(rec->name, input, MAX_NAME_LEN - 1);
        rec->name[MAX_NAME_LEN - 1] = '\0';
        break;
    }
    
    /* Get Programme */
    while (1) {
        getInput("Programme: ", input, sizeof(input));
        
        if (strcasecmp(input, "cancel") == 0) {
            printf("CMS: Cancelled.\n");
            return OP_CANCELLED;
        }
        
        if (isEmpty(input)) {
            printf("CMS: Programme is required.\n");
            continue;
        }
        
        if (!validateProgramme(input, errorMsg)) {
            printf("CMS: %s\n", errorMsg);
            continue;
        }
        
        strncpy(rec->programme, input, MAX_PROGRAMME_LEN - 1);
        rec->programme[MAX_PROGRAMME_LEN - 1] = '\0';
        break;
    }
    
    /* Get Mark */
    while (1) {
        getInput("Mark (0-100): ", input, sizeof(input));
        
        if (strcasecmp(input, "cancel") == 0) {
            printf("CMS: Cancelled.\n");
            return OP_CANCELLED;
        }
        
        if (isEmpty(input)) {
            printf("CMS: Mark is required.\n");
            continue;
        }
        
        float mark;
        if (!validateMark(input, &mark, errorMsg)) {
            printf("CMS: %s\n", errorMsg);
            continue;
        }
        
        rec->mark = mark;
        break;
    }
    
    db->count++;
    db->isDirty = 1;
    
    printf("CMS: A new record with ID=%d is successfully inserted.\n", id);
    displayRecordSummary(rec);
    
    markDatabaseModified(db);
    return OP_SUCCESS;
}

/* ============================================================================
 * Update Operations
 * ============================================================================ */

int updateRecordDirect(Database* db, int id, const char* params) {
    int pos = findRecordIndex(db, id);
    
    if (pos == -1) {
        printf("CMS: The record with ID=%d does not exist.\n", id);
        return OP_ERROR;
    }
    
    StudentRecord* rec = &db->records[pos];
    StudentRecord oldRec = *rec;
    char value[MAX_NAME_LEN];
    char errorMsg[256];
    int changed = 0;
    
    printf("CMS: Updating record ID=%d...\n", id);
    
    /* Update Name if provided */
    if (parseKeyValue(params, "Name", value) && !isEmpty(value)) {
        if (validateName(value, errorMsg)) {
            strncpy(rec->name, value, MAX_NAME_LEN - 1);
            rec->name[MAX_NAME_LEN - 1] = '\0';
            changed = 1;
        } else {
            printf("CMS: Warning - %s Name not updated.\n", errorMsg);
        }
    }
    
    /* Update Programme if provided */
    if (parseKeyValue(params, "Programme", value) && !isEmpty(value)) {
        if (validateProgramme(value, errorMsg)) {
            strncpy(rec->programme, value, MAX_PROGRAMME_LEN - 1);
            rec->programme[MAX_PROGRAMME_LEN - 1] = '\0';
            changed = 1;
        } else {
            printf("CMS: Warning - %s Programme not updated.\n", errorMsg);
        }
    }
    
    /* Update Mark if provided */
    if (parseKeyValue(params, "Mark", value) && !isEmpty(value)) {
        float mark;
        if (validateMark(value, &mark, errorMsg)) {
            rec->mark = mark;
            changed = 1;
        } else {
            printf("CMS: Warning - %s Mark not updated.\n", errorMsg);
        }
    }
    
    if (changed) {
        db->isDirty = 1;
        printf("CMS: The record with ID=%d is successfully updated.\n", id);
        
        /* Show changes */
        printf("-------------------------------------------------------------------------------\n");
        printf("Changes Made\n");
        printf("-------------------------------------------------------------------------------\n");
        if (strcmp(oldRec.name, rec->name) != 0) {
            printf("Name:      '%s' -> '%s'\n", oldRec.name, rec->name);
        }
        if (strcmp(oldRec.programme, rec->programme) != 0) {
            printf("Programme: '%s' -> '%s'\n", oldRec.programme, rec->programme);
        }
        if (oldRec.mark != rec->mark) {
            printf("Mark:      %.1f -> %.1f\n", oldRec.mark, rec->mark);
        }
        printf("-------------------------------------------------------------------------------\n");
        
        displayRecordSummary(rec);
        markDatabaseModified(db);
        return OP_SUCCESS;
    } else {
        printf("CMS: No changes made to record ID=%d.\n", id);
        return OP_CANCELLED;
    }
}

int updateRecordInteractive(Database* db, int id) {
    int pos = findRecordIndex(db, id);
    
    if (pos == -1) {
        printf("CMS: The record with ID=%d does not exist.\n", id);
        return OP_ERROR;
    }
    
    StudentRecord* rec = &db->records[pos];
    StudentRecord oldRec = *rec;
    char input[MAX_NAME_LEN];
    char errorMsg[256];
    
    printf("-------------------------------------------------------------------------------\n");
    printf("UPDATE ID=%d\n", id);
    printf("-------------------------------------------------------------------------------\n");
    
    /* Update Name */
    while (1) {
        printf("Current Name: %s\n", rec->name);
        getInput("New Name (Enter to keep, 'cancel' to abort): ", input, sizeof(input));
        
        if (strcasecmp(input, "cancel") == 0) {
            printf("CMS: Update cancelled.\n");
            return OP_CANCELLED;
        }
        
        if (isEmpty(input)) break; /* Keep current value */
        
        if (validateName(input, errorMsg)) {
            strncpy(rec->name, input, MAX_NAME_LEN - 1);
            rec->name[MAX_NAME_LEN - 1] = '\0';
            db->isDirty = 1;
            break;
        } else {
            printf("CMS: %s Please try again.\n", errorMsg);
        }
    }
    
    /* Update Programme */
    while (1) {
        printf("Current Programme: %s\n", rec->programme);
        getInput("New Programme (Enter to keep, 'cancel' to abort): ", input, sizeof(input));
        
        if (strcasecmp(input, "cancel") == 0) {
            printf("CMS: Update cancelled.\n");
            return OP_CANCELLED;
        }
        
        if (isEmpty(input)) break; /* Keep current value */
        
        if (validateProgramme(input, errorMsg)) {
            strncpy(rec->programme, input, MAX_PROGRAMME_LEN - 1);
            rec->programme[MAX_PROGRAMME_LEN - 1] = '\0';
            db->isDirty = 1;
            break;
        } else {
            printf("CMS: %s Please try again.\n", errorMsg);
        }
    }
    
    /* Update Mark */
    while (1) {
        printf("Current Mark: %.1f\n", rec->mark);
        getInput("New Mark (Enter to keep, 'cancel' to abort): ", input, sizeof(input));
        
        if (strcasecmp(input, "cancel") == 0) {
            printf("CMS: Update cancelled.\n");
            return OP_CANCELLED;
        }
        
        if (isEmpty(input)) break; /* Keep current value */
        
        float mark;
        if (validateMark(input, &mark, errorMsg)) {
            rec->mark = mark;
            db->isDirty = 1;
            break;
        } else {
            printf("CMS: %s Please try again.\n", errorMsg);
        }
    }
    
    printf("CMS: The record with ID=%d is successfully updated.\n", id);
    
    /* Show changes if any were made */
    if (strcmp(oldRec.name, rec->name) != 0 ||
        strcmp(oldRec.programme, rec->programme) != 0 ||
        oldRec.mark != rec->mark) {
        
        printf("-------------------------------------------------------------------------------\n");
        printf("Changes Made\n");
        printf("-------------------------------------------------------------------------------\n");
        if (strcmp(oldRec.name, rec->name) != 0) {
            printf("Name:      '%s' -> '%s'\n", oldRec.name, rec->name);
        }
        if (strcmp(oldRec.programme, rec->programme) != 0) {
            printf("Programme: '%s' -> '%s'\n", oldRec.programme, rec->programme);
        }
        if (oldRec.mark != rec->mark) {
            printf("Mark:      %.1f -> %.1f\n", oldRec.mark, rec->mark);
        }
        printf("-------------------------------------------------------------------------------\n");
    }
    
    displayRecordSummary(rec);
    markDatabaseModified(db);
    return OP_SUCCESS;
}

/* ============================================================================
 * Delete Operation
 * ============================================================================ */

int deleteRecord(Database* db, int id) {
    int pos = findRecordIndex(db, id);
    
    if (pos == -1) {
        printf("CMS: The record with ID=%d does not exist.\n", id);
        return OP_ERROR;
    }
    
    StudentRecord deletedRec = db->records[pos];
    
    while (1) {
        printf("CMS: Are you sure you want to delete record with ID=%d? Type \"Y\" to confirm or \"N\" to cancel.\n", id);
        printf("P2_2: ");
        
        char response[16];
        if (!fgets(response, sizeof(response), stdin)) {
            printf("CMS: The deletion is cancelled.\n");
            return OP_CANCELLED;
        }
        trimWhitespace(response);
        if (isEmpty(response)) {
            printf("CMS: Please enter Y or N.\n");
            continue;
        }
        
        if (response[0] == 'Y' || response[0] == 'y') {
            /* Shift records to fill the gap */
            for (int i = pos; i < db->count - 1; i++) {
                db->records[i] = db->records[i + 1];
            }
            db->count--;
            db->isDirty = 1;
            
            printf("-------------------------------------------------------------------------------\n");
            printf("Deleted Record\n");
            printf("-------------------------------------------------------------------------------\n");
            printf("ID:         %d\n", deletedRec.id);
            printf("Name:       %s\n", deletedRec.name);
            printf("Programme:  %s\n", deletedRec.programme);
            printf("Mark:       %.1f\n", deletedRec.mark);
            printf("-------------------------------------------------------------------------------\n");
            
            markDatabaseModified(db);
            return OP_SUCCESS;
        }
        
        if (response[0] == 'N' || response[0] == 'n') {
            printf("CMS: The deletion is cancelled.\n");
            return OP_CANCELLED;
        }
        
        printf("CMS: Please enter Y or N.\n");
    }
}

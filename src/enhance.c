/*
 * enhance.c - Enhancement Features Implementation
 * Provides sorting, filtering, and statistical analysis
 */

#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include "enhance.h"
#include "utils.h"

/* Constants */
#define PASS_MARK 50.0
#define TOP_N 3

/* ============================================================================
 * Sorting Helper Functions
 * ============================================================================ */

/* Returns non-zero if the two IDs are out of order for the given direction */
static int compareByID(StudentRecord a, StudentRecord b, int ascending) {
    return ascending ? (a.id > b.id) : (a.id < b.id);
}

/* Returns non-zero if the two marks are out of order for the given direction */
static int compareByMark(StudentRecord a, StudentRecord b, int ascending) {
    return ascending ? (a.mark > b.mark) : (a.mark < b.mark);
}

/* Sort student records by ID (using optimized bubble sort with early exit) */
static void sortByID(StudentRecord* records, int count, int ascending) {
    for (int i = 0; i < count - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < count - i - 1; j++) {
            if (compareByID(records[j], records[j + 1], ascending)) {
                StudentRecord temp = records[j];
                records[j] = records[j + 1];
                records[j + 1] = temp;
                swapped = 1;
            }
        }
        /* If we went through a full pass with no swaps, the array is sorted */
        if (!swapped) break;
    }
}

/* Sort student records by Mark (using optimized bubble sort with early exit) */
static void sortByMark(StudentRecord* records, int count, int ascending) {
    for (int i = 0; i < count - 1; i++) {
        /* Track whether any swap happened during this pass */
        int swapped = 0;
        for (int j = 0; j < count - i - 1; j++) {
            if (compareByMark(records[j], records[j + 1], ascending)) {
                StudentRecord temp = records[j];
                records[j] = records[j + 1];
                records[j + 1] = temp;
                swapped = 1;
            }
        }
        /* If no swaps occurred in this pass, the array is already sorted */
        if (!swapped) break;
    }
}

/* ============================================================================
 * Display with Sorting
 * ============================================================================ */

void showAllSorted(Database* db, const char* sortBy) {
    if (db->count == 0) {
        printf("CMS: No records found in the database.\n");
        return;
    }
    
    StudentRecord workingCopy[MAX_RECORDS];
    memcpy(workingCopy, db->records, sizeof(StudentRecord) * db->count);
    
    /* Apply sorting based on requested field and direction */
    if (sortBy) {
        if (strcmp(sortBy, "ID") == 0 || strcmp(sortBy, "ID_ASC") == 0) {
            sortByID(workingCopy, db->count, 1);
        } else if (strcmp(sortBy, "ID_DESC") == 0) {
            sortByID(workingCopy, db->count, 0);
        } else if (strcmp(sortBy, "MARK") == 0 || strcmp(sortBy, "MARK_ASC") == 0) {
            sortByMark(workingCopy, db->count, 1);
        } else if (strcmp(sortBy, "MARK_DESC") == 0) {
            sortByMark(workingCopy, db->count, 0);
        }
    }
    
    /* Display records with simple pagination (20 per page) */
    printf("CMS: Here are all the records found in the table \"StudentRecords\" (sorted view).\n");

    const int PAGE_SIZE = 20;
    char input[32];
    int displayed = 0;
    
    printf("%-10s %-25s %-30s %-10s\n", "ID", "Name", "Programme", "Mark");
    printf("-------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < db->count; i++) {
        printf("%-10d %-25s %-30s %-10.1f\n",
               workingCopy[i].id,
               workingCopy[i].name,
               workingCopy[i].programme,
               workingCopy[i].mark);
        displayed++;

        if ((i + 1) % PAGE_SIZE == 0 && (i + 1) < db->count) {
            /* Pause after each page and allow the user to cancel the listing */
            printf("-------------------------------------------------------------------------------\n");
            printf("-- Showing %d of %d record(s). Press Enter to continue, or type 'cancel' to stop --\n",
                   i + 1, db->count);
            if (!fgets(input, sizeof(input), stdin)) {
                break;
            }
            trimWhitespace(input);
            if (strcasecmp(input, "cancel") == 0) {
                printf("CMS: Sorted listing cancelled by user.\n");
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

/* ============================================================================
 * Command Parsing for Sorting
 * ============================================================================ */

/* Parse and execute "SHOW ALL SORT BY ..." commands. */
/* Returns: 1 if valid and executed, 0 if invalid, -1 if not a sort command. */
int parseAndExecuteSort(Database* db, const char* command) {
    /* Convert to uppercase for comparison */
    char upperCmd[512];
    strncpy(upperCmd, command, 511);
    upperCmd[511] = '\0';
    toUpperCase(upperCmd);
    
    /* Check if it's a SHOW ALL SORT BY command */
    if (strncmp(upperCmd, "SHOW ALL SORT BY", 16) != 0) {
        return -1; /* Not a sort command */
    }
    
    const char* params = upperCmd + 16;
    while (*params == ' ') params++;
    
    /* Parse field (ID or MARK) */
    char field[20] = "";
    if (strncmp(params, "ID", 2) == 0) {
        strcpy(field, "ID");
        params += 2;
    } else if (strncmp(params, "MARK", 4) == 0) {
        strcpy(field, "MARK");
        params += 4;
    } else {
        printf("CMS: Invalid field.\nUsage: SHOW ALL SORT BY <FIELD> <ORDER>\n");
        printf("       FIELD: ID or MARK\n");
        printf("       ORDER: ASC or DESC (optional, defaults to ASC)\n");
        return 0;
    }
    
    /* Skip whitespace */
    while (*params == ' ') params++;
    
    /* Parse order (ASC or DESC) - optional */
    char order[10] = "ASC"; /* Default */
    if (strlen(params) > 0) {
        if (strncmp(params, "ASC", 3) == 0) {
            strcpy(order, "ASC");
        } else if (strncmp(params, "DESC", 4) == 0) {
            strcpy(order, "DESC");
        } else {
            printf("CMS: Invalid field.\nUsage: SHOW ALL SORT BY <FIELD> <ORDER>\n");
        printf("       FIELD: ID or MARK\n");
        printf("       ORDER: ASC or DESC (optional, defaults to ASC)\n");
            return 0;
        }
    }
    
    /* Build sort string and execute */
    char sortBy[50];
    sprintf(sortBy, "%s_%s", field, order);
    showAllSorted(db, sortBy);
    
    return 1;
}

/* ============================================================================
 * Filtering by Mark
 * ============================================================================ */

void showAllFilteredByMark(Database* db, const char* op, float value) {
    if (db->count == 0) {
        printf("CMS: No records found.\n");
        return;
    }
    
    const float EPSILON = 1e-6f;
    int totalMatches = 0;
    int displayed = 0;
    int headerPrinted = 0;
    
    /* Scan and display matching records */
    for (int i = 0; i < db->count; i++) {
        float mark = db->records[i].mark;
        int match = 0;
        
        /* Check condition */
        if (strcmp(op, "=") == 0) {
            match = (fabsf(mark - value) <= EPSILON);
        } else if (strcmp(op, "!=") == 0) {
            match = (fabsf(mark - value) > EPSILON);
        } else if (strcmp(op, "<") == 0) {
            match = (mark < value);
        } else if (strcmp(op, ">") == 0) {
            match = (mark > value);
        } else if (strcmp(op, "<=") == 0) {
            match = (mark <= value);
        } else if (strcmp(op, ">=") == 0) {
            match = (mark >= value);
        }
        
        if (match) {
            totalMatches++;
            
            if (!headerPrinted) {
                printf("CMS: Records where Mark %s %.1f.\n", op, value);
                printf("%-10s %-25s %-30s %-10s\n", "ID", "Name", "Programme", "Mark");
                printf("-------------------------------------------------------------------------------\n");
                headerPrinted = 1;
            }
            
            printf("%-10d %-25s %-30s %-10.1f\n",
                   db->records[i].id,
                   db->records[i].name,
                   db->records[i].programme,
                   db->records[i].mark);
            displayed++;

            if (displayed % 20 == 0) {
                char input[32];
                printf("-------------------------------------------------------------------------------\n");
                printf("-- Showing %d of %d matching record(s). Press Enter to continue, or type 'cancel' to stop --\n",
                       displayed, totalMatches);
                if (!fgets(input, sizeof(input), stdin)) {
                    break;
                }
                trimWhitespace(input);
                if (strcasecmp(input, "cancel") == 0) {
                    printf("CMS: Filtered listing cancelled by user.\n");
                    printf("CMS: Displayed %d of %d matching record(s).\n", displayed, totalMatches);
                    return;
                }
                printf("\n%-10s %-25s %-30s %-10s\n", "ID", "Name", "Programme", "Mark");
                printf("-------------------------------------------------------------------------------\n");
            }
        }
    }
    
    if (totalMatches == 0) {
        printf("CMS: No records found that satisfy MARK %s %.1f\n", op, value);
    } else {
        printf("-------------------------------------------------------------------------------\n");
        printf("CMS: Displayed %d of %d matching record(s).\n", displayed, totalMatches);
    }
}

/* ============================================================================
 * Command Parsing for Mark Filtering
 * ============================================================================ */

int parseAndExecuteMarkFilter(Database* db, const char* command) {
    /* Make uppercase copy for detection */
    char upperCmd[512];
    strncpy(upperCmd, command, 511);
    upperCmd[511] = '\0';
    toUpperCase(upperCmd);
    
    const char* prefix = "SHOW ALL WHERE MARK";
    if (strncmp(upperCmd, prefix, strlen(prefix)) != 0) {
        return -1; /* Not a mark filter command */
    }
    
    /* Point to remainder in original command */
    const char* params = command + strlen(prefix);
    while (*params == ' ') params++;
    
    if (*params == '\0') {
        printf("CMS: Missing operator and value. \nUsage: SHOW ALL WHERE MARK <op> <value>\n");
        printf("       Operators: =, !=, <, >, <=, >=\n");
        return 0;
    }
    
    /* Parse operator */
    char op[3] = {0};
    if (strncmp(params, "<=", 2) == 0 || strncmp(params, ">=", 2) == 0 || 
        strncmp(params, "!=", 2) == 0) {
        op[0] = params[0];
        op[1] = params[1];
        params += 2;
    } else if (*params == '=' || *params == '<' || *params == '>') {
        op[0] = *params;
        params += 1;
    } else {
        printf("CMS: Invalid operator. Allowed: =, !=, <, >, <=, >=\n");
        return 0;
    }
    
    /* Skip whitespace before value */
    while (*params == ' ') params++;
    
    if (*params == '\0') {
        printf("CMS: Missing value.\nUsage: SHOW ALL WHERE MARK <op> <value>\n");
        return 0;
    }
    
    /* Parse numeric value */
    char* endptr = NULL;
    double val = strtod(params, &endptr);
    if (params == endptr) {
        printf("CMS: Invalid numeric value for mark: \"%s\"\n", params);
        return 0;
    }
    
    /* Check for extra characters */
    const char* tail = endptr;
    while (*tail == ' ') tail++;
    if (*tail != '\0') {
        printf("CMS: Unexpected text after numeric value: \"%s\"\n", tail);
        return 0;
    }
    
    showAllFilteredByMark(db, op, (float)val);
    return 1;
}

/* ============================================================================
 * Statistical Analysis
 * ============================================================================ */

void showSummary(Database* db) {
    if (db->count == 0) {
        printf("CMS: No records in the database.\n");
        return;
    }
    
    /* Initialize tracking variables */
    float totalMarks = 0;
    float highestMark = db->records[0].mark;
    float lowestMark = db->records[0].mark;
    int highestIndex = 0;
    int lowestIndex = 0;
    int passCount = 0;
    
    /* Programme tracking arrays */
    char programmes[MAX_RECORDS][MAX_PROGRAMME_LEN];
    int programmeCounts[MAX_RECORDS] = {0};
    float programmeTotals[MAX_RECORDS] = {0};
    int uniqueProgrammes = 0;
    
    /* First pass: collect basic statistics */
    for (int i = 0; i < db->count; i++) {
        float mark = db->records[i].mark;
        totalMarks += mark;
        
        if (mark > highestMark) {
            highestMark = mark;
            highestIndex = i;
        }
        
        if (mark < lowestMark) {
            lowestMark = mark;
            lowestIndex = i;
        }
        
        if (mark >= PASS_MARK) {
            passCount++;
        }
        
        /* Track programmes */
        int found = 0;
        for (int j = 0; j < uniqueProgrammes; j++) {
            if (strcmp(programmes[j], db->records[i].programme) == 0) {
                programmeCounts[j]++;
                programmeTotals[j] += mark;
                found = 1;
                break;
            }
        }
        
        if (!found && uniqueProgrammes < MAX_RECORDS) {
            strcpy(programmes[uniqueProgrammes], db->records[i].programme);
            programmeCounts[uniqueProgrammes] = 1;
            programmeTotals[uniqueProgrammes] = mark;
            uniqueProgrammes++;
        }
    }
    
    /* Calculate statistics */
    float average = totalMarks / db->count;
    float passRate = (passCount * 100.0) / db->count;
    
    /* Display summary */
    printf("-------------------------------------------------------------------------------\n");
    printf("CMS: Summary Statistics\n");
    printf("-------------------------------------------------------------------------------\n");
    
    /* Overall statistics */
    printf("OVERALL STATISTICS\n");
    printf("-------------------------------------------------------------------------------\n");
    printf("Total number of students: %d\n", db->count);
    printf("Average mark: %.2f\n", average);
    printf("Highest mark: %.1f (%s)\n", highestMark, db->records[highestIndex].name);
    printf("Lowest mark: %.1f (%s)\n", lowestMark, db->records[lowestIndex].name);
    printf("Mark range: %.1f\n", highestMark - lowestMark);
    printf("\n");
    /* Pass/Fail breakdown */
    printf("-------------------------------------------------------------------------------\n");
    printf("PASS/FAIL BREAKDOWN\n");
    printf("-------------------------------------------------------------------------------\n");
    printf("Students passing (>= %.0f): %d (%.1f%%)\n", PASS_MARK, passCount, passRate);
    printf("Students failing (< %.0f): %d (%.1f%%)\n", PASS_MARK, 
           db->count - passCount, 100.0 - passRate);
    printf("\n");
    
    /* Grade distribution */
    printf("-------------------------------------------------------------------------------\n");
    printf("GRADE DISTRIBUTION\n");
    printf("-------------------------------------------------------------------------------\n");
    
    int gradeCount[5] = {0};
    for (int i = 0; i < db->count; i++) {
        float mark = db->records[i].mark;
        if (mark >= 80) gradeCount[0]++;
        else if (mark >= 70) gradeCount[1]++;
        else if (mark >= 60) gradeCount[2]++;
        else if (mark >= 50) gradeCount[3]++;
        else gradeCount[4]++;
    }
    
    const char* grades[] = {"A", "B", "C", "D", "F"};
    const char* ranges[] = {"80-100", "70-79", "60-69", "50-59", "0-49"};
    
    for (int i = 0; i < 5; i++) {
        float percentage = (gradeCount[i] * 100.0) / db->count;
        printf("  %s    %-9s  %4d   ", grades[i], ranges[i], gradeCount[i]);
        
        for (int j = 0; j < gradeCount[i]; j++) {
            printf("=");
        }
        printf(" (%.1f%%)\n", percentage);
    }
    printf("\n");
    
    /* Programme breakdown */
    printf("-------------------------------------------------------------------------------\n");
    printf("PROGRAMME BREAKDOWN\n");
    printf("-------------------------------------------------------------------------------\n");
    
    /* Sort programmes by count (descending) */
    for (int i = 0; i < uniqueProgrammes - 1; i++) {
        for (int j = 0; j < uniqueProgrammes - i - 1; j++) {
            if (programmeCounts[j] < programmeCounts[j + 1]) {
                /* Swap counts */
                int tempCount = programmeCounts[j];
                programmeCounts[j] = programmeCounts[j + 1];
                programmeCounts[j + 1] = tempCount;
                
                /* Swap totals */
                float tempTotal = programmeTotals[j];
                programmeTotals[j] = programmeTotals[j + 1];
                programmeTotals[j + 1] = tempTotal;
                
                /* Swap names */
                char tempName[MAX_PROGRAMME_LEN];
                strcpy(tempName, programmes[j]);
                strcpy(programmes[j], programmes[j + 1]);
                strcpy(programmes[j + 1], tempName);
            }
        }
    }
    
    for (int i = 0; i < uniqueProgrammes; i++) {
        float progAvg = programmeTotals[i] / programmeCounts[i];
        float percentage = (programmeCounts[i] * 100.0) / db->count;
        printf("%-30s: %2d students (%.1f%%) - Avg: %.2f\n", 
               programmes[i], programmeCounts[i], percentage, progAvg);
    }
    printf("\n");
    
    /* Top performers */
    printf("-------------------------------------------------------------------------------\n");
    printf("TOP PERFORMERS\n");
    printf("-------------------------------------------------------------------------------\n");
    
    StudentRecord topPerformers[MAX_RECORDS];
    memcpy(topPerformers, db->records, sizeof(StudentRecord) * db->count);
    sortByMark(topPerformers, db->count, 0); /* Sort descending */
    
    int displayCount = (db->count < TOP_N) ? db->count : TOP_N;
    for (int i = 0; i < displayCount; i++) {
        printf("%d. %-25s (ID: %d) - %.1f\n", 
               i + 1, topPerformers[i].name, topPerformers[i].id, topPerformers[i].mark);
    }
    
    printf("--------------------------------------------------------------------------------\n");
}

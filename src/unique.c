/*
 * unique.c - Unique Features Implementation
 * Provides help, search, duplicates, export, undo/redo, and autosave
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include "unique.h"
#include "utils.h"

/* ============================================================================
 * UNIQUE FEATURE 1: Help and Declaration System
 * ============================================================================ */

/* Print a one-page overview of all commands and rules */
void displayHelp(void) {
    printf("\n");
    printf("===============================================================================\n");
    printf("                    CLASS MANAGEMENT SYSTEM - COMMANDS\n");
    printf("===============================================================================\n");
    
    printf("FILE MANAGEMENT:\n");
    printf("  OPEN [filename]      Open database (files stored in data/)\n");
    printf("  SWITCH filename      Switch to different file\n");
    printf("  CLOSE                Close current database\n");
    printf("  SAVE                 Save changes\n");
    printf("  AUTOSAVE [ON|OFF]    Enable/disable autosave (check status if no param)\n");
    printf("  EXIT                 Exit application\n");
    printf("\n");
    
    printf("DATA OPERATIONS:\n");
    printf("  SHOW                 Show menu with display options\n");
    printf("  SHOW ALL             Display all records\n");
    printf("  SHOW ALL SORT BY [FIELD] [ASC/DESC]    Sort by field and order\n");
    printf("  SHOW ALL WHERE MARK [OP] [VALUE]       Filter by mark (=, !=, <, >, <=, >=)\n");
    printf("  SHOW SUMMARY         Display statistics\n");
    printf("  INSERT ID=xxx        Add new record (include direct/interactive mode)\n");
    printf("  QUERY ID=xxx         Find record by ID\n");
    printf("  UPDATE ID=xxx        Update record (include direct/interactive mode)\n");
    printf("  DELETE ID=xxx        Delete record\n");
    printf("\n");
    printf("  Examples: INSERT ID=2301234 Name=\"John Doe\" Programme=\"CS\" Mark=85.5\n\n");
    
    printf("SEARCH & ANALYTICS:\n");
    printf("  SEARCH <keyword>     Search in all fields (ID, Name, Programme, Mark)\n");
    printf("  EXPORT [filename]    Export to CSV (optional custom name)\n");
    printf("  CHECK DUPLICATES     Detect and remove duplicate records\n");
    printf("\n");
    
    printf("ADVANCED:\n");
    printf("  UNDO                 Undo last operation\n");
    printf("  REDO                 Redo undone operation\n");
    printf("\n");
    
    printf("VALIDATION RULES:\n");
    printf("  ID:         7 digits (YYXXXXX), year 01-25\n");
    printf("  Name:       2-50 chars, letters/spaces/hyphens only\n");
    printf("  Programme:  2+ chars, letters/spaces only\n");
    printf("  Mark:       0-100, numeric only\n");
    printf("\n");
    
    printf("STATUS INDICATORS:\n");
    printf("  P1_2*:  = Unsaved changes\n");
    printf("  P1_2:   = No unsaved changes\n");
    printf("\n");
    printf("===============================================================================\n");
    printf("\n");
}

/* Show the academic integrity / plagiarism declaration once at startup */
void displayDeclaration(void) {
    printf("\n");
    printf("===============================================================================\n");
    printf("                        ACADEMIC INTEGRITY DECLARATION\n");
    printf("===============================================================================\n");
    printf("\n");
    
    printf("  Singapore Institute of Technology - Academic Honesty Policy\n");
    printf("\n");
    printf("  SIT's policy on copying does not allow students to copy source code or\n");
    printf("  assessment solutions from another person, AI, or other sources. It is\n");
    printf("  the students' responsibility to guarantee that their assessment solutions\n");
    printf("  are their own work. Students must also ensure that their work is not\n");
    printf("  accessible by others.\n");
    printf("\n");
    printf("  Where plagiarism is detected, both assessments involved will receive\n");
    printf("  ZERO marks.\n");
    printf("\n");
    
    printf("-------------------------------------------------------------------------------\n");
    printf("                            OUR TEAM'S DECLARATION\n");
    printf("-------------------------------------------------------------------------------\n");
    printf("\n");
    
    printf("  We, the undersigned members of Group P1_2, hereby declare that:\n");
    printf("\n");
    printf("  - We fully understand and agree to the plagiarism policy stated above\n");
    printf("  - We did not copy any code from others or external sources\n");
    printf("  - We did not share our code with others or upload it for public access\n");
    printf("  - We will not share this work publicly in the future\n");
    printf("  - We agree our project will receive zero marks if plagiarism is detected\n");
    printf("  - We did not copy code directly from AI-generated sources\n");
    printf("  - We will not disclose project materials to unauthorized parties\n");
    printf("\n");
    
    printf("-------------------------------------------------------------------------------\n");
    printf("  Declared by: Group P1_2\n");
    printf("\n");
    printf("  Team Members:\n");
    printf("    1. [Member Name 1]\n");
    printf("    2. [Member Name 2]\n");
    printf("    3. [Member Name 3]\n");
    printf("    4. [Member Name 4]\n");
    printf("    5. [Member Name 5]\n");
    printf("\n");
    
    /* Display current date */
    time_t now;
    struct tm* timeInfo;
    char dateStr[80];
    
    time(&now);
    timeInfo = localtime(&now);
    strftime(dateStr, sizeof(dateStr), "%B %d, %Y", timeInfo);
    
    printf("  Declaration Date: %s\n\n", dateStr);
}

/* ============================================================================
 * UNIQUE FEATURE 2: Advanced Keyword Search
 * ============================================================================ */

/* Search for a free-text keyword across ID, name, programme, and mark */
void searchByKeyword(Database* db, const char* searchTerm) {
    if (db->count == 0) {
        printf("CMS: No records in the database to search.\n");
        return;
    }
    
    /* Use a lowercased copy of the search term for case-insensitive matches */
    char lowerSearch[MAX_NAME_LEN];
    strncpy(lowerSearch, searchTerm, MAX_NAME_LEN - 1);
    lowerSearch[MAX_NAME_LEN - 1] = '\0';
    toLowerCase(lowerSearch);
    
    int totalMatches = 0;
    int displayed = 0;
    int headerPrinted = 0;
    
    for (int i = 0; i < db->count; i++) {
        char lowerName[MAX_NAME_LEN];
        char lowerProgramme[MAX_PROGRAMME_LEN];
        char idStr[20], markStr[20];
        
        /* Prepare searchable strings */
        sprintf(idStr, "%d", db->records[i].id);
        sprintf(markStr, "%.1f", db->records[i].mark);
        
        strncpy(lowerName, db->records[i].name, MAX_NAME_LEN - 1);
        lowerName[MAX_NAME_LEN - 1] = '\0';
        toLowerCase(lowerName);
        
        strncpy(lowerProgramme, db->records[i].programme, MAX_PROGRAMME_LEN - 1);
        lowerProgramme[MAX_PROGRAMME_LEN - 1] = '\0';
        toLowerCase(lowerProgramme);
        
        /* Search in all fields */
        if (strstr(idStr, searchTerm) || strstr(lowerName, lowerSearch) || 
            strstr(lowerProgramme, lowerSearch) || strstr(markStr, searchTerm)) {
            
            totalMatches++;
            
            if (!headerPrinted) {
                printf("CMS: Searching for keyword \"%s\" in all fields.\n", 
                       searchTerm);
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
                /* Paginate results so long searches do not flood the screen */
                char input[32];
                printf("-------------------------------------------------------------------------------\n");
                printf("-- Showing %d of %d matching record(s). Press Enter to continue, or type 'cancel' to stop --\n",
                       displayed, totalMatches);
                if (!fgets(input, sizeof(input), stdin)) {
                    break;
                }
                trimWhitespace(input);
                if (strcasecmp(input, "cancel") == 0) {
                    printf("CMS: Search listing cancelled by user.\n");
                    printf("\nCMS: Displayed %d of %d matching record(s).\n", displayed, totalMatches);
                    return;
                }

                printf("\n%-10s %-25s %-30s %-10s\n", "ID", "Name", "Programme", "Mark");
                printf("-------------------------------------------------------------------------------\n");
            }
        }
    }
    
    if (totalMatches == 0) {
        printf("CMS: No records found matching keyword \"%s\".\n", searchTerm);
    } else {
        printf("-------------------------------------------------------------------------------\n");
        printf("\n");
        printf("CMS: Displayed %d of %d matching record(s).\n", displayed, totalMatches);
    }
}

/* ============================================================================
 * UNIQUE FEATURE 3: Duplicate Detection and Removal
 * ============================================================================ */

/* Check if two records are exact copies */
static int isExactDuplicate(StudentRecord* a, StudentRecord* b) {
    return (a->id == b->id &&
            strcmp(a->name, b->name) == 0 &&
            strcmp(a->programme, b->programme) == 0 &&
            fabs(a->mark - b->mark) < 1e-6);
}

void checkDuplicates(Database* db) {
    if (db->count == 0) {
        printf("CMS: No records in the database to check.\n");
        return;
    }
    
    printf("-------------------------------------------------------------------------------\n");
    printf("CMS: Checking for Duplicates\n");
    printf("-------------------------------------------------------------------------------\n");
    
    /* Track records that are removed so we can report them at the end */
    StudentRecord deleted[MAX_RECORDS];
    int deletedCount = 0;
    /* Keep track of which IDs we have already inspected */
    int processedIDs[MAX_RECORDS];
    int processedCount = 0;
    
    for (int i = 0; i < db->count; i++) {
        int id = db->records[i].id;
        
        /* Skip if already processed */
        int alreadyProcessed = 0;
        for (int p = 0; p < processedCount; p++) {
            if (processedIDs[p] == id) {
                alreadyProcessed = 1;
                break;
            }
        }
        if (alreadyProcessed) continue;
        
        processedIDs[processedCount++] = id;
        
        /* Find all records with this ID */
        int positions[MAX_RECORDS];
        int posCount = 0;
        for (int j = 0; j < db->count; j++) {
            if (db->records[j].id == id) {
                positions[posCount++] = j;
            }
        }
        
        if (posCount <= 1) continue;
        
        /* Remove exact duplicates for this ID but keep one copy */
        for (int a = 0; a < posCount; a++) {
            for (int b = a + 1; b < posCount; b++) {
                if (isExactDuplicate(&db->records[positions[a]], &db->records[positions[b]])) {
                    /* Save deleted record */
                    deleted[deletedCount++] = db->records[positions[b]];
                    
                    /* Remove from database */
                    for (int k = positions[b]; k < db->count - 1; k++) {
                        db->records[k] = db->records[k + 1];
                    }
                    db->count--;
                    
                    /* Update positions array */
                    for (int s = b; s < posCount - 1; s++) {
                        positions[s] = positions[s + 1];
                    }
                    posCount--;
                    b--;
                }
            }
        }
        
        /* Report conflicts if same ID still has multiple non-identical records */
        if (posCount > 1) {
            printf("Conflict for ID %d (%d unique records): positions ", id, posCount);
            for (int u = 0; u < posCount; u++) {
                printf("%d", positions[u] + 1);
                if (u < posCount - 1) printf(", ");
            }
            printf("\n");
        }
    }
    
    /* Report deleted duplicates */
    if (deletedCount > 0) {
        printf("\n");
        printf("Exact Duplicates Removed:\n");
        printf("-------------------------------------------------------------------------------\n");
        for (int i = 0; i < deletedCount; i++) {
            printf("- ID %d (%s, %s, %.1f)\n",
                   deleted[i].id, deleted[i].name, 
                   deleted[i].programme, deleted[i].mark);
        }
        printf("-------------------------------------------------------------------------------\n");
        db->isDirty = 1;
        markDatabaseModified(db);
        printf("\n");
        printf("CMS: Database has been modified. Remember to SAVE.\n");
    } else {
        printf("CMS: No exact duplicate records found.\n");
    }
    printf("-------------------------------------------------------------------------------\n");
}

/* ============================================================================
 * UNIQUE FEATURE 4: CSV Export
 * ============================================================================ */

void exportToCSV(Database* db, const char* command) {
    if (db->count == 0) {
        printf("CMS: No records to export.\n");
        return;
    }
    
    char filename[256];
    const char* customName = NULL;
    
    /* Check for custom filename after the EXPORT keyword, if any */
    if (command && strlen(command) > 6) {
        customName = command + 6;
        while (*customName == ' ') customName++;
    }
    
    if (customName && strlen(customName) > 0) {
        strncpy(filename, customName, sizeof(filename) - 5);
        filename[sizeof(filename) - 5] = '\0';
        trimWhitespace(filename);
        
        /* Ensure the file has a .csv extension */
        if (strlen(filename) < 4 || strcmp(filename + strlen(filename) - 4, ".csv") != 0) {
            strcat(filename, ".csv");
        }
    } else {
        strcpy(filename, "StudentRecords.csv");
    }
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("CMS: Error creating export file.\n");
        return;
    }
    
    /* Write CSV header row */
    fprintf(file, "ID,Name,Programme,Mark\n");
    
    /* Write each record as a single CSV line */
    for (int i = 0; i < db->count; i++) {
        fprintf(file, "%d,\"%s\",\"%s\",%.1f\n",
                db->records[i].id,
                db->records[i].name,
                db->records[i].programme,
                db->records[i].mark);
    }
    
    fclose(file);
    printf("-------------------------------------------------------------------------------\n");
    printf("CSV Export Complete\n");
    printf("-------------------------------------------------------------------------------\n");
    printf("File:     %s\n", filename);
    printf("Records:  %d\n", db->count);
    printf("Format:   CSV (Comma-Separated Values)\n");
    printf("Status:   Successfully exported\n");
    printf("-------------------------------------------------------------------------------\n");
}

/* ============================================================================
 * UNIQUE FEATURE 5: Undo/Redo System 
 * ============================================================================ */
#define MAX_HISTORY 10

typedef struct {
    StudentRecord records[MAX_RECORDS];
    int count;
    char filename[MAX_FILENAME_LEN];
    int isValid;
} DatabaseSnapshot;

static DatabaseSnapshot* history = NULL;
static int historyCount = 0;
static int currentPosition = -1;
static int historyInitialized = 0;

void initializeUndoHistory(void) {
    if (historyInitialized) return;
    
    /* Allocate a fixed ring buffer of snapshots for undo/redo */
    history = (DatabaseSnapshot*)calloc(MAX_HISTORY, sizeof(DatabaseSnapshot));
    if (!history) {
        printf("  WARNING: Unable to allocate memory for undo/redo feature.\n");
        printf("           Undo/Redo will be disabled.\n");
        return;
    }
    
    for (int i = 0; i < MAX_HISTORY; i++) {
        history[i].isValid = 0;
    }
    
    historyInitialized = 1;
    
    /* Print a small diagnostic so the user knows memory cost of undo/redo */
    size_t totalSize = sizeof(DatabaseSnapshot) * MAX_HISTORY;
    printf("  Undo/Redo initialized: %d snapshots, %.2f KB total memory\n", 
           MAX_HISTORY, totalSize / 1024.0);
}

void cleanupUndoHistory(void) {
    if (historyInitialized && history) {
        free(history);
        history = NULL;
        historyInitialized = 0;
        historyCount = 0;
        currentPosition = -1;
    }
}

/* Take a snapshot of current database */
static void createSnapshot(DatabaseSnapshot* snapshot, Database* db) {
    if (!snapshot || !db) return;
    
    /* Copy only the active records and basic metadata into the snapshot */
    memcpy(snapshot->records, db->records, sizeof(StudentRecord) * db->count);
    snapshot->count = db->count;
    strncpy(snapshot->filename, db->filename, MAX_FILENAME_LEN - 1);
    snapshot->filename[MAX_FILENAME_LEN - 1] = '\0';
    snapshot->isValid = 1;
}

/* Restore database content from snapshot */
static void restoreSnapshot(Database* db, DatabaseSnapshot* snapshot) {
    if (!db || !snapshot || !snapshot->isValid) return;
    
    /* Overwrite the in-memory database with the stored snapshot */
    memcpy(db->records, snapshot->records, sizeof(StudentRecord) * snapshot->count);
    db->count = snapshot->count;
    db->isDirty = 1;
}

/* New state recorded */
void saveStateForUndo(Database* db) {
    if (!db->isOpen) return;
    
    if (!historyInitialized) {
        initializeUndoHistory();
        if (!historyInitialized) return;
    }
    
    /* Move current position forward to store a new snapshot */
    currentPosition++;
    
    /* Invalidate any redo history ahead of the new position */
    if (currentPosition < historyCount) {
        for (int i = currentPosition; i < historyCount; i++) {
            history[i].isValid = 0;
        }
        historyCount = currentPosition;
    }
    
    /* When buffer is full, drop the oldest snapshot and compact the array */
    if (currentPosition >= MAX_HISTORY) {
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            if (history[i + 1].isValid) {
                memcpy(&history[i], &history[i + 1], sizeof(DatabaseSnapshot));
            } else {
                history[i].isValid = 0;
            }
        }
        currentPosition = MAX_HISTORY - 1;
    }
    
    createSnapshot(&history[currentPosition], db);
    historyCount = currentPosition + 1;
}

void undoLastOperation(Database* db) {
    if (!historyInitialized || !history) {
        printf("CMS: Undo/Redo system not initialized.\n");
        return;
    }
    
    if (currentPosition <= 0) {
        printf("CMS: Nothing to undo.\n");
        return;
    }
    
    /* Step back one snapshot in the history buffer */
    currentPosition--;
    
    if (history[currentPosition].isValid) {
        restoreSnapshot(db, &history[currentPosition]);
        printf("CMS: Undo successful. Restored to previous state.\n");
        printf("     Current record count: %d\n", db->count);
        markDatabaseModified(db);
    } else {
        printf("CMS: Error: Invalid undo state.\n");
    }
}

void redoLastOperation(Database* db) {
    if (!historyInitialized || !history) {
        printf("CMS: Undo/Redo system not initialized.\n");
        return;
    }
    
    if (currentPosition >= historyCount - 1) {
        printf("CMS: Nothing to redo.\n");
        return;
    }
    
    /* Step forward one snapshot in the history buffer */
    currentPosition++;
    
    /* Can only redo if there is snapshot ahead of current position */
    if (history[currentPosition].isValid) {
        restoreSnapshot(db, &history[currentPosition]);
        printf("CMS: Redo successful. Restored to next state.\n");
        printf("     Current record count: %d\n", db->count);
        markDatabaseModified(db);
    } else {
        printf("CMS: Error: Invalid redo state.\n");
    }
}

/* ============================================================================
 * UNIQUE FEATURE 6: Autosave System
 * ============================================================================ */
static struct {
    Database* db;
    int enabled;
    int modified;
    int initialized;
} autosaveState = {NULL, 0, 0, 0};

void initializeAutosave(Database* db) {
    if (autosaveState.initialized) return;
    
    /* Bind autosave to the active database instance */
    autosaveState.db = db;
    autosaveState.enabled = 0;
    autosaveState.modified = 0;
    autosaveState.initialized = 1;
    
    printf("  Autosave initialized (Status: OFF by default)\n");
}

void setAutosaveEnabled(int enabled) {
    if (!autosaveState.initialized) {
        printf("CMS: Autosave not initialized.\n");
        return;
    }
    
    /* Store autosave flag as 0/1 for clarity */
    autosaveState.enabled = enabled ? 1 : 0;
    
    /* If autosave is turned on, immediately flush any pending changes */
    if (autosaveState.enabled && autosaveState.modified) {
        /* Trigger autosave for pending changes */
        if (autosaveState.db && autosaveState.db->isOpen && autosaveState.db->isDirty) {
            saveDatabase(autosaveState.db);
            autosaveState.modified = 0;
            printf("[Autosave triggered]\n");
        }
    }
}

int isAutosaveEnabled(void) {
    return autosaveState.initialized ? autosaveState.enabled : 0;
}

void cleanupAutosave(void) {
    if (!autosaveState.initialized) return;
    
    if (autosaveState.enabled && autosaveState.modified) {
        /* Final autosave before cleanup so no changes are lost */
        if (autosaveState.db && autosaveState.db->isOpen && autosaveState.db->isDirty) {
            saveDatabase(autosaveState.db);
        }
    }
    
    autosaveState.initialized = 0;
    autosaveState.enabled = 0;
    autosaveState.modified = 0;
    autosaveState.db = NULL;
}

/* Indicates database content have changed */
void markDatabaseModified(Database* db) {
    if (!autosaveState.initialized || autosaveState.db != db) return;
    
    autosaveState.modified = 1;
    
    if (autosaveState.enabled) {
        /* Trigger autosave immediately */
        if (db->isOpen && db->isDirty) {
            saveDatabase(db);
            autosaveState.modified = 0;
            printf("[Autosave triggered]\n");
        }
    }
}

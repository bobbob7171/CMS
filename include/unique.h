/*
 * unique.h - Unique Features
 * Advanced features including help, search, duplicates, export, undo/redo, autosave
 */

#ifndef UNIQUE_H
#define UNIQUE_H

#include "fileio.h"

// Feature 1: Help and Declaration
void displayHelp(void);
void displayDeclaration(void);

// Feature 2: Advanced Search
void searchByKeyword(Database* db, const char* searchTerm);

// Feature 3: Duplicate Management
void checkDuplicates(Database* db);

// Feature 4: CSV Export
void exportToCSV(Database* db, const char* command);

// Feature 5: Undo/Redo System
void initializeUndoHistory(void);
void cleanupUndoHistory(void);
void saveStateForUndo(Database* db);
void undoLastOperation(Database* db);
void redoLastOperation(Database* db);

// Feature 6: Autosave System
void initializeAutosave(Database* db);
void setAutosaveEnabled(int enabled);
int isAutosaveEnabled(void);
void cleanupAutosave(void);
void markDatabaseModified(Database* db);

#endif
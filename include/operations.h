/*
 * operations.h - Core CRUD Operations
 * Provides interactive and direct command interfaces for database operations
 */

#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "fileio.h"

/* Operation result codes */
#define OP_SUCCESS 0
#define OP_CANCELLED 1
#define OP_ERROR 2
#define OP_EXIT 3

/* Display operations */
void showAll(Database* db);
void showMenu(Database* db);

/* CRUD operations - Interactive mode */
int insertRecordInteractive(Database* db, int id);
int updateRecordInteractive(Database* db, int id);

/* CRUD operations - Direct command mode */
int insertRecordDirect(Database* db, const char* params);
int updateRecordDirect(Database* db, int id, const char* params);
void queryRecord(Database* db, int id);
int deleteRecord(Database* db, int id);

/* Helper functions */
int findRecordIndex(Database* db, int id);
void displayRecordSummary(StudentRecord* rec);

#endif

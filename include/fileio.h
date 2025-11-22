/*
 * fileio.h - Database File I/O and Validation
 * Handles database file operations, record validation, and data integrity
 */

#ifndef FILEIO_H
#define FILEIO_H

/* Database configuration constants */
#define MAX_RECORDS 100
#define MAX_NAME_LEN 100
#define MAX_PROGRAMME_LEN 100
#define MAX_LINE_LEN 512
#define MAX_FILENAME_LEN 256

/* Student record structure */
typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    char programme[MAX_PROGRAMME_LEN];
    float mark;
    int hasName;
    int hasProgramme;
    int hasMark;
} StudentRecord;

/* Database structure */
typedef struct {
    StudentRecord records[MAX_RECORDS];
    int count;
    int isOpen;
    int isDirty;
    char filename[MAX_FILENAME_LEN];
} Database;

/* Database operations */
void initDatabase(Database* db);
int openDatabase(Database* db, const char* filename);
int saveDatabase(Database* db);
void closeDatabase(Database* db);
int switchDatabase(Database* db, const char* newFilename);

/* Validation functions */
int validateStudentID(int id, char* errorMsg);
int validateName(const char* name, char* errorMsg);
int validateProgramme(const char* programme, char* errorMsg);
int validateMark(const char* markStr, float* outMark, char* errorMsg);

/* Data integrity */
int checkDuplicateIDs(Database* db);

#endif

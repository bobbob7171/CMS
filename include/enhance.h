/*
 * enhance.h - Enhancement Features
 * Sorting, filtering, and statistical analysis capabilities
 */

#ifndef ENHANCE_H
#define ENHANCE_H

#include "fileio.h"

/* Sorting operations */
void showAllSorted(Database* db, const char* sortBy);
int parseAndExecuteSort(Database* db, const char* command);

/* Filtering operations */
void showAllFilteredByMark(Database* db, const char* op, float value);
int parseAndExecuteMarkFilter(Database* db, const char* command);

/* Statistical analysis */
void showSummary(Database* db);

#endif

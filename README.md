# Class Management System (CMS) - Group P2_2

A comprehensive command-line application for managing student records with advanced features including sorting, filtering, statistical analysis, search, undo/redo, and autosave capabilities.

## 📋 Table of Contents

- [Features](#features)
- [Project Structure](#project-structure)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Commands Reference](#commands-reference)
- [Validation Rules](#validation-rules)
- [Advanced Features](#advanced-features)
- [Data Format](#data-format)
- [Examples](#examples)
- [Team](#team)

## ✨ Features

### Core Functionality
- **CRUD Operations**: Create, Read, Update, and Delete student records
- **File Management**: Open, switch, save, and close database files
- **Interactive & Direct Modes**: Commands support both interactive prompts and direct parameter input

### Enhancement Features
- **Sorting**: Sort records by ID or Mark in ascending/descending order
- **Filtering**: Filter records by mark using comparison operators (=, !=, <, >, <=, >=)
- **Statistical Analysis**: Comprehensive summary statistics including:
  - Overall statistics (average, highest, lowest marks)
  - Pass/Fail breakdown
  - Grade distribution
  - Programme breakdown
  - Top performers

### Unique Features
1. **Help System**: Complete command reference and academic integrity declaration
2. **Advanced Search**: Keyword search across all fields (ID, Name, Programme, Mark)
3. **Duplicate Management**: Detect and remove duplicate records
4. **CSV Export**: Export database to CSV format
5. **Undo/Redo System**: Revert or reapply up to 10 operations
6. **Autosave**: Optional automatic saving after each modification

## 📁 Project Structure

```
P2_2-CMS/
├── include/
│   ├── enhance.h       # Sorting, filtering, and statistics
│   ├── fileio.h        # Database file I/O and validation
│   ├── operations.h    # Core CRUD operations
│   ├── unique.h        # Advanced features (search, export, undo/redo)
│   └── utils.h         # Utility functions
├── src/
│   ├── main.c          # Main program and command processing
│   ├── enhance.c       # Enhancement features implementation
│   ├── fileio.c        # File I/O implementation
│   ├── operations.c    # CRUD operations implementation
│   ├── unique.c        # Unique features implementation
│   └── utils.c         # Utility functions implementation
├── data/
│   ├── P2_2-CMS.txt    # Default database file
│   └── testdata.txt    # Sample test data
└── README.md
```

## 🔧 Requirements

- **Compiler**: GCC or any C compiler supporting C99 standard
- **Operating System**: Windows, Linux, or macOS
- **Memory**: Minimum 50MB RAM
- **Disk Space**: ~1MB for program and data files

## 🚀 Installation

### Compile the Program

**Linux/macOS:**
```bash
gcc -I./include -o cms src/*.c -lm
```

**Windows:**
```cmd
gcc -I.\include -o cms.exe src\*.c -lm
```

### Create Data Directory

Ensure the `data/` directory exists in the same location as the executable:

```bash
mkdir -p data
```

## 💻 Usage

### Start the Program

```bash
./cms
```

### Basic Workflow

1. **Open a database**:
   ```
   OPEN P2_2-CMS.txt
   ```

2. **View records**:
   ```
   SHOW ALL
   ```

3. **Insert a record**:
   ```
   INSERT ID=2301234 Name="John Doe" Programme="Computer Science" Mark=85.5
   ```

4. **Save changes**:
   ```
   SAVE
   ```

5. **Exit**:
   ```
   EXIT
   ```

## 📖 Commands Reference

### File Management

| Command | Description | Example |
|---------|-------------|---------|
| `OPEN [filename]` | Open database file | `OPEN P2_2-CMS.txt` |
| `SWITCH filename` | Switch to different file | `SWITCH testdata.txt` |
| `CLOSE` | Close current database | `CLOSE` |
| `SAVE` | Save changes | `SAVE` |
| `AUTOSAVE [ON\|OFF]` | Enable/disable autosave | `AUTOSAVE ON` |
| `EXIT` | Exit application | `EXIT` |

### Data Operations

| Command | Description | Example |
|---------|-------------|---------|
| `SHOW` | Display menu | `SHOW` |
| `SHOW ALL` | Display all records | `SHOW ALL` |
| `SHOW ALL SORT BY [FIELD] [ORDER]` | Sort records | `SHOW ALL SORT BY MARK DESC` |
| `SHOW ALL WHERE MARK [OP] [VALUE]` | Filter by mark | `SHOW ALL WHERE MARK >= 70` |
| `SHOW SUMMARY` | Display statistics | `SHOW SUMMARY` |
| `INSERT ID=xxx [params]` | Add new record | `INSERT ID=2301234` |
| `QUERY ID=xxx` | Find record by ID | `QUERY ID=2301234` |
| `UPDATE ID=xxx [params]` | Update record | `UPDATE ID=2301234 Mark=90` |
| `DELETE ID=xxx` | Delete record | `DELETE ID=2301234` |

### Advanced Commands

| Command | Description | Example |
|---------|-------------|---------|
| `SEARCH <keyword>` | Search all fields | `SEARCH Computer` |
| `EXPORT [filename]` | Export to CSV | `EXPORT records.csv` |
| `CHECK DUPLICATES` | Find duplicates | `CHECK DUPLICATES` |
| `UNDO` | Undo last operation | `UNDO` |
| `REDO` | Redo operation | `REDO` |
| `HELP` | Display help | `HELP` |

## ✅ Validation Rules

### Student ID
- **Format**: 7 digits (YYXXXXX)
- **Year Range**: 01-25 (representing 2001-2025)
- **Example**: 2301234 (enrolled in 2023)

### Name
- **Length**: 2-50 characters
- **Allowed**: Letters, spaces, hyphens (-), apostrophes ('), periods (.)
- **Required**: At least one letter

### Programme
- **Length**: Minimum 2 characters
- **Allowed**: Letters, spaces, hyphens (-), ampersands (&)
- **Required**: At least one letter

### Mark
- **Range**: 0.0 - 100.0
- **Format**: Numeric (integer or decimal)

## 🎯 Advanced Features

### Sorting
Sort records by ID or Mark in ascending/descending order with optimized bubble sort algorithm:
```
SHOW ALL SORT BY ID ASC
SHOW ALL SORT BY MARK DESC
```

### Filtering
Filter records using comparison operators:
```
SHOW ALL WHERE MARK >= 70    # Pass marks
SHOW ALL WHERE MARK < 50     # Failing students
SHOW ALL WHERE MARK = 100    # Perfect scores
```

### Statistical Analysis
Get comprehensive statistics:
- Overall averages and ranges
- Pass/fail breakdown (passing mark: 50.0)
- Grade distribution (A: 80-100, B: 70-79, C: 60-69, D: 50-59, F: 0-49)
- Programme breakdown with averages
- Top 3 performers

### Undo/Redo System
- Stores up to 10 operation snapshots
- Revert changes with `UNDO`
- Reapply changes with `REDO`
- Memory usage: ~150KB

### Autosave
- Toggle with `AUTOSAVE ON` or `AUTOSAVE OFF`
- Automatically saves after each modification
- Check status with `AUTOSAVE`

## 📄 Data Format

Database files must follow this format:

```
Database Name: P2_2-CMS
Authors: Assistant Prof Oran Zane Devilly

Table Name: StudentRecords
ID	Name		Programme		Mark
2301234	Joshua Chen	Software Engineering	70.5
2201234	Isaac Teo	Computer Science	63.4
```

**Notes:**
- Fields separated by tabs (`\t`)
- Lines 1-4: Header information
- Line 5: Column names
- Line 6+: Data records

## 📝 Examples

### Example 1: Adding a Student

```
P2_2: INSERT ID=2301234 Name="Alice Wong" Programme="Computer Science" Mark=88.5
CMS: A new record with ID=2301234 is successfully inserted.
-------------------------------------------------------------------------------
Record Summary
-------------------------------------------------------------------------------
ID:         2301234
Name:       Alice Wong
Programme:  Computer Science
Mark:       88.5
-------------------------------------------------------------------------------
```

### Example 2: Interactive Update

```
P2_2: UPDATE ID=2301234
-------------------------------------------------------------------------------
UPDATE ID=2301234
-------------------------------------------------------------------------------
Current Name: Alice Wong
New Name (Enter to keep, 'cancel' to abort): Alice Wang
Current Programme: Computer Science
New Programme (Enter to keep, 'cancel' to abort): 
Current Mark: 88.5
New Mark (Enter to keep, 'cancel' to abort): 92.0
CMS: The record with ID=2301234 is successfully updated.
```

### Example 3: Searching

```
P2_2: SEARCH Computer
CMS: Searching for keyword "Computer" in all fields.
ID         Name                      Programme                      Mark      
-------------------------------------------------------------------------------
2201234    Isaac Teo                 Computer Science               63.4
2301234    Alice Wang                Computer Science               92.0
-------------------------------------------------------------------------------

CMS: Displayed 2 of 2 matching record(s).
```

### Example 4: Exporting to CSV

```
P2_2: EXPORT student_records.csv
-------------------------------------------------------------------------------
CSV Export Complete
-------------------------------------------------------------------------------
File:     student_records.csv
Records:  25
Format:   CSV (Comma-Separated Values)
Status:   Successfully exported
-------------------------------------------------------------------------------
```

## 👥 Team

**Group P2_2**

Team Members:
1. Lee Jia Shin (2503254)
2. Phen Jian Xuan (2500598)
3. Gan Wei Yang (2501043)
4. Pham Anh Bao Khang (2503012)

**Instructor**: Prof Oran Zane Devilly

**Institution**: Singapore Institute of Technology

## 🔒 Academic Integrity

This project is developed in accordance with SIT's Academic Honesty Policy. All code is original work by the team members. We do not allow copying, sharing, or unauthorized distribution of this code.

## 📊 Status Indicators

- `P2_2:` - No unsaved changes
- `P2_2*:` - Unsaved changes present

## ⚙️ Configuration

### Constants (fileio.h)
- `MAX_RECORDS`: 100 (maximum records in database)
- `MAX_NAME_LEN`: 100 (maximum name length)
- `MAX_PROGRAMME_LEN`: 100 (maximum programme name length)

### Undo/Redo (unique.c)
- `MAX_HISTORY`: 10 (maximum undo/redo snapshots)

### Display
- Page size: 20 records per page
- Automatic pagination with user prompts

## 🐛 Error Handling

The system includes comprehensive error handling:
- Input validation for all fields
- File operation error checking
- Duplicate ID detection
- Database capacity warnings
- Invalid command notifications
- Memory allocation verification

## 📞 Support

For issues or questions:
1. Type `HELP` in the application for command reference
2. Check validation rules for data entry errors
3. Review the examples section for usage patterns

---

**Version**: 1.0  
**Last Updated**: November 2025  
**License**: Academic Use Only
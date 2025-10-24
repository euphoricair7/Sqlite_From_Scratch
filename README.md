# SQLite From Scratch

A simple database implementation in C, inspired by SQLite, with support for basic insert and select operations.

## Features

- **Insert Command**: Add records with ID, username, and email
- **Select Command**: Retrieve all stored records
- **Data Validation**: 
  - ID must be positive
  - Username limited to 32 characters
  - Email limited to 255 characters
- **Persistent Storage**: Data saved to disk using a custom pager system
- **Meta Commands**: `.exit` to quit the database

## Building the Database

### Compile the C program
```bash
gcc -o maincode maincode.c
```

### Compile with extra warnings (recommended)
```bash
gcc -Wall -Wextra -std=c11 -o maincode maincode.c
```

## Running the Database

### Start the database
```bash
./maincode
```

### Start with a specific database file
```bash
./maincode mydb.db
```

## Available Commands

### Database Commands

#### Insert a record
```
insert 1 username email@example.com
```

#### Select all records
```
select
```

### Meta Commands

#### Exit the database
```
.exit
```

## Testing

### Run the Python test suite
```bash
python3 test_database.py
```

This will run comprehensive tests covering:
- Basic insert and select operations
- Error conditions (negative IDs, strings too long, syntax errors)
- Storage limits (table full scenarios)
- Edge cases (boundary values, maximum length strings)

### Run tests with verbose output
```bash
python3 test_database.py -v
```

## Example Usage

```bash
$ ./maincode mydb.db
db > insert 1 john john@example.com
Executed.
db > insert 2 jane jane@example.com
Executed.
db > select
(1, john, john@example.com)
(2, jane, jane@example.com)
Executed.
db > .exit
```

## Error Handling

The database handles various error conditions:

### Negative ID
```bash
db > insert -1 user email@test.com
ID must be positive.
```

### String too long
```bash
db > insert 1 verylongusernamethatexceedsthirtytwocharacters email@test.com
String is too long.
```

### Syntax errors
```bash
db > insert 1
Syntax error. Could not parse statement.
```

### Table full
```bash
db > insert 1401 user email@test.com
Error: Table full.
```

## Data Limits

- **Maximum rows**: 1400 rows (100 pages × 14 rows per page)
- **Maximum username length**: 32 characters
- **Maximum email length**: 255 characters
- **Page size**: 4096 bytes

## File Structure

```
.
├── maincode.c          # Main database implementation
├── maincode            # Compiled executable
├── test_database.py    # Comprehensive Python test suite
├── run_tests.sh        # Test runner script
└── README.md           # This file
```

## Development

### Clean build
```bash
rm -f maincode *.db
gcc -o maincode maincode.c
```

### Run with debugging symbols
```bash
gcc -g -Wall -Wextra -std=c11 -o maincode maincode.c
gdb ./maincode
```

### Check for memory leaks
```bash
valgrind --leak-check=full ./maincode test.db
```

## Technical Details

### Row Structure
- **ID**: 4 bytes (uint32_t)
- **Username**: 33 bytes (32 chars + null terminator)
- **Email**: 256 bytes (255 chars + null terminator)
- **Total**: 293 bytes per row

### Page Structure
- **Page size**: 4096 bytes
- **Rows per page**: 14 rows
- **Total pages**: 100 pages

### Storage Format
Data is persisted to disk using a custom pager system that manages memory pages and file I/O.

## License

See LICENSE file for details.
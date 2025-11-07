#!/bin/bash

DB_FILE="show_tables_test.db"
EXECUTABLE="./maincode"

# Clean up old database file
rm -f $DB_FILE

# Compile the code
gcc -o $EXECUTABLE maincode.c

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

# Run the test script
$EXECUTABLE $DB_FILE <<EOF
show tables
create table users
create table products
show tables
.exit
EOF

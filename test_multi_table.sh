#!/bin/bash

DB_FILE="mydb.db"
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
create table users
insert into default 1 "default_user" "default@example.com"
insert into users 1 "new_user" "new@example.com"
select * from default
select * from users
.exit
EOF

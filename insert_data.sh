#!/bin/zsh

# Script to insert sample data into SQLite database

# Check if file path argument is provided
if [ $# -lt 1 ]; then
  echo "Usage: $0 <database_file> [test_type]"
  echo "Example: $0 mydb.db"
  echo "Optional test_type: simple (default), four_node_btree"
  exit 1
fi

DB_FILE=$1
TEST_TYPE="${2:-simple}"
COMMANDS=""

# Remove existing database file if it exists
if [ -f "$DB_FILE" ]; then
  rm "$DB_FILE"
  echo "Removed existing database file."
fi

# Make sure the program is compiled
if [ ! -x "./maincode" ]; then
  echo "Compiling database program..."
  gcc -o maincode maincode.c
  if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
  fi
fi

if [ "$TEST_TYPE" = "simple" ]; then
  echo "Running simple insertion test (15 records)..."
  # Generate insert commands for users 1-15
  for i in {1..15}; do
    COMMANDS="${COMMANDS}insert $i user$i person$i@example.com\n"
  done

elif [ "$TEST_TYPE" = "four_node_btree" ]; then
  echo "Running 4-leaf-node btree test (30 records)..."
  # Insert many values in a specific order for 4-leaf-node btree
  COMMANDS="${COMMANDS}insert 18 user18 person18@example.com\n"
  COMMANDS="${COMMANDS}insert 7 user7 person7@example.com\n"
  COMMANDS="${COMMANDS}insert 10 user10 person10@example.com\n"
  COMMANDS="${COMMANDS}insert 29 user29 person29@example.com\n"
  COMMANDS="${COMMANDS}insert 23 user23 person23@example.com\n"
  COMMANDS="${COMMANDS}insert 4 user4 person4@example.com\n"
  COMMANDS="${COMMANDS}insert 14 user14 person14@example.com\n"
  COMMANDS="${COMMANDS}insert 30 user30 person30@example.com\n"
  COMMANDS="${COMMANDS}insert 15 user15 person15@example.com\n"
  COMMANDS="${COMMANDS}insert 26 user26 person26@example.com\n"
  COMMANDS="${COMMANDS}insert 22 user22 person22@example.com\n"
  COMMANDS="${COMMANDS}insert 19 user19 person19@example.com\n"
  COMMANDS="${COMMANDS}insert 2 user2 person2@example.com\n"
  COMMANDS="${COMMANDS}insert 1 user1 person1@example.com\n"
  COMMANDS="${COMMANDS}insert 21 user21 person21@example.com\n"
  COMMANDS="${COMMANDS}insert 11 user11 person11@example.com\n"
  COMMANDS="${COMMANDS}insert 6 user6 person6@example.com\n"
  COMMANDS="${COMMANDS}insert 20 user20 person20@example.com\n"
  COMMANDS="${COMMANDS}insert 5 user5 person5@example.com\n"
  COMMANDS="${COMMANDS}insert 8 user8 person8@example.com\n"
  COMMANDS="${COMMANDS}insert 9 user9 person9@example.com\n"
  COMMANDS="${COMMANDS}insert 3 user3 person3@example.com\n"
  COMMANDS="${COMMANDS}insert 12 user12 person12@example.com\n"
  COMMANDS="${COMMANDS}insert 27 user27 person27@example.com\n"
  COMMANDS="${COMMANDS}insert 17 user17 person17@example.com\n"
  COMMANDS="${COMMANDS}insert 16 user16 person16@example.com\n"
  COMMANDS="${COMMANDS}insert 13 user13 person13@example.com\n"
  COMMANDS="${COMMANDS}insert 24 user24 person24@example.com\n"
  COMMANDS="${COMMANDS}insert 25 user25 person25@example.com\n"
  COMMANDS="${COMMANDS}insert 28 user28 person28@example.com\n"
  COMMANDS="${COMMANDS}.btree\n" # Show the btree structure
fi

# Add select command to view data
COMMANDS="${COMMANDS}select\n"
# Add exit to terminate the program
COMMANDS="${COMMANDS}.exit\n"

echo -e $COMMANDS | ./maincode $DB_FILE
# Run the database program with the commands using a temp file so we can
# capture the program exit status reliably (pipes can hide exit codes).
TMP_IN=$(mktemp /tmp/sqlite_input.XXXXXX)
printf "%b" "$COMMANDS" > "$TMP_IN"
./maincode "$DB_FILE" < "$TMP_IN"
EXIT_CODE=$?
rm -f "$TMP_IN"

if [ $EXIT_CODE -ne 0 ]; then
  echo "Program exited with code $EXIT_CODE. Insertion may have failed."
  echo "If you saw a message like 'Need to implement updating parent after split',"
  echo "that means the insertion hit an unimplemented code path and the program"
  echo "exited before persisting the final state. The DB file may be incomplete."
  exit $EXIT_CODE
fi

echo "Test completed successfully!"
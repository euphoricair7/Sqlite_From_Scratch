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
    COMMANDS="${COMMANDS}insert into default $i \"user$i\" \"person$i@example.com\"\n"
  done

elif [ "$TEST_TYPE" = "seven_leaf_btree" ]; then
  echo "Running 7-leaf-node btree test (64 records)..."
  # Insert records in specific order for 7-leaf-node btree structure
  RECORDS=(
    "58 user58 person58@example.com"
    "56 user56 person56@example.com"
    "8 user8 person8@example.com"
    "54 user54 person54@example.com"
    "77 user77 person77@example.com"
    "7 user7 person7@example.com"
    "25 user25 person25@example.com"
    "71 user71 person71@example.com"
    "13 user13 person13@example.com"
    "22 user22 person22@example.com"
    "53 user53 person53@example.com"
    "51 user51 person51@example.com"
    "59 user59 person59@example.com"
    "32 user32 person32@example.com"
    "36 user36 person36@example.com"
    "79 user79 person79@example.com"
    "10 user10 person10@example.com"
    "33 user33 person33@example.com"
    "20 user20 person20@example.com"
    "4 user4 person4@example.com"
    "35 user35 person35@example.com"
    "76 user76 person76@example.com"
    "49 user49 person49@example.com"
    "24 user24 person24@example.com"
    "70 user70 person70@example.com"
    "48 user48 person48@example.com"
    "39 user39 person39@example.com"
    "15 user15 person15@example.com"
    "47 user47 person47@example.com"
    "30 user30 person30@example.com"
    "86 user86 person86@example.com"
    "31 user31 person31@example.com"
    "68 user68 person68@example.com"
    "37 user37 person37@example.com"
    "66 user66 person66@example.com"
    "63 user63 person63@example.com"
    "40 user40 person40@example.com"
    "78 user78 person78@example.com"
    "19 user19 person19@example.com"
    "46 user46 person46@example.com"
    "14 user14 person14@example.com"
    "81 user81 person81@example.com"
    "72 user72 person72@example.com"
    "6 user6 person6@example.com"
    "50 user50 person50@example.com"
    "85 user85 person85@example.com"
    "67 user67 person67@example.com"
    "2 user2 person2@example.com"
    "55 user55 person55@example.com"
    "69 user69 person69@example.com"
    "5 user5 person5@example.com"
    "65 user65 person65@example.com"
    "52 user52 person52@example.com"
    "1 user1 person1@example.com"
    "29 user29 person29@example.com"
    "9 user9 person9@example.com"
    "43 user43 person43@example.com"
    "75 user75 person75@example.com"
    "21 user21 person21@example.com"
    "82 user82 person82@example.com"
    "12 user12 person12@example.com"
    "18 user18 person18@example.com"
    "60 user60 person60@example.com"
    "44 user44 person44@example.com"
  )
  
  for record in "${RECORDS[@]}"; do
    id=$(echo "$record" | awk '{print $1}')
    username=$(echo "$record" | awk '{print $2}')
    email=$(echo "$record" | awk '{print $3}')
    COMMANDS="${COMMANDS}insert into default $id \"$username\" \"$email\"\n"
  done
  COMMANDS="${COMMANDS}.btree\n"

elif [ "$TEST_TYPE" = "four_node_btree" ]; then
  echo "Running 4-leaf-node btree test (30 records)..."
  # Insert many values in a specific order for 4-leaf-node btree
  COMMANDS="${COMMANDS}insert into default 18 \"user18\" \"person18@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 7 \"user7\" \"person7@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 10 \"user10\" \"person10@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 29 \"user29\" \"person29@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 23 \"user23\" \"person23@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 4 \"user4\" \"person4@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 14 \"user14\" \"person14@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 30 \"user30\" \"person30@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 15 \"user15\" \"person15@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 26 \"user26\" \"person26@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 22 \"user22\" \"person22@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 19 \"user19\" \"person19@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 2 \"user2\" \"person2@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 1 \"user1\" \"person1@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 21 \"user21\" \"person21@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 11 \"user11\" \"person11@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 6 \"user6\" \"person6@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 20 \"user20\" \"person20@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 5 \"user5\" \"person5@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 8 \"user8\" \"person8@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 9 \"user9\" \"person9@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 3 \"user3\" \"person3@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 12 \"user12\" \"person12@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 27 \"user27\" \"person27@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 17 \"user17\" \"person17@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 16 \"user16\" \"person16@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 13 \"user13\" \"person13@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 24 \"user24\" \"person24@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 25 \"user25\" \"person25@example.com\"\n"
  COMMANDS="${COMMANDS}insert into default 28 \"user28\" \"person28@example.com\"\n"
  COMMANDS="${COMMANDS}.btree\n" # Show the btree structure
fi

# Add select command to view data
COMMANDS="${COMMANDS}select from default\n"
# Add exit to terminate the program
COMMANDS="${COMMANDS}.exit\n"

# Run all commands in one go
echo -e "$COMMANDS" | ./maincode "$DB_FILE"
EXIT_CODE=$?


if [ $EXIT_CODE -ne 0 ]; then
  echo "Program exited with code $EXIT_CODE. Insertion may have failed."
  echo "If you saw a message like 'Need to implement updating parent after split',"
  echo "that means the insertion hit an unimplemented code path and the program"
  echo "exited before persisting the final state. The DB file may be incomplete."
  exit $EXIT_CODE
fi

echo "Test completed successfully!"
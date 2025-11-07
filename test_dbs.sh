#!/bin/bash

# 1. Compile the code
gcc -o maincode maincode.c
if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

# 2. Clean up previous test files
rm -f test1.db test2.db

# 3. Prepare the input commands
INPUT=$(cat <<EOF
test1.db
insert 1 user1 user1@example.com
select
.exit
EOF
)

INPUT2=$(cat <<EOF
test2.db
insert 2 user2 user2@example.com
select
.exit
EOF
)


# 4. Run the program and feed it the input for db1
echo "--- Testing test1.db ---"
OUTPUT1=$(echo "$INPUT" | ./maincode test1.db test2.db)
echo "$OUTPUT1"

# 5. Run the program and feed it the input for db2
echo "--- Testing test2.db ---"
OUTPUT2=$(echo "$INPUT2" | ./maincode test1.db test2.db)
echo "$OUTPUT2"


# 6. Verify the output
echo "--- Verifying test1.db ---"
if echo "$OUTPUT1" | grep -q "(1, user1, user1@example.com)"; then
    echo "SUCCESS: Record found in test1.db."
else
    echo "FAILURE: Record not found in test1.db."
fi

if echo "$OUTPUT1" | grep -q "(2, user2, user2@example.com)"; then
    echo "FAILURE: Record from test2.db found in test1.db."
else
    echo "SUCCESS: Record from test2.db not found in test1.db."
fi


echo "--- Verifying test2.db ---"
if echo "$OUTPUT2" | grep -q "(2, user2, user2@example.com)"; then
    echo "SUCCESS: Record found in test2.db."
else
    echo "FAILURE: Record not found in test2.db."
fi

if echo "$OUTPUT2" | grep -q "(1, user1, user1@example.com)"; then
    echo "FAILURE: Record from test1.db found in test2.db."
else
    echo "SUCCESS: Record from test1.db not found in test2.db."
fi


# 7. Clean up
rm -f maincode test1.db test2.db

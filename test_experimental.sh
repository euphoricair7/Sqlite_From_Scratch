#!/bin/zsh

# Compile experimental version
gcc -o maincode_experimental maincode_experimental.c
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

# Remove any existing test database
rm -f test.db

# Test sequence
echo "Testing experimental version..."
echo "insert 1 user1 person1@example.com" | ./maincode_experimental test.db
echo ".btree" | ./maincode_experimental test.db
echo "select" | ./maincode_experimental test.db

echo "Press Enter to continue with more tests or Ctrl+C to exit"
read

# More complex test sequence
for i in {1..5}; do
    echo "insert $i user$i person$i@example.com" | ./maincode_experimental test.db
    echo ".btree" | ./maincode_experimental test.db
done

echo "Final tree structure:"
echo ".btree" | ./maincode_experimental test.db
echo "All records:"
echo "select" | ./maincode_experimental test.db

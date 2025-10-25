#!/bin/zsh

# Remove existing database
rm -f mydb.db

echo "Testing database operations..."

# Try a simple insert and verify
echo "insert 1 user1 person1@example.com" | ./maincode mydb.db
echo "Checking insert..."
echo ".btree" | ./maincode mydb.db
echo "select" | ./maincode mydb.db

# Check what's in the database file
echo "Database file size: $(ls -l mydb.db | awk '{print $5}')"
echo "Database file contents (hex):"
xxd mydb.db | head -n 10

# Try to understand persistence
echo "\nTesting persistence..."
echo "insert 2 user2 person2@example.com" | ./maincode mydb.db
echo "select" | ./maincode mydb.db
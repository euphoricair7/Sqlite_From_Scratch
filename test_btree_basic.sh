#!/bin/zsh

DB_FILE="$1"
rm -f "$DB_FILE"  # Start fresh

# Test sequential inserts to build tree gradually
echo "Testing sequential B-tree construction..."

insert_and_verify() {
    local key=$1
    local username="user$key"
    local email="person$key@example.com"
    
    # Insert record
    echo "insert $key $username $email"
    if ! echo "insert $key $username $email" | ./maincode "$DB_FILE"; then
        echo "Failed to insert key $key"
        return 1
    fi
    
    # Verify it exists
    if ! echo "select" | ./maincode "$DB_FILE" | grep -q "$key.*$username.*$email"; then
        echo "Inserted key $key not found in select output"
        return 1
    fi
    
    # Show tree structure
    echo ".btree" | ./maincode "$DB_FILE"
    
    return 0
}

# Insert records with proper error checking
keys=(1 2 3 4 5)
for key in "${keys[@]}"; do
    if ! insert_and_verify "$key"; then
        echo "Failed at key $key"
        echo ".exit" | ./maincode "$DB_FILE"  # Ensure clean shutdown
        exit 1
    fi
    echo "----------------------------------------"
done

# Final verification
echo "Final tree structure:"
echo ".btree" | ./maincode "$DB_FILE"
echo "All records:"
echo "select" | ./maincode "$DB_FILE"
echo ".exit" | ./maincode "$DB_FILE"

# Check database file
if [ -f "$DB_FILE" ]; then
    size=$(stat -f%z "$DB_FILE" 2>/dev/null || stat -c%s "$DB_FILE")
    echo "Database file size: $size bytes"
    if [ "$size" -eq 0 ]; then
        echo "Warning: Database file is empty!"
        exit 1
    fi
else
    echo "Error: Database file not created"
    exit 1
fi
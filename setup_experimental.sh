#!/bin/zsh

# Safety check
if [ -f maincode_experimental.c ]; then
    echo "Warning: maincode_experimental.c already exists."
    echo "Do you want to overwrite it? (y/n)"
    read answer
    if [ "$answer" != "y" ]; then
        echo "Aborting."
        exit 1
    fi
fi

# Create backup of original file
cp maincode.c maincode.c.backup
echo "Created backup: maincode.c.backup"

# Create experimental copy
cp maincode.c maincode_experimental.c
echo "Created experimental copy: maincode_experimental.c"

# Create experimental header if it doesn't exist
if [ ! -f maincode_experimental.h ]; then
    cp maincode.h maincode_experimental.h
    echo "Created experimental header: maincode_experimental.h"
fi

# Update include in experimental file
sed -i 's/#include "maincode.h"/#include "maincode_experimental.h"/' maincode_experimental.c

# Create test script for experimental version
cat > test_experimental.sh << 'EOF'
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
EOF

chmod +x test_experimental.sh

echo "Setup complete! You can now:"
echo "1. Edit maincode_experimental.c"
echo "2. Run ./test_experimental.sh to test changes"
echo "3. Use maincode.c.backup to restore original if needed"
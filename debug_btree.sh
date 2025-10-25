#!/bin/zsh

# Remove existing database
if [ -f "mydb.db" ]; then
    rm mydb.db
    echo "Removed existing database."
fi

# Records to insert in sequence
RECORDS=(
    "18 user18 person18@example.com"
    "7 user7 person7@example.com"
    "10 user10 person10@example.com"
    "29 user29 person29@example.com"
    "23 user23 person23@example.com"
    "4 user4 person4@example.com"
    "14 user14 person14@example.com"
    "30 user30 person30@example.com"
    "15 user15 person15@example.com"
    "26 user26 person26@example.com"
    "22 user22 person22@example.com"
    "19 user19 person19@example.com"
    "2 user2 person2@example.com"
    "1 user1 person1@example.com"
    "21 user21 person21@example.com"
    "11 user11 person11@example.com"
    "6 user6 person6@example.com"
    "20 user20 person20@example.com"
    "5 user5 person5@example.com"
    "8 user8 person8@example.com"
    "9 user9 person9@example.com"
    "3 user3 person3@example.com"
    "12 user12 person12@example.com"
    "27 user27 person27@example.com"
    "17 user17 person17@example.com"
    "16 user16 person16@example.com"
    "13 user13 person13@example.com"
    "24 user24 person24@example.com"
    "25 user25 person25@example.com"
    "28 user28 person28@example.com"
)

count=1
for record in "${RECORDS[@]}"; do
    echo "=== Insert #$count: $record ==="
    echo "insert $record" > commands.tmp
    echo ".btree" >> commands.tmp
    echo "select" >> commands.tmp
    echo ".exit" >> commands.tmp
    
    ./maincode mydb.db < commands.tmp
    exit_code=$?
    
    if [ $exit_code -ne 0 ]; then
        echo "Program failed at insert #$count with exit code $exit_code"
        rm commands.tmp
        exit 1
    fi
    
    echo "----------------------------------------"
    count=$((count + 1))
done

rm commands.tmp
echo "All inserts completed successfully!"
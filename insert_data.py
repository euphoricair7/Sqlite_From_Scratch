#!/usr/bin/env python3

import sys
import subprocess

def insert_sample_data(db_file):
    """Insert sample data into the SQLite database"""
    print(f"Inserting sample data into {db_file}...")
    
    # Prepare the commands
    commands = []
    for i in range(1, 16):
        commands.append(f"insert {i} user{i} person{i}@example.com")
    
    # Add select command to view all data
    commands.append("select")
    # Add exit command
    commands.append(".exit")
    
    # Join commands with newlines
    input_data = '\n'.join(commands) + '\n'
    
    try:
        # Run the database program
        process = subprocess.run(
            ["./maincode", db_file],
            input=input_data,
            capture_output=True,
            text=True,
            timeout=5  # Timeout to prevent hanging
        )
        
        # Print the output
        print("\nCommand output:")
        print(process.stdout)
        
        if process.returncode == 0:
            print("✅ Data inserted successfully!")
        else:
            print(f"❌ Process returned error code: {process.returncode}")
            if process.stderr:
                print("Error output:")
                print(process.stderr)
    
    except subprocess.TimeoutExpired:
        print("❌ Process timed out after 5 seconds")
    except Exception as e:
        print(f"❌ Error: {str(e)}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 insert_data.py <database_file>")
        print("Example: python3 insert_data.py mydb.db")
        sys.exit(1)
    
    db_file = sys.argv[1]
    insert_sample_data(db_file)
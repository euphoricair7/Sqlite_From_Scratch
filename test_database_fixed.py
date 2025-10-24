#!/usr/bin/env python3

import subprocess
import sys
import os

from typing import List, Dict, Any

class DatabaseTestHarness:
    def __init__(self, executable_path: str = './maincode', db_path: str = 'test.db'):
        self.executable_path = executable_path
        self.db_path = db_path
        self.compile_database()
    
    def compile_database(self):
        """Compile the C database program, ensuring it's up-to-date."""
        print("🔨 Compiling database...")
        try:
            # Use standard C11 and common warning flags
            compile_result = subprocess.run(
                ['gcc', '-o', self.executable_path, 'maincode.c', '-Wall', '-std=c11'],
                capture_output=True, text=True, check=True
            )
        except subprocess.CalledProcessError as e:
            print("❌ Compilation Failed!")
            print(e.stderr)
            raise
    
    def run_script(self, commands: List[str]) -> Dict[str, Any]:
        """Run the database with given commands on a clean database file."""
        # Ensure the database file is removed before the run for a clean state.
        if os.path.exists(self.db_path):
            os.remove(self.db_path)

        input_data = '\n'.join(commands) + '\n'
        
        try:
            process = subprocess.run(
                [self.executable_path, self.db_path],
                input=input_data,
                capture_output=True,
                text=True,
                timeout=5  # Add a timeout to prevent hanging
            )
            
            lines = [line for line in process.stdout.strip().split('\n') if line]
            
            return {
                'lines': lines,
                'stderr': process.stderr,
                'returncode': process.returncode
            }
        except subprocess.TimeoutExpired:
            raise RuntimeError("Database process timed out")
    
    def run_until_exit(self, commands: List[str]) -> Dict[str, Any]:
        """Run commands and automatically add .exit"""
        commands = commands.copy()
        if not commands or commands[-1] != '.exit':
            commands.append('.exit')
        return self.run_script(commands)

def output_contains(lines, substring):
    """Check if any line in the output contains the substring"""
    return any(substring in line for line in lines)

def test_basic_operations():
    """Test basic insert and select operations"""
    print("🧪 Testing basic operations...")
    
    db = DatabaseTestHarness()
    
    # Test single insert and select
    result = db.run_until_exit([
        'insert 1 user1 person1@example.com',
        'select'
    ])
    
    assert output_contains(result['lines'], "Executed"), "Insert should execute successfully"
    assert output_contains(result['lines'], "(1, user1, person1@example.com)"), "Select should return inserted data"
    
    # Test multiple inserts
    result = db.run_until_exit([
        'insert 2 user2 person2@example.com',
        'insert 3 user3 person3@example.com',
        'insert 4 user4 person4@example.com',
        'select'
    ])
    
    # Check for presence of all rows
    assert output_contains(result['lines'], "Executed"), "Inserts should execute"
    assert output_contains(result['lines'], "(2, user2, person2@example.com)"), "Second row should be present"
    assert output_contains(result['lines'], "(3, user3, person3@example.com)"), "Third row should be present"
    assert output_contains(result['lines'], "(4, user4, person4@example.com)"), "Fourth row should be present"
    
    print("✅ Basic operations tests passed!")

def test_error_conditions():
    """Test error handling"""
    print("🧪 Testing error conditions...")
    
    db = DatabaseTestHarness()
    
    # Test missing parameters
    result = db.run_until_exit(['insert'])
    assert output_contains(result['lines'], "Syntax error"), "Should handle missing parameters"
    
    result = db.run_until_exit(['insert 1'])
    assert output_contains(result['lines'], "Syntax error"), "Should handle incomplete insert"
    
    # Test negative ID
    result = db.run_until_exit(['insert -1 user email@example.com'])
    assert output_contains(result['lines'], "ID must be positive"), "Should reject negative IDs"
    
    # Test unrecognized commands
    result = db.run_until_exit(['delete'])
    assert output_contains(result['lines'], "Unrecognized keyword"), "Should reject unknown commands"
    
    # Test string length limits
    long_username = 'a' * 33  # Longer than COLUMN_USERNAME_SIZE (32)
    result = db.run_until_exit([f'insert 1 {long_username} email@example.com'])
    assert output_contains(result['lines'], "String is too long"), "Should reject long usernames"
    
    long_email = 'a' * 250 + '@example.com'  # Longer than COLUMN_EMAIL_SIZE (255)
    result = db.run_until_exit([f'insert 1 username {long_email}'])
    assert output_contains(result['lines'], "String is too long"), "Should reject long emails"
    
    print("✅ Error condition tests passed!")

def test_boundary_conditions():
    """Test boundary conditions"""
    print("🧪 Testing boundary conditions...")
    
    db = DatabaseTestHarness()
    
    # Test ID zero
    result = db.run_until_exit([
        'insert 0 user email@example.com',
        'select'
    ])
    assert output_contains(result['lines'], "Executed"), "Should accept ID zero"
    assert output_contains(result['lines'], "(0, user, email@example.com)"), "Should store ID zero correctly"
    
    # Test maximum length strings
    max_username = 'a' * 32  # Exactly COLUMN_USERNAME_SIZE
    result = db.run_until_exit([
        f'insert 1 {max_username} email@example.com',
        'select'
    ])
    assert output_contains(result['lines'], "Executed"), "Should accept max length username"
    assert output_contains(result['lines'], f"(1, {max_username}, email@example.com)"), "Should store max length username"
    
    print("✅ Boundary condition tests passed!")

def test_meta_commands():
    """Test meta commands"""
    print("🧪 Testing meta commands...")
    
    db = DatabaseTestHarness()
    
    # Test .exit command - it should run without errors
    result = db.run_script(['.exit'])
    assert result['returncode'] == 0, "Should exit cleanly"
    
    # Test unrecognized meta command
    result = db.run_until_exit(['.foo'])
    assert output_contains(result['lines'], "Unrecognized command"), "Should handle unknown meta commands"
    
    print("✅ Meta command tests passed!")

def test_prints_constants():
    """Test .constants meta-command output"""
    print("🧪 Testing .constants output...")

    db = DatabaseTestHarness()
    result = db.run_script(['.constants', '.exit'])

    # Check for presence of expected constant values
    assert output_contains(result['lines'], "Constants:"), "Should show Constants header"
    assert output_contains(result['lines'], "ROW_SIZE:"), "Should show ROW_SIZE"
    assert output_contains(result['lines'], "COMMON_NODE_HEADER_SIZE:"), "Should show COMMON_NODE_HEADER_SIZE"
    assert output_contains(result['lines'], "LEAF_NODE_HEADER_SIZE:"), "Should show LEAF_NODE_HEADER_SIZE"
    assert output_contains(result['lines'], "LEAF_NODE_CELL_SIZE:"), "Should show LEAF_NODE_CELL_SIZE"
    assert output_contains(result['lines'], "LEAF_NODE_SPACE_FOR_CELLS:"), "Should show LEAF_NODE_SPACE_FOR_CELLS"
    assert output_contains(result['lines'], "LEAF_NODE_MAX_CELLS:"), "Should show LEAF_NODE_MAX_CELLS"
    
    print("✅ .constants output test passed!")

def test_btree_structure_one_node():
    """Test printing the structure of a one-node btree"""
    print("🧪 Testing b-tree structure for a single node...")

    db = DatabaseTestHarness()

    script = [
        "insert 3 user3 person3@example.com",
        "insert 1 user1 person1@example.com",
        "insert 2 user2 person2@example.com",
        ".btree",
        ".exit",
    ]
    result = db.run_script(script)

    # Check for presence of btree structure elements
    assert output_contains(result['lines'], "Tree:"), "Should show Tree header"
    assert output_contains(result['lines'], "leaf"), "Should show leaf node"
    assert output_contains(result['lines'], "1"), "Should contain key 1"
    assert output_contains(result['lines'], "2"), "Should contain key 2"
    assert output_contains(result['lines'], "3"), "Should contain key 3"

    print("✅ B-tree structure (one-node) test passed!")

def test_duplicate_id_error():
    """Test error on duplicate ID insertion"""
    print("🧪 Testing duplicate ID error...")

    db = DatabaseTestHarness()
    script = [
        "insert 1 user1 person1@example.com",
        "insert 1 user1 person1@example.com",
        "select",
        ".exit",
    ]
    result = db.run_script(script)

    assert output_contains(result['lines'], "Executed"), "First insert should execute"
    assert output_contains(result['lines'], "Error: Duplicate key"), "Should show duplicate key error"
    assert output_contains(result['lines'], "(1, user1, person1@example.com)"), "Original row should be present"

    print("✅ Duplicate ID error test passed!")

def test_btree_structure_three_leaf_node():
    """Test printing the structure of a 3-leaf-node btree"""
    print("🧪 Testing b-tree structure for a three-leaf-node setup...")

    db = DatabaseTestHarness()

    # Insert enough keys to cause a split
    script = []
    for i in range(1, 15):
        script.append(f"insert {i} user{i} person{i}@example.com")
    
    script.append(".btree")
    script.append("insert 15 user15 person15@example.com")
    script.append(".exit")

    result = db.run_script(script)

    # Check for presence of btree structure elements
    assert output_contains(result['lines'], "Tree:"), "Should show Tree header"
    assert output_contains(result['lines'], "internal"), "Should show internal node"
    assert output_contains(result['lines'], "leaf"), "Should show leaf nodes"
    
    # Check for some specific keys to verify the structure
    assert output_contains(result['lines'], "- key 7"), "Should contain internal node key 7"

    print("✅ B-tree structure (three-leaf-node) test passed!")

def test_select_on_multi_level_tree():
    """Test select statement on a multi-level tree"""
    print("🧪 Testing select on a multi-level tree...")

    db = DatabaseTestHarness()

    # Insert enough keys to create a multi-level tree
    script = []
    for i in range(1, 16):
        script.append(f"insert {i} user{i} person{i}@example.com")
    
    script.append("select")
    script.append(".exit")

    result = db.run_script(script)

    # Check that all rows are present in the output
    for i in range(1, 16):
        assert output_contains(result['lines'], f"({i}, user{i}, person{i}@example.com)"), f"Row {i} should be present"

    print("✅ Select on multi-level tree test passed!")

def run_single_test(test_func):
    """Run a single test and return success or failure"""
    try:
        test_func()
        return True, None
    except Exception as e:
        return False, str(e)

def main():
    """Run all tests"""
    print("🚀 Starting database tests...")
    
    all_tests = [
        test_basic_operations,
        test_error_conditions,
        test_boundary_conditions,
        test_meta_commands,
        test_prints_constants,
        test_btree_structure_one_node,
        test_duplicate_id_error,
        test_btree_structure_three_leaf_node,
        test_select_on_multi_level_tree
    ]
    
    passed_tests = []
    failed_tests = []
    
    for test_func in all_tests:
        success, error = run_single_test(test_func)
        if success:
            passed_tests.append(test_func.__name__)
        else:
            failed_tests.append((test_func.__name__, error))
    
    # Print summary
    print("\n📊 Test Summary:")
    print(f"  ✅ Passed: {len(passed_tests)} tests")
    print(f"  ❌ Failed: {len(failed_tests)} tests")
    
    if passed_tests:
        print("\n✅ The following tests passed:")
        for test_name in passed_tests:
            print(f"  - {test_name}")
    
    if failed_tests:
        print("\n❌ The following tests failed:")
        for test_name, error in failed_tests:
            print(f"  - {test_name}: {error}")
        return 1
    else:
        print("\n🎉 All tests passed successfully!")
        return 0

if __name__ == '__main__':
    sys.exit(main())
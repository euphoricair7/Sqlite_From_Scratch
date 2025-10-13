#!/usr/bin/env python3

import subprocess
import sys

from typing import List, Dict, Any

class DatabaseTestHarness:
    def __init__(self, executable_path: str = './maincode'):
        self.executable_path = executable_path
        self.compile_database()
    
    def compile_database(self):
        """Compile the C database program"""
        try:
            result = subprocess.run(
                ['gcc', '-Wall', '-Wextra', '-std=c11', '-o', 'maincode', 'maincode.c'],
                capture_output=True, text=True, check=True
            )
        except subprocess.CalledProcessError as e:
            raise RuntimeError(f"Failed to compile database:\n{e.stderr}")
    
    def run_script(self, commands: List[str]) -> Dict[str, Any]:
        """Run the database with given commands"""
        input_data = '\n'.join(commands) + '\n'
        
        try:
            result = subprocess.run(
                [self.executable_path],
                input=input_data,
                capture_output=True,
                text=True,
                timeout=10
            )
            
            return {
                'output': result.stdout,
                'error': result.stderr,
                'exit_status': result.returncode,
                'lines': [line.strip() for line in result.stdout.split('\n') if line.strip()]
            }
        except subprocess.TimeoutExpired:
            raise RuntimeError("Database process timed out")
    
    def run_until_exit(self, commands: List[str]) -> Dict[str, Any]:
        """Run commands and automatically add .exit"""
        commands = commands.copy()
        if not commands or commands[-1] != '.exit':
            commands.append('.exit')
        return self.run_script(commands)

def test_basic_operations():
    """Test basic insert and select operations"""
    print("🧪 Testing basic operations...")
    
    db = DatabaseTestHarness()
    
    # Test single insert and select
    result = db.run_until_exit([
        'insert 1 user1 person1@example.com',
        'select'
    ])
    
    assert 'Executed.' in result['lines'], "Insert should execute successfully"
    assert '(1, user1, person1@example.com)' in result['lines'], "Select should return inserted data"
    
    # Test multiple inserts
    result = db.run_until_exit([
        'insert 1 user1 person1@example.com',
        'insert 2 user2 person2@example.com',
        'insert 3 user3 person3@example.com',
        'select'
    ])
    
    assert result['lines'].count('Executed.') >= 3, "All inserts should execute"
    assert '(1, user1, person1@example.com)' in result['lines'], "First row should be present"
    assert '(2, user2, person2@example.com)' in result['lines'], "Second row should be present"
    assert '(3, user3, person3@example.com)' in result['lines'], "Third row should be present"
    
    print("✅ Basic operations tests passed!")

def test_error_conditions():
    """Test error handling"""
    print("🧪 Testing error conditions...")
    
    db = DatabaseTestHarness()
    
    # Test missing parameters
    result = db.run_until_exit(['insert'])
    assert 'Syntax error. Could not parse statement.' in result['lines'], "Should handle missing parameters"
    
    result = db.run_until_exit(['insert 1'])
    assert 'Syntax error. Could not parse statement.' in result['lines'], "Should handle incomplete insert"
    
    # Test negative ID
    result = db.run_until_exit(['insert -1 user email@example.com'])
    assert 'ID must be positive.' in result['lines'], "Should reject negative IDs"
    
    # Test unrecognized commands
    result = db.run_until_exit(['delete'])
    assert "Unrecognized keyword at start of 'delete'." in result['lines'], "Should reject unknown commands"
    
    # Test string length limits
    long_username = 'a' * 33  # Longer than COLUMN_USERNAME_SIZE (32)
    result = db.run_until_exit([f'insert 1 {long_username} email@example.com'])
    assert 'String is too long.' in result['lines'], "Should reject long usernames"
    
    long_email = 'a' * 250 + '@example.com'  # Longer than COLUMN_EMAIL_SIZE (255)
    result = db.run_until_exit([f'insert 1 username {long_email}'])
    assert 'String is too long.' in result['lines'], "Should reject long emails"
    
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
    assert 'Executed.' in result['lines'], "Should accept ID zero"
    assert '(0, user, email@example.com)' in result['lines'], "Should store ID zero correctly"
    
    # Test maximum length strings
    max_username = 'a' * 32  # Exactly COLUMN_USERNAME_SIZE
    result = db.run_until_exit([
        f'insert 1 {max_username} email@example.com',
        'select'
    ])
    assert 'Executed.' in result['lines'], "Should accept max length username"
    assert f'(1, {max_username}, email@example.com)' in result['lines'], "Should store max length username"
    
    print("✅ Boundary condition tests passed!")

def test_meta_commands():
    """Test meta commands"""
    print("🧪 Testing meta commands...")
    
    db = DatabaseTestHarness()
    
    # Test .exit command
    result = db.run_script(['.exit'])
    assert result['exit_status'] == 0, "Should exit cleanly"
    
    # Test unrecognized meta command
    result = db.run_until_exit(['.foo'])
    assert "Unrecognized command '.foo'" in result['lines'], "Should handle unknown meta commands"
    
    print("✅ Meta command tests passed!")

def main():
    """Run all tests"""
    print("🚀 Starting database tests...")
    
    try:
        test_basic_operations()
        test_error_conditions()
        test_boundary_conditions()
        test_meta_commands()
        
        print("\n🎉 All tests passed successfully!")
        return 0
        
    except Exception as e:
        print(f"\n❌ Test failed: {e}")
        return 1

if __name__ == '__main__':
    sys.exit(main())
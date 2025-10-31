#ifndef MAINCODE_H
#define MAINCODE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Constants and Macros
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100
#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

/*avoids ghost parents
Ensures that uninitialized parent pointers do not point 
to valid page numbers
*/
#define INVALID_PAGE_NUM UINT32_MAX

// Enums
typedef enum { NODE_INTERNAL, NODE_LEAF } NodeType;

typedef enum  { 
  EXECUTE_SUCCESS, 
  EXECUTE_TABLE_FULL, 
  EXECUTE_DUPLICATE_KEY 
}ExecuteResult;

typedef enum {
  PREPARE_SUCCESS,
  PREPARE_STRING_TOO_LONG,
  PREPARE_NEGATIVE_ID,
  PREPARE_SYNTAX_ERROR,
  PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum { STATEMENT_INSERT, STATEMENT_SELECT, STATEMENT_UPDATE, STATEMENT_DELETE } StatementType;

typedef enum{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;


// Structs
typedef struct {
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE + 1];
  char email[COLUMN_EMAIL_SIZE + 1];
} Row;

typedef struct {
  int file_descriptor;
  uint32_t file_length;
  uint32_t num_pages;
  void* pages[TABLE_MAX_PAGES];
} Pager;

typedef struct {
  uint32_t root_page_num;
  Pager* pager;
} Table;

typedef struct {
  Table* table;
  uint32_t page_num;
  uint32_t cell_num;
  bool end_of_table;
} Cursor;

typedef struct {
  StatementType type;
  Row row_to_insert; //only used by insert statement
  Row row_to_update; //only used by update statement
} Statement;

typedef struct {
  char* buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;



// Function Prototypes
void print_row(Row* row);
void serialize_row(Row* source, void* destination);
void deserialize_row(void* source, Row* destination);
void* get_page(Pager* pager, uint32_t page_num);
Pager* pager_open(const char* filename);
Table* db_open(const char* filename);
void db_close(Table* table);
void print_constants();
void print_tree(Pager* pager, uint32_t page_num, uint32_t indentation_level);
NodeType get_node_type(void* node);
void set_node_type(void* node, NodeType type);
uint32_t* leaf_node_num_cells(void* node);
void* leaf_node_cell(void* node, uint32_t cell_num);
uint32_t* leaf_node_key(void* node, uint32_t cell_num);
void* leaf_node_value(void* node, uint32_t cell_num);
void initialize_leaf_node(void* node);
void set_node_root(void* node, bool is_root);
Cursor* table_find(Table* table, uint32_t key);
Cursor* leaf_node_find(Table* table, uint32_t page_num, uint32_t key);
Cursor* internal_node_find(Table* table, uint32_t page_num, uint32_t key);
void* cursor_value(Cursor* cursor);
ExecuteResult execute_statement(Statement* statement, Table *table);
MetaCommandResult do_meta_command(InputBuffer* input_buffer, Table *table);
void internal_node_split_and_insert(Table* table, uint32_t parent_page_num, uint32_t child_page_num);
uint32_t get_node_max_key(Pager* pager, void* node);
bool is_node_root(void* node);
void create_new_root(Table* table, uint32_t right_child_page_num);
uint32_t *internal_node_child(void *node, uint32_t child_num);
uint32_t* node_parent(void* node);
void initialize_internal_node(void* node);
uint32_t* internal_node_num_keys(void* node);
uint32_t* internal_node_right_child(void* node);
uint32_t* internal_node_cell(void* node, uint32_t cell_num);
void internal_node_insert(Table* table, uint32_t parent_page_num, uint32_t child_page_num);
void update_internal_node_key(void* node, uint32_t old_key, uint32_t new_key);
uint32_t internal_node_find_child(void* node, uint32_t key);

#endif


#define _GNU_SOURCE  // For getline
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include "maincode.h"


#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

const uint32_t PAGE_SIZE = 4096;


#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255


const uint32_t ID_SIZE = size_of_attribute(Row, id);



const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

//common Node Header Layout

const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
const uint32_t NODE_TYPE_OFFSET = 0;
const uint32_t IS_ROOT_SIZE=sizeof(uint8_t);
const uint32_t IS_ROOT_OFFSET = NODE_TYPE_SIZE;
const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_OFFSET + IS_ROOT_SIZE;
const uint32_t COMMON_NODE_HEADER_SIZE = NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;



//Leaf Node Header Layout
const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_NEXT_LEAF_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NEXT_LEAF_OFFSET = LEAF_NODE_NUM_CELLS_OFFSET + LEAF_NODE_NUM_CELLS_SIZE;
const uint32_t LEAF_NODE_HEADER_SIZE=COMMON_NODE_HEADER_SIZE+LEAF_NODE_NUM_CELLS_SIZE+ LEAF_NODE_NEXT_LEAF_SIZE;

//leaf Node Body Layout
const uint32_t LEAF_NODE_KEY_SIZE=sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET=0;
const uint32_t LEAF_NODE_VALUE_SIZE=ROW_SIZE;
const uint32_t LEAF_NODE_VALUE_OFFSET=LEAF_NODE_KEY_OFFSET+LEAF_NODE_KEY_SIZE;
const uint32_t LEAF_NODE_CELL_SIZE=LEAF_NODE_KEY_SIZE+LEAF_NODE_VALUE_SIZE;
const uint32_t LEAF_NODE_SPACE_FOR_CELLS= PAGE_SIZE-LEAF_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_MAX_CELLS=LEAF_NODE_SPACE_FOR_CELLS/LEAF_NODE_CELL_SIZE;

uint32_t get_unused_page_num(Pager* pager) { return pager->num_pages; }

const uint32_t LEAF_NODE_RIGHT_SPLIT_COUNT = (LEAF_NODE_MAX_CELLS + 1) / 2;
const uint32_t LEAF_NODE_LEFT_SPLIT_COUNT = (LEAF_NODE_MAX_CELLS + 1) - LEAF_NODE_RIGHT_SPLIT_COUNT;








//Internal Node Implementation
//common node header layout
const uint32_t INTERNAL_NODE_NUM_KEYS_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_NUM_KEYS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t INTERNAL_NODE_RIGHT_CHILD_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_RIGHT_CHILD_OFFSET = INTERNAL_NODE_NUM_KEYS_OFFSET + INTERNAL_NODE_NUM_KEYS_SIZE;
const uint32_t INTERNAL_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE + INTERNAL_NODE_NUM_KEYS_SIZE + INTERNAL_NODE_RIGHT_CHILD_SIZE;

// Internal Node Body Layout
const uint32_t INTERNAL_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_CHILD_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_CELL_SIZE = INTERNAL_NODE_KEY_SIZE + INTERNAL_NODE_CHILD_SIZE;
//accessing leaf node fields
/* Keep this small for testing */
const uint32_t INTERNAL_NODE_MAX_CELLS = 2;

//this function returns a pointer to the number of cells in the leaf node
uint32_t* leaf_node_num_cells(void* node){
  return node + LEAF_NODE_NUM_CELLS_OFFSET;
}

//this function return a pointer to the cell number cell_num in the leaf node
void* leaf_node_cell(void* node, uint32_t cell_num){
  return node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}


//this function returns a pointer to the value in the cell number cell_num
void* leaf_node_value(void* node, uint32_t cell_num){
  return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}


//this function returns a pointer to the key in the cell number cell_num
uint32_t* leaf_node_key(void* node, uint32_t cell_num){
  return leaf_node_cell(node, cell_num);
}
uint32_t* leaf_node_next_leaf(void* node){
  return node + LEAF_NODE_NEXT_LEAF_OFFSET;
}


//function to split internal nodeas and then insert

/*

- creates a new internal/sibling node and moves half the children there
- decides wheather root needs to split
- updates parent-child relationships
- updates keys in parent node
- Recursively reinserts the split node into its parent if necessary.
*/

void internal_node_split_and_insert(Table* table, uint32_t parent_page_num, uint32_t child_page_num){
  uint32_t old_page_num=parent_page_num;
  void* old_node = get_page(table-> pager, parent_page_num);
  uint32_t old_max = get_node_max_key(table->pager, old_node);

  void* child = get_page(table->pager, child_page_num);
  uint32_t child_max = get_node_max_key(table->pager,child);

  uint32_t new_page_num = get_unused_page_num(table->pager);


  /*
+  Declaring a flag before updating pointers which
+  records whether this operation involves splitting the root -
+  if it does, we will insert our newly created node during
+  the step where the table's new root is created. If it does
+  not, we have to insert the newly created node into its parent
+  after the old node's keys have been transferred over. We are not
+  able to do this if the newly created node's parent is not a newly
+  initialized root node, because in that case its parent may have existing
+  keys aside from our old node which we are splitting. If that is true, we
+  need to find a place for our newly created node in its parent, and we
+  cannot insert it at the correct index if it does not yet have any keys
+  */


  uint32_t splitting_root = is_node_root(old_node);

  void* parent;
  void* new_node;
  if (splitting_root) {
    create_new_root(table, new_page_num);
    parent = get_page(table->pager,table->root_page_num);
    /*
    If we are splitting the root, we need to update old_node to point
    to the new root's left child, new_page_num will already point to
    the new root's right child
    */
    old_page_num = *internal_node_child(parent,0);
    old_node = get_page(table->pager, old_page_num);
  } else {
    parent = get_page(table->pager,*node_parent(old_node));
    new_node = get_page(table->pager, new_page_num);
    initialize_internal_node(new_node);
  }

  uint32_t* old_num_keys = internal_node_num_keys(old_node);

  uint32_t cur_page_num = *internal_node_right_child(old_node);
  void* cur = get_page(table->pager, cur_page_num);

  /*
  First put right child into new node and set right child of old node to invalid page number
  */
  internal_node_insert(table, new_page_num, cur_page_num);
  *node_parent(cur) = new_page_num;
  *internal_node_right_child(old_node) = INVALID_PAGE_NUM;
  /*
  For each key until you get to the middle key, move the key and the child to the new node
  */
  for (int i = INTERNAL_NODE_MAX_CELLS - 1; i > INTERNAL_NODE_MAX_CELLS / 2; i--) {
    cur_page_num = *internal_node_child(old_node, i);
    cur = get_page(table->pager, cur_page_num);

    internal_node_insert(table, new_page_num, cur_page_num);
    *node_parent(cur) = new_page_num;

    (*old_num_keys)--;
  }

  /*
  Set child before middle key, which is now the highest key, to be node's right child,
  and decrement number of keys
  */
  *internal_node_right_child(old_node) = *internal_node_child(old_node,*old_num_keys - 1);
  (*old_num_keys)--;

  /*
  Determine which of the two nodes after the split should contain the child to be inserted,
  and insert the child
  */
  uint32_t max_after_split = get_node_max_key(table->pager, old_node);

  uint32_t destination_page_num = child_max < max_after_split ? old_page_num : new_page_num;

  internal_node_insert(table, destination_page_num, child_page_num);
  *node_parent(child) = destination_page_num;

  update_internal_node_key(parent, old_max, get_node_max_key(table->pager, old_node));

  if (!splitting_root) {
    internal_node_insert(table,*node_parent(old_node),new_page_num);
    *node_parent(new_node) = *node_parent(old_node);
  }


}
//METHODS TO ACCESS INTERNAL NODE FIELDS
//READINBG AND WRITING

uint32_t* internal_node_num_keys(void* node){
  return node + INTERNAL_NODE_NUM_KEYS_OFFSET;
}

uint32_t* internal_node_right_child(void* node){
  return node + INTERNAL_NODE_RIGHT_CHILD_OFFSET;
}

uint32_t* internal_node_cell(void* node, uint32_t cell_num){
  return node + INTERNAL_NODE_HEADER_SIZE + cell_num * INTERNAL_NODE_CELL_SIZE;
}

uint32_t* internal_node_child(void* node, uint32_t child_num){
  uint32_t num_keys=*internal_node_num_keys(node);
  if(child_num > num_keys){
    printf("Tried to access child_num %d > num_keys %d\n", child_num, num_keys);
    exit(EXIT_FAILURE);
  }else if(child_num == num_keys){
    uint32_t* right_child = internal_node_right_child(node);
    if(*right_child == INVALID_PAGE_NUM){
      printf("Tried to access right child of internal node with no right child\n");
      exit(EXIT_FAILURE);
    }
    return right_child;
  }else{
    uint32_t* child =internal_node_cell(node, child_num);
    if(*child == INVALID_PAGE_NUM){
      printf("Tried to access child_num %d of internal node with no child\n", child_num);
      exit(EXIT_FAILURE);
    }
    return child;
  }
}

uint32_t* internal_node_key(void* node, uint32_t key_num){
  return (void*)(internal_node_cell(node, key_num) + INTERNAL_NODE_CHILD_SIZE);
}

NodeType get_node_type(void* node){
  uint8_t value = *((uint8_t*)(node+NODE_TYPE_OFFSET));
}

void set_node_type(void* node, NodeType type){
  uint8_t value = type;
  *((uint8_t*)(node + NODE_TYPE_OFFSET)) = value;

}
uint32_t get_node_max_key(Pager* pager, void* node){
   if (get_node_type(node) == NODE_LEAF) {
    return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
  }
  void* right_child = get_page(pager,*internal_node_right_child(node));
  return get_node_max_key(pager, right_child);
}

bool is_node_root(void* node){
  uint8_t value= *((uint8_t*)(node + IS_ROOT_OFFSET));
  return (bool)value;
}

void set_node_root(void* node, bool is_root){
  uint8_t value = is_root;
  *((uint32_t*)(node + IS_ROOT_OFFSET))=value;
}
/*About Leaf Nodes*/
/*Btree Implementation*/





void initialize_internal_node(void* node){
  set_node_type(node, NODE_INTERNAL);
  set_node_root(node,false);
  *internal_node_num_keys(node)=0;
  /*
  Necessary because the root page number is 0; by not initializing an internal 
  node's right child to an invalid page number when initializing the node, we may
  end up with 0 as the node's right child, which makes the node a parent of the root
  */
 *internal_node_right_child(node)=INVALID_PAGE_NUM;
}

void create_new_root(Table* table, uint32_t right_child_page_num){
  /*
  Handle splitting the root.
  Old root copied to new page, becomes left child.
  Address of right child passed in.
  Re-initialize root page to contain the new root node
  which has one key and two children
  */
  void* root = get_page(table->pager, table->root_page_num);
  void* right_child = get_page(table->pager, right_child_page_num);
  uint32_t left_child_page_num = get_unused_page_num(table->pager);
  void* left_child = get_page(table->pager, left_child_page_num);

  if(get_node_type(root)== NODE_INTERNAL){
    initialize_internal_node(left_child);
    initialize_internal_node(right_child);
  }
  
  //Left child has all the data of the old root
  memcpy(left_child, root, PAGE_SIZE);
  set_node_root(left_child, false);

  if(get_node_type(left_child) ==NODE_INTERNAL){
    void* child;
    for(int i =0; i<= *internal_node_num_keys(left_child); i++){
      
      child = get_page(table->pager, *internal_node_child(left_child,i));
      *node_parent(child) = left_child_page_num;
    }
    child= get_page(table->pager, *internal_node_right_child(left_child));
    *node_parent(child) = left_child_page_num;
  }
  //Root node is a new internal node with one key and two children
  initialize_internal_node(root);
  set_node_root(root, true);
  *internal_node_num_keys(root) = 1;
  *internal_node_child(root, 0) = left_child_page_num;
  uint32_t left_child_max_key = get_node_max_key(table->pager, left_child);
  *internal_node_key(root, 0) = left_child_max_key;
  *internal_node_child(root, 1) = right_child_page_num;
  *node_parent(left_child) = table->root_page_num;
  *node_parent(right_child) = table->root_page_num;

  set_node_root(left_child, false);
  set_node_root(right_child,true);
}

void update_internal_node_key(void* node, uint32_t old_key, uint32_t new_key) {
   uint32_t old_child_index = internal_node_find_child(node, old_key);
   *internal_node_key(node, old_child_index) = new_key;
}

//this function initializes a leaf node
void initialize_leaf_node(void* node){
  set_node_type(node, NODE_LEAF);
  set_node_root(node,false);
  *leaf_node_next_leaf(node)=0; //0 represents no siblings
}

void leaf_node_split_and_insert(Cursor* cursor, uint32_t key, Row* value){
  /*Create a new node and move half of the cells over,
  Insert the new value in one of the two nodes.
  Update parent or create a new parent
  */
  void * old_node = get_page(cursor->table->pager, cursor-> page_num);
  uint32_t old_max= get_node_max_key(cursor->table->pager,old_node);
  uint32_t new_page_num = get_unused_page_num(cursor->table->pager);
  void* new_node = get_page(cursor->table->pager, new_page_num);  
  initialize_leaf_node(new_node);
  *node_parent(new_node) = *node_parent(old_node);
  *leaf_node_next_leaf(new_node)=*leaf_node_next_leaf(old_node);
  *leaf_node_next_leaf(old_node)=new_page_num;

  /*
  All existing  keys plus new key should be divided
  evenly between old (left) and new (right) nodes.
  Starting from the right, move each key to correct position
  */

  for(int32_t i=LEAF_NODE_MAX_CELLS; i>=0; i--){
    void* destination_node;
    if(i >= LEAF_NODE_MAX_CELLS/2){
      destination_node=new_node;
    }else{
      destination_node=old_node;
    }

    uint32_t index_within_node=i% LEAF_NODE_LEFT_SPLIT_COUNT;
    void* destination=leaf_node_cell(destination_node, index_within_node);

    if(i==cursor->cell_num){
      //this is the new key to be inserted
      serialize_row(value,leaf_node_value(destination_node, index_within_node));
      *(leaf_node_key(destination_node, index_within_node))=key;
    }else if(i > cursor->cell_num){
      //move from old node, one cell to the right
      memcpy(destination, leaf_node_cell(old_node, i-1), LEAF_NODE_CELL_SIZE);
    }else{
      //move from old node
      memcpy(destination, leaf_node_cell(old_node, i), LEAF_NODE_CELL_SIZE);
    }
  }
  *(leaf_node_num_cells(old_node)) = LEAF_NODE_LEFT_SPLIT_COUNT;
  *(leaf_node_num_cells(new_node)) = LEAF_NODE_RIGHT_SPLIT_COUNT;

  if (is_node_root(old_node)) {
    return create_new_root(cursor->table, new_page_num);
  } else {
    uint32_t parent_page_num = *node_parent(old_node);
    uint32_t new_max = get_node_max_key(cursor->table->pager, old_node);
    void* parent = get_page(cursor->table->pager, parent_page_num);
    update_internal_node_key(parent, old_max, new_max);
    internal_node_insert(cursor->table, parent_page_num, new_page_num);
    return;

  }
}
/*end of btree implementation*/

uint32_t* node_parent(void* node) { return node + PARENT_POINTER_OFFSET; }

Cursor* table_start(Table* table) {
  Cursor* cursor = table_find(table,0);
  
  void* node = get_page(table->pager, cursor->page_num);
  uint32_t num_cells = *leaf_node_num_cells(node);

  cursor->end_of_table = (num_cells == 0);

  return cursor;
}



// Cursor* table_end(Table* table) {
//   Cursor* cursor = malloc(sizeof(Cursor));
//   cursor->table = table;
//   cursor->page_num=table->root_page_num;
//   void* root_node=get_page(table->pager, table->root_page_num);
//   uint32_t num_cells= *leaf_node_num_cells(root_node);
//   cursor->cell_num = num_cells;
//   cursor->end_of_table = true;

//   return cursor;
// }

/*
+Return the position of the given key.
+If the key is not present, return the position
+where it should be inserted
*/
Cursor* table_find(Table* table, uint32_t key) {
  uint32_t root_page_num = table->root_page_num;
  void* root_node = get_page(table->pager, root_page_num);

  if (get_node_type(root_node) == NODE_LEAF) {
    return leaf_node_find(table, root_page_num, key);
  } else {
    return internal_node_find(table, root_page_num, key);
  }
}

Cursor* find_existing_key(Table* table, uint32_t key){
  Cursor* cursor = table_find(table, key);
    if (!cursor) {
        return NULL;
    }

    void* node = get_page(table->pager, cursor->page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);
    
    if (cursor->cell_num >= num_cells) {
        free(cursor);
        return NULL;
    }

    uint32_t key_at_index = *leaf_node_key(node, cursor->cell_num);
    if (key_at_index != key) {
        free(cursor);
        return NULL;
    }

    return cursor;
}


uint32_t internal_node_find_child(void* node, uint32_t key) {
  /*
  Return the index of the child which should contain
  the given key.
  */

  uint32_t num_keys = *internal_node_num_keys(node);
  /* Binary search */
  uint32_t min_index = 0;
  uint32_t max_index = num_keys; /* there is one more child than key */

  while (min_index != max_index) {
    uint32_t index = (min_index + max_index) / 2;
    uint32_t key_to_right = *internal_node_key(node, index);
    if (key_to_right >= key) {
      max_index = index;
    } else {
      min_index = index + 1;
    }
  }

  return min_index;
}

Cursor* internal_node_find(Table* table, uint32_t page_num, uint32_t key) {
  void* node = get_page(table->pager, page_num);

  uint32_t child_index = internal_node_find_child(node, key);
  uint32_t child_num = *internal_node_child(node, child_index);
  void* child = get_page(table->pager, child_num);
  switch (get_node_type(child)) {
    case NODE_LEAF:
      return leaf_node_find(table, child_num, key);
    case NODE_INTERNAL:
      return internal_node_find(table, child_num, key);
  }
}


void internal_node_insert(Table* table, uint32_t parent_page_num, uint32_t child_page_num) {
  /*
  Add a new child/key pair to parent that corresponds to child
  */

  void* parent = get_page(table->pager, parent_page_num);
  void* child = get_page(table->pager, child_page_num);
  uint32_t child_max_key = get_node_max_key(table->pager, child);
  uint32_t index = internal_node_find_child(parent, child_max_key);

  uint32_t original_num_keys = *internal_node_num_keys(parent);
  

  if (original_num_keys >= INTERNAL_NODE_MAX_CELLS) {
    internal_node_split_and_insert(table, parent_page_num, child_page_num);
    return;
  }

  uint32_t right_child_page_num = *internal_node_right_child(parent);

  /*
  An internal node with a right child of INVALID_PAGE_NUM is empty
  */

  if(right_child_page_num == INVALID_PAGE_NUM){
    *internal_node_right_child(parent)=child_page_num;
    return;
  }

  void* right_child = get_page(table->pager, right_child_page_num);

  /*
  If we are already at the max number of cells for a node, we cannot increment
  before splitting. Incrementing without inserting a new key/child pair
  and immediately calling internal_node_split_and_insert has the effect
  of creating a new key at (max_cells + 1) with an uninitialized value
  */
  *internal_node_num_keys(parent) += 1;

  if (child_max_key > get_node_max_key(table->pager, right_child)) {
    /* Replace right child */
    *internal_node_child(parent, original_num_keys) = right_child_page_num;
    *internal_node_key(parent, original_num_keys) = get_node_max_key(table->pager, right_child);
    *internal_node_right_child(parent) = child_page_num;
  } else {
    /* Make room for the new cell */
    for (uint32_t i = original_num_keys; i > index; i--) {
      void* destination = internal_node_cell(parent, i);
      void* source = internal_node_cell(parent, i - 1);
      memcpy(destination, source, INTERNAL_NODE_CELL_SIZE);
    }
    *internal_node_child(parent, index) = child_page_num;
    *internal_node_key(parent, index) = child_max_key;
  }
}

Cursor* leaf_node_find(Table* table, uint32_t page_num, uint32_t key) {
  void* node = get_page(table->pager, page_num);
  uint32_t num_cells = *leaf_node_num_cells(node);

  Cursor* cursor = malloc(sizeof(Cursor));
  cursor->table = table;
  cursor->page_num = page_num;

  //Binary Search
  uint32_t min_index = 0;
  uint32_t one_past_max_index = num_cells;
  while (one_past_max_index != min_index) {
    uint32_t index = (min_index + one_past_max_index) / 2;
    uint32_t key_at_index = *leaf_node_key(node, index);
    if (key == key_at_index) {
      cursor->cell_num = index;
      return cursor;
    }
    if (key < key_at_index) {
      one_past_max_index = index;
    } else {
      min_index = index + 1;
    }
  }

  cursor->cell_num = min_index;
  return cursor;
}



void cursor_advance(Cursor* cursor) {
  uint32_t page_num=cursor->page_num;
  void* node=get_page(cursor->table->pager, page_num);
  cursor->cell_num += 1;
  if (cursor->cell_num >= (*leaf_node_num_cells(node))) {

    /* Advance to next leaf node */
    uint32_t next_page_num = *leaf_node_next_leaf(node);
    if (next_page_num == 0) {
      /* This was rightmost leaf */
      cursor->end_of_table = true;
    } else {
      cursor->page_num = next_page_num;
      cursor->cell_num = 0;
  }
}
}

void* cursor_value(Cursor* cursor){
  uint32_t page_num = cursor->page_num;
  void* page = get_page(cursor->table->pager, page_num);
  return leaf_node_value(page, cursor->cell_num);
}




void print_row(Row* row) {
  printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

void serialize_row(Row* source, void* destination) {
  memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
  strncpy(destination + USERNAME_OFFSET, source->username, USERNAME_SIZE);
  strncpy(destination + EMAIL_OFFSET, source->email, EMAIL_SIZE);

}

void serialize_update_row(Row* source, void* destination, const char* field) {

  char* dest = (char*)destination;
  if(strcmp(field,"username")==0){
    strncpy(dest + USERNAME_OFFSET, source->username, USERNAME_SIZE);
  }
  if(strcmp(field,"email")==0){
    strncpy(dest + EMAIL_OFFSET, source->email, EMAIL_SIZE);
    //strcpy((destination+EMAIL_OFFSET),(source->email));
  }

}

void deserialize_row(void *source, Row* destination) {
  memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
  memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
  memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

//returns the pointer to page_num using pager
void* get_page(Pager* pager, uint32_t page_num) {

  if (page_num > TABLE_MAX_PAGES) {
    printf("Tried to fetch page number out of bounds. %d > %d\n", page_num,
           TABLE_MAX_PAGES);
    exit(EXIT_FAILURE);

  }
  
  if (pager->pages[page_num] == NULL) {
    // Cache miss. Allocate memory and load from file.
    void* page = malloc(PAGE_SIZE);
    uint32_t num_pages = pager->file_length / PAGE_SIZE;

    // We might save a partial page at the end of the file
    if (pager->file_length % PAGE_SIZE) {
      num_pages += 1;
    }

    if (page_num <= num_pages) {
      lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
      ssize_t bytes_read = read(pager->file_descriptor, page, PAGE_SIZE);
      if (bytes_read == -1) {
        printf("Error reading file: %d\n", errno);
        exit(EXIT_FAILURE);
      }
    }

    pager->pages[page_num] = page;
  }

  if(page_num >= pager->num_pages){
    pager->num_pages=page_num+1;
  }
  return pager->pages[page_num];
}



Pager* pager_open(const char* filename) {
  int fd = open(filename,
                O_RDWR |      // Read/Write mode
                    O_CREAT,  // Create file if it does not exist
                S_IWUSR |     // User write permission
                    S_IRUSR   // User read permission
                );

  if (fd == -1) {
    printf("Unable to open file\n");
    exit(EXIT_FAILURE);
  }

  off_t file_length = lseek(fd, 0, SEEK_END);

  Pager* pager = malloc(sizeof(Pager));
  pager->file_descriptor = fd;
  pager->file_length = file_length;
  pager->num_pages = (file_length / PAGE_SIZE);
  if (file_length % PAGE_SIZE != 0) {
    printf("DB file is not a whole number of pages. Corrupt file.\n");
    exit(EXIT_FAILURE);
  }

  for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
    pager->pages[i] = NULL;
  }

  return pager;
}

void pager_flush(Pager* pager, uint32_t page_num) {
  if (pager->pages[page_num] == NULL) {
    printf("Tried to flush null page\n");
    exit(EXIT_FAILURE);
  }

  off_t offset = lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);

  if (offset == -1) {
    printf("Error seeking: %d\n", errno);
    exit(EXIT_FAILURE);
  }

  ssize_t bytes_written =
      write(pager->file_descriptor, pager->pages[page_num], PAGE_SIZE);

  if (bytes_written == -1) {
    printf("Error writing: %d\n", errno);
    exit(EXIT_FAILURE);
  }
}


void db_close(Table* table) {
  Pager* pager = table->pager;
  
  

  for (uint32_t i = 0; i < pager->num_pages; i++) {
    if (pager->pages[i] == NULL) {
      continue;
    }
    pager_flush(pager, i);
    free(pager->pages[i]);
    pager->pages[i] = NULL;
  }

  // There may be a partial page to write to the end of the file
  // This should not be needed after we switch to a B-tree
  // uint32_t num_additional_rows = table->num_rows % ROWS_PER_PAGE;
  // if (num_additional_rows > 0) {
  //   uint32_t page_num = num_full_pages;
  //   if (pager->pages[page_num] != NULL) {
  //     pager_flush(pager, page_num, num_additional_rows * ROW_SIZE);
  //     free(pager->pages[page_num]);
  //     pager->pages[page_num] = NULL;
  //   }
  // }

  int result = close(pager->file_descriptor);
  if (result == -1) {
    printf("Error closing db file.\n");
    exit(EXIT_FAILURE);
  }
  for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
    void* page = pager->pages[i];
    if (page) {
      free(page);
      pager->pages[i] = NULL;
    }
  }
  free(pager);
  free(table);
}


Table* db_open(const char* filename) {
    Pager* pager = pager_open(filename);
    Table* table = malloc(sizeof(Table));
    table->pager = pager;
    table->root_page_num = 0;

    // Initialize constraints
    init_constraints(&table->constraints);
    
    // Add primary key constraint for id column
    add_primary_key_constraint(&table->constraints, "id");

    //Add unique constraint to email column
    add_unique_constraint(&table->constraints, "email");

    if(pager->num_pages == 0) {
        // New database file. Initialize page 0 as leaf node
        void* root_node = get_page(pager, 0);
        initialize_leaf_node(root_node);
        set_node_root(root_node, true);
    }
    
    return table;
}

void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value){
  void* node = get_page(cursor->table->pager, cursor->page_num);

  uint32_t num_cells = *leaf_node_num_cells(node);
  if(num_cells >= LEAF_NODE_MAX_CELLS){
    // Node full
    leaf_node_split_and_insert(cursor, key, value);
    return;
  }

  if(cursor-> cell_num< num_cells){
    //make room for new cell
    for (uint32_t i = num_cells; i > cursor->cell_num; i--) {
      memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1), LEAF_NODE_CELL_SIZE);
    }
  }

  *(leaf_node_num_cells(node))+=1;
  *(leaf_node_key(node, cursor->cell_num))=key;
  serialize_row(value, leaf_node_value(node, cursor->cell_num));

}

void leaf_node_update(Cursor* cursor, uint32_t key, Row* value, const char* field) {
    void* node = get_page(cursor->table->pager, cursor->page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);

    // Basic validation
    if(cursor->cell_num >= num_cells) {
        printf("Error: Key %u not found for update\n", key);
        return;
    }

    // Validate key match
    uint32_t stored_key = *leaf_node_key(node, cursor->cell_num);
    if (stored_key != key) {
        printf("Error: Key mismatch: expected %u got %u\n", key, stored_key);
        return;
    }

    // Validate field name
    if (strcmp(field, "username") != 0 && strcmp(field, "email") != 0) {
        printf("Error: Invalid field name '%s'\n", field);
        return;
    }

    // Get existing row data
    void* dest = leaf_node_value(node, cursor->cell_num);
    Row existing_row;
    deserialize_row(dest, &existing_row);

    // Update only the specified field
    if (strcmp(field, "username") == 0) {
        strncpy(existing_row.username, value->username, COLUMN_USERNAME_SIZE);
    } else { // email
        strncpy(existing_row.email, value->email, COLUMN_EMAIL_SIZE);
    }

    // Write back the complete updated row
    serialize_row(&existing_row, dest);
}

void init_constraints(ConstraintList* list) {
    list->constraints = malloc(sizeof(Constraint) * 10); // Start with space for 10 constraints
    list->num_constraints = 0;
    list->max_constraints = 10;
}

void add_primary_key_constraint(ConstraintList* list, const char* column_name) {
    if (list->num_constraints >= list->max_constraints) {
        // Resize if needed
        list->max_constraints *= 2;
        list->constraints = realloc(list->constraints, sizeof(Constraint) * list->max_constraints);
    }
    
    Constraint* constraint = &list->constraints[list->num_constraints++];
    constraint->type = CONSTRAINT_PRIMARY_KEY;
    strncpy(constraint->column_name, column_name, 31);
    constraint->column_name[31] = '\0';  // Ensure null termination
    constraint->is_enforced = true;
}

void add_unique_constraint(ConstraintList* list, const char* column_name) {
    if (list->num_constraints >= list->max_constraints) {
        // Resize if needed
        list->max_constraints *= 2;
        list->constraints = realloc(list->constraints, sizeof(Constraint) * list->max_constraints);
    }
    
    Constraint* constraint = &list->constraints[list->num_constraints++];
    constraint->type = CONSTRAINT_UNIQUE;
    strncpy(constraint->column_name, column_name, 31);
    constraint->column_name[31] = '\0';  // Ensure null termination
    constraint->is_enforced = true;
}

void free_table(Table* table) {
    if (table->pager) {
        for (int i = 0; table->pager->pages[i]; i++) {
            free(table->pager->pages[i]);
        }
        free(table->pager);
    }
    
    // Free the constraints list
    if (table->constraints.constraints) {
        free(table->constraints.constraints);
    }
    
    free(table);
}

InputBuffer* new_input_buffer() {
  InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
  input_buffer->buffer = NULL;
  input_buffer->buffer_length = 0;
  input_buffer->input_length = 0;
  return input_buffer;
}

void close_input_buffer(InputBuffer* input_buffer) {
  free(input_buffer->buffer);
  free(input_buffer);
}



MetaCommandResult do_meta_command(InputBuffer* input_buffer, Table *table) {
  if (strcmp(input_buffer->buffer, ".exit") == 0) {
    db_close(table);
    exit(EXIT_SUCCESS);
  } else if (strcmp(input_buffer->buffer, ".btree") == 0) {
    printf("Tree:\n");
    print_tree(table->pager, 0, 0);
    return META_COMMAND_SUCCESS;

} else if (strcmp(input_buffer->buffer, ".constants") == 0) {
    printf("Constants:\n");
    print_constants();
    return META_COMMAND_SUCCESS;
  } else {
    return META_COMMAND_UNRECOGNIZED_COMMAND;
  }
}

void print_prompt() {
  printf("db > ");
}

// Simple line reader using POSIX getline
void read_input(InputBuffer* input_buffer) {
  ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
  if (bytes_read <= 0) {
    printf("Error reading input\n");
    exit(EXIT_FAILURE);
  }
  // strip trailing newline
  input_buffer->input_length = bytes_read - 1;
  input_buffer->buffer[bytes_read - 1] = '\0';
}

PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statement) {
  statement->type = STATEMENT_INSERT;

  char* keyword = strtok(input_buffer->buffer, " ");
  char* id_string = strtok(NULL, " ");
  char* username = strtok(NULL, " ");
  char* email = strtok(NULL, " ");

  if (id_string == NULL || username == NULL || email == NULL) {
    return PREPARE_SYNTAX_ERROR;
  }

  int id = atoi(id_string);
  if (id < 0) {
    return PREPARE_NEGATIVE_ID;
  }
  if (strlen(username) > COLUMN_USERNAME_SIZE) {
    return PREPARE_STRING_TOO_LONG;
  }
  if (strlen(email) > COLUMN_EMAIL_SIZE) {
    return PREPARE_STRING_TOO_LONG;
  }

  statement->row_to_insert.id = id;
  strcpy(statement->row_to_insert.username, username);
  strcpy(statement->row_to_insert.email, email);

  return PREPARE_SUCCESS;
}

PrepareResult prepare_update(InputBuffer* input_buffer, Statement* statement) {
    statement->type = STATEMENT_UPDATE;

    // Update syntax: update field=value where id=number
    char* keyword = strtok(input_buffer->buffer, " ");    // "update"
    char* field_part = strtok(NULL, " ");                 // "field=value"
    char* where = strtok(NULL, " ");                      // "where"
    char* id_part = strtok(NULL, " ");                    // "id=number"

    if (!field_part || !where || !id_part || 
        strcmp(where, "where") != 0) {
        return PREPARE_SYNTAX_ERROR;
    }

    // Parse field=value part
    char* field = strtok(field_part, "=");
    char* value = strtok(NULL, "=");
    if (!field || !value) {
        return PREPARE_SYNTAX_ERROR;
    }

    // Parse id=number part
    char* id_field = strtok(id_part, "=");
    char* id_string = strtok(NULL, "=");
    if (!id_field || !id_string || strcmp(id_field, "id") != 0) {
        return PREPARE_SYNTAX_ERROR;
    }

    // Validate field name
    if (strcmp(field, "username") != 0 && strcmp(field, "email") != 0) {
        printf("Error: Can only update 'username' or 'email' fields.\n");
        return PREPARE_SYNTAX_ERROR;
    }

    // Store field name
    statement->field_to_be_updated = strdup(field);

    // Parse and validate ID
    int id = atoi(id_string);
    if (id < 0) {
        free(statement->field_to_be_updated);
        return PREPARE_NEGATIVE_ID;
    }

    // Store ID and validate+store value based on field type
    statement->row_to_update.id = id;
    if (strcmp(field, "username") == 0) {
        if (strlen(value) > COLUMN_USERNAME_SIZE) {
            free(statement->field_to_be_updated);
            return PREPARE_STRING_TOO_LONG;
        }
        strcpy(statement->row_to_update.username, value);
    } else { // email
        if (strlen(value) > COLUMN_EMAIL_SIZE) {
            free(statement->field_to_be_updated);
            return PREPARE_STRING_TOO_LONG;
        }
        strcpy(statement->row_to_update.email, value);
    }

    return PREPARE_SUCCESS;
}


PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement) {
  if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
    return prepare_insert(input_buffer, statement);
  }
  if (strcmp(input_buffer->buffer, "select") == 0) {
    statement->type = STATEMENT_SELECT;
    return PREPARE_SUCCESS;
  }
  if(strncmp(input_buffer->buffer,"update",6)==0){
    return prepare_update(input_buffer, statement);
  }

  if(strncmp(input_buffer->buffer,"desc",4)==0){
    statement->type=STATEMENT_DESC;
    return PREPARE_SUCCESS;
  }

  // if(strncmp(input_buffer->buffer,"delete",6)==0){
  //   return prepare_delete(input_buffer, statement);
  // }

  return PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecuteResult check_unique_email(Table* table, const char* email) {
    Cursor* cursor = table_start(table);
    Row row;
    
    while (!cursor->end_of_table) {
        deserialize_row(cursor_value(cursor), &row);
        if (strcmp(row.email, email) == 0) {
            free(cursor);
            printf("Error: Email '%s' already exists (unique constraint violation)\n", email);
            return EXECUTE_DUPLICATE_KEY;
        }
        cursor_advance(cursor);
    }
    
    free(cursor);
    return EXECUTE_SUCCESS;
}

ExecuteResult execute_insert(Statement* statement, Table* table) {
    void* node = get_page(table->pager, table->root_page_num);
    uint32_t num_cells = *(leaf_node_num_cells(node));
    
    Row* row_to_insert = &(statement->row_to_insert);
    uint32_t key_to_insert = row_to_insert->id;

    // Check primary key (ID) constraint
    Cursor* cursor = table_find(table, key_to_insert);
    if (cursor->cell_num < num_cells) {
        uint32_t key_at_index = *leaf_node_key(node, cursor->cell_num);
        if (key_at_index == key_to_insert) {
            free(cursor);
            printf("Error: Duplicate ID %d (primary key violation)\n", key_to_insert);
            return CONSTRAINT_PRIMARY_KEY;
        }
    }

    // Check unique email constraint
    ExecuteResult email_result = check_unique_email(table, row_to_insert->email);
    if (email_result == CONSTRAINT_UNIQUE) {
        free(cursor);
        return CONSTRAINT_UNIQUE;
    }

    // If all constraints pass, insert the row
    leaf_node_insert(cursor, row_to_insert->id, row_to_insert);
    free(cursor);
    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement* statement, Table* table) {
    Cursor* cursor = table_start(table);
    Row row;
    while (!(cursor->end_of_table)) {
      deserialize_row(cursor_value(cursor), &row);
      print_row(&row);
      cursor_advance(cursor);
    }

    free(cursor);
    return EXECUTE_SUCCESS;
}

ExecuteResult execute_update(Statement* statement, Table* table) {
  printf("DEBUG: inside execute_update function.\n");

  Row* row_to_update = &(statement->row_to_update);
  uint32_t key_to_update = row_to_update->id;
  Cursor* cursor = find_existing_key(table, key_to_update);

  //debug statements
  printf("printing row_to_update: %s %s %u", row_to_update->username,row_to_update->email,row_to_update->id);
  
  if (!cursor) {  // Add null check
    return EXECUTE_KEY_NOT_FOUND;
  }

  // If updating email field, check uniqueness constraint
  if (strcmp(statement->field_to_be_updated, "email") == 0) {
    // Get current row to avoid checking against itself
    Row current_row;
    deserialize_row(cursor_value(cursor), &current_row);
    
    // Skip uniqueness check if new email is same as current
    if (strcmp(current_row.email, row_to_update->email) != 0) {
      ExecuteResult email_result = check_unique_email(table, row_to_update->email);
      if (email_result == EXECUTE_DUPLICATE_KEY) {
        free(cursor);
        printf("Error: Email '%s' already exists (unique constraint violation)\n", 
               row_to_update->email);
        free(statement->field_to_be_updated);
        return CONSTRAINT_UNIQUE;
      }
    }
  }

  leaf_node_update(cursor, row_to_update->id, row_to_update, statement->field_to_be_updated);
  
  free(cursor);
  free(statement->field_to_be_updated);
  return EXECUTE_SUCCESS;
}

// ExecuteResult execute_delete(Statement* statement, Table* table){

// }

ExecuteResult execute_desc(Statement* statement, Table* table) {
  printf("\nTable Description:\n");
  printf("----------------\n");
  printf("Column Name     Type        Size    Constraints\n");
  printf("----------------------------------------\n");

  // For each column, we'll print its info and then its constraints
  const char* columns[] = {"id", "username", "email"};
  const char* types[] = {"INTEGER", "VARCHAR", "VARCHAR"};
  const int sizes[] = {ID_SIZE, USERNAME_SIZE, EMAIL_SIZE};
  
  for (int i = 0; i < 3; i++) {
    // Print column base info
    printf("%-14s %-11s %-8d ", columns[i], types[i], sizes[i]);
    
    // Print constraints for this column
    bool first_constraint = true;
    for (uint32_t j = 0; j < table->constraints.num_constraints; j++) {
      Constraint* constraint = &table->constraints.constraints[j];
      if (constraint->is_enforced && strcmp(constraint->column_name, columns[i]) == 0) {
        if (!first_constraint) {
          printf(", ");
        }
        first_constraint = false;
        
        switch (constraint->type) {
          case CONSTRAINT_PRIMARY_KEY:
            printf("PRIMARY KEY");
            break;
          case CONSTRAINT_UNIQUE:
            printf("UNIQUE");
            break;
          case CONSTRAINT_NOT_NULL:
            printf("NOT NULL");
            break;
          default:
            break;
        }
      }
    }
    printf("\n");  // New line after each column
  }

  printf("\nStorage Information:\n");
  printf("------------------\n");
  printf("Page size: %d bytes\n", PAGE_SIZE);
  printf("Row size: %d bytes\n", ROW_SIZE);
  printf("Max cells per leaf: %d (= (PageSize - HeaderSize) / (KeySize + RowSize))\n", LEAF_NODE_MAX_CELLS);
  printf("                    = (%d - %d) / (%d + %d) = %d\n", 
         PAGE_SIZE, LEAF_NODE_HEADER_SIZE, LEAF_NODE_KEY_SIZE, ROW_SIZE, LEAF_NODE_MAX_CELLS);
  printf("Max keys per internal node: %d\n", INTERNAL_NODE_MAX_CELLS);

  return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement* statement, Table *table) {
  switch (statement->type) {
    case (STATEMENT_INSERT):
      return execute_insert(statement, table);
    case (STATEMENT_SELECT):
      return execute_select(statement, table);
    case (STATEMENT_UPDATE):
      return execute_update(statement,table);
    // case (STATEMENT_DELETE):
    //   return execute_delete(statement, table);
    case (STATEMENT_DESC):
      return execute_desc(statement,table);
  }
}



void print_constants(){
  printf("ROW_SIZE: %d\n", ROW_SIZE);
  printf("COMMON_NODE_HEADER_SIZE: %d\n", COMMON_NODE_HEADER_SIZE);
  printf("LEAF_NODE_HEADER_SIZE: %d\n", LEAF_NODE_HEADER_SIZE);
  printf("LEAF_NODE_CELL_SIZE: %d\n", LEAF_NODE_CELL_SIZE);
  printf("LEAF_NODE_SPACE_FOR_CELLS: %d\n", LEAF_NODE_SPACE_FOR_CELLS);
  printf("LEAF_NODE_MAX_CELLS: %d\n", LEAF_NODE_MAX_CELLS);
}
/*
void print_leaf_nodes(void*node){
  uint32_t num_cells= *leaf_node_num_cells(node);
  printf("leaf (size %d)\n", num_cells);
  for (uint32_t i = 0; i < num_cells; i++) {
    uint32_t key = *leaf_node_key(node, i);
    printf("  - %d : %d\n", i, key);
  }
}
  */

  
void indent(uint32_t level){
  for(uint32_t i=0; i<level;i++){
    printf(" ");
  }
}

void print_tree(Pager* pager, uint32_t page_num, uint32_t indentation_level){
  void* node=get_page(pager, page_num);
  uint32_t num_keys, child;

  switch(get_node_type(node)){
    case NODE_LEAF:
      num_keys=*leaf_node_num_cells(node);
      indent(indentation_level);
      printf("- leaf (size %d)\n", num_keys);
      for(uint32_t i=0; i<num_keys; i++){
        indent(indentation_level + 1);
        printf("- %d\n", *leaf_node_key(node,i));
      }
      break;
    case NODE_INTERNAL:
      num_keys=*internal_node_num_keys(node);
      indent(indentation_level);
      printf("- internal (size %d)\n", num_keys);
      if(num_keys>0){  
        for(uint32_t i=0; i<num_keys; i++){
            child=*internal_node_child(node,i);
            print_tree(pager, child, indentation_level + 1);

            indent(indentation_level + 1);
            printf("- key %d\n", *internal_node_key(node,i));
          }
      }
      child=*internal_node_right_child(node);
      print_tree(pager, child, indentation_level + 1);
      break;
  }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
     printf("Must supply a database filename.\n");
     exit(EXIT_FAILURE);
   }


 
   char* filename = argv[1];
   
   Table* table = db_open(filename);
   InputBuffer* input_buffer = new_input_buffer();
   while (true) {
     print_prompt();
     read_input(input_buffer);

    if (input_buffer->buffer[0] == '.') {
      switch (do_meta_command(input_buffer, table)) {
        case (META_COMMAND_SUCCESS):
          continue;
        case (META_COMMAND_UNRECOGNIZED_COMMAND):
          printf("Unrecognized command '%s'\n", input_buffer->buffer);
          continue;
      }
    }

    Statement statement;
    switch (prepare_statement(input_buffer, &statement)) {
      case (PREPARE_SUCCESS):
        break;
      case (PREPARE_NEGATIVE_ID):
        printf("ID must be positive.\n");
        continue;
      case (PREPARE_STRING_TOO_LONG):
        printf("String is too long.\n");
        continue;
      case (PREPARE_SYNTAX_ERROR):
        printf("Syntax error. Could not parse statement.\n");
        continue;
      case (PREPARE_UNRECOGNIZED_STATEMENT):
        printf("Unrecognized keyword at start of '%s'.\n",
               input_buffer->buffer);
        continue;
    }

    ExecuteResult result = execute_statement(&statement, table);
    
    // Handle general execution results first
    if (result == EXECUTE_SUCCESS) {
        printf("Executed.\n");
    }
    else if (result == EXECUTE_TABLE_FULL) {
        printf("Error: Table full.\n");
    }
    else if (result == EXECUTE_KEY_NOT_FOUND) {
        printf("Error: Key not found.\n");
    }

    // Handle constraint violations separately
    //due to enum conflict between COnstraintType and ExecuteResult
    else if (result == CONSTRAINT_PRIMARY_KEY) {
        printf("Error: Primary key violation.\n");
    }
    else if (result == CONSTRAINT_UNIQUE) {
        printf("Error: Unique constraint violation.\n");
    }
   }
   return 0;
 }
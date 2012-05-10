#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "set.h"
#include "common.h"


// Local defenitions
typedef enum rbtree_node_color color;

// Private functions
static snode_t *grandparent(snode_t *n);
static snode_t *sibling(snode_t *n);
static snode_t *uncle(snode_t *n);
static void verify_properties(set_t *t);
static void verify_property_1(snode_t *root);
static void verify_property_2(snode_t *root);
static color node_color(snode_t *n);
static void verify_property_4(snode_t *root);
static void verify_property_5(snode_t *root);
static void verify_property_5_helper(snode_t *n, int black_count, int *black_count_path);
static snode_t *lookup_node(set_t *t, void *key);

static snode_t *new_node(void *key, void *value, color node_color, snode_t *left, snode_t *right);
static snode_t *lookup_node(set_t *t, void *key);
static void rotate_left(set_t *t, snode_t *n);
static void rotate_right(set_t *t, snode_t *n);

static void replace_node(set_t *t, snode_t *oldn, snode_t *newn);
static void insert_case1(set_t *t, snode_t *n);
static void insert_case2(set_t *t, snode_t *n);
static void insert_case3(set_t *t, snode_t *n);
static void insert_case4(set_t *t, snode_t *n);
static void insert_case5(set_t *t, snode_t *n);

static void _set_destroy(snode_t *node);
static snode_t *maximum_node(snode_t *n);
static snode_t *minimum_node(snode_t *n);

//Set node structure
struct snode    {
    void *key;
    void *value;
    snode_t *left;
    snode_t *right;
    snode_t *parent;
    enum rbtree_node_color color;
};
//Set iterator structure
struct set_iter {
    snode_t *node;
};

//Set structure
struct set  {
    snode_t *root;
    cmpfunc_t compare;
    int numitems;
};

//Find the grandparent node
snode_t *grandparent(snode_t *n)
{
    assert (n != NULL);
    assert (n->parent != NULL); //not root node
    assert (n->parent->parent != NULL);
    return n->parent->parent;
}

//find the sibling node, if parent only have 1 child, sibling = NULL
snode_t *sibling(snode_t *n)
{
    assert (n != NULL);
    assert (n->parent != NULL);
    if (n == n->parent->left)
        return n->parent->right;
    else
        return n->parent->left;
}

// Sibling of the parent. If parent only have 1 child, uncle maay be NULL
snode_t *uncle(snode_t *n)
{
    assert (n != NULL);
    assert (n->parent != NULL); //root node has no uncle
    assert (n->parent->parent != NULL); //childrent of root have no uncle
    return sibling(n->parent);
}

// Debug functions to make sure the properties have encforced a balanced tree. They
//are slow, since they all itterate through the entire tree
void verify_properties(set_t *t)
{
#ifdef DEBUG
    verify_property_1(t->root);
    verify_property_2(t->root);
    //property 3 is implicit
    verify_property_4(t->root);
    verify_property_5(t->root);
#endif
}

// Make sure the nodes are either red or black. If not terminate program
void verify_property_1(snode_t *n)
{
    assert(node_color(n) == RED || node_color(n) == BLACK);
    if (n == NULL) return;
    verify_property_1(n->left);
    verify_property_1(n->right);
}

// Make sure the root node is black.
void verify_property_2(snode_t *root)
{
    assert(node_color(root) == BLACK);
}

// All leaves are black and contain no data. All NULL nodes are treated as black
color node_color(snode_t *n)
{
    return n == NULL ? BLACK : n->color;
}

// The parent of of every red node is black
void verify_property_4(snode_t *n)
{
    if (node_color(n) == RED) {
        assert (node_color(n->left)   == BLACK);
        assert (node_color(n->right)  == BLACK);
        assert (node_color(n->parent) == BLACK);
    }
    if (n == NULL) return;
    verify_property_4(n->left);
    verify_property_4(n->right);
}

// Vertify that all paths from a given leaf contains the same number of black nodes
void verify_property_5(snode_t *root)
{
    int black_count_path = -1;
    verify_property_5_helper(root, 0, &black_count_path);
}

//Traverse both sides of the tree, save the number of black nodes per side, and compare them
void verify_property_5_helper(snode_t *n, int black_count, int *path_black_count)
{
    if (node_color(n) == BLACK) {
        black_count++;
    }
    if (n == NULL) {
        if (*path_black_count == -1) {
            *path_black_count = black_count;
        } else {
            assert (black_count == *path_black_count);
        }
        return;
    }
    verify_property_5_helper(n->left,  black_count, path_black_count);
    verify_property_5_helper(n->right, black_count, path_black_count);
}

// Search the tree for a specific key, if not found, return NULL 
snode_t *lookup_node(set_t *t, void *key)
{
    snode_t *n = t->root;
    while (n != NULL) {
        int comp_result = t->compare(key, n->key);
        if (comp_result == 0) {
            return n;
        } else if (comp_result < 0) {
            n = n->left;
        } else {
            assert(comp_result > 0);
            n = n->right;
        }
    }
    return n;
}

// Rotate the tree left, both take highest node in the subtree as argument 
void rotate_left(set_t *t, snode_t *n)
{
    snode_t *r = n->right;
    replace_node(t, n, r);
    n->right = r->left;
    if (r->left != NULL) {
        r->left->parent = n;
    }
    r->left = n;
    n->parent = r;
}

// Op.sit left = right 
void rotate_right(set_t *t, snode_t *n)
{
    snode_t *L = n->left;
    replace_node(t, n, L);
    n->left = L->right;
    if (L->right != NULL) {
        L->right->parent = n;
    }
    L->right = n;
    n->parent = L;
}

// Substitutes a new node or NULL in the place of another node
void replace_node(set_t *t, snode_t *oldn, snode_t *newn)
{
    if (oldn->parent == NULL) {
        t->root = newn;
    } else {
        if (oldn == oldn->parent->left)
            oldn->parent->left = newn;
        else
            oldn->parent->right = newn;
    }
    if (newn != NULL) {
        newn->parent = oldn->parent;
    }
}

//Creates a new set
set_t *set_create(cmpfunc_t compare)
{
    set_t *set;
    set = (set_t*)malloc(sizeof(set_t));
    if (set == NULL)
        return NULL;
    
    set->root = NULL;
    set->compare = compare;
    set->numitems = 0;
    verify_properties(set);
    return set;
}

/* Create new node */
snode_t *new_node(void *key, void *value, color node_color, snode_t *left, snode_t *right)
{
    snode_t *result = (snode_t*)malloc(sizeof(snode_t));
    result->key = key;
    result->value = value;
    result->color = node_color;
    result->left = left;
    result->right = right;
    if (left  != NULL)  left->parent = result;
    if (right != NULL) right->parent = result;
    result->parent = NULL;
    return result;
}

void set_destroy(set_t *set)
{
    if (set->root != NULL)
        _set_destroy(set->root);
    free(set);
}

void _set_destroy(snode_t *node)
{
    if (node == NULL) return;
    _set_destroy(node->left);
    _set_destroy(node->right);
    free(node);
    
}

int set_size(set_t *set)
{
    return set->numitems;
}

//Adds a element to the set
void set_add(set_t *set, void *key, void *elem)
{
    snode_t *inserted_node = new_node(key, elem, RED, NULL, NULL);
    if (set->root == NULL)  {
        set->root = inserted_node;
    }   else    {
        snode_t *n = set->root;
        while (1)   {
            int comp_results = set->compare(key, n->key);
            if (comp_results == 0)  {
                n->value = elem;
                // Clean up inserted node, it is not used
                free(inserted_node);
                return;
            }   else if (comp_results < 0)   {
                if (n->left == NULL)    {
                    n->left = inserted_node;
                    break;
                }   else    {
                    n = n->left;
                }
            }   else    {
                    assert (comp_results > 0);
                    if (n->right == NULL)   {
                        n->right = inserted_node;
                        break;
                    }   else    {
                        n = n->right;
                    }
                }
            }
            inserted_node->parent = n;
        }
        insert_case1(set, inserted_node);
        set->numitems++;
        verify_properties(set);
}


/*Functions to make sure the tree statisfy red-black tree properties
    newnode = root? Then make it so! */
void insert_case1(set_t *t, snode_t *n)
{
    if (n->parent == NULL)
        n->color = BLACK;
    else
        insert_case2(t, n); /* If not root, check next rule */
}

// IF newnode has black parent
void insert_case2(set_t *t, snode_t *n)
{
    if (node_color(n->parent) == BLACK)
        return; /* Tree is still valid */
    else
        insert_case3(t, n); /*If the newnode doesnt have a black parent, check next rule */
}

//Check if the uncle is red
void insert_case3(set_t *t, snode_t *n)
{
    if (node_color(uncle(n)) == RED) { /* Is the uncle red, recolor parent, uncle and grandparent */
        n->parent->color = BLACK;
        uncle(n)->color = BLACK;
        grandparent(n)->color = RED;
        insert_case1(t, grandparent(n)); //Call case one, inscase the red grandparent might break
    } else {
        insert_case4(t, n); //if the uncle isnt red, call rule 4
    }
}

//Fix mirror, left child rotate, else right rotate
void insert_case4(set_t *t, snode_t *n)
{
    if (n == n->parent->right && n->parent == grandparent(n)->left) {
        rotate_left(t, n->parent);
        n = n->left;
    } else if (n == n->parent->left && n->parent == grandparent(n)->right) {
        rotate_right(t, n->parent);
        n = n->right;
    }
    insert_case5(t, n);
}

/*Fix final mirror matchup */
void insert_case5(set_t *t, snode_t *n)
{
    n->parent->color = BLACK;
    grandparent(n)->color = RED;
    if (n == n->parent->left && n->parent == grandparent(n)->left) {
        rotate_right(t, grandparent(n));
    } else {
        assert (n == n->parent->right && n->parent == grandparent(n)->right);
        rotate_left(t, grandparent(n));
    }
}

int set_contains(set_t *set, void *key)
{
    snode_t *n = lookup_node(set, key);
    return n == NULL ? 0 : 1;
}

// Union of the given sets
set_t *set_union(set_t *a, set_t *b)
{
    set_t *unionset = set_copy(a);
    set_iter_t *iter = set_createiter(b);
    
    while (set_hasnext(iter) != 0)  {
        set_add(unionset, iter->node->key, iter->node->value);
        set_next(iter);
    }
    return unionset;
}

// Intersection of the given sets
set_t *set_intersection(set_t *a, set_t *b)
{
    set_t *intersect = set_create(a->compare);
    set_iter_t *iter = set_createiter(a); 
    
    while (set_hasnext(iter) != 0)  {
        if (set_contains(b, iter->node->key))    {
            set_add(intersect, iter->node->key, iter->node->value);
        }
        set_next(iter);
    }
    return intersect;
}

// Difference of the given sets
set_t *set_difference(set_t *a, set_t *b)
{
    set_t *difference = set_create(a->compare);
    set_iter_t *iter = set_createiter(a); 
    
    while (set_hasnext(iter) != 0)  {
        if (!set_contains(b, iter->node->key))    {
            set_add(difference, iter->node->key, iter->node->value);
        }
        set_next(iter);
    }
    return difference;
    
}

// Returns a copy of the given set
set_t *set_copy(set_t *set)
{
    set_t *copyset = set_create(set->compare);
    if (copyset == NULL)
        return NULL;
    set_iter_t *iter = set_createiter(set);
    if (iter == NULL)
        return NULL;
    
    while (set_hasnext(iter) != 0)    {
        set_add(copyset, iter->node->key, iter->node->value);
        set_next(iter);
    }
    return copyset;
}

// Create set iterator
set_iter_t *set_createiter(set_t *set)
{
    //assert(set != NULL);
    //assert(set->root != NULL);

    set_iter_t *iter = (set_iter_t*)malloc(sizeof(set_iter_t));
    if (iter == NULL)
        return NULL;
    iter->node = minimum_node(set->root);
    return iter;
}

void set_destroyiter(set_iter_t *iter)
{
    free(iter);
}

int set_hasnext(set_iter_t *iter)
{
    return iter != NULL && iter->node != NULL;
}

// Returns the next element in the set
void *set_next(set_iter_t *iter)
{
    if (iter == NULL)
        return NULL;
    
    void *elem = iter->node->value; //Used as a temporary value to hold the current value of the node
    
    if (iter->node->right != NULL)  {
        iter->node = iter->node->right;
        while (iter->node->left != NULL)    {
            iter->node = iter->node->left;
        }
    }   else    {
        snode_t *child; // Child of the iterator
        while(1){
            child = iter->node;
            iter->node = iter->node->parent;
            if (iter->node == NULL) {
                break;
            }
            if (child == iter->node->left)
                break;
            
        }
    }
    
    return elem;
}

static snode_t *maximum_node(snode_t *n)
{
    assert (n != NULL);
    while (n->right != NULL) {
        n = n->right;
    }
    return n;
}

static snode_t *minimum_node(snode_t *n)
{
    //assert (n != NULL);
    if(n == NULL) return NULL;
    while (n->left != NULL) {
        n = n->left;
    }
    return n;
}









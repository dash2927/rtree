#ifndef RTREE_H__
#define RTREE_H__

#include <memory>
#include <vector>
#include <array>

using std::shared_ptr; using std::unique_ptr; using std::vector; using std::array;

// David Shackelford
// Resources: 
// https://www.cse.cuhk.edu.hk/~taoyf/course/infs4205/lec/rtree.pdf
// https://www.ibm.com/docs/en/informix-servers/14.10?topic=method-insertion-into-r-tree-index

// Define the maximum number of objects that can be held in 
// each node before a split is carried out.
#define ORDER 4

typedef struct rect rect;
typedef struct entry entry;
typedef struct node node;

struct rect {
    array<double, 2> min; // x, y coordinates of minimum (bot-left) 
    array<double, 2> max; // x, y coordinates of maximum (top-right)
};

struct entry {
    shared_ptr<node> next;
    shared_ptr<node> curr;
    array<double, 2> data;  // If the node is a leaf, data will be x, y point
};

struct node {
    bool is_leaf;
    int num_entries;
    unique_ptr<rect> bounds;                // If the node is an internal node, data will be rect
    shared_ptr<entry> parent;               // ptr to parent node
    array<shared_ptr<entry>, ORDER+1> keys; // entries within a node for rtree
};

class rtree {
public:
    // constructor
    rtree();
    // deconstructor
    ~rtree();
    // create a new node with initiated values, will set to leaf node automatically
    shared_ptr<node> createnode();
    // insert adds a datapoint (x, y) to the tree
    void insert(array<double, 2>& datapt);
    // query finds all entries that are within a boxed-boundary
    vector<shared_ptr<entry>> query(double xmin, double xmax, double ymin, double ymax);
    // get_count gets the number of entries total in the tree
    int get_count();
    // get the head node of the tree
    shared_ptr<node> get_head();
    // get volume get the volume of an entry if it is an internal node. Gives -1 if it is a datapoint
    double get_area(shared_ptr<node> n);
    // get the perimeter of an entry if it is an internal node. Gives -1 if it is a datapoint
    double get_perimeter(shared_ptr<node> n);
    // prints contents of rectangle including bounds and number of entries
    // used for debugging and as a substritute for graphical visualization
    void printnode(shared_ptr<node> node);
    // prints entire tree in hierarchal fashion
    void print();
    
private:
    int count;
    shared_ptr<node> head;

    // helper for insert function
    void insert_hlp(shared_ptr<node> root, array<double, 2>& datapt);
    // helper for query function
    void query_hlp(double& xmin, double& xmax, double& ymin, double& ymax, vector<shared_ptr<entry>>& ret, shared_ptr<node> curr);
    // if the range box intersects the bounds of the node, return true
    bool intersects(shared_ptr<node> curr, double& xmin, double& xmax, double& ymin, double& ymax);
    // helper for print function
    void print_hlp(shared_ptr<node> curr, int& level);
    // update bounds for current node and all nodes up tree
    void update_bound(shared_ptr<node> node);
    // if creating a new leaf node, will get new boundary values
    void get_bound(shared_ptr<node> node);
    // splits an internal node and outputs its parent
    // split_internal is based on pseudocode described on ppt from Uni of Queensland
    // https://www.cse.cuhk.edu.hk/~taoyf/course/infs4205/lec/rtree.pdf
    shared_ptr<node> split_internal(shared_ptr<node> root);
    // splits a leaf node and outputs its parent
    // split_leaf is based on pseudocode described on ppt from Uni of Queensland
    // https://www.cse.cuhk.edu.hk/~taoyf/course/infs4205/lec/rtree.pdf
    shared_ptr<node> split_leaf(shared_ptr<node> root);
    // chooses best subtree for the current datapoint
    shared_ptr<node> choose_subtree(shared_ptr<node> root, array<double, 2>& datapt);
    // get perimeter based on an entry
    double get_perimeter(shared_ptr<entry> e);
    // get perimeter of a set of node entries. Used as a helper if an entry is not given
    double get_perimeter(vector<shared_ptr<entry>>& dataset, int start, int stop);
    // if the node has overflow, handle overflow
    double get_added_perimeter(shared_ptr<entry> e, array<double, 2>& datapt);
    // if a node has ORDER+1 entries, trigger an overflow to rebalance tree
    void handle_overflow(shared_ptr<node> root);
};

#endif // RTREE_H__

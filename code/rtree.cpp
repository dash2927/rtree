#include "rtree.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <float.h>
#include <limits>
#include <vector>

using std::cout; using std::endl; using std::begin; using std::end;

// constructor, initialize class variables and pointers here if need.
rtree::rtree(){
    shared_ptr<node> new_node = shared_ptr<node>(new node);
    this->head = createnode();
    this->count = 0;
}

//deconstructor,
rtree::~rtree(){
}

shared_ptr<node> rtree::createnode() {
    shared_ptr<node> ret = shared_ptr<node>(new node);
    ret->num_entries = 0;
    ret->is_leaf = true;
    ret->parent = nullptr;
    for (int i=0; i <= ORDER; i++) {
        ret->keys[i] = nullptr;
    }
    return ret;
}

void rtree::insert_hlp(shared_ptr<node> root, array<double, 2>& datapt) {
    shared_ptr<node> curr(root);
    if (curr->is_leaf) {
        shared_ptr<entry> new_entry = shared_ptr<entry>(new entry);
        new_entry->data = datapt;
        new_entry->next = nullptr;
        new_entry->curr = curr;
        curr->keys[curr->num_entries] = new_entry;
        curr->num_entries++;
        if (curr->num_entries == ORDER+1) {
            handle_overflow(curr);
        }
    } else {
        curr = choose_subtree(curr, datapt);
        insert_hlp(curr, datapt);
    }
    update_bound(curr);
    return;
}

void rtree::insert(array<double, 2>& datapt) {
    insert_hlp(this->head, datapt);
    this->count++;
    return;
}

void rtree::handle_overflow(shared_ptr<node> root) {
    shared_ptr<node> curr(root);
    if (curr->is_leaf == true) {
        curr = split_leaf(root);
    } else {
        curr = split_internal(root);
    }
    return;
}

shared_ptr<node> rtree::choose_subtree(shared_ptr<node> root, array<double, 2>& datapt) {
    shared_ptr<node> minnode;
    shared_ptr<node> curr(root);
    shared_ptr<entry> e;
    double perim;
    double minperim;
    // go through rectangles, get minimum perimeter change, stop at leaf
    while (!curr->is_leaf) {
        minperim = std::numeric_limits<double>::max();
        for (int i = 0; i < curr->num_entries; i++) {
            e = curr->keys[i];
            if ((perim = get_added_perimeter(e, datapt) - get_perimeter(e)) < minperim) {
                minnode = e->next;
                minperim = perim;
            }
        }
        curr = minnode;
    }
    return curr;
}

shared_ptr<node> rtree::split_internal(shared_ptr<node> root) {
    int m = root->num_entries;
    int split_ratio = std::ceil(m * 0.4);
    if (root == this->head) { 
        // if we are at the head, we need to produce an extra node/entry
        shared_ptr<node> new_head = createnode();
        shared_ptr<entry> up = shared_ptr<entry>(new entry);
        up->next = root;
        up->curr = new_head;
        new_head->is_leaf = false;
        new_head->keys[new_head->num_entries] = up;
        new_head->num_entries++;
        root->parent = up;
        this->head = new_head;
    }
    shared_ptr<entry> new_entry = shared_ptr<entry>(new entry);
    shared_ptr<node> new_child = createnode(); // our new leaf node
    new_child->is_leaf = false;
    new_child->parent = new_entry;
    new_entry->next = new_child;
    // find the minimum perimeter combination
    // sort in order of left boundaries of x-dimension
    vector<shared_ptr<entry>> nodevector(begin(root->keys), end(root->keys));
    vector<shared_ptr<entry>> nodevectormin = nodevector;
    int min_index = ORDER+1;
    double min_perim = std::numeric_limits<double>::max();
    double peri;
    std::sort(begin(nodevector), end(nodevector),
              [](shared_ptr<entry> a, shared_ptr<entry> b) {
                  return a->next->bounds->min[0] < b->next->bounds->min[0];});
    for (int i = split_ratio; i <= m-split_ratio; i++) {
        if ((peri = (get_perimeter(nodevector, 0, i-1) +
                get_perimeter(nodevector, i, ORDER))) < min_perim) {
            min_perim = peri;
            nodevectormin = nodevector;
            min_index = i;
        }
    }
    // sort in order of right boundaries of x-dimension
    std::sort(begin(root->keys), end(root->keys),
              [](shared_ptr<entry> a, shared_ptr<entry> b) {
                  return a->next->bounds->max[0] < b->next->bounds->max[0];});
    for (int i = split_ratio; i <= m-split_ratio; i++) {
        if ((peri = (get_perimeter(nodevector, 0, i-1) +
                get_perimeter(nodevector, i, ORDER))) < min_perim) {
            min_perim = peri;
            nodevectormin = nodevector;
            min_index = i;
        }
    }
    // sort in order of bottom boundaries of y-dimension
    std::sort(begin(root->keys), end(root->keys),
              [](shared_ptr<entry> a, shared_ptr<entry> b) {
                  return a->next->bounds->min[1] < b->next->bounds->min[1];});
    for (int i = split_ratio; i <= m-split_ratio; i++) {
        if ((peri = (get_perimeter(nodevector, 0, i-1) +
                get_perimeter(nodevector, i, ORDER))) < min_perim) {
            min_perim = peri;
            nodevectormin = nodevector;
            min_index = i;
        }
    }
    // sort in order of top boundaries of y-dimension
    std::sort(begin(root->keys), end(root->keys),
              [](shared_ptr<entry> a, shared_ptr<entry> b) {
                  return a->next->bounds->max[1] < b->next->bounds->max[1];});
    for (int i = split_ratio; i <= m-split_ratio; i++) {
        if ((peri = (get_perimeter(nodevector, 0, i-1) +
                get_perimeter(nodevector, i, ORDER))) < min_perim) {
            min_perim = peri;
            nodevectormin = nodevector;
            min_index = i;
        }
    }
    int i = 0;
    for (; i < min_index; i++) {
        root->keys[i] = nodevectormin[i];
    }
    for (; i <= ORDER; i++) {
        new_child->keys[i-min_index] = nodevectormin[i];
        new_child->keys[i-min_index]->curr = new_child;
        root->keys[i] = nullptr;
        new_child->num_entries++;
        root->num_entries--;
    }
    // update new_entry pointers and insert it into parent node
    new_entry->curr = root->parent->curr;
    new_entry->curr->keys[new_entry->curr->num_entries] = new_entry;
    new_entry->curr->num_entries++;
    // get the bounds of new node and update rest
    get_bound(new_child);
    if (new_entry->curr->num_entries == ORDER+1){
        handle_overflow(new_entry->curr);
    }
    update_bound(root);
    return new_entry->curr;
}

shared_ptr<node> rtree::split_leaf(shared_ptr<node> root) {
    int m = root->num_entries;
    int split_ratio = std::ceil(m*0.4);
    if (root == this->head) { 
        // if we are at the head, we need to produce an extra node/entry
        shared_ptr<node> new_head = createnode();
        shared_ptr<entry> up = shared_ptr<entry>(new entry);
        up->next = root;
        up->curr = new_head;
        new_head->is_leaf = false;
        root->is_leaf = true;
        new_head->keys[new_head->num_entries] = up;
        new_head->num_entries++;
        root->parent = up;
        this->head = new_head;
    }
    shared_ptr<entry> new_entry = shared_ptr<entry>(new entry);
    shared_ptr<node> new_child = createnode();// our new leaf node
    new_child->parent = new_entry;
    new_entry->next = new_child;
    // find minimum perimeter combination
    // sort in order of x-value
    std::sort(begin(root->keys), end(root->keys),
              [](shared_ptr<entry> a, shared_ptr<entry> b) {
                  return a->data[0] < b->data[0];}); // nlogn time complexity sort
    // assuming we have a full leaf, vector should be length ORDER
    vector<shared_ptr<entry>> leafvector(begin(root->keys), end(root->keys));
    // init minimum perims as maximum double, index for splitting
    double min_perim_x = std::numeric_limits<double>::max();
    double min_perim_y = min_perim_x;
    int min_index_x = ORDER+1;
    int min_index_y = min_index_x;
    double peri; 
    for (int i = split_ratio; i <= m-split_ratio; i++) {
        if ((peri = (get_perimeter(leafvector, 0, i-1) +
                get_perimeter(leafvector, i, ORDER))) < min_perim_x) {
            min_perim_x = peri;
            min_index_x = i;
        }
    }
    // sort in order of y-value - set it to leafvector
    std::sort(begin(leafvector), end(leafvector),
        [](shared_ptr<entry> a, shared_ptr<entry> b) {
            return a->data[1] < b->data[1];});
    for (int i = split_ratio; i <= m-split_ratio; i++) {
        if ((peri = (get_perimeter(leafvector, 0, i-1) + 
                get_perimeter(leafvector, i, ORDER))) < min_perim_y) {
            min_perim_y = peri;
            min_index_y = i; 
        }
    }
    // allocate new children
    // if perim based on y split is smaller, use y-split
    int i = 0;
    if (min_perim_y < min_perim_x) { // use values from leafvector
        for (; i < min_index_y; i++) {
            root->keys[i] = leafvector[i];
        }
        for (; i <=ORDER; i++) {
            new_child->keys[i-min_index_y] = leafvector[i];
            root->keys[i] = nullptr;
            new_child->num_entries++;
            root->num_entries--;
        }
    } else { // else use x-split; use values from root->keys
        for (i = min_index_x; i <=ORDER; i++) {
            new_child->keys[i-min_index_x] = root->keys[i];
            root->keys[i] = nullptr;
            new_child->num_entries++;
            root->num_entries--;
        }
    }
    // update new_entry pointers and insert it into parent node
    new_entry->curr = root->parent->curr;
    new_entry->curr->keys[new_entry->curr->num_entries] = new_entry;
    new_entry->curr->num_entries++;
    // get the bounds of new node and update rest
    get_bound(new_child);
    if (new_entry->curr->num_entries == ORDER+1){
        handle_overflow(new_entry->curr);
    }
    update_bound(root);
    return new_entry->curr;
}

void rtree::get_bound(shared_ptr<node> node) {
    node->bounds = unique_ptr<rect>(new rect);
    node->bounds->min = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max()};
    node->bounds->max = {-std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()};
    shared_ptr<entry> child;
    for (int i = 0; i < node->num_entries; i++) {
        child = node->keys[i];
        if (node->is_leaf) {
            if (child->data[0] > node->bounds->max[0])
                node->bounds->max[0] = child->data[0];
            if (child->data[0] < node->bounds->min[0])
                node->bounds->min[0] = child->data[0];
            if (child->data[1] > node->bounds->max[1])
                node->bounds->max[1] = child->data[1];
            if (child->data[1] < node->bounds->min[1])
                node->bounds->min[1] = child->data[1];
        } else {
            if (child->next->bounds->max[0] > node->bounds->max[0])
                node->bounds->max[0] = child->next->bounds->max[0];
            if (child->next->bounds->min[0] < node->bounds->min[0])
                node->bounds->min[0] = child->next->bounds->min[0];
            if (child->next->bounds->max[1] > node->bounds->max[1])
                node->bounds->max[1] = child->next->bounds->max[1];
            if (child->next->bounds->min[1] < node->bounds->min[1])
                node->bounds->min[1] = child->next->bounds->min[1];
        }
    }
    
}

void rtree::update_bound(shared_ptr<node> node) {
    double max_x = -std::numeric_limits<double>::max();
    double max_y = max_x;
    double min_x = std::numeric_limits<double>::max();
    double min_y = min_x;
    if (!node->bounds) { // if we dont have a bounds, we must create one.
        node->bounds = unique_ptr<rect>(new rect);
        node->bounds->min = {min_x, min_y};
        node->bounds->max = {max_x, max_y};
    }
    bool found_new_bounds = false;
    shared_ptr<entry> child;
    // if our entry has a next node (meaning no data),
    // base the new bounds off of the bounds lower in the tree
    if (!node->is_leaf) { 
        for (int i = 0; i < node->num_entries; i++) {
            child = node->keys[i];
            if (child->next->bounds->min[0] < min_x) {
                min_x = child->next->bounds->min[0];
            }
            if (child->next->bounds->min[1] < min_y) {
                min_y = child->next->bounds->min[1];
            }
            if (child->next->bounds->max[0] > max_x) {
                max_x = child->next->bounds->max[0];
            }
            if (child->next->bounds->max[1] > max_y) {
                max_y = child->next->bounds->max[1];
            }
        }
    } else { // otherwise, we have data
        for (int i = 0; i < node->num_entries; i++) {
            child = node->keys[i];
            if (child->data[0] > max_x) {
                max_x = child->data[0];
            }
            if (child->data[0] < min_x) {
                min_x = child->data[0];
            }
            if (child->data[1] > max_y) {
                max_y = child->data[1];
            }
            if (child->data[1] < min_y) {
                min_y = child->data[1];
            }
        }
    }
    if (max_x != node->bounds->max[0]) {
        node->bounds->max[0] = max_x;
        found_new_bounds = true;
    }
    if (max_y != node->bounds->max[1]) {
        node->bounds->max[1] = max_y;
        found_new_bounds = true;
    }
    if (min_x != node->bounds->min[0]) {
        node->bounds->min[0] = min_x;
        found_new_bounds = true;
    }
    if (min_y != node->bounds->min[1]) {
        node->bounds->min[1] = min_y;
        found_new_bounds = true;
    }
    if (node->parent && found_new_bounds){
        update_bound(node->parent->curr);}
    return;
}

bool rtree::intersects(shared_ptr<node> curr, double& xmin, double& xmax, double& ymin, double& ymax) {
    if (curr->bounds->max[0] <= xmin || curr->bounds->min[0] >= xmax)
        return false;
    if (curr->bounds->max[1] <= ymin || curr->bounds->min[1] >= ymax)
        return false;
    return true;
}

void rtree::query_hlp(double& xmin, double& xmax, double& ymin, double& ymax, vector<shared_ptr<entry>>& ret, shared_ptr<node> curr) {
    if (curr->is_leaf) { // if the current node is a leaf node,
        for (int i = 0; i < curr->num_entries; i++) { // store all entries covered
            if (curr->keys[i]->data[0] >= xmin &&
                curr->keys[i]->data[0] <= xmax &&
                curr->keys[i]->data[1] >= ymin &&
                curr->keys[i]->data[1] <= ymax) {
                ret.push_back(curr->keys[i]);
            }
        }
    } else {
        for (int i = 0; i < curr->num_entries; i++) {
            if (intersects(curr->keys[i]->next, xmin, xmax, ymin, ymax)) {
                query_hlp(xmin, xmax, ymin, ymax, ret, curr->keys[i]->next);
            }
        }
    }
}

vector<shared_ptr<entry>> rtree::query(double xmin,  double ymin, double xmax, double ymax) {
    vector<shared_ptr<entry>> ret;
    shared_ptr<node> curr(this->head);
    query_hlp(xmin, xmax, ymin, ymax, ret, curr);
    return ret;
}

int rtree::get_count() {
    return this->count;
}

shared_ptr<node> rtree::get_head() {
    return this->head;
}

double rtree::get_area(shared_ptr<node> n) {
    if (!(n->bounds))
        return -1;
    double max_x, max_y, min_x, min_y;
    max_x = n->bounds->max[0];
    min_x = n->bounds->min[0];
    max_y = n->bounds->max[1];
    min_y = n->bounds->min[1];
    return (max_x - min_x)*(max_y - min_y);
}

double rtree::get_perimeter(shared_ptr<node> n) {
    if (!(n->bounds))
        return -1;
    double max_x, max_y, min_x, min_y;
    max_x = n->bounds->max[0];
    min_x = n->bounds->min[0];
    max_y = n->bounds->max[1];
    min_y = n->bounds->min[1];
    return 2*(max_x - min_x) + 2*(max_y - min_y);
}

double rtree::get_perimeter(shared_ptr<entry> e) {
    if (!(e->next->bounds))
        return -1;
    return 2*(e->next->bounds->max[0] - e->next->bounds->min[0])
        + 2*(e->next->bounds->max[1] - e->next->bounds->min[1]);
}

double rtree::get_perimeter(vector<shared_ptr<entry>>& dataset, int start, int stop) {
    double max_x = -std::numeric_limits<double>::max();
    double max_y = max_x;
    double min_x = std::numeric_limits<double>::max();
    double min_y = min_x;
    for (int i = start; i <= stop; i++) {
        if (dataset[i]->curr->is_leaf) {
            if (dataset[i]->data[0] > max_x) max_x = dataset[i]->data[0];
            if (dataset[i]->data[0] < min_x) min_x = dataset[i]->data[0];
            if (dataset[i]->data[1] > max_y) max_y = dataset[i]->data[1];
            if (dataset[i]->data[1] < min_y) min_y = dataset[i]->data[1];
        } else {
            if (dataset[i]->next->bounds->max[0] > max_x) max_x = dataset[i]->next->bounds->max[0];
            if (dataset[i]->next->bounds->min[0] < min_x) min_x = dataset[i]->next->bounds->min[0];
            if (dataset[i]->next->bounds->max[1] > max_y) max_y = dataset[i]->next->bounds->max[1];
            if (dataset[i]->next->bounds->min[1] < min_y) min_y = dataset[i]->next->bounds->min[1];
        }
    }
    return 2*(max_x - min_x) + 2*(max_y - min_y);
}

double rtree::get_added_perimeter(shared_ptr<entry> e, array<double, 2>& datapt) {
    if (!(e->next->bounds))
        return -1;
    double perim = 2*(e->next->bounds->max[0] - e->next->bounds->min[0])
        + 2*(e->next->bounds->max[1] - e->next->bounds->min[1]);
    if (datapt[0] > e->next->bounds->max[0]) {
        perim += 2*(datapt[0] - e->next->bounds->max[0]);
    } else if (datapt[0] < e->next->bounds->min[0]) {
        perim += 2*(e->next->bounds->min[0] - datapt[0]);
    }
    if (datapt[1] > e->next->bounds->max[1]) {
        perim += 2*(datapt[1] - e->next->bounds->max[1]);
    } else if (datapt[1] < e->next->bounds->min[1]) {
        perim += 2*(e->next->bounds->min[1] - datapt[1]);
    }
    return perim;
}

void rtree::printnode(shared_ptr<node> node) {
    if (node->is_leaf) {
        cout << "leaf";
    } else {
        cout << "internal";
    }
    cout << " node has " << node->num_entries << " entries" << endl;
    if (node->is_leaf) {
        for (int i = 0; i < node->num_entries; i++) {
            cout << " " << node->keys[i]->data[0] << ", " 
                 << node->keys[i]->data[1] << endl;
        }
    } else {
        for (int i = 0; i < node->num_entries; i++) {
            cout << " (min[x,y],max[x,y])=[" 
                 << node->keys[i]->next->bounds->min[0] << " "
                 << node->keys[i]->next->bounds->min[1] << "],[" 
                 << node->keys[i]->next->bounds->max[0]
                 << " " << node->keys[i]->next->bounds->max[1] << "]" 
                 << endl;
        }
    }
    cout << "end node: bound (min[x,y],max[x,y])=[" << node->bounds->min[0] << " "
         << node->bounds->min[1] << "],[" << node->bounds->max[0]
         << " " << node->bounds->max[1] << "]" << endl;
}

void rtree::print_hlp(shared_ptr<node> curr, int& level) {
    for (int i = 0; i < level; i++) {
        cout << "|";
    }
    if (curr->is_leaf) {
        cout << "+leaf ";
    } else {
        cout << "+internal ";
    }
    cout << "node has " << curr->num_entries << " entries and a bound of "
         << "(min[x,y],max[x,y])=[" << curr->bounds->min[0] << " "
         << curr->bounds->min[1] << "],[" << curr->bounds->max[0]
         << " " << curr->bounds->max[1] << "]:" << endl;
    if (curr->is_leaf){
        for (int i = 0; i < curr->num_entries; i++) {
            for (int i = 0; i < level+1; i++) {
                cout << "|";
            }
            cout << "(" << curr->keys[i]->data[0] << ", " << curr->keys[i]->data[1] << ")";
            cout << endl;
        }
    } else {
        level++;
        for (int i = 0; i < curr->num_entries; i++) {
            print_hlp(curr->keys[i]->next, level);
        }
        level--;
    }
}

void rtree::print() {
    cout << "TREE START: TOTAL_DATAPTS=" << get_count() << endl;
    int level = 0;
    print_hlp(this->head, level);
}
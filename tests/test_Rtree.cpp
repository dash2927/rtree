#include <gtest/gtest.h>
#include "../code/rtree.h"

#include <iostream>
#include <string>

using std::cout; using std::endl; using std::array;

class test_Rtree : public ::testing::Test {
protected:
	// This function runs only once before any TEST_F function
	static void SetUpTestCase(){
	}
	// This function runs after all TEST_F functions have been executed
	static void TearDownTestCase(){
	}
	// this function runs before every TEST_F function
	void SetUp() override {
		cout << "START TESTS" << endl;
    }
	// this function runs after every TEST_F function
	void TearDown() override {
		cout << "END OF TESTS" << endl;
	}
};

TEST_F(test_Rtree, SanityChecks){
	cout << "Testing for blank node..." << endl;
    rtree blanktree = rtree();
	for (int i = 0; i <= ORDER; i++) {
		ASSERT_EQ(blanktree.get_head()->keys[i], nullptr);
	}
	ASSERT_EQ(blanktree.get_head()->num_entries, 0);
	ASSERT_EQ(blanktree.get_head()->is_leaf, true);
	ASSERT_EQ(blanktree.get_head()->parent, nullptr);
	cout << "Testing for insert into single node..." << endl;
	array<double, 2> pt1{{1.0, 1.0}};
	array<double, 2> pt2{{2.0, 2.0}};
	array<double, 2> pt3{{3.0, 3.0}};
	array<double, 2> pt4{{4.0, 4.0}};
	blanktree.insert(pt1);
	ASSERT_EQ(blanktree.get_count(), 1);
	blanktree.insert(pt2);
	ASSERT_EQ(blanktree.get_count(), 2);
	blanktree.insert(pt3);
	ASSERT_EQ(blanktree.get_count(), 3);
	blanktree.insert(pt4);
	ASSERT_EQ(blanktree.get_count(), 4);
	ASSERT_EQ(blanktree.get_head()->num_entries, ORDER);
	ASSERT_EQ(blanktree.get_head()->keys[0]->data, pt1);
	ASSERT_EQ(blanktree.get_head()->keys[1]->data, pt2);
	ASSERT_EQ(blanktree.get_head()->keys[2]->data, pt3);
	ASSERT_EQ(blanktree.get_head()->keys[3]->data, pt4);
	ASSERT_EQ(blanktree.get_head()->keys[4], nullptr);
}

TEST_F(test_Rtree, BasicFunctions){
	cout << "Testing for trangle shaped data..." << endl;
	// make a traiangle which will have a square boundary of side 4
	rtree triangle = rtree();
	array<double, 2> pt1{{1.0, 1.0}};
	array<double, 2> pt2{{5.0, 1.0}};
	array<double, 2> pt3{{2.0, 5.0}};
	triangle.insert(pt1);
	triangle.insert(pt2);
	triangle.insert(pt3);
	double perimeter = triangle.get_perimeter(triangle.get_head());
	double area = triangle.get_area(triangle.get_head());
	cout << "perimeter: " << perimeter << ", area:" << area << endl;
	ASSERT_EQ(perimeter, 16);
	ASSERT_EQ(area, 16);
	cout << "Testing for rectangle..." << endl;
	// make a long rectangle with some negative and decimal pts
	rtree longrectangle = rtree();
	pt1 = {-1.0, -1.0};
	pt2 = {-1.0, 1000.0};
	pt3 = {-0.5, 1000.0};
	array<double, 2> pt4{{-0.5, -1.0}};
	longrectangle.insert(pt1);
	longrectangle.insert(pt2);
	longrectangle.insert(pt3);
	longrectangle.insert(pt4);
	perimeter = longrectangle.get_perimeter(longrectangle.get_head());
	area = longrectangle.get_area(longrectangle.get_head());
	cout << "perimeter: " << perimeter << ", area:" << area << endl;
	ASSERT_EQ(perimeter, 2003);
	ASSERT_EQ(area, 500.5);
}

TEST_F(test_Rtree, TestSplitLeaf){
	// testing for a simple tree from scratch
    rtree simpletree = rtree();
	cout << "Testing for overfilled leaf node..." << endl;
	array<double, 2> pt1{{1.0, 5.0}};
	array<double, 2> pt2{{2.0, 4.0}};
	array<double, 2> pt3{{3.0, 3.0}};
	array<double, 2> pt4{{4.0, 2.0}};
	array<double, 2> pt5{{5.0, 1.0}};
	simpletree.insert(pt3);
	simpletree.insert(pt1);
	simpletree.insert(pt4);
	simpletree.insert(pt2);
	simpletree.insert(pt5);
	simpletree.printnode(simpletree.get_head());
	simpletree.printnode(simpletree.get_head()->keys[0]->next);
	simpletree.printnode(simpletree.get_head()->keys[1]->next);
	cout << "Testing for simple leaf node with ysplit..." << endl;
	rtree ysplitsimpletree = rtree();
	// pt1 and pt4 should be combined to one node
	pt1 = {-1.0, 20.7}; //
	pt2 = {5.0, -22.0};
	pt3 = {-5.0, -21.0};
	pt4 = {1.0, 22.5}; //
	pt5 = {-1.0, -20.0};
	ysplitsimpletree.insert(pt3);
	ysplitsimpletree.insert(pt1);
	ysplitsimpletree.insert(pt4);
	ysplitsimpletree.insert(pt2);
	ysplitsimpletree.insert(pt5);
	shared_ptr<node> curr(ysplitsimpletree.get_head());
	ASSERT_TRUE(!curr->is_leaf);
	ASSERT_EQ(curr->num_entries, 2);
	ASSERT_EQ(curr->bounds->min[0], -5.0);
	ASSERT_EQ(curr->bounds->min[1], -22.0);
	ASSERT_EQ(curr->bounds->max[0], 5.0);
	ASSERT_EQ(curr->bounds->max[1], 22.5);
	curr = curr->keys[0]->next;
	ASSERT_TRUE(curr->is_leaf);
	ASSERT_EQ(curr->num_entries, 3);
	ASSERT_EQ(curr->bounds->min[0], -5.0);
	ASSERT_EQ(curr->bounds->min[1], -22.0);
	ASSERT_EQ(curr->bounds->max[0], 5.0);
	ASSERT_EQ(curr->bounds->max[1], -20);
	curr = ysplitsimpletree.get_head()->keys[1]->next;
	ASSERT_TRUE(curr->is_leaf);
	ASSERT_EQ(curr->num_entries, 2);
	ASSERT_EQ(curr->bounds->min[0], -1.0);
	ASSERT_EQ(curr->bounds->min[1], 20.7);
	ASSERT_EQ(curr->bounds->max[0], 1.0);
	ASSERT_EQ(curr->bounds->max[1], 22.5);
	ysplitsimpletree.printnode(ysplitsimpletree.get_head());
	ysplitsimpletree.printnode(ysplitsimpletree.get_head()->keys[0]->next);
	ysplitsimpletree.printnode(ysplitsimpletree.get_head()->keys[1]->next);
}

TEST_F(test_Rtree, TestChooseSubtree) {
	// simple test for running through split
	rtree incrementaltree = rtree();
	cout << "Testing for subtree decision making..." << endl;
	array<double, 2> inspt;
	// create an incremental tree as before
	for (int i = 0; i < 5; i++){
		inspt = {(double) i + 1, 5 - (double)i};
		incrementaltree.insert(inspt);
	}
	shared_ptr<node> curr(incrementaltree.get_head());
	// test for point inside a rectangle
	inspt = {1.5, 4.5}; // will be placed in [1 4],[2 5] bound node
	incrementaltree.insert(inspt);
	incrementaltree.print();
	// test for point outside a rectangle (test for min perimeter)
	inspt = {2.80, 2.90}; // will be placed in [3 1],[5 3] bound node
	incrementaltree.insert(inspt);
	incrementaltree.print();
	inspt = {1.5, 4.5}; // insert a node that's the same
	incrementaltree.insert(inspt);
	ASSERT_EQ(curr->keys[0]->next->num_entries, 4);
	ASSERT_EQ(curr->num_entries, 2);
	incrementaltree.print();
	inspt = {4, 2}; // trigger a second split
	incrementaltree.insert(inspt);
	incrementaltree.print();
	ASSERT_EQ(curr->num_entries, 3);
}

TEST_F(test_Rtree, TestSplitInternal) {
	rtree incrementaltree = rtree();
	cout << "Testing for subtree decision making..." << endl;
	array<double, 2> inspt;
	// create an incremental tree as before
	for (int i = 0; i < 10; i++){
		inspt = {(double) i + 1, 10 - (double)i};
		incrementaltree.insert(inspt);
	}
	shared_ptr<node> curr(incrementaltree.get_head());
	// will be inserted to 4th entry, cause leaf split, and cause internal node split
	inspt = {5, 6}; 
	incrementaltree.insert(inspt);
	inspt = {32, 27}; 
	incrementaltree.insert(inspt);
	ASSERT_EQ(incrementaltree.get_count(), 12);
	ASSERT_EQ(curr->num_entries, 2);
	ASSERT_FALSE(curr->is_leaf);
	// incrementaltree.print();
	cout << "Testing for split when many of the same entries are inputed..." << endl;
	rtree sametreetree = rtree();
	inspt = {0, 0};
	// create an incremental tree as before
	for (int i = 0; i < 50; i++){
		sametreetree.insert(inspt);
	}
	ASSERT_EQ(sametreetree.get_count(), 50);
	rtree bigtree = rtree();
	cout << "Testing for large varied tree construction..." << endl;
	// create a large tree
	for (double i = 0; i < 5000; i++){
		inspt = { ((i)*(i) - (i - 7)*(i - 7)), (2 - i)*(2 - i)*(2 - i)};
		bigtree.insert(inspt);
	}
	for (double i = -1; i > -5000; i--){
		inspt = { ((i)*(i) - (i - 7)*(i + 7)), (2 - i)*(2 - i)*(2 - i)};
		bigtree.insert(inspt);
	}
	bigtree.insert(inspt);
	ASSERT_EQ(bigtree.get_count(), 10000);
	// bigtree.print();
	
}

TEST_F(test_Rtree, TestQuery) {
	cout << "Testing for query of a tree..." << endl;
	rtree incrementaltree = rtree();
	array<double, 2> inspt;
	// create an incremental tree as before
	for (int i = 0; i < 10; i++){
		inspt = {(double) i + 1, 10 - (double)i};
		incrementaltree.insert(inspt);
	}
	inspt = {5, 6}; 
	incrementaltree.insert(inspt);
	inspt = {32, 27}; 
	incrementaltree.insert(inspt);
	incrementaltree.print();
	cout << "Testing for all pts in box..." << endl;
	vector<shared_ptr<entry>> values = incrementaltree.query(1, 9, 2, 10); // query small tree
	// query results should be (1, 10), (2, 9)
	ASSERT_EQ(values[0]->data[0], 1);
	ASSERT_EQ(values[0]->data[1], 10);
	ASSERT_EQ(values[1]->data[0], 2);
	ASSERT_EQ(values[1]->data[1], 9);
	for (shared_ptr<entry> e : values) {
		cout << "(" << e->data[0] << ", " << e->data[1] << ") ";
	}
	cout << endl;
	cout << "Testing for excluding pt in box..." << endl;
	values = incrementaltree.query(8.75, .5, 10.1, 2.1); // exclude one point from bound
	ASSERT_EQ(values[0]->data[0], 9); // should be (9, 2), (10, 1)
	ASSERT_EQ(values[0]->data[1], 2);
	ASSERT_EQ(values[1]->data[0], 10);
	ASSERT_EQ(values[1]->data[1], 1);
	for (shared_ptr<entry> e : values) {
		cout << "(" << e->data[0] << ", " << e->data[1] << ") ";
	}
	cout << endl;
	cout << "Testing for including pt in other box..." << endl;
	values = incrementaltree.query(7.9, .5, 10.1, 3.1); // exclude one point from bound
	ASSERT_EQ(values[0]->data[0], 8); // should be (8, 3), (9, 2), (10, 1)
	ASSERT_EQ(values[0]->data[1], 3);
	ASSERT_EQ(values[1]->data[0], 9);
	ASSERT_EQ(values[1]->data[1], 2);
	ASSERT_EQ(values[2]->data[0], 10);
	ASSERT_EQ(values[2]->data[1], 1);
	for (shared_ptr<entry> e : values) {
		cout << "(" << e->data[0] << ", " << e->data[1] << ") ";
	}
	cout << endl;
}


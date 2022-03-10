David Shackelford

Description:
Simple RTree implementation with query function: This is my first implementation of an RTree data structure which
will store/query sets of x, y datapoints (as objects) in an efficient manner (O(m*log_mn)), and create rectangular 
minimum-bound ranges for sets of datapoints. The amount of datapoints in a range set can be defined in the rtree.h 
file under the ORDER variable. Visual examples of this data structure can be seen in resources below.

Future Plans: 
KNN function, 3d implementation, multidimmensional object storage, better visualization through plotly wrapper

Resources: 
    * https://www.cse.cuhk.edu.hk/~taoyf/course/infs4205/lec/rtree.pdf
    * https://www.ibm.com/docs/en/informix-servers/14.10?topic=method-insertion-into-r-tree-index

To Run:

1. unzip folder into desired location

    tar -zxf rtree.tar ./

2. cd into build folder 

    cd rtree/build

3. run cmake

    cmake ..

4. run make

    make

5. To run tests, type ./run_tests

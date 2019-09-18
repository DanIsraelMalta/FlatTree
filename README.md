# FlatTree

FlatTree is a general purpose flat tree data structure (i.e. - data structure in which values have parent-child relationships to each other).

Main characteristics:
* each node can have only one parent.
* nodes are stored in continuous manner and can be easily iterated using 'begin'/'end' iterators (so all STL algorithms can be used).
* root node is located at index '0'.
* a tree always has a root node.

example usage:

```c

// create tree
FlatTree<std::string> a("root");

// add nodes to tree
a << std::make_pair(0, "child1");                                                       // add node "child1" as a child to the root node
a << std::make_pair(0, "child2");                                                       // add node "child2" as a child to the root node
a << std::make_pair(1, "grand child 0");                                                // add node "grand child 0" as a child to the ''child1' node
a << std::make_pair(1, std::vector<std::string>{ "grand child 1", "grand child 2" });   // add nodes "grand child 1", "grand child 2" as a childreb to the ''child1' node
a << std::make_pair(2, std::vector<std::string>{ "grand child 3", "grand child 4" });   // add nodes "grand child 3", "grand child 4" as a childreb to the ''child2' node

// traverse sub-tree staring with node "child1" in a sequential manner
a.Traverse(1, std::execution::seq, [i = 0](auto& node) mutable {
	node += "_";
	++i;
});

// traverse all the tree in parallel manner
a.Traverse(0, std::execution::par, [i = 0](auto& node) mutable {
	node += std::to_string(i);
	++i;
});

// use STL algorithm to operate on the tree
std::for_each(a.begin(), a.end(), [&, i = 0](auto& node) mutable {
	node += std::to_string(i);
	++i;
});

 // get/change nodes
auto first_child_name = a[1];
a[1] = "changed_name";
first_child_name = a[1];
assert(first_child_name.compare("changed_name") == 0);

// get all 'child1' descendants
std::vector<std::size_t> child1_kids;
a.getAllDescendants(1, child1_kids);

// remove nodes
a >> 1; // remove "child1" and its descendants
    
'''

see main.c for more API and operations.


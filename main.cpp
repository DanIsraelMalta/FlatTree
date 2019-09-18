#include "FlatTree.h"
#include <string.h>
#include <algorithm>
#include <array>
#include <list>
#include <iostream>

void constructionTest() {

    // -- trivial constructors ---
    FlatTree<std::string> a("root"),
                          b(std::vector<std::string>{"coco", "moly", "acra", "cricket"},
                            std::vector<std::size_t>{0, 0, 0, 2});

    assert(a.size()  == 1);
    assert(a.empty() == true);

    assert(b.size()  == 4);
    assert(b.empty() == false);

    std::vector<std::string> data{ "coco", "moly", "acra", "cricket" };
    std::for_each(b.begin(), b.end(), [&, i = 0](const auto& elm) mutable {
        assert(elm.compare(data[i]) == 0);
        ++i;
    });

    // --- construct from various collections ---

    // construct from array
    FlatTree<std::string> c(std::array<std::string, 4>{ "coco", "moly", "acra", "cricket" }, 
                            std::array<std::size_t, 4>{0, 0, 0, 2});
    std::for_each(c.begin(), c.end(), [&, i = 0](const auto& elm) mutable {
        assert(elm.compare(data[i]) == 0);
        ++i;
    });

    // construct from list and array
    const std::list<std::string> names({ "coco", "moly", "acra", "cricket" });
    const std::array<std::size_t, 4> index{ 0, 0, 0, 2 };
    FlatTree<std::string> d(names, index);
    std::for_each(d.begin(), d.end(), [&, i = 0](const auto& elm) mutable {
        assert(elm.compare(data[i]) == 0);
        ++i;
    });
}

void modifyTreeTest() {

    // create tree
    FlatTree<std::string> a("root");

    // add nodes to tree
    a << std::make_pair(0, "child1");
    a << std::make_pair(0, "child2");
    a << std::make_pair(1, "grand child 0");
    a << std::make_pair(1, std::vector<std::string>{ "grand child 1", "grand child 2" });
    a << std::make_pair(2, std::vector<std::string>{ "grand child 3", "grand child 4" });

    // tree info
    const auto num_of_root_nodes = a.getNumOfDescendants(0);
    assert(num_of_root_nodes == 3);

    const auto num_of_child1_kids = a.getNumOfDescendants(1);
    assert(num_of_child1_kids == 3);

    assert(a.isLeaf(0) == false);   // root
    assert(a.isLeaf(3) == true);    // grand child 0
    assert(a.doesIndexExist(0) == true);
    assert(a.doesIndexExist(3) == false);

    // get parents of kids
    assert(a.getParentIndex(1) == 0);
    assert(a.getParentIndex(2) == 0);
    assert(a.getParentIndex(3) == 1);
    assert(a.getParentIndex(4) == 1);
    assert(a.getParentIndex(5) == 1);
    assert(a.getParentIndex(6) == 2);
    assert(a.getParentIndex(7) == 2);

    // print structure
    std::cout << "tree (simple dump): \n"; a.dumpToConsoleSimple(); std::cout << "\n";
    std::cout << "tree (multimap dump): \n"; a.dumpToConsoleMultiMap(); std::cout << "\n";

    // get/change nodes
    auto first_child_name = a[1];
    assert(first_child_name.compare("child1") == 0);

    a[1] = "changed_name";
    first_child_name = a[1];
    assert(first_child_name.compare("changed_name") == 0);

    // get all 'child1' descendants
    std::vector<std::size_t> child1_kids;
    a.getAllDescendants(1, child1_kids);
    assert(child1_kids[0] == 3);
    assert(child1_kids[1] == 4);
    assert(child1_kids[2] == 5);
    assert(child1_kids.size() == 3);

    // remove nodes
    a >> 1; // remove "child1" and its descendants
    std::cout << "tree after 'chold1' removal: \n"; a.dumpToConsoleMultiMap(); std::cout << "\n";
}

void traverseTreeTest() {
    // create tree
    FlatTree<std::string> a({ "root", "child1", "child2", "grand child 0", "grand child 1", "grand child 2", "grand child 3", "grand child 4" },
                            {   0,      0,         0,       1,                  1,             1,                 2,                 2});

    // traverse sub-tree staring with node "child1" in a sequential manner
    a.Traverse(1, std::execution::seq, [i = 0](auto& node) mutable {
        node += "_";
        ++i;
    });
    std::cout << "tree (multimap dump): \n"; a.dumpToConsoleMultiMap(); std::cout << "\n";

    // traverse all the tree in parallel manner
    a.Traverse(0, std::execution::par, [i = 0](auto& node) mutable {
        node += std::to_string(i);
        ++i;
    });
    std::cout << "tree (multimap dump): \n"; a.dumpToConsoleMultiMap(); std::cout << "\n";
}

int main() {
    constructionTest();
    modifyTreeTest();
    traverseTreeTest();
    return 1;
}

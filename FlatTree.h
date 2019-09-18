/**
* General flat tree data structure.
*
* Dan Israel Malta
**/
#pragma once
#include <vector>
#include <type_traits>
#include <assert.h>
#include <iostream>
#include <string>
#include <execution>
#include <algorithm>
#include <memory>

// type traits
namespace {
    // test if an object is iterate-able
    template<typename T, typename = void> struct is_iterate_able                                                                                                 : std::false_type {};
    template<typename T>                  struct is_iterate_able<T, std::void_t<decltype(std::begin(std::declval<T>())), decltype(std::end(std::declval<T>()))>> : std::true_type  {};
    template<typename T> inline constexpr bool is_iterate_able_v = is_iterate_able<T>::value;

    // test if an object has a 'size' method
    template<typename T, typename = void> struct has_size_method                                : std::false_type {};
    template<typename T>                  struct has_size_method<T, decltype(&T::size, void())> : std::true_type  {};
    template<typename T> inline constexpr bool has_size_method_v = has_size_method<T>::value;

    // test if an object has a 'push_back' method
    template<typename T, typename = void> struct has_pushback_method                                     : std::false_type {};
    template<typename T>                  struct has_pushback_method<T, decltype(&T::push_back, void())> : std::true_type {};
    template<typename T> inline constexpr bool has_pushback_method_v = has_pushback_method<T>::value;

    // test if an object has a 'emplace_back' method
    template<typename T, typename = void> struct has_emplaceback_method                                        : std::false_type {};
    template<typename T>                  struct has_emplaceback_method<T, decltype(&T::emplace_back, void())> : std::true_type {};
    template<typename T> inline constexpr bool has_emplaceback_method_v = has_emplaceback_method<T>::value;

    // test if an object is std::vector or std::initializer_list
    template<typename>   struct is_vector                           : std::false_type {};
    template<typename T> struct is_vector<std::initializer_list<T>> : std::true_type {};
    template<typename T> struct is_vector<std::vector<T>>           : std::true_type {};
    template<typename T> inline constexpr bool is_vector_v = is_vector<T>::value;
}

/**
* \brief a general purpose flat tree data structure.
*        tree is built such that each node can have only one parent.
*        nodes are stored in a continuous manner and can be easily iterated using 'begin'/'end' iterators (so all STL algorithms can be used).
*        tree root is the node located at index 0. a tree always has a root node.
* 
* @param {T,              in} tree node type (should have a trivial constructor)
* @param {DataAllocator,  in} data container allocator
* @param {IndexAllocator, in} index container allocator
**/
template<typename T, 
         class DataAllocator = std::allocator<T>, 
         class IndexAllocator = std::allocator<std::size_t>> 
class FlatTree {

    // properties
    private:
        static constexpr std::size_t size_for_parallelization{ 2'000 }; // above this number of tree nodes, certain operations shall be parallelized
        std::vector<T, DataAllocator> m_data;                           // collection holding tree node values
        std::vector<std::size_t, IndexAllocator> m_parent_index;        // collection holding tree nodes parent index.

    // member types
    public:
        using value_type      = T;
        using key_type        = std::size_t;
        using difference_type = std::ptrdiff_t;
        using pointer         = T*;
        using const_pointer   = const T*;

    // constructor
    public:

        // basic constructor (a tree which only has a root)
        explicit constexpr FlatTree(const T& xi_data) { m_parent_index.emplace_back(0); m_data.emplace_back(xi_data); }
        explicit constexpr FlatTree(T&& xi_data)      { m_parent_index.emplace_back(0); m_data.emplace_back(xi_data); }

        // construct from two iterate-able collections
        template<typename C1, typename C2, typename std::enable_if<!is_vector_v<C1> && !is_vector_v<C2>>::type* = nullptr>
        explicit constexpr FlatTree(const C1& xi_data, const C2& xi_parent_index) {
            static_assert(is_iterate_able_v<C1> && is_iterate_able_v<C2>, "input arguments are not iterate-able collections.");
            static_assert(has_size_method_v<C1> && has_size_method_v<C2>, "input arguments do not have a 'size' method.");

            const std::size_t len{ xi_data.size() };
            assert(len == xi_parent_index.size() && " FlatTree input collections are not of equal size.");

            // allocate
            m_data.reserve(len);
            m_parent_index.reserve(len);

            // copy data
            for (const auto& c : xi_data)         { m_data.emplace_back(c);         }
            for (const auto& c : xi_parent_index) { m_parent_index.emplace_back(c); }

            // check that root is defined properly
            assert(m_parent_index[0] == 0 && " root node must be the first node in tree.");
        }

        template<typename C1, typename C2, typename std::enable_if<!is_vector_v<C1> && !is_vector_v<C2>>::type* = nullptr>
        explicit constexpr FlatTree(C1&& xi_data, C2&& xi_parent_index) {
            static_assert(is_iterate_able_v<C1> && is_iterate_able_v<C2>, "input arguments are not iterate-able collections.");
            static_assert(has_size_method_v<C1> && has_size_method_v<C2>, "input arguments do not have a 'size' method.");

            const std::size_t len{ xi_data.size() };
            assert(len == xi_parent_index.size() && " FlatTree input collections are not of equal size.");

            // allocate
            m_data.reserve(len);
            m_parent_index.reserve(len);

            // move data
            for (auto&& c : xi_data)         { m_data.emplace_back(std::move(c));         }
            for (auto&& c : xi_parent_index) { m_parent_index.emplace_back(std::move(c)); }

            // check that root is defined properly
            assert(m_parent_index[0] == 0 && " root node must be the first node in tree.");
        }
        
        // specialize constructor for vector/list
        explicit constexpr FlatTree(const std::vector<T>& xi_data, const std::vector<std::size_t>& xi_parent_index) : m_data(xi_data), m_parent_index(xi_parent_index) {
            assert(m_data.size() == m_parent_index.size() && " FlatTree input collections are not of equal size.");
        }
        explicit constexpr FlatTree(std::vector<T>&& xi_data, std::vector<std::size_t>&& xi_parent_index) : m_data(std::move(xi_data)), m_parent_index(std::move(xi_parent_index)) {
            assert(m_data.size() == m_parent_index.size() && " FlatTree input collections are not of equal size.");
        }
        
        // copy semantics
        FlatTree(const FlatTree&)            = default;
        FlatTree& operator=(const FlatTree&) = default;

        // move semantics
        FlatTree(FlatTree&&)            noexcept = default;
        FlatTree& operator=(FlatTree&&) noexcept = default;

    // iterators (allow iteration on all tree values)
    public:

        auto begin()   noexcept -> decltype(m_data.begin())   { return m_data.begin();   }
        auto rbegin()  noexcept -> decltype(m_data.rbegin())  { return m_data.rbegin();  }
        auto cbegin()  noexcept -> decltype(m_data.cbegin())  { return m_data.cbegin();  }
        auto crbegin() noexcept -> decltype(m_data.crbegin()) { return m_data.crbegin(); }

        auto end()   noexcept -> decltype(m_data.end())   { return m_data.end();   }
        auto rend()  noexcept -> decltype(m_data.rend())  { return m_data.rend();  }
        auto cend()  noexcept -> decltype(m_data.cend())  { return m_data.cend();  }
        auto crend() noexcept -> decltype(m_data.crend()) { return m_data.crend(); }

    // capacity related functions
    public:

        // return amount of nodes in tree
        inline constexpr std::size_t size() const noexcept { return m_data.size(); }

        // is tree empty? (i.e. - has only the root node...)
        inline constexpr bool empty() const noexcept { return (m_data.size() == 1); }

        // return the maximum possible number of nodes in tree
        inline constexpr std::size_t max_size() const noexcept { return m_data.max_size(); }

        // return the number of nodes that can be held in currently allocated tree
        inline constexpr std::size_t capacity() const noexcept { return m_data.capacity(); }

        // reserve tree storage
        inline constexpr void reserve(const std::size_t xi_new_capacity) {
            m_data.reserve(xi_new_capacity);
            m_parent_index.reserve(xi_new_capacity);
        }

        // reduces memory usage by freeing unused memory
        inline constexpr void shrink_to_fit() {
            m_data.shrink_to_fit();
            m_parent_index.shrink_to_fit();
        }

    // Modifiers
    public:

        // clear the tree content (maintains root node)
        inline constexpr void clear() noexcept {
            const T root{ m_data[0] };

            m_data.clear();
            m_parent_index.clear();

            m_data.emplace_back(root);
            m_parent_index.emplace_back(0);
        }

        // resize the tree to contain {@xi_count} elements
        inline constexpr void resize(const std::size_t xi_count) {
            m_data.resize(xi_count);
            m_parent_index.resize(xi_count);
        }

        // return true if node (given by its index) exists
        inline constexpr bool doesIndexExist(const std::size_t xi_index) noexcept {
            const std::size_t len{ size() };
            return (len < size_for_parallelization)   ? 
                   doesIndexExistSequential(xi_index) : 
                   doesIndexExistParallel(xi_index);
        }

        // return true if node (given by its index) is a leaf
        inline constexpr bool isLeaf(const std::size_t xi_index) noexcept {
            assert((xi_index < m_parent_index.size()) && " node index is invalid");
            return (getNumOfDescendants(xi_index) == 0);
        }

        // given node (given by its index), return the amount of first generation descendants
        inline constexpr std::size_t getNumOfDescendants(const std::size_t xi_parent_index) noexcept {      
            const std::size_t len{ size() };
            return (len < size_for_parallelization)        ? 
                   getNumOfDescendantsSequential(xi_parent_index) :
                   getNumOfDescendantsParallel(xi_parent_index);
        }

        /**
        * \brief given node (given by its index), return a list of all its first generation descendants nodes
        * 
        * @param {size_t,             in}  parent index 
        * @param {collection<size_t>, out} collection holding descendants nodes
        * @param {bool,               out} true if operation was succusfull, false otherwise
        **/
        template<typename C, typename std::enable_if<is_iterate_able_v<C>>::type* = nullptr>
        constexpr bool getDescendants(const std::size_t xi_parent_index, C& xo_descendants) {
            if (!isValid()) return false;
            if (xi_parent_index == 0) return false;

            // get descendants
            bool xo_output{ false };
            for (std::size_t i{1}; i < size(); ++i) {
                if (m_parent_index[i] != xi_parent_index) continue;

                xo_output = true;

                if constexpr (has_pushback_method_v<C>) {
                    xo_descendants.push_back(i);
                } else {
                    xo_descendants.emplace_back(i);
                }
            }

            // output
            return xo_output;
        }

        /**
        * \brief return a collection holding all descendants of a given node (given by its index)
        *
        * @param {std::size_t, in}  parent index
        * @param {T...,        out} iterable collection holding indices of all node descendants
        * @param {bool,        out} true if operation was succusfull, false otherwise
        **/
        template<typename C> constexpr bool getAllDescendants(const std::size_t xi_parent_index, C& xo_descendants) {
            static_assert(is_iterate_able_v<C>, "output argument must be an iterate-able collection");
            return (xi_parent_index != 0)                                        ? 
                   getAllDescendantsNotFromRoot(xi_parent_index, xo_descendants) : 
                   getAllDescendantsFromRoot(xo_descendants);
        }

        /**
        * \brief given a node (by its index), return its parent index
        * 
        * @param {std::size_t, in} descendant index
        * @param {std::size_t, in} parent index
        **/
        inline constexpr std::size_t getParentIndex(const std::size_t xi_index) {
            assert(isValid() && " tree structure is invalid");
            assert(xi_index < m_parent_index.size() && " node index is invalid");
            return (xi_index > 0) ? m_parent_index[xi_index] : 0;
        }

        /**
        * \brief add a variadic amount of node to a given parent
        * 
        * @param {size_T, in} parent index
        * @param {T...,   in} iterable collection of nodes to be inserted
        * @param {bool,  out} true if operation is succesfull 
        **/
        inline constexpr bool insert(const std::size_t xi_parent_id, T&& xi_node) {
            // parent exists?
            if ((xi_parent_id >= m_parent_index.size()) || !isValid()) return false;

            // insert node
            m_data.emplace_back(std::move(xi_node));
            m_parent_index.emplace_back(xi_parent_id);

            // output
            return true;
        }
        template<typename C, typename std::enable_if<is_iterate_able_v<C>>::type* = nullptr>
        inline constexpr bool insert(const std::size_t xi_parent_id, C&& xi_nodes) {
            static_assert(is_iterate_able_v<C>, "input arguments are not iterate-able collections.");

            // parent exists?
            if ((xi_parent_id >= m_parent_index.size()) || !isValid()) return false;

            for (auto&& node : xi_nodes) {
                m_data.emplace_back(std::move(node));
                m_parent_index.emplace_back(xi_parent_id);
            }

            // output
            return true;
        }

        /**
        * \brief remove a node (given by its index) and all its descendants
        * 
        * @param {size_t, in}  node index
        * @param {bool,   out} true if operarion was sucessfull, false otherwise
        **/
        constexpr bool remove(const std::size_t xi_parent_id) {
            if ((xi_parent_id >= m_parent_index.size()) || !isValid()) return false;

            // get all descendants
            std::vector<std::size_t> descendants;
            descendants.reserve(size());
            const bool hasKids{ getAllDescendants(xi_parent_id, descendants) };
            if (!hasKids) return false;

            // remove descendants
            for (std::size_t kid : descendants) {
                remove_node(kid);
            }

            // output
            return true;
        }

        /**
        * \brief out-of-order tree traversal from a given node (given by its index) "downwards" using a given execution policy
        * 
        * @param {size_t,   in} index of node from which depth first search will be performed  
        * @param {executer, in} execution policy (std::execution::seq, std::execution::par, std::execution::par_unseq, std::execution::unseq)
        * @param {function, in} operation to be performed on descendants
        **/
        template<class EXECUTER, class FUNC> inline constexpr void Traverse(const std::size_t xi_parent_index, EXECUTER&& xi_exec, FUNC&& xi_func) {
            // get all descendants
            std::vector<std::size_t> descendants;
            descendants.reserve(size());
            const bool hasKids{ getAllDescendants(xi_parent_index, descendants) };
            if (!hasKids) return;
            
            // apply function on descendants
            std::for_each(std::forward<EXECUTER>(xi_exec), descendants.begin(), descendants.end(), [this, f = std::forward<FUNC>(xi_func)](auto& elm) mutable {
                f(m_data[elm]);
            });
        }

    // output tree structure
    public:

        // output tree as a pair of {node value, node parent index}
        inline constexpr void dumpToConsoleSimple() noexcept {
            const std::size_t len{ size() };
            if (len == 0) return;

            std::cout << m_data[0] << " {" << std::to_string(m_parent_index[0]) << "}";
            for (std::size_t i{1}; i < len; ++i) {
                std::cout << ", " << m_data[i] << " {" << std::to_string(m_parent_index[i]) << "}";
            }
        }


        // output tree as a multi-map, i.e. - list of first generation descendants for each parent
        // notice that descendants are not printed in order
        constexpr void dumpToConsoleMultiMap() noexcept {
            const std::size_t len{ size() };
            if (len == 0) return;

            // get unique parent id's
            std::vector<std::size_t> parents{ m_parent_index };         
            std::sort(parents.begin(), parents.end());
            parents.erase(std::unique(parents.begin(), parents.end()), parents.end());

            // iterate over each parent
            for (std::size_t it : parents) {
                std::cout << m_data[it] << ": ";

                // get descendants
                std::vector<std::size_t> kids;
                const bool succeed{ getDescendants(it, kids) };
                if (!succeed) {
                    std::cout << "\n";
                    continue;
                }

                // print descendants
                std::cout << m_data[kids[0]];
                for (std::vector<std::size_t>::iterator k{ kids.begin() + 1 }; k != kids.end(); ++k) {
                    std::cout << "," << m_data[*k];
                }
                std::cout << "\n";
            }
        }

    // operator overload
    public:

        // insert a node under a given parent (given by its index)
        // syntax is: 
        //  tree << {parent index, node value}
        //  tree << {parent index, {node value 1, node value 2, ...}}
        friend FlatTree& operator << (FlatTree& tree, std::pair<std::size_t, T>&& xi_node) {
            const bool succeed{ tree.insert(std::forward<std::size_t>(xi_node.first), std::forward<T>(xi_node.second)) };
            assert(succeed && " failed to insert a node to tree.");
            return tree;
        }

        template<typename C, typename U, typename std::enable_if<is_iterate_able_v<C> && !std::is_same_v<T, C> && std::is_integral_v<U>>::type* = nullptr>
        friend FlatTree& operator << (FlatTree& tree, std::pair<U, C>&& xi_node) {
            const bool succeed{ tree.insert(static_cast<std::size_t>(xi_node.first), std::forward<C>(xi_node.second)) };
            assert(succeed && " failed to insert a node to tree.");
            return tree;
        }

        // get/change (but not insert!) node at a given index
        const T  operator[](const std::size_t xi_index) const { assert(isValid() && (xi_index < size())); return m_data[xi_index]; }
              T& operator[](const std::size_t xi_index)       { assert(isValid() && (xi_index < size())); return m_data[xi_index]; }

        // delete a list of nodes (given by their indices) and all their descendants
        // syntax is: 
        //  tree >> index n
        //  tree >> {index 1, index 2, ...}
        friend FlatTree& operator >> (FlatTree& tree, const std::size_t xi_node) {
          const bool succeed{ tree.remove(xi_node) };
          assert(succeed && " failed to remove a node from tree.");
          return tree;
        }
        template<typename C, typename std::enable_if<is_iterate_able_v<C> && !std::is_same_v<T, C>>::type* = nullptr>
        friend FlatTree& operator >> (FlatTree& tree, C&& xi_nodes) {
            bool succeed{ true };
            
            for (auto&& i : xi_nodes) {
                const bool succeedi{ tree.remove(i) };
                succeed &= succeedi;
            }

            assert(succeed && " failed to remove nodes from tree.");
            return tree;
        }

    // internal methods
    private:

        // validate tree structure
        inline constexpr bool isValid() const noexcept {
            // check that root is defined properly
            return ((m_data.size() == m_parent_index.size()) &&     // tree internal collections match in their size
                    (m_parent_index[0] == 0));                      // tree node is located in the correct place
        }

        // fast removal of a node from tree
        inline constexpr void remove_node(const std::size_t xi_index) noexcept {
            // can not remove the root
            if (xi_index == 0) return;

            // remove node value
            m_data[xi_index] = m_data[m_data.size() - 1];
            m_data.pop_back();

            // remove its parent index
            m_parent_index[xi_index] = m_parent_index[m_parent_index.size() - 1];
            m_parent_index.pop_back();

            assert(isValid() && " something went wrong when trying to remove a node from tree.");
        }

        // sequential index searching
        inline constexpr bool doesIndexExistSequential(const std::size_t xi_index) noexcept {
            const auto iend = m_parent_index.end();
            return (std::find(std::execution::seq, m_parent_index.begin(), m_parent_index.end(), xi_index) == iend) ? false : true;
        }

        // parallel index searching
        inline constexpr bool doesIndexExistParallel(const std::size_t xi_index) noexcept {
            const auto iend = m_parent_index.end();
            return (std::find(std::execution::par, m_parent_index.begin(), m_parent_index.end(), xi_index) == iend) ? false : true;
        }

        // sequential counting of first generation descendants
        inline constexpr std::size_t getNumOfDescendantsSequential(const std::size_t xi_parent_index) noexcept {
            return std::count(std::execution::seq, m_parent_index.begin(), m_parent_index.end(), xi_parent_index);
        }

        // parallel counting of first generation descendants
        inline constexpr std::size_t getNumOfDescendantsParallel(const std::size_t xi_parent_index) noexcept {
            return std::count(std::execution::par_unseq, m_parent_index.begin(), m_parent_index.end(), xi_parent_index);
        }

        // get all descendants from a given node
        template<typename C> constexpr bool getAllDescendantsNotFromRoot(const std::size_t xi_parent_index, C& xo_descendants) {
            // first generation descendants
            bool xo_output{ getDescendants(xi_parent_index, xo_descendants) };
            if (!xo_output) return false;

            // other generations...
            for (const auto i : xo_descendants) {
                const bool out{ getAllDescendants(i, xo_descendants) };
            }

            // output
            return true;
        }

        // get all descendants from root
        template<typename C> constexpr bool getAllDescendantsFromRoot(C& xo_descendants) {
            bool xo_output{ false };
            for (std::size_t i{ 1 }; i < size(); ++i) {
                if constexpr (has_pushback_method_v<C>) {
                    xo_descendants.push_back(i);
                    xo_output = true;
                } else {
                    xo_descendants.emplace_back(i);
                    xo_output = true;
                }
            }

            return xo_output;
        }
};

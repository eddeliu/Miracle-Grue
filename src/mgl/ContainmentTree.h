/* 
 * File:   ContainmentTree.h
 * Author: FILIPP
 *
 * Created on October 29, 2012, 12:16 PM
 */

#ifndef CONTAINMENTTREE_H
#define	CONTAINMENTTREE_H

#include "loop_path.h"
#include <list>

namespace mgl {

class ContainmentTree {
public:
    /**
     Construct a root tree
     */
    ContainmentTree();
    /**
     Construct a normal tree
     @param loop a valid non-empty loop
     */
    ContainmentTree(const Loop& loop);
    /**
     @brief Test if this tree contains another hierarchy
     @param other does this contain other?
     @return whether this contains other
     
     Roots return true for all normal trees, false for other roots
     Normal trees use winding test for other normal trees, false for roots
     */
    bool contains(const ContainmentTree& other) const;
    /**
     @brief Test if this tree contains a this point
     @param point does this contain point?
     @return whether this contains @a point
     
     Normal trees use the winding test, invalid trees return true always.
     */
    bool contains(const Point2Type& point) const;
    /**
     @brief Test if this is a valid tree?
     
     Normal trees have a loop and other stuff. Invalid trees are roots
     @return true for normal tree, false for roots
     */
    bool isValid() const;
    /**
     @brief Return reference to deepest child that contains point, 
     or to this tree if there are no such children.
     @param point the point to test for containment
     @return reference to deepest tree that contains @a point
     */
    ContainmentTree& select(const Point2Type& point);
    /**
     @brief Return reference to deepest child that contains point, 
     or to this tree if there are no such children.
     @param point the point to test for containment
     @return reference to deepest tree that contains @a point
     */
    const ContainmentTree& select(const Point2Type& point) const;
    /**
     @brief Swap the contents of this tree with other.
     @param other The tree with which to swap contents
     
     Swap the contents of this tree with other. This involves swapping 
     each data member of this tree with the corresponding member in other. 
     Where possible, the fast, constant time implementation of swap is 
     used, so this function should run in constant time.
     */
    void swap(ContainmentTree& other);
private:
    typedef std::list<ContainmentTree> containment_list;
    
    Loop m_loop;
    containment_list m_children;
};

}

namespace std {

/**
 @brief This is the ContainmentTree specialization of std::swap()
 */
void swap(mgl::ContainmentTree& lhs, mgl::ContainmentTree& rhs);

}

#endif	/* CONTAINMENTTREE_H */

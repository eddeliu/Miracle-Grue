#include <list>
#include <vector>

#include "pather_optimizer_fastgraph.h"

namespace mgl {

void pather_optimizer_fastgraph::addPath(const OpenPath& path, 
        const PathLabel& label) {
    node_index last = -1;
    //bucket& currentBucket = pickBucket(*path.fromStart());
    for(OpenPath::const_iterator iter = path.fromStart(); 
            iter != path.end(); 
            ++iter) {
        OpenPath::const_iterator next = iter;
        ++next;
        if(iter == path.fromStart()) {
            last = graph.createNode(NodeData(*iter, 
                    label.myValue, true)).getIndex();
        }
        if(next != path.end()) {
            OpenPath::const_iterator future = next;
            ++future;
            node& curNode = graph.createNode(NodeData(*next, 
                    label.myValue, future==path.end()));
            node& lastNode = graph[last];
            libthing::LineSegment2 connection( 
                    curNode.data().getPosition(), 
                    lastNode.data().getPosition());
            PointType normal;
            try {
                normal = (connection.b - connection.a).unit();
            } catch (const libthing::Exception& le) {}
            Scalar distance = connection.length();
            Cost frontCost(label, distance, normal);
            Cost backCost(label, distance, normal * -1.0);
            curNode.connect(lastNode, backCost);
            lastNode.connect(curNode, frontCost);
            last = curNode.getIndex();
        }
    }
//    std::cout << "Path Size: " << path.size() << std::endl;
//    std::cout << "Graph Nodes: " << graph.count() << std::endl;
}
void pather_optimizer_fastgraph::addPath(const Loop& loop, 
        const PathLabel& label) {
    node_index last = -1;
    node_index first = -1;
    //bucket& currentBucket = pickBucket(*loop.clockwise());
    for(Loop::const_finite_cw_iterator iter = loop.clockwiseFinite(); 
            iter != loop.clockwiseEnd(); 
            ++iter) {
        Loop::const_finite_cw_iterator next = iter;
        ++next;
        if(iter == loop.clockwiseFinite()) {
            last = graph.createNode(NodeData(*iter, 
                    label.myValue, true)).getIndex();
            first = last;
        }
        if(next != loop.clockwiseEnd()) {
            NodeData curNodeData(*next, label.myValue, true);
            node& curNode = graph.createNode(curNodeData);
            node& lastNode = graph[last];
            libthing::LineSegment2 connection( 
                    curNode.data().getPosition(), 
                    lastNode.data().getPosition());
            PointType normal;
            try {
                normal = (connection.b - connection.a).unit();
            } catch (const libthing::Exception& le) {}
            Scalar distance = connection.length();
            Cost frontCost(label, distance, normal);
            Cost backCost(label, distance, normal * -1.0);
            curNode.connect(lastNode, backCost);
            lastNode.connect(curNode, frontCost);
            last = curNode.getIndex();
        }
    }
    node& lastNode = graph[last];
    node& curNode = graph[first];
    libthing::LineSegment2 connection( 
            curNode.data().getPosition(), 
            lastNode.data().getPosition());
    PointType normal;
    try {
        normal = (connection.b - connection.a).unit();
    } catch (const libthing::Exception& le) {}
    Scalar distance = connection.length();
    Cost frontCost(label, distance, normal);
    Cost backCost(label, distance, normal * -1.0);
    curNode.connect(lastNode, backCost);
    lastNode.connect(curNode, frontCost);
}
void pather_optimizer_fastgraph::addBoundary(const OpenPath& path) {
    for(OpenPath::const_iterator iter = path.fromStart(); 
            iter != path.end(); 
            ++iter) {
        OpenPath::const_iterator next = iter;
        ++next;
        if(next != path.end()) {
            boundaries.insert(libthing::LineSegment2(*iter, *next));
        }
    }
}
void pather_optimizer_fastgraph::addBoundary(const Loop& loop) {
    buckets.push_back(bucket());
    for(Loop::const_finite_cw_iterator iter = loop.clockwiseFinite(); 
            iter != loop.clockwiseEnd(); 
            ++iter) {
        libthing::LineSegment2 segment = loop.segmentAfterPoint(iter);
        boundaryLimits.expandTo(segment.a);
        boundaryLimits.expandTo(segment.b);
        boundaries.insert(segment);
        buckets.back().first.insert(segment);
    }
    
}
void pather_optimizer_fastgraph::clearBoundaries() {
    boundaries = boundary_container();
    buckets.clear();
    boundaryLimits.reset();
}
void pather_optimizer_fastgraph::clearPaths() {
    graph.clear();
}
pather_optimizer_fastgraph::entry_iterator& 
        pather_optimizer_fastgraph::entry_iterator::operator ++() {
    do { ++m_base; } while(m_base != m_end && !m_base->data().isEntry());
    return *this;
}
pather_optimizer_fastgraph::entry_iterator
        pather_optimizer_fastgraph::entry_iterator::operator ++(int) {
    entry_iterator copy = *this;
    ++*this;
    return copy;
}
pather_optimizer_fastgraph::node&
        pather_optimizer_fastgraph::entry_iterator::operator *() {
    return *m_base;
}
bool pather_optimizer_fastgraph::entry_iterator::operator ==(
        const entry_iterator& other) const {
    return m_base == other.m_base;
}
pather_optimizer_fastgraph::entry_iterator 
        pather_optimizer_fastgraph::entryBegin() {
    entry_iterator ret(graph.begin(), graph.end());
    if(!ret->data().isEntry())
        ++ret;
    return ret;
}
pather_optimizer_fastgraph::entry_iterator
        pather_optimizer_fastgraph::entryEnd() {
    return entry_iterator(graph.end(), graph.end());
}
Scalar pather_optimizer_fastgraph::splitPaths(multipath_type& destionation, 
        const LabeledOpenPaths& source) {
    PointType lastPoint(std::numeric_limits<Scalar>::min(), 
            std::numeric_limits<Scalar>::min());
    Scalar ret = 0;
    for(LabeledOpenPaths::const_iterator iter = source.begin(); 
            iter != source.end(); 
            ++iter) {
        if(iter->myPath.size() < 2) 
            continue;
        if(*(iter->myPath.fromStart()) != lastPoint) {
            if(!destionation.empty()) {
                ret += (*(iter->myPath.fromStart()) - lastPoint).magnitude();
            }
            destionation.push_back(LabeledOpenPaths());
        }
        destionation.back().push_back(*iter);
        lastPoint = *destionation.back().back().myPath.fromEnd();
    }
    return ret;
}
size_t pather_optimizer_fastgraph::countIntersections(libthing::LineSegment2& line, 
        boundary_container& boundContainer) {
    typedef std::vector<libthing::LineSegment2> result_type;
    result_type result;
    size_t count = 0;
    boundContainer.search(result, LineSegmentFilter(line));
    for(result_type::const_iterator iter = result.begin(); 
            iter != result.end(); 
            ++iter) {
        if(iter->intersects(line))
            ++count;
    }
    return count;
}

pather_optimizer_fastgraph::bucket& 
        pather_optimizer_fastgraph::pickBucket(PointType point) {
    libthing::LineSegment2 testLine(point, 
            boundaryLimits.bottom_left() - PointType(20,20));
    for(bucket_list::iterator iter = buckets.begin(); 
            iter != buckets.end(); 
            ++iter) {
        size_t intersections = countIntersections(testLine, 
                iter->first);
        if(intersections & 1) { //if odd, then inside
            return *iter;
        }
    }
    std::cout << "Point Location: " << point << std::endl;
    std::cout << "Bounds: " << boundaryLimits.bottom_left() <<
            boundaryLimits.top_right() << std::endl;
    throw GraphException("Thing did not fall into any bucket!");
}




}


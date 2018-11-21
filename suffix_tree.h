#pragma once

#include "print.h"

#include <map>
#include <vector>
#include <utility>
#include <unordered_set>
#include <deque>
#include <memory>
#include <string>
#include <iostream>

class CEdge;
class CVirtualEdge;
class CNode;
class CPoint;
class CSuffixTree;

typedef ssize_t BigInt;

// Point in a suffix tree, used in splitting
class CPoint {
public:
    // Owning tree
    CSuffixTree *tree;
    BigInt relativeIndex;
    // Current edge for processing
    CEdge *edge;

    // It either increases current point position or splits an edge/adds new edge
    // returns 1 if increase succeeded
    //         0 if split/add happened
    // If split happens the point doesn't change.
    BigInt increaseOrSplit(char letter, CNode ** return_node);
    CPoint(CSuffixTree*);
};

// Prefixed edge keeps a prefix string for this edge
// that is a concatenation of all edges from root to this edge
// Also keeps sum of strings on those edges, whish is equal to prefix string length
// Also keeps location of the first non-letter symbol on this edge, if any.
// And a pointer to the edge itself
//
// Used only for getting top frequent substrings by number of occurrences
struct CPrefixedEdge {
    // Number of letters from beginning of the tree root
     BigInt prefixLength = 0;
     // Accumulated strings on edges from tree root
     std::string prefixString;
     // First occurrence of non letter on this edge, -1 for never
     BigInt firstNonLetterOffset = -1;
     // The edge we are talking about
     CEdge *edgePtr;

     CPrefixedEdge(BigInt prefixLength_, std::string s, BigInt firstNonLetterOffset_, CEdge *e)
        : prefixLength(prefixLength_), prefixString(s), firstNonLetterOffset(firstNonLetterOffset_), edgePtr(e)
     {}
     CPrefixedEdge() {}
};

// Data structure used in computation of top frequently occurring substrings
// number of occurrences => vector of prefixed edges with this occurrence
typedef std::map<BigInt, std::vector<CPrefixedEdge>, std::greater<int> > CFrequencyToEdgeMap;

//---------------------------------------------------
// The Ukkonnen's suffix tree
class CSuffixTree {
public:
    // Value for free end of the edge
    static const BigInt FREE = -10;

    //-------
    // Suffix tree data

    // Auxiliry point with virtual edges for every symbol in the dicationary
    CNode *preRoot;
    // Root of the suffix tree
    CNode *root;
    // Current processing point
    CPoint activePoint;
    // Previous processing point
    CPoint previousPoint;
    // The string under consideration
    std::string sourceString;
    BigInt currentIndex;

    // Simple ad-hoc way to ensure clearing of memory after all pointers used here
    std::deque<std::unique_ptr<CEdge>> edgesKeeper;
    std::deque<std::unique_ptr<CNode>> nodesKeeper;
    std::deque<std::unique_ptr<CVirtualEdge>> virtualEdgesKeeper;
    CEdge* newCEdge();
    CNode* newCNode();

    //---------
    // Suffix tree related functions:

    // Finds a place in a tree to which leads a current string without the first letter
    CPoint getNextPoint(CPoint point);
    // Print tree into console in human readable form
    void print(CNode*, std::string);
    // Initilaize suffix tree data to prepare for a buildTree() call
    CSuffixTree(std::string);

    // Actually creates a suffix tree out of data prepared in CSuffixTree(...)
    void buildTree();

    //-----------
    //Code below is needed only for counting of substring frequences:

    // Get top N substrings by occurrence frequency
    CFrequencyInfo getTopSuitableSubstrings(const size_t takeTopN, ssize_t minimalLength);

    // Get number of substrings longer than given minimalLength
    BigInt getNumbetOfSubstringsLongerThan(BigInt minimalLength, CNode *node = nullptr, BigInt depth = 0);

    // Debug print function
    void printCurrentTopEdges(CFrequencyToEdgeMap &);

private:
    // Auxiliary function for counting frequences of substrings
    // It fiils in the number of occurrences for each edge
    // Info is stored in CEdge::occurrenceNumber of every edge
    BigInt countOccurrences(CNode *node);

    // Auxiliary function for counting frequences of substrings
    // Initializes CFrequencyToEdgeMap for use in getTopSuitableSubstrings(...)
    void initializeFreqToEdgeMap(CNode *node, CFrequencyToEdgeMap &freqToEdgeMap, BigInt minimalLength, BigInt depth = 0, std::string prefix = "");
};

// Auxiliary classes for CSuffixTree

// Node for a suffix tree, holds charecter to edge map
class CNode {
public:
    CSuffixTree *tree;
    std::map<char, CEdge*> edges;
    CEdge *inEdge;
    CNode *suffixLink;

    BigInt number;

    CEdge* edgeFromLetter(char letter) {
        CEdge * edge_pointer = NULL;
        if (edges.count( letter ) > 0) {
            edge_pointer = edges[letter];
        }
        return edge_pointer;
    }

    CNode (CSuffixTree *tree_)
        : tree(tree_)
        , inEdge(nullptr)
        , suffixLink(nullptr)
    {
    }

    BigInt getSuffixLinkNumber() {
        BigInt retval = -1;
        if (suffixLink != nullptr)
            retval = suffixLink->number;
        return retval;
    }
};

// Edge for a suffix tree, connects nodes in suffix tree
class CEdge {
public:
    CNode *beginNode
         ,*endNode;

    BigInt beginIndex;
    BigInt endIndex;

    // Used only for substring frequency of occurrences counting
    BigInt occurrenceNumber = 0;

    // Parent tree
    CSuffixTree *tree;

    BigInt length() {
        BigInt length;
        length = 1 + ( ( endIndex != CSuffixTree::FREE ) ? endIndex : tree->currentIndex ) - beginIndex;
        return length;
    }

    virtual char letterFromRelativeIndex( BigInt relativeIndex ) {
        return tree->sourceString[ beginIndex + relativeIndex ];
    }

    CEdge(CSuffixTree *tree_)
        : beginNode(nullptr), endNode(nullptr), tree(tree_)
    {
    }

    CEdge(CSuffixTree *tree_, CNode *beginNode_, CNode *endNode_)
        : beginNode(beginNode_), endNode(endNode_), tree(tree_)
    {
    }

    BigInt getRealEndIndex() {
        return ( endIndex == CSuffixTree::FREE ) ? tree->currentIndex : endIndex;
    }
};

// Edge to connect pre_root and root in suffix tree
// For every charecter in vocabulary there is a virtual edge from pre_root to root in suffix tree
class CVirtualEdge : public CEdge {
public:
    char character;

    virtual char letterFromRelativeIndex(BigInt relativeIndex) {
        return tree->sourceString[ beginIndex + relativeIndex ];
    }

    CVirtualEdge(CSuffixTree *tree_, char character_, CNode *beginNode, CNode *endNode)
        : CEdge (tree_, beginNode, endNode)
        , character(character_)
    {
        beginIndex = -1;
        endIndex = -1;
    }
};


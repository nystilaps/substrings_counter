#include "suffix_tree.h"

#include <cctype>

using std::map;
using std::vector;
using std::pair;
using std::cout;
using std::endl;
using std::string;

//---------------------------------------------------
//-----------  CPoint Implementation  ---------------
//---------------------------------------------------

// It either increases current point position or splits an edge/adds new edge
// returns 1 if increase succeeded
//         0 if split/add happened
// If split happens the point doesn't change.
BigInt CPoint::increaseOrSplit(char letter, CNode **returnNode) {
    *returnNode = nullptr;

    CEdge *newEdge = edge;

    if (edge->beginIndex + relativeIndex < edge->getRealEndIndex()) {
        if(letter == edge->letterFromRelativeIndex(relativeIndex + 1) ) { // Just go further the current edge
            ++relativeIndex;
            *returnNode = edge->beginNode;
            return 1;
        } else { // Split the edge with a new node;
            CNode *newNode = tree->newCNode();
            CEdge *edge_1 = tree->newCEdge();
            CEdge *edge_2 = tree->newCEdge();

            newNode->inEdge = this->edge;
            edge_1->endNode = edge->endNode;
            edge_1->beginNode = newNode;
            edge_2->beginNode = newNode;
            edge->endNode = newNode;

            edge_1->beginIndex = edge->beginIndex + relativeIndex + 1;
            edge_1->endIndex = edge->endIndex;
            edge->endIndex = edge->beginIndex + relativeIndex;
            edge_2->endIndex = CSuffixTree::FREE;
            edge_2->beginIndex = tree->currentIndex;

            newNode->edges[ edge_1->letterFromRelativeIndex(0) ] = edge_1;
            newNode->edges[ edge_2->letterFromRelativeIndex(0) ] = edge_2;

            *returnNode = newNode;
            return 0;
        }
    } else {
        newEdge = edge->endNode->edgeFromLetter(letter);
        if (newEdge == nullptr) { // Create new child edge
            CEdge *edge_1 = tree->newCEdge();
            edge_1->beginIndex = tree->currentIndex;
            edge_1->endIndex = CSuffixTree::FREE;
            edge_1->beginNode = edge->endNode;
            edge->endNode->edges[letter] = edge_1;

            *returnNode = edge_1->beginNode;
            return 0;
        } else { // No new children, just follow the right edge
            *returnNode = edge->endNode;

            edge = newEdge;
            relativeIndex = 0;

            return 1;
        }
    }
}

CPoint::CPoint(CSuffixTree *tree_) : tree(tree_) {}

//---------------------------------------------------
//---------  CSuffixTree Implementation  ------------
//---------------------------------------------------

// Finds a place in a tree to which leads a current string without the first letter
CPoint CSuffixTree::getNextPoint(CPoint point) {
    if ( (point.edge->beginIndex == -1) && (point.relativeIndex <= 1) )
        return point;

    CPoint nextPoint(this);
    BigInt distanceGone = 0;

    CNode *node = point.edge->beginNode->suffixLink;
    CEdge *edge = node->edgeFromLetter(point.edge->letterFromRelativeIndex(0));

    while (distanceGone + edge->length() <= point.relativeIndex) {
        node = edge->endNode;
        distanceGone += edge->length();
        edge = node->edgeFromLetter(point.edge->letterFromRelativeIndex(distanceGone));
    }
    nextPoint.edge = edge;
    nextPoint.relativeIndex = point.relativeIndex - distanceGone;

    return nextPoint;
}

// Initilaize suffix tree data to prepare for a buildTree() call
CSuffixTree::CSuffixTree(string sourceString_)
    : activePoint(this), previousPoint(this)
{
    sourceString = std::move(sourceString_);

    root = newCNode();
    preRoot = newCNode();

    root->suffixLink = preRoot;
    preRoot->suffixLink = preRoot;

    // Vocabulary this tree will accept
    string letters("ABCDEFGHIJKLMNOPQRSTUVWXYZ\t\n\r \"',.[]{}()-*&^%$#@!1?;:234567890_abcdefghijklmnopqrstuvwxyz");
    {
        const std::unordered_set<char> dictionary(letters.begin(), letters.end());
        // Check for symbols not in dictionary
        for (const auto c: sourceString) {
            if (dictionary.count(c) == 0) {
                std::string errorString = "String contains a symbol '";
                errorString += c;
                errorString += "' which is not in dictionary!";

                throw std::runtime_error(errorString);
            }
            if (c == '$') {
                throw std::runtime_error("There must be no symbol '$' in a string! It is a special symbol.");
            }
        }
    }

    sourceString += '$';

    // Adding a virtual edge from pre-root to root for every charachter in vocabulary
    for (size_t i = 0 ; i < letters.size() ; ++i) {
        virtualEdgesKeeper.emplace_back(new CVirtualEdge(this, letters[i], preRoot, root));
        preRoot->edges[ letters[i] ] = virtualEdgesKeeper.back().get();
    }

    activePoint.edge = preRoot->edges['a'];
    activePoint.relativeIndex = 0;
}

// Does tha ctual work of building the suffix tree
void CSuffixTree::buildTree() {
    CNode *newNode = nullptr
         ,*oldNode = nullptr;

    char current_letter;

    for (currentIndex = 0; currentIndex < (ssize_t)sourceString.size(); ++currentIndex) {
        current_letter = sourceString[currentIndex];

        while (!activePoint.increaseOrSplit(current_letter, &newNode)) {
            if (oldNode != nullptr) {
                if (oldNode->suffixLink == nullptr) {
                    oldNode->suffixLink = newNode;
                }
            }
            oldNode = newNode;
            previousPoint = activePoint;
            activePoint = getNextPoint(activePoint);

            // The following condition happens when we have reached an edge second time at the same place
            // It happens only on virtual edges and only when it is time to stop
            if( previousPoint.edge == activePoint.edge && previousPoint.relativeIndex == activePoint.relativeIndex )
                break;
        }

        if (oldNode != nullptr) {
            if( oldNode->suffixLink == nullptr ) {
                oldNode->suffixLink = newNode;
            }
        }
    }

    // After a for loop currentIndex = sourceString.size() but we need a little less
    // because we have added '$' to the end of the string:
    currentIndex = sourceString.size() - 1;

    //std::cout << "Calculating occurrences...";
    // Prepare occurrentNumber on edges
    countOccurrences(root);
    //std::cout << "done." << std::endl;
}

// Print tree horizontally into console in hierarchically offsetted way
void CSuffixTree::print(CNode *node, string offset) {
    cout << offset << "*"
         << endl;

    for (auto edgesIterator = node->edges.begin(); edgesIterator != node->edges.end(); ++edgesIterator) {
        cout
            << " "
            <<  offset
            <<  sourceString.substr(
                      edgesIterator->second->beginIndex
                    , edgesIterator->second->getRealEndIndex() - edgesIterator->second->beginIndex + 1
                )
            << " ("
            << edgesIterator->second->beginIndex
            << ", "
            << edgesIterator->second->getRealEndIndex() //endIndex
            << ") '"
            <<  edgesIterator->first
            << "' "
            << "occurrences=" << edgesIterator->second->occurrenceNumber
            << endl;

        if( edgesIterator->second->endNode != nullptr )
            print( edgesIterator->second->endNode , offset + " " );
    }
}

// Computes how many times specific edge happened in the text
BigInt CSuffixTree::countOccurrences(CNode *node) {
    BigInt count = 0;
    for (auto edgesIterator = node->edges.begin() ; edgesIterator != node->edges.end() ; ++edgesIterator) {
        if( edgesIterator->second->endNode != nullptr ) {
            BigInt occurrenceNumber = countOccurrences( edgesIterator->second->endNode );
            edgesIterator->second->occurrenceNumber = occurrenceNumber;
            count += occurrenceNumber;
        } else {
            edgesIterator->second->occurrenceNumber = 1;
            count += 1;
        }
    }
    return count;
}

// Prepares CFrequencyToEdgeMap for use in getTopSuitableSubstrings(...)
void CSuffixTree::initializeFreqToEdgeMap(CNode *node, CFrequencyToEdgeMap &freqToEdgeMap, BigInt minimalLength, BigInt depth, string prefix) {



    // Get first edges with enough letters in the beginning to meet the requirement
    // of substing length more or equal to minimalLength.
    // Length is counted as a sum or all edges lengths from the tree root.
    for (auto edgesIterator = node->edges.begin() ; edgesIterator != node->edges.end() ; ++edgesIterator) {
        BigInt edgeLength = 1 + edgesIterator->second->getRealEndIndex() - edgesIterator->second->beginIndex;

        string edgeString = sourceString.substr(
            edgesIterator->second->beginIndex
            ,edgeLength
        );

        BigInt firstNonLetterOffset = -1; // initially end of current substring
        for ( BigInt index = 0 ; index < (ssize_t)edgeString.length() ; ++index ) {
            // Find first non-letter symbol on this edge
            if ( !isalpha(static_cast<unsigned char>(edgeString[index])) ) {
                firstNonLetterOffset = index;
                break;
            }
        }

        // No spaces on the edge
        if (firstNonLetterOffset == -1) {
            // Add either this edge or its children
            if ( (depth + edgeLength) >= minimalLength ) {
                freqToEdgeMap[ edgesIterator->second->occurrenceNumber ].emplace_back( depth, prefix, firstNonLetterOffset, edgesIterator->second );
            } else {
                if ( edgesIterator->second->endNode != nullptr ) {
                    initializeFreqToEdgeMap( edgesIterator->second->endNode, freqToEdgeMap, minimalLength, (depth + edgeLength), (prefix + edgeString) );
                }
            }
        } else {
            // Have a space in the middle of the string
            if ( (depth + firstNonLetterOffset) >= minimalLength ) {
                freqToEdgeMap[ edgesIterator->second->occurrenceNumber ].emplace_back( depth, prefix, firstNonLetterOffset, edgesIterator->second );
            }
        }
    }
}

// Auxiliry function to initialis
BigInt CSuffixTree::getNumbetOfSubstringsLongerThan(BigInt minimalLength, CNode *node, BigInt depth) {
    if (node == nullptr)
        node = root;

    BigInt count = 0;
    for (auto edgesIterator = node->edges.begin() ; edgesIterator != node->edges.end() ; ++edgesIterator) {
        BigInt edgeLength = 1 + edgesIterator->second->getRealEndIndex() - edgesIterator->second->beginIndex;

        // Temporary string for convenience
        string edgeString = sourceString.substr(edgesIterator->second->beginIndex, edgeLength);

        BigInt firstNonLetterOffset = -1; // initially end of current substring
        for (BigInt index = 0 ; index < (ssize_t)edgeString.length() ; ++index) {

            if ( !isalpha(static_cast<unsigned char>(edgeString[index])) ) {
                firstNonLetterOffset = index;
                break;
            }

            // Calculate substrings longer or equal than minimum and before nonletter symbol appears
            if ( (depth + index + 1) >= minimalLength ) {
                // For every letter count as many times as this edge has occurred in the text
                long long times = edgesIterator->second->occurrenceNumber;
                count += times;
            }
        }

        // No spaces on the edge
        if (firstNonLetterOffset == -1) {
            // If edge has a nonempty end node
            if ( edgesIterator->second->endNode != nullptr ) {
                long long nestedSubstringsNumber = getNumbetOfSubstringsLongerThan(minimalLength, edgesIterator->second->endNode, (depth + edgeLength) );
                count += nestedSubstringsNumber;
            }
        }
    }
    return count;
}

// Get tpp N substrings by occurrence frequency with minimal length more or equal to minimalLength
CFrequencyInfo CSuffixTree::getTopSuitableSubstrings(const size_t takeTopN, ssize_t minimalLength) {

    CFrequencyToEdgeMap freqToEdgeMap;
    initializeFreqToEdgeMap(root, freqToEdgeMap, minimalLength, 0);

    CFrequencyInfo topN;

    // Number of all substrings of appropriate length in the text
    double numberOfLongSubstrings = getNumbetOfSubstringsLongerThan(minimalLength);

    // While we have candidate prefixed edges to process
    while (!freqToEdgeMap.empty()) {
        // Take top frequency bucket
        auto freqTopIt = freqToEdgeMap.begin();

        // Iterate the top bucket
        while (!freqTopIt->second.empty()) {
            // Get current edge from vector
            CPrefixedEdge prefixedEdge = freqTopIt->second.back();
            freqTopIt->second.pop_back();

            // String the edge represents
            string edgeSubstring = sourceString.substr(
                        prefixedEdge.edgePtr->beginIndex
                        ,prefixedEdge.edgePtr->getRealEndIndex() - prefixedEdge.edgePtr->beginIndex + 1);

            // Edge has non-letter charackters
            if (prefixedEdge.firstNonLetterOffset != -1) {
                // Take only substring before first non-letter symbol
                edgeSubstring = edgeSubstring.substr(0, prefixedEdge.firstNonLetterOffset);
            }

            // Iterate over symbols on this edge from closest to root
            const ssize_t beginIndex = std::max((BigInt)1, minimalLength - prefixedEdge.prefixLength);
            for (size_t cutOff = beginIndex; cutOff <= edgeSubstring.size(); ++cutOff) {
                // Output substring and its frequency into vector topN
                topN.emplace_back(prefixedEdge.prefixString + edgeSubstring.substr(0, cutOff), 100 * freqTopIt->first / numberOfLongSubstrings);

                // If we have enough data in topN, quit
                if ( (takeTopN > 0) && (topN.size() >= takeTopN) ) {
                    return topN;
                }
            }

            if (prefixedEdge.firstNonLetterOffset == -1) {
                // Fill in helper data about this edge before processing child edges
                auto prefix = prefixedEdge.prefixString + edgeSubstring;
                auto node = prefixedEdge.edgePtr->endNode;
                BigInt depth = prefixedEdge.prefixLength;

                // If edge has a valid end node
                if (node != nullptr) {
                    // For every child edge
                    for (auto edgesIterator = node->edges.begin(); edgesIterator != node->edges.end(); ++edgesIterator) {
                        BigInt edgeLength = 1 + edgesIterator->second->getRealEndIndex() - edgesIterator->second->beginIndex;

                        string edgeString = sourceString.substr(
                            edgesIterator->second->beginIndex
                            ,edgeLength
                        );

                        // Find if there is a non-letter charecter on this edge
                        BigInt firstNonLetterOffset = -1; // initially end of current substring
                        for (size_t index = 0; index <= edgeString.length() ; ++index) {
                            if ( isspace(edgeString[ index ]) || (edgeString[ index ] == '$') ) {
                                // Bot first non-letter
                                firstNonLetterOffset = index;
                                break;
                            }
                        }

                        if (firstNonLetterOffset == -1) { // No non-letter characters on this edge
                            freqToEdgeMap[edgesIterator->second->occurrenceNumber].emplace_back(depth, prefix, firstNonLetterOffset, edgesIterator->second);
                        } else if (firstNonLetterOffset > 0) { // Have a non-letter character in the middle of this edge
                            freqToEdgeMap[edgesIterator->second->occurrenceNumber].emplace_back(depth, prefix, firstNonLetterOffset, edgesIterator->second);
                        }
                    }
                }
            }
        }
        // If we got here, freqTopIt->second has no entries left nad is ready to be removed
        freqToEdgeMap.erase(freqTopIt);
    }
    return topN;
}

// Auxiliary debug print function
void CSuffixTree::printCurrentTopEdges ( CFrequencyToEdgeMap &freqToEdgeMap ) {
    for (auto &freqPair : freqToEdgeMap) {
        for (auto &prefixedEdge : freqPair.second) {
            string edgeSubstring = sourceString.substr(
                        prefixedEdge.edgePtr->beginIndex
                        ,prefixedEdge.edgePtr->getRealEndIndex() - prefixedEdge.edgePtr->beginIndex + 1
                    );

            if (prefixedEdge.firstNonLetterOffset != -1) {
                    edgeSubstring = edgeSubstring.substr(0, prefixedEdge.firstNonLetterOffset);
            }

            std::cout
                    << freqPair.first << " : '"
                    << prefixedEdge.prefixString
                    << edgeSubstring
                    << "'"
                    << std::endl;
        }
    }
}

// For memory recaimation on tree destruction
CEdge* CSuffixTree::newCEdge() {
    edgesKeeper.emplace_back(new CEdge(this)); // TODO: Use make_unique
    return edgesKeeper.back().get();
}

// For memory recaimation on tree destruction
CNode* CSuffixTree::newCNode() {
    nodesKeeper.emplace_back(new CNode(this)); // TODO: Use make_unique
    return nodesKeeper.back().get();
}

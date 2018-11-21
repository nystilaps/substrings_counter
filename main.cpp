#include "suffix_tree.h"

#include <sys/resource.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <climits>
#include <set>
#include <string>

// Increase stack size for recursizely walking suffix trees for bigger inputs
void setStackLimit() {
    // Really big number is needed to fit the recursion depth
    const rlim_t kStackSize = INT_MAX - 7;
    struct rlimit rl;
    int result;

    result = getrlimit(RLIMIT_STACK, &rl);
    if (result == 0)
    {
        if (rl.rlim_cur < kStackSize)
        {
            rl.rlim_cur = kStackSize;
            result = setrlimit(RLIMIT_STACK, &rl);
            if (result != 0)
            {
                fprintf(stderr, "setrlimit returned result = %d\n", result);
            }
        }
    }
}

void printUsage() {
    std::cerr << "Usage:" << std::endl
              << "./counter <file to read>" << std::endl;
}

int main(int argc, char *argv[]) {
    try {
        setStackLimit();

        std::string fileName = "";

        if (argc >= 2) {
            const std::set<std::string> helpCommands = {"-h", "--help", "-help" };
            if (helpCommands.count(argv[1]) > 0) {
                printUsage();
                return 0;
            }
        }

        if (argc != 2) {
            printUsage();
            return 1;
        }

        if (argc == 2) {
            fileName = argv[1];
        }

        std::ifstream inFile(fileName);
        if (inFile) {
            std::cout << "Reading file '" << fileName << "'" << std::endl;
            std::string input_string((std::istreambuf_iterator<char>(inFile)),
                                      std::istreambuf_iterator<char>());

            CSuffixTree tree( std::move(input_string) );
            tree.buildTree( );
            std::cout << "Suffix tree constructed." << std::endl;

            const int maximumTopNumber = 10;
            const int minimalLength = 4;

            std::cout << "Performing computation of top " << maximumTopNumber << " substrings longer or equal to " << minimalLength << " by occurrence frequency." << std::endl;
            std::cout << std::endl;

            auto topN = tree.getTopSuitableSubstrings(maximumTopNumber, minimalLength);

            double numberOfLongSubstrings = tree.getNumbetOfSubstringsLongerThan(minimalLength);
            std::cout << "Number of substrings longer or equal to " << minimalLength << " is: " << numberOfLongSubstrings << std::endl;
            std::cout << std::endl;

            prettyPrintFrequencyResults(topN);

            return 0;
        } else {
            std::cerr << "Couldn't open file '" << fileName << "'" << std::endl;
            printUsage();
            return 1;
        }

    } catch(std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    } catch(...) {
        std::cerr << "Error: Unknown error!" << std::endl;
    }
    return 1;
}

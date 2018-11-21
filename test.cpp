#include "suffix_tree.h"

#include <sys/resource.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <climits>
#include <set>
#include <string>
#include <random>
#include <algorithm>

const std::string AllLetters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

// Generate random string
std::string generateRandomString(std::string letters = AllLetters) {
    std::string result;
    //std::string spaces = " \n\r\t";
    std::string spaces = " \t";

    // Need static to be giving different results on different runs
    static std::mt19937 rng;

    // I prefere deterministic set of tests with pseudarandom numbers
    // Uncomment next line if you need tests different every time you run tests
    //rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> numberOfWordsDistribution(1, 60);
    std::uniform_int_distribution<std::mt19937::result_type> numberOfLettersInAWordDistrubution(1, 100);

    std::uniform_int_distribution<std::mt19937::result_type> distLetters(0, letters.size() - 1);
    std::uniform_int_distribution<std::mt19937::result_type> distSpaces(1, spaces.size() - 1);

    int numberOfWords = numberOfWordsDistribution(rng);
    for (int i = 0; i < numberOfWords; ++i) {
        int numberOfLetters = numberOfLettersInAWordDistrubution(rng);
        for (int j = 0; j < numberOfLetters; ++j) {
            result += letters[distLetters(rng)];
        }
        int numberOfSpaces = numberOfLettersInAWordDistrubution(rng);
        for (int j = 0; j < numberOfSpaces; ++j) {
            result += spaces[distSpaces(rng)];
        }
    }

    return result;
}

typedef std::pair<double, std::string> ResultEntry;
typedef std::vector<ResultEntry> FreqResults;

// Function that counts substring frequences in a slower, but more simple way
// Works only for words of alphabet letters and space symbols
ssize_t countFrequences(const std::string &text, FreqResults &results) {
    ssize_t numberOfSubstrings = 0;
    std::map<std::string, ssize_t> substringToOccurrenceNumber;

    const size_t minWordLength = 4;

    // Split string into words long enough
    std::vector<std::string> words;
    std::string word = "";
    for (auto c: text) {
        if (::isalpha(c)) {
            word += c;
        } else {
            if (word.length() >= minWordLength) {
                words.push_back(word);
            }
            word = "";
        }
    }
    // Take last accumulated word
    if (word.length() >= minWordLength) {
        words.push_back(word);
    }

    // Enumerate all substrings in every word
    for (auto &word: words) {
        for (size_t substringLength = minWordLength; substringLength < (word.length() + 1); ++substringLength) {
            for (size_t offset = 0; offset < (1 + word.length() - substringLength); ++offset) {
                auto substring = word.substr(offset, substringLength);
                substringToOccurrenceNumber[substring] += 1;
                numberOfSubstrings += 1;
            }
        }
    }

    // Format result like the suffix tree implementation
    for (auto p: substringToOccurrenceNumber) {
        results.emplace_back(100 * p.second / (double)numberOfSubstrings, p.first);
    }

    // Sort in descending order
    std::sort(results.begin(), results.end(), std::greater<ResultEntry>());

    return numberOfSubstrings;
}

// Test one string whether results of suffix tree implementation and anive on coinside
void runSingleTest(std::string testStr) {
    FreqResults res;
    countFrequences(testStr, res);

    CSuffixTree tree( testStr );
    tree.buildTree( );

    auto topN = tree.getTopSuitableSubstrings(10, 4);

    if (res.size() < topN.size()) {
        std::cout << testStr << std::endl;

        std::cout << "res" << std::endl;
        for (auto p: res) {
            std::cout << p.first << " " << p.second << std::endl;
        }

        std::cout << "topN" << std::endl;
        for (auto p: topN) {
            std::cout << p.first << " " << p.second << std::endl;
        }

        throw std::runtime_error("Sizes differ");
    }

    const double epsilon = 1e-12;

    for (size_t index = 0; index < topN.size(); ++index) {
        const double diff = fabs(res[index].first - topN[index].second);
        if (diff > epsilon) {
            throw std::runtime_error("Percentages differ");
        }
    }
}

void runTests() {
    std::cout << "Test on predetermined strings..." << std::endl;
    for (auto s: {"hall feels heels", "aaaa", "aaaaa", "aaaaaa", "abab", "ababa", "abababa"}) {
        std::string testStr = s;
        for (int repeat = 0; repeat < 7; ++repeat) {
            runSingleTest(testStr);
            testStr = testStr + " " + testStr;
        }
    }
    std::cout << "Test on predetermined strings passed." << std::endl;

    std::vector<std::string> dictionaries = { AllLetters, "abc", "ab", "a" };
    int dictionaryNumber = 1;
    for (auto dictionary: dictionaries) {
        std::cout << "Test set " << dictionaryNumber++ << " of " << dictionaries.size() << std::endl;
        //const std::string letters = "ab";
        const int numberOfTests = 10;
        for (int i = 0 ; i < numberOfTests; ++i) {
            std::cout << "  Test on random string of letters '" << dictionary << "' test num " << i << " of " << numberOfTests << std::endl;
            std::string testStr = generateRandomString(dictionary);
            runSingleTest(testStr);
        }
    }

    std::cout << "All tests successfully passed." << std::endl;
}

void printUsage() {
    std::cerr << "Usage:" << std::endl
              << "./counter <file to read>" << std::endl;
}

int main(int argc, char *argv[]) {
    try {
        runTests();
        return 0;
    } catch(std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    } catch(...) {
        std::cerr << "Error: Unknown error!" << std::endl;
    }
    return 1;
}

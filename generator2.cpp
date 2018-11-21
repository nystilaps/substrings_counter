#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <set>

std::vector<std::string> defaultWords = { "hall ", "feels ", "heels " };

void printUsage() {
    std::cerr << "Usage:" << std::endl
              << "./generator2 <number of repeats N> <string1> <string2> ... <stringN>" << std::endl
              << "Outputs all given strings N times with random shuffle with _no_ spaces between them." << std::endl
              << "By default N=5, strings are: ";
    std::string comma;
    for (auto word : defaultWords) {
        std::cerr << comma << "'" << word << "'";
        comma = ", ";
    }
    std::cerr << std::endl;
}

int main(int argc, char *argv[]) try {
    using namespace std;

    long long numberOfRepeats = 5;
    string pattern = "asdf ";

    if (argc >= 2) {
        const std::set<std::string> helpCommands = {"-h", "--help", "-help" };
        if (helpCommands.count(argv[1]) > 0) {
            printUsage();
            return 0;
        }

        numberOfRepeats = stoll( argv[1] );

        if (numberOfRepeats < 0) {
            cerr << "First argument must be number of repeats, second and following - strings pattern to repeat and shuffle" << endl;
            printUsage();
        }
    }

    std::vector<std::string> words = defaultWords;

    if (argc >= 3) {
        words = std::vector<std::string>();
        for (int i = 2; i < argc; ++i) {
            words.emplace_back(argv[i]);
        }
    }

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    auto rng = std::default_random_engine{seed};

    for (long long i = 0 ; i < numberOfRepeats ; ++i) {
        std::shuffle(std::begin(words), std::end(words), rng);
        for (auto word: words) {
            std::cout << word;
        }
    }

    return 0;
} catch(std::exception &e) {
    std::cerr << "Error1 " << e.what() << std::endl;
    printUsage();
    return 1;
} catch(...) {
    std::cerr << "Unknown error!" << std::endl;
    printUsage();
    return 1;
}

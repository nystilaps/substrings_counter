#include <iostream>
#include <string>
#include <set>

void printUsage() {
    std::cerr << "Usage:" << std::endl
              << "./generator1 <number of repeats> <string>" << std::endl
              << "Outputs given string N times" << std::endl
              << "By default N=5, string is \"asdf \" " << std::endl;
}

int main(int argc, char *argv[]) try {
	using namespace std;

    long long numberOfRepeats = 5;
    string pattern = "asdf ";

    /*if (argc < 3) {
        std::cout << "Error! Need at least two paramenters!" << std::endl;
    }*/

    if (argc >= 2) {
        const std::set<std::string> helpCommands = {"-h", "--help", "-help" };
        if (helpCommands.count(argv[1]) > 0) {
            printUsage();
            return 0;
        }
        numberOfRepeats = stoll( argv[1] );
        if (numberOfRepeats < 0) {
            cerr << "First argument must be number of repeats, second - string pattern to repeat" << endl;
            printUsage();
            return 1;
        }
    }

    if (argc >= 3) {
        pattern = argv[2];
    }

    if (argc > 3) {
        std::cerr << "Need no more than two parameters!" << std::endl;
        printUsage();
        return 1;
    }

    for (long long i = 0 ; i < numberOfRepeats ; ++i) {
        std::cout << pattern;
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

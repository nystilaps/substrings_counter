//#include "suffix_tree.h"
#include "print.h"

#include <math.h>

#include <iomanip>
#include <iostream>

const double epsilon = 1e-12;

void prettyPrintFrequencyResults(CFrequencyInfo topN) {

    if ( topN.empty() ) {
        std::cout << "No results to show." << std::endl;
        return;
    }

    // Table headers
    const std::string idHeader = "Id";
    const std::string substringHeader = "Substring";
    const std::string percentageHeader = "Percentage";

    // Table sizes
    size_t idColumnWidth = std::max((size_t)4, idHeader.length());
    size_t textColumnWidth = std::max((size_t)4, substringHeader.length());
    for (auto p : topN)
        textColumnWidth = std::max(textColumnWidth, p.first.length());
    size_t numberColumnWidth = std::max((size_t)30, percentageHeader.length());

    // Write one separator line in table
    auto writeSeparatorLine = [&]() {
        std::cout << std::setfill('-');
        std::cout << "+" << std::setw(idColumnWidth) << ""
                  << "+" << std::setw(textColumnWidth) << ""
                  << "+" << std::setw(numberColumnWidth + 1) << ""  << "+"
                  << std::endl;
    };

    // Print table header
    writeSeparatorLine();
    std::cout << std::setfill(' ');
    std::cout << "|" << std::setw(idColumnWidth) << std::right << idHeader
              << "|" << std::setw(textColumnWidth) << std::right << substringHeader
              << "|" << std::setw(numberColumnWidth + 1) << std::right << percentageHeader << "|"
              << std::endl;
    writeSeparatorLine();

    double maxPercentage = topN.front().second;
    double minPercentage = topN.front().second;

    // Print data in table
    {
        ssize_t index = 0; // Id of the current value
        for (auto p: topN) {
            maxPercentage = std::max(maxPercentage, p.second);
            minPercentage = std::min(minPercentage, p.second);

            std::cout << std::setfill(' ');
            std::cout << "|" << std::setw(idColumnWidth) << std::right << index
                      << "|" << std::setw(textColumnWidth) << std::right << p.first
                      << "|" << std::setw(numberColumnWidth) << std::right
                             //<< std::setprecision(numberColumnWidth - 6)
                             << p.second << "%" << "|"
                      << std::endl;

            writeSeparatorLine();

            index++;
        }
    }

    std::cout << std::endl;

    // Uncomment the following block, if no chart for same values is needed
    // Too small difference in results, nothing to show
    /*if (::fabs(maxPercentage - minPercentage) < epsilon) {
        std::cout << "Results are approximately equal, nothing to show on a chart" << std::endl;
        return;
    }*/

    // Discard minPercentage and set it to zero
    // Showing bars from non-zero level looks less informative
    minPercentage = 0;

    const double difference = ::fabs(maxPercentage - minPercentage);

    // Not enought info to draw a chart
    if (topN.size() < 2) {
        std::cout << "Only one result, nothing to show on a chart" << std::endl;
        return;
    }

    std::cout << "Frequency difference horizontal bar chart:" << std::endl;
    std::cout << std::endl;

    // Maximal length of bars
    ssize_t graphWidth = 70;

    // Print bar header
    std::cout << std::setfill(' ');
    std::cout << std::setw(idColumnWidth) << std::right << ""
              << std::setw(graphWidth - 1) << std::left << "| Minimal %"
              << "| Maximal %"
              << std::endl;

    std::cout << std::setw(idColumnWidth) << std::left << idHeader << "|"
              << std::setw(graphWidth - 2) << std::left << minPercentage
              << "|" << maxPercentage
              << std::endl;

    std::cout << std::setfill('-');
    std::cout << std::setw(idColumnWidth) << std::right << "" << "+"
              << std::setw(graphWidth - 1) << std::right << "+"
              << std::setw(numberColumnWidth) << std::right << ""
              << std::endl;

    std::cout << std::setfill(' ');

    // Print bars for results
    ssize_t index = 0; // Id of the current bar
    for (auto p: topN) {
        ssize_t points = (int)::round(graphWidth * (p.second - minPercentage) / difference);

        std::cout << std::setw(idColumnWidth) << std::left << index;

        // Filling in the bar
        for (int i = 1 ; i < points; ++i) {
            if (i == 1) {
                std::cout << "|";
            } else {
                std::cout << "#";
            }
        }

        // Print percentage at the end of the bar
        std::cout << "| "
                  //<< std::setprecision(numberColumnWidth - 6)
                  << p.second << "%"
                  << std::endl;
        index++;
    }
}

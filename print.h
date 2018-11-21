#pragma once

#include <string>
#include <vector>
#include <utility>

typedef std::vector<std::pair<std::string, double>> CFrequencyInfo;

// Prints CFrequencyInfo as a table ans as a bar chart
void prettyPrintFrequencyResults(CFrequencyInfo topN);

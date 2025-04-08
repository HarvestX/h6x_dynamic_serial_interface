#pragma once
#include <vector>

bool initFileSystem(const char* filename);
void writeDataBuffered(const std::vector<String>& buffer, const char* filename);

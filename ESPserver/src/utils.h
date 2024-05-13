#ifndef UTILS_H
#define UTILS_H

#include <unordered_map>
#include <string>
#include <fstream>
#include <iostream>

std::unordered_map<std::string, std::string> readCredentials(const std::string &filename);

#endif // UTILS_H
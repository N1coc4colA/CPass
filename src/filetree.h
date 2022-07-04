#pragma once

#include <string>
#include "dom.h"

bool putToFile(std::string path, Document doc, bool force = false);
Document loadFromFile(std::string path);

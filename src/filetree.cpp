#include "filetree.h"

#include <fstream>
#include <iostream>

#define contains(x, y) (std::find(x.begin(), x.end(), y) != x.end())

#define FMODE | std::ios::binary

bool putToFile(std::string path, Document doc, bool force)
{
	if (!doc.valid && !force) {
		std::cout << "Error: document to write is invalid." << std::endl;
		return false;
	}
	File f;
	f.file.open(path, std::ios::out | std::ios::trunc FMODE);
	if (!f.file) {
#ifndef NDEBUG
		std::cout << "DBG: Failed to open file." << std::endl;
#endif
		return false;
	}

	f << doc;

	f.file.close();
	return true;
}

Document loadFromFile(std::string path)
{
	Document d;
	d.valid = false;

	File f;
	f.file.open(path, std::ios::in FMODE);
	if (f.file) {
		f >> d;
		d.valid = true;
		f.file.close();
	} else {
#ifndef NDEBUG
		std::cout << "DBG: Failed to open file." << std::endl;
#endif
		d.valid = false;
	}

	return d;
}

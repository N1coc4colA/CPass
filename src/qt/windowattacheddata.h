#pragma once

#include <QString>

#include "../cypheria.h"
#include "../dom.h"
#include "../storage.h"

class WindowAttachedData
{
public:
	Cypheria cypher;
	Storage storage;
	QString path;
};

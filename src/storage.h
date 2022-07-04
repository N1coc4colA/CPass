#pragma once

#include <map>
#include <string>
#include <list>
#include "dom.h"
#include "cypheria.h"

class Storage
{
public:
	Storage();
	Storage(Storage &other);

	void loadFrom(Document *d);
	Document exportContent();

	bool setCypher(Cypheria *cyph);

	virtual bool addContainer(std::string app);
	virtual bool addField(std::string app, std::string name, std::string content = "");

	virtual bool removeContainer(std::string app);
	virtual bool removeField(std::string app, std::string name);

	virtual bool changeContainerName(std::string prev, std::string next);
	virtual bool changeFieldName(std::string app, std::string prev, std::string next);
	virtual bool changeFieldValue(std::string app, std::string name, std::string val);

	virtual bool setImageValue(std::string app, std::string v);

	std::list<std::string> getContainersNames();
	std::list<std::string> getFieldsNames(std::string app);
	std::string value(std::string app, std::string name);
	std::string imageValue(std::string app);

	virtual bool switchAll(Cypheria *cyph);

	Storage& operator=(const Storage&);

protected:
	std::map<std::string, Container> _containers;
	std::map<std::string, std::map<std::string, Field>> _fields;

	Cypheria *cypher = nullptr;
};

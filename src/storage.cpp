#include "storage.h"

#include <vector>
#include <iostream>

#ifdef NDEBUG
#define 	CYPH_CHECK(x) if (!cypher) {return x;}
#else
#include <iostream>
#define 	CYPH_CHECK(x) if (!cypher) {std::cout << "DBG: No cypher." << " (" << __LINE__ << ": " << __func__ << ")" << std::endl; return x;}
#define 	CTR_NF std::cout << "DBG: Container not found: " << capp << " (" << __LINE__ << ": " << __func__ << ")" << std::endl;
#define 	CTR_AF std::cout << "DBG: Container already found: " << capp << " (" << __LINE__ << ": " << __func__ << ")" << std::endl;
#define 	FLD_NF std::cout << "DBG: Field not found: " << cname << " (" << __LINE__ << ": " << __func__ << ")" << std::endl;
#define 	FLD_AF std::cout << "DBG: Field already found: " << cname << " (" << __LINE__ << ": " << __func__ << ")" << std::endl;
#endif

/*
 * Always use variable names as following for smoother log while debugging.
 * app: refers to the non crypted name of the container.
 * name: refers to the non crypted name of the field.
 *
 * c*: refers to the variable's value when it has been crpted.
 */

Storage::Storage() {}

Storage::Storage(Storage &other)
{
	_containers = other._containers;
	_fields = other._fields;
	cypher = other.cypher;
}

Storage& Storage::operator=(const Storage&other)
{
	_containers = other._containers;
	_fields = other._fields;
	cypher = other.cypher;
	return *this;
}

void Storage::loadFrom(Document *d)
{
	CYPH_CHECK();

	_containers.clear();
	_fields.clear();

	for (auto v : d->model.content) {

		std::string name = cyph->decrypt(v.name, v.n_size, v.n_fill);
		_containers[name] = v;

		std::map<std::string, Field> _map;
		std::cout << v.name << std::endl;

		for (auto f : v.fields) {
			_map[cyph->decrypt(f.name, f.n_size, f.n_fill)] = f;
		}
		_fields[name] = _map;

		v.fields.clear();
	}
}

Document Storage::exportContent()
{
	Document d;
	d.valid = false;

	CYPH_CHECK(d);

	std::map<std::string, Container> copies;

	for (auto const& [key, val] : _containers) {
		Container tmp = val;
		tmp.fields.clear();
		for (auto const& [k, v] : _fields[key]) {
			tmp.fields.push_back(v);
		}
		d.model.content.push_back(tmp);
	}

	d.header.salt = cypher->getSalt();
	d.header.sample = cypher->getSample(&d.header.size, &d.header.fill);

	d.valid = true;

	d.print();

	return d;
}

bool Storage::setCypher(Cypheria *cyph)
{
	cypher = cyph;
	if (cyph) {
		cyph->isSame("", "");
	}
	return cyph != nullptr;
}

bool Storage::addContainer(std::string app)
{
	CYPH_CHECK(false);

	if (_containers.contains(app)) {
#ifndef NDEBUG
		CTR_AF
#endif
		return false;
	}

	Container ctr;
	size_t s = 0;
	char chr = 0;
	ctr.name = cypher->crypt(app, &ctr.n_size, &ctr.n_fill);
	ctr.icon = "";
	_containers[app] = ctr;
	_fields[app] = {};

	return true;
}

bool Storage::addField(std::string app, std::string name, std::string content)
{
	CYPH_CHECK(false);

	size_t s = 0;
	char chr = 0;
	std::string capp = cypher->crypt(app, &s, &chr);
	if (!_containers.contains(capp)) {
#ifndef NDEBUG
		CTR_NF
#endif
		return false;
	}

	std::string cname = cypher->crypt(name, &s, &chr);
	if (_fields[capp].contains(cname)) {
#ifndef NDEBUG
		FLD_AF
#endif
		return false;
	}

	Field f;
	f.name = cname;
	f.n_size = s;
	f.n_fill = chr;

	f.content = cypher->crypt(content, &f.c_size, &f.c_fill);
#ifndef NDEBUG
	std::cout << "Crypted field name: " << f.name << std::endl;
#endif
	_fields[capp][cname] = f;

	return true;
}

bool Storage::removeContainer(std::string app)
{
	CYPH_CHECK(false);

	if (!_containers.contains(app)) {
#ifndef NDEBUG
		CTR_NF
#endif
		return false;
	}

	_containers.erase(app);
	_fields.erase(app);

	return true;
}

bool Storage::removeField(std::string app, std::string name)
{
	CYPH_CHECK(false);

	if (!_containers.contains(app)) {
#ifndef NDEBUG
		CTR_NF
#endif
		return false;
	}

	if (!_fields[capp].contains(name)) {
#ifndef NDEBUG
		FLD_NF
#endif
		return false;
	}

	_fields[app].erase(name);

	return true;
}

bool Storage::changeContainerName(std::string app, std::string next)
{
	CYPH_CHECK(false);

	if (!_containers.contains(app)) {
#ifndef NDEBUG
		CTR_NF
#endif
		return false;
	}

	Container f = _containers[app];
	f.name = cypher->crypt(next, &f.n_size, &f.n_fill);
	_containers[next] = f;

	auto node = _containers.extract(app);
	node.key() = next;
	_containers.insert(std::move(node));

	return true;
}

bool Storage::changeFieldName(std::string app, std::string name, std::string next)
{
	CYPH_CHECK(false);

	if (!_containers.contains(app)) {
#ifndef NDEBUG
		CTR_NF
#endif
		return false;
	}

	if (!_fields[app].contains(name)) {
#ifndef NDEBUG
		FLD_NF
#endif
		return false;
	}

	Field f = _fields[app][name];
	f.name = cypher->crypt(next, &f.n_size, &f.n_fill);
	_fields[app][name] = f;

	auto node = _fields[app].extract(name);
	node.key() = next;
	_fields[app].insert(std::move(node));

	return true;
}

bool Storage::changeFieldValue(std::string app, std::string name, std::string val)
{
	CYPH_CHECK(false);

	if (!_containers.contains(app)) {
#ifndef NDEBUG
		CTR_NF
#endif
		return false;
	}

	if (!_fields[app].contains(name)) {
#ifndef NDEBUG
		FLD_NF
#endif
		return false;
	}

	_fields[app][name].content = cypher->crypt(val, &_fields[app][name].c_size, &_fields[app][name].c_fill);

	return true;
}

bool Storage::setImageValue(std::string app, std::string v)
{
	CYPH_CHECK(false);

	if (!_containers.contains(app)) {
#ifndef NDEBUG
		CTR_NF
#endif
		return false;
	}

	_containers[app].icon = v;
	return true;
}

std::list<std::string> Storage::getContainersNames()
{
	CYPH_CHECK({});

	std::list<std::string> list;
	for (auto &x : _containers) {
		list.push_back(x.first);
	}

	return list;
}

std::list<std::string> Storage::getFieldsNames(std::string app)
{
	CYPH_CHECK({});

	if (!_containers.contains(app)) {
#ifndef NDEBUG
		for (auto const& [key, val] : _containers) {
			std::cout << ((key == app) ? "ok: " : "nope: ") << key << std::endl;
		}
		CTR_NF
#endif
		return {};
	}

	std::list<std::string> list;
	for (auto const& [key, val] : _fields[capp]) {
		list.push_back(key);
	}

	return list;
}

std::string Storage::value(std::string app, std::string name)
{
	CYPH_CHECK("");

	if (!_containers.contains(app)) {
#ifndef NDEBUG
		CTR_NF
#endif
		return "";
	}

	if (!_fields[app].contains(name)) {
#ifndef NDEBUG
		FLD_NF
#endif
		return "";
	}

	return cypher->decrypt(_fields[app][name].content, _fields[app][name].c_size, _fields[app][name].c_fill);
}

std::string Storage::imageValue(std::string app)
{
	CYPH_CHECK("");

	if (!_containers.contains(app)) {
#ifndef NDEBUG
		CTR_NF
#endif
		return "";
	}

	return _containers[app].icon;
}

bool Storage::switchAll(Cypheria *cyph)
{
	CYPH_CHECK(false);

	Storage other;
	if (!other.setCypher(cyph)) {
#ifndef NDEBUG
		std::cout << "DBG: Other cypher invalid." << std::endl;
#endif
		return false;
	}

	std::vector<std::string> list;
	std::vector<std::string> dlist;
	for (auto const& [key, val] : _containers) {
		list.push_back(key);
		std::string d = cypher->decrypt(key, val.n_size, val.n_fill);
		dlist.push_back(d);
		other.addContainer(d);
	}

	//Now put the fields.
	size_t i = 0;
	while (i < list.size()) {
		for (auto const& [key, val] : _fields[list[i]]) {
			std::string dname = cypher->decrypt(key, val.n_size, val.n_fill);
			std::string dcontent = cypher->decrypt(val.content, val.c_size, val.c_fill);
			other.addField(dlist[i], dname, dcontent);
		}
		i++;
	}

	*this = other;

	return true;
}

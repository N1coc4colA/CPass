#pragma once

#include <fstream>
#include <string>
#include <list>

struct File
{
	std::fstream file;
};

struct Field
{
	std::string name;
	char n_fill;
	size_t n_size;
	std::string content;
	char c_fill;
	size_t c_size;
	void print(std::string (*c)(std::string, size_t, const char) = nullptr);
};

struct Container
{
	std::string name;
	char n_fill;
	size_t n_size;
	std::string icon;
	std::list<Field> fields;
	void print(std::string (*c)(std::string, size_t, const char) = nullptr);
};

struct Model
{
	std::list<Container> content;
	void print(std::string (*c)(std::string, size_t, const char) = nullptr);
};

struct MHeader
{
	std::string salt;
	std::string sample;
	char fill;
	size_t size;

	void print(std::string (*c)(std::string, size_t, const char) = nullptr);
};

struct Document
{
	MHeader header;
	Model model;
	bool valid = true; //RT only

	void print(std::string (*c)(std::string, size_t, const char) = nullptr);
};

File &operator<<(File &f, char v);
File &operator<<(File &f, size_t v);
File &operator<<(File &f, const char *v);
File &operator<<(File &f, std::string v);
File &operator<<(File &f, Field v);
File &operator<<(File &f, Container v);
File &operator<<(File &f, MHeader v);
File &operator<<(File &f, Model v);
File &operator<<(File &f, Document v);

File &operator>>(File &f, char &v);
File &operator>>(File &f, size_t &v);
File &operator>>(File &f, std::string &v);
File &operator>>(File &f, Field &v);
File &operator>>(File &f, Container &v);
File &operator>>(File &f, MHeader &v);
File &operator>>(File &f, Model &v);
File &operator>>(File &f, Document &v);

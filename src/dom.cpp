#include "dom.h"

#include <iostream>
#include <cstring>

template<typename T>
File &operator<<(File &f, std::list<T> v)
{
	size_t s = v.size();
	f << s;
	if (s > 0) {
		for (auto c : v) {
			f << c;
		}
	}
	return f;
}

template<typename T>
File &operator>>(File &f, std::list<T> &v)
{
	v.clear();
	size_t s = 0;
	f >> s;
	if (s > 0) {
		size_t i = 0;
		while (i < s) {
			T tmp;
			f >> tmp;
			v.push_back(tmp);
			i++;
		}
	}
	return f;
}

File &operator<<(File &f, char v)
{
	f.file.write((const char *)&v, sizeof(v));
	return f;
}

File &operator<<(File &f, size_t v)
{
	f.file.write((const char *)&v, sizeof(v));
	return f;
}

File &operator<<(File &f, const char *v)
{
	size_t s = strlen(v);
	f << s;
	if (s > 0) {
		f.file.write(v, s * sizeof(const char));
	}
	return f;
}

File &operator<<(File &f, std::string v)
{
	size_t s = v.length();
	f << s;
	if (s > 0) {
		std::cout << "Size: " << s << "\nValue: " << v << std::endl;
		f.file.write(v.c_str(), s * sizeof(const char));
	}
	return f;
}

File &operator<<(File &f, Field v)
{
	return f << v.name << v.n_fill << v.n_size << v.content << v.c_fill << v.c_size;
}

File &operator<<(File &f, Container v)
{
	return f << v.name << v.n_fill << v.n_size << v.icon << v.fields;
}

File &operator<<(File &f, MHeader v)
{
	return f << v.salt << v.sample << v.fill << v.size;
}

File &operator<<(File &f, Model v)
{
	return f << v.content;
}

File &operator<<(File &f, Document v)
{
	f << v.header << v.model;
	return f;
}

File &operator>>(File &f, char &v)
{
	f.file.read((char *)&v, sizeof(v));
	return f;
}

File &operator>>(File &f, size_t &v)
{
	f.file.read((char *)&v, sizeof(v));
	return f;
}

File &operator>>(File &f, std::string &v)
{
	v.clear();
	size_t s = 0;
	f >> s;
	if (s > 0) {
		std::cout << "size: " << s << std::endl;
		char *buff = new char[s];
		//Null that first!
		memset(buff, 0, s * sizeof(char));
		f.file.read(buff, s * sizeof(char));
		std::cout << "Value: " << buff << std::endl;
		v = std::string(buff);
		delete [] buff;
	}
	return f;
}

File &operator>>(File &f, Field &v)
{
	return f >> v.name >> v.n_fill >> v.n_size >> v.content >> v.c_fill >> v.c_size;
}

File &operator>>(File &f, Container &v)
{
	f >> v.name >> v.n_fill >> v.n_size >> v.icon >> v.fields;
	std::cout << "Container's name: " << v.name << std::endl;
	return f;
}

File &operator>>(File &f, MHeader &v)
{
	return f >> v.salt >> v.sample >> v.fill >> v.size;
}

File &operator>>(File &f, Model &v)
{
	return f >> v.content;
}

File &operator>>(File &f, Document &v)
{
	return f >> v.header >> v.model;
}

void Field::print(std::string (*c)(std::string, size_t, const char))
{
	std::cout << " |   |   +- Field: " << (c ? c(name, n_size, n_fill) : name) << " (" << (c ? std::to_string(c(name, n_size, n_fill).size()) : "") << ( c ? " - " : "") << n_size << " - " << n_fill << ")" << "\n"
			  << " |   |       +- Value: " << (c ? c(content, c_size, c_fill) : content) << " (" << (c ? std::to_string(c(content, c_size, c_fill).size()) : "") << ( c ? " - " : "") << c_size << " - " << c_fill << ")" << std::endl;
}

void Container::print(std::string (*c)(std::string, size_t, const char))
{
	std::cout << " |   +- Container: " << (c ? c(name, n_size, n_fill) : name) << " (" << (c ? std::to_string(c(name, n_size, n_fill).size()) : "") << ( c ? " - " : "") << n_size << " - " << n_fill << ")" << std::endl;
	for (auto v : fields) {
		v.print(c);
	}
}

void Model::print(std::string (*c)(std::string, size_t, const char))
{
	std::cout << " +- Model:" << std::endl;
	for (auto v : content) {
		v.print(c);
	}
}

void MHeader::print(std::string (*c)(std::string, size_t, const char))
{
	std::cout << " +- Header:" << "\n"
			  << " |   +- Sample: " << (c ? c(sample, size, fill) : sample) << " (" << (c ? std::to_string(c(sample, size, fill).size()) : "") << ( c ? " - " : "") << size << " - " << fill << ")\n"
			  << " |   +- Salt: " << salt << "\n"
			  << " |" << std::endl;
}

void Document::print(std::string (*c)(std::string, size_t, const char))
{
	std::cout << "Document:" << std::endl;
	header.print(c);
	model.print(c);
}

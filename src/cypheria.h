#pragma once

#include <string>

class Cypheria
{
public:
	Cypheria();
	Cypheria(Cypheria &);
	Cypheria &operator=(Cypheria &);
	Cypheria &operator=(Cypheria);

	void setSalt(std::string);
	void setKey(std::string);

	std::string crypt(std::string, size_t *s, char *c);
	std::string decrypt(std::string, size_t s, char c);

	bool isSame(std::string d, std::string c);

	std::string getSample(size_t *s, char *c);
	std::string getSalt();

private:
	size_t keyLength;
	size_t blkLength;

	std::string key;
	std::string salt;
};

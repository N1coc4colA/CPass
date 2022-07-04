#include "cypheria.h"

#include <gcrypt.h>
#include <iostream>

#define GCRY_CIPHER GCRY_CIPHER_SERPENT256
#define GCRY_C_MODE GCRY_CIPHER_MODE_ECB

Cypheria::Cypheria()
{
	keyLength = gcry_cipher_get_algo_keylen(GCRY_CIPHER);
    blkLength = gcry_cipher_get_algo_blklen(GCRY_CIPHER);
}

Cypheria::Cypheria(Cypheria &other)
{
	keyLength = other.keyLength;
	blkLength = other.blkLength;
	key = other.key;
	salt = other.salt;
}

Cypheria &Cypheria::operator=(Cypheria &other)
{
	keyLength = other.keyLength;
	blkLength = other.blkLength;
	key = other.key;
	salt = other.salt;
	return *this;
}

Cypheria &Cypheria::operator=(Cypheria other)
{
	keyLength = other.keyLength;
	blkLength = other.blkLength;
	key = other.key;
	salt = other.salt;
	return *this;
}

void Cypheria::setSalt(std::string s)
{
	salt = s;
}

void Cypheria::setKey(std::string k)
{
	key = k;
}

bool Cypheria::isSame(std::string d, std::string c)
{
	size_t s = 0;
	char chr = 0;
	return c == crypt(d, &s, &chr);
}

std::string Cypheria::getSample(size_t *s, char *chr)
{
	return crypt(key, s, chr);
}

std::string Cypheria::getSalt()
{
	return salt;
}

std::string Cypheria::crypt(std::string txt, size_t *s, char *c)
{
	//[TODO] We need to run it one time first to then have proper results, don't ask me why.
	/*size_t __e_size = 0;
	char   __e_char = 0;
	crypt("", &__e_size, &__e_char);*/

	gcry_error_t     gcryError;
	gcry_cipher_hd_t gcryCipherHd;

	*s = 0;
	*c = 0;
	//If the size is not good, add it.
	if (((txt.length()) % keyLength) != 0) {
		*c = 1;
		while (txt.find(c) != std::string::npos) {
			*c = (*c)+1;
		}

		while (((txt.length()) % keyLength) != 0) {
			txt.push_back(*c);
		}
	}

	const char *txtBuffer = txt.c_str();
	size_t txtLength = txt.length();
	char *encBuffer = new char [txtLength];

	gcryError = gcry_cipher_open(
		&gcryCipherHd, // gcry_cipher_hd_t *
		GCRY_CIPHER,   // int
		GCRY_C_MODE,   // int
		0);            // unsigned int
	if (gcryError) {
		printf("gcry_cipher_open failed:  %s/%s\n",
			   gcry_strsource(gcryError),
			   gcry_strerror(gcryError));
		return "";
	}

	//[TODO] The key might have a bigger size than keyLength?
	gcryError = gcry_cipher_setkey(gcryCipherHd, key.c_str(), key.length());
	if (gcryError) {
		printf("gcry_cipher_setkey failed:  %s/%s\n",
			   gcry_strsource(gcryError),
			   gcry_strerror(gcryError));
		return "";
	}

	gcryError = gcry_cipher_setiv(gcryCipherHd, salt.c_str(), blkLength);
	if (gcryError) {
		printf("gcry_cipher_setiv failed:  %s/%s\n",
			   gcry_strsource(gcryError),
			   gcry_strerror(gcryError));
		return "";
	}

	gcryError = gcry_cipher_encrypt(
		gcryCipherHd, // gcry_cipher_hd_t
		encBuffer,    // void *
		txtLength,    // size_t
		txtBuffer,    // const void *
		txtLength);   // size_t
	if (gcryError) {
		printf("gcry_cipher_encrypt failed:  %s/%s\n",
			   gcry_strsource(gcryError),
			   gcry_strerror(gcryError));
		return "";
	}

	std::string out(encBuffer);
	*s = txtLength;

	// clean up after ourselves
	gcry_cipher_close(gcryCipherHd);
	delete [] encBuffer;

	return out;
}

std::string Cypheria::decrypt(std::string txt, size_t s, char c)
{
	if (txt.empty()) {
		return "";
	}

	gcry_error_t     gcryError;
	gcry_cipher_hd_t gcryCipherHd;

	char *outBuffer = new char[s];

	gcryError = gcry_cipher_open(
		&gcryCipherHd, // gcry_cipher_hd_t *
		GCRY_CIPHER,   // int
		GCRY_C_MODE,   // int
		0);            // unsigned int
	if (gcryError) {
		printf("gcry_cipher_open failed:  %s/%s\n",
			   gcry_strsource(gcryError),
			   gcry_strerror(gcryError));
		return "";
	}

	gcryError = gcry_cipher_setkey(gcryCipherHd, key.c_str(), key.length());
	if (gcryError) {
		printf("gcry_cipher_setkey failed:  %s/%s\n",
			   gcry_strsource(gcryError),
			   gcry_strerror(gcryError));
		return "";
	}

	gcryError = gcry_cipher_setiv(gcryCipherHd, salt.c_str(), blkLength);
	if (gcryError) {
		printf("gcry_cipher_setiv failed:  %s/%s\n",
			   gcry_strsource(gcryError),
			   gcry_strerror(gcryError));
		return "";
	}

	gcryError = gcry_cipher_decrypt(
		gcryCipherHd, // gcry_cipher_hd_t
		outBuffer,    // void *
		s,			    // size_t
		txt.c_str(),    // const void *
		s);   // size_t
	if (gcryError) {
		printf("gcry_cipher_decrypt failed:  %s/%s\n",
			   gcry_strsource(gcryError),
			   gcry_strerror(gcryError));
		return "";
	}

	std::string out(outBuffer);

	// clean up after ourselves
	gcry_cipher_close(gcryCipherHd);
	delete [] outBuffer;

	//Now trim.
	size_t p = out.find_first_of(c);
	if (p != std::string::npos) {
		out.erase(p, out.size()-p);
	}
	return out;
}

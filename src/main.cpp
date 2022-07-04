#include <gcrypt.h>
#include <assert.h>

#include <iostream>

#include <execinfo.h> // for backtrace
#include <dlfcn.h>    // for dladdr
#include <cxxabi.h>   // for __cxa_demangle

#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>

#include <unistd.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <signal.h>

#include <QApplication>

#include "cypheria.h"
#include "filetree.h"
#include "storage.h"
#include "qt/loginwindow.h"

#define NEED_LIBGCRYPT_VERSION "1.8.8"
#define GCRY_CIPHER GCRY_CIPHER_SERPENT256

#define PSWD_ERR std::cout << "Error: password is wrong." << std::endl;

const char *helpText = "Usage of this tool:\n"
" [password] create [file]\n"
"   Create a new file used to store data.\n"
" [password] [file]   list\n"
"   List the sections available in the file.\n"
" [password] [file]   list   [section]\n"
"   List the titles available in a section of the file.\n"
" [password] [file]    add   [section]\n"
"   Add a section to the file.\n"
" [password] [file]    add   [section]            [title]        ([value])\n"
"   Add a title (and maybe its value) to a section of the file.\n"
" [password] [file]    set   [section]            [title]          [value]\n"
"   Set the value for a title in a section of the file.\n"
" [password] [file] rename   [section] [new_section_name]\n"
"   Rename a section of the file.\n"
" [password] [file] rename   [section]            [title] [new_title_name]\n"
"   Rename a title in a section of the file.\n"
" [password] [file] remove   [section]\n"
"   Remove a whole section from the file.\n"
" [password] [file] remove   [section]            [title]\n"
"   Remove a title from a section of the file.\n"
" [password] [file]   show   [section]            [title]\n"
"   Show the value of a title in a section of the file.\n"
" [password] [file] change  [password]\n"
"   Change the password that is used in the file.\n"
" [password] [file]  print\n"
"   Print the data contained in the file: header, sections, titles and their values.\n"
"      print [file]\n"
"   Print the raw data contained in the file: header, sections, titles, and their values.\n"
"      crypt [salt] [password] [size] [filler] [data]\n"
"   Crypt some data using the given salt and password as the app would do.\n"
"    decrypt [salt] [password] [size] [filler] [data]\n"
"   Decrypt some data using the given salt and password as the app would do.\n"
"    --force -f [args...]\n"
"   On commands that require the use of password, force to process even if the password is wrong.\n"
"  --version -v\n"
"   Show the version of the binary\n"
"    --infos -i\n"
"   Show informations about the binary.\n"
"     --help -h\n"
"   Show this text.\n"
"";

extern std::string gen_random(const int len);

Cypheria *_cyr = nullptr;

std::string Backtrace(const int MSF, int skip = 1)
{
	void *callstack[MSF];
	const int nMaxFrames = sizeof(callstack) / sizeof(callstack[0]);
	char buf[MSF];
	int nFrames = backtrace(callstack, nMaxFrames);
	char **symbols = backtrace_symbols(callstack, nFrames);

	std::ostringstream trace_buf;
	for (int i = skip; i < nFrames; i++) {
		Dl_info info;
		if (dladdr(callstack[i], &info) && info.dli_sname) {
			char *demangled = nullptr;
			int status = -1;
			if (info.dli_sname[0] == '_')
				demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
			snprintf(buf, sizeof(buf), "%-3d %*p %s + %zd\n",
					 i, int(2 + sizeof(void*) * 2), callstack[i],
					 status == 0 ? demangled :
					 info.dli_sname == nullptr ? symbols[i] : info.dli_sname,
			(char *)callstack[i] - (char *)info.dli_saddr);
			free(demangled);
		} else {
			snprintf(buf, sizeof(buf), "%-3d %*p %s\n",
					 i, int(2 + sizeof(void*) * 2), callstack[i], symbols[i]);
		}
		trace_buf << buf;
	}
	free(symbols);
	if (nFrames == nMaxFrames)
		trace_buf << "[truncated]\n";
	return trace_buf.str();
}

void printStack(std::ostream &stream, const int MAX_STACK_FRAMES = 128)
{
	stream << Backtrace(MAX_STACK_FRAMES) << std::endl;
}

void handleSignal [[ noreturn ]] (int sig)
{
	std::string sigVal;
	switch (sig) {
		case SIGTERM: sigVal = "SIGTERM"; break;
		case SIGILL: sigVal = "SIGILL"; break;
		case SIGSEGV: sigVal = "SIGSEGV"; break;
		case SIGINT: sigVal = "SIGINT"; break;
		case SIGABRT: sigVal = "SIGABRT"; break;
		case SIGFPE: sigVal = "SIGFPE"; break;
	}

	std::cerr << "Received signal " << sigVal << "\nBacktrace:" << std::endl;

	printStack(std::cerr);
	exit(sig);
}

std::string decrypt(std::string c, size_t s, const char chr)
{
	return _cyr->decrypt(c, s, chr);
}

int main(int argc, char **argv)
{
	signal(SIGTERM, handleSignal);
	signal(SIGSEGV, handleSignal);
	signal(SIGILL, handleSignal);
	signal(SIGINT, handleSignal);
	signal(SIGABRT, handleSignal);
	signal(SIGFPE, handleSignal);

	/*
	 V *ersion check should be the very first call because it
	 makes sure that important subsystems are initialized.
	 #define NEED_LIBGCRYPT_VERSION to the minimum required version.
	*/

	if (!gcry_check_version(NEED_LIBGCRYPT_VERSION)) {
		std::cerr << "libgcrypt is too old (need " << NEED_LIBGCRYPT_VERSION << ", have " << gcry_check_version(NULL) << ")" << std::endl;
		exit(2);
	}

	/*
	 We don't want to see any warnings, e.g. because we have not yet
	 parsed program options which might be used to suppress such
	 warnings.
	*/
	gcry_control(GCRYCTL_SUSPEND_SECMEM_WARN);

	/*
	 ... If required, other initialization goes here.  Note that the
	 process might still be running with increased privileges and that
	 the secure memory has not been initialized.
	*/

	/*
	 Allocate a pool of 16k secure memory.  This makes the secure memory
	 available and also drops privileges where needed.  Note that by
	 using functions like gcry_xmalloc_secure and gcry_mpi_snew Libgcrypt
	 may expand the secure memory pool with memory which lacks the
	 property of not being swapped out to disk.
	*/
	gcry_control(GCRYCTL_INIT_SECMEM, 16384, 0);

	/*
	 It is now okay to let Libgcrypt complain when there was/is
	 a problem with the secure memory.
	*/
	gcry_control(GCRYCTL_RESUME_SECMEM_WARN);

	/*
	 ... If required, other initialization goes here.
	*/

	/*
	 Tell Libgcrypt that initialization has completed.
	*/
	gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);

	if (!gcry_control (GCRYCTL_INITIALIZATION_FINISHED_P)) {
		std::cerr << "libgcrypt has not been initialized" << std::endl;
		abort ();
	}

	bool force = false;

	//First of all, parse the arguments.
	std::vector<std::string> args;
	int i = 1; //Escape binary path
	while (i < argc)  {
		std::string arg(argv[i]);
		if (arg == "-f" || arg == "--force") {
			std::cout << "Using forcing mode." << std::endl;
			force = true;
		} else {
			args.push_back(arg);
		}
		i++;
	}
	const int c = args.size();

	/*
	 * What are the possible commands?
	 *
	 * [password] create [file]
	 * [password] [file]       list
	 * [password] [file]       list   [section]
	 * [password] [file]        add   [section]
	 * [password] [file]        add   [section]            [title]        ([value])
	 * [password] [file]        set   [section]            [title]          [value]
	 * [password] [file]     rename   [section] [new_section_name]
	 * [password] [file]     rename   [section]            [title] [new_title_name]
	 * [password] [file]     remove   [section]
	 * [password] [file]     remove   [section]            [title]
	 * [password] [file]       show   [section]            [title]
	 * [password] [file]     change  [password]
	 * crypt      [salt] [password]      [data]
	 * decrypt    [salt] [password]      [data]
	 * [password] [file]  print
	 *      print [file]
	 *     --help
	 *  --version
	 *     --info
	 *
	 * Ordered by length:
	 *
	 * (1)
	 *     --help
	 *    --infos
	 * 	--version
	 * (2)
	 *      print [file]
	 * (3)
	 * [password] create     [file]
	 * [password] [file]      print
	 * [password] [file]       list
	 * (4)
	 * crypt      [salt] [password]      [data]
	 * decrypt    [salt] [password]      [data]
	 * [password] [file]       list   [section]
	 * [password] [file]     change  [password]
	 * [password] [file]        add   [section]
	 * [password] [file]     remove   [section]
	 * (5)
	 * [password] [file]       show   [section]            [title]
	 * [password] [file]     rename   [section] [new_section_name]
	 * [password] [file]     remove   [section]            [title]
	 * (5-6)
	 * [password] [file]        add   [section]            [title]        ([value])
	 * (6)
	 * [password] [file]        set   [section]            [title]          [value]
	 * [password] [file]     rename   [section]            [title] [new_title_name]
	 */

	//We have to run the Cypheria instance one time at least, or it will generate a wrong result on the first use.
	//[TODO] Fix that thing, obviously linked to libgcrypt and Cypheria::crypt.

	bool ok = false;

	switch(c) {
		case 0:{
			QApplication app(argc, argv);

			LoginWindow win;
			win.show();

			return app.exec();
		}
		case 1: {
			if (args[0] == "--help" || args[0] == "-h") {
				ok = true;
				std::cout << helpText << std::endl;
			} else if (args[0] == "--version" || args[0] == "-v") {
				ok = true;
				std::cout << "Version 1.0.0" << std::endl;
			} else if (args[0] == "--infos" || args[0] == "-i") {
				ok = true;
				std::cout << "Version 1.0.0\n"
						  << "Required minimum libgcrypt version: " << NEED_LIBGCRYPT_VERSION << "\n"
						  << "Encryption Algorithm: Serpent, 256\n"
						  << "Key length: " << gcry_cipher_get_algo_keylen(GCRY_CIPHER) << "\n"
						  << "Block size: " << gcry_cipher_get_algo_blklen(GCRY_CIPHER) << "\n"
						  << "Mode: ECB\n"
						  << "Salt size: 16\n"
						  << "Embeds autofill: yes\n"
						  << "Has UI mode: no" << std::endl;
			}
			break;
		}
		case 2: {
			if (args[0] == "print") {
				ok = true;
				loadFromFile(args[1]).print();
			}
			break;
		}
		case 3: {
			if (args[1] == "create") {
				ok = true;

				Document doc;
				doc.header.salt = gen_random(16);

				Cypheria cyr;
				cyr.setSalt(doc.header.salt);
				cyr.setKey(args[0]);

				Storage storage;
				storage.setCypher(&cyr);
				storage.loadFrom(&doc);
				doc = storage.exportContent();

				_cyr = &cyr;
				doc.print(&decrypt);

				putToFile(args[2], doc);

			} else if (args[2] == "print") {
				ok = true;

				Document doc = loadFromFile(args[1]);

				Cypheria cyr;
				cyr.setSalt(doc.header.salt);
				cyr.setKey(args[0]);
				cyr.isSame("", "");

				if (cyr.isSame(args[0], doc.header.sample) || force) {
					_cyr = &cyr;
					doc.print(&decrypt);
				} else {
					PSWD_ERR
				}
			} else if (args[2] == "list") {
				ok = true;

				Document doc = loadFromFile(args[1]);

				Cypheria cyr;
				cyr.setSalt(doc.header.salt);
				cyr.setKey(args[0]);

				Storage storage;
				storage.setCypher(&cyr);
				storage.loadFrom(&doc);

				if (cyr.isSame(args[0], doc.header.sample) || force) {
					std::list<std::string> l = storage.getContainersNames();
					for (auto v : l) {
						std::cout << v << std::endl;
					}
				} else {
					PSWD_ERR
				}
			}
			break;
		}
		case 4: {
			if (args[2] == "add") {
				ok = true;

				Document doc = loadFromFile(args[1]);

				Cypheria cyr;
				cyr.setSalt(doc.header.salt);
				cyr.setKey(args[0]);

				Storage storage;
				storage.setCypher(&cyr);
				storage.loadFrom(&doc);
				if (cyr.isSame(args[0], doc.header.sample) || force) {
					if (storage.addContainer(args[3])) {
						doc = storage.exportContent();
						putToFile(args[1], doc);
					} else {
						std::cout << "Error: Failed to add container." << std::endl;
					}
				} else {
					PSWD_ERR
				}
			} else if (args[2] == "change") {
				ok = true;

				Document doc = loadFromFile(args[1]);

				Cypheria cyr;
				cyr.setSalt(doc.header.salt);
				cyr.setKey(args[0]);

				Cypheria n;
				n.setSalt(doc.header.salt);
				n.setKey(args[3]);

				Storage storage;
				storage.setCypher(&cyr);
				storage.loadFrom(&doc);
				if (cyr.isSame(args[0], doc.header.sample) || force) {
					if (storage.switchAll(&n)) {
						doc = storage.exportContent();
						putToFile(args[1], doc);
					} else {
						std::cout << "Error: Failed to change the password." << std::endl;
					}
				} else {
					PSWD_ERR
				}

			} else if (args[2] == "remove") {
				ok = true;

				Document doc = loadFromFile(args[1]);

				Cypheria cyr;
				cyr.setSalt(doc.header.salt);
				cyr.setKey(args[0]);

				Storage storage;
				storage.setCypher(&cyr);
				storage.loadFrom(&doc);
				if (cyr.isSame(args[0], doc.header.sample) || force) {
					if (!storage.removeContainer(args[3])) {
						doc = storage.exportContent();
						putToFile(args[1], doc);
					} else {
						std::cout << "Error: Failed to remove container." << std::endl;
					}
				} else {
					PSWD_ERR
				}

			} else if (args[2] == "list") {
				ok = true;

				Document doc = loadFromFile(args[1]);

				Cypheria cyr;
				cyr.setSalt(doc.header.salt);
				cyr.setKey(args[0]);

				Storage storage;
				storage.setCypher(&cyr);
				storage.loadFrom(&doc);

				if (cyr.isSame(args[0], doc.header.sample) || force) {
					std::list<std::string> l = storage.getFieldsNames(argv[3]);
					for (auto v : l) {
						std::cout << v << std::endl;
					}
				} else {
					PSWD_ERR
				}
			}
			break;
		}
		case 5: {
			if (args[2] == "show") {
				ok = true;

				Document doc = loadFromFile(args[1]);

				Cypheria cyr;
				cyr.setSalt(doc.header.salt);
				cyr.setKey(args[0]);

				Storage storage;
				storage.setCypher(&cyr);
				storage.loadFrom(&doc);
				if (cyr.isSame(args[0], doc.header.sample) || force) {
					std::cout << storage.value(args[3], args[4]) << std::endl;
				} else {
					PSWD_ERR
				}

			} else if (args[2] == "remove") {
				ok = true;

				Document doc = loadFromFile(args[1]);

				Cypheria cyr;
				cyr.setSalt(doc.header.salt);
				cyr.setKey(args[0]);

				Storage storage;
				storage.setCypher(&cyr);
				storage.loadFrom(&doc);
				if (cyr.isSame(args[0], doc.header.sample) || force) {
					if (storage.removeField(args[3], args[4])) {
						doc = storage.exportContent();
						putToFile(args[1], doc);
					} else {
						std::cout << "Error: Failed to remove the field." << std::endl;
					}
				} else {
					PSWD_ERR
				}

			} else if (args[2] == "rename") {
				ok = true;

				Document doc = loadFromFile(args[1]);

				Cypheria cyr;
				cyr.setSalt(doc.header.salt);
				cyr.setKey(args[0]);

				Storage storage;
				storage.setCypher(&cyr);
				storage.loadFrom(&doc);
				if (cyr.isSame(args[0], doc.header.sample) || force) {
					if (storage.changeContainerName(args[3], args[4])) {
						doc = storage.exportContent();
						putToFile(args[1], doc);
					} else {
						std::cout << "Error: Failed to rename container." << std::endl;
					}
				} else {
					PSWD_ERR
				}

			} else if (args[2] == "add") {
				ok = true;

				Document doc = loadFromFile(args[1]);

				Cypheria cyr;
				cyr.setSalt(doc.header.salt);
				cyr.setKey(args[0]);

				Storage storage;
				storage.setCypher(&cyr);
				storage.loadFrom(&doc);
				if (cyr.isSame(args[0], doc.header.sample) || force) {
					if (storage.addField(args[3], args[4])) {
						doc = storage.exportContent();
						_cyr = &cyr;
						doc.print(&decrypt);
						putToFile(args[1], doc);
					} else {
						std::cout << "Error: Failed to add field." << std::endl;
					}
				} else {
					PSWD_ERR
				}
			}
			break;
		}
		case 6: {
			if (args[2] == "add") {
				ok = true;

				Document doc = loadFromFile(args[1]);

				Cypheria cyr;
				cyr.setSalt(doc.header.salt);
				cyr.setKey(args[0]);

				Storage storage;
				storage.setCypher(&cyr);
				storage.loadFrom(&doc);
				if (cyr.isSame(args[0], doc.header.sample) || force) {
					if (storage.addField(args[3], args[4], args[5])) {
						doc = storage.exportContent();
						putToFile(args[1], doc);
					} else {
						std::cout << "Error: Failed to add field." << std::endl;
					}
				} else {
					PSWD_ERR
				}

			} else if (args[2] == "set") {
				ok = true;

				Document doc = loadFromFile(args[1]);

				Cypheria cyr;
				cyr.setSalt(doc.header.salt);
				cyr.setKey(args[0]);

				Storage storage;
				storage.setCypher(&cyr);
				storage.loadFrom(&doc);
				if (cyr.isSame(args[0], doc.header.sample) || force) {
					if (storage.changeFieldValue(args[3], args[4], args[5])) {
						doc = storage.exportContent();
						putToFile(args[1], doc);
					} else {
						std::cout << "Error: Failed to change the filed's value." << std::endl;
					}
				} else {
					PSWD_ERR
				}

			} else if (args[2] == "rename") {
				ok = true;

				Document doc = loadFromFile(args[1]);

				Cypheria cyr;
				cyr.setSalt(doc.header.salt);
				cyr.setKey(args[0]);

				Storage storage;
				storage.setCypher(&cyr);
				storage.loadFrom(&doc);
				if (cyr.isSame(args[0], doc.header.sample) || force) {
					if (storage.changeFieldName(args[3], args[4], args[5])) {
						doc = storage.exportContent();
						putToFile(args[1], doc);
					} else {
						std::cout << "Error: Failed to rename field." << std::endl;
					}
				} else {
					PSWD_ERR
				}
			}
			break;
		}
	}

	if (!ok) {
		std::cout << "The arguents are invalid. See the help by using --help" << std::endl;
	}

	std::cout << std::flush;
	std::cerr << std::flush;

	 return 0;
}

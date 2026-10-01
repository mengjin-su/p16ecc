#ifndef _PATH_H
#define _PATH_H

class Path {
	private:
		std::string pathBuf;
		bool ok;

	public:
		Path(char *env[]);
		char *get() { return ok? (char*)pathBuf.c_str(): NULL; }
};

extern bool windowsOS;

#endif

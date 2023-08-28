#ifndef _PATH_H
#define _PATH_H

class Path {
	private:
		char *pathBuf;

	public:
		Path(char *env[]);
		~Path();
		char *get() { return pathBuf; }
};

extern bool windowsOS;

#endif

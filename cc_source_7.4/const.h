#ifndef _CONST_H
#define _CONST_H



class Const_t
{
	public:
		char	*str;
		int		index;
		Const_t *next;

	public:
		Const_t(char *s, int indx);
		~Const_t();
		char *strName(void);
};


class Const {

	public:
		Const_t *list;
		int 	index;

	public:
		Const();
		~Const();

		char *add(char *str);
};

#endif
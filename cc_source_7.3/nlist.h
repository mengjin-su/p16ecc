#ifndef _NLIST_H
#define _NLIST_H
#include "common.h"

class Nnode {
	public:
		Nnode(int _type, char *_name=NULL, node *np1=NULL, node *np2=NULL);
		~Nnode();
	
		int     type;			// Nnode 类别 (DEFINE/ENUM)
		char   *name;			// Nnode 符号名
		node   *np[2];			// 符号名定义的参数
		attrib *attr;			// 特征/类型
		Nnode  *next;			// 同层级Nnode链接
		Nnode  *parent;			// 更外一层Nnode链接
		int 	nops(void);		// 有效np[]个数（0~2）
};

class Nlist {
	private:
		Nnode *list;
		Nnode *find(char *name, int type);

	public:
		Nlist ();
		~Nlist();
		void    addLayer(void);
		void    delLayer(void);
		Nnode  *add(char *name, int type, node *np1=NULL, node *np2=NULL);
		Nnode  *add(Nnode *p);
		void    del(char *name);
		Nnode  *search(char *name, int type=0);
		attrib *search(attrib *attr);
};

#endif
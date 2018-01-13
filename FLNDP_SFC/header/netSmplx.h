#pragma once
//实现一个 网络单纯形接口 及 实现
//参考一个python版本的Net simplex改写的
#include <math.h>
#include <algorithm>
#include <vector>
#include <deque>
#include <string.h>
#include <stdarg.h>
//#include "load_solve.h"
//#include "on_off.h"


#define f_alloc(n, type) ((type *)malloc((n)* sizeof(type)))//快速分配内存
static inline void free_all(int num, ...)
{
	va_list var_arg;
	va_start(var_arg, num);
	for (int i = 0; i < num; ++i) {
		free(va_arg(var_arg, void*));
	}
	va_end(var_arg);
}

//using index N as virtual node
#define _1 n
#define None -1
#define __MAXL 2001
#define  __MID __MAXL/2
struct edg {//点数+原边数 个
	int s, t, u, c;
};
//edge index begin from 0

class NetSmplx
{
public:
	using pint = int*;
	using num = int;
	using ID = int;
	using eind = int;
	using nind = int;
#define SV f_alloc(1+n,int)
	NetSmplx(edg* E, int e, int n, pint demand) :edges(E), e(e), n(n), 
		x(f_alloc(e + n + 1, int)), pi(SV), parent(SV), edge(SV),
		size(SV), next(SV), prev(SV), last(SV), demand(demand), 
		B(int(ceil(sqrt(e)))), M((e + B - 1) / B)
	{
#undef SV
		int tmp = 0;
		for (int i = 0; i < e; ++i) tmp += edges[i].u;
		faux_inf = tmp; tmp = 0;
		for (int i = 0; i < e; ++i)tmp += edges[i].c;
		if (tmp > faux_inf)faux_inf = tmp; tmp = 0;
		for (int i = 0; i < n; ++i)if ((tmp = abs(demand[i])) > faux_inf)faux_inf = tmp;
		faux_inf *= 3;
		int ne = e;
		auto insert = [this, &ne](ID s, ID t) {
			edges[ne].s = s;
			edges[ne].t = t;
			edges[ne].u = faux_inf;//u
			edges[ne].c = faux_inf;//c
			++ne;
		};

		for (nind j = 0; j < n; ++j) {
			//printf("%d ", j);
			demand[j] > 0 ?
				insert(_1, j) :
				insert(j, _1);
		}
		memset(x, 0, e * sizeof(int));//x clear for e
		for (nind j = 0; j < n; ++j)
		{
			x[j + e] = abs(demand[j]);//x set for e~ne
			pi[j] = demand[j] <= 0 ? faux_inf : -faux_inf;
			parent[j] = _1;
			edge[j] = e + j;
			size[j] = 1;
			next[j] = j + 1;//range(1, n)
			prev[j] = j - 1;//range(0, n-1)
			last[j] = j;
		}
		prev[0] = _1;//fix for prev[0]
		parent[_1] = None;
		edge[_1] = e + n;
		size[_1] = n + 1;
		next[_1] = 0;
		prev[_1] = n - 1;
		last[_1] = n - 1;
		clearWn();
	}
	~NetSmplx() {
		free_all(8, x, pi, parent, edge, size, next, prev, last);
	}
	//主循环
	void Pivot_loop();

private:
	//返回边i的reduced cost
	int reduced_cost(eind i);

	//找两个子节点的在生成树中的公共父节点
	int find_apex(nind p, nind q);
	
	//找包含由p到q ，id为i的边的环
	void find_cycle(eind i, nind p, nind q);

	//返回残余量
	int residual_capacity(eind i, nind p);

	//返回出基边
	void find_leaving_edge(eind & j, nind & s, nind & t);

	//在路径上增加流量f
	void augment_flow(int f);

	// 在生成树中删除边 edge[s,t]
	void remove_edge(nind s, nind t);

	//使节点q成为根节点
	void make_root(nind q);

	//q为子树的根节点，将i->p加入
	void add_edge(eind i, nind p, nind q);

	//更新潜在点
	void update_potentials(eind i, nind p, nind q);

	inline int find_(int &a, int &b) {
		for (auto ite = he; ite != te; ++ite) {
			if (*ite == a)return -1;
			if (*ite == b)return 1;
		}
		return 1;
	}

	edg *edges;//点数+原边数个//spaning tree
	const num e;//边数
	const num n;//点数
public:
	bool check_feasible() {
		for (int i = e; i < e + n; ++i) {
			if (x[i] != 0)return false;
		}
		return true;
	}
	pint x;
private:
	//j for node i for edge
	pint pi, parent, edge, size, next, prev, last, demand;
	//Const num
	const int B, M;
	int faux_inf;
	//假定最长100
	inline void clearWn() {
		he = te = (We + __MID);
		hn = tn = (Wn + __MID);
	}
	int We[__MAXL];
	int *he, *te;
	int Wn[__MAXL];
	int *hn, *tn;
	std::vector<int> ancestors;
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
};
#undef _1
#undef None

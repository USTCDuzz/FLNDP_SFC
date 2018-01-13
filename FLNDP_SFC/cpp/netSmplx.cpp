//实现一个 网络单纯形
//参考一个python版本的Net simplex改写的
#include <vector>
#include <climits>
#include "netSmplx.h"
using namespace std;

#define swap(a,b) auto tmp=(a);(a)=(b);(b)=(tmp)
#define _minus1 n
#define None -1

void NetSmplx::Pivot_loop()
{
	eind i = -1;
	nind p, q;
	eind j = -1;
	nind s = -1, t;
	int m = 0,f = 0;
	while (m < M)
	{
		int l = f + B;
		int c = INT_MAX, tmpc;
		if (l <= e)
		{
			for (int ii = f; ii < l; ++ii) {
				if ((tmpc = reduced_cost(ii)) < c) {
					c = tmpc;
					i = ii;
				}
			}
		}
		else {
			l -= e;
			for (int ii = f; ii < e; ++ii) {
				if ((tmpc = reduced_cost(ii)) < c) {
					c = tmpc;
					i = ii;
				}
			}
			for (int ii = 0; ii < l; ++ii) {
				if ((tmpc = reduced_cost(ii)) < c) {
					c = tmpc;
					i = ii;
				}
			}
		}
		f = l;
		if (c >= 0) { m++; }
		else {
			if (x[i] == 0) {
				p = edges[i].s;
				q = edges[i].t;
			}
			else {
				p = edges[i].t;
				q = edges[i].s;
			}
			//////////////////////////////yield/i/p/q////////////////////////////////
			clearWn();
			find_cycle(i, p, q);
			find_leaving_edge(j, s, t);
			augment_flow(residual_capacity(j, s));
			if (i != j)
			{
				if (parent[t] != s)
				{
					swap(s, t);
				}
				if (find_(i, j) > 0)
				{
					swap(p, q);
				}
				remove_edge(s, t);
				make_root(q);
				add_edge(i, p, q);
				update_potentials(i, p, q);
			}
			m = 0;
		}
	}
}

int NetSmplx::reduced_cost(eind i)
{
	int c = edges[i].c - pi[edges[i].s] + pi[edges[i].t];
	if (x[i] == 0)return c;
	else return -c;
}

int NetSmplx::find_apex(nind p, nind q)
{
	int size_p = size[p];
	int size_q = size[q];
	while (true)
	{
		while (size_p < size_q)
		{
			p = parent[p];
			size_p = size[p];
		}
		while (size_p > size_q)
		{
			q = parent[q];
			size_q = size[q];
		}
		if (size_p == size_q)
		{
			if (p != q)
			{
				p = parent[p];
				size_p = size[p];
				q = parent[q];
				size_q = size[q];
			}
			else
			{
				return p;
			}
		}
	}
}

void NetSmplx::find_cycle(eind i, nind p, nind q)
{
	int w = find_apex(p, q);
	*(--hn) = p;
	while (p != w) {
		*(--he) = edge[p];
		p = parent[p];
		*(--hn) = p;
	}
	*te++ = i;
	*tn++ = q;
	while (q != w) {
		*te++ = edge[q];
		q = parent[q];
		*tn++ = q;
	}
	--tn;
}
int NetSmplx::residual_capacity(eind i, nind p)
{
	return edges[i].s == p ? edges[i].u - x[i] : x[i];
}

void NetSmplx::find_leaving_edge(eind & j, nind & s, nind & t)
{
	int c = INT_MAX, tmpc;
	for (int *nit = hn, *eit = he; nit != tn; ++nit, ++eit)
	{
		if ((tmpc = residual_capacity(*eit, *nit)) < c)
		{
			j = *eit;
			s = *nit;
			c = tmpc;
		}
	}
	t = edges[j].s == s ? edges[j].t : edges[j].s;
}

void NetSmplx::augment_flow(int f)
{
	for (int *nit = hn, *eit = he; nit != tn; ++nit, ++eit)
	{
		int i = *eit;
		int p = *nit;
		x[i] += (edges[i].s == p ? f : -f);
	}
}
void NetSmplx::remove_edge(nind s, nind t)
{
	int size_t = size[t];
	int prev_t = prev[t];
	int last_t = last[t];
	int next_last_t = next[last_t];
	// Remove s,t
	parent[t] = None;
	edge[t] = None;
	// Remove the subtree rooted at t from the depth-first thread.
	next[prev_t] = next_last_t;
	prev[next_last_t] = prev_t;
	next[last_t] = t;
	prev[t] = last_t;
	//Update the subtree sizes and last descendants of the (old) acenstors of t.
	while (s != None)
	{
		size[s] -= size_t;
		if (last[s] == last_t)
		{
			last[s] = prev_t;
		}
		s = parent[s];
	}
}

void NetSmplx::make_root(nind q)
{
	ancestors.clear();
	while (q != None)
	{
		ancestors.emplace_back(q);
		q = parent[q];
	}
	for (unsigned i = ancestors.size() - 1; i > 0; --i)
	{
		int p = ancestors[i];
		int q = ancestors[i - 1];
		int size_p = size[p];
		int last_p = last[p];
		int prev_q = prev[q];
		int last_q = last[q];
		int next_last_q = next[last_q];
		parent[p] = q;
		parent[q] = None;
		edge[p] = edge[q];
		edge[q] = None;
		size[p] = size_p - size[q];
		size[q] = size_p;
		next[prev_q] = next_last_q;
		prev[next_last_q] = prev_q;
		next[last_q] = q;
		prev[q] = last_q;
		if (last_p == last_q)
		{
			last[p] = prev_q;
			last_p = prev_q;
		}
		prev[p] = last_q;
		next[last_q] = p;
		next[last_p] = q;
		prev[q] = last_p;
		last[q] = last_p;
	}
}

void NetSmplx::add_edge(eind i, nind p, nind q)
{
	int last_p = last[p];
	int next_last_p = next[last_p];
	int size_q = size[q];
	int last_q = last[q];
	parent[q] = p;
	edge[q] = i;
	next[last_p] = q;
	prev[q] = last_p;
	prev[next_last_p] = last_q;
	next[last_q] = next_last_p;
	while (p != None)
	{
		size[p] += size_q;
		if (last[p] == last_p)
		{
			last[p] = last_q;
		}
		p = parent[p];
	}
}

void NetSmplx::update_potentials(eind i, nind p, nind q)
{
	int d = 0;
	if (q == edges[i].t)
	{
		d = pi[p] - edges[i].c - pi[q];
	}
	else
	{
		d = pi[p] + edges[i].c - pi[q];
	}
	pi[q] += d;//yield p
	int l = last[q];
	while (q != l)
	{
		q = next[q];
		pi[q] += d;//yield p
	}
}

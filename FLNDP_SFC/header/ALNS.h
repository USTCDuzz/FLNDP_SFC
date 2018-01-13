#pragma once
#include <unordered_map>
#include <vector>
#include <string.h>
#include <stdarg.h>
#include "solver_main.h"
#include "netSmplx.h"
#define f_alloc(n, type) ((type *)malloc((n)* sizeof(type)))//快速分配内存

struct mh_Sol {//仅仅对服务器编码，0(不用)，1(level1)，……类推
	int length;//编码长度
	int M2;
	int N;
	// [(M2)-link][(?)code server][(N)for simplex]
	int* code;//+1
 	int* output;
 	int* costofset;
// 	int* codebetter;
// 	int* codebest;
	int* linkinfo;

	int netcost;
	int cost;
// 	int costbetter;
// 	int costbest;

	int outputability;
//	size_t hash_key;
//	std::unordered_map<size_t, int> hashtable;
	//staticdata staticd;
// 	inline void add2hashtable() {
// 		hashtable[hash_key] = cost;
// 	}
#define SV(X) X=f_alloc(length+1,int),memset(X+1,0,sizeof(int)*length)
	//////////////////////////////////////////////////////////////////////////
	mh_Sol(mainSolver &ms):pp(ms.pp),vv(ms.vv) {//construct
		length = ms.pp.siteNum;
		M2 = ms.pp.linkNum * 2;
		N = ms.pp.siteNum;
		SV(code),SV(output),SV(costofset);
#undef SV
		linkinfo = f_alloc(M2 + 1, int);
		outputability = cost = 0;
	//	const int tmp[3] = { 33,9,13 };
		//staticd.init(tmp, *this);
		//////init for netsimplex	
		auto &vl = ms.vl;
		auto &vv = ms.vv;
		es = f_alloc(1 + M2 + N*2,edg);
		for (int i = 1; i <= M2; ++i) {//前0~M2-1个边是固定的。
			es[i - 1] = edg{ vl[i].vertexi,vl[i].vertexj,vl[i].volume,vl[i].percost };
		}
// 		for (int i = 1; i <= M2; ++i) {//前0~M2-1个边是固定的。
// 			es[i - 1] = edg{ vl[i].vertexi,vl[i].vertexj,vl[i].volume,1 };
// 		}
		demand = f_alloc(1 + N,int);
		for (int i = 1; i <= N; ++i)
		{//设置demand
			if (vv[i].demand > 0)
				demand[i] = vv[i].demand;
			else demand[i] = 0;
		}
		demand[0] = -ms.pp.demandall;
	}
	~mh_Sol() {
		free_all(5,code,output,costofset, es, demand);
	}

// 
// 	//在当前解中寻找hash value
// 	bool findinhashtable(size_t _val) const {
// 		return hashtable.find(_val) != hashtable.end();
// 	}
// 
// 	//得到当前解的hash value
// 	inline size_t gethashvalue() {
// 		return (hash_key = gethashvalueConst());
// 	}
// 	inline size_t gethashvalueConst() const{
// 		constexpr size_t _FNV_offset_basis = 14695981039346656037ULL;
// 		constexpr size_t _FNV_prime = 1099511628211ULL;
// 		size_t _VAL = _FNV_offset_basis;
// 		for (int i = 1; i <= length; ++i) {
// 			_VAL ^= (size_t)code[i];
// 			_VAL *= _FNV_prime;
// 		}
// 		return _VAL;
// 	}
// 	inline void now2best()
// 	{
// 		memcpy(codebest, code, sizeof(int)*(length + 1));
// 		costbest = cost;
// 	}
// 	inline void now2better()
// 	{
// 		memcpy(codebetter, code, sizeof(int)*(length + 1));
// 		costbetter = cost;
// 	}
// 	inline void better2now()
// 	{
// 		memcpy(code, codebetter, sizeof(int)*(length + 1));
// 		cost = costbetter;
// 	}
// 	inline void best2now()
// 	{
// 		memcpy(code, codebest, sizeof(int)*(length + 1));
// 		cost = costbest;
// 	}
// 	void clearcode() {
// 		memset(code, 0, sizeof(int)*(length + 1));
// 	}
// 	int getCost();
// 	void UpdateSol();
// 	std::vector<int> ShawRemoval(unsigned q, double prob);
// 	std::vector<int> RandomRemoval(unsigned q, const double);
// 	std::vector<int> WorstRemoval(unsigned q, double prob) ;
// 	void RemoveD(std::vector<int>&);
// 	void Regret_K(std::vector<int>& D, int k);
// 	int getfitnessOnly(int * code)const;
// 	void getnowfitness() {
// 		cost = getfitness(code);
// 	}
	//////////////////////////////////////////////////////////////////////////
	// intface to netSimplex
	edg* es;
	int* demand;
	pp_& pp;
	std::vector<node>& vv;
	std::vector<int> map2vv;
	int getfitness(int *code);
	//////////////////////////////////////////////////////////////////////////
// 	void constructSolution();
// 	void recover();

};
#include <stdio.h>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <functional>
#include <fstream>
#include "solver_main.h"
#include "ALNS.h"
using namespace std;
//extern bool get11;
double mainSolver::get_max_lvl() {
	double t1 = nowtime();
	if (togetmax) {
		const int N = pp.siteNum;
		//const int M = pp.linkNum;
		push_demand();

		mh_Sol S(*this);
		auto &chuzlvl = max_lvl;
		//auto &mii = vfitness;
		chuzlvl.resize(1 + N);
		//mii.clear();
		//get11 = 1;
		for (int i = 1; i <= N; ++i) {
			S.code[i] = pp.Levelnum;
			S.getfitness(S.code);
			// 		mii.emplace_back(pair<double, int>{
			// 			S.outputability / double(S.netcost + vv[i].const_cost + pp.levelcost[S.code[i]]),
			// 				i
			// 		});
			chuzlvl[i] = S.code[i];
			S.code[i] = 0;
		}
		//get11 = 0;
		span_demand();
	}
	else {
		const int N = pp.siteNum;
		max_lvl.clear();
		max_lvl.resize(1 + N);
		for (int i = 1; i <= N; ++i) {
			auto it = std::upper_bound(pp.leveloutput + 1, pp.leveloutput + pp.Levelnum, vv[i].sumsupply - 1);
			max_lvl[i] = distance(pp.leveloutput, it);
		}
	}
	return nowtime() - t1;
}
/*

void mainSolver::FindInitialSolution(bool as_init)
{
	const int N = pp.siteNum;
	const int M = pp.linkNum;
	push_demand();

	mh_Sol S(*this);
	vector<int> chuzlvl;
	chuzlvl.resize(1 + N);
	vector<pair<double, int>> mii;
	get11 = 1;
	for (int i = 1; i <= N; ++i) {
		if (v_cand[i] != 0)
			S.code[i] = min(v_cand[i], pp.Levelnum);
		else
			S.code[i] = pp.Levelnum;
		S.getfitness(S.code);
		mii.emplace_back(pair<double, int>{
			S.outputability / double(S.netcost + vv[i].const_cost + pp.levelcost[S.code[i]]), i});
		chuzlvl[i] = S.code[i];
		S.code[i] = 0;
	}
	std::sort(mii.begin(), mii.end(), less<pair<double, int>>());
	get11 = 0;
	S.getnowfitness();//xia ya
	S.now2best();
// 	for (int i = 1; i <= N; ++i) {
// 		if (v_cand[i] == 0)chuzlvl[i] = 0;
// 	}
	for (int i = 0; i < N; ++i) {
		S.clearcode();
		for (auto &it : mii) {//insert the code
			if (it.second != 0)//没有被删掉
				S.code[it.second] = chuzlvl[it.second];
		}
		if (S.code[mii[i].second] == 0)continue;
		S.code[mii[i].second] = 0;//delete a
		S.getnowfitness();
		if (S.cost < S.costbest)
			S.now2best();
		if (S.cost == INT_MAX) {//不可行解
		}
		else {//可行解就删掉
			mii[i].second = 0;
		}
	}
	S.best2now();
	S.getnowfitness();
	initcost = S.cost;
	if (initanswer != nullptr)
		delete[] initanswer;
	initanswer = new int[1 + M + M + pp.Levelnum * N];//列数个
	memset(initanswer, 0, sizeof(int)*(1 + M + M + pp.Levelnum * N));
	for (int i = 1; i <= M * 2; ++i)
	{
		initanswer[i] = S.linkinfo[i];
	}
	for (int i = 1; i <= N; ++i) {
		if (S.code[i] != 0) {
			int lvl = S.code[i];
			int ind = M * 2 + (lvl - 1)*N + i;
			initanswer[ind] = 1;//初始解
		}
	}
	span_demand();
}*/
void mainSolver::push_demand()
{
	int link_num = (int)(vl.size());
	for (int i = 1 + pp.linkNum * 2; i < link_num; ++i) {//将消费节点的demand push到网点上
		vv[vl[i].vertexi].demand += vv[vl[i].vertexj].demand;
	}
}

void mainSolver::span_demand()
{
	int link_num = (int)(vl.size());
	for (int i = 1 + pp.linkNum * 2; i < link_num; ++i) {//恢复消费节点
		vv[vl[i].vertexi].demand -= vv[vl[i].vertexj].demand; //连在消费点上的网络点，demand减去
	}
}
void mainSolver::clearAnsinfo() {
	for (int i = 1; i <= pp.siteNum; ++i)
		vv[i].self_output = vv[i].server_level = 0;
	server_ind.clear();
}
using itL = std::list<link>::iterator;
using PLI = std::pair<itL, int>;
//恢复路径
std::vector<std::list<int>> mainSolver::x2solution(vector<int> &x) {
	const int M = pp.linkNum;
	typesol solution;
	std::list<link> validlink;
#ifdef __FanXiangQ
	for (int i = 1; i <= M; ++i) {
		int t = x[i];
		if (t) {
			validlink.push_back(vl[i + M]);
			validlink.back().canused = t;
		}
	}
	for (int i = M + 1; i <= 2 * M; ++i) {
		int t = x[i];
		if (t) {
			validlink.push_back(vl[i - M]);
			validlink.back().canused = t;
		}
	}
#else
	for (int i = 1; i <= 2 * M; ++i)
	{
		if (x[i]) {
			validlink.push_back(vl[i]);
			validlink.back().canused = x[i];
		}
	}
#endif // __FanXiangQ
	for (unsigned int i = 1 + pp.linkNum * 2; i < vl.size(); ++i) {//恢复消费节点
		validlink.push_back(vl[i]);//加入边
		validlink.back().canused = vv[vl[i].vertexj].demand;//加入边上的可用流量
	}
	for (auto it = validlink.begin(); it != validlink.end(); ++it)//求服务器的净输出 ，selfoutput，服务器的所有入度减去出度
	{
		if (vv[it->vertexi].server_level > 0) {
			vv[it->vertexi].self_output += it->canused;
		}
		if (vv[it->vertexj].server_level > 0) {
			vv[it->vertexj].self_output -= it->canused;
		}
	}
	decltype(vl) vlsuper(server_ind.size());//超级边
	int k = 0;
	vv[0].server_level = 100;//超级服务等级
	for (auto &sid : server_ind) {
		vlsuper[k] = link(-1, 0, sid, 10000, 0);//id = -1 from 0 to sid
		vlsuper[k].canused = vv[sid].self_output;
		validlink.push_back(vlsuper[k]);//加入超级边
		k++;
	}
	validlink.sort([this](link& l1, link& l2) {//将server排序到前面
		if ((vv[l1.vertexi].server_level) > (vv[l2.vertexi].server_level))
			return true;
		return false;
	});
	for (auto it = validlink.begin(); it != validlink.end(); ++it)
	{
		vv[it->vertexi].adjlist.push_back(it);//把迭代器放入adjlist里
	}
	vector<PLI> link_history;//记录用过的path
	while (!validlink.empty())//当边表不空
	{
		auto top = *validlink.begin();//取第一个，一定为服务器点，且流量未全部分配
		link_history.clear();
		list<int> elsol;//记录一条路
		int most_small_flow = 50001;
		int vid = top.vertexi;
		elsol.push_back(top.vertexi);//push 第一个点
		link_history.emplace_back(PLI{ validlink.begin() , top.vertexi });
		most_small_flow = min(top.canused, most_small_flow);//求最小流
		elsol.push_back(vid = top.vertexj);//push 第二个点,push一个点之前需要计算最小流 ， 记录用过的path
		while (!vv[vid].demand != 0) {//dfs 寻找一条路径中的点，直到该点demand不为0, 不为0说明不是消费点，就一定有出度，邻接表一定不为空
			auto &vlast = (*vv[vid].adjlist.back());//邻接表最后一个
			link_history.emplace_back(PLI{ vv[vid].adjlist.back() ,vid });
			most_small_flow = min(vlast.canused, most_small_flow);
			elsol.push_back(vid = vlast.vertexj);
		}
		elsol.push_back(most_small_flow);//加入这条路径流量
		elsol.push_back(vv[link_history[1].second].server_level);
		elsol.pop_front();
		for (auto &it : link_history) {
			auto &lk = *(it.first);
			lk.canused -= most_small_flow;//减去使用掉的流量
			if (lk.canused == 0) {
				validlink.erase(it.first);
				vv[it.second].adjlist.pop_back();
			}
		}
		solution.emplace_back(move(elsol));
	}
	return solution;
}
void mainSolver::Write_best_to_file(string fname) {
	ofstream ofs(fname);
	server_ind[1];
	for (auto &each : server_ind) {
		ofs << each << "\t" << vv[each].server_level << endl;
	}
	ofs.close();
}
void mainSolver::Write_to_file()
{
	const char *write_type = 1 ? "w" : "a";//1:覆盖写文件，0:追加写文件
	FILE *fp = fopen(pp.pfileinfo->result.c_str(), write_type);
	if (fp == NULL)
	{
		printf("Fail to open file %s, %s.\n", pp.pfileinfo->result.c_str(), strerror(errno));
	}
	if (solution.size()) {
		fprintf(fp, "%zu\n\n", solution.size());
		for (auto &itrt : solution)
		{
			auto end1 = prev(prev(itrt.end()));
			for (auto it = itrt.begin(); it != end1; ++it)
			{
				fprintf(fp, "%d ", vv[*it]._realid);
			}
			fprintf(fp, "%d ", *next(itrt.rbegin()));
			fprintf(fp, "%d\n", (*itrt.rbegin()) - 1);
		}
	}
	fclose(fp);
}
/*
bool mainSolver::Check_feasible(typesol &solution)
{
	bool checkpass = true;
	bool allcheckpass = true;
	map<pair<int, int>, int> mii;//两点间流过的流量
	map<int, int> mi2;//需求
	map<int, int> m3;//服务器输出能力
	set<int> sserver;
	//int servercost=0;
	int zcheck = 0;
	for (auto &it : solution)
	{
		sserver.insert(*it.begin());
		for (auto it2 = next(it.begin()); it2 != it.end(); ++it2)
		{
			mii[pair<int, int>{(*prev(it2)), *it2}] += *next(it.rbegin());//倒数第二个是流量
			if (vv[*it2].demand != 0) {//需求点
				mi2[*it2] += *next(it.rbegin());
				m3[*it.begin()] = m3[*it.begin()] + *next(it.rbegin());//服务器的输出能力++
				break;
			}
		}
	}
	{
		checkpass = 1;
		for (auto &it : sserver) {
			zcheck += vv[it].const_cost + pp.levelcost[vv[it].server_level];//固代 + 浮动代价
			printf("服务器真实id:%d 固定代价:%d 浮动代价:%d 输出等级:%d 输出能力:%d 实际输出:%d\n",
				it - 1, vv[it].const_cost, pp.levelcost[vv[it].server_level],
				vv[it].server_level - 1, pp.leveloutput[vv[it].server_level], m3[it]);
			checkpass &= (m3[it] <= pp.leveloutput[vv[it].server_level]);
		}
		printf("服务器总设立代价:%d\n", zcheck);
		if (!checkpass) {
			printf("\n*****************\nCheck Error ! \n服务器输出能力超过上限!\n*****************\n");
		}
		else {
			printf("服务器输出能力 check pass!\n");
		}
		checkpass = true;
		//zcheck += static_cast<int>(sserver.size()) * pp.levelcost[];//服务器代价
		for (unsigned int i = 1; i < vl.size(); ++i) {
			pair<int, int> pr(vl[i].vertexi, vl[i].vertexj);
			checkpass &= (mii[pr] <= vl[i].volume);
			zcheck += mii[pr] * vl[i].percost;//路径代价
		}
		if (zcheck != int(pp.z + 0.1)) {
			printf("real cost = %d, calc cost = %d\n", zcheck, int(pp.z + 0.1));
			printf("\n*****************\nCheck Error ! \ncost not match!\n*****************\n");
			return false;
		}
		else
			printf("所有成本 check pass!\n");

		if (!checkpass) {
			printf("\n*****************\nCheck Error ! \nbeyond volume!\n*****************\n");
			return false;
		}
		else
			printf("所有容量 check pass!\n");
		allcheckpass &= checkpass;
	}
	{
		checkpass = true;
		checkpass &= (unsigned int)(mii.size()) == (unsigned int)(pp.linkNum + pp.linkNum + pp.customerNum);
		if (!checkpass) {
			printf("\n*****************\nCheck Error ! \nlink not exist!\n*****************\n");
			return false;
		}
		else
			printf("链路存在性 check pass!\n");
		allcheckpass &= checkpass;
	}
	{
		checkpass = true;
		for (int i = 1 + pp.siteNum; i <= pp.siteNum + pp.customerNum; ++i) {
			checkpass &= mi2[i] == vv[i].demand;
		}
		if (!checkpass) {
			printf("\n*****************\nCheck Error ! \ndemand not match!\n*****************\n");
			return false;
		}
		else
			printf("需求满足 check pass!\n");
		allcheckpass &= checkpass;
	}
	if (allcheckpass)
	{
		printf("check all pass!\n");
	}
	return allcheckpass;
}*/
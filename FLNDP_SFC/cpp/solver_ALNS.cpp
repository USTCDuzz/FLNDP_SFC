#include <algorithm>
//#include "tools.h"
#include <climits>
#include "ALNS.h"
using namespace std;
#ifdef _MSC_VER1

int mh_Sol::getCost()
{
	return cost;
}
void mh_Sol::UpdateSol()
{
}
using pid = std::pair< double,int>;//vv中,code中的id + 效率
std::vector<int> mh_Sol::ShawRemoval(unsigned q, double prob)
{//随机选一个已经是服务器的点，将其降档，然后在他周围选一个候选集,转移操作
	//随机时考虑服务器的效率
	//效率定义为 输出量/设立代价
	static vector<pid> L(200);
	L.clear();
	//int* lvlc = pp.levelcost;
	//int* lvlo = pp.leveloutput;

	for (int i = 1; i <= length; ++i) {//当前使用的服务器 
		if (code[i] > 0) {
			L.emplace_back(pid{double(output[i])/costofset[i],i});
		}
	}
	int rdnum = int(pow(double(rand() - 1) / RAND_MAX, prob) * L.size());
	nth_element(L.begin(), L.begin() + rdnum, L.end());
	int chuzind = L[rdnum].second;//选择了一个将转移的ind
	_Assert(code[chuzind] > 0);
	return std::vector<int>{chuzind};
}

std::vector<int> mh_Sol::RandomRemoval(unsigned q, const double)
{
	//随机选择一个
	static vector<int> L(200);
	L.clear();
	for (int i = 1; i <= length; ++i) {//当前使用的服务器 
		if (code[i] > 0) {
			L.emplace_back(i);
		}
	}
	int chuzind = L[rand() % L.size()];
	return std::vector<int>{chuzind};
}

std::vector<int> mh_Sol::WorstRemoval(unsigned q, double prob)
{
	static vector<pid> L(200);
	L.clear();
	for (int i = 1; i <= length; ++i) {//当前使用的服务器 
		if (code[i] > 0) {
			L.emplace_back(pid{ double(output[i]) / costofset[i],i });
		}
	}
	nth_element(L.begin(), L.begin()+0, L.end());//最小的
	int chuzind = L[0].second;//选择了一个将转移的ind
	_Assert(code[chuzind] > 0);
	return std::vector<int>{chuzind};
}

void mh_Sol::RemoveD(std::vector<int>&D)
{
// 	int orilvl;
// 	for (auto &it : D) {//移除每一个D里的输出能力
// 		orilvl = code[it];//原始等级
// 		code[it]--;//降级
// 		outputability -= (pp.leveloutput[orilvl]-pp.leveloutput[orilvl-1]);//消减输出能力
// 		for (auto &adj : vv[it].adjvector) {//在邻接表里选一个插入
// 
// 		}
// 	}
}
using  pii = pair<int, int>;
void mh_Sol::Regret_K(std::vector<int>& D, int k)
{//k暂定为增加的个数
	int orilvl;
	unsigned int costbyreduce;
	for (auto &it : D) {//移除每一个D里的输出能力
		orilvl = code[it];//原始等级
		code[it]--;//降级
		outputability -= (pp.leveloutput[orilvl] - pp.leveloutput[orilvl - 1]);//消减输出能力
	}
	if (outputability > pp.demandall) {
		//printf("still enough\n");
		costbyreduce = getfitnessOnly(code);//消减过后的值
	}
	else {
		//printf("not enough\n");
		costbyreduce = INT_MAX;
	}
	//int aftercost = INT_MAX;
	static vector<pii> costv(10);
	costv.clear();
	for (auto &it : D) {//暂时只有一个
		for (auto &adj : vv[it].adjvector) {//在邻接表里选一个插入
			orilvl = code[adj];
			code[adj]++;
			if (code[adj] > pp.Levelnum) { 
				code[adj] = pp.Levelnum; //不变
				costv.push_back(pii{ costbyreduce,adj });
			}
			else {
				int c = getfitnessOnly(code);
				costv.push_back(pii{ c,adj });
				//auto h = gethashvalueConst();
				//hashtable[h] = c;//中间解也可以放入hashtable
			}
			code[adj] = orilvl;//恢复code
		}
	}
	auto it = min_element(costv.begin(), costv.end());
	unsigned min1 = (*it).first;
	if (costbyreduce < min1 && costbyreduce!= INT_MAX) {
		min1 = costbyreduce;//costbyreduce更好
		//printf("reduce is better!");//code 已经减过了
		return;
	}
	//此时costbyreduce更差
	if (int(min1) < cost) {
		code[(*it).second]++;
		if (code[(*it).second] > pp.Levelnum) {
			code[(*it).second] = pp.Levelnum; //不变
		}
		getfitness(code);
		return;
	}
	else {
		code[*(D.begin())]++;
		getfitness(code);
		return;
	}
// 	if (min1 < cost) {
// 		printf("accept! %d", *it);
// 	}
// 	const int nowcost = cost;//目前的成本
}
#endif // _MSC_VER
/*
int mh_Sol::getfitnessOnly(int* code) const{
	int ha = gethashvalueConst();
	if (findinhashtable(ha)) {
		return 	hashtable.at(hash_key);
	}
	int cost1 = 0;

	const int nodenum = 1 + pp.siteNum;
	int* lvlc = pp.levelcost;
	int* lvlo = pp.leveloutput;
	int servernum = 0;
	for (int i = 1; i <= length; ++i) {//超级源到每个服务器的边 
		if (code[i] > 0) {
			es[servernum + M2] = edg{ 0,i,lvlo[code[i]], 0 };
			cost1 += lvlc[code[i]] + vv[i].const_cost;//服务器成本
			servernum++;
		}
	}
	int ednum = M2 + servernum;
	NetSmplx NSolver(es, ednum, nodenum, demand);
	NSolver.Pivot_loop();
	bool feas = NSolver.check_feasible();
	if (!feas) {
		return INT_MAX;
	}
	for (int i = 0; i < ednum; ++i) {
		cost1 += es[i].c * NSolver.x[i];
	}
	return cost1;
}*/
//bool get11 = 0;
int mh_Sol::getfitness(int * code)
{
	outputability = 0;
	netcost = 0;
	memset(linkinfo, 0, (M2 + 1) * sizeof(int));
	//memset(output + 1, 0, length * sizeof(int));//output clear
	//memset(costofset + 1, 0, length * sizeof(int));//服务器设立成本 clear
	int costtmp = 0;
	const int nodenum = 1 + pp.siteNum;//super source
	map2vv.clear();
	int* lvlc = pp.levelcost;
	int* lvlo = pp.leveloutput;
	int servernum = 0;
	for (int i = 1; i <= length; ++i) {//超级源到每个服务器的边 
		if (code[i] > 0) {
			es[servernum + M2] = edg{ 0,i,lvlo[code[i]], 0 };//服务能力即档次，代价为0
			servernum++;
			map2vv.push_back(i);
		}
	}
	int ednum = M2 + servernum;
	NetSmplx NSolver(es, ednum, nodenum, demand);
	//	double begin_ = nowtime();
	NSolver.Pivot_loop();
	//	double nowt = nowtime() - begin_;
	//printf("%f \n", nowt);
	//x[0~M2-1]边上流量，x[M2~code[i]不为0的个数]:虚拟边上的流量
	//if (get11) {
		outputability = pp.demandall - NSolver.x[ednum];
		//printf("ccc = %d demandall = %d 输出能力%d  ", NSolver.x[ednum], pp.demandall, outputability);
	//}
	//NSolver.x[ednum];
	bool feas = NSolver.check_feasible();//feas = true feasible
	if (!feas) {
		cost = INT_MAX;
		//if (get11) {
			for (int i = 0; i < ednum; ++i) {
				netcost += es[i].c * NSolver.x[i];
			}
			int lvl = 1;
			int id = map2vv[0];
			//printf("id = %d ", id);
			while (outputability > lvlo[lvl]) { lvl++; }//降低输出量
			code[id] = lvl;
			//printf("lvl %d \n", code[id]);
		//}
		//gethashvalue();
		//add2hashtable();
		return cost;
	}
	for (int i = 0; i < ednum; ++i) {
		costtmp += es[i].c * NSolver.x[i];
	}
	for (int i = 0; i < M2; ++i) {
		linkinfo[i + 1] = NSolver.x[i];
	}
	int *x = NSolver.x;
	int lvl, inds, indvv;
	for (int i = M2; i < M2 + servernum; ++i)//inner i
	{
		inds = i - M2;
		indvv = map2vv[inds];
		if (x[i] > 0) {//使用了这个备选服务器
			lvl = 1;
			while (x[i] > lvlo[lvl]) { lvl++; }//降低输出量
			code[indvv] = lvl;//shrink_to_fit
			output[indvv] = x[i];
			outputability += lvlo[lvl];//统计总输出能力
			costofset[indvv] = lvlc[lvl] + vv[indvv].const_cost;
			costtmp += costofset[indvv];//加上服务器代价及固定成本
		}
		else {
			code[map2vv[inds]] = 0;//不使用了
		}
	}
	cost = costtmp;
	return cost;
}
/*
int mh_Sol::getfitness(int * code)
{
	outputability = 0;
	netcost = 0;
	memset(linkinfo, 0, (M2 + 1) * sizeof(int));
	memset(output + 1, 0, length * sizeof(int));//output clear
	memset(costofset + 1, 0, length * sizeof(int));//服务器设立成本 clear
	int cost1 = 0;
	const int nodenum = 1 + pp.siteNum;//super source
	map2vv.clear();
	int* lvlc = pp.levelcost;
	int* lvlo = pp.leveloutput;
	int servernum = 0;
	for (int i = 1; i <= length; ++i) {//超级源到每个服务器的边 
		if (code[i] > 0){
			es[servernum + M2] = edg{ 0,i,lvlo[code[i]], 0 };//服务能力即档次，代价为0
			servernum++;
			map2vv.push_back(i);
		}
	}
	int ednum = M2+servernum;
	NetSmplx NSolver(es, ednum, nodenum, demand);
//	double begin_ = nowtime();
	NSolver.Pivot_loop();
//	double nowt = nowtime() - begin_;
	//printf("%f \n", nowt);
	//x[0~M2-1]边上流量，x[M2~code[i]不为0的个数]:虚拟边上的流量
	if (get11) {
		outputability = pp.demandall - NSolver.x[ednum];
		//printf("ccc = %d demandall = %d 输出能力%d  ", NSolver.x[ednum], pp.demandall, outputability);
	}
	//NSolver.x[ednum];
	bool feas = NSolver.check_feasible();//feas = true feasible
	if (!feas) {
		cost = INT_MAX;
		if(get11){
			for (int i = 0; i < ednum; ++i) {
				netcost += es[i].c * NSolver.x[i];
			}
			int lvl = 1;
			int id = map2vv[0];
			//printf("id = %d ", id);
			while (outputability > lvlo[lvl]) { lvl++; }//降低输出量
			code[id] = lvl;
			//printf("lvl %d \n", code[id]);
		}
		gethashvalue();
		add2hashtable();
		return cost;
	}
	for (int i = 0; i < ednum; ++i) {
		cost1 += es[i].c * NSolver.x[i];
	}
	for (int i = 0; i < M2; ++i) {
		linkinfo[i + 1] = NSolver.x[i];
	}
	int *x = NSolver.x;
	int lvl,inds,indvv;
	for (int i = M2;i<M2+servernum;++i)//inner i
	{
		inds = i - M2;
		indvv = map2vv[inds];
		if (x[i] > 0) {//使用了这个备选服务器
			lvl = 1;
			while (x[i] > lvlo[lvl]) { lvl++; }//降低输出量
			code[indvv] = lvl;//shrink
			output[indvv] = x[i];
			outputability += lvlo[lvl];//统计总输出能力
			costofset[indvv] = lvlc[lvl] + vv[indvv].const_cost;
			cost1 += costofset[indvv];//加上服务器代价及固定成本
		}
		else {
			code[map2vv[inds]] = 0;//不使用了
		}
	}
	//map2vv.clear();
	//map2vv.shrink_to_fit();
	//reduce output num to fit the output ability.
	cost = cost1;
	gethashvalue();
	add2hashtable();
	//printf("\ncost = %d\n", cost );
	return cost;
}*/
#ifdef _MSC_VER1



vector<vector<int>> init_lp;
typesol mainSolver::solver_ALNS()
{
	//const int M = pp.linkNum;//链接数
	int link_num = (int)(vl.size());
	for (int i = 1 + pp.linkNum * 2; i < link_num; ++i) {
		vv[vl[i].vertexi].demand += vv[vl[i].vertexj].demand;
	}//demand向前push
	vector<int> x;
	solveonce_ori(2, 1, 500, x);
	extern vector<int> vcand;

	mh_Sol S(*this);
	if (!init_lp.empty()) {
		for (int i = 1; i <= S.length; ++i) {
			S.code[i] = init_lp[0][i];
			//printf("%d ", init_lp[i]);
		}
	}
	else {
		for (int i = 1; i <= S.length; ++i) {
			if (rand() % 100 < 90)
				S.code[i] = pp.Levelnum;
		}
	}
	S.getnowfitness();
	S.now2best();
	S.now2better();
	/*********************
	others
	*********************/
	const int cumscore[3] = { 33,9,13 };
	double tlst = nowtime();
	int nowit = 1;
	//extern int g_cheatnum;
	//cheat ch(g_cheatnum=0);
	/*********************
	Cost about
	*********************/
	int mincost = S.getCost(), nowcost;
	int origincost = mincost;

	/*********************
	Simu Anneal about
	*********************/
	const int Iter = 8500,
		recoverIter = 50;
	double iniProb = 0.9;// , endProb = 0.001;
	const double iniT = (-origincost*0.05) / log(iniProb);
	double T = iniT;
	double rate = 0.95;

	/*********************
	Adaptive about
	*********************/
	//staticdata staticd;

	//staticd.solvetime = tlst;
	//staticd.veciter.reserve(Iter);
	printf("S origin = %d S length = %d\n", mincost, S.length);
	//S.dump(staticd, 0);
	while (nowit<Iter)
	{
		//S.stack_mv.clear();
		//printelm(nowit,d);
		if (!(nowit % 80)) {//100次
			S.staticd.update_weight();
			S.staticd.clear();
		}
		auto pmf = S.staticd.choosefunc();//choose a function
		int dsize = 1;
		int k = 1;
		//int k = S.staticd.chooseK();//choose a K
		//int dsize = S.staticd.chooseDsize();//min(100, int(0.4*S.od.size()));//
		auto D = (S.*pmf)(dsize, 5);//prob越大，随机性越小 5 6
									//S.RemoveD(D);
		S.Regret_K(D, k);//当前route长度与k的较小者。
		S.gethashvalue();
		//S.UpdateSol();
		// 		if (nowit > 100 && !(nowit % 2)) {
		// 			S.RouteReduce();
		// 			S.UpdateSol();
		// 		}
		nowcost = S.getCost();
		//printf("pdpnum = %d \n", S.current.pdp_num);
		//S.move2result();
		if (nowcost < mincost) {//必然不在hash_table中
			S.now2best();
			S.now2better();
			S.staticd.update_score(cumscore[0], pmf, k, dsize);
			S.add2hashtable();
			mincost = nowcost;
			printf("new best = %d, iter = %d \n", mincost, nowit);
			printf("time = %f \n", nowtime() - tlst);
		}
		if (S.costbetter > nowcost)//new solution cost better than before
		{//正常接受
			if (!S.findinhashtable(S.hash_key)) {//cant be found in hashtable
				S.staticd.update_score(cumscore[1], pmf, k, dsize);
				S.add2hashtable();
			}
		}
		else if (exp((S.costbetter - nowcost) / T) > double(rand()) / RAND_MAX)//nowcost更小，或者满足退火条件则接受新解，否则不接受
		{//退火接受
			if (!S.findinhashtable(S.hash_key)) {//cant be found in hashtable
				S.staticd.update_score(cumscore[2], pmf, k, dsize);
				S.add2hashtable();
			}
			S.now2better();
		}
		else {//不接受,从result移到Route
			  //S.recover();
			S.better2now();
		}
		T *= rate;

		nowit++;
		if (nowit % recoverIter == 0) {
			//S.dump(staticd, 0);
			//			if (0) {
			//			}
			//			//S.dump(staticd,0);
			//			S.findthebestcheat(ch);
			//if(nowit<2000)T = iniT;
			//rate = 0.95;
			//S.dump(staticd, 0);
			if (double(clock() - tlst) / CLOCKS_PER_SEC > 1200) {
				break;
			}
		}
	}
	S.staticd.solvetime = double(clock() - tlst) / CLOCKS_PER_SEC;
	//	printf("time = %d\n", time(0) - tlst);
	//	printf("question = %d, best = %d\n", g_index, mincost);
	return typesol();
}
#endif // _MSC_VER1

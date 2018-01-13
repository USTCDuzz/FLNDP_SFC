#include <string>
#include <fstream>
#include <iostream>
#include "solver_main.h"
#include "fileinfo.h"

using namespace std;
//#define A_test_parameters
//#define A_my
//#define A_origin
#define A_test_shoot
void deploy_server(fileinfo &fi)
{
#ifdef A_origin
	for (int togm = 0; togm <= 1; ++togm)
	{
		problem pb;
		pb.togetmax = togm;
		pb.pp.pfileinfo = &fi;
		if (pb.togetmax) {
			pb.path_rec = fi.origin_record_withmax;
			pb.path_sol = fi.origin_sol_withmax;
			pb.path_result = fi.result;
		}
		else {
			pb.path_rec = fi.origin_record;
			pb.path_sol = fi.origin_sol;
			pb.path_result = fi.result;
		}
		ifstream test(pb.path_sol);
		if (test.is_open()) {
			test.close(); continue;
		}
		cout << "Now is " << fi.folderName << " algo = origin " 
			<< string(togm ? string("to get max = 1") : string("to get max = 0")) << endl;
		double durat = pb.load_and_solve(fi.topo,4);

		ofstream logfile(pb.path_rec,ios::app);
		logfile << "alltime = " << durat << endl << "maxtime = " << pb.get_max_time << endl;
		logfile.close();
		pb.Write_to_file();
		pb.Write_best_to_file(pb.path_sol);
	}
#endif
#ifdef A_my
	for (int togm = 0; togm < 2; ++togm) {
		problem pb;
		pb.togetmax = togm;
		
		pb.pp.pfileinfo = &fi;
		if (pb.togetmax) {//1
			pb.path_rec = fi.my_record;
			pb.path_sol = fi.my_sol;
			pb.path_core_record = fi.my_core_record;
			pb.path_result = fi.result;
		}
		else {//0
			pb.path_rec = fi.my_record_nomax;
			pb.path_sol = fi.my_sol_nomax;
			pb.path_core_record = fi.my_core_record_nomax;
			pb.path_result = fi.result;
		}
// 		ifstream test(pb.path_sol);
// 		if (test.is_open()) {
// 			test.close(); continue;
// 		}
		cout << "Now is " << fi.folderName << " algo = my "
			<< string(pb.togetmax ? string("to get max = 1") : string("to get max = 0"))
			<< endl;
		double durat = pb.load_and_solve(fi.topo,1);

		ofstream logfile(pb.path_rec, ios::app);
		logfile << "alltime = " << durat << endl << "maxtime = " << pb.get_max_time << endl;
		logfile.close();
		pb.Write_to_file();
		pb.Write_best_to_file(pb.path_sol);
	}
#endif
#ifdef A_test_shoot
	for(int togm=0;togm<2;++togm){
		problem pb;
		pb.togetmax = togm;
		pb.pp.pfileinfo = &fi;
		if (pb.togetmax) {//1
			pb.path_rec = fi.my_record;
			pb.path_sol = fi.my_sol;
			pb.path_iter = fi.my_iter_rate;
			//pb.path_core_record = fi.my_core_record;
			pb.path_result = fi.result;
		}
		else {//0
			pb.path_rec = fi.my_record_nomax;
			pb.path_sol = fi.my_sol_nomax;
			pb.path_iter = fi.my_iter_rate_nomax;
			//pb.path_core_record = fi.my_core_record_nomax;
			pb.path_result = fi.result;
		}
		ifstream test(pb.path_iter);
		if (test.is_open()) {
			test.close(); continue;
		}
		cout << "Now is " << fi.folderName << " algo = test "
			<< string(pb.togetmax ? string("to get max = 1") : string("to get max = 0")) 
			<< endl;
		double durat = pb.load_and_solve(fi.topo, 5);
	}
#endif
#ifdef A_test_parameters
	for (int togm = 0; togm < 2; ++togm) {
		problem pb;
		pb.togetmax = togm;

		pb.pp.pfileinfo = &fi;
		if (pb.togetmax) {//1
			pb.path_rec = fi.my_record;
			pb.path_sol = fi.my_sol;
			pb.path_core_record = fi.my_core_record;
			pb.path_result = fi.result;
		}
		else {//0
			pb.path_rec = fi.my_record_nomax;
			pb.path_sol = fi.my_sol_nomax;
			pb.path_core_record = fi.my_core_record_nomax;
			pb.path_result = fi.result;
		}
		// 		ifstream test(pb.path_sol);
		// 		if (test.is_open()) {
		// 			test.close(); continue;
		// 		}
		cout << "Now is " << fi.folderName << " algo = my "
			<< string(pb.togetmax ? string("to get max = 1") : string("to get max = 0"))
			<< endl;
		double durat = pb.load_and_solve(fi.topo, 1);

		ofstream logfile(pb.path_rec, ios::app);
		logfile << "alltime = " << durat << endl << "maxtime = " << pb.get_max_time << endl;
		logfile.close();
		pb.Write_to_file();
		pb.Write_best_to_file(pb.path_sol);
	}
#endif
}
//bool on_get_cand;
double mainSolver::load_and_solve(string fname,int chuz)
{
	g_chuz = chuz;
	on_get_cand = 0;
	begintime = nowtime();
	//ofstream newfile("topo1.txt");
	ifstream infile(fname);
	if (!infile) { cout << "error open topo file" << endl; exit(1); }
	infile >> pp.siteNum >> pp.linkNum >> pp.customerNum >> pp.Levelnum;
	//newfile << pp.siteNum << " " << pp.linkNum << " " << pp.customerNum << " " << pp.Levelnum<<endl<<endl;
	vv.resize(1 + pp.siteNum + pp.customerNum);
	vl.resize(1 + pp.linkNum + pp.linkNum + pp.customerNum);//1 + 两倍网点数的链接 + customerNum

	for (int i = 1; i <= pp.Levelnum; ++i)
	{
		infile.ignore(256, ' ');
		infile >> pp.leveloutput[i] >> pp.levelcost[i];
		//newfile<<i-1<<" "<< pp.leveloutput[i]<<" "<< pp.levelcost[i]<<endl;
	}
	//newfile << endl;
	for (int i = 1; i <= pp.siteNum; ++i) {//读取消费节点设置服务器的成本，假设连续
		infile.ignore(256, ' ');
		infile >> vv[i].const_cost;
		//newfile << i - 1 <<" "<< vv[i].const_cost << endl;
		vv[i].id = i;
		vv[i]._realid = i - 1;
	}
	//newfile << endl;
	int a, b, c, d;
	for (int i = 1; i <= pp.linkNum; ++i) {//读取链接
		infile >> a >> b >> c >> d;
	//	newfile << a <<" "<< b <<" "<< c <<" "<< d<<endl;
		++a, ++b;
		vl[i] = link(i, a, b, c, d);
		vl[i + pp.linkNum] = link(i + pp.linkNum, b, a, c, d);
		vv[a].sumsupply += c;
		vv[b].sumsupply += c;
		vv[a].adjvector.push_back(b);
		vv[b].adjvector.push_back(a);
	}
//	newfile << endl;
	const int L2 = pp.linkNum * 2;
	const int sn = pp.siteNum;
	for (int i = 1; i <= pp.customerNum; ++i) {//读取消费节点
		infile >> a >> b >> c ;
		//newfile << a << " " << b << " " << c << endl;
		a += sn + 1, ++b;
		vl[i + L2] = link(i + L2, b, a, 100000, 0);
		vv[a].id = a;
		vv[a].demand = c;
		vv[a]._realid = i - 1;
		vv[b].sumsupply += c;
		vv[b].linkedCustomer = a;
		pp.demandall += c;
	}
	//newfile.close();
	get_max_time = get_max_lvl();

	int AlgoChuz = g_chuz;
	if (AlgoChuz == 1) {
		//double now = nowtime();
		get_cand();
		//printf("cand use time = %.2f", nowtime() - now);
		solution = solver_candidate_based();
	}
	else if (AlgoChuz == 2) {
		//三段candidate法
// 		get_cand();
// 		for (int i = 1; i <= 2; ++i) {
// 			on_get_cand = 1;
// 			solver_candidate_based();
// 			on_get_cand = 0;
// 		}
// 		solution = solver_candidate_based();
	}
	else if (AlgoChuz == 3) {//DE方法未完善
							 // 		get_cand();
							 // 		solver_DE_GA();// GA solver
	}
	else if (AlgoChuz == 4) {//原始方法
		solution = solver_origin_mip();
	}
	else if (AlgoChuz == 5) {
		get_cand();
	}
	fclose(stdin);
#ifdef _MSC_VER
	//Check_feasible(solution);
#endif // _MSC_VER
	return nowtime() - begintime;
}

void mainSolver::get_cand()
{
	on_get_cand = 1;
	push_demand();
	vector<int> x;
	solveonce_ori_GRB(-1, -1, 999999, x);
	span_demand();
	on_get_cand = 0;
}
void mainSolver::FindInitialSolution(bool as_init)
{


 	const int N = pp.siteNum;
 	const int M = pp.linkNum;
// 	push_demand();
// 
// 	mh_Sol S(*this);
// 	vector<int> chuzlvl;
// 	chuzlvl.resize(1 + N);
// 	vector<pair<double, int>> mii;
// 	get11 = 1;
// 	for (int i = 1; i <= N; ++i) {
// 		if (v_cand[i] != 0)
// 			S.code[i] = min(v_cand[i], pp.Levelnum);
// 		else
// 			S.code[i] = pp.Levelnum;
// 		S.getfitness(S.code);
// 		mii.emplace_back(pair<double, int>{
// 			S.outputability / double(S.netcost + vv[i].const_cost + pp.levelcost[S.code[i]]), i});
// 		chuzlvl[i] = S.code[i];
// 		S.code[i] = 0;
// 	}
// 	std::sort(mii.begin(), mii.end(), less<pair<double, int>>());
// 	get11 = 0;
// 	S.getnowfitness();//xia ya
// 	S.now2best();
// 	// 	for (int i = 1; i <= N; ++i) {
// 	// 		if (v_cand[i] == 0)chuzlvl[i] = 0;
// 	// 	}
// 	for (int i = 0; i < N; ++i) {
// 		S.clearcode();
// 		for (auto &it : mii) {//insert the code
// 			if (it.second != 0)//没有被删掉
// 				S.code[it.second] = chuzlvl[it.second];
// 		}
// 		if (S.code[mii[i].second] == 0)continue;
// 		S.code[mii[i].second] = 0;//delete a
// 		S.getnowfitness();
// 		if (S.cost < S.costbest)
// 			S.now2best();
// 		if (S.cost == INT_MAX) {//不可行解
// 		}
// 		else {//可行解就删掉
// 			mii[i].second = 0;
// 		}
// 	}
// 	S.best2now();
// 	S.getnowfitness();
//	initcost = S.cost;
	if (initanswer != nullptr)
		delete[] initanswer;
	initanswer = new int[1 + M + M + pp.Levelnum * N];//列数个
	memset(initanswer, 0, sizeof(int)*(1 + M + M + pp.Levelnum * N));
	copy(forinit.begin(), forinit.end(), initanswer);
// 	for (int i = 1; i <= M * 2; ++i)
// 	{
// 		initanswer[i] = S.linkinfo[i];
// 	}
// 	for (int i = 1; i <= N; ++i) {
// 		if (S.code[i] != 0) {
// 			int lvl = S.code[i];
// 			int ind = M * 2 + (lvl - 1)*N + i;
// 			initanswer[ind] = 1;//初始解
// 		}
// 	}
// 	span_demand();
}

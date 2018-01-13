#include <map>
#include <sstream>
#include<iomanip>
#include "solver_main.h"

#include "gurobi_c++.h"
#include "myCallback.h"
using namespace std;
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

int mainSolver::solveonce_static_GRB(int, int, int timelim, std::vector<int>& x) {
	const int N = pp.siteNum;//点数
	const int M = pp.linkNum;//链接数
	const int WholeCol = M + M + pp.Levelnum * N;
	//const int WholeRow = N + N + 1;
	stringstream ss;
	ss << path_rec;
	ofstream logfile(ss.str());
	if (!logfile.is_open()) {
		cout << "Cannot open s_t.txt for callback message" << endl;
		return 1;
	}
	logfile << setiosflags(ios::fixed)<<setprecision(4);

	x.reserve(WholeCol + 1);
	GRBEnv env=GRBEnv();
	GRBModel model(env);
	
	auto ArcVar = model.addVars(2 * M, GRB_CONTINUOUS);
	setLink(ArcVar);
	auto ServerVar = model.addVars(pp.Levelnum*N,GRB_BINARY);
	setServer(ServerVar);
	int Belif_range = 3;//置信范围
	for (int i = 1; i <= N; ++i)
	{
		int low = max(v_cand[i] - Belif_range, 0);
		int up = min(v_cand[i]+1,pp.Levelnum);
		if (v_cand[i] == 0)up = 0;
		//[low ~ up] were choosed
		for (int j = 1; j <= pp.Levelnum; ++j) {
			if (j >= low && j <= up) {//置信区间内，不fix
				continue;
			}
			if (initanswer != nullptr &&
				initanswer[M + M + N * (j - 1) + i] != 0){
				//printf("%d ", initanswer[M + M + N * (j - 1) + i]);
				continue;//如果是初始解，那么不fix这个档次的该服务器
			}
			ServerVar[N * (j - 1) + i - 1].set(GRB_DoubleAttr_UB, 0);
		}
	}
	//auto flowB = model.addConstrs(2 * N);

	//int elemnum = 0;
	for (int i = 1; i <= N; ++i) {
		GRBLinExpr flowi;
		for (int j = 0; j < M; ++j) {//流守恒
			if (vl[j+1].vertexi == i) {//i流出
				flowi -= ArcVar[j];
				flowi += ArcVar[M + j];
			}
			else if (vl[j+1].vertexj == i) {//流入i
				flowi += ArcVar[j];
				flowi -= ArcVar[M + j];
			}
		}
		for (int j = 1; j <= pp.Levelnum; ++j) {
			flowi += min(pp.leveloutput[j], vv[i].sumsupply)
				* ServerVar[(j - 1)*N + i - 1];
		}
		model.addConstr(flowi>=vv[i].demand);
	}
	for (int i = 0; i < N; ++i) {
		GRBLinExpr packi;
		for (int j = 0; j < pp.Levelnum; ++j) {
			packi += ServerVar[j*N + i];
		}
		model.addConstr(packi <= 1);
	}
	GRBLinExpr validinq;
	for (int j = 1; j <= pp.Levelnum; ++j) {
		int indp = 0;
		for (int i = (j - 1)*N ; i < j * N; i++) {
			indp++;
			validinq += ServerVar[i] * min(vv[indp].sumsupply, pp.leveloutput[j]);
		}
	}
	model.addConstr(validinq >= pp.demandall);

	model.set(GRB_DoubleParam_MIPGapAbs, 1);
	model.set(GRB_DoubleParam_MIPGap, 1e-10);
	
// 	int numvars = model.get(GRB_IntAttr_NumVars);
// 	auto vars = model.getVars();

	mycallback cb(pp.Levelnum*N, ServerVar, &logfile);
	cb.wholeVarnum = pp.Levelnum*N + 2 * M;
	model.setCallback(&cb);

	model.set(GRB_DoubleParam_TimeLimit, timelim/1000.0);
	model.set(GRB_IntParam_LogToConsole,0);
	model.set(GRB_IntParam_Threads, 1);
	model.optimize();

	printf("bound = %f\n", model.get(GRB_DoubleAttr_ObjBound));
	logfile << "bound" << "\t" << model.get(GRB_DoubleAttr_ObjBound) << endl;
	logfile << "all binary = " << cb.wholeVarnum << std::endl;
	logfile << "pre_coldel = " << cb.delcol << std::endl;
	logfile << "pre_rowdel = " << cb.delrow << std::endl;
	logfile << "time" << "\t" << model.get(GRB_DoubleAttr_Runtime) << endl;
	logfile.close();
	// Print solution
	cout << "\nTOTAL COSTS: " << model.get(GRB_DoubleAttr_ObjVal) << endl;
	//delete[]flowB;
	pp.z = model.get(GRB_DoubleAttr_ObjVal);
	printf("\n********\nz=%f\n", pp.z);
	//把解保存在x里
	x.push_back(0);//ind 0 no use
	for (int i = 0; i < 2*M; ++i) {
		x.push_back(static_cast<int>(ArcVar[i].get(GRB_DoubleAttr_X) + 0.1));
	}
	for (int i = 0; i < N*pp.Levelnum; ++i) {
		x.push_back(static_cast<int>(ServerVar[i].get(GRB_DoubleAttr_X) + 0.1));
	}
	clearAnsinfo();
	for (int j = 1; j <= pp.Levelnum; ++j) {
		int indp = 0;
		for (int i = M + M + (j - 1)*N + 1; i <= M + M + j * N; i++) {
			indp++;
			if (x[i] > 0) {
				vv[indp].server_level = j;
				server_ind.push_back(indp);
			}
		}
	}
	if (initanswer != nullptr) {
		delete[] initanswer;
		initanswer = nullptr;
	}

	delete[]ServerVar;
	delete[]ArcVar;
	return 0;
}
/*

int mainSolver::solveonce_static(int, int, int timelim, vector<int>& x) {
	const int N = pp.siteNum;//点数
	const int M = pp.linkNum;//链接数
	const int WholeCol = M + M + pp.Levelnum * N;
	const int WholeRow = N + N + 1;
	x.resize(WholeCol + 1);
	ipro p;
	p.pPo = this;
	p.add_many_rows(WholeRow);
	p.add_many_cols(WholeCol);
	setLink(p);
	int flagBig = 0;
#ifdef Multiply_2
	//当cost特征不明显时，拉开特征差距，以换取高效求解
	if (N > 1000)flagBig = 1;
#endif // Multiply_2
	for (int j = 1; j <= pp.Levelnum; ++j) {
		//列的界 和 类型
		int indp = 0;
		for (int i = M + M + (j - 1)*N + 1; i <= M + M + j * N; i++)
		{
			indp++;
			if (j == 1)
			{
				p.s_c_bnd(i, Bnd_both, 0, 1);
				p.col[i]->coef = (vv[indp].const_cost + pp.levelcost[j])*(flagBig ? 2 : 1);
				p.col[i]->var_type = ColType_Z;
			}
			if (j < pp.Levelnum && vv[indp].sumsupply <= pp.leveloutput[j]) {
				p.s_c_bnd(i + N, Bnd_fixed, 0, 0);
				p.col[i + N]->coef = 0;
				p.col[i + N]->var_type = ColType_Z;
			}
			else if (j < pp.Levelnum) {
				p.s_c_bnd(i + N, Bnd_both, 0, 1);
				p.col[i + N]->coef = (vv[indp].const_cost + pp.levelcost[j + 1])*(flagBig ? 2 : 1);
				p.col[i + N]->var_type = ColType_Z;
			}

		}
	}
	//////////////////////////////////////////////////////////////////////////
	//额外判断g_candi，实际上只需要将一些列从Bnd_both变成Bnd_fixed:1.非基解fix掉3.4.5基解fix掉0.1.2.3or0.1(或者不fix）
	int Belif_range = 2;//置信范围
	//range for [v_cand & v_cand - Belif_range]@
	 //列的界 和 类型
	for (int i = 1; i <= N; ++i)
	{
		// 			if (initanswer != nullptr && v_cand[i] == 0) {
		// 				for (int j = 1; j <= pp.Levelnum; ++j) {
		// 					if (initanswer[M + M + N * (j - 1) + i] != 0) {
		// 						v_cand[i] = j;
		// 						break;
		// 					}
		// 				}
		// 			}
		int low = max(v_cand[i] - Belif_range, 0);
		int up = v_cand[i];
		//[low ~ up] were choosed
		for (int j = 1; j <= pp.Levelnum; ++j) {
			if (j >= low && j <= up) {//置信区间内，不fix
				continue;
			}
			if (initanswer != nullptr &&
				initanswer[M + M + N * (j - 1) + i] != 0) continue;//如果是初始解，那么不fix这个档次的该服务器
			p.s_c_bnd(M + M + N * (j - 1) + i, Bnd_fixed, 0, 0);
		}
	}
	
	constexpr int FanQ =
#ifdef __FanXiangQ
		- 1;
#else
		1;
#endif // __FanXiangQ
	int elemnum = 0;
	for (int i = 1; i <= N; ++i) {
		for (int j = 1; j <= M; ++j) {//流守恒
			if (vl[j].vertexi == i) {//vertexid 从 1开始
				SetPtVal(i, j, FanQ* -1);
				SetPtVal(i, M + j, FanQ * 1);
			}
			else if (vl[j].vertexj == i) {
				SetPtVal(i, j, FanQ * 1);
				SetPtVal(i, M + j, FanQ*-1);
			}
		}
		for (int j = 1; j <= pp.Levelnum; ++j) {
			SetPtVal(i, M + M + (j - 1)*N + i, min(pp.leveloutput[j], vv[i].sumsupply));
		}
		p.s_r_bnd(i, Bnd_low, vv[i].demand, vv[i].demand);
	}
	for (int i = 1; i <= N; ++i) {
		for (int j = 1; j <= pp.Levelnum; ++j) {
			SetPtVal(N + i, M + M + (j - 1)*N + i, -1);
		}
		p.s_r_bnd(N + i, Bnd_low, -1, -1);
	}
	int indrow = N * 2 + 1;
	for (int j = 1; j <= pp.Levelnum; ++j) {
		int indp = 0;
		for (int i = M + M + (j - 1)*N + 1; i <= M + M + j * N; i++) {
			indp++;
			SetPtVal(indrow, i, min(vv[indp].sumsupply, pp.leveloutput[j]));
		}
	}
	p.s_r_bnd(indrow, Bnd_low, pp.demandall, pp.demandall);
	p.construct_mat(elemnum, rr, cc, ar);
	mip_control parm_MIP;
#ifdef _MSC_VER
	parm_MIP.p_output = 2;
#endif // _MSC_VER
	parm_MIP.time_lim = timelim;

	int ret = p.mip_main(parm_MIP);
	if (ret == -1)
	{
		pp.z = 0;
#ifdef _MSC_VER
	printf("No answer!");
#endif // _MSC_VER
		return -1;
	}
	pp.z = p.mip_obj;
	printf("\n********\nz=%f\n", pp.z);
	//把解保存在x里
	for (int i = 1; i <= WholeCol; ++i) {
		x[i] = static_cast<int>(p.col[i]->mipx + 0.1);
	}
	clearAnsinfo();
	for (int j = 1; j <= pp.Levelnum; ++j) {
		int indp = 0;
		for (int i = M + M + (j - 1)*N + 1; i <= M + M + j * N; i++) {
			indp++;
			if (x[i] > 0) {
				vv[indp].server_level = j;
				server_ind.push_back(indp);
			}
		}
	}
	if (initanswer != nullptr) {
		delete[] initanswer;
		initanswer = nullptr;
	}
	return ret;
}*/
typesol mainSolver::solver_candidate_based() {
	//extern int maxA;maxA = 15;//默认15
	//extern bool on_get_cand;
	//extern bool midcase;
	if (on_get_cand) {//use glpk to find candidate
		push_demand();
		int time1 = 3000;
		vector<int> x;
		solveonce_static_GRB(-1, -1, time1, x);
		span_demand();
		return typesol();
	}
	else {//use gurobi to solve final problem
		//if (midcase) {
		FindInitialSolution(true);
		//}
		//int time1 = 88000 - int(nowtime() - begintime);
		push_demand();
		vector<int> x;
		solveonce_static_GRB(-1, -1, 7200000, x); //7200000
		if (pp.z == 0)return typesol();
		span_demand();
		return x2solution(x);
	}
// 
// 	if (!on_get_cand && midcase) {
// 		FindInitialSolution(true);//寻找初始解
// 	}
// 	int time1 = 88000 - int(nowtime() - begintime);
// 	push_demand();
// 	if (on_get_cand) {
// 		time1 = 3000;
// 	}
// 	vector<int> x;
// 	solveonce_static(-1, -1, time1, x);
// 	if (on_get_cand) {
// 		span_demand();
// 		return typesol();
// 	}
// 	if (pp.z == 0)return typesol();
// 	span_demand();
// 	return x2solution(x);
}

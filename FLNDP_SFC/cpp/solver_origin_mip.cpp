#include <sstream>
#include <iomanip>
#include "solver_main.h"
#include "ALNS.h"
#include "myCallback.h"

using namespace std;
using itL = list<link>::iterator;
using PLI = pair<itL, int>;
//#define  MAXNET 1000
#define min(a,b) ((a)<(b)?(a):(b))

//extern bool on_get_cand;
int mainSolver::solveonce_ori_GRB(int, int, int timelim, vector<int>& x) {
	const int N = pp.siteNum;//点数
	const int M = pp.linkNum;//链接数
	const int WholeCol = M + M + pp.Levelnum * N;
	//const int WholeRow = N + N + 1;

	ansfile af(pp.pfileinfo->origin_sol.c_str());
	af.setN_lvl(N, pp.Levelnum,g_chuz);
	if (!af.openread())cout << "can't open the answerfile";
	// 	for (auto pr : af.id_lvl) {
	// 		cout << pr.first << "\t" << pr.second << endl;
	// 	}


	x.reserve(WholeCol + 1);
	GRBEnv env = GRBEnv();
	GRBModel model(env);
	auto ArcVar = model.addVars(2 * M, GRB_CONTINUOUS);
	setLink(ArcVar);
	auto ServerVar = model.addVars(pp.Levelnum*N, GRB_BINARY);
	setServer(ServerVar);
	//int Belif_range = 2;//置信范围
	for (int i = 1; i <= N; ++i)
	{
		int low = 0;
		int up = max_lvl[i];//pp.Levelnum;
		//[low ~ up] were choosed
		for (int j = 1; j <= pp.Levelnum; ++j) {
			if (j >= low && j <= up) {//置信区间内，不fix
				continue;
			}
			ServerVar[N * (j - 1) + i - 1].set(GRB_DoubleAttr_UB, 0);
		}
	}
	//auto flowB = model.addConstrs(2 * N);

	//int elemnum = 0;
	for (int i = 1; i <= N; ++i) {
		GRBLinExpr flowi;
		for (int j = 0; j < M; ++j) {//流守恒
			if (vl[j + 1].vertexi == i) {//i流出
				flowi -= ArcVar[j];
				flowi += ArcVar[M + j];
			}
			else if (vl[j + 1].vertexj == i) {//流入i
				flowi += ArcVar[j];
				flowi -= ArcVar[M + j];
			}
		}
		for (int j = 1; j <= pp.Levelnum; ++j) {
			flowi += min(pp.leveloutput[j],vv[i].sumsupply)
				* ServerVar[(j - 1)*N + i - 1];
		}
		model.addConstr(flowi >= vv[i].demand);
	}
// 
// 	for (int i = 1; i <= N; ++i) {
// 		GRBLinExpr flowi;
// 		GRBLinExpr left;
// 		for (int j = 0; j < M; ++j) {//流守恒
// 			if (vl[j + 1].vertexi == i) {//i流出
// 				flowi -= ArcVar[j];
// 				flowi += ArcVar[M + j];
// 			}
// 			else if (vl[j + 1].vertexj == i) {//流入i
// 				flowi += ArcVar[j];
// 				flowi -= ArcVar[M + j];
// 			}
// 		}
// 		for (int j = 1; j <= pp.Levelnum; ++j) {
// 			flowi += min(pp.leveloutput[j], 1000000*vv[i].sumsupply)
// 				* ServerVar[(j - 1)*N + i - 1];
// 		}
// 		for (int j = 1; j <= pp.Levelnum; ++j) {
// 			left += 100000*ServerVar[(j - 1)*N + i - 1];
// 		}
// 		left += vv[i].demand;
// 		model.addConstr(left >= flowi);
// 	}


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
		for (int i = (j - 1)*N; i < j * N; i++) {
			indp++;
			validinq += ServerVar[i] * min(vv[indp].sumsupply, pp.leveloutput[j]);
		}
	}
	model.addConstr(validinq >= pp.demandall);

	model.set(GRB_DoubleParam_MIPGapAbs, 1);
	model.set(GRB_DoubleParam_MIPGap, 1e-10);
	model.set(GRB_DoubleParam_TimeLimit, timelim / 1000.0);

	ofstream logfile(g_chuz == 4 ? path_rec /*origin*/: 
		(g_chuz==5 ? path_iter /*test shoot ratio*/ : 
			path_core_record /*my function*/) );
	//ofstream logfile(g_chuz == 4 ? path_rec : pp.pfileinfo->my_iter_rate);
	if (!logfile.is_open()) {
		cout << "Cannot open solution_time_long.txt for callback message" << endl;
		return 1;
	}
	logfile << setiosflags(ios::fixed) << setprecision(4);
	mycallback cb = mycallback(pp.Levelnum*N, ServerVar, &logfile,&af,on_get_cand);
	cb.wholeVarnum = pp.Levelnum*N + 2 * M;
	model.setCallback(&cb);
	model.set(GRB_IntParam_Threads, 1);
	model.set(GRB_IntParam_LogToConsole,0);

 

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
	for (int i = 0; i < 2 * M; ++i) {
		x.push_back(static_cast<int>(ArcVar[i].get(GRB_DoubleAttr_X) + 0.1));
	}
	for (int i = 0; i < N*pp.Levelnum; ++i) {
		x.push_back(static_cast<int>(ServerVar[i].get(GRB_DoubleAttr_X) + 0.1));
	}

	if (on_get_cand) {
		forinit = x;
		v_cand.swap(cb.vcand);
		delete[]ServerVar;
		delete[]ArcVar;
		return 0;
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

	delete[]ServerVar;
	delete[]ArcVar;
	return 0;
}

/*

int mainSolver::solveonce_ori(int, int, int timelim, vector<int>& x) {
	const int N = pp.siteNum;//点数
	const int M = pp.linkNum;//链接数
	const int WholeCol = M + M + pp.Levelnum * N;
	x.resize(WholeCol + 1);
	const int WholeRow = N + N + 1;
	
	ipro p;
	p.pPo = this;
	p.add_many_rows(WholeRow);
	p.add_many_cols(WholeCol);
	setLink(p);
	int indp = 0;
	//正常的列约束不变，当g_candi的第一项为1时，需要额外fix非基变量大容量部分和基变量小容量部分
	for (int j = 1; j <= pp.Levelnum; ++j) {
		//列的界 和 类型
		indp = 0;
		for (int i = M + M + (j - 1)*N + 1; i <= M + M + j * N; i++)
		{
			indp++;
			if (j == 1)
			{
				p.s_c_bnd(i, Bnd_both, 0, 1);
				p.col[i]->coef = (vv[indp].const_cost + pp.levelcost[j]);
				p.col[i]->var_type = ColType_Z;
			}
			if (j < pp.Levelnum && vv[indp].sumsupply <= pp.leveloutput[j]) {
				p.s_c_bnd(i + N, Bnd_fixed, 0, 0);
				p.col[i + N]->coef = 0;
				p.col[i + N]->var_type = ColType_Z;
			}
			else if (j < pp.Levelnum) {
				p.s_c_bnd(i + N, Bnd_both, 0, 1);
				p.col[i + N]->coef =( vv[indp].const_cost + pp.levelcost[j + 1]);
				p.col[i + N]->var_type = ColType_Z;
			}

		}
	}
	for (int i = 1; i <= N; ++i)
	{
		int low = 0;
		int up = max_lvl[i];
		for (int j = 1; j <= pp.Levelnum; ++j) {
			if (j >= low && j <= up) {//区间内，不fix
				continue;
			}
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
		for (int j = 1; j <= M; ++j) {
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
		indp = 0;
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
	if (ret == -1)printf("wrong!\n");
	if (on_get_cand) {
		return 0;//仅用来得到candidate
	}
	pp.z = p.mip_obj;
#ifdef _MSC_VER
	printf("\n********\nz=%f\n", pp.z);
#endif // _MSC_VER
	//恢复边上信息
	for (int i = 1; i <= WholeCol; ++i) {
		x[i] = static_cast<int>(p.col[i]->mipx + 0.1);
	}
	clearAnsinfo();
	for (int j = 1; j <= pp.Levelnum; ++j) {
		indp = 0;
		for (int i = M + M + (j - 1)*N + 1; i <= M + M + j * N; i++) {
			indp++;
			if (x[i] > 0) {
				vv[indp].server_level = j;
				server_ind.push_back(indp);
			}
		}
	}
	if (pp.z == 0)return -1;
	return ret;
}*/
//无用
typesol mainSolver::solver_origin_mip() {
	push_demand();
	vector<int> x;//extern int maxA;maxA = 15;
	solveonce_ori_GRB(-1, -1, 7200000, x);
	if (pp.z == 0)return typesol();
	span_demand();
	return x2solution(x);
}
/*
void mainSolver::setLink(ipro &p)
{
	const int M = pp.linkNum;
	for (int i = 1; i <= M + M; ++i) {//列的界 和 类型
		if (i <= M)//m个 uij
		{
			p.s_c_bnd(i, Bnd_both, 0, vl[i].volume);
			p.col[i]->coef = vl[i].percost;//coeff
			p.col[i]->var_type = ColType_L;
			//必须设定为连续值,否则需要改spslp中的值。如果是L，那么所有Z均为0-1变量，可以简化问题
			//常数项为0
		}
		else if (i <= 2 * M) {//m个 uji
			p.s_c_bnd(i, Bnd_both, 0, vl[i - M].volume);
			p.col[i]->coef = vl[i - M].percost;//coeff
			p.col[i]->var_type = ColType_L;
		}
	}//前2M个
}
*/

void mainSolver::setLink(GRBVar* var)
{
	const int M = pp.linkNum;
	for (int i = 0; i < M; ++i) {//列的界 和 类型
		var[i].set(GRB_DoubleAttr_LB, 0);
		var[i].set(GRB_DoubleAttr_UB, vl[i + 1].volume);
		var[i].set(GRB_DoubleAttr_Obj, vl[i + 1].percost);
	}
	const int offset = M - 1;
	for (int i = M; i < M*2; ++i) {//列的界 和 类型
		var[i].set(GRB_DoubleAttr_LB, 0);
		var[i].set(GRB_DoubleAttr_UB, vl[i - offset].volume);
		var[i].set(GRB_DoubleAttr_Obj, vl[i - offset].percost);
	}
}

void mainSolver::setServer(GRBVar *serv)
{
	const int N = pp.siteNum;//点数
	//const int M = pp.linkNum;//链接数
	for (int j = 1; j <= pp.Levelnum; ++j) {
		//列的界 和 类型
		int indp = 0;
		for (int i = (j - 1)*N ; i < j * N; i++)
		{
			indp++;
			if (j == 1)
			{
				serv[i].set(GRB_DoubleAttr_Obj, (vv[indp].const_cost + pp.levelcost[j]));
			}
			if (j < pp.Levelnum && vv[indp].sumsupply <= pp.leveloutput[j]) {
				serv[i+N].set(GRB_DoubleAttr_UB, 0);
				serv[i+N].set(GRB_DoubleAttr_Obj, 0);
// 				p.s_c_bnd(i + N, Bnd_fixed, 0, 0);
// 				p.col[i + N]->coef = 0;
// 				p.col[i + N]->var_type = ColType_Z;
			}
			else if (j < pp.Levelnum) {
				serv[i+N].set(GRB_DoubleAttr_Obj, (vv[indp].const_cost + pp.levelcost[j+1]));
// 				p.s_c_bnd(i + N, Bnd_both, 0, 1);
// 				p.col[i + N]->coef = (vv[indp].const_cost + pp.levelcost[j + 1])*(flagBig ? 2 : 1);
// 				p.col[i + N]->var_type = ColType_Z;
			}

		}
	}
}


#pragma once
#include <string>
#include <vector>
#include <list>
#include <map>
#include <sys/timeb.h>
#include "linknode.h"
#include "fileinfo.h"
#include "gurobi_c++.h"

using typesol = std::vector<std::list<int>>;

struct pp_
{
	//////////////////////////////////////////////////////////////////////////
	int siteNum, linkNum, customerNum;
	//////////////////////////////////////////////////////////////////////////
	int demandall = 0;
	std::vector<std::vector<int>> prdlist;
	//////////////////////////////////////////////////////////////////////////
	int Levelnum;
	int leveloutput[21];//level的输出能力
	int levelcost[21];//level的代价
					  //////////////////////////////////////////////////////////////////////////
	fileinfo* pfileinfo;
	double z;
	//////////////////////////////////////////////////////////////////////////
};


typedef class mainSolver
{
public:
	double begintime;
	double load_and_solve(std::string,int);
	//基于候选集的模型
	int solveonce_static_GRB(int, int, int timelim, std::vector<int>& x);
	//	int solveonce_static(int,int,int timelim,std::vector<int>& x);
	int solveonce_ori_GRB(int, int, int timelim, std::vector<int>& x);
	//原始模型
	//	int solveonce_ori(int, int, int timelim, std::vector<int>& x);
	//	int solver_step_variable(int, int, int, std::vector<int>& x, std::vector<int>& lst);

	typesol solver_origin_mip();//MIP solver 0 1 2 3 4 5 6
	typesol solver_candidate_based();//基于统计数据的求解接口
									 //	typesol solver_ALNS();//ALNS solver
									 //	int solver_DE_GA();//GA solver
	void push_demand();
	void span_demand();
	void clearAnsinfo();
	typesol x2solution(std::vector<int>& x);
	void Write_best_to_file(std::string fname);
	void Write_to_file();
	//	void getTransProbabilityMat();
	//	bool Check_feasible(typesol&);
	void FindInitialSolution(bool as_init = false);
	void setLink(GRBVar*);
	void setServer(GRBVar*);
	std::vector<node> vv;
	std::vector<link> vl;
	std::vector<int> forinit;
	typesol solution;
	mutable pp_ pp;

	void get_cand();
	double get_max_lvl();
	bool togetmax = 0;
	double get_max_time;
	std::vector<int> server_ind;
	std::vector<int> v_cand;
	std::vector<int> max_lvl;
	std::string path_sol, path_rec, path_result,path_iter,path_core_record;

	//std::vector<std::pair<double, int>> vfitness;
	int* initanswer = nullptr;
	int initcost = -1;

	double nowtime()
	{
		timeb rawtime;
		ftime(&rawtime);
		return static_cast<double>(rawtime.millitm + rawtime.time * 1000);
	}
	int g_chuz;
	bool on_get_cand;
	~mainSolver() {
		if (initanswer != nullptr)
			delete[] initanswer;
	}
}problem;

void deploy_server(fileinfo &fi);
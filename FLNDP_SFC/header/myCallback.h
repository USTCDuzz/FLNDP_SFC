#pragma once
#include <fstream>
#include <map>
#include <algorithm>
#include <memory>
#include "gurobi_c++.h"
using std::cout;
class ansfile {
public:
	ansfile(const char* name):fname(name) {

	}
	int openread() {
		std::ifstream f(fname);
		if (!f.is_open()) { return 0; }
		id_lvl.clear();
		int ID, LVL;
		while (f>>ID>>LVL)
		{
			id_lvl[ID] = LVL;
		}
		f.close();
		return 1;
	}
	void setN_lvl(int N, int lvlNum,int algo) {
		this->N = N;
		this->lvlNum = lvlNum;
		this->algo = algo;
	}
	std::map<int, int> id_lvl;
	std::vector<int> is1;
	int N, lvlNum,algo;
private:
	std::string fname;
};

class mycallback : public GRBCallback
{
public:
	using ofstream=std::ofstream;
	int nodect = 0;
	int delcol = -1;
	int delrow = -1;
	double lastiter;
	double lastnode;
	int numvars;
	GRBVar* vars;
	ofstream* logfile;
	ansfile* pans;
	bool flag_quit_done=false;
	bool tofind_cand = false;
	bool tst = true;
	std::vector<int> vcand; 
	std::vector<double> averageCand;
	unsigned ct = 0;
	unsigned simplexloop = 0;
	int wholeVarnum = 0;
	mycallback(int xnumvars, GRBVar* xvars, ofstream* xlogfile,
		ansfile *pans=nullptr,bool tfcand = false) {
		lastiter = lastnode = -GRB_INFINITY;
		numvars = xnumvars;
		vars = xvars;
		logfile = xlogfile;
		tofind_cand = tfcand;
		this->pans = pans;
	}
protected:
	void callback() {
		try {
			if (where == GRB_CB_MIPSOL) {
				if (pans==nullptr || pans->algo != 5) {
					double times = getDoubleInfo(GRB_CB_RUNTIME);
					*logfile << getDoubleInfo(GRB_CB_MIPSOL_OBJ) << " " << times << std::endl;
				}
			}
			if (where == GRB_CB_PRESOLVE) {
				delcol = getIntInfo(GRB_CB_PRE_COLDEL);
				delrow = getIntInfo(GRB_CB_PRE_ROWDEL);
			}
			else if (!tofind_cand) {
				return;
			}
// 			if (where == GRB_CB_POLLING) {
// 				// Ignore polling callback
// 			}
// 			else if (where == GRB_CB_PRESOLVE) {
// 				// Presolve callback
// 				int cdels = getIntInfo(GRB_CB_PRE_COLDEL);
// 				int rdels = getIntInfo(GRB_CB_PRE_ROWDEL);
// 				if (cdels || rdels) {
// 					cout << cdels << " columns and " << rdels
// 						<< " rows are removed" << std::endl;
// 				}
// 			}
// 			else if (where == GRB_CB_SIMPLEX) {
// 				// Simplex callback
// 				double itcnt = getDoubleInfo(GRB_CB_SPX_ITRCNT);
// 				if (itcnt - lastiter >= 100) {
// 					lastiter = itcnt;
// 					double obj = getDoubleInfo(GRB_CB_SPX_OBJVAL);
// 					int ispert = getIntInfo(GRB_CB_SPX_ISPERT);
// 					double pinf = getDoubleInfo(GRB_CB_SPX_PRIMINF);
// 					double dinf = getDoubleInfo(GRB_CB_SPX_DUALINF);
// 					char ch;
// 					if (ispert == 0)      ch = ' ';
// 					else if (ispert == 1) ch = 'S';
// 					else                  ch = 'P';
// 					cout << itcnt << " " << obj << ch << " "
// 						<< pinf << " " << dinf << std::endl;
// 				}
// 			}
			else if (where == GRB_CB_MIP) {
				// General MIP callback
				//int itcnt = (int)getDoubleInfo(GRB_CB_MIP_ITRCNT);//simplex num
				//printf("simplex : %d\n", itcnt);
				if (tofind_cand &&(simplexloop=(int)getDoubleInfo(GRB_CB_MIP_ITRCNT)) >
					/*1700*/ (wholeVarnum - delcol) / 3) {
					//4.5
				/*(numvars- delcol)/1.4
				ans =
						0.974530735930736
						ans =
						0.6667*/
					flag_quit_done = true;
					//cout << "OOOOOOOOOOOOOOOOOOOOOOOOOOOO" << std::endl;
				}
				//if( (int)getDoubleInfo(GRB_CB_MIP_ITRCNT)%100==0)tst=true;
// 				double nodecnt = getDoubleInfo(GRB_CB_MIP_NODCNT);
// 				double objbst = getDoubleInfo(GRB_CB_MIP_OBJBST);
// 				double objbnd = getDoubleInfo(GRB_CB_MIP_OBJBND);
// 				int solcnt = getIntInfo(GRB_CB_MIP_SOLCNT);
// 				if (nodecnt - lastnode >= 100) {
// 					lastnode = nodecnt;
// 					int actnodes = (int)getDoubleInfo(GRB_CB_MIP_NODLFT);
// 					int itcnt = (int)getDoubleInfo(GRB_CB_MIP_ITRCNT);
// 					int cutcnt = getIntInfo(GRB_CB_MIP_CUTCNT);
// 					cout << nodecnt << " " << actnodes << " " << itcnt
// 						<< " " << objbst << " " << objbnd << " "
// 						<< solcnt << " " << cutcnt << std::endl;
// 				}
// 				*logfile << objbst << std::endl;
// 				*logfile << objbst << std::endl;
// 				*logfile << objbst << std::endl;
// 				if (fabs(objbst - objbnd) < 0.1 * (1.0 + fabs(objbst))) {
// 					cout << "Stop early - 10% gap achieved" << std::endl;
// 					abort();
// 				}
// 				if (nodecnt >= 10000 && solcnt) {
// 					cout << "Stop early - 10000 nodes explored" << std::endl;
// 					abort();
// 				}
			}
// 			else if (where == GRB_CB_MIPSOL) {
// 				// MIP solution callback
// 				int nodecnt = (int)getDoubleInfo(GRB_CB_MIPSOL_NODCNT);
// 				double obj = getDoubleInfo(GRB_CB_MIPSOL_OBJ);
// 				int solcnt = getIntInfo(GRB_CB_MIPSOL_SOLCNT);
// 				double* x = getSolution(vars, numvars);
// 				cout << "**** New solution at node " << nodecnt
// 					<< ", obj " << obj << ", sol " << solcnt
// 					<< ", x[0] = " << x[0] << " ****" << std::endl;
// 				delete[] x;
// 			}
			else if (where == GRB_CB_MIPNODE) {
				//simplexloop = (int)getDoubleInfo(GRB_CB_MIP_ITRCNT);
				//cout << simplexloop << std::endl;
				if (getIntInfo(GRB_CB_MIPNODE_STATUS) == GRB_OPTIMAL) {
					double* x = getNodeRel(vars, numvars);
					averageCand.resize(numvars,0.0);
					ct++;
					std::transform(averageCand.begin(), averageCand.end(), 
						x, averageCand.begin(), std::plus<int>());
					delete[]x;
				}
				if (flag_quit_done && getIntInfo(GRB_CB_MIPNODE_STATUS) == GRB_OPTIMAL) {
					std::vector<int> vcand;
					vcand.resize(pans->N + 1);
					std::map<int, double> map_id_prim;
					//double* x = getNodeRel(vars, numvars);
					std::unique_ptr<double[]> x(getNodeRel(vars, numvars));
					//unsigned n = ct;
					// 					auto x = new double[numvars];
					// 					std::transform(averageCand.begin(), averageCand.end(), x, 
					// 						[&n](double i)	{return i/n; });
					//setSolution(vars, x, numvars);
					int lvl, ID;
					for (int i = 0; i < numvars; ++i) {
						if (x[i] > 0.0000000001 / pans->lvlNum) {
						//if (x[i] > 0.1 / pans->lvlNum) {
							lvl = i / pans->N + 1;
							ID = i%pans->N + 1;
							if (map_id_prim.find(ID) == map_id_prim.end()
								|| map_id_prim[ID] <= x[i]) {
								map_id_prim[ID] = x[i];
								vcand[ID] = lvl;
							}
							//printf("ID = %d,LVL = %d\n", ID, lvl);
							//printf("[%5d %.2f] ", i, x[i]);
						}
					}
					// 					for (int i = 1; i < vcand.size();++i) {
					// 						if(vcand[i])printf("ID %d lvl %d\n", i, vcand[i]);
					// 					}
					int ctfound = 0;
					auto &mm = pans->id_lvl;
					int allnum = 0;
					for (unsigned i = 1; i < vcand.size(); ++i) {
						if (vcand[i]) {
							int lb = vcand[i] - 3 < 1 ? 1 : vcand[i] - 3;
							int ub = vcand[i] + 1 > pans->lvlNum ? pans->lvlNum : vcand[i] + 1;
							allnum += ub - lb + 1;
							auto it = mm.find(i);
							if (it != mm.end() /*&& vcand[i] == mm[i]*/) {//找到一个
																		  //	printf("ID %d best lvl %d candlvl %d\n", i, it->second, vcand[i]);
								ctfound++;
							}
						}
					}
					size_t bestnum = mm.size();
					int shootnum = ctfound;
					double rate = 1.0*shootnum / bestnum;
					printf("************************************\nshoot ratio %.2f%%\n************************************\n", rate*100.0);
					*logfile << "finaliter = " << simplexloop << "\t" << rate << std::endl;
					*logfile << shootnum << " " << bestnum << " " <<allnum << " " << numvars <<" "<< wholeVarnum - delcol <<  std::endl;
					this->vcand.swap(vcand);
					abort();
				}
				else
				if (tst && getIntInfo(GRB_CB_MIPNODE_STATUS) == GRB_OPTIMAL) {//测试最优解命中率
					int lvl, ID;
					std::vector<int> vcand;
					vcand.resize(pans->N + 1);
					std::map<int, double> map_id_prim;
					std::unique_ptr<double[]> x (getNodeRel(vars, numvars));
					for (int i = 0; i < numvars; ++i) {
						
						//if (x[i] > 0.1/pans->lvlNum) {
						if (x[i] > 0.0000000001 / pans->lvlNum) {
							//std::cout << i << " " << x[i] << std::endl;
							lvl = i / pans->N + 1;
							ID = i%pans->N + 1;
							if (map_id_prim.find(ID) == map_id_prim.end()
								|| map_id_prim[ID] <= x[i]) {
								map_id_prim[ID] = x[i];
								vcand[ID] = lvl;
							}
						}
						//else if (x[i]>0)std::cout<<"***" << i << " " << x[i] << std::endl;
					}
					int ctfound = 0;
					const auto &mm = pans->id_lvl;
					for (unsigned i = 1; i < vcand.size(); ++i) {
						if (vcand[i]) {
							auto it = mm.find(i);
							if (it != mm.end()) {//找到一个
												 //	printf("ID %d best lvl %d candlvl %d\n", i, it->second, vcand[i]);
								ctfound++;
							}
						}
					}
					size_t bestnum = mm.size();
					int shootnum = ctfound;
					double rate = 1.0*shootnum / bestnum;
					if (rate > 0.98) {
						//*logfile << "loop = " << simplexloop << " value = " << r
					}
					//printf("************************************\nshoot ratio %.2f%%\n************************************\n", rate*100.0);
					if(pans->algo==5)
					*logfile<< simplexloop << "\t" << rate << std::endl;
					else
					*logfile << "simpiter = " << simplexloop << "\t shoot ratio = " << rate << std::endl;
					//tst = false;
				}
				// MIP node callback
				//cout << "**** New node ****" << std::endl;
				//getDoubleInfo(GRB_CB_MIPNODE_REL);
				
			}
// 			else if (where == GRB_CB_BARRIER) {
// 				// Barrier callback
// 				int itcnt = getIntInfo(GRB_CB_BARRIER_ITRCNT);
// 				double primobj = getDoubleInfo(GRB_CB_BARRIER_PRIMOBJ);
// 				double dualobj = getDoubleInfo(GRB_CB_BARRIER_DUALOBJ);
// 				double priminf = getDoubleInfo(GRB_CB_BARRIER_PRIMINF);
// 				double dualinf = getDoubleInfo(GRB_CB_BARRIER_DUALINF);
// 				double cmpl = getDoubleInfo(GRB_CB_BARRIER_COMPL);
// 				cout << itcnt << " " << primobj << " " << dualobj << " "
// 					<< priminf << " " << dualinf << " " << cmpl << std::endl;
// 			}
// 			else if (where == GRB_CB_MESSAGE) {
// 				// Message callback
// 				string msg = getStringInfo(GRB_CB_MSG_STRING);
// 				*logfile << msg;
// 			}
		}
		catch (GRBException e) {
			cout << "Error number: " << e.getErrorCode() << std::endl;
			cout << e.getMessage() << std::endl;
		}
		catch (...) {
			cout << "Error during callback" << std::endl;
		}
	}
};

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdio.h>

#include "fileinfo.h"
#include "solver_main.h"
#include "thpool.h"

#define TEST_jump
//#define data_test

using namespace std;
void par_func(fileinfo fi)
{
	deploy_server(fi);
}
#ifdef _WIN32
#include <io.h>
vector<string> visit(string path) {
	if (path.back() != '/')path.append("/");
	path.append("*");
	vector<string> vs;
	_finddata_t filefind;
	int i, done=0;
	auto handle = _findfirst(path.c_str(), &filefind);
	if (handle == -1)return vs;
	while (!(done=_findnext(handle,&filefind)))
	{
		if (!strcmp(filefind.name, ".."))continue;
		if ((_A_SUBDIR == filefind.attrib)) {
			vs.emplace_back(filefind.name);
		}
	}
	_findclose(handle);
	sort(vs.begin(),vs.end());
	return vs;
}
#else
#include <dirent.h>
vector<string> visit(string path) {
	if (path.back() != '/')path.append("/");
	//path.append("*");
	vector<string> vs;
	DIR *dfd;
	dirent *dp;
	if((dfd=opendir(path.c_str()))==nullptr){
		printf("can't open %s file\n",path.c_str());
		return vs;
	}
	while((dp=readdir(dfd))!=nullptr){
		if(strncmp(dp->d_name,".",1)==0)continue;
		if(dp->d_type==4){
			vs.emplace_back(string(dp->d_name));
		}
	}
	closedir(dfd);
	sort(vs.begin(),vs.end());
	return vs;
}



#endif


int main(int argc, char *argv[]) {
	//int caseID;
#ifdef _WIN32
#ifdef data_test
	string rootpath("D:/Paper Experiment/FCNF/data_test");
#else
	string rootpath("D:/Paper Experiment/FCNF/data");
#endif
#else
#ifdef data_test
	string rootpath("/home/qiankun/G/FCNF/data_test");
#else
	string rootpath("/home/qiankun/G/FCNF/data");
#endif 
#endif
	//string rootpath("D:/Paper Experiment/FCNF/data");
	if (rootpath.back() != '/')rootpath.append("/");
	auto folders = visit(rootpath);
//	char buffer[300];
	//auto it = folders[5];
// 	for (auto it : folders) {
// 		it = rootpath + it + "/";
// 		cout << it << endl;
// 		fileinfo fi(it);
// #ifdef TEST_jump
// 		//ifstream test(fi.origin_record);
// 		//if (test.is_open()) { test.close(); continue; }
// #endif
// 		deploy_server(fi);
// 	}
#ifdef _MSC_VER
	const size_t MAX_TH = 4;
#else
	const size_t MAX_TH = 22;
#endif
	ilovers::TaskExecutor executor{ MAX_TH };
	//auto it = folders[48];
	for (auto it:folders)
	{
		it = rootpath + it + "/";
		cout << it << endl;
		fileinfo fi(it);
		std::future<void> ff = executor.commit(par_func, fi);
	}
	//executor.shutdown();
	executor.shutdown();
	//Sleep(300000);
	
	return 0;
}

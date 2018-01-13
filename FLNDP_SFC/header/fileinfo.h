#pragma once
#include <string>
#define infn(name) name(fn+(#name)+".txt")
class fileinfo {
public:
	fileinfo(const std::string &fn):
		folderName(fn),
		infn(topo),	infn(origin_sol),infn(origin_record),infn(origin_sol_withmax),
		infn(origin_record_withmax),infn(my_sol),infn(my_record),infn(my_core_record),
		infn(my_iter_rate), infn(my_sol_nomax), infn(my_record_nomax),
		infn(my_core_record_nomax),infn(my_iter_rate_nomax),
		infn(result)
	{	
	}
	const std::string folderName;//�ļ�����
	const std::string topo;//��������

	const std::string origin_sol;//��¼���Ž�_withmaxlvl
	const std::string origin_record;//��¼ ��_ʱ�� pair

	const std::string origin_sol_withmax;
	const std::string origin_record_withmax;//��¼ ��_ʱ�� pair

	const std::string my_sol;//��¼���Ž�
	const std::string my_record;//��¼ ��_ʱ�� pair
	const std::string my_core_record;
	const std::string my_iter_rate;

	const std::string my_sol_nomax;//��¼���Ž�
	const std::string my_record_nomax;//��¼ ��_ʱ�� pair
	const std::string my_core_record_nomax;
	const std::string my_iter_rate_nomax;


	const std::string result;//��¼·�����
};
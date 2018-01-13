#pragma once
#include <vector>
#include <list>
#include <iterator>
#include <string.h>

struct link
{
	int id;
	int vertexi, vertexj;
	int volume;
	int percost;
	int canused;
	link(int id, int vertexi, int vertexj, int volume, int percost) :
		id(id), vertexi(vertexi), vertexj(vertexj), volume(volume), percost(percost)
	{
		canused = 0;
	}
	link() { canused = 0; }
};

struct node
{
	int id;
	int _realid;
	int demand;
	int sumsupply;
	int const_cost;
	int linkedCustomer;
	int server_level = 0;
	int self_output;
	//假定6个档次
	//double trasMat[7][7] = {0.0};
	std::vector<std::list<link>::iterator > adjlist;
	std::vector<int> adjvector;
	node(int id,int _realid,int demand) :
		id(id),_realid(_realid),demand(demand){
		adjvector.reserve(10);
		demand = 0;
	}
	node() {
		demand = 0;
		sumsupply = 0;
		adjvector.reserve(10);
	}
};

struct pthelm
{
	int load;
	int nodeid;
	int nxtlinkid;
};


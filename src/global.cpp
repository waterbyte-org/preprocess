/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.0.3
 * File       ：全局函数
 * Date       ：05/13/2022
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#include "global.h"

#include <iomanip>
#include <fstream>

int get_total_degree(INode* node)
//------------------------------------------------------------------------------
// 目的：获取节点的度（=出度+入度）
//------------------------------------------------------------------------------
{
	return node->getDnLinkCounts() + node->getUpLinkCounts();
}

void sub_node_invert(INode* node, double suber)
//------------------------------------------------------------------------------
// 目的：将节点的井底标高减小suber
//------------------------------------------------------------------------------
{
	node->setInvertElevation(node->getInvertElevation() - suber);	
}

void add_node_full_depth(INode* node, double adder)
//------------------------------------------------------------------------------
// 目的：将节点的井深增加adder
//------------------------------------------------------------------------------
{
	node->setFullDepth(node->getFullDepth() + adder);
}

void add_link_offset(INode* node, double adder)
//------------------------------------------------------------------------------
// 目的：将关联连接的偏移增加adder
//------------------------------------------------------------------------------
{
	ILink* link = nullptr;

	for (int j = 0; j < node->getUpLinkCounts(); ++j)
	{
		link = node->getUpLink(j);
		link->setEndOffset(link->getEndOffset() + adder);
	}
	for (int j = 0; j < node->getDnLinkCounts(); ++j)
	{
		link = node->getDnLink(j);
		link->setBeginOffset(link->getBeginOffset() + adder);
	}
}

void merge_node_inflow(INetwork* net, INode* del_node, INode* res_node)
//------------------------------------------------------------------------------
// 参数：res_node = 被保留节点
//       del_node = 被删除节点
// 目的：将del_node节点的入流整合至res_node节点
//------------------------------------------------------------------------------
{
	// 1）整合旱季流量（假设产污系数和时间模式完全相同）
	if (IDryWeatherFlow* del_dwf = del_node->getDryWeatherFlow())
	{
		// 将del_node的旱季流量对象的基准值加到res_node
		if (IDryWeatherFlow* res_dwf = res_node->getDryWeatherFlow())
			res_dwf->setBase(res_dwf->getBase() + del_dwf->getBase());
		// res_node没有旱季流量对象，直接从del_node复制过来
		else
			res_node->setDryWeatherFlow(del_dwf);
	}

	// 2）整合外部直接入流（假设时间序列和时间模式完全相同）
	char ts_name[30];
	char pat_name[30];	
	IPatternSet* ips = net->getPatternSet();
	ITimeseriesSet* its = net->getTimeseriesSet();
	if (IExternDirectFlow* del_edf = del_node->getExternDirectFlow())
	{		
		if (IExternDirectFlow* res_edf = res_node->getExternDirectFlow())
		{
			// 将del_node的外部直接入流对象的基准值加到res_node
			if (IPattern* del_pat = del_edf->getBasePattern())
			{
				ips->getPatternName(del_pat, pat_name);
				res_edf->setBasePattern(pat_name, ips);
				res_edf->setBaseline(
					res_edf->getBaseline() + del_edf->getBaseline());
			}
			// 将del_node的外部直接入流对象的比例因子加到res_node
			if (ITimeseries* del_ts = del_edf->getTimeseries())
			{
				its->getTimeseriesName(del_ts, ts_name);
				res_edf->setTimeseries(ts_name, its);
				res_edf->setScaleFactor(
					res_edf->getScaleFactor() + del_edf->getScaleFactor());
			}
		}
		// res_node没有外部直接入流对象，直接从del_node复制过来
		else
			res_node->setExternDirectFlow(del_edf);
	}

	// 3）整合RDII（假设单位过程线组完全一样）
	if (IRDII* del_rdii = del_node->getRdii())
	{
		// 将del_node的RDII对象的服务面积加到res_node		
		if (IRDII* res_rdii = res_node->getRdii())
			res_rdii->setArea(res_rdii->getArea() + del_rdii->getArea());
		// res_node没有RDII对象，直接从del_node复制过来
		else
			res_node->setRdii(del_rdii);
	}
}

int get_leaf_link_counts(INode* node)
//------------------------------------------------------------------------------
// 目的：获取节点的叶子连接数
//------------------------------------------------------------------------------
{
	int leaf_link_counts = 0;

	for (int i = 0; i < node->getDnLinkCounts(); ++i)
	{
		if (get_total_degree(node->getDnLink(i)->getEndNode()) == 1)
			++leaf_link_counts;
	}
	for (int i = 0; i < node->getUpLinkCounts(); ++i)
	{
		if (get_total_degree(node->getUpLink(i)->getBeginNode()) == 1)
			++leaf_link_counts;
	}

	return leaf_link_counts;
}

void merge_file(const char* inp_file, const char* geo_file)
//------------------------------------------------------------------------------
// 目的：将geo_file合并至inp_file
//------------------------------------------------------------------------------
{
	if (strcmp(inp_file, geo_file) == 0) return;
	
	char c[1024];
	std::ifstream ifile(geo_file);
	std::ofstream ofile(inp_file, std::ios::app);
	while (!ifile.eof()) 
	{
		ifile.getline(c, 1023);		
		ofile << c << "\n";
	}
	ifile.close();
	ofile.close();		
}

void record_node_inflow(INode* node, std::ostringstream& oss)
//------------------------------------------------------------------------------
// 目的：记录节点入流信息
//------------------------------------------------------------------------------
{
	if (node->getDryWeatherFlow())
		oss << std::setw(11) << "是" << " ";
	else
		oss << std::setw(11) << "否" << " ";
	if (node->getExternDirectFlow())
		oss << std::setw(11) << "是" << " ";
	else
		oss << std::setw(11) << "否" << " ";
	if (node->getRdii())
		oss << std::setw(11) << "是" << " ";
	else
		oss << std::setw(11) << "否" << " ";
}

double get_ground_elevation(INode* node)
//------------------------------------------------------------------------------
// 目的：获取节点地面标高
//------------------------------------------------------------------------------
{
	return node->getFullDepth() + node->getInvertElevation();
}
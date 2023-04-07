/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.1.1
 * File       ：调整高度相关
 * Date       ：03/27/2023
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#include "global.h"
#include "convertdepth.h"

#include <cmath>
#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

static double get_min_offset(INode* node)
//------------------------------------------------------------------------------
// 目的：获取节点node的关联连接的最小偏移
//------------------------------------------------------------------------------
{
	ILink* link = nullptr;
	double min_offset = 1.0E6;

	for (int i = 0; i < node->getUpLinkCounts(); ++i)
	{
		link = node->getUpLink(i);
		min_offset = std::min(min_offset, link->getEndOffset());
	}
	for (int i = 0; i < node->getDnLinkCounts(); ++i)
	{
		link = node->getDnLink(i);
		min_offset = std::min(min_offset, link->getBeginOffset());
	}

	return min_offset;
}

static double get_max_crown(INode* node)
//------------------------------------------------------------------------------
// 目的：获取节点node的关联连接的最大顶标高
// 注意：不是所有的连接都有横断面对象
//------------------------------------------------------------------------------
{
	ILink* link = nullptr;
	ISection* sect = nullptr;
	double max_crown = -1.0E6;

	for (int i = 0; i < node->getUpLinkCounts(); ++i)
	{
		link = node->getUpLink(i);
		sect = link->getSection();
		if (sect) max_crown = std::max(max_crown, node->getInvertElevation() +
			link->getEndOffset() + sect->getGeom1());
	}
	for (int i = 0; i < node->getDnLinkCounts(); ++i)
	{
		link = node->getDnLink(i);
		sect = link->getSection();
		if (sect) max_crown = std::max(max_crown, node->getInvertElevation() +
			link->getBeginOffset() + sect->getGeom1());
	}

	return max_crown;
}

static double get_height_diff(IConduit* conduit)
//------------------------------------------------------------------------------
// 目的：获取连接内底高差
//------------------------------------------------------------------------------
{
	return std::fabs(conduit->getBeginNode()->getInvertElevation() +
		conduit->getBeginOffset() - conduit->getEndNode()->getInvertElevation()
		- conduit->getEndOffset());
}

static bool is_link_error_length(IConduit* conduit)
//------------------------------------------------------------------------------
// 目的：检查连接高差是否大于等于长度？
//------------------------------------------------------------------------------
{	
	// 直角边必须小于斜边
	return get_height_diff(conduit) >= conduit->getInpLength();
}

int get_concave_counts(INodeSet* nodes)
//------------------------------------------------------------------------------
// 目的：统计连接插入节点底部的节点数量
//------------------------------------------------------------------------------
{
	char name[30];
	INode* node = nullptr;
	int concave_counts = 0;

	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		node = nodes->getNodeObjectAndName(name, i);
		if (get_min_offset(node) < 0.0)
			++concave_counts;
	}

	return concave_counts;
}

int get_convex_counts(INodeSet* nodes)
//------------------------------------------------------------------------------
// 目的：统计连接露出地面的节点数量
//------------------------------------------------------------------------------
{
	char name[30];
	INode* node = nullptr;
	int convex_counts = 0;

	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		node = nodes->getNodeObjectAndName(name, i);		
		if (node->isOutfall()) continue; // 排放口不检查

		if (get_max_crown(node) > get_ground_elevation(node))
			++convex_counts;
	}

	return convex_counts;
}

int get_error_link_counts(ILinkSet* links)
//------------------------------------------------------------------------------
// 目的：统计高差大于等于长度的连接数量
//------------------------------------------------------------------------------
{
	char name[30];
	int error_counts = 0;
	ILink* link = nullptr;

	for (int i = 0; i < links->getLinkCounts(); ++i)
	{
		link = links->getLinkObjectAndName(name, i);
		if (!link->isConduit()) continue;

		if (is_link_error_length(dynamic_cast<IConduit*>(link)))
			++error_counts;
	}

	return error_counts;
}

void convert_depth(const char* inp_file, const char* new_inp_file,
	const char* record_file, double length_multiplier)
//------------------------------------------------------------------------------
// 参数：length_multiplier = 管渠延长倍数
// 目的：调整inp_file，生成new_inp_file，并将过程记录在record_file
//------------------------------------------------------------------------------
{
	// 1）创建模型对象，读入inp_file
	INetwork* net = createNetwork();
	if (!net->readFromFile(inp_file))
	{
		char* error_text = new char[100000]; // 要确保足够大
		net->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return;
	}

	// 2）创建拓扑关系，这样才能获取节点的上下游连接
	ILinkSet* links = net->getLinkSet();
	INodeSet* nodes = net->getNodeSet();
	links->associateNode(nodes);

	// 3）打开记录文件
	std::ofstream record(record_file);
	if (!record)
	{
		std::cout << "打开文件 " << record_file << " 失败!" << "\n";
		return;
	}	
	record << std::setiosflags(std::ios::left);

	// 4）处理连接插入节点底部的情况
	record << std::setw(23) << "节点名称"   << " "
		   << std::setw(15) << "原底标高/m" << " "
		   << "新底标高/m"  << "\n";
	char name[30];
	double min_offset;
	INode* node = nullptr;
	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		node = nodes->getNodeObjectAndName(name, i);
		min_offset = get_min_offset(node);
		// 最小偏移出现负数，说明有连接插入节点底部
		if (min_offset < 0.0)
		{
			record << std::setw(23) << name                       << " " 
				   << std::setw(15) << node->getInvertElevation() << " ";
			sub_node_invert(node, -min_offset);
			add_node_full_depth(node, -min_offset);
			record << node->getInvertElevation() << "\n";
			add_link_offset(node, -min_offset);
		}
	}

	// 5）处理连接露出地面的情况
	record << "\n\n";
	record << std::setw(23)  << "节点名称"     << " "
		   << std::setw(15)  << "原地面标高/m" << " "
		   << "新地面标高/m" << "\n";
	double max_crown;
	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		node = nodes->getNodeObjectAndName(name, i);
		if (node->isOutfall()) continue; // 排放口不检查
		max_crown = get_max_crown(node);
		// 露出地面
		if (max_crown > get_ground_elevation(node))
		{
			record << std::setw(23) << name                       << " " 
				   << std::setw(15) << get_ground_elevation(node) << " ";
			// 额外增加0.1mm，处理浮点数计算误差
			node->setFullDepth(max_crown - node->getInvertElevation() + 1.0E-4);
			record << get_ground_elevation(node) << "\n";
		}
	}

	// 6）处理管渠长度小于高差的情况，将其修改为高差的length_multiplier倍
	record << "\n\n";
	record << std::setw(23) << "管渠名称"     << " "
		   << std::setw(15) << "原管渠长度/m" << " "
		   << std::setw(15) << "新管渠长度/m" << " "
		   << "管渠坡降/m"  << "\n";
	ILink* link = nullptr;
	IConduit* conduit = nullptr;		
	for (int i = 0; i < links->getLinkCounts(); ++i)
	{
		link = links->getLinkObjectAndName(name, i);
		if (!link->isConduit()) continue;
		conduit = dynamic_cast<IConduit*>(link);
		if (is_link_error_length(conduit))
		{
			record << std::setw(23) << name                    << " " 
				   << std::setw(15) << conduit->getInpLength() << " ";
			conduit->setInpLength(length_multiplier * get_height_diff(conduit));
			record << std::setw(15) << conduit->getInpLength() << " "
				   << get_height_diff(conduit)                 << "\n";
		}
	}
	
	// 7）将处理后的模型保存为新的inp文件	
	if (!net->save(new_inp_file))
	{ 
		std::cout << "打开文件 " << new_inp_file << " 失败!" << "\n";
		return;
	}

	record.close();
	deleteNetwork(net);
}
/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.1.1
 * File       ：寻找大管接小管
 * Date       ：03/27/2023
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#include "global.h"
#include "large2small.h"

#include <iomanip>
#include <fstream>
#include <cassert>
#include <iostream>
#include <algorithm>

static IConduit* get_min_up_conduit(
	INode* node, double& min_up_geom1, bool& ignore)
//------------------------------------------------------------------------------
// 目的：获取上游最小管径的管渠连接
//------------------------------------------------------------------------------
{
	ignore                = false;
	min_up_geom1          = 1.0E6;   // 初始化上游最小管径为极大值
	ILink*    link        = nullptr;	
	IConduit* conduit     = nullptr;	
	IConduit* min_conduit = nullptr; // 最小管径对应的管渠连接

	for (int i = 0; i < node->getUpLinkCounts(); ++i)
	{
		// 存在非管渠连接，此时不检查大管或小管
		link = node->getUpLink(i);
		if (!link->isConduit())
		{
			ignore = true;
			return nullptr;
		}

		// 获取上游管渠
		conduit = dynamic_cast<IConduit*>(link);
		if (conduit->isDirectionChanged() != conduit->isOutDirection())
		{ 
			if (const double geom1 = conduit->getSection()->getGeom1();
				geom1 < min_up_geom1)
			{
				min_conduit = conduit;
				min_up_geom1 = geom1;
			}		
		}
	}

	for (int i = 0; i < node->getDnLinkCounts(); ++i)
	{
		// 存在非管渠连接，此时不检查大管或小管
		link = node->getDnLink(i);
		if (!link->isConduit())
		{
			ignore = true;
			return nullptr;
		}

		// 获取下游管渠（实际是上游）
		conduit = dynamic_cast<IConduit*>(link);
		if (conduit->isDirectionChanged() == conduit->isOutDirection())
		{
			if (const double geom1 = conduit->getSection()->getGeom1();
				geom1 < min_up_geom1)
			{
				min_conduit = conduit;
				min_up_geom1 = geom1;
			}
		}
	}

	return min_conduit;
}

static IConduit* get_max_dn_conduit(
	INode* node, double& max_dn_geom1, bool& ignore)
//------------------------------------------------------------------------------
// 目的：获取下游最大管径的管渠连接
//------------------------------------------------------------------------------
{
	ignore                = false;
	max_dn_geom1          = -1.0E6;  // 初始化下游最大管径为极小值
	ILink*    link        = nullptr;
	IConduit* conduit     = nullptr;
	IConduit* max_conduit = nullptr; // 最大管径对应的管渠连接

	for (int i = 0; i < node->getDnLinkCounts(); ++i)
	{
		// 存在非管渠连接，此时不检查大管或小管
		link = node->getDnLink(i);
		if (!link->isConduit()) 
		{
			ignore = true;
			return nullptr;
		}

		// 获取下游管渠
		conduit = dynamic_cast<IConduit*>(link);
		if (conduit->isDirectionChanged() != conduit->isOutDirection())
		{
			if (const double geom1 = conduit->getSection()->getGeom1();
				geom1 > max_dn_geom1)
			{
				max_conduit = conduit;
				max_dn_geom1 = geom1;
			}
		}		
	}

	for (int i = 0; i < node->getUpLinkCounts(); ++i)
	{
		// 存在非管渠连接，此时不检查大管或小管
		link = node->getUpLink(i);
		if (!link->isConduit())
		{
			ignore = true;
			return nullptr;
		}

		// 获取上游管渠（实际是下游）
		conduit = dynamic_cast<IConduit*>(link);
		if (conduit->isDirectionChanged() == conduit->isOutDirection())
		{
			if (const double geom1 = conduit->getSection()->getGeom1();
				geom1 > max_dn_geom1)
			{
				max_conduit = conduit;
				max_dn_geom1 = geom1;
			}
		}
	}

	return max_conduit;
}

int get_large2small_counts(INodeSet* nodes)
//------------------------------------------------------------------------------
// 目的：获取大管接小管的数量
//------------------------------------------------------------------------------
{		
	char      node_name[30];            // 占位用
	bool      ignore         = false;   // 是否关联非管渠连接
	INode*    node           = nullptr;	// 遍历用节点
	double    min_up_geom1   = 0.0;     // 上游最小管径
	IConduit* min_up_conduit = nullptr; // 上游最小管径对应的管渠连接
	double    max_dn_geom1   = 0.0;     // 下游最大管径
	IConduit* max_dn_conduit = nullptr; // 下游最大管径对应的管渠连接
	int       total_counts   = 0;       // 大管接小管的数量
	
	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		node = nodes->getNodeObjectAndName(node_name, i);		
		min_up_conduit = get_min_up_conduit(node, min_up_geom1, ignore);
		if (ignore) continue;
		max_dn_conduit = get_max_dn_conduit(node, max_dn_geom1, ignore);
		if (ignore) continue;

		// 必须同时具有上下游管渠连接
		if (min_up_conduit == nullptr || max_dn_conduit == nullptr)
			continue;

		// 大管接小管
		if (min_up_geom1 > max_dn_geom1)
			++total_counts;
	}

	return total_counts;
}

void record_large2small(const char* inp_file, const char* record_file)
//------------------------------------------------------------------------------
// 目的：检查inp_file，将大管接小管的信息保存在record_file
//------------------------------------------------------------------------------
{
	// 1）创建模型对象，读入inp_file，并检查数据
	INetwork* net = createNetwork();
	if (!net->readFromFile(inp_file))
	{
		char* error_text = new char[100000];  // 要确保足够大
		net->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return;
	}
	if (!net->validateData())
	{
		char* error_text = new char[100000];  // 要确保足够大
		net->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return;
	}

	// 2. 保存大管接小管信息
	std::ofstream ofs(record_file);
	ofs << std::setiosflags(std::ios::left);
	ofs << std::setw(23) << "大管名称"   << " "
		<< std::setw(23) << "小管名称"   << " "
		<< std::setw(23) << "大管管径/m" << " "
		<< "小管管径/m"  << "\n";

	char      node_name[30];            // 占位用
	bool      ignore         = false;   // 是否关联非管渠连接
	INode*    node           = nullptr;	// 遍历用节点
	double    min_up_geom1   = 0.0;     // 上游最小管径
	IConduit* min_up_conduit = nullptr; // 上游最小管径对应的管渠连接
	char      min_up_name[30];          // 上游最小管径对应的管渠连接名称
	double    max_dn_geom1   = 0.0;     // 下游最大管径
	IConduit* max_dn_conduit = nullptr; // 下游最大管径对应的管渠连接
	char      max_up_name[30];          // 下游最大管径对应的管渠连接名称
	INodeSet* nodes = net->getNodeSet();
	ILinkSet* links = net->getLinkSet();

	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		node = nodes->getNodeObjectAndName(node_name, i);		
		min_up_conduit = get_min_up_conduit(node, min_up_geom1, ignore);
		if (ignore) continue;
		max_dn_conduit = get_max_dn_conduit(node, max_dn_geom1, ignore);
		if (ignore) continue;

		// 必须同时具有上下游管渠连接
		if (min_up_conduit == nullptr || max_dn_conduit == nullptr)
			continue;

		// 大管接小管
		if (min_up_geom1 > max_dn_geom1)
		{
			links->getLinkName(min_up_conduit, min_up_name);
			links->getLinkName(max_dn_conduit, max_up_name);
			ofs << std::setw(23) << min_up_name  << " "
				<< std::setw(23) << max_up_name  << " "
				<< std::setw(23) << min_up_geom1 << " "
				<< max_dn_geom1  << "\n";
		}
	}

	ofs.close();
	deleteNetwork(net);
}

static IConduit* get_another_conduit(INode* node, ILink* link)
//------------------------------------------------------------------------------
// 目的：找到另一条管渠连接
//------------------------------------------------------------------------------
{
	assert(get_total_degree(node) == 2);

	ILink* another_link = nullptr;

	for (int i = 0; i < node->getUpLinkCounts(); ++i)
	{
		another_link = node->getUpLink(i);
		if (another_link == link)
			continue;
		else if (!another_link->isConduit())
			return nullptr;
		else
			return dynamic_cast<IConduit*>(another_link);
	}
	for (int i = 0; i < node->getDnLinkCounts(); ++i)
	{
		another_link = node->getDnLink(i);
		if (another_link == link)
			continue;
		else if (!another_link->isConduit())
			return nullptr;
		else
			return dynamic_cast<IConduit*>(another_link);
	}

	return nullptr;
}

int get_discordant_counts(ILinkSet* links)
//------------------------------------------------------------------------------
// 目的：获取直线段上不协调管道数量
//------------------------------------------------------------------------------
{
	char name[30];                    // 当前管渠连接名称	
	double geom1;                     // 当前管渠连接高度
	double beg_geom1;                 // 上游管渠连接高度
	double end_geom1;                 // 下游管渠连接高度
	ILink*    link         = nullptr; // 遍历用连接
	IConduit* conduit      = nullptr; // 遍历用管渠连接
	INode*    beg_node     = nullptr; // 起端节点
	IConduit* beg_conduit  = nullptr; // 上游管渠连接	
	INode*    end_node     = nullptr; // 终端节点
	IConduit* end_conduit  = nullptr; // 下游管渠连接
	int       total_counts = 0;       // 直线段上不协调管道数量

	for (int i = 0; i < links->getLinkCounts(); ++i)
	{
		link = links->getLinkObjectAndName(name, i);
		if (!link->isConduit()) continue;
		
		beg_node = link->getBeginNode();
		end_node = link->getEndNode();
		if (get_total_degree(beg_node) != 2 || get_total_degree(end_node) != 2)
			continue;

		beg_conduit = get_another_conduit(beg_node, link);
		end_conduit = get_another_conduit(end_node, link);
		if (beg_conduit == nullptr || end_conduit == nullptr)
			continue;

		conduit   = dynamic_cast<IConduit*>(link);
		geom1     = conduit->getSection()->getGeom1();
		beg_geom1 = beg_conduit->getSection()->getGeom1();
		end_geom1 = end_conduit->getSection()->getGeom1();

		// 不协调管道
		if ((geom1 < beg_geom1 && geom1 < end_geom1) ||
			(geom1 > beg_geom1 && geom1 > end_geom1))
			++total_counts;
	}

	return total_counts;
}

void record_discordant(const char* inp_file, const char* record_file)
//------------------------------------------------------------------------------
// 目的：检查inp_file，将直线段上不协调管道的信息保存在record_file
//------------------------------------------------------------------------------
{
	// 1）创建模型对象，读入inp_file，并检查数据
	INetwork* net = createNetwork();
	if (!net->readFromFile(inp_file))
	{
		char* error_text = new char[100000];  // 要确保足够大
		net->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return;
	}
	if (!net->validateData())
	{
		char* error_text = new char[100000];  // 要确保足够大
		net->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return;
	}

	// 2. 保存直线段上不协调管道的信息
	std::ofstream ofs(record_file);
	ofs << std::setiosflags(std::ios::left);
	ofs << std::setw(23)    << "中间管道名称"   << " "
		<< std::setw(23)    << "上游管道管径/m" << " "
		<< std::setw(23)    << "中间管道管径/m" << " "
		<< "下游管道管径/m" << "\n";

	char name[30];                   // 当前管渠连接名称	
	double geom1;                    // 当前管渠连接高度
	double beg_geom1;                // 上游管渠连接高度
	double end_geom1;                // 下游管渠连接高度
	ILink*    link        = nullptr; // 遍历用连接
	IConduit* conduit     = nullptr; // 遍历用管渠连接
	INode*    beg_node    = nullptr; // 起端节点
	IConduit* beg_conduit = nullptr; // 上游管渠连接	
	INode*    end_node    = nullptr; // 终端节点
	IConduit* end_conduit = nullptr; // 下游管渠连接
	ILinkSet* links = net->getLinkSet();

	for (int i = 0; i < links->getLinkCounts(); ++i)
	{
		link = links->getLinkObjectAndName(name, i);
		if (!link->isConduit()) continue;

		beg_node = link->getBeginNode();
		end_node = link->getEndNode();
		if (get_total_degree(beg_node) != 2 || get_total_degree(end_node) != 2)
			continue;

		beg_conduit = get_another_conduit(beg_node, link);
		end_conduit = get_another_conduit(end_node, link);
		if (beg_conduit == nullptr || end_conduit == nullptr)
			continue;

		conduit   = dynamic_cast<IConduit*>(link);
		geom1     = conduit->getSection()->getGeom1();
		beg_geom1 = beg_conduit->getSection()->getGeom1();
		end_geom1 = end_conduit->getSection()->getGeom1();

		// 不协调管道
		if ((geom1 < beg_geom1 && geom1 < end_geom1) ||
			(geom1 > beg_geom1 && geom1 > end_geom1))
		{
			ofs << std::setw(23) << name      << " "
				<< std::setw(23) << beg_geom1 << " "
				<< std::setw(23) << geom1     << " "
				<< end_geom1     << "\n";
		}
	}

	ofs.close();
	deleteNetwork(net);
}

int get_strict_discordant_counts(ILinkSet* links, int flag)
//------------------------------------------------------------------------------
// 目的：获取直线段上严格不协调管道数量
// 说明：严格不协调：上下游管渠管径相同，但当前管渠管径和上下游管渠管径不同
// 参数：flag =  0，代表当前管渠管径大于或小于上下游管渠管径 
//       flag =  1，代表当前管渠管径大于上下游管渠管径
//       flag = -1，代表当前管渠管径小于上下游管渠管径
//------------------------------------------------------------------------------
{
	char name[30];                    // 当前管渠连接名称	
	double geom1;                     // 当前管渠连接高度
	double beg_geom1;                 // 上游管渠连接高度
	double end_geom1;                 // 下游管渠连接高度
	ILink*    link         = nullptr; // 遍历用连接
	IConduit* conduit      = nullptr; // 遍历用管渠连接
	INode*    beg_node     = nullptr; // 起端节点
	IConduit* beg_conduit  = nullptr; // 上游管渠连接	
	INode*    end_node     = nullptr; // 终端节点
	IConduit* end_conduit  = nullptr; // 下游管渠连接
	int       total_counts = 0;       // 直线段上严格不协调管道数量

	for (int i = 0; i < links->getLinkCounts(); ++i)
	{
		link = links->getLinkObjectAndName(name, i);
		if (!link->isConduit()) continue;

		beg_node = link->getBeginNode();
		end_node = link->getEndNode();
		if (get_total_degree(beg_node) != 2 || get_total_degree(end_node) != 2)
			continue;

		beg_conduit = get_another_conduit(beg_node, link);
		end_conduit = get_another_conduit(end_node, link);
		if (beg_conduit == nullptr || end_conduit == nullptr)
			continue;

		conduit = dynamic_cast<IConduit*>(link);
		geom1 = conduit->getSection()->getGeom1();
		beg_geom1 = beg_conduit->getSection()->getGeom1();
		end_geom1 = end_conduit->getSection()->getGeom1();

		// 上下游管渠管径相同
		if (std::abs(beg_geom1 - end_geom1) < 1.0E-6) 
		{
			// 严格不协调管道
			if (flag <= 0 && geom1 < beg_geom1)
				++total_counts;
			if (flag >= 0 && geom1 > beg_geom1)
				++total_counts;
		}			
	}

	return total_counts;
}

void adjust_strict_discordant(const char* inp_file, const char* new_inp_file,
	const char* record_file, int flag)
//------------------------------------------------------------------------------
// 目的：检查inp_file，调整直线段上严格不协调管道管径，并记录调整过程
//------------------------------------------------------------------------------
{
	// 1）创建模型对象，读入inp_file，并检查数据
	INetwork* net = createNetwork();
	if (!net->readFromFile(inp_file))
	{
		char* error_text = new char[100000];  // 要确保足够大
		net->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return;
	}
	if (!net->validateData())
	{
		char* error_text = new char[100000];  // 要确保足够大
		net->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return;
	}

	// 2. 调整并记录严格不协调管道
	std::ofstream ofs(record_file);
	ofs << std::setiosflags(std::ios::left);
	ofs << std::setw(23)  << "被调整管道名称" << " "		
		<< std::setw(23)  << "调整前管径/m"   << " "
		<< "调整后管径/m" << "\n";

	char name[30];                   // 当前管渠连接名称	
	double geom1;                    // 当前管渠连接高度
	double beg_geom1;                // 上游管渠连接高度
	double end_geom1;                // 下游管渠连接高度
	ILink*    link        = nullptr; // 遍历用连接
	IConduit* conduit     = nullptr; // 遍历用管渠连接
	INode*    beg_node    = nullptr; // 起端节点
	IConduit* beg_conduit = nullptr; // 上游管渠连接	
	INode*    end_node    = nullptr; // 终端节点
	IConduit* end_conduit = nullptr; // 下游管渠连接
	ILinkSet* links       = net->getLinkSet();

	for (int i = 0; i < links->getLinkCounts(); ++i)
	{
		link = links->getLinkObjectAndName(name, i);
		if (!link->isConduit()) continue;

		beg_node = link->getBeginNode();
		end_node = link->getEndNode();
		if (get_total_degree(beg_node) != 2 || get_total_degree(end_node) != 2)
			continue;

		beg_conduit = get_another_conduit(beg_node, link);
		end_conduit = get_another_conduit(end_node, link);
		if (beg_conduit == nullptr || end_conduit == nullptr)
			continue;

		conduit   = dynamic_cast<IConduit*>(link);
		geom1     = conduit->getSection()->getGeom1();
		beg_geom1 = beg_conduit->getSection()->getGeom1();
		end_geom1 = end_conduit->getSection()->getGeom1();

		// 上下游管渠管径相同
		if (std::abs(beg_geom1 - end_geom1) < 1.0E-6)
		{
			// 严格不协调管道
			if (flag <= 0 && geom1 < beg_geom1)
			{ 
				conduit->getSection()->setGeom1(beg_geom1);
				ofs << std::setw(23) << name  << " "
					<< std::setw(23) << geom1 << " "
					<< beg_geom1     << "\n";
			}
			if (flag >= 0 && geom1 > beg_geom1)
			{ 
				conduit->getSection()->setGeom1(beg_geom1);
				ofs << std::setw(23) << name  << " "
					<< std::setw(23) << geom1 << " "
					<< beg_geom1     << "\n";
			}
		}
	}

	// 3. 保存修改后的inp文件
	if (!net->save(new_inp_file))
	{
		std::cout << "打开文件 " << new_inp_file << " 失败!" << "\n";
		return;
	}

	ofs.close();
	deleteNetwork(net);
}
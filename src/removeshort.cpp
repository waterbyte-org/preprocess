/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.1.1
 * File       ：删除短管
 * Date       ：03/27/2023
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#include "global.h"
#include "removeshort.h"

#include <set>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <iostream>

static double get_del_res_node(ILink* del_link, INode** del_node, 
	INode** res_node)
//------------------------------------------------------------------------------
// 目的：确定被删除节点和被保留节点，返回被删除节点和被保留节点的底高差  
//------------------------------------------------------------------------------
{
	// 如果端点为蓄水节点，则保留蓄水节点（注意：端点不可能是排放口节点！）
	if (*res_node = del_link->getBeginNode(); (*res_node)->isStorage())
	{
		*del_node = del_link->getEndNode();
		return (*del_node)->getInvertElevation() -
			(*res_node)->getInvertElevation();
	}
	else if (*res_node = del_link->getEndNode(); (*res_node)->isStorage())
	{
		*del_node = del_link->getBeginNode();
		return (*del_node)->getInvertElevation() -
			(*res_node)->getInvertElevation();
	}

	// 否则，保留底标高较小的节点
	const double delta = del_link->getBeginNode()->getInvertElevation() -
		del_link->getEndNode()->getInvertElevation();
	if (delta >= 0.0)
	{
		*del_node = del_link->getBeginNode();
		*res_node = del_link->getEndNode();
		return delta;
	}
	else
	{
		*del_node = del_link->getEndNode();
		*res_node = del_link->getBeginNode();
		return -delta;
	}
}

static void substitute_del_node(INetwork* net, INode* del_node, 
	const char* new_node_name)
//------------------------------------------------------------------------------
// 目的：将del_node的关联连接的del_node端，替换为new_node_name
//------------------------------------------------------------------------------
{
	char link_name[30];
	ILink* link = nullptr;
	INodeSet* nodes = net->getNodeSet();
	ILinkSet* links = net->getLinkSet();

	for (int i = 0; i < del_node->getUpLinkCounts(); ++i)
	{
		link = del_node->getUpLink(i);
		links->getLinkName(link, link_name);
		link->setEndNode(link_name, new_node_name, nodes, links);
	}
	for (int i = 0; i < del_node->getDnLinkCounts(); ++i)
	{
		link = del_node->getDnLink(i);
		links->getLinkName(link, link_name);
		link->setBeginNode(link_name, new_node_name, nodes, links);
	}
}

static double get_max_ground_elevation(INode* res_node, INode* del_node)
//------------------------------------------------------------------------------
// 返回：较大地面标高
//------------------------------------------------------------------------------
{
	return std::max(
		get_ground_elevation(res_node), get_ground_elevation(del_node));
}

static void adjust_full_depth(INode* res_node, double max_ground_elevation)
//------------------------------------------------------------------------------
// 目的：调整被保留节点的井深
//------------------------------------------------------------------------------
{
	// 新的井深，额外增加0.1mm
	const double res_full_depth =
		max_ground_elevation - res_node->getInvertElevation() + 1.0E-4;
	res_node->setFullDepth(res_full_depth);
}

static void remove_link(INetwork* net, const char* del_link_name, 
	const char* del_node_name, const char* res_node_name, double delta)
//------------------------------------------------------------------------------
// 目的：1）将del_node_name的入流数据整合至res_node_name
//       2）调整res_node_name的井底标高和井深
//       3）调整del_node_name的关联连接偏移值
//       4）将del_node_name的关联连接的del_node_name端，替换为new_node_name
//       5）删除del_node_name和del_link_name
//------------------------------------------------------------------------------
{
	INodeSet* nodes = net->getNodeSet();
	INode* del_node = nodes->getNodeObject(del_node_name);
	INode* res_node = nodes->getNodeObject(res_node_name);
	ILinkSet* links = net->getLinkSet();
	ILink* link = links->getLinkObject(del_link_name);

	// 1）将del_node_name的入流数据整合至res_node_name
	merge_node_inflow(net, del_node, res_node);

	// 2）调整res_node_name的井底标高和井深
	// 先找到较大的地面标高
	const double max_ground_elevation = 
		get_max_ground_elevation(res_node, del_node);
	// 修改井底标高
	if (delta < 0)
	{
		assert(res_node->isStorage());
		sub_node_invert(res_node, -delta);
	}
	adjust_full_depth(res_node, max_ground_elevation);

	// 3）调整del_node_name的关联连接偏移值
	if (delta < 0)
		add_link_offset(res_node, -delta);
	else
		add_link_offset(del_node, delta);

	// 4）将del_node_name的关联连接的del_node_name端，替换为new_node_name
	substitute_del_node(net, del_node, res_node_name);

	// 5）删除del_node_name和del_link_name
	links->removeLinkObject(del_link_name);
	nodes->removeNodeObject(del_node_name);
}

static bool is_shortest_link(ILink* link, double shortest)
//------------------------------------------------------------------------------
// 目的：检查是不是可以直接删除的最短连接？
//------------------------------------------------------------------------------
{
	// 必须是管渠连接才能删除
	if (!link->isConduit()) return false;
		
	INode* beg_node = link->getBeginNode();
	INode* end_node = link->getEndNode();

	// 两端节点不能是排放口（因为排放口不允许有入流，故不能合并）
	// 两端节点也不能都是蓄水节点
	if (beg_node->isOutfall() || end_node->isOutfall() ||
		(beg_node->isStorage() && end_node->isStorage()))
		return false;

	// 两端节点不能是叶子节点
	if (get_total_degree(beg_node) == 1 || get_total_degree(end_node) == 1)
		return false;

	return dynamic_cast<IConduit*>(link)->getInpLength() < shortest;
}

int get_shortest_link_counts(ILinkSet* links, double shortest)
//------------------------------------------------------------------------------
// 目的：统计极短连接的数量
//------------------------------------------------------------------------------
{
	char name[30];
	ILink* link = nullptr;
	int shortest_link_counts = 0;

	for (int i = 0; i < links->getLinkCounts(); ++i)
	{
		link = links->getLinkObjectAndName(name, i);
		if (is_shortest_link(link, shortest))
			++shortest_link_counts;
	}

	return shortest_link_counts;
}

static void record_shortest_link(INetwork* net, const char* res_node_name,
	const char* del_node_name, char* del_link_name, double delta, 
	std::ostringstream& oss)
//------------------------------------------------------------------------------
// 目的：记录删除极短连接过程中的信息
// 参数：net           = 用来查询对象的模型对象
//       res_node_name = 被保留节点的名称
//       del_node_name = 被删除节点的名称
//       del_link_name = 被删除连接的名称
//       delta         = 被删除节点和被保留节点之间的底高差
//------------------------------------------------------------------------------
{
	// 第1列，输出被删除连接的名称
	oss << std::setw(15) << del_link_name << " ";

	// 第2列，输出被保留节点的名称，它也负责接收被删除节点的入流
	oss << std::setw(15) << res_node_name << " ";

	// 第3列，输出被删除节点的名称
	oss << std::setw(15) << del_node_name << " ";

	// 第4-6列，输出是否整合了旱季流量、外部直接入流和RDII
	INodeSet* nodes = net->getNodeSet();
	INode* node = nodes->getNodeObject(del_node_name);
	record_node_inflow(node, oss);

	// 第7列，输出底高差
	oss << delta << "\n";
}

void remove_shortest_link(const char* inp_file, const char* new_inp_file,
	const char* record_file, double shortest)
//------------------------------------------------------------------------------
// 目的：调整inp_file，生成new_inp_file，并将过程记录在record_file
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

	// 2）创建空间信息对象，读入并检查空间信息数据
	IGeoprocess* igp = createGeoprocess();
	if (!igp->openGeoFile(inp_file))
	{
		char* error_text = new char[100000];
		igp->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return;
	}
	if (!igp->validateData())
	{
		char* error_text = new char[100000];
		igp->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return;
	}

	// 3）克隆模型，用于删除支管（注意：克隆模型不包含空间信息）
	INetwork* net_copy = net->cloneNetwork();

	// 4）创建字符串流
	std::ostringstream oss;
	oss << std::setiosflags(std::ios::left);
	oss << std::setw(15) << "被删除连接" << " "
		<< std::setw(15) << "被保留节点" << " "
		<< std::setw(15) << "被删除节点" << " "
		<< std::setw(11) << "整合DWF?"   << " "
		<< std::setw(11) << "整合EDF?"   << " "
		<< std::setw(11) << "整合RDII?"  << " "
		<< "底高差/m"    << "\n";

	// 5）删除极短连接	
	double delta = 0.0;         // 被删除节点和被保留节点的底高差
	char del_link_name[30];     // 被删除连接
	char del_node_name[30];     // 被删除节点
	char res_node_name[30];	    // 被保留节点
	ILink* del_link = nullptr;  // 被删除连接
	INode* del_node = nullptr;  // 被删除节点	
	INode* res_node = nullptr;  // 被保留节点
	INodeSet* nodes = net->getNodeSet();
	ILinkSet* links = net->getLinkSet();
	std::set<std::string> used_nodes; // 记录不能再使用的节点，防止出错

	for (int i = 0; i < links->getLinkCounts(); ++i)
	{
		del_link = links->getLinkObjectAndName(del_link_name, i);
		if (!is_shortest_link(del_link, shortest)) continue;

		// 获取被删除节点和被保留节点，以及它们底高差
		delta = get_del_res_node(del_link, &del_node, &res_node);

		// 两端不能是已经用过的节点
		nodes->getNodeName(del_node, del_node_name);
		nodes->getNodeName(res_node, res_node_name);
		if (used_nodes.find(del_node_name) != cend(used_nodes) ||
			used_nodes.find(res_node_name) != cend(used_nodes))
			continue;

		// 删除极短连接		
		record_shortest_link(net, res_node_name, del_node_name, del_link_name, 
			delta, oss);
		remove_link(
			net_copy, del_link_name, del_node_name, res_node_name, delta);

		// 将用过的节点加入used_nodes
		used_nodes.insert(del_node_name);
		used_nodes.insert(res_node_name);		
	}

	// 6）将处理后的模型保存为新的inp文件	
	if (!net_copy->save(new_inp_file))
	{
		std::cout << "打开文件 " << new_inp_file << " 失败!" << "\n";
		return;
	}
	if (!igp->saveGeoFile(record_file)) // 借用record_file临时保存空间信息数据
	{
		std::cout << "打开文件 " << record_file << " 失败!" << "\n";
		return;
	}
	merge_file(new_inp_file, record_file);

	// 7）保存调整过程信息
	std::ofstream record(record_file);
	record << oss.str();
	record.close();

	deleteNetwork(net);
	deleteNetwork(net_copy);
	deleteGeoprocess(igp);
}

static IConduit* get_unique_associate_conduit(INode* node, IConduit* del_link)
//------------------------------------------------------------------------------
// 目的：获取节点可以合并的唯一关联管渠连接
//------------------------------------------------------------------------------
{	
	assert(get_total_degree(node) == 2);

	// 只有连接节点可以删除
	if (!node->isJunction()) return nullptr;

	IConduit* conduit = nullptr;	
	for (int i = 0; i < node->getUpLinkCounts(); ++i)
	{
		conduit = dynamic_cast<IConduit*>(node->getUpLink(i));
		if (conduit != nullptr && conduit != del_link)
		{ 
			// 排放口节点不能接收入流
			if (conduit->getBeginNode()->isOutfall())
				return nullptr;
			// 管径必须相等
			if (conduit->getSection()->getGeom1() !=
				del_link->getSection()->getGeom1())
				return nullptr;				
			return conduit;
		}
	}
	for (int i = 0; i < node->getDnLinkCounts(); ++i)
	{
		conduit = dynamic_cast<IConduit*>(node->getDnLink(i));
		if (conduit != nullptr && conduit != del_link)
		{ 
			// 排放口节点不能接收入流
			if (conduit->getEndNode()->isOutfall())
				return nullptr;
			// 管径必须相等
			if (conduit->getSection()->getGeom1() !=
				del_link->getSection()->getGeom1())
				return nullptr;				
			return conduit;
		}
	}

	return nullptr;
}

static bool is_shorter_link(ILink* link, double shorter, double loss)
//------------------------------------------------------------------------------
// 目的：检查是不是可以合并的较短连接？
//------------------------------------------------------------------------------
{
	// 必须是管渠连接才能删除
	if (!link->isConduit()) return false;

	// 必须是较短连接
	IConduit* conduit = dynamic_cast<IConduit*>(link);
	const double length = conduit->getInpLength();
	if (length > shorter)
		return false;

	INode* beg_node = link->getBeginNode();
	INode* end_node = link->getEndNode();
	// 两端节点不能是排放口（因为排放口不允许有入流，故不能合并）
	// 两端节点也不能都是蓄水节点	
	if (beg_node->isOutfall() || end_node->isOutfall() ||
		(beg_node->isStorage() && end_node->isStorage()))
		return false;
	// 至少要有一端度为2
	if (get_total_degree(beg_node) != 2 && get_total_degree(end_node) != 2)
		return false;

	// 至少一端要有可以合并的管渠连接	
	IConduit* beg_conduit = nullptr;
	IConduit* end_conduit = nullptr;
	if (get_total_degree(beg_node) == 2)
		beg_conduit = get_unique_associate_conduit(beg_node, conduit);
	if (get_total_degree(end_node) == 2)
		end_conduit = get_unique_associate_conduit(end_node, conduit);
	if (beg_conduit == nullptr && end_conduit == nullptr)
		return false;

	// 至少一端要满足水头变化的要求
	double beg_loss = 1.0E6;
	double end_loss = 1.0E6;
	const double slope = conduit->getSlope();		
	if (beg_conduit)
		beg_loss = length * (slope - beg_conduit->getSlope());
	if (end_conduit)
		end_loss = length * (slope - end_conduit->getSlope());
	if (std::fabs(beg_loss) > loss && std::fabs(end_loss) > loss)
		return false;

	return true;
}

int get_shorter_link_counts(ILinkSet* links, double shorter, double loss)
//------------------------------------------------------------------------------
// 目的：统计较短连接的数量
//------------------------------------------------------------------------------
{
	char name[30];
	ILink* link = nullptr;
	int shorter_link_counts = 0;

	for (int i = 0; i < links->getLinkCounts(); ++i)
	{
		link = links->getLinkObjectAndName(name, i);
		if (is_shorter_link(link, shorter, loss))
			++shorter_link_counts;
	}

	return shorter_link_counts;
}

static double get_del_res_delta(INode* del_node, INode* res_node)
//------------------------------------------------------------------------------
// 目的：获取被删节点和被保留节点的井底高差，m
//------------------------------------------------------------------------------
{
	return del_node->getInvertElevation() - res_node->getInvertElevation();
}

static void remove_link_ex(INetwork* net, const char* del_link_name,
	const char* res_link_name, const char* del_node_name,
	const char* res_node_name, double delta, bool del_is_beg)
//------------------------------------------------------------------------------
// 参数：del_is_beg = true代表被删除节点为被删除连接起端，负数为终端      
// 目的：1）将del_node_name的入流数据整合至res_node_name
//       2）调整res_node_name的井底标高和井深
//       3）调整del_node_name的关联连接偏移值
//       4）将del_node_name的关联连接的del_node_name端，替换为new_node_name
//       5）删除del_node_name和del_link_name
//------------------------------------------------------------------------------
{
	INodeSet* nodes = net->getNodeSet();
	INode* del_node = nodes->getNodeObject(del_node_name);
	INode* res_node = nodes->getNodeObject(res_node_name);
	ILinkSet* links = net->getLinkSet();
	ILink* del_link = links->getLinkObject(del_link_name);
	ILink* res_link = links->getLinkObject(res_link_name);

	// 1）将del_node_name的入流数据整合至res_node_name
	merge_node_inflow(net, del_node, res_node);

	// 2）调整res_node_name的井底标高和井深
	// 先找到较大的地面标高
	const double max_ground_elevation =
		get_max_ground_elevation(res_node, del_node);
	// 调整res_node_name的关联连接偏移值
	assert(del_link->isConduit());
	IConduit* del_conduit = dynamic_cast<IConduit*>(del_link);
	const double extra_delta = (del_is_beg ? 
		(del_conduit->getSlope() * del_conduit->getInpLength()) : 0);
	if (delta < 0) 
	{ 
		sub_node_invert(res_node, -delta + extra_delta);
		add_link_offset(res_node, -delta + extra_delta);
		add_link_offset(del_node, extra_delta);
	}
	else
	{
		sub_node_invert(res_node, extra_delta);
		add_link_offset(res_node, extra_delta);
		add_link_offset(del_node, delta + extra_delta);
	}	
	adjust_full_depth(res_node, max_ground_elevation);

	// 3）调整res_link_name的管长
	assert(res_link->isConduit());
	IConduit* res_conduit = dynamic_cast<IConduit*>(res_link);	
	res_conduit->setInpLength(res_conduit->getInpLength() +
		del_conduit->getInpLength());	

	// 4）将del_node_name的关联连接的del_node_name端，替换为new_node_name
	substitute_del_node(net, del_node, res_node_name);

	// 5）删除del_node_name和del_link_name
	links->removeLinkObject(del_link_name);
	nodes->removeNodeObject(del_node_name);
}

static void record_shorter_link(INetwork* net, const char* res_node_name,
	const char* del_node_name, const char* res_link_name,
	char* del_link_name, std::ostringstream& oss)
//------------------------------------------------------------------------------
// 目的：记录删除极短连接过程中的信息
// 参数：net           = 用来查询对象的模型对象
//       res_node_name = 被保留节点的名称
//       del_node_name = 被删除节点的名称
//       res_link_name = 被保留连接的名称
//       del_link_name = 被删除连接的名称
//------------------------------------------------------------------------------
{
	// 第1-3行
	oss << std::setw(15) << del_link_name << " "
		<< std::setw(15) << del_node_name << " "
		<< std::setw(15) << res_node_name << " ";
		
	// 第4-6列，输出是否整合了旱季流量、外部直接入流和RDII
	INodeSet* nodes = net->getNodeSet();
	INode* node = nodes->getNodeObject(del_node_name);
	record_node_inflow(node, oss);

	// 第7-9列，输出被保留连接信息
	ILinkSet* links = net->getLinkSet();
	IConduit* res_link = dynamic_cast<IConduit*>(links->getLinkObject(
		res_link_name));
	IConduit* del_link = dynamic_cast<IConduit*>(links->getLinkObject(
		del_link_name));
	oss << std::setw(15) << res_link_name                      << " "
		<< std::setw(11) << res_link->getInpLength()           << " "
		<< res_link->getInpLength() + del_link->getInpLength() << "\n";
}

void remove_shorter_link(const char* inp_file, const char* new_inp_file, 
	const char* record_file, double shorter, double loss)
//------------------------------------------------------------------------------
// 目的：调整inp_file，生成new_inp_file，并将过程记录在record_file
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

	// 2）创建空间信息对象，读入并检查空间信息数据
	IGeoprocess* igp = createGeoprocess();
	if (!igp->openGeoFile(inp_file))
	{
		char* error_text = new char[100000];
		igp->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return;
	}
	if (!igp->validateData())
	{
		char* error_text = new char[100000];
		igp->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return;
	}

	// 3）克隆模型，用于删除支管（注意：克隆模型不包含空间信息）
	INetwork* net_copy = net->cloneNetwork();

	// 4）创建字符串流
	std::ostringstream oss;
	oss << std::setiosflags(std::ios::left);
	oss << std::setw(15) << "被删除连接" << " "
		<< std::setw(15) << "被删除节点" << " "
		<< std::setw(15) << "被保留节点" << " "		
		<< std::setw(11) << "整合DWF?"   << " "
		<< std::setw(11) << "整合EDF?"   << " "
		<< std::setw(11) << "整合RDII?"  << " "
		<< std::setw(15) << "被保留连接" << " "
		<< std::setw(11) << "原长度/m"   << " "
		<< "新长度/m"    << "\n";

	// 5）删除较短连接	
	double beg_loss   = 1.0E6;  // 起端水头改变值 
	double end_loss   = 1.0E6;  // 终端水头改变值 
	double del_slope  = 0.0;    // 被删除管渠坡度	
	double del_length = 0.0;    // 被删除管渠长度
	double del_res_delta = 0.0; // 被删除节点和被保留节点井底高差	
	char del_link_name[30];     // 被删除连接名称
	char del_node_name[30];     // 被删除节点名称
	char res_link_name[30];     // 被保留连接名称
	char res_node_name[30];     // 被保留节点名称
	INode* beg_node = nullptr;  // 起端节点
	INode* end_node = nullptr;	// 终端节点
	ILink* del_link = nullptr;  // 被删除连接
	IConduit* beg_conduit = nullptr; // 起端关联管渠
	IConduit* end_conduit = nullptr; // 终端关联管渠
	IConduit* del_conduit = nullptr; // 被删除管渠
	INodeSet* nodes = net->getNodeSet();
	ILinkSet* links = net->getLinkSet();
	std::set<std::string> used_nodes; // 记录不能再使用的节点，防止出错

	for (int i = 0; i < links->getLinkCounts(); ++i)
	{
		del_link = links->getLinkObjectAndName(del_link_name, i);
		
		// 必须是管渠连接才能删除
		if (!del_link->isConduit()) continue;

		// 必须是较短连接
		del_conduit = dynamic_cast<IConduit*>(del_link);
		del_length = del_conduit->getInpLength();
		if (del_length > shorter) continue;

		beg_node = del_link->getBeginNode();
		end_node = del_link->getEndNode();
		// 两端节点不能是排放口（因为排放口不允许有入流，故不能合并）
	    // 两端节点也不能都是蓄水节点	
		if (beg_node->isOutfall() || end_node->isOutfall() ||
			(beg_node->isStorage() && end_node->isStorage()))
			continue;
		// 至少要有一端度为2		
		if (get_total_degree(beg_node) != 2 && get_total_degree(end_node) != 2)
			continue;

		// 至少一端要有可以合并的管渠连接	
		beg_conduit = nullptr;
		end_conduit = nullptr;
		if (get_total_degree(beg_node) == 2)
			beg_conduit = get_unique_associate_conduit(beg_node, del_conduit);
		if (get_total_degree(end_node) == 2)
			end_conduit = get_unique_associate_conduit(end_node, del_conduit);			
		if (beg_conduit == nullptr && end_conduit == nullptr)
			continue;

		// 至少一端要满足水头变化的要求
		beg_loss = 1.0E6;
		end_loss = 1.0E6;
		del_slope = del_conduit->getSlope();
		if (beg_conduit)
			beg_loss = del_length * (del_slope - beg_conduit->getSlope());
		if (end_conduit)
			end_loss = del_length * (del_slope - end_conduit->getSlope());
		if (std::fabs(beg_loss) > loss && std::fabs(end_loss) > loss)
			continue;

		// 根据水头变化值决定和起端或终端管渠合并
		if (std::fabs(beg_loss) < std::fabs(end_loss))
		{
			assert(beg_conduit != nullptr);

			// 获取对象名称
			links->getLinkName(beg_conduit, res_link_name);
			nodes->getNodeName(beg_node, del_node_name);
			nodes->getNodeName(del_conduit->getEndNode(), res_node_name);

			// 检查是否能用
			if (used_nodes.find(del_node_name) != cend(used_nodes) ||
				used_nodes.find(res_node_name) != cend(used_nodes))
				continue;	

			// 删除较短连接
			del_res_delta = get_del_res_delta(
				beg_node, del_conduit->getEndNode());	
			record_shorter_link(net, res_node_name, del_node_name,
				res_link_name, del_link_name, oss);
			remove_link_ex(net_copy, del_link_name, res_link_name,
				del_node_name, res_node_name, del_res_delta, true);
		}
		else
		{
			assert(end_conduit != nullptr);

			// 获取对象名称
			links->getLinkName(end_conduit, res_link_name);
			nodes->getNodeName(end_node, del_node_name);
			nodes->getNodeName(del_conduit->getBeginNode(), res_node_name);
			
			// 检查是否能用
			if (used_nodes.find(del_node_name) != cend(used_nodes) ||
				used_nodes.find(res_node_name) != cend(used_nodes))
				continue;

			// 删除较短连接
			del_res_delta = get_del_res_delta(
				end_node, del_conduit->getBeginNode());
			record_shorter_link(net, res_node_name, del_node_name,
				res_link_name, del_link_name, oss);
			remove_link_ex(net_copy, del_link_name, res_link_name,
				del_node_name, res_node_name, del_res_delta, false);
		}		

		// 将用过的节点加入used_nodes
		used_nodes.insert(del_node_name);
		used_nodes.insert(res_node_name);
	}

	// 6）将处理后的模型保存为新的inp文件	
	if (!net_copy->save(new_inp_file))
	{
		std::cout << "打开文件 " << new_inp_file << " 失败!" << "\n";
		return;
	}
	if (!igp->saveGeoFile(record_file)) // 借用record_file临时保存空间信息数据
	{
		std::cout << "打开文件 " << record_file << " 失败!" << "\n";
		return;
	}
	merge_file(new_inp_file, record_file);

	// 7）保存调整过程信息
	std::ofstream record(record_file);
	record << oss.str();
	record.close();

	deleteNetwork(net);
	deleteNetwork(net_copy);
	deleteGeoprocess(igp);
}
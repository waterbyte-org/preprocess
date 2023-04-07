/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.1.1
 * File       ：删除支管
 * Date       ：03/27/2023
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#include "global.h"
#include "removebranch.h"

#include <iomanip>
#include <fstream>
#include <sstream>
#include <cassert>
#include <iostream>

static bool has_one_link_lateral_branch(INode* node)
//------------------------------------------------------------------------------
// 目的：检查节点是否有边侧单连接支管
//------------------------------------------------------------------------------
{
	// 节点度小于3，不可能有支管
	const int total_degree = get_total_degree(node);
	if (total_degree < 3) return false;
	
	// 节点没有叶子连接，不可能有单连接支管
	const int leaf_link_counts = get_leaf_link_counts(node);
	if (leaf_link_counts == 0) return false;

	// 非叶子连接至少要有2条，这样才能形成干管，也才会存在边侧单连接支管
	if ((total_degree - leaf_link_counts) < 2) return false;

	// 被删除节点必须是连接节点，被删除连接必须是管渠连接
	INode* del_node = nullptr;  // 被删除节点
	ILink* del_link = nullptr;  // 被删除连接
	for (int j = 0; j < node->getUpLinkCounts(); ++j)
	{
		del_link = node->getUpLink(j);
		del_node = del_link->getBeginNode();
		if (get_total_degree(del_node) == 1)
		{			
			if (del_node->isJunction() && del_link->isConduit())
				return true;
		}
	}
	for (int j = 0; j < node->getDnLinkCounts(); ++j)
	{
		del_link = node->getDnLink(j);
		del_node = del_link->getEndNode();
		if (get_total_degree(del_node) == 1)
		{
			if (del_node->isJunction() && del_link->isConduit())
				return true;
		}
	}
	return false;
}

int get_one_link_lateral_branch_counts(INodeSet* nodes)
//------------------------------------------------------------------------------
// 目的：统计存在边侧单连接支管的节点数
//------------------------------------------------------------------------------
{
	char name[30];
	INode* node = nullptr;
	int one_link_lateral_branch_counts = 0;

	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		node = nodes->getNodeObjectAndName(name, i);
		if (has_one_link_lateral_branch(node))
			++one_link_lateral_branch_counts;
	}

	return one_link_lateral_branch_counts;
}

static void remove_one_link_branch(INetwork* net, IGeoprocess* igp, 
	const char* res_node_name, const char* del_node_name, 
	const char* del_link_name)
//------------------------------------------------------------------------------
// 目的：删除单连接支管及其关联叶子节点
// 参数：net           = 用来删除对象的模型对象
//       igp           = 用来删除对象的空间信息对象
//       res_node_name = 被保留节点的名称
//       del_node_name = 被删除节点的名称
//       del_link_name = 被删除连接的名称
//------------------------------------------------------------------------------
{
	// 1）将del_node节点的入流整合至res_node节点
	INodeSet* nodes = net->getNodeSet();
	INode* del_node = nodes->getNodeObject(del_node_name);
	INode* res_node = nodes->getNodeObject(res_node_name);
	merge_node_inflow(net, del_node, res_node);

	// 2）从模型对象中删除del_node_name合del_link_name
	nodes->removeNodeObject(del_node_name);
	ILinkSet* links = net->getLinkSet();
	links->removeLinkObject(del_link_name);

	// 3）从空间信息对象中删除del_node_name合del_link_name
	ICoordinates* icoords = igp->getCoordinates();
	IVertices* iverts     = igp->getVertices();
	icoords->removeObject(del_node_name);
	iverts->removeObject(del_link_name);	
}

static void record_one_link_branch(INetwork* net, const char* res_node_name, 
	const char* del_node_name, char* del_link_name, std::ostringstream& oss)
//------------------------------------------------------------------------------
// 目的：记录删除单连接支管及其关联叶子节点过程中的信息
// 参数：net           = 用来查询对象的模型对象
//       res_node_name = 被保留节点的名称
//       del_node_name = 被删除节点的名称
//       del_link_name = 被删除连接的名称
//------------------------------------------------------------------------------
{
	// 第1列，输出被保留节点的名称，它也负责接收被删除节点的入流
	oss << std::setw(15) << res_node_name << " ";

	// 第2列，输出被删除节点的名称
	oss << std::setw(15) << del_node_name << " ";

	// 第3-5列，输出是否整合了旱季流量、外部直接入流和RDII
	INodeSet* nodes = net->getNodeSet();	
	INode* node = nodes->getNodeObject(del_node_name);
	record_node_inflow(node, oss);

	// 第6列，输出被删除连接的名称
	oss << del_link_name << "\n";
}

void remove_one_link_lateral_branch(const char* inp_file,
	const char* new_inp_file, const char* record_file)
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
	oss << std::setw(15) << "被保留节点" << " " 
		<< std::setw(15) << "被删除节点" << " "		
		<< std::setw(11) << "整合DWF?"   << " "
		<< std::setw(11) << "整合EDF?"   << " "
		<< std::setw(11) << "整合RDII?"  << " "
		<< "被删除连接"  << "\n";

	// 5）删除边侧单连接支管
	char res_node_name[30];     // 被保留节点的名称		
	char del_node_name[30];     // 被删除节点的名称
	char del_link_name[30];     // 被删除连接的名称
	INode* res_node = nullptr;  // 被保留节点
	INode* del_node = nullptr;  // 被删除节点
	ILink* del_link = nullptr;  // 被删除连接
	INodeSet* nodes = net->getNodeSet();
	ILinkSet* links = net->getLinkSet();
	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		res_node = nodes->getNodeObjectAndName(res_node_name, i);
		if (!has_one_link_lateral_branch(res_node)) continue;

		for (int j = 0; j < res_node->getUpLinkCounts(); ++j)
		{
			del_link = res_node->getUpLink(j);
			del_node = del_link->getBeginNode();
			// 只允许删除叶子连接
			if (get_total_degree(del_node) != 1) continue;
			// 只允许删除连接节点和管渠连接
			if (!del_node->isJunction()) continue;
			if (!del_link->isConduit()) continue;

			nodes->getNodeName(del_node, del_node_name);
			links->getLinkName(del_link, del_link_name);
			record_one_link_branch(
				net, res_node_name, del_node_name, del_link_name, oss);
			remove_one_link_branch(
				net_copy, igp, res_node_name, del_node_name, del_link_name);
		}
		for (int j = 0; j < res_node->getDnLinkCounts(); ++j)
		{
			del_link = res_node->getDnLink(j);
			del_node = del_link->getEndNode();
			// 只允许删除叶子连接
			if (get_total_degree(del_node) != 1) continue;
			// 只允许删除连接节点和管渠连接
			if (!del_node->isJunction()) continue;
			if (!del_link->isConduit()) continue;

			nodes->getNodeName(del_node, del_node_name);
			links->getLinkName(del_link, del_link_name);
			record_one_link_branch(
				net, res_node_name, del_node_name, del_link_name, oss);
			remove_one_link_branch(
				net_copy, igp, res_node_name, del_node_name, del_link_name);
		}
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

static double get_cos_value(const char* res_node_name, 
	const char* del_node_name, const char* node_name, IGeoprocess* igp)
//------------------------------------------------------------------------------
// 目的：获取res_node_name所在角的余弦值
//------------------------------------------------------------------------------
{
	// 从空间信息中找到3个节点坐标
	IMap* imap            = igp->getMap();
	ICoordinates* icoords = igp->getCoordinates();	
	double x0, y0, x1, y1, x2, y2;
	if (!icoords->getCoordinateByName(res_node_name, &x0, &y0, imap) ||
		!icoords->getCoordinateByName(del_node_name, &x1, &y1, imap) ||
		!icoords->getCoordinateByName(node_name,     &x2, &y2, imap))
		return -10.0;

	// 求三条边长度
	const double len_01_sqr = (x0 - x1) * (x0 - x1) + (y0 - y1) * (y0 - y1);
	const double len_01 = sqrt(len_01_sqr);
	const double len_02_sqr = (x0 - x2) * (x0 - x2) + (y0 - y2) * (y0 - y2);
	const double len_02 = sqrt(len_02_sqr);
	const double len_12_sqr = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);

	// 根据余弦定理求角(x2, y2)的余弦值
	return (len_01_sqr + len_02_sqr - len_12_sqr) / (2.0 * len_01 * len_02);
}

static bool is_one_link_terminal_branch(INode* del_node, INodeSet* nodes, 
	IGeoprocess* igp, double critical_cos, double critical_length)
//------------------------------------------------------------------------------
// 参数：critical_cos    = 临界余弦值
//       critical_length = 临界连接长度
// 目的：检查节点del_node的关联连接是否末端单连接支管
//------------------------------------------------------------------------------
{
	// 被删除节点必须是叶子连接节点
	if (!del_node->isJunction() || (get_total_degree(del_node) > 1)) 
		return false;

	// 被保留节点不能是排放口节点，被删除连接只能是管渠连接
	INode* res_node = nullptr;
	ILink* del_link = nullptr;	
	for (int i = 0; i < del_node->getDnLinkCounts(); ++i)
	{
		del_link = del_node->getDnLink(i);
		res_node = del_link->getEndNode();
	}
	for (int i = 0; i < del_node->getUpLinkCounts(); ++i)
	{
		del_link = del_node->getUpLink(i);
		res_node = del_link->getBeginNode();
	}
	assert(del_link != nullptr);
	assert(res_node != nullptr);
	if (res_node->isOutfall() || !del_link->isConduit())
		return false;

	// 被删除连接需要是短管
	IConduit* conduit = dynamic_cast<IConduit*>(del_link);
	if (conduit->getInpLength() > critical_length)
		return false;

	// 被保留节点只允许有一条关联非叶子连接		
	int not_leaf = 0;
	INode* node = nullptr;      // 被保留节点	
	INode* tmp_node = nullptr;  // 某节点	
	ILink* tmp_link = nullptr;  // 某连接
	for (int i = 0; i < res_node->getDnLinkCounts(); ++i)
	{
		tmp_link = res_node->getDnLink(i);
		tmp_node = tmp_link->getEndNode();
		if (get_total_degree(tmp_node) != 1)
		{
			++not_leaf;
			node = tmp_node;
		}
	}
	for (int i = 0; i < res_node->getUpLinkCounts(); ++i)
	{
		tmp_link = res_node->getUpLink(i);
		tmp_node = tmp_link->getBeginNode();
		if (get_total_degree(tmp_node) != 1)
		{
			++not_leaf;
			node = tmp_node;
		}
	}
	if (not_leaf != 1)	return false;

	// 获取需要的对象名称	
	char res_node_name[30];     // 被保留节点名称（计算角度）	
	char del_node_name[30];     // 被删除节点名称	
	char node_name[30];	        // 被保留节点名称
	nodes->getNodeName(res_node, res_node_name);
	nodes->getNodeName(del_node, del_node_name);
	nodes->getNodeName(node,     node_name);

	// 求末端单连接支管和干管夹角的余弦值
	const double cosvalue = get_cos_value(
		res_node_name, del_node_name, node_name, igp);

	return cosvalue > critical_cos;
}

int get_one_link_terminal_branch_counts(INodeSet* nodes, IGeoprocess* igp, 
	double critical_cos, double critical_length)
//------------------------------------------------------------------------------
// 参数：critical_cos    = 临界余弦值
//       critical_length = 临界连接长度
// 目的：统计存在末端单连接支管的节点数
//------------------------------------------------------------------------------
{
	char name[30];
	INode* node = nullptr;
	int one_link_terminal_branch_counts = 0;

	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		node = nodes->getNodeObjectAndName(name, i);
		if (is_one_link_terminal_branch(
			node, nodes, igp, critical_cos, critical_length))
			++one_link_terminal_branch_counts;
	}

	return one_link_terminal_branch_counts;
}

void remove_one_link_terminal_branch(const char* inp_file,
	const char* new_inp_file, const char* record_file, double critical_cos, 
	double critical_length)
//------------------------------------------------------------------------------
// 参数：critical_cos    = 临界余弦值
//       critical_length = 临界连接长度
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
	oss << std::setw(15) << "被保留节点" << " "
		<< std::setw(15) << "被删除节点" << " "
		<< std::setw(11) << "整合DWF?"   << " "
		<< std::setw(11) << "整合EDF?"   << " "
		<< std::setw(11) << "整合RDII?"  << " "
		<< "被删除连接"  << "\n";

	// 5）删除末端单连接支管
	double cosvalue;              // 末端单连接支管和干管夹角的余弦值
	char del_node_name[30];       // 被删除节点名称
	char res_node_name[30];       // 被保留节点名称（计算角度）	
	char node_name[30];	          // 被保留节点名称
	char del_link_name[30];       // 被删除连接名称
	INode* del_node   = nullptr;  // 被删除节点
	INode* res_node   = nullptr;  // 被保留节点(计算角度)		
	INode* node       = nullptr;  // 被保留节点	
	ILink* del_link   = nullptr;  // 被删除连接
	IConduit* conduit = nullptr;  // 被删除管渠连接
	INode* tmp_node   = nullptr;  // 某节点	
	ILink* tmp_link   = nullptr;  // 某连接
	INodeSet* nodes   = net->getNodeSet();
	ILinkSet* links   = net->getLinkSet();

	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		del_node = nodes->getNodeObjectAndName(del_node_name, i);

		// 被删除节点必须是叶子连接节点
		if (!del_node->isJunction() || (get_total_degree(del_node) > 1))
			continue;

		// 被保留节点不能是排放口节点，被删除连接只能是管渠连接		
		for (int j = 0; j < del_node->getDnLinkCounts(); ++j)
		{
			del_link = del_node->getDnLink(j);
			res_node = del_link->getEndNode();
		}
		for (int j = 0; j < del_node->getUpLinkCounts(); ++j)
		{
			del_link = del_node->getUpLink(j);
			res_node = del_link->getBeginNode();
		}
		assert(del_link != nullptr);
		assert(res_node != nullptr);
		if (res_node->isOutfall() || !del_link->isConduit())
			continue;

		// 被删除连接需要是短管
		IConduit* conduit = dynamic_cast<IConduit*>(del_link);
		if (conduit->getInpLength() > critical_length)
			continue;

		// 被保留节点只允许有一条关联非叶子连接	
		int not_leaf = 0;
		for (int j = 0; j < res_node->getDnLinkCounts(); ++j)
		{
			tmp_link = res_node->getDnLink(j);
			tmp_node = tmp_link->getEndNode();
			if (get_total_degree(tmp_node) != 1)
			{				
				++not_leaf;
				node = tmp_node;				
			}
		}
		for (int j = 0; j < res_node->getUpLinkCounts(); ++j)
		{
			tmp_link = res_node->getUpLink(j);
			tmp_node = tmp_link->getBeginNode();
			if (get_total_degree(tmp_node) != 1)
			{				
				++not_leaf;
				node = tmp_node;				
			}
		}
		if (not_leaf != 1) continue;

		// 获取需要的对象名称
		nodes->getNodeName(res_node, res_node_name);
		nodes->getNodeName(del_node, del_node_name);		
		nodes->getNodeName(node,     node_name);		

		// 求末端单连接支管和干管夹角的余弦值
		cosvalue = get_cos_value(res_node_name, del_node_name, node_name, igp);

		// 如果夹角比较小，可以删除
		if (cosvalue > critical_cos)
		{
			links->getLinkName(del_link, del_link_name);
			record_one_link_branch(
				net, res_node_name, del_node_name, del_link_name, oss);
			remove_one_link_branch(
				net_copy, igp, res_node_name, del_node_name, del_link_name);
		}
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

static bool has_two_link_lateral_branch(INode* del_node_1)
//------------------------------------------------------------------------------
// 目的：检查节点是否有边侧双连接支管
//------------------------------------------------------------------------------
{
	if (!del_node_1->isJunction() || get_total_degree(del_node_1) > 1) 
		return false;
		
	// 寻找关联连接和关联节点
	ILink* del_link_1 = nullptr;
	INode* del_node_2 = nullptr;
	for (int i = 0; i < del_node_1->getDnLinkCounts(); ++i)
	{
		del_link_1 = del_node_1->getDnLink(i);
		del_node_2 = del_link_1->getEndNode();
	}
	for (int i = 0; i < del_node_1->getUpLinkCounts(); ++i)
	{
		del_link_1 = del_node_1->getUpLink(i);
		del_node_2 = del_link_1->getBeginNode();
	}
	assert(del_link_1 != nullptr);
	assert(del_node_2 != nullptr);
	// 被删除连接必须是管渠连接
	if(!del_link_1->isConduit())
		return false;
	// 被删除节点必须是连接节点，且度必须是2
	if (!del_node_2->isJunction() || get_total_degree(del_node_2) != 2) 
		return false;

	// 继续寻找关联连接和关联节点
	ILink* del_link_2 = nullptr;
	INode* res_node   = nullptr;
	for (int i = 0; i < del_node_2->getDnLinkCounts(); ++i)
	{
		del_link_2 = del_node_2->getDnLink(i);
		if (del_link_2 != del_link_1)
		{
			res_node = del_link_2->getEndNode();
			break;
		}
	}
	for (int i = 0; i < del_node_2->getUpLinkCounts(); ++i)
	{		
		if (res_node != nullptr) break; // 已经找到了

		del_link_2 = del_node_2->getUpLink(i);
		if (del_link_2 != del_link_1)
		{
			res_node = del_link_2->getBeginNode();
			break;
		}
	}
	assert(del_link_2 != nullptr);
	assert(res_node != nullptr);
	// 被删除连接必须是管渠连接
	if (!del_link_2->isConduit())
		return false;
	// 被保留节点不能是排放口节点，且非叶子连接数必须大于2
	if (res_node->isOutfall() ||
		(get_total_degree(res_node) - get_leaf_link_counts(res_node) < 3))
		return false;
		
	return true;
}

int get_two_link_lateral_branch_counts(INodeSet* nodes)
//------------------------------------------------------------------------------
// 目的：统计存在边侧双连接支管的节点数
//------------------------------------------------------------------------------
{
	char name[30];
	INode* node = nullptr;
	int two_link_lateral_branch_counts = 0;

	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		node = nodes->getNodeObjectAndName(name, i);
		if (has_two_link_lateral_branch(node))
			++two_link_lateral_branch_counts;
	}

	return two_link_lateral_branch_counts;
}

static void record_two_link_branch(INetwork* net, const char* res_node_name,
	const char* del_node_name_1, const char* del_node_name_2,
	const char* del_link_name_1, const char* del_link_name_2, 
	std::ostringstream& oss)
//------------------------------------------------------------------------------
// 目的：记录删除双连接支管及其关联节点过程中的信息
// 参数：net             = 用来查询对象的模型对象
//       res_node_name   = 被保留节点的名称
//       del_node_name_1 = 被删除节点1的名称
//       del_node_name_2 = 被删除节点2的名称
//       del_link_name_1 = 被删除连接1的名称
//       del_link_name_2 = 被删除连接2的名称
//------------------------------------------------------------------------------
{
	INodeSet* nodes = net->getNodeSet();

	// 第1列，输出被保留节点的名称，它也负责接收被删除节点的入流
	oss << std::setw(15) << res_node_name << " ";

	// 第2-5列，输出被删除节点1的信息	
	oss << std::setw(15) << del_node_name_1 << " ";
	INode* node = nodes->getNodeObject(del_node_name_1);
	record_node_inflow(node, oss);

	// 第6-8列，输出被删除节点1的信息	
	oss << std::setw(15) << del_node_name_2 << " ";
	node = nodes->getNodeObject(del_node_name_2);
	record_node_inflow(node, oss);

	// 第9-10列，输出被删除连接信息
	oss << std::setw(15)   << del_link_name_1 << " "
		<< del_link_name_2 << "\n";
}

void remove_two_link_lateral_branch(const char* inp_file,
	const char* new_inp_file, const char* record_file)
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
	oss << std::setw(15) << "被保留节点"  << " "
		<< std::setw(15) << "被删除节点1" << " "
		<< std::setw(11) << "整合DWF?"    << " "
		<< std::setw(11) << "整合EDF?"    << " "
		<< std::setw(11) << "整合RDII?"   << " "
		<< std::setw(15) << "被删除节点2" << " "
		<< std::setw(11) << "整合DWF?"    << " "
		<< std::setw(11) << "整合EDF?"    << " "
		<< std::setw(11) << "整合RDII?"   << " "
		<< std::setw(15) << "被删除连接1" << " "
		<< "被删除连接2" << "\n";

	// 5）删除边侧双连接支管
	char res_node_name[30];       // 被保留节点名称
	char del_node_name_1[30];     // 被删除节点名称1	
	char del_node_name_2[30];     // 被删除节点名称2
	char del_link_name_1[30];     // 被删除连接名称1
	char del_link_name_2[30];     // 被删除连接名称2
	INode* res_node = nullptr;    // 被保留节点
	INode* del_node_1 = nullptr;  // 被删除节点1	
	INode* del_node_2 = nullptr;  // 被删除节点2	
	ILink* del_link_1 = nullptr;  // 被删除连接1
	ILink* del_link_2 = nullptr;  // 被删除连接2
	INodeSet* nodes = net->getNodeSet();
	ILinkSet* links = net->getLinkSet();

	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		del_node_1 = nodes->getNodeObjectAndName(del_node_name_1, i);
		if (!del_node_1->isJunction() || get_total_degree(del_node_1) > 1)
			continue;
	
		// 寻找关联连接和关联节点
		for (int j = 0; j < del_node_1->getDnLinkCounts(); ++j)
		{
			del_link_1 = del_node_1->getDnLink(j);
			del_node_2 = del_link_1->getEndNode();
		}
		for (int j = 0; j < del_node_1->getUpLinkCounts(); ++j)
		{
			del_link_1 = del_node_1->getUpLink(j);
			del_node_2 = del_link_1->getBeginNode();
		}
		assert(del_link_1 != nullptr);
		assert(del_node_2 != nullptr);
		// 被删除连接必须是管渠连接
		if (!del_link_1->isConduit())
			continue;
		// 被删除节点必须是连接节点，且度必须是2
		if (!del_node_2->isJunction() || get_total_degree(del_node_2) != 2)
			continue;

		// 继续寻找关联连接和关联节点		
		res_node = nullptr; // 必须要有
		for (int j = 0; j < del_node_2->getDnLinkCounts(); ++j)
		{
			del_link_2 = del_node_2->getDnLink(j);
			if (del_link_2 != del_link_1)
			{
				res_node = del_link_2->getEndNode();
				break;
			}
		}
		for (int j = 0; j < del_node_2->getUpLinkCounts(); ++j)
		{			
			if (res_node != nullptr) break; // 已经找到了
			del_link_2 = del_node_2->getUpLink(j);
			if (del_link_2 != del_link_1)
			{
				res_node = del_link_2->getBeginNode();
				break;
			}
		}
		assert(del_link_2 != nullptr);
		assert(res_node != nullptr);
		// 被删除连接必须是管渠连接
		if (!del_link_2->isConduit())
			continue;
		// 被保留节点不能是排放口节点，且非叶子连接数必须大于2
		if (res_node->isOutfall() ||
			(get_total_degree(res_node) - get_leaf_link_counts(res_node) < 3))
			continue;

		// 获取对象名称
		nodes->getNodeName(del_node_2, del_node_name_2);
		nodes->getNodeName(res_node,   res_node_name);
		links->getLinkName(del_link_1, del_link_name_1);
		links->getLinkName(del_link_2, del_link_name_2);
		
		record_two_link_branch(net, res_node_name, del_node_name_1,
			del_node_name_2, del_link_name_1, del_link_name_2, oss);
		remove_one_link_branch(
			net_copy, igp, del_node_name_2, del_node_name_1, del_link_name_1);
		remove_one_link_branch(
			net_copy, igp, res_node_name, del_node_name_2, del_link_name_2);
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
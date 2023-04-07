/*
 *******************************************************************************
 * Project    ��PreProcess
 * Version    ��1.1.1
 * File       ��ɾ���̹�
 * Date       ��03/27/2023
 * Author     ����С��
 * Copyright  ������ˮ�ֽڿƼ����޹�˾
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
// Ŀ�ģ�ȷ����ɾ���ڵ�ͱ������ڵ㣬���ر�ɾ���ڵ�ͱ������ڵ�ĵ׸߲�  
//------------------------------------------------------------------------------
{
	// ����˵�Ϊ��ˮ�ڵ㣬������ˮ�ڵ㣨ע�⣺�˵㲻�������ŷſڽڵ㣡��
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

	// ���򣬱����ױ�߽�С�Ľڵ�
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
// Ŀ�ģ���del_node�Ĺ������ӵ�del_node�ˣ��滻Ϊnew_node_name
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
// ���أ��ϴ������
//------------------------------------------------------------------------------
{
	return std::max(
		get_ground_elevation(res_node), get_ground_elevation(del_node));
}

static void adjust_full_depth(INode* res_node, double max_ground_elevation)
//------------------------------------------------------------------------------
// Ŀ�ģ������������ڵ�ľ���
//------------------------------------------------------------------------------
{
	// �µľ����������0.1mm
	const double res_full_depth =
		max_ground_elevation - res_node->getInvertElevation() + 1.0E-4;
	res_node->setFullDepth(res_full_depth);
}

static void remove_link(INetwork* net, const char* del_link_name, 
	const char* del_node_name, const char* res_node_name, double delta)
//------------------------------------------------------------------------------
// Ŀ�ģ�1����del_node_name����������������res_node_name
//       2������res_node_name�ľ��ױ�ߺ;���
//       3������del_node_name�Ĺ�������ƫ��ֵ
//       4����del_node_name�Ĺ������ӵ�del_node_name�ˣ��滻Ϊnew_node_name
//       5��ɾ��del_node_name��del_link_name
//------------------------------------------------------------------------------
{
	INodeSet* nodes = net->getNodeSet();
	INode* del_node = nodes->getNodeObject(del_node_name);
	INode* res_node = nodes->getNodeObject(res_node_name);
	ILinkSet* links = net->getLinkSet();
	ILink* link = links->getLinkObject(del_link_name);

	// 1����del_node_name����������������res_node_name
	merge_node_inflow(net, del_node, res_node);

	// 2������res_node_name�ľ��ױ�ߺ;���
	// ���ҵ��ϴ�ĵ�����
	const double max_ground_elevation = 
		get_max_ground_elevation(res_node, del_node);
	// �޸ľ��ױ��
	if (delta < 0)
	{
		assert(res_node->isStorage());
		sub_node_invert(res_node, -delta);
	}
	adjust_full_depth(res_node, max_ground_elevation);

	// 3������del_node_name�Ĺ�������ƫ��ֵ
	if (delta < 0)
		add_link_offset(res_node, -delta);
	else
		add_link_offset(del_node, delta);

	// 4����del_node_name�Ĺ������ӵ�del_node_name�ˣ��滻Ϊnew_node_name
	substitute_del_node(net, del_node, res_node_name);

	// 5��ɾ��del_node_name��del_link_name
	links->removeLinkObject(del_link_name);
	nodes->removeNodeObject(del_node_name);
}

static bool is_shortest_link(ILink* link, double shortest)
//------------------------------------------------------------------------------
// Ŀ�ģ�����ǲ��ǿ���ֱ��ɾ����������ӣ�
//------------------------------------------------------------------------------
{
	// �����ǹ������Ӳ���ɾ��
	if (!link->isConduit()) return false;
		
	INode* beg_node = link->getBeginNode();
	INode* end_node = link->getEndNode();

	// ���˽ڵ㲻�����ŷſڣ���Ϊ�ŷſڲ��������������ʲ��ܺϲ���
	// ���˽ڵ�Ҳ���ܶ�����ˮ�ڵ�
	if (beg_node->isOutfall() || end_node->isOutfall() ||
		(beg_node->isStorage() && end_node->isStorage()))
		return false;

	// ���˽ڵ㲻����Ҷ�ӽڵ�
	if (get_total_degree(beg_node) == 1 || get_total_degree(end_node) == 1)
		return false;

	return dynamic_cast<IConduit*>(link)->getInpLength() < shortest;
}

int get_shortest_link_counts(ILinkSet* links, double shortest)
//------------------------------------------------------------------------------
// Ŀ�ģ�ͳ�Ƽ������ӵ�����
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
// Ŀ�ģ���¼ɾ���������ӹ����е���Ϣ
// ������net           = ������ѯ�����ģ�Ͷ���
//       res_node_name = �������ڵ������
//       del_node_name = ��ɾ���ڵ������
//       del_link_name = ��ɾ�����ӵ�����
//       delta         = ��ɾ���ڵ�ͱ������ڵ�֮��ĵ׸߲�
//------------------------------------------------------------------------------
{
	// ��1�У������ɾ�����ӵ�����
	oss << std::setw(15) << del_link_name << " ";

	// ��2�У�����������ڵ�����ƣ���Ҳ������ձ�ɾ���ڵ������
	oss << std::setw(15) << res_node_name << " ";

	// ��3�У������ɾ���ڵ������
	oss << std::setw(15) << del_node_name << " ";

	// ��4-6�У�����Ƿ������˺����������ⲿֱ��������RDII
	INodeSet* nodes = net->getNodeSet();
	INode* node = nodes->getNodeObject(del_node_name);
	record_node_inflow(node, oss);

	// ��7�У�����׸߲�
	oss << delta << "\n";
}

void remove_shortest_link(const char* inp_file, const char* new_inp_file,
	const char* record_file, double shortest)
//------------------------------------------------------------------------------
// Ŀ�ģ�����inp_file������new_inp_file���������̼�¼��record_file
//------------------------------------------------------------------------------
{
	// 1������ģ�Ͷ��󣬶���inp_file�����������
	INetwork* net = createNetwork();
	if (!net->readFromFile(inp_file))
	{
		char* error_text = new char[100000];  // Ҫȷ���㹻��
		net->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return;
	}
	if (!net->validateData())
	{
		char* error_text = new char[100000];  // Ҫȷ���㹻��
		net->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return;
	}

	// 2�������ռ���Ϣ���󣬶��벢���ռ���Ϣ����
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

	// 3����¡ģ�ͣ�����ɾ��֧�ܣ�ע�⣺��¡ģ�Ͳ������ռ���Ϣ��
	INetwork* net_copy = net->cloneNetwork();

	// 4�������ַ�����
	std::ostringstream oss;
	oss << std::setiosflags(std::ios::left);
	oss << std::setw(15) << "��ɾ������" << " "
		<< std::setw(15) << "�������ڵ�" << " "
		<< std::setw(15) << "��ɾ���ڵ�" << " "
		<< std::setw(11) << "����DWF?"   << " "
		<< std::setw(11) << "����EDF?"   << " "
		<< std::setw(11) << "����RDII?"  << " "
		<< "�׸߲�/m"    << "\n";

	// 5��ɾ����������	
	double delta = 0.0;         // ��ɾ���ڵ�ͱ������ڵ�ĵ׸߲�
	char del_link_name[30];     // ��ɾ������
	char del_node_name[30];     // ��ɾ���ڵ�
	char res_node_name[30];	    // �������ڵ�
	ILink* del_link = nullptr;  // ��ɾ������
	INode* del_node = nullptr;  // ��ɾ���ڵ�	
	INode* res_node = nullptr;  // �������ڵ�
	INodeSet* nodes = net->getNodeSet();
	ILinkSet* links = net->getLinkSet();
	std::set<std::string> used_nodes; // ��¼������ʹ�õĽڵ㣬��ֹ����

	for (int i = 0; i < links->getLinkCounts(); ++i)
	{
		del_link = links->getLinkObjectAndName(del_link_name, i);
		if (!is_shortest_link(del_link, shortest)) continue;

		// ��ȡ��ɾ���ڵ�ͱ������ڵ㣬�Լ����ǵ׸߲�
		delta = get_del_res_node(del_link, &del_node, &res_node);

		// ���˲������Ѿ��ù��Ľڵ�
		nodes->getNodeName(del_node, del_node_name);
		nodes->getNodeName(res_node, res_node_name);
		if (used_nodes.find(del_node_name) != cend(used_nodes) ||
			used_nodes.find(res_node_name) != cend(used_nodes))
			continue;

		// ɾ����������		
		record_shortest_link(net, res_node_name, del_node_name, del_link_name, 
			delta, oss);
		remove_link(
			net_copy, del_link_name, del_node_name, res_node_name, delta);

		// ���ù��Ľڵ����used_nodes
		used_nodes.insert(del_node_name);
		used_nodes.insert(res_node_name);		
	}

	// 6����������ģ�ͱ���Ϊ�µ�inp�ļ�	
	if (!net_copy->save(new_inp_file))
	{
		std::cout << "���ļ� " << new_inp_file << " ʧ��!" << "\n";
		return;
	}
	if (!igp->saveGeoFile(record_file)) // ����record_file��ʱ����ռ���Ϣ����
	{
		std::cout << "���ļ� " << record_file << " ʧ��!" << "\n";
		return;
	}
	merge_file(new_inp_file, record_file);

	// 7���������������Ϣ
	std::ofstream record(record_file);
	record << oss.str();
	record.close();

	deleteNetwork(net);
	deleteNetwork(net_copy);
	deleteGeoprocess(igp);
}

static IConduit* get_unique_associate_conduit(INode* node, IConduit* del_link)
//------------------------------------------------------------------------------
// Ŀ�ģ���ȡ�ڵ���Ժϲ���Ψһ������������
//------------------------------------------------------------------------------
{	
	assert(get_total_degree(node) == 2);

	// ֻ�����ӽڵ����ɾ��
	if (!node->isJunction()) return nullptr;

	IConduit* conduit = nullptr;	
	for (int i = 0; i < node->getUpLinkCounts(); ++i)
	{
		conduit = dynamic_cast<IConduit*>(node->getUpLink(i));
		if (conduit != nullptr && conduit != del_link)
		{ 
			// �ŷſڽڵ㲻�ܽ�������
			if (conduit->getBeginNode()->isOutfall())
				return nullptr;
			// �ܾ��������
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
			// �ŷſڽڵ㲻�ܽ�������
			if (conduit->getEndNode()->isOutfall())
				return nullptr;
			// �ܾ��������
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
// Ŀ�ģ�����ǲ��ǿ��Ժϲ��Ľ϶����ӣ�
//------------------------------------------------------------------------------
{
	// �����ǹ������Ӳ���ɾ��
	if (!link->isConduit()) return false;

	// �����ǽ϶�����
	IConduit* conduit = dynamic_cast<IConduit*>(link);
	const double length = conduit->getInpLength();
	if (length > shorter)
		return false;

	INode* beg_node = link->getBeginNode();
	INode* end_node = link->getEndNode();
	// ���˽ڵ㲻�����ŷſڣ���Ϊ�ŷſڲ��������������ʲ��ܺϲ���
	// ���˽ڵ�Ҳ���ܶ�����ˮ�ڵ�	
	if (beg_node->isOutfall() || end_node->isOutfall() ||
		(beg_node->isStorage() && end_node->isStorage()))
		return false;
	// ����Ҫ��һ�˶�Ϊ2
	if (get_total_degree(beg_node) != 2 && get_total_degree(end_node) != 2)
		return false;

	// ����һ��Ҫ�п��Ժϲ��Ĺ�������	
	IConduit* beg_conduit = nullptr;
	IConduit* end_conduit = nullptr;
	if (get_total_degree(beg_node) == 2)
		beg_conduit = get_unique_associate_conduit(beg_node, conduit);
	if (get_total_degree(end_node) == 2)
		end_conduit = get_unique_associate_conduit(end_node, conduit);
	if (beg_conduit == nullptr && end_conduit == nullptr)
		return false;

	// ����һ��Ҫ����ˮͷ�仯��Ҫ��
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
// Ŀ�ģ�ͳ�ƽ϶����ӵ�����
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
// Ŀ�ģ���ȡ��ɾ�ڵ�ͱ������ڵ�ľ��׸߲m
//------------------------------------------------------------------------------
{
	return del_node->getInvertElevation() - res_node->getInvertElevation();
}

static void remove_link_ex(INetwork* net, const char* del_link_name,
	const char* res_link_name, const char* del_node_name,
	const char* res_node_name, double delta, bool del_is_beg)
//------------------------------------------------------------------------------
// ������del_is_beg = true����ɾ���ڵ�Ϊ��ɾ��������ˣ�����Ϊ�ն�      
// Ŀ�ģ�1����del_node_name����������������res_node_name
//       2������res_node_name�ľ��ױ�ߺ;���
//       3������del_node_name�Ĺ�������ƫ��ֵ
//       4����del_node_name�Ĺ������ӵ�del_node_name�ˣ��滻Ϊnew_node_name
//       5��ɾ��del_node_name��del_link_name
//------------------------------------------------------------------------------
{
	INodeSet* nodes = net->getNodeSet();
	INode* del_node = nodes->getNodeObject(del_node_name);
	INode* res_node = nodes->getNodeObject(res_node_name);
	ILinkSet* links = net->getLinkSet();
	ILink* del_link = links->getLinkObject(del_link_name);
	ILink* res_link = links->getLinkObject(res_link_name);

	// 1����del_node_name����������������res_node_name
	merge_node_inflow(net, del_node, res_node);

	// 2������res_node_name�ľ��ױ�ߺ;���
	// ���ҵ��ϴ�ĵ�����
	const double max_ground_elevation =
		get_max_ground_elevation(res_node, del_node);
	// ����res_node_name�Ĺ�������ƫ��ֵ
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

	// 3������res_link_name�Ĺܳ�
	assert(res_link->isConduit());
	IConduit* res_conduit = dynamic_cast<IConduit*>(res_link);	
	res_conduit->setInpLength(res_conduit->getInpLength() +
		del_conduit->getInpLength());	

	// 4����del_node_name�Ĺ������ӵ�del_node_name�ˣ��滻Ϊnew_node_name
	substitute_del_node(net, del_node, res_node_name);

	// 5��ɾ��del_node_name��del_link_name
	links->removeLinkObject(del_link_name);
	nodes->removeNodeObject(del_node_name);
}

static void record_shorter_link(INetwork* net, const char* res_node_name,
	const char* del_node_name, const char* res_link_name,
	char* del_link_name, std::ostringstream& oss)
//------------------------------------------------------------------------------
// Ŀ�ģ���¼ɾ���������ӹ����е���Ϣ
// ������net           = ������ѯ�����ģ�Ͷ���
//       res_node_name = �������ڵ������
//       del_node_name = ��ɾ���ڵ������
//       res_link_name = ���������ӵ�����
//       del_link_name = ��ɾ�����ӵ�����
//------------------------------------------------------------------------------
{
	// ��1-3��
	oss << std::setw(15) << del_link_name << " "
		<< std::setw(15) << del_node_name << " "
		<< std::setw(15) << res_node_name << " ";
		
	// ��4-6�У�����Ƿ������˺����������ⲿֱ��������RDII
	INodeSet* nodes = net->getNodeSet();
	INode* node = nodes->getNodeObject(del_node_name);
	record_node_inflow(node, oss);

	// ��7-9�У����������������Ϣ
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
// Ŀ�ģ�����inp_file������new_inp_file���������̼�¼��record_file
//------------------------------------------------------------------------------
{
	// 1������ģ�Ͷ��󣬶���inp_file�����������
	INetwork* net = createNetwork();
	if (!net->readFromFile(inp_file))
	{
		char* error_text = new char[100000];  // Ҫȷ���㹻��
		net->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return;
	}
	if (!net->validateData())
	{
		char* error_text = new char[100000];  // Ҫȷ���㹻��
		net->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return;
	}

	// 2�������ռ���Ϣ���󣬶��벢���ռ���Ϣ����
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

	// 3����¡ģ�ͣ�����ɾ��֧�ܣ�ע�⣺��¡ģ�Ͳ������ռ���Ϣ��
	INetwork* net_copy = net->cloneNetwork();

	// 4�������ַ�����
	std::ostringstream oss;
	oss << std::setiosflags(std::ios::left);
	oss << std::setw(15) << "��ɾ������" << " "
		<< std::setw(15) << "��ɾ���ڵ�" << " "
		<< std::setw(15) << "�������ڵ�" << " "		
		<< std::setw(11) << "����DWF?"   << " "
		<< std::setw(11) << "����EDF?"   << " "
		<< std::setw(11) << "����RDII?"  << " "
		<< std::setw(15) << "����������" << " "
		<< std::setw(11) << "ԭ����/m"   << " "
		<< "�³���/m"    << "\n";

	// 5��ɾ���϶�����	
	double beg_loss   = 1.0E6;  // ���ˮͷ�ı�ֵ 
	double end_loss   = 1.0E6;  // �ն�ˮͷ�ı�ֵ 
	double del_slope  = 0.0;    // ��ɾ�������¶�	
	double del_length = 0.0;    // ��ɾ����������
	double del_res_delta = 0.0; // ��ɾ���ڵ�ͱ������ڵ㾮�׸߲�	
	char del_link_name[30];     // ��ɾ����������
	char del_node_name[30];     // ��ɾ���ڵ�����
	char res_link_name[30];     // ��������������
	char res_node_name[30];     // �������ڵ�����
	INode* beg_node = nullptr;  // ��˽ڵ�
	INode* end_node = nullptr;	// �ն˽ڵ�
	ILink* del_link = nullptr;  // ��ɾ������
	IConduit* beg_conduit = nullptr; // ��˹�������
	IConduit* end_conduit = nullptr; // �ն˹�������
	IConduit* del_conduit = nullptr; // ��ɾ������
	INodeSet* nodes = net->getNodeSet();
	ILinkSet* links = net->getLinkSet();
	std::set<std::string> used_nodes; // ��¼������ʹ�õĽڵ㣬��ֹ����

	for (int i = 0; i < links->getLinkCounts(); ++i)
	{
		del_link = links->getLinkObjectAndName(del_link_name, i);
		
		// �����ǹ������Ӳ���ɾ��
		if (!del_link->isConduit()) continue;

		// �����ǽ϶�����
		del_conduit = dynamic_cast<IConduit*>(del_link);
		del_length = del_conduit->getInpLength();
		if (del_length > shorter) continue;

		beg_node = del_link->getBeginNode();
		end_node = del_link->getEndNode();
		// ���˽ڵ㲻�����ŷſڣ���Ϊ�ŷſڲ��������������ʲ��ܺϲ���
	    // ���˽ڵ�Ҳ���ܶ�����ˮ�ڵ�	
		if (beg_node->isOutfall() || end_node->isOutfall() ||
			(beg_node->isStorage() && end_node->isStorage()))
			continue;
		// ����Ҫ��һ�˶�Ϊ2		
		if (get_total_degree(beg_node) != 2 && get_total_degree(end_node) != 2)
			continue;

		// ����һ��Ҫ�п��Ժϲ��Ĺ�������	
		beg_conduit = nullptr;
		end_conduit = nullptr;
		if (get_total_degree(beg_node) == 2)
			beg_conduit = get_unique_associate_conduit(beg_node, del_conduit);
		if (get_total_degree(end_node) == 2)
			end_conduit = get_unique_associate_conduit(end_node, del_conduit);			
		if (beg_conduit == nullptr && end_conduit == nullptr)
			continue;

		// ����һ��Ҫ����ˮͷ�仯��Ҫ��
		beg_loss = 1.0E6;
		end_loss = 1.0E6;
		del_slope = del_conduit->getSlope();
		if (beg_conduit)
			beg_loss = del_length * (del_slope - beg_conduit->getSlope());
		if (end_conduit)
			end_loss = del_length * (del_slope - end_conduit->getSlope());
		if (std::fabs(beg_loss) > loss && std::fabs(end_loss) > loss)
			continue;

		// ����ˮͷ�仯ֵ��������˻��ն˹����ϲ�
		if (std::fabs(beg_loss) < std::fabs(end_loss))
		{
			assert(beg_conduit != nullptr);

			// ��ȡ��������
			links->getLinkName(beg_conduit, res_link_name);
			nodes->getNodeName(beg_node, del_node_name);
			nodes->getNodeName(del_conduit->getEndNode(), res_node_name);

			// ����Ƿ�����
			if (used_nodes.find(del_node_name) != cend(used_nodes) ||
				used_nodes.find(res_node_name) != cend(used_nodes))
				continue;	

			// ɾ���϶�����
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

			// ��ȡ��������
			links->getLinkName(end_conduit, res_link_name);
			nodes->getNodeName(end_node, del_node_name);
			nodes->getNodeName(del_conduit->getBeginNode(), res_node_name);
			
			// ����Ƿ�����
			if (used_nodes.find(del_node_name) != cend(used_nodes) ||
				used_nodes.find(res_node_name) != cend(used_nodes))
				continue;

			// ɾ���϶�����
			del_res_delta = get_del_res_delta(
				end_node, del_conduit->getBeginNode());
			record_shorter_link(net, res_node_name, del_node_name,
				res_link_name, del_link_name, oss);
			remove_link_ex(net_copy, del_link_name, res_link_name,
				del_node_name, res_node_name, del_res_delta, false);
		}		

		// ���ù��Ľڵ����used_nodes
		used_nodes.insert(del_node_name);
		used_nodes.insert(res_node_name);
	}

	// 6����������ģ�ͱ���Ϊ�µ�inp�ļ�	
	if (!net_copy->save(new_inp_file))
	{
		std::cout << "���ļ� " << new_inp_file << " ʧ��!" << "\n";
		return;
	}
	if (!igp->saveGeoFile(record_file)) // ����record_file��ʱ����ռ���Ϣ����
	{
		std::cout << "���ļ� " << record_file << " ʧ��!" << "\n";
		return;
	}
	merge_file(new_inp_file, record_file);

	// 7���������������Ϣ
	std::ofstream record(record_file);
	record << oss.str();
	record.close();

	deleteNetwork(net);
	deleteNetwork(net_copy);
	deleteGeoprocess(igp);
}
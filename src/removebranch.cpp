/*
 *******************************************************************************
 * Project    ��PreProcess
 * Version    ��1.1.1
 * File       ��ɾ��֧��
 * Date       ��03/27/2023
 * Author     ����С��
 * Copyright  ������ˮ�ֽڿƼ����޹�˾
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
// Ŀ�ģ����ڵ��Ƿ��б߲൥����֧��
//------------------------------------------------------------------------------
{
	// �ڵ��С��3����������֧��
	const int total_degree = get_total_degree(node);
	if (total_degree < 3) return false;
	
	// �ڵ�û��Ҷ�����ӣ��������е�����֧��
	const int leaf_link_counts = get_leaf_link_counts(node);
	if (leaf_link_counts == 0) return false;

	// ��Ҷ����������Ҫ��2�������������γɸɹܣ�Ҳ�Ż���ڱ߲൥����֧��
	if ((total_degree - leaf_link_counts) < 2) return false;

	// ��ɾ���ڵ���������ӽڵ㣬��ɾ�����ӱ����ǹ�������
	INode* del_node = nullptr;  // ��ɾ���ڵ�
	ILink* del_link = nullptr;  // ��ɾ������
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
// Ŀ�ģ�ͳ�ƴ��ڱ߲൥����֧�ܵĽڵ���
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
// Ŀ�ģ�ɾ��������֧�ܼ������Ҷ�ӽڵ�
// ������net           = ����ɾ�������ģ�Ͷ���
//       igp           = ����ɾ������Ŀռ���Ϣ����
//       res_node_name = �������ڵ������
//       del_node_name = ��ɾ���ڵ������
//       del_link_name = ��ɾ�����ӵ�����
//------------------------------------------------------------------------------
{
	// 1����del_node�ڵ������������res_node�ڵ�
	INodeSet* nodes = net->getNodeSet();
	INode* del_node = nodes->getNodeObject(del_node_name);
	INode* res_node = nodes->getNodeObject(res_node_name);
	merge_node_inflow(net, del_node, res_node);

	// 2����ģ�Ͷ�����ɾ��del_node_name��del_link_name
	nodes->removeNodeObject(del_node_name);
	ILinkSet* links = net->getLinkSet();
	links->removeLinkObject(del_link_name);

	// 3���ӿռ���Ϣ������ɾ��del_node_name��del_link_name
	ICoordinates* icoords = igp->getCoordinates();
	IVertices* iverts     = igp->getVertices();
	icoords->removeObject(del_node_name);
	iverts->removeObject(del_link_name);	
}

static void record_one_link_branch(INetwork* net, const char* res_node_name, 
	const char* del_node_name, char* del_link_name, std::ostringstream& oss)
//------------------------------------------------------------------------------
// Ŀ�ģ���¼ɾ��������֧�ܼ������Ҷ�ӽڵ�����е���Ϣ
// ������net           = ������ѯ�����ģ�Ͷ���
//       res_node_name = �������ڵ������
//       del_node_name = ��ɾ���ڵ������
//       del_link_name = ��ɾ�����ӵ�����
//------------------------------------------------------------------------------
{
	// ��1�У�����������ڵ�����ƣ���Ҳ������ձ�ɾ���ڵ������
	oss << std::setw(15) << res_node_name << " ";

	// ��2�У������ɾ���ڵ������
	oss << std::setw(15) << del_node_name << " ";

	// ��3-5�У�����Ƿ������˺����������ⲿֱ��������RDII
	INodeSet* nodes = net->getNodeSet();	
	INode* node = nodes->getNodeObject(del_node_name);
	record_node_inflow(node, oss);

	// ��6�У������ɾ�����ӵ�����
	oss << del_link_name << "\n";
}

void remove_one_link_lateral_branch(const char* inp_file,
	const char* new_inp_file, const char* record_file)
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
	oss << std::setw(15) << "�������ڵ�" << " " 
		<< std::setw(15) << "��ɾ���ڵ�" << " "		
		<< std::setw(11) << "����DWF?"   << " "
		<< std::setw(11) << "����EDF?"   << " "
		<< std::setw(11) << "����RDII?"  << " "
		<< "��ɾ������"  << "\n";

	// 5��ɾ���߲൥����֧��
	char res_node_name[30];     // �������ڵ������		
	char del_node_name[30];     // ��ɾ���ڵ������
	char del_link_name[30];     // ��ɾ�����ӵ�����
	INode* res_node = nullptr;  // �������ڵ�
	INode* del_node = nullptr;  // ��ɾ���ڵ�
	ILink* del_link = nullptr;  // ��ɾ������
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
			// ֻ����ɾ��Ҷ������
			if (get_total_degree(del_node) != 1) continue;
			// ֻ����ɾ�����ӽڵ�͹�������
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
			// ֻ����ɾ��Ҷ������
			if (get_total_degree(del_node) != 1) continue;
			// ֻ����ɾ�����ӽڵ�͹�������
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

static double get_cos_value(const char* res_node_name, 
	const char* del_node_name, const char* node_name, IGeoprocess* igp)
//------------------------------------------------------------------------------
// Ŀ�ģ���ȡres_node_name���ڽǵ�����ֵ
//------------------------------------------------------------------------------
{
	// �ӿռ���Ϣ���ҵ�3���ڵ�����
	IMap* imap            = igp->getMap();
	ICoordinates* icoords = igp->getCoordinates();	
	double x0, y0, x1, y1, x2, y2;
	if (!icoords->getCoordinateByName(res_node_name, &x0, &y0, imap) ||
		!icoords->getCoordinateByName(del_node_name, &x1, &y1, imap) ||
		!icoords->getCoordinateByName(node_name,     &x2, &y2, imap))
		return -10.0;

	// �������߳���
	const double len_01_sqr = (x0 - x1) * (x0 - x1) + (y0 - y1) * (y0 - y1);
	const double len_01 = sqrt(len_01_sqr);
	const double len_02_sqr = (x0 - x2) * (x0 - x2) + (y0 - y2) * (y0 - y2);
	const double len_02 = sqrt(len_02_sqr);
	const double len_12_sqr = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);

	// �������Ҷ������(x2, y2)������ֵ
	return (len_01_sqr + len_02_sqr - len_12_sqr) / (2.0 * len_01 * len_02);
}

static bool is_one_link_terminal_branch(INode* del_node, INodeSet* nodes, 
	IGeoprocess* igp, double critical_cos, double critical_length)
//------------------------------------------------------------------------------
// ������critical_cos    = �ٽ�����ֵ
//       critical_length = �ٽ����ӳ���
// Ŀ�ģ����ڵ�del_node�Ĺ��������Ƿ�ĩ�˵�����֧��
//------------------------------------------------------------------------------
{
	// ��ɾ���ڵ������Ҷ�����ӽڵ�
	if (!del_node->isJunction() || (get_total_degree(del_node) > 1)) 
		return false;

	// �������ڵ㲻�����ŷſڽڵ㣬��ɾ������ֻ���ǹ�������
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

	// ��ɾ��������Ҫ�Ƕ̹�
	IConduit* conduit = dynamic_cast<IConduit*>(del_link);
	if (conduit->getInpLength() > critical_length)
		return false;

	// �������ڵ�ֻ������һ��������Ҷ������		
	int not_leaf = 0;
	INode* node = nullptr;      // �������ڵ�	
	INode* tmp_node = nullptr;  // ĳ�ڵ�	
	ILink* tmp_link = nullptr;  // ĳ����
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

	// ��ȡ��Ҫ�Ķ�������	
	char res_node_name[30];     // �������ڵ����ƣ�����Ƕȣ�	
	char del_node_name[30];     // ��ɾ���ڵ�����	
	char node_name[30];	        // �������ڵ�����
	nodes->getNodeName(res_node, res_node_name);
	nodes->getNodeName(del_node, del_node_name);
	nodes->getNodeName(node,     node_name);

	// ��ĩ�˵�����֧�ܺ͸ɹܼнǵ�����ֵ
	const double cosvalue = get_cos_value(
		res_node_name, del_node_name, node_name, igp);

	return cosvalue > critical_cos;
}

int get_one_link_terminal_branch_counts(INodeSet* nodes, IGeoprocess* igp, 
	double critical_cos, double critical_length)
//------------------------------------------------------------------------------
// ������critical_cos    = �ٽ�����ֵ
//       critical_length = �ٽ����ӳ���
// Ŀ�ģ�ͳ�ƴ���ĩ�˵�����֧�ܵĽڵ���
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
// ������critical_cos    = �ٽ�����ֵ
//       critical_length = �ٽ����ӳ���
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
	oss << std::setw(15) << "�������ڵ�" << " "
		<< std::setw(15) << "��ɾ���ڵ�" << " "
		<< std::setw(11) << "����DWF?"   << " "
		<< std::setw(11) << "����EDF?"   << " "
		<< std::setw(11) << "����RDII?"  << " "
		<< "��ɾ������"  << "\n";

	// 5��ɾ��ĩ�˵�����֧��
	double cosvalue;              // ĩ�˵�����֧�ܺ͸ɹܼнǵ�����ֵ
	char del_node_name[30];       // ��ɾ���ڵ�����
	char res_node_name[30];       // �������ڵ����ƣ�����Ƕȣ�	
	char node_name[30];	          // �������ڵ�����
	char del_link_name[30];       // ��ɾ����������
	INode* del_node   = nullptr;  // ��ɾ���ڵ�
	INode* res_node   = nullptr;  // �������ڵ�(����Ƕ�)		
	INode* node       = nullptr;  // �������ڵ�	
	ILink* del_link   = nullptr;  // ��ɾ������
	IConduit* conduit = nullptr;  // ��ɾ����������
	INode* tmp_node   = nullptr;  // ĳ�ڵ�	
	ILink* tmp_link   = nullptr;  // ĳ����
	INodeSet* nodes   = net->getNodeSet();
	ILinkSet* links   = net->getLinkSet();

	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		del_node = nodes->getNodeObjectAndName(del_node_name, i);

		// ��ɾ���ڵ������Ҷ�����ӽڵ�
		if (!del_node->isJunction() || (get_total_degree(del_node) > 1))
			continue;

		// �������ڵ㲻�����ŷſڽڵ㣬��ɾ������ֻ���ǹ�������		
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

		// ��ɾ��������Ҫ�Ƕ̹�
		IConduit* conduit = dynamic_cast<IConduit*>(del_link);
		if (conduit->getInpLength() > critical_length)
			continue;

		// �������ڵ�ֻ������һ��������Ҷ������	
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

		// ��ȡ��Ҫ�Ķ�������
		nodes->getNodeName(res_node, res_node_name);
		nodes->getNodeName(del_node, del_node_name);		
		nodes->getNodeName(node,     node_name);		

		// ��ĩ�˵�����֧�ܺ͸ɹܼнǵ�����ֵ
		cosvalue = get_cos_value(res_node_name, del_node_name, node_name, igp);

		// ����нǱȽ�С������ɾ��
		if (cosvalue > critical_cos)
		{
			links->getLinkName(del_link, del_link_name);
			record_one_link_branch(
				net, res_node_name, del_node_name, del_link_name, oss);
			remove_one_link_branch(
				net_copy, igp, res_node_name, del_node_name, del_link_name);
		}
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

static bool has_two_link_lateral_branch(INode* del_node_1)
//------------------------------------------------------------------------------
// Ŀ�ģ����ڵ��Ƿ��б߲�˫����֧��
//------------------------------------------------------------------------------
{
	if (!del_node_1->isJunction() || get_total_degree(del_node_1) > 1) 
		return false;
		
	// Ѱ�ҹ������Ӻ͹����ڵ�
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
	// ��ɾ�����ӱ����ǹ�������
	if(!del_link_1->isConduit())
		return false;
	// ��ɾ���ڵ���������ӽڵ㣬�Ҷȱ�����2
	if (!del_node_2->isJunction() || get_total_degree(del_node_2) != 2) 
		return false;

	// ����Ѱ�ҹ������Ӻ͹����ڵ�
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
		if (res_node != nullptr) break; // �Ѿ��ҵ���

		del_link_2 = del_node_2->getUpLink(i);
		if (del_link_2 != del_link_1)
		{
			res_node = del_link_2->getBeginNode();
			break;
		}
	}
	assert(del_link_2 != nullptr);
	assert(res_node != nullptr);
	// ��ɾ�����ӱ����ǹ�������
	if (!del_link_2->isConduit())
		return false;
	// �������ڵ㲻�����ŷſڽڵ㣬�ҷ�Ҷ���������������2
	if (res_node->isOutfall() ||
		(get_total_degree(res_node) - get_leaf_link_counts(res_node) < 3))
		return false;
		
	return true;
}

int get_two_link_lateral_branch_counts(INodeSet* nodes)
//------------------------------------------------------------------------------
// Ŀ�ģ�ͳ�ƴ��ڱ߲�˫����֧�ܵĽڵ���
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
// Ŀ�ģ���¼ɾ��˫����֧�ܼ�������ڵ�����е���Ϣ
// ������net             = ������ѯ�����ģ�Ͷ���
//       res_node_name   = �������ڵ������
//       del_node_name_1 = ��ɾ���ڵ�1������
//       del_node_name_2 = ��ɾ���ڵ�2������
//       del_link_name_1 = ��ɾ������1������
//       del_link_name_2 = ��ɾ������2������
//------------------------------------------------------------------------------
{
	INodeSet* nodes = net->getNodeSet();

	// ��1�У�����������ڵ�����ƣ���Ҳ������ձ�ɾ���ڵ������
	oss << std::setw(15) << res_node_name << " ";

	// ��2-5�У������ɾ���ڵ�1����Ϣ	
	oss << std::setw(15) << del_node_name_1 << " ";
	INode* node = nodes->getNodeObject(del_node_name_1);
	record_node_inflow(node, oss);

	// ��6-8�У������ɾ���ڵ�1����Ϣ	
	oss << std::setw(15) << del_node_name_2 << " ";
	node = nodes->getNodeObject(del_node_name_2);
	record_node_inflow(node, oss);

	// ��9-10�У������ɾ��������Ϣ
	oss << std::setw(15)   << del_link_name_1 << " "
		<< del_link_name_2 << "\n";
}

void remove_two_link_lateral_branch(const char* inp_file,
	const char* new_inp_file, const char* record_file)
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
	oss << std::setw(15) << "�������ڵ�"  << " "
		<< std::setw(15) << "��ɾ���ڵ�1" << " "
		<< std::setw(11) << "����DWF?"    << " "
		<< std::setw(11) << "����EDF?"    << " "
		<< std::setw(11) << "����RDII?"   << " "
		<< std::setw(15) << "��ɾ���ڵ�2" << " "
		<< std::setw(11) << "����DWF?"    << " "
		<< std::setw(11) << "����EDF?"    << " "
		<< std::setw(11) << "����RDII?"   << " "
		<< std::setw(15) << "��ɾ������1" << " "
		<< "��ɾ������2" << "\n";

	// 5��ɾ���߲�˫����֧��
	char res_node_name[30];       // �������ڵ�����
	char del_node_name_1[30];     // ��ɾ���ڵ�����1	
	char del_node_name_2[30];     // ��ɾ���ڵ�����2
	char del_link_name_1[30];     // ��ɾ����������1
	char del_link_name_2[30];     // ��ɾ����������2
	INode* res_node = nullptr;    // �������ڵ�
	INode* del_node_1 = nullptr;  // ��ɾ���ڵ�1	
	INode* del_node_2 = nullptr;  // ��ɾ���ڵ�2	
	ILink* del_link_1 = nullptr;  // ��ɾ������1
	ILink* del_link_2 = nullptr;  // ��ɾ������2
	INodeSet* nodes = net->getNodeSet();
	ILinkSet* links = net->getLinkSet();

	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		del_node_1 = nodes->getNodeObjectAndName(del_node_name_1, i);
		if (!del_node_1->isJunction() || get_total_degree(del_node_1) > 1)
			continue;
	
		// Ѱ�ҹ������Ӻ͹����ڵ�
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
		// ��ɾ�����ӱ����ǹ�������
		if (!del_link_1->isConduit())
			continue;
		// ��ɾ���ڵ���������ӽڵ㣬�Ҷȱ�����2
		if (!del_node_2->isJunction() || get_total_degree(del_node_2) != 2)
			continue;

		// ����Ѱ�ҹ������Ӻ͹����ڵ�		
		res_node = nullptr; // ����Ҫ��
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
			if (res_node != nullptr) break; // �Ѿ��ҵ���
			del_link_2 = del_node_2->getUpLink(j);
			if (del_link_2 != del_link_1)
			{
				res_node = del_link_2->getBeginNode();
				break;
			}
		}
		assert(del_link_2 != nullptr);
		assert(res_node != nullptr);
		// ��ɾ�����ӱ����ǹ�������
		if (!del_link_2->isConduit())
			continue;
		// �������ڵ㲻�����ŷſڽڵ㣬�ҷ�Ҷ���������������2
		if (res_node->isOutfall() ||
			(get_total_degree(res_node) - get_leaf_link_counts(res_node) < 3))
			continue;

		// ��ȡ��������
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
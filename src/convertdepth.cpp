/*
 *******************************************************************************
 * Project    ��PreProcess
 * Version    ��1.1.1
 * File       �������߶����
 * Date       ��03/27/2023
 * Author     ����С��
 * Copyright  ������ˮ�ֽڿƼ����޹�˾
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
// Ŀ�ģ���ȡ�ڵ�node�Ĺ������ӵ���Сƫ��
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
// Ŀ�ģ���ȡ�ڵ�node�Ĺ������ӵ���󶥱��
// ע�⣺�������е����Ӷ��к�������
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
// Ŀ�ģ���ȡ�����ڵ׸߲�
//------------------------------------------------------------------------------
{
	return std::fabs(conduit->getBeginNode()->getInvertElevation() +
		conduit->getBeginOffset() - conduit->getEndNode()->getInvertElevation()
		- conduit->getEndOffset());
}

static bool is_link_error_length(IConduit* conduit)
//------------------------------------------------------------------------------
// Ŀ�ģ�������Ӹ߲��Ƿ���ڵ��ڳ��ȣ�
//------------------------------------------------------------------------------
{	
	// ֱ�Ǳ߱���С��б��
	return get_height_diff(conduit) >= conduit->getInpLength();
}

int get_concave_counts(INodeSet* nodes)
//------------------------------------------------------------------------------
// Ŀ�ģ�ͳ�����Ӳ���ڵ�ײ��Ľڵ�����
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
// Ŀ�ģ�ͳ������¶������Ľڵ�����
//------------------------------------------------------------------------------
{
	char name[30];
	INode* node = nullptr;
	int convex_counts = 0;

	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		node = nodes->getNodeObjectAndName(name, i);		
		if (node->isOutfall()) continue; // �ŷſڲ����

		if (get_max_crown(node) > get_ground_elevation(node))
			++convex_counts;
	}

	return convex_counts;
}

int get_error_link_counts(ILinkSet* links)
//------------------------------------------------------------------------------
// Ŀ�ģ�ͳ�Ƹ߲���ڵ��ڳ��ȵ���������
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
// ������length_multiplier = �����ӳ�����
// Ŀ�ģ�����inp_file������new_inp_file���������̼�¼��record_file
//------------------------------------------------------------------------------
{
	// 1������ģ�Ͷ��󣬶���inp_file
	INetwork* net = createNetwork();
	if (!net->readFromFile(inp_file))
	{
		char* error_text = new char[100000]; // Ҫȷ���㹻��
		net->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return;
	}

	// 2���������˹�ϵ���������ܻ�ȡ�ڵ������������
	ILinkSet* links = net->getLinkSet();
	INodeSet* nodes = net->getNodeSet();
	links->associateNode(nodes);

	// 3���򿪼�¼�ļ�
	std::ofstream record(record_file);
	if (!record)
	{
		std::cout << "���ļ� " << record_file << " ʧ��!" << "\n";
		return;
	}	
	record << std::setiosflags(std::ios::left);

	// 4���������Ӳ���ڵ�ײ������
	record << std::setw(23) << "�ڵ�����"   << " "
		   << std::setw(15) << "ԭ�ױ��/m" << " "
		   << "�µױ��/m"  << "\n";
	char name[30];
	double min_offset;
	INode* node = nullptr;
	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		node = nodes->getNodeObjectAndName(name, i);
		min_offset = get_min_offset(node);
		// ��Сƫ�Ƴ��ָ�����˵�������Ӳ���ڵ�ײ�
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

	// 5����������¶����������
	record << "\n\n";
	record << std::setw(23)  << "�ڵ�����"     << " "
		   << std::setw(15)  << "ԭ������/m" << " "
		   << "�µ�����/m" << "\n";
	double max_crown;
	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		node = nodes->getNodeObjectAndName(name, i);
		if (node->isOutfall()) continue; // �ŷſڲ����
		max_crown = get_max_crown(node);
		// ¶������
		if (max_crown > get_ground_elevation(node))
		{
			record << std::setw(23) << name                       << " " 
				   << std::setw(15) << get_ground_elevation(node) << " ";
			// ��������0.1mm�����������������
			node->setFullDepth(max_crown - node->getInvertElevation() + 1.0E-4);
			record << get_ground_elevation(node) << "\n";
		}
	}

	// 6�������������С�ڸ߲������������޸�Ϊ�߲��length_multiplier��
	record << "\n\n";
	record << std::setw(23) << "��������"     << " "
		   << std::setw(15) << "ԭ��������/m" << " "
		   << std::setw(15) << "�¹�������/m" << " "
		   << "�����½�/m"  << "\n";
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
	
	// 7����������ģ�ͱ���Ϊ�µ�inp�ļ�	
	if (!net->save(new_inp_file))
	{ 
		std::cout << "���ļ� " << new_inp_file << " ʧ��!" << "\n";
		return;
	}

	record.close();
	deleteNetwork(net);
}
/*
 *******************************************************************************
 * Project    ��PreProcess
 * Version    ��1.1.1
 * File       ��Ѱ�Ҵ�ܽ�С��
 * Date       ��03/27/2023
 * Author     ����С��
 * Copyright  ������ˮ�ֽڿƼ����޹�˾
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
// Ŀ�ģ���ȡ������С�ܾ��Ĺ�������
//------------------------------------------------------------------------------
{
	ignore                = false;
	min_up_geom1          = 1.0E6;   // ��ʼ��������С�ܾ�Ϊ����ֵ
	ILink*    link        = nullptr;	
	IConduit* conduit     = nullptr;	
	IConduit* min_conduit = nullptr; // ��С�ܾ���Ӧ�Ĺ�������

	for (int i = 0; i < node->getUpLinkCounts(); ++i)
	{
		// ���ڷǹ������ӣ���ʱ������ܻ�С��
		link = node->getUpLink(i);
		if (!link->isConduit())
		{
			ignore = true;
			return nullptr;
		}

		// ��ȡ���ι���
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
		// ���ڷǹ������ӣ���ʱ������ܻ�С��
		link = node->getDnLink(i);
		if (!link->isConduit())
		{
			ignore = true;
			return nullptr;
		}

		// ��ȡ���ι�����ʵ�������Σ�
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
// Ŀ�ģ���ȡ�������ܾ��Ĺ�������
//------------------------------------------------------------------------------
{
	ignore                = false;
	max_dn_geom1          = -1.0E6;  // ��ʼ���������ܾ�Ϊ��Сֵ
	ILink*    link        = nullptr;
	IConduit* conduit     = nullptr;
	IConduit* max_conduit = nullptr; // ���ܾ���Ӧ�Ĺ�������

	for (int i = 0; i < node->getDnLinkCounts(); ++i)
	{
		// ���ڷǹ������ӣ���ʱ������ܻ�С��
		link = node->getDnLink(i);
		if (!link->isConduit()) 
		{
			ignore = true;
			return nullptr;
		}

		// ��ȡ���ι���
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
		// ���ڷǹ������ӣ���ʱ������ܻ�С��
		link = node->getUpLink(i);
		if (!link->isConduit())
		{
			ignore = true;
			return nullptr;
		}

		// ��ȡ���ι�����ʵ�������Σ�
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
// Ŀ�ģ���ȡ��ܽ�С�ܵ�����
//------------------------------------------------------------------------------
{		
	char      node_name[30];            // ռλ��
	bool      ignore         = false;   // �Ƿ�����ǹ�������
	INode*    node           = nullptr;	// �����ýڵ�
	double    min_up_geom1   = 0.0;     // ������С�ܾ�
	IConduit* min_up_conduit = nullptr; // ������С�ܾ���Ӧ�Ĺ�������
	double    max_dn_geom1   = 0.0;     // �������ܾ�
	IConduit* max_dn_conduit = nullptr; // �������ܾ���Ӧ�Ĺ�������
	int       total_counts   = 0;       // ��ܽ�С�ܵ�����
	
	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		node = nodes->getNodeObjectAndName(node_name, i);		
		min_up_conduit = get_min_up_conduit(node, min_up_geom1, ignore);
		if (ignore) continue;
		max_dn_conduit = get_max_dn_conduit(node, max_dn_geom1, ignore);
		if (ignore) continue;

		// ����ͬʱ���������ι�������
		if (min_up_conduit == nullptr || max_dn_conduit == nullptr)
			continue;

		// ��ܽ�С��
		if (min_up_geom1 > max_dn_geom1)
			++total_counts;
	}

	return total_counts;
}

void record_large2small(const char* inp_file, const char* record_file)
//------------------------------------------------------------------------------
// Ŀ�ģ����inp_file������ܽ�С�ܵ���Ϣ������record_file
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

	// 2. �����ܽ�С����Ϣ
	std::ofstream ofs(record_file);
	ofs << std::setiosflags(std::ios::left);
	ofs << std::setw(23) << "�������"   << " "
		<< std::setw(23) << "С������"   << " "
		<< std::setw(23) << "��ܹܾ�/m" << " "
		<< "С�ܹܾ�/m"  << "\n";

	char      node_name[30];            // ռλ��
	bool      ignore         = false;   // �Ƿ�����ǹ�������
	INode*    node           = nullptr;	// �����ýڵ�
	double    min_up_geom1   = 0.0;     // ������С�ܾ�
	IConduit* min_up_conduit = nullptr; // ������С�ܾ���Ӧ�Ĺ�������
	char      min_up_name[30];          // ������С�ܾ���Ӧ�Ĺ�����������
	double    max_dn_geom1   = 0.0;     // �������ܾ�
	IConduit* max_dn_conduit = nullptr; // �������ܾ���Ӧ�Ĺ�������
	char      max_up_name[30];          // �������ܾ���Ӧ�Ĺ�����������
	INodeSet* nodes = net->getNodeSet();
	ILinkSet* links = net->getLinkSet();

	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		node = nodes->getNodeObjectAndName(node_name, i);		
		min_up_conduit = get_min_up_conduit(node, min_up_geom1, ignore);
		if (ignore) continue;
		max_dn_conduit = get_max_dn_conduit(node, max_dn_geom1, ignore);
		if (ignore) continue;

		// ����ͬʱ���������ι�������
		if (min_up_conduit == nullptr || max_dn_conduit == nullptr)
			continue;

		// ��ܽ�С��
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
// Ŀ�ģ��ҵ���һ����������
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
// Ŀ�ģ���ȡֱ�߶��ϲ�Э���ܵ�����
//------------------------------------------------------------------------------
{
	char name[30];                    // ��ǰ������������	
	double geom1;                     // ��ǰ�������Ӹ߶�
	double beg_geom1;                 // ���ι������Ӹ߶�
	double end_geom1;                 // ���ι������Ӹ߶�
	ILink*    link         = nullptr; // ����������
	IConduit* conduit      = nullptr; // �����ù�������
	INode*    beg_node     = nullptr; // ��˽ڵ�
	IConduit* beg_conduit  = nullptr; // ���ι�������	
	INode*    end_node     = nullptr; // �ն˽ڵ�
	IConduit* end_conduit  = nullptr; // ���ι�������
	int       total_counts = 0;       // ֱ�߶��ϲ�Э���ܵ�����

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

		// ��Э���ܵ�
		if ((geom1 < beg_geom1 && geom1 < end_geom1) ||
			(geom1 > beg_geom1 && geom1 > end_geom1))
			++total_counts;
	}

	return total_counts;
}

void record_discordant(const char* inp_file, const char* record_file)
//------------------------------------------------------------------------------
// Ŀ�ģ����inp_file����ֱ�߶��ϲ�Э���ܵ�����Ϣ������record_file
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

	// 2. ����ֱ�߶��ϲ�Э���ܵ�����Ϣ
	std::ofstream ofs(record_file);
	ofs << std::setiosflags(std::ios::left);
	ofs << std::setw(23)    << "�м�ܵ�����"   << " "
		<< std::setw(23)    << "���ιܵ��ܾ�/m" << " "
		<< std::setw(23)    << "�м�ܵ��ܾ�/m" << " "
		<< "���ιܵ��ܾ�/m" << "\n";

	char name[30];                   // ��ǰ������������	
	double geom1;                    // ��ǰ�������Ӹ߶�
	double beg_geom1;                // ���ι������Ӹ߶�
	double end_geom1;                // ���ι������Ӹ߶�
	ILink*    link        = nullptr; // ����������
	IConduit* conduit     = nullptr; // �����ù�������
	INode*    beg_node    = nullptr; // ��˽ڵ�
	IConduit* beg_conduit = nullptr; // ���ι�������	
	INode*    end_node    = nullptr; // �ն˽ڵ�
	IConduit* end_conduit = nullptr; // ���ι�������
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

		// ��Э���ܵ�
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
// Ŀ�ģ���ȡֱ�߶����ϸ�Э���ܵ�����
// ˵�����ϸ�Э���������ι����ܾ���ͬ������ǰ�����ܾ��������ι����ܾ���ͬ
// ������flag =  0������ǰ�����ܾ����ڻ�С�������ι����ܾ� 
//       flag =  1������ǰ�����ܾ����������ι����ܾ�
//       flag = -1������ǰ�����ܾ�С�������ι����ܾ�
//------------------------------------------------------------------------------
{
	char name[30];                    // ��ǰ������������	
	double geom1;                     // ��ǰ�������Ӹ߶�
	double beg_geom1;                 // ���ι������Ӹ߶�
	double end_geom1;                 // ���ι������Ӹ߶�
	ILink*    link         = nullptr; // ����������
	IConduit* conduit      = nullptr; // �����ù�������
	INode*    beg_node     = nullptr; // ��˽ڵ�
	IConduit* beg_conduit  = nullptr; // ���ι�������	
	INode*    end_node     = nullptr; // �ն˽ڵ�
	IConduit* end_conduit  = nullptr; // ���ι�������
	int       total_counts = 0;       // ֱ�߶����ϸ�Э���ܵ�����

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

		// �����ι����ܾ���ͬ
		if (std::abs(beg_geom1 - end_geom1) < 1.0E-6) 
		{
			// �ϸ�Э���ܵ�
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
// Ŀ�ģ����inp_file������ֱ�߶����ϸ�Э���ܵ��ܾ�������¼��������
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

	// 2. ��������¼�ϸ�Э���ܵ�
	std::ofstream ofs(record_file);
	ofs << std::setiosflags(std::ios::left);
	ofs << std::setw(23)  << "�������ܵ�����" << " "		
		<< std::setw(23)  << "����ǰ�ܾ�/m"   << " "
		<< "������ܾ�/m" << "\n";

	char name[30];                   // ��ǰ������������	
	double geom1;                    // ��ǰ�������Ӹ߶�
	double beg_geom1;                // ���ι������Ӹ߶�
	double end_geom1;                // ���ι������Ӹ߶�
	ILink*    link        = nullptr; // ����������
	IConduit* conduit     = nullptr; // �����ù�������
	INode*    beg_node    = nullptr; // ��˽ڵ�
	IConduit* beg_conduit = nullptr; // ���ι�������	
	INode*    end_node    = nullptr; // �ն˽ڵ�
	IConduit* end_conduit = nullptr; // ���ι�������
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

		// �����ι����ܾ���ͬ
		if (std::abs(beg_geom1 - end_geom1) < 1.0E-6)
		{
			// �ϸ�Э���ܵ�
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

	// 3. �����޸ĺ��inp�ļ�
	if (!net->save(new_inp_file))
	{
		std::cout << "���ļ� " << new_inp_file << " ʧ��!" << "\n";
		return;
	}

	ofs.close();
	deleteNetwork(net);
}
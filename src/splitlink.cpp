/*
 *******************************************************************************
 * Project    ��PreProcess
 * Version    ��1.1.1
 * File       ���ֿ��غ�����
 * Date       ��03/27/2023
 * Author     ����С��
 * Copyright  ������ˮ�ֽڿƼ����޹�˾
 *******************************************************************************
 */

#include "global.h"
#include "splitlink.h"

#include <map>
#include <string>
#include <iostream>

static std::multimap<std::pair<std::string, 
	std::string>, std::string> create_node_name_map(INetwork* net)
//------------------------------------------------------------------------------
// Ŀ�ģ������������˽ڵ����ƺ��������Ƶ�ӳ��
//------------------------------------------------------------------------------
{	
	char link_name[30];
	char beg_node_name[30];
	char end_node_name[30];
	INode* node = nullptr;
	ILink* link = nullptr;
	INodeSet* nodes = net->getNodeSet();
	ILinkSet* links = net->getLinkSet();
	std::pair<std::string, std::string> node_pair;
	std::multimap<
		std::pair<std::string, std::string>, std::string> node_name_map;

	for (int i = 0; i < links->getLinkCounts(); ++i)
	{
		link = links->getLinkObjectAndName(link_name, i);
		node = link->getBeginNode();
		nodes->getNodeName(node, beg_node_name);
		node = link->getEndNode();
		nodes->getNodeName(node, end_node_name);
		node_pair = std::make_pair(beg_node_name, end_node_name);
		node_name_map.insert(std::make_pair(node_pair, link_name));
	}

	return node_name_map;
}

static std::multimap<std::pair<std::string, std::string>, std::string> 
    create_collineated_node_name_map(INetwork* net, IGeoprocess* igp)
//------------------------------------------------------------------------------
// Ŀ�ģ��ҵ����ܹ��ߵ�����
//------------------------------------------------------------------------------
{
	std::multimap<std::pair<std::string, std::string>,
		std::string> collineated_node_name_map;
	const auto& node_name_map = create_node_name_map(net);
	IVertices* verts = igp->getVertices();
	std::string first_link_name;
	std::string other_link_name;

	for (auto iter = std::begin(node_name_map); iter != std::end(node_name_map);
		iter = node_name_map.upper_bound(iter->first))
	{
		if (node_name_map.count(iter->first) < 2) continue;

		auto iter_pair = node_name_map.equal_range(iter->first);
		//  ��ȡ��һ����������
		auto it = iter_pair.first;
		first_link_name = it->second;
		collineated_node_name_map.insert(
			std::make_pair(it->first, first_link_name));
		// �ӵڶ�����ʼ�Ƚ�		
		for (++it; it != iter_pair.second; ++it)
		{
			// ����Ƿ���
			other_link_name = it->second;
			if (verts->isOverlapped(
				first_link_name.c_str(), other_link_name.c_str()))
			{
				collineated_node_name_map.insert(
					std::make_pair(it->first, other_link_name));
			}			
		}
	}

	return collineated_node_name_map;
}

static std::multimap<std::pair<std::string, std::string>, 
	std::string> get_collineated_node_name_map(INetwork* net, IGeoprocess* igp)
//------------------------------------------------------------------------------
// Ŀ�ģ��ҵ����ߵ�����
//------------------------------------------------------------------------------
{
	std::multimap<std::pair<std::string, std::string>,
		std::string> collineated_node_name_map;
	const auto& node_name_map = create_collineated_node_name_map(net, igp);

	for (auto iter = std::begin(node_name_map); iter != std::end(node_name_map);
		iter = node_name_map.upper_bound(iter->first))
	{
		if (node_name_map.count(iter->first) < 2) continue;

		auto iter_pair = node_name_map.equal_range(iter->first);
		for (auto it = iter_pair.first; it != iter_pair.second; ++it)
			collineated_node_name_map.insert(*it);
	}

	return collineated_node_name_map;
}

static void get_new_position(double x1, double y1, double x2, double y2,
	double x0, double y0, double& new_x0, double& new_y0, double f)
//------------------------------------------------------------------------------
// ������(x1, y1)         = ��˽ڵ�����
//       (x2, y2)         = �ն˽ڵ�����
//       (x0, y0)         = ԭ��������
//       (new_x0, new_y0) = �¶�������
//       f                = Ų����������
// Ŀ�ģ�����û�ж�����غ����ӣ��������㣻����Ų��ԭ����
//------------------------------------------------------------------------------
{	
	new_x0 = x0 + f * (y2 - y1);
	new_y0 = y0 + f * (x1 - x2);
}

int get_collineated_link_counts(INetwork* net, IGeoprocess* igp)
//------------------------------------------------------------------------------
// Ŀ�ģ�ͳ���ж��ٹ��ߵ�����
//------------------------------------------------------------------------------
{
	return get_collineated_node_name_map(net, igp).size();
}

void split_collineated_link(const char* inp_file, const char* new_inp_file, 
	const char* tmp_geo_file, double f)
//------------------------------------------------------------------------------
// Ŀ�ģ��ֿ����ߵ�����
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
	IMap* map = igp->getMap();
	double x_offset, y_offset;
	map->getBottomLeft(&x_offset, &y_offset);
	IVertices* iverts = igp->getVertices();
	ICoordinates* icoords = igp->getCoordinates();

	// 3����տռ���Ϣ���Ա�����ļ��ϲ����������ظ�
	net->clearGeometryData();

	// 4����ȡ����������Ϣ
	const auto& node_name_map = get_collineated_node_name_map(net, igp);

	// 5�������������Ӷ����Ų������
	int counts;
	int sgn = 1;
	double factor;
	size_t idx = 0;	
	std::string link_name;	
	double x1, y1, x2, y2;
	double x0, y0, new_x0, new_y0;	
	for (auto iter = std::begin(node_name_map); iter != std::end(node_name_map);
		iter = node_name_map.upper_bound(iter->first))
	{
		// ��ȡ���˽ڵ�����		
		icoords->getCoordinateByName(iter->first.first.c_str(), &x1, &y1, map);
		icoords->getCoordinateByName(iter->first.second.c_str(), &x2, &y2, map);

		// ������������
		auto iter_pair = node_name_map.equal_range(iter->first);
		auto it = iter_pair.first;

		// ��������û�ж���
		link_name = iter->second; // ��һ����������
		if (!iverts->getVerticeCountsByName(link_name.c_str(), &counts))
		{
			idx = 0;
			sgn = 1;
			x0 = 0.5 * (x1 + x2);
			y0 = 0.5 * (y1 + y2);
			// �ӵڶ����غ����ӿ�ʼ��������		
			for (++it; it != iter_pair.second; ++it)
			{
				factor = sgn * f * ((idx++ / 2) + 1);
				get_new_position(
					x1, y1, x2, y2, x0, y0, new_x0, new_y0, factor);
				sgn *= -1; // ��ת����һ��	
				// ת��Ϊinp����
				new_x0 -= x_offset;
				new_y0 -= y_offset;
				// ��������
				iverts->addVertice(it->second.c_str(), new_x0, new_y0);				
			}
		}
		else
		{
			idx = 0;
			sgn = 1;			
			// �ӵڶ����غ����ӿ�ʼŲ������		
			for (++it; it != iter_pair.second; ++it)
			{
				factor = sgn * f * ((idx++ / 2) + 1);
				// ����ÿ1������
				for (int i = 0; i < counts; ++i)
				{
					iverts->getVerticeByName(it->second.c_str(), i, &x0, &y0, map);
					get_new_position(
						x1, y1, x2, y2, x0, y0, new_x0, new_y0, factor);
					// ת��Ϊinp����
					new_x0 -= x_offset;
					new_y0 -= y_offset;
					// ���¶�������
					iverts->modifyVerticesPosition(
						it->second.c_str(), i, new_x0, new_y0);
				}
				sgn *= -1; // ��ת����һ��			
			}
		}
	}

	// 6����������ģ�ͱ���Ϊ�µ�inp�ļ�	
	if (!net->save(new_inp_file))
	{
		std::cout << "���ļ� " << new_inp_file << " ʧ��!" << "\n";
		return;
	}
	if (!igp->saveGeoFile(tmp_geo_file))   
	{
		std::cout << "���ļ� " << tmp_geo_file << " ʧ��!" << "\n";
		return;
	}
	merge_file(new_inp_file, tmp_geo_file);

	deleteNetwork(net);	
	deleteGeoprocess(igp);
}
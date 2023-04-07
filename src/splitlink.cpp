/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.1.1
 * File       ：分开重合连接
 * Date       ：03/27/2023
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
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
// 目的：创建连接两端节点名称和连接名称的映射
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
// 目的：找到可能共线的连接
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
		//  获取第一个连接名称
		auto it = iter_pair.first;
		first_link_name = it->second;
		collineated_node_name_map.insert(
			std::make_pair(it->first, first_link_name));
		// 从第二个开始比较		
		for (++it; it != iter_pair.second; ++it)
		{
			// 检查是否共线
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
// 目的：找到共线的连接
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
// 参数：(x1, y1)         = 起端节点坐标
//       (x2, y2)         = 终端节点坐标
//       (x0, y0)         = 原顶点坐标
//       (new_x0, new_y0) = 新顶点坐标
//       f                = 挪动距离因子
// 目的：对于没有顶点的重合连接，新增顶点；否则，挪动原顶点
//------------------------------------------------------------------------------
{	
	new_x0 = x0 + f * (y2 - y1);
	new_y0 = y0 + f * (x1 - x2);
}

int get_collineated_link_counts(INetwork* net, IGeoprocess* igp)
//------------------------------------------------------------------------------
// 目的：统计有多少共线的连接
//------------------------------------------------------------------------------
{
	return get_collineated_node_name_map(net, igp).size();
}

void split_collineated_link(const char* inp_file, const char* new_inp_file, 
	const char* tmp_geo_file, double f)
//------------------------------------------------------------------------------
// 目的：分开共线的连接
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
	IMap* map = igp->getMap();
	double x_offset, y_offset;
	map->getBottomLeft(&x_offset, &y_offset);
	IVertices* iverts = igp->getVertices();
	ICoordinates* icoords = igp->getCoordinates();

	// 3）清空空间信息，以便后面文件合并不会重现重复
	net->clearGeometryData();

	// 4）获取共线连接信息
	const auto& node_name_map = get_collineated_node_name_map(net, igp);

	// 5）遍历对象，增加顶点或挪动顶点
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
		// 获取两端节点坐标		
		icoords->getCoordinateByName(iter->first.first.c_str(), &x1, &y1, map);
		icoords->getCoordinateByName(iter->first.second.c_str(), &x2, &y2, map);

		// 遍历共线连接
		auto iter_pair = node_name_map.equal_range(iter->first);
		auto it = iter_pair.first;

		// 共线连接没有顶点
		link_name = iter->second; // 第一个连接名称
		if (!iverts->getVerticeCountsByName(link_name.c_str(), &counts))
		{
			idx = 0;
			sgn = 1;
			x0 = 0.5 * (x1 + x2);
			y0 = 0.5 * (y1 + y2);
			// 从第二个重合连接开始新增顶点		
			for (++it; it != iter_pair.second; ++it)
			{
				factor = sgn * f * ((idx++ / 2) + 1);
				get_new_position(
					x1, y1, x2, y2, x0, y0, new_x0, new_y0, factor);
				sgn *= -1; // 翻转到另一侧	
				// 转换为inp坐标
				new_x0 -= x_offset;
				new_y0 -= y_offset;
				// 新增顶点
				iverts->addVertice(it->second.c_str(), new_x0, new_y0);				
			}
		}
		else
		{
			idx = 0;
			sgn = 1;			
			// 从第二个重合连接开始挪动顶点		
			for (++it; it != iter_pair.second; ++it)
			{
				factor = sgn * f * ((idx++ / 2) + 1);
				// 遍历每1个顶点
				for (int i = 0; i < counts; ++i)
				{
					iverts->getVerticeByName(it->second.c_str(), i, &x0, &y0, map);
					get_new_position(
						x1, y1, x2, y2, x0, y0, new_x0, new_y0, factor);
					// 转换为inp坐标
					new_x0 -= x_offset;
					new_y0 -= y_offset;
					// 更新顶点坐标
					iverts->modifyVerticesPosition(
						it->second.c_str(), i, new_x0, new_y0);
				}
				sgn *= -1; // 翻转到另一侧			
			}
		}
	}

	// 6）将处理后的模型保存为新的inp文件	
	if (!net->save(new_inp_file))
	{
		std::cout << "打开文件 " << new_inp_file << " 失败!" << "\n";
		return;
	}
	if (!igp->saveGeoFile(tmp_geo_file))   
	{
		std::cout << "打开文件 " << tmp_geo_file << " 失败!" << "\n";
		return;
	}
	merge_file(new_inp_file, tmp_geo_file);

	deleteNetwork(net);	
	deleteGeoprocess(igp);
}
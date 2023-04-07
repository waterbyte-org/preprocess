/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.0.0
 * File       ：寻找非测量偏移的连接
 * Date       ：04/12/2022
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#include "unmeasuredoffset.h"

#include <map>
#include <string>
#include <iomanip>
#include <fstream>
#include <iostream>

static std::multimap<std::pair<
	long long, long long>, std::string> create_search_map(INetwork* net)
//------------------------------------------------------------------------------
// 目的：在管渠连接起始偏移和连接名称之间建立映射，以便查询
//------------------------------------------------------------------------------
{
	char name[30];
	ILink* link = nullptr;
	IConduit* conduit = nullptr;
	IOption* op = net->getOption();
	ILinkSet* links = net->getLinkSet();

	long long beg_offset = 0;
	long long end_offset = 0;
	std::pair<long long, long long> offsets;
	std::multimap<std::pair<long long, long long>, std::string> offsets_map;	

	for (int i = 0; i < links->getLinkCounts(); ++i)
	{
		link = links->getLinkObjectAndName(name, i);
		if (!link->isConduit()) continue;
		conduit = dynamic_cast<IConduit*>(link);

		// 将偏移值乘以10000，并转化为整数
		beg_offset = (long long)std::floor(
			conduit->getInpBeginOffset(op) * 10000 + 0.5);
		end_offset = (long long)std::floor(
			conduit->getInpEndOffset(op) * 10000 + 0.5);
		offsets = std::make_pair(beg_offset, end_offset);
		offsets_map.insert(std::make_pair(offsets, name));
	}

	return offsets_map;
}

int get_unmeasured_link_counts(INetwork* net, int dup_times)
//------------------------------------------------------------------------------
// 目的：统计具有完全相同偏移值的组合数量
//------------------------------------------------------------------------------
{
	int unmeasured_link_counts = 0;
	const std::multimap<std::pair<long long, 
		long long>, std::string>& offsets_map = create_search_map(net);

	for (auto iter = std::begin(offsets_map); iter != std::end(offsets_map);
		iter = offsets_map.upper_bound(iter->first))
	{
		if (offsets_map.count(iter->first) >= dup_times)
			++unmeasured_link_counts;
	}

	return unmeasured_link_counts;
}

void record_unmeasured_link(const char* inp_file, const char* record_file,
	int dup_times)
//------------------------------------------------------------------------------
// 目的：检查inp_file，将存在疑问的偏移组合及连接名称保存在record_file
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

	// 2. 保存符合条件的连接名称
	std::ofstream ofs(record_file);

	const std::multimap<std::pair<long long,
		long long>, std::string>& offsets_map = create_search_map(net);
	std::pair<long long, long long> offsets;
	int items = 0;

	for (auto iter = std::begin(offsets_map); iter != std::end(offsets_map);
		iter = offsets_map.upper_bound(iter->first))
	{
		if (offsets_map.count(iter->first) >= dup_times)
		{
			offsets = iter->first;
			ofs << offsets.first / 10000.0  << " ----> "
				<< offsets.second / 10000.0 << "\n";
			items = 0;
			auto iter_pair = offsets_map.equal_range(offsets);
			for (auto it = iter_pair.first; it != iter_pair.second; ++it)
			{				
				ofs << it->second << "  ";
				++items;
				if (items % 5 == 0) ofs << "\n";
			}
			ofs << "\n******************************************************\n";
		}
	}	
	ofs.close();

	deleteNetwork(net);
}
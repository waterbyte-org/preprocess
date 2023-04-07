/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.0.3
 * File       ：查找和修正inp数据问题
 * Date       ：05/11/2022
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#include "path.h"
#include "../src/removeshort.h"
#include "../src/convertdepth.h"
#include "../src/removebranch.h"
#include "../src/unmeasuredoffset.h"

#include <string>
#include <iostream>

int main()
{
	using namespace std;

	// 1. 创建模型对象，读入inp文件（已调整流量单位）
	INetwork* net = createNetwork();
	const string inp_2 = string(inp_path) + "gusumodel-2.inp";
	if (!net->readFromFile(inp_2.c_str()))
		return 1;

	// 2. 创建拓扑关系，并检查是否存在高程相关的问题
	INodeSet* nodes = net->getNodeSet();
	ILinkSet* links = net->getLinkSet();
	links->associateNode(nodes);
	cout << "连接插入井底的节点数量：" << get_concave_counts(nodes) << "\n";
	cout << "连接露出地面的节点数量：" << get_convex_counts(nodes) << "\n";
	cout << "长度不大于坡降的管渠连接数量："
		<< get_error_link_counts(links) << "\n";

	// 3. 修正高程相关的问题
	const string inp_3 = string(inp_path) + "gusumodel-3.inp";
	const string record_1 = string(inp_path) + "record-1.txt";
	convert_depth(inp_2.c_str(), inp_3.c_str(), record_1.c_str(), 1.5);

	// 4. 读入并检查修正标高后的inp文件
	if (!net->readFromFile(inp_3.c_str()))
		return 2;
	if (!net->validateData())
		return 3;
	IGeoprocess* igp = createGeoprocess(); // 后面要用到
	if (!igp->openGeoFile(inp_3.c_str()))
		return 4;
	if (!igp->validateData())
		return 5;

	// 5. 先检查是否还存在标高问题，再检查需要删除的各类支管数量
	nodes = net->getNodeSet();
	links = net->getLinkSet();	
	cout << "连接插入井底的节点数量：" << get_concave_counts(nodes) << "\n";
	cout << "连接露出地面的节点数量：" << get_convex_counts(nodes) << "\n";
	cout << "长度不大于坡降的管渠连接数量："
		<< get_error_link_counts(links) << "\n";
	cout << "拥有边侧单连接支管的节点数：" <<
		get_one_link_lateral_branch_counts(nodes) << "\n";	
	cout << "拥有末端单连接支管的节点数：" <<
		get_one_link_terminal_branch_counts(nodes, igp, -0.866, 15) << "\n";
	cout << "拥有边侧双连接支管的节点数："
		<< get_two_link_lateral_branch_counts(nodes) << "\n";

	// 6. 逐步删除各类支管。注意：不能一口气删除！
	const string inp_4 = string(inp_path) + "gusumodel-4.inp";
	const string record_2 = string(inp_path) + "record-2.txt";
	remove_one_link_lateral_branch(
		inp_3.c_str(), inp_4.c_str(), record_2.c_str());
	const string inp_5 = string(inp_path) + "gusumodel-5.inp";
	const string record_3 = string(inp_path) + "record-3.txt";
	remove_one_link_terminal_branch(
		inp_4.c_str(), inp_5.c_str(), record_3.c_str(), -0.866, 15);
	const string inp_6 = string(inp_path) + "gusumodel-6.inp";
	const string record_4 = string(inp_path) + "record-4.txt";
	remove_two_link_lateral_branch(
		inp_5.c_str(), inp_6.c_str(), record_4.c_str());

	// 7. 读入并检查删除支管后的inp文件，再检查具有完全相同偏移值的组合
	if (!net->readFromFile(inp_6.c_str()))
		return 6;
	if (!net->validateData())
		return 7;
	cout << "具有完全相同偏移值的组合数量（3条连接以上）：" 
		 << get_unmeasured_link_counts(net, 3) << "\n";

	// 8. 查找并将具有完全相同偏移值的组合记录下来
	const string record_5 = string(inp_path) + "record-5.txt";
	record_unmeasured_link(inp_6.c_str(), record_5.c_str(), 3);

	// 9. 检查是否存在短管。注意：较短连接包含极短连接
	links = net->getLinkSet();
	cout << "极短连接数："
		<< get_shortest_link_counts(links, 2.0) << "\n";
	cout << "较短连接数：" 
		<< get_shorter_link_counts(links, 10.0, 0.1) << "\n";

	// 10. 逐步删除各类短管。注意：不能一口气删除！
	const string inp_7 = string(inp_path) + "gusumodel-7.inp";
	const string record_6 = string(inp_path) + "record-6.txt";
	remove_shortest_link(inp_6.c_str(), inp_7.c_str(), record_6.c_str(), 2.0);
	const string inp_8 = string(inp_path) + "gusumodel-8.inp";
	const string record_7 = string(inp_path) + "record-7.txt";
	remove_shorter_link(
		inp_7.c_str(), inp_8.c_str(), record_7.c_str(), 10.0, 0.1);

	// 11. 检查各类问题是否还存在
	if (!net->readFromFile(inp_8.c_str()))
		return 8;
	if (!net->validateData())
		return 9;	
	if (!igp->openGeoFile(inp_8.c_str()))
		return 10;
	if (!igp->validateData())
		return 11;
	nodes = net->getNodeSet();
	links = net->getLinkSet();
	cout << "连接插入井底的节点数量：" << get_concave_counts(nodes) << "\n";
	cout << "连接露出地面的节点数量：" << get_convex_counts(nodes) << "\n";
	cout << "长度不大于坡降的管渠连接数量："
		<< get_error_link_counts(links) << "\n";
	cout << "拥有边侧单连接支管的节点数：" <<
		get_one_link_lateral_branch_counts(nodes) << "\n";
	cout << "拥有末端单连接支管的节点数：" <<
		get_one_link_terminal_branch_counts(nodes, igp, -0.866, 15) << "\n";
	cout << "拥有边侧双连接支管的节点数："
		<< get_two_link_lateral_branch_counts(nodes) << "\n";
	cout << "极短连接数："
		<< get_shortest_link_counts(links, 2.0) << "\n";
	cout << "较短连接数："
		<< get_shorter_link_counts(links, 10.0, 0.1) << "\n";

	// 12. 仍然存在一些问题，针对这些问题继续处理
	const string inp_9 = string(inp_path) + "gusumodel-9.inp";
	const string record_8 = string(inp_path) + "record-8.txt";
	remove_one_link_lateral_branch(
		inp_8.c_str(), inp_9.c_str(), record_8.c_str());
	const string inp_10 = string(inp_path) + "gusumodel-10.inp";
	const string record_9 = string(inp_path) + "record-9.txt";
	remove_one_link_terminal_branch(
		inp_9.c_str(), inp_10.c_str(), record_9.c_str(), -0.866, 15);
	const string inp_11 = string(inp_path) + "gusumodel-11.inp";
	const string record_10 = string(inp_path) + "record-10.txt";
	remove_two_link_lateral_branch(
		inp_10.c_str(), inp_11.c_str(), record_10.c_str());
	const string inp_12 = string(inp_path) + "gusumodel-12.inp";
	const string record_11 = string(inp_path) + "record-11.txt";
	remove_shorter_link(
		inp_11.c_str(), inp_12.c_str(), record_11.c_str(), 10.0, 0.1);

	// 13. 再次检查各类问题是否还存在
	if (!net->readFromFile(inp_12.c_str()))
		return 12;
	if (!net->validateData())
		return 13;
	if (!igp->openGeoFile(inp_12.c_str()))
		return 14;
	if (!igp->validateData())
		return 15;
	nodes = net->getNodeSet();
	links = net->getLinkSet();
	cout << "连接插入井底的节点数量：" << get_concave_counts(nodes) << "\n";
	cout << "连接露出地面的节点数量：" << get_convex_counts(nodes) << "\n";
	cout << "长度不大于坡降的管渠连接数量："
		<< get_error_link_counts(links) << "\n";
	cout << "拥有边侧单连接支管的节点数：" <<
		get_one_link_lateral_branch_counts(nodes) << "\n";
	cout << "拥有末端单连接支管的节点数：" <<
		get_one_link_terminal_branch_counts(nodes, igp, -0.866, 15) << "\n";
	cout << "拥有边侧双连接支管的节点数："
		<< get_two_link_lateral_branch_counts(nodes) << "\n";
	cout << "极短连接数："
		<< get_shortest_link_counts(links, 2.0) << "\n";
	cout << "较短连接数："
		<< get_shorter_link_counts(links, 10.0, 0.1) << "\n";

	// 13. 仍然存在一些问题，针对这些问题继续处理
	//     省略

	deleteNetwork(net);
	deleteGeoprocess(igp);

	return 0;
}
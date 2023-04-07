/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.0.3
 * File       ：调整高度相关
 * Date       ：05/13/2022
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#include "path.h"
#include "../src/global.h"
#include "../src/convertdepth.h"

#include <string>
#include <iostream>

int main()
{
	using namespace std;

	// 1. 检查inp是否存在相关问题
	const string inp_2 = string(inp_path) + "gusumodel-2.inp";
	INetwork* net = createNetwork();
	if (!net->readFromFile(inp_2.c_str()))
	{
		char* error_text = new char[100000]; // 要确保足够大
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 1;
	}
	INodeSet* nodes = net->getNodeSet();
	ILinkSet* links = net->getLinkSet();
	links->associateNode(nodes);
	cout << "连接插入井底的节点数量：" << get_concave_counts(nodes) << "\n";
	cout << "连接露出地面的节点数量：" << get_convex_counts(nodes)  << "\n";
	cout << "长度不大于坡降的管渠连接数量：" 
		 << get_error_link_counts(links) << "\n";

	// 2. 调整有问题的数据
	const string inp_3 = string(inp_path) + "gusumodel-3.inp";
	const string record_1 = string(inp_path) + "record-1.txt";
	convert_depth(inp_2.c_str(), inp_3.c_str(), record_1.c_str(), 1.5);

	// 3. 检查调整后是否正确
	if (!net->readFromFile(inp_3.c_str()))
	{
		char* error_text = new char[100000]; // 要确保足够大
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 2;
	}
	nodes = net->getNodeSet();
	links = net->getLinkSet();
	links->associateNode(nodes);
	cout << "*****调整后*****" << "\n";
	cout << "连接插入井底的节点数量：" << get_concave_counts(nodes) << "\n";
	cout << "连接露出地面的节点数量：" << get_convex_counts(nodes)  << "\n";
	cout << "长度不大于坡降的管渠连接数量：" 
		 << get_error_link_counts(links) << "\n";

	deleteNetwork(net);

	system("pause");

	return 0;
}
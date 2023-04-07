/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.0.3
 * File       ：删除短管
 * Date       ：05/13/2022
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#include "path.h"
#include "../src/removeshort.h"

#include <set>
#include <string>
#include <assert.h>
#include <iostream>

int main()
{
	using namespace std;

	// 1. 检查inp是否存在相关问题（极短连接）
	INetwork* net = createNetwork();
	const string inp_6 = string(inp_path) + "gusumodel-6.inp";
	if (!net->readFromFile(inp_6.c_str()))
	{
		char* error_text = new char[100000];  // 要确保足够大
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 1;
	}
	if (!net->validateData())
	{
		char* error_text = new char[100000]; // 要确保足够大
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 2;
	}
	ILinkSet* links = net->getLinkSet();
	cout << "极短连接数：" << get_shortest_link_counts(links, 2.0) << "\n";

	// 2. 调整有问题的数据
	const string inp_7 = string(inp_path) + "gusumodel-7.inp";
	const string record_6 = string(inp_path) + "record-6.txt";
	remove_shortest_link(inp_6.c_str(), inp_7.c_str(), record_6.c_str(), 2.0);

	// 3. 检查调整后是否正确
	if (!net->readFromFile(inp_7.c_str()))
	{
		char* error_text = new char[100000];  // 要确保足够大
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 3;
	}
	if (!net->validateData())
	{
		char* error_text = new char[100000]; // 要确保足够大
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 4;
	}
	links = net->getLinkSet();
	cout << "极短连接数：" << get_shortest_link_counts(links, 2.0) << "\n";
	
	// 4. 检查inp是否存在相关问题（较短连接）
	cout << "较短连接数：" << get_shorter_link_counts(links, 10.0, 0.1) << "\n";

	// 5. 调整有问题的数据
	const string inp_8 = string(inp_path) + "gusumodel-8.inp";
	const string record_7 = string(inp_path) + "record-7.txt";
	remove_shorter_link(
		inp_7.c_str(), inp_8.c_str(), record_7.c_str(), 10.0, 0.1);

	// 6. 检查调整后是否正确
	if (!net->readFromFile(inp_8.c_str()))
	{
		char* error_text = new char[100000];  // 要确保足够大
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 5;
	}
	if (!net->validateData())
	{
		char* error_text = new char[100000]; // 要确保足够大
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 6;
	}
	links = net->getLinkSet();
	cout << "较短连接数：" << get_shorter_link_counts(links, 10.0, 0.1) << "\n";

	deleteNetwork(net); 

	system("pause");

	return 0;
}
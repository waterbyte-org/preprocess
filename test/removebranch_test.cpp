/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.0.3
 * File       ：删除支管
 * Date       ：05/11/2022
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#include "path.h"
#include "../src/removebranch.h"

#include <string>
#include <iostream>

int main()
{
	using namespace std;

	// 1. 检查inp是否存在相关问题（边侧单连通）
	INetwork* net = createNetwork();
	const string inp_3 = string(inp_path) + "gusumodel-3.inp";
	if (!net->readFromFile(inp_3.c_str()))
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
	INodeSet* nodes = net->getNodeSet();
	cout << "拥有边侧单连接支管的节点数：" <<
		get_one_link_lateral_branch_counts(nodes) << "\n";

	// 2. 调整有问题的数据
	const string inp_4 = string(inp_path) + "gusumodel-4.inp";
	const string record_2 = string(inp_path) + "record-2.txt";
	remove_one_link_lateral_branch(
		inp_3.c_str(), inp_4.c_str(), record_2.c_str());

	// 3. 检查调整后是否正确
	if (!net->readFromFile(inp_4.c_str()))
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
	nodes = net->getNodeSet();
	cout << "拥有边侧单连接支管的节点数：" <<
		get_one_link_lateral_branch_counts(nodes) << "\n";

	// 4. 检查inp是否存在相关问题（末端单连接）
	//    创建空间信息对象，读入并检查空间信息数据
	IGeoprocess* igp = createGeoprocess();
	if (!igp->openGeoFile(inp_4.c_str()))
	{
		char* error_text = new char[100000];
		igp->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return 5;
	}
	if (!igp->validateData())
	{
		char* error_text = new char[100000];
		igp->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return 6;
	}
	cout << "拥有末端单连接支管的节点数：" <<
		get_one_link_terminal_branch_counts(nodes, igp, -0.866, 15) << "\n";

	// 5. 调整有问题的数据
	const string inp_5 = string(inp_path) + "gusumodel-5.inp";
	const string record_3 = string(inp_path) + "record-3.txt";
	remove_one_link_terminal_branch(
		inp_4.c_str(), inp_5.c_str(), record_3.c_str(), -0.866, 15);

	// 6. 检查调整后是否正确
	if (!net->readFromFile(inp_5.c_str()))
	{
		char* error_text = new char[100000];  // 要确保足够大
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 7;
	}
	if (!net->validateData())
	{
		char* error_text = new char[100000]; // 要确保足够大
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 8;
	}	
	if (!igp->openGeoFile(inp_5.c_str()))
	{
		char* error_text = new char[100000];
		igp->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return 9;
	}
	if (!igp->validateData())
	{
		char* error_text = new char[100000];
		igp->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return 10;
	}
	nodes = net->getNodeSet();
	cout << "拥有末端单连接支管的节点数：" <<
		get_one_link_terminal_branch_counts(nodes, igp, -0.866, 15) << "\n";

	// 7. 检查inp是否存在相关问题（边侧双连接）    
	cout << "拥有边侧双连接支管的节点数：" 
		 << get_two_link_lateral_branch_counts(nodes) << "\n";

	// 8. 调整有问题的数据
	const string inp_6 = string(inp_path) + "gusumodel-6.inp";
	const string record_4 = string(inp_path) + "record-4.txt";
	remove_two_link_lateral_branch(
		inp_5.c_str(), inp_6.c_str(), record_4.c_str());

	// 9. 检查调整后是否正确
	if (!net->readFromFile(inp_6.c_str()))
	{
		char* error_text = new char[100000];  // 要确保足够大
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 11;
	}
	if (!net->validateData())
	{
		char* error_text = new char[100000]; // 要确保足够大
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 12;
	}	
	nodes = net->getNodeSet();
	cout << "拥有边侧双连接支管的节点数："
		 << get_two_link_lateral_branch_counts(nodes) << "\n";

	deleteGeoprocess(igp);
	deleteNetwork(net); 

	system("pause");

	return 0;
}
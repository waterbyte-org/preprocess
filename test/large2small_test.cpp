/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.0.3
 * File       ：大管接小管测试
 * Date       ：05/12/2022
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#include "path.h"
#include "../src/large2small.h"

#include <string>
#include <iostream>

int main()
{
	using namespace std;

	// 1. 检查inp是否存在相关问题
	INetwork* net = createNetwork();
	const string inp_13 = string(inp_path) + "gusumodel-13.inp";
	if (!net->readFromFile(inp_13.c_str()))
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

	// 2. 检查大管接小管数量
	INodeSet* nodes = net->getNodeSet();
	ILinkSet* links = net->getLinkSet();
	cout << "大管接小管数量：" << get_large2small_counts(nodes) << "\n";
	cout << "直线段不协调管道数量：" << get_discordant_counts(links) << "\n";
	cout << "直线段严格不协调管道数量：" << 
		get_strict_discordant_counts(links, 0) << "\n";
	cout << "直线段严格不协调管道数量(中间小)：" <<
		get_strict_discordant_counts(links, -1) << "\n";
	cout << "直线段严格不协调管道数量(中间大)：" <<
		get_strict_discordant_counts(links, 1) << "\n";

	// 3. 保存相关信息
	const string record_12 = string(inp_path) + "record-12.txt";
	const string record_13 = string(inp_path) + "record-13.txt";
	const string record_14 = string(inp_path) + "record-14.txt";
	const string inp_14 = string(inp_path) + "gusumodel-14.inp";
	record_large2small(inp_13.c_str(), record_12.c_str());
	record_discordant(inp_13.c_str(), record_13.c_str());
	adjust_strict_discordant(
		inp_13.c_str(), inp_14.c_str(), record_14.c_str(), 0);

	// 4. 检查调整后是否还有严格不协调管道
	if (!net->readFromFile(inp_14.c_str()))
		return 3;
	if (!net->validateData())
		return 4;
	links = net->getLinkSet();
	cout << "直线段严格不协调管道数量：" <<
		get_strict_discordant_counts(links, 0) << "\n";

	return 0;
}
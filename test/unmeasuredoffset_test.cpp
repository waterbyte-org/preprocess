/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.0.3
 * File       ：寻找非测量连接偏移
 * Date       ：05/11/2022
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#include "path.h"
#include "../src/unmeasuredoffset.h"

#include <iostream>

int main()
{
	using namespace std;

	// 1. 检查inp是否存在相关问题
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

	// 2. 查看有多少问题连接
	cout << "具有完全相同偏移值的组合数（3条连接以上）：" << 
		get_unmeasured_link_counts(net, 3) << "\n";

	// 3. 保存可能有问题的连接名称
	const string record_7 = string(inp_path) + "record-7.txt";
	record_unmeasured_link(inp_6.c_str(), record_7.c_str(), 3);

	return 0;
}
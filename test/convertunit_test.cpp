/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.0.3
 * File       ：流量单位转换
 * Date       ：05/11/2022
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#include "path.h"
#include "../src/convertunit.h"

#include <string>
#include <iostream>

int main()
{
	using namespace std;

	// 1. 创建模型对象
	INetwork* net = createNetwork();
	if (net == nullptr)
	{
		cout << "创建模型对象失败!" << "\n";
		return 1;
	}

	// 2. 读入inp文件
	const string inp_1 = string(inp_path) + "gusumodel-1.inp";
	if (!net->readFromFile(inp_1.c_str()))
	{
		char* error_text = new char[100000];  // 要确保足够大
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 2;
	}

	// 3. 将节点旱季流量基准值除以86400，即乘以1/86400
	INodeSet* nodes = net->getNodeSet();
	multiply_dwf_base(nodes, 1.0 / 86400);

	// 4. 将节点旱季流量产污系数乘以0.001267 * 864 = 1.094688
	//    说明：产污系数初始值为1.0
	multiply_dwf_scale(nodes, 1.094688);
		
	// 5. 将时变化模式值乘以100.0
	IPatternSet* ips = net->getPatternSet();
	multiply_hourly_pattern(ips, 100.0);

	// 6. 将日变化模式值设为1.0
	set_daily_pattern(ips, 1.0);

	// 7. 将处理后的模型保存为新的inp文件
	const string inp_2 = string(inp_path) + "gusumodel-2.inp";
	net->save(inp_2.c_str());    

	deleteNetwork(net);

	system("pause");

	return 0;
}
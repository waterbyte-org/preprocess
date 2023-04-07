/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.0.3
 * File       ：分开重合连接测试
 * Date       ：05/11/2022
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#include "path.h"
#include "../src/splitlink.h"

#include <string>
#include <iostream>

int main()
{
	using namespace std;

	INetwork* net = createNetwork();
	const string inp_12 = string(inp_path) + "gusumodel-12.inp";
	if (!net->readFromFile(inp_12.c_str()))
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
	IGeoprocess* igp = createGeoprocess();
	if (!igp->openGeoFile(inp_12.c_str()))
	{
		char* error_text = new char[100000];
		igp->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return 3;
	}
	if (!igp->validateData())
	{
		char* error_text = new char[100000];
		igp->getErrorText(error_text);
		std::cout << error_text << "\n";
		delete[] error_text;
		return 4;
	}

	cout << "共线连接数：" << get_collineated_link_counts(net, igp) << "\n";
	const string inp_13 = string(inp_path) + "gusumodel-13.inp";
	const string tmp_geo = string(inp_path) + "tmp_geo.txt";
	split_collineated_link(
		inp_12.c_str(), inp_13.c_str(), tmp_geo.c_str(), 0.1);

	return 0;
}
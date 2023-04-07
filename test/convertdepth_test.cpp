/*
 *******************************************************************************
 * Project    ��PreProcess
 * Version    ��1.0.3
 * File       �������߶����
 * Date       ��05/13/2022
 * Author     ����С��
 * Copyright  ������ˮ�ֽڿƼ����޹�˾
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

	// 1. ���inp�Ƿ�����������
	const string inp_2 = string(inp_path) + "gusumodel-2.inp";
	INetwork* net = createNetwork();
	if (!net->readFromFile(inp_2.c_str()))
	{
		char* error_text = new char[100000]; // Ҫȷ���㹻��
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 1;
	}
	INodeSet* nodes = net->getNodeSet();
	ILinkSet* links = net->getLinkSet();
	links->associateNode(nodes);
	cout << "���Ӳ��뾮�׵Ľڵ�������" << get_concave_counts(nodes) << "\n";
	cout << "����¶������Ľڵ�������" << get_convex_counts(nodes)  << "\n";
	cout << "���Ȳ������½��Ĺ�������������" 
		 << get_error_link_counts(links) << "\n";

	// 2. ���������������
	const string inp_3 = string(inp_path) + "gusumodel-3.inp";
	const string record_1 = string(inp_path) + "record-1.txt";
	convert_depth(inp_2.c_str(), inp_3.c_str(), record_1.c_str(), 1.5);

	// 3. ���������Ƿ���ȷ
	if (!net->readFromFile(inp_3.c_str()))
	{
		char* error_text = new char[100000]; // Ҫȷ���㹻��
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 2;
	}
	nodes = net->getNodeSet();
	links = net->getLinkSet();
	links->associateNode(nodes);
	cout << "*****������*****" << "\n";
	cout << "���Ӳ��뾮�׵Ľڵ�������" << get_concave_counts(nodes) << "\n";
	cout << "����¶������Ľڵ�������" << get_convex_counts(nodes)  << "\n";
	cout << "���Ȳ������½��Ĺ�������������" 
		 << get_error_link_counts(links) << "\n";

	deleteNetwork(net);

	system("pause");

	return 0;
}
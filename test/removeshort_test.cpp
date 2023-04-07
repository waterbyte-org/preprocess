/*
 *******************************************************************************
 * Project    ��PreProcess
 * Version    ��1.0.3
 * File       ��ɾ���̹�
 * Date       ��05/13/2022
 * Author     ����С��
 * Copyright  ������ˮ�ֽڿƼ����޹�˾
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

	// 1. ���inp�Ƿ����������⣨�������ӣ�
	INetwork* net = createNetwork();
	const string inp_6 = string(inp_path) + "gusumodel-6.inp";
	if (!net->readFromFile(inp_6.c_str()))
	{
		char* error_text = new char[100000];  // Ҫȷ���㹻��
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 1;
	}
	if (!net->validateData())
	{
		char* error_text = new char[100000]; // Ҫȷ���㹻��
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 2;
	}
	ILinkSet* links = net->getLinkSet();
	cout << "������������" << get_shortest_link_counts(links, 2.0) << "\n";

	// 2. ���������������
	const string inp_7 = string(inp_path) + "gusumodel-7.inp";
	const string record_6 = string(inp_path) + "record-6.txt";
	remove_shortest_link(inp_6.c_str(), inp_7.c_str(), record_6.c_str(), 2.0);

	// 3. ���������Ƿ���ȷ
	if (!net->readFromFile(inp_7.c_str()))
	{
		char* error_text = new char[100000];  // Ҫȷ���㹻��
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 3;
	}
	if (!net->validateData())
	{
		char* error_text = new char[100000]; // Ҫȷ���㹻��
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 4;
	}
	links = net->getLinkSet();
	cout << "������������" << get_shortest_link_counts(links, 2.0) << "\n";
	
	// 4. ���inp�Ƿ����������⣨�϶����ӣ�
	cout << "�϶���������" << get_shorter_link_counts(links, 10.0, 0.1) << "\n";

	// 5. ���������������
	const string inp_8 = string(inp_path) + "gusumodel-8.inp";
	const string record_7 = string(inp_path) + "record-7.txt";
	remove_shorter_link(
		inp_7.c_str(), inp_8.c_str(), record_7.c_str(), 10.0, 0.1);

	// 6. ���������Ƿ���ȷ
	if (!net->readFromFile(inp_8.c_str()))
	{
		char* error_text = new char[100000];  // Ҫȷ���㹻��
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 5;
	}
	if (!net->validateData())
	{
		char* error_text = new char[100000]; // Ҫȷ���㹻��
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 6;
	}
	links = net->getLinkSet();
	cout << "�϶���������" << get_shorter_link_counts(links, 10.0, 0.1) << "\n";

	deleteNetwork(net); 

	system("pause");

	return 0;
}
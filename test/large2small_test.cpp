/*
 *******************************************************************************
 * Project    ��PreProcess
 * Version    ��1.0.3
 * File       ����ܽ�С�ܲ���
 * Date       ��05/12/2022
 * Author     ����С��
 * Copyright  ������ˮ�ֽڿƼ����޹�˾
 *******************************************************************************
 */

#include "path.h"
#include "../src/large2small.h"

#include <string>
#include <iostream>

int main()
{
	using namespace std;

	// 1. ���inp�Ƿ�����������
	INetwork* net = createNetwork();
	const string inp_13 = string(inp_path) + "gusumodel-13.inp";
	if (!net->readFromFile(inp_13.c_str()))
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

	// 2. ����ܽ�С������
	INodeSet* nodes = net->getNodeSet();
	ILinkSet* links = net->getLinkSet();
	cout << "��ܽ�С��������" << get_large2small_counts(nodes) << "\n";
	cout << "ֱ�߶β�Э���ܵ�������" << get_discordant_counts(links) << "\n";
	cout << "ֱ�߶��ϸ�Э���ܵ�������" << 
		get_strict_discordant_counts(links, 0) << "\n";
	cout << "ֱ�߶��ϸ�Э���ܵ�����(�м�С)��" <<
		get_strict_discordant_counts(links, -1) << "\n";
	cout << "ֱ�߶��ϸ�Э���ܵ�����(�м��)��" <<
		get_strict_discordant_counts(links, 1) << "\n";

	// 3. ���������Ϣ
	const string record_12 = string(inp_path) + "record-12.txt";
	const string record_13 = string(inp_path) + "record-13.txt";
	const string record_14 = string(inp_path) + "record-14.txt";
	const string inp_14 = string(inp_path) + "gusumodel-14.inp";
	record_large2small(inp_13.c_str(), record_12.c_str());
	record_discordant(inp_13.c_str(), record_13.c_str());
	adjust_strict_discordant(
		inp_13.c_str(), inp_14.c_str(), record_14.c_str(), 0);

	// 4. ���������Ƿ����ϸ�Э���ܵ�
	if (!net->readFromFile(inp_14.c_str()))
		return 3;
	if (!net->validateData())
		return 4;
	links = net->getLinkSet();
	cout << "ֱ�߶��ϸ�Э���ܵ�������" <<
		get_strict_discordant_counts(links, 0) << "\n";

	return 0;
}
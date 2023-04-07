/*
 *******************************************************************************
 * Project    ��PreProcess
 * Version    ��1.0.3
 * File       ��Ѱ�ҷǲ�������ƫ��
 * Date       ��05/11/2022
 * Author     ����С��
 * Copyright  ������ˮ�ֽڿƼ����޹�˾
 *******************************************************************************
 */

#include "path.h"
#include "../src/unmeasuredoffset.h"

#include <iostream>

int main()
{
	using namespace std;

	// 1. ���inp�Ƿ�����������
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

	// 2. �鿴�ж�����������
	cout << "������ȫ��ͬƫ��ֵ���������3���������ϣ���" << 
		get_unmeasured_link_counts(net, 3) << "\n";

	// 3. ����������������������
	const string record_7 = string(inp_path) + "record-7.txt";
	record_unmeasured_link(inp_6.c_str(), record_7.c_str(), 3);

	return 0;
}
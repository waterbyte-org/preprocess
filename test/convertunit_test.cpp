/*
 *******************************************************************************
 * Project    ��PreProcess
 * Version    ��1.0.3
 * File       ��������λת��
 * Date       ��05/11/2022
 * Author     ����С��
 * Copyright  ������ˮ�ֽڿƼ����޹�˾
 *******************************************************************************
 */

#include "path.h"
#include "../src/convertunit.h"

#include <string>
#include <iostream>

int main()
{
	using namespace std;

	// 1. ����ģ�Ͷ���
	INetwork* net = createNetwork();
	if (net == nullptr)
	{
		cout << "����ģ�Ͷ���ʧ��!" << "\n";
		return 1;
	}

	// 2. ����inp�ļ�
	const string inp_1 = string(inp_path) + "gusumodel-1.inp";
	if (!net->readFromFile(inp_1.c_str()))
	{
		char* error_text = new char[100000];  // Ҫȷ���㹻��
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 2;
	}

	// 3. ���ڵ㺵��������׼ֵ����86400��������1/86400
	INodeSet* nodes = net->getNodeSet();
	multiply_dwf_base(nodes, 1.0 / 86400);

	// 4. ���ڵ㺵����������ϵ������0.001267 * 864 = 1.094688
	//    ˵��������ϵ����ʼֵΪ1.0
	multiply_dwf_scale(nodes, 1.094688);
		
	// 5. ��ʱ�仯ģʽֵ����100.0
	IPatternSet* ips = net->getPatternSet();
	multiply_hourly_pattern(ips, 100.0);

	// 6. ���ձ仯ģʽֵ��Ϊ1.0
	set_daily_pattern(ips, 1.0);

	// 7. ��������ģ�ͱ���Ϊ�µ�inp�ļ�
	const string inp_2 = string(inp_path) + "gusumodel-2.inp";
	net->save(inp_2.c_str());    

	deleteNetwork(net);

	system("pause");

	return 0;
}
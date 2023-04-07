/*
 *******************************************************************************
 * Project    ��PreProcess
 * Version    ��1.0.3
 * File       ��ɾ��֧��
 * Date       ��05/11/2022
 * Author     ����С��
 * Copyright  ������ˮ�ֽڿƼ����޹�˾
 *******************************************************************************
 */

#include "path.h"
#include "../src/removebranch.h"

#include <string>
#include <iostream>

int main()
{
	using namespace std;

	// 1. ���inp�Ƿ����������⣨�߲൥��ͨ��
	INetwork* net = createNetwork();
	const string inp_3 = string(inp_path) + "gusumodel-3.inp";
	if (!net->readFromFile(inp_3.c_str()))
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
	INodeSet* nodes = net->getNodeSet();
	cout << "ӵ�б߲൥����֧�ܵĽڵ�����" <<
		get_one_link_lateral_branch_counts(nodes) << "\n";

	// 2. ���������������
	const string inp_4 = string(inp_path) + "gusumodel-4.inp";
	const string record_2 = string(inp_path) + "record-2.txt";
	remove_one_link_lateral_branch(
		inp_3.c_str(), inp_4.c_str(), record_2.c_str());

	// 3. ���������Ƿ���ȷ
	if (!net->readFromFile(inp_4.c_str()))
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
	nodes = net->getNodeSet();
	cout << "ӵ�б߲൥����֧�ܵĽڵ�����" <<
		get_one_link_lateral_branch_counts(nodes) << "\n";

	// 4. ���inp�Ƿ����������⣨ĩ�˵����ӣ�
	//    �����ռ���Ϣ���󣬶��벢���ռ���Ϣ����
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
	cout << "ӵ��ĩ�˵�����֧�ܵĽڵ�����" <<
		get_one_link_terminal_branch_counts(nodes, igp, -0.866, 15) << "\n";

	// 5. ���������������
	const string inp_5 = string(inp_path) + "gusumodel-5.inp";
	const string record_3 = string(inp_path) + "record-3.txt";
	remove_one_link_terminal_branch(
		inp_4.c_str(), inp_5.c_str(), record_3.c_str(), -0.866, 15);

	// 6. ���������Ƿ���ȷ
	if (!net->readFromFile(inp_5.c_str()))
	{
		char* error_text = new char[100000];  // Ҫȷ���㹻��
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 7;
	}
	if (!net->validateData())
	{
		char* error_text = new char[100000]; // Ҫȷ���㹻��
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
	cout << "ӵ��ĩ�˵�����֧�ܵĽڵ�����" <<
		get_one_link_terminal_branch_counts(nodes, igp, -0.866, 15) << "\n";

	// 7. ���inp�Ƿ����������⣨�߲�˫���ӣ�    
	cout << "ӵ�б߲�˫����֧�ܵĽڵ�����" 
		 << get_two_link_lateral_branch_counts(nodes) << "\n";

	// 8. ���������������
	const string inp_6 = string(inp_path) + "gusumodel-6.inp";
	const string record_4 = string(inp_path) + "record-4.txt";
	remove_two_link_lateral_branch(
		inp_5.c_str(), inp_6.c_str(), record_4.c_str());

	// 9. ���������Ƿ���ȷ
	if (!net->readFromFile(inp_6.c_str()))
	{
		char* error_text = new char[100000];  // Ҫȷ���㹻��
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 11;
	}
	if (!net->validateData())
	{
		char* error_text = new char[100000]; // Ҫȷ���㹻��
		net->getErrorText(error_text);
		cout << error_text << "\n";
		delete[] error_text;
		return 12;
	}	
	nodes = net->getNodeSet();
	cout << "ӵ�б߲�˫����֧�ܵĽڵ�����"
		 << get_two_link_lateral_branch_counts(nodes) << "\n";

	deleteGeoprocess(igp);
	deleteNetwork(net); 

	system("pause");

	return 0;
}
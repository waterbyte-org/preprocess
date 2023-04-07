/*
 *******************************************************************************
 * Project    ��PreProcess
 * Version    ��1.0.3
 * File       �����Һ�����inp��������
 * Date       ��05/11/2022
 * Author     ����С��
 * Copyright  ������ˮ�ֽڿƼ����޹�˾
 *******************************************************************************
 */

#include "path.h"
#include "../src/removeshort.h"
#include "../src/convertdepth.h"
#include "../src/removebranch.h"
#include "../src/unmeasuredoffset.h"

#include <string>
#include <iostream>

int main()
{
	using namespace std;

	// 1. ����ģ�Ͷ��󣬶���inp�ļ����ѵ���������λ��
	INetwork* net = createNetwork();
	const string inp_2 = string(inp_path) + "gusumodel-2.inp";
	if (!net->readFromFile(inp_2.c_str()))
		return 1;

	// 2. �������˹�ϵ��������Ƿ���ڸ߳���ص�����
	INodeSet* nodes = net->getNodeSet();
	ILinkSet* links = net->getLinkSet();
	links->associateNode(nodes);
	cout << "���Ӳ��뾮�׵Ľڵ�������" << get_concave_counts(nodes) << "\n";
	cout << "����¶������Ľڵ�������" << get_convex_counts(nodes) << "\n";
	cout << "���Ȳ������½��Ĺ�������������"
		<< get_error_link_counts(links) << "\n";

	// 3. �����߳���ص�����
	const string inp_3 = string(inp_path) + "gusumodel-3.inp";
	const string record_1 = string(inp_path) + "record-1.txt";
	convert_depth(inp_2.c_str(), inp_3.c_str(), record_1.c_str(), 1.5);

	// 4. ���벢���������ߺ��inp�ļ�
	if (!net->readFromFile(inp_3.c_str()))
		return 2;
	if (!net->validateData())
		return 3;
	IGeoprocess* igp = createGeoprocess(); // ����Ҫ�õ�
	if (!igp->openGeoFile(inp_3.c_str()))
		return 4;
	if (!igp->validateData())
		return 5;

	// 5. �ȼ���Ƿ񻹴��ڱ�����⣬�ټ����Ҫɾ���ĸ���֧������
	nodes = net->getNodeSet();
	links = net->getLinkSet();	
	cout << "���Ӳ��뾮�׵Ľڵ�������" << get_concave_counts(nodes) << "\n";
	cout << "����¶������Ľڵ�������" << get_convex_counts(nodes) << "\n";
	cout << "���Ȳ������½��Ĺ�������������"
		<< get_error_link_counts(links) << "\n";
	cout << "ӵ�б߲൥����֧�ܵĽڵ�����" <<
		get_one_link_lateral_branch_counts(nodes) << "\n";	
	cout << "ӵ��ĩ�˵�����֧�ܵĽڵ�����" <<
		get_one_link_terminal_branch_counts(nodes, igp, -0.866, 15) << "\n";
	cout << "ӵ�б߲�˫����֧�ܵĽڵ�����"
		<< get_two_link_lateral_branch_counts(nodes) << "\n";

	// 6. ��ɾ������֧�ܡ�ע�⣺����һ����ɾ����
	const string inp_4 = string(inp_path) + "gusumodel-4.inp";
	const string record_2 = string(inp_path) + "record-2.txt";
	remove_one_link_lateral_branch(
		inp_3.c_str(), inp_4.c_str(), record_2.c_str());
	const string inp_5 = string(inp_path) + "gusumodel-5.inp";
	const string record_3 = string(inp_path) + "record-3.txt";
	remove_one_link_terminal_branch(
		inp_4.c_str(), inp_5.c_str(), record_3.c_str(), -0.866, 15);
	const string inp_6 = string(inp_path) + "gusumodel-6.inp";
	const string record_4 = string(inp_path) + "record-4.txt";
	remove_two_link_lateral_branch(
		inp_5.c_str(), inp_6.c_str(), record_4.c_str());

	// 7. ���벢���ɾ��֧�ܺ��inp�ļ����ټ�������ȫ��ͬƫ��ֵ�����
	if (!net->readFromFile(inp_6.c_str()))
		return 6;
	if (!net->validateData())
		return 7;
	cout << "������ȫ��ͬƫ��ֵ�����������3���������ϣ���" 
		 << get_unmeasured_link_counts(net, 3) << "\n";

	// 8. ���Ҳ���������ȫ��ͬƫ��ֵ����ϼ�¼����
	const string record_5 = string(inp_path) + "record-5.txt";
	record_unmeasured_link(inp_6.c_str(), record_5.c_str(), 3);

	// 9. ����Ƿ���ڶ̹ܡ�ע�⣺�϶����Ӱ�����������
	links = net->getLinkSet();
	cout << "������������"
		<< get_shortest_link_counts(links, 2.0) << "\n";
	cout << "�϶���������" 
		<< get_shorter_link_counts(links, 10.0, 0.1) << "\n";

	// 10. ��ɾ������̹ܡ�ע�⣺����һ����ɾ����
	const string inp_7 = string(inp_path) + "gusumodel-7.inp";
	const string record_6 = string(inp_path) + "record-6.txt";
	remove_shortest_link(inp_6.c_str(), inp_7.c_str(), record_6.c_str(), 2.0);
	const string inp_8 = string(inp_path) + "gusumodel-8.inp";
	const string record_7 = string(inp_path) + "record-7.txt";
	remove_shorter_link(
		inp_7.c_str(), inp_8.c_str(), record_7.c_str(), 10.0, 0.1);

	// 11. �����������Ƿ񻹴���
	if (!net->readFromFile(inp_8.c_str()))
		return 8;
	if (!net->validateData())
		return 9;	
	if (!igp->openGeoFile(inp_8.c_str()))
		return 10;
	if (!igp->validateData())
		return 11;
	nodes = net->getNodeSet();
	links = net->getLinkSet();
	cout << "���Ӳ��뾮�׵Ľڵ�������" << get_concave_counts(nodes) << "\n";
	cout << "����¶������Ľڵ�������" << get_convex_counts(nodes) << "\n";
	cout << "���Ȳ������½��Ĺ�������������"
		<< get_error_link_counts(links) << "\n";
	cout << "ӵ�б߲൥����֧�ܵĽڵ�����" <<
		get_one_link_lateral_branch_counts(nodes) << "\n";
	cout << "ӵ��ĩ�˵�����֧�ܵĽڵ�����" <<
		get_one_link_terminal_branch_counts(nodes, igp, -0.866, 15) << "\n";
	cout << "ӵ�б߲�˫����֧�ܵĽڵ�����"
		<< get_two_link_lateral_branch_counts(nodes) << "\n";
	cout << "������������"
		<< get_shortest_link_counts(links, 2.0) << "\n";
	cout << "�϶���������"
		<< get_shorter_link_counts(links, 10.0, 0.1) << "\n";

	// 12. ��Ȼ����һЩ���⣬�����Щ�����������
	const string inp_9 = string(inp_path) + "gusumodel-9.inp";
	const string record_8 = string(inp_path) + "record-8.txt";
	remove_one_link_lateral_branch(
		inp_8.c_str(), inp_9.c_str(), record_8.c_str());
	const string inp_10 = string(inp_path) + "gusumodel-10.inp";
	const string record_9 = string(inp_path) + "record-9.txt";
	remove_one_link_terminal_branch(
		inp_9.c_str(), inp_10.c_str(), record_9.c_str(), -0.866, 15);
	const string inp_11 = string(inp_path) + "gusumodel-11.inp";
	const string record_10 = string(inp_path) + "record-10.txt";
	remove_two_link_lateral_branch(
		inp_10.c_str(), inp_11.c_str(), record_10.c_str());
	const string inp_12 = string(inp_path) + "gusumodel-12.inp";
	const string record_11 = string(inp_path) + "record-11.txt";
	remove_shorter_link(
		inp_11.c_str(), inp_12.c_str(), record_11.c_str(), 10.0, 0.1);

	// 13. �ٴμ����������Ƿ񻹴���
	if (!net->readFromFile(inp_12.c_str()))
		return 12;
	if (!net->validateData())
		return 13;
	if (!igp->openGeoFile(inp_12.c_str()))
		return 14;
	if (!igp->validateData())
		return 15;
	nodes = net->getNodeSet();
	links = net->getLinkSet();
	cout << "���Ӳ��뾮�׵Ľڵ�������" << get_concave_counts(nodes) << "\n";
	cout << "����¶������Ľڵ�������" << get_convex_counts(nodes) << "\n";
	cout << "���Ȳ������½��Ĺ�������������"
		<< get_error_link_counts(links) << "\n";
	cout << "ӵ�б߲൥����֧�ܵĽڵ�����" <<
		get_one_link_lateral_branch_counts(nodes) << "\n";
	cout << "ӵ��ĩ�˵�����֧�ܵĽڵ�����" <<
		get_one_link_terminal_branch_counts(nodes, igp, -0.866, 15) << "\n";
	cout << "ӵ�б߲�˫����֧�ܵĽڵ�����"
		<< get_two_link_lateral_branch_counts(nodes) << "\n";
	cout << "������������"
		<< get_shortest_link_counts(links, 2.0) << "\n";
	cout << "�϶���������"
		<< get_shorter_link_counts(links, 10.0, 0.1) << "\n";

	// 13. ��Ȼ����һЩ���⣬�����Щ�����������
	//     ʡ��

	deleteNetwork(net);
	deleteGeoprocess(igp);

	return 0;
}
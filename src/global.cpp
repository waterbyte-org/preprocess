/*
 *******************************************************************************
 * Project    ��PreProcess
 * Version    ��1.0.3
 * File       ��ȫ�ֺ���
 * Date       ��05/13/2022
 * Author     ����С��
 * Copyright  ������ˮ�ֽڿƼ����޹�˾
 *******************************************************************************
 */

#include "global.h"

#include <iomanip>
#include <fstream>

int get_total_degree(INode* node)
//------------------------------------------------------------------------------
// Ŀ�ģ���ȡ�ڵ�Ķȣ�=����+��ȣ�
//------------------------------------------------------------------------------
{
	return node->getDnLinkCounts() + node->getUpLinkCounts();
}

void sub_node_invert(INode* node, double suber)
//------------------------------------------------------------------------------
// Ŀ�ģ����ڵ�ľ��ױ�߼�Сsuber
//------------------------------------------------------------------------------
{
	node->setInvertElevation(node->getInvertElevation() - suber);	
}

void add_node_full_depth(INode* node, double adder)
//------------------------------------------------------------------------------
// Ŀ�ģ����ڵ�ľ�������adder
//------------------------------------------------------------------------------
{
	node->setFullDepth(node->getFullDepth() + adder);
}

void add_link_offset(INode* node, double adder)
//------------------------------------------------------------------------------
// Ŀ�ģ����������ӵ�ƫ������adder
//------------------------------------------------------------------------------
{
	ILink* link = nullptr;

	for (int j = 0; j < node->getUpLinkCounts(); ++j)
	{
		link = node->getUpLink(j);
		link->setEndOffset(link->getEndOffset() + adder);
	}
	for (int j = 0; j < node->getDnLinkCounts(); ++j)
	{
		link = node->getDnLink(j);
		link->setBeginOffset(link->getBeginOffset() + adder);
	}
}

void merge_node_inflow(INetwork* net, INode* del_node, INode* res_node)
//------------------------------------------------------------------------------
// ������res_node = �������ڵ�
//       del_node = ��ɾ���ڵ�
// Ŀ�ģ���del_node�ڵ������������res_node�ڵ�
//------------------------------------------------------------------------------
{
	// 1�����Ϻ����������������ϵ����ʱ��ģʽ��ȫ��ͬ��
	if (IDryWeatherFlow* del_dwf = del_node->getDryWeatherFlow())
	{
		// ��del_node�ĺ�����������Ļ�׼ֵ�ӵ�res_node
		if (IDryWeatherFlow* res_dwf = res_node->getDryWeatherFlow())
			res_dwf->setBase(res_dwf->getBase() + del_dwf->getBase());
		// res_nodeû�к�����������ֱ�Ӵ�del_node���ƹ���
		else
			res_node->setDryWeatherFlow(del_dwf);
	}

	// 2�������ⲿֱ������������ʱ�����к�ʱ��ģʽ��ȫ��ͬ��
	char ts_name[30];
	char pat_name[30];	
	IPatternSet* ips = net->getPatternSet();
	ITimeseriesSet* its = net->getTimeseriesSet();
	if (IExternDirectFlow* del_edf = del_node->getExternDirectFlow())
	{		
		if (IExternDirectFlow* res_edf = res_node->getExternDirectFlow())
		{
			// ��del_node���ⲿֱ����������Ļ�׼ֵ�ӵ�res_node
			if (IPattern* del_pat = del_edf->getBasePattern())
			{
				ips->getPatternName(del_pat, pat_name);
				res_edf->setBasePattern(pat_name, ips);
				res_edf->setBaseline(
					res_edf->getBaseline() + del_edf->getBaseline());
			}
			// ��del_node���ⲿֱ����������ı������Ӽӵ�res_node
			if (ITimeseries* del_ts = del_edf->getTimeseries())
			{
				its->getTimeseriesName(del_ts, ts_name);
				res_edf->setTimeseries(ts_name, its);
				res_edf->setScaleFactor(
					res_edf->getScaleFactor() + del_edf->getScaleFactor());
			}
		}
		// res_nodeû���ⲿֱ����������ֱ�Ӵ�del_node���ƹ���
		else
			res_node->setExternDirectFlow(del_edf);
	}

	// 3������RDII�����赥λ����������ȫһ����
	if (IRDII* del_rdii = del_node->getRdii())
	{
		// ��del_node��RDII����ķ�������ӵ�res_node		
		if (IRDII* res_rdii = res_node->getRdii())
			res_rdii->setArea(res_rdii->getArea() + del_rdii->getArea());
		// res_nodeû��RDII����ֱ�Ӵ�del_node���ƹ���
		else
			res_node->setRdii(del_rdii);
	}
}

int get_leaf_link_counts(INode* node)
//------------------------------------------------------------------------------
// Ŀ�ģ���ȡ�ڵ��Ҷ��������
//------------------------------------------------------------------------------
{
	int leaf_link_counts = 0;

	for (int i = 0; i < node->getDnLinkCounts(); ++i)
	{
		if (get_total_degree(node->getDnLink(i)->getEndNode()) == 1)
			++leaf_link_counts;
	}
	for (int i = 0; i < node->getUpLinkCounts(); ++i)
	{
		if (get_total_degree(node->getUpLink(i)->getBeginNode()) == 1)
			++leaf_link_counts;
	}

	return leaf_link_counts;
}

void merge_file(const char* inp_file, const char* geo_file)
//------------------------------------------------------------------------------
// Ŀ�ģ���geo_file�ϲ���inp_file
//------------------------------------------------------------------------------
{
	if (strcmp(inp_file, geo_file) == 0) return;
	
	char c[1024];
	std::ifstream ifile(geo_file);
	std::ofstream ofile(inp_file, std::ios::app);
	while (!ifile.eof()) 
	{
		ifile.getline(c, 1023);		
		ofile << c << "\n";
	}
	ifile.close();
	ofile.close();		
}

void record_node_inflow(INode* node, std::ostringstream& oss)
//------------------------------------------------------------------------------
// Ŀ�ģ���¼�ڵ�������Ϣ
//------------------------------------------------------------------------------
{
	if (node->getDryWeatherFlow())
		oss << std::setw(11) << "��" << " ";
	else
		oss << std::setw(11) << "��" << " ";
	if (node->getExternDirectFlow())
		oss << std::setw(11) << "��" << " ";
	else
		oss << std::setw(11) << "��" << " ";
	if (node->getRdii())
		oss << std::setw(11) << "��" << " ";
	else
		oss << std::setw(11) << "��" << " ";
}

double get_ground_elevation(INode* node)
//------------------------------------------------------------------------------
// Ŀ�ģ���ȡ�ڵ������
//------------------------------------------------------------------------------
{
	return node->getFullDepth() + node->getInvertElevation();
}
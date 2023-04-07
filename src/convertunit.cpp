/*
 *******************************************************************************
 * Project    ��PreProcess
 * Version    ��1.0.0
 * File       ����������λ�����йصĺ���
 * Date       ��04/01/2022
 * Author     ����С��
 * Copyright  ������ˮ�ֽڿƼ����޹�˾
 *******************************************************************************
 */

#include "convertunit.h"

void multiply_dwf_base(INodeSet* nodes, double multiplier)
//------------------------------------------------------------------------------
// Ŀ�ģ����ڵ�ĺ���������׼ֵ������ˮ�������Ŵ�multiplier��
//------------------------------------------------------------------------------
{
	char name[30];                   // �ڵ����ƣ���ռλ���ã�	
	INode* node = nullptr;	         // �ڵ�
	IDryWeatherFlow* dwf = nullptr;  // ��������

	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		node = nodes->getNodeObjectAndName(name, i);		
		dwf = node->getDryWeatherFlow();
		if (dwf != nullptr)  // ��Щ�ڵ�û�к�����������
			dwf->setBase(dwf->getBase() * multiplier);
	}
}

void multiply_dwf_scale(INodeSet* nodes, double multiplier)
//------------------------------------------------------------------------------
// Ŀ�ģ����ڵ�ĺ�����������ϵ�����Ŵ�multiplier��
//------------------------------------------------------------------------------
{
	char name[30];                   // �ڵ����ƣ���ռλ���ã�	
	INode* node = nullptr;	         // �ڵ�
	IDryWeatherFlow* dwf = nullptr;  // ��������

	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		node = nodes->getNodeObjectAndName(name, i);
		dwf = node->getDryWeatherFlow();
		if (dwf != nullptr)  // ��Щ�ڵ�û�к�����������
			dwf->setScale(dwf->getScale() * multiplier);
	}
}

void multiply_hourly_pattern(IPatternSet* ips, double multiplier)
//------------------------------------------------------------------------------
// Ŀ�ģ���ʱ�仯ģʽֵ���Ŵ�multiplier��
//------------------------------------------------------------------------------
{
	char name[30];                   // ʱ��ģʽ���ƣ���ռλ���ã�	
	IPattern* pattern = nullptr;	

	for (int i = 0; i < ips->getPatternCounts(); ++i)
	{
		pattern = ips->getPatternObjectAndName(name, i);		
		if (pattern->isHourlyPattern())
		{
			for (int j = 0; j < pattern->getItemCounts(); ++j)
				pattern->setFactor(pattern->getFactor(j) * multiplier, j);
		}		
	}
}

void set_daily_pattern(IPatternSet* ips, double value)
//------------------------------------------------------------------------------
// Ŀ�ģ���һ���ձ仯ģʽֵ����Ϊvalue
//------------------------------------------------------------------------------
{
	char name[30];                   // ʱ��ģʽ���ƣ���ռλ���ã�
	IPattern* pattern = nullptr;
	
	for (int i = 0; i < ips->getPatternCounts(); ++i)
	{
		pattern = ips->getPatternObjectAndName(name, i);		
		if (pattern->isDailyPattern())
		{
			for (int j = 0; j < pattern->getItemCounts(); ++j)
				pattern->setFactor(value, j);
		}
	}
}
/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.0.0
 * File       ：和流量单位换算有关的函数
 * Date       ：04/01/2022
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#include "convertunit.h"

void multiply_dwf_base(INodeSet* nodes, double multiplier)
//------------------------------------------------------------------------------
// 目的：将节点的旱季流量基准值（即用水量），放大multiplier倍
//------------------------------------------------------------------------------
{
	char name[30];                   // 节点名称（起占位作用）	
	INode* node = nullptr;	         // 节点
	IDryWeatherFlow* dwf = nullptr;  // 旱季流量

	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		node = nodes->getNodeObjectAndName(name, i);		
		dwf = node->getDryWeatherFlow();
		if (dwf != nullptr)  // 有些节点没有旱季流量对象
			dwf->setBase(dwf->getBase() * multiplier);
	}
}

void multiply_dwf_scale(INodeSet* nodes, double multiplier)
//------------------------------------------------------------------------------
// 目的：将节点的旱季流量产污系数，放大multiplier倍
//------------------------------------------------------------------------------
{
	char name[30];                   // 节点名称（起占位作用）	
	INode* node = nullptr;	         // 节点
	IDryWeatherFlow* dwf = nullptr;  // 旱季流量

	for (int i = 0; i < nodes->getNodeCounts(); ++i)
	{
		node = nodes->getNodeObjectAndName(name, i);
		dwf = node->getDryWeatherFlow();
		if (dwf != nullptr)  // 有些节点没有旱季流量对象
			dwf->setScale(dwf->getScale() * multiplier);
	}
}

void multiply_hourly_pattern(IPatternSet* ips, double multiplier)
//------------------------------------------------------------------------------
// 目的：将时变化模式值，放大multiplier倍
//------------------------------------------------------------------------------
{
	char name[30];                   // 时间模式名称（起占位作用）	
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
// 目的：将一周日变化模式值，设为value
//------------------------------------------------------------------------------
{
	char name[30];                   // 时间模式名称（起占位作用）
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
/*
 *******************************************************************************
 * Project    ��PreProcess
 * Version    ��1.0.3
 * File       ����������λ�����йصĺ���
 * Date       ��05/11/2022
 * Author     ����С��
 * Copyright  ������ˮ�ֽڿƼ����޹�˾
 *******************************************************************************
 */

#pragma once

#include "headers.h"

extern void multiply_dwf_base(INodeSet* nodes, double multiplier);
extern void multiply_dwf_scale(INodeSet* nodes, double multiplier);
extern void multiply_hourly_pattern(IPatternSet* ips, double multiplier);
extern void set_daily_pattern(IPatternSet* ips, double value);
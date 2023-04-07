/*
 *******************************************************************************
 * Project    ��PreProcess
 * Version    ��1.0.3
 * File       �������߶����
 * Date       ��05/11/2022
 * Author     ����С��
 * Copyright  ������ˮ�ֽڿƼ����޹�˾
 *******************************************************************************
 */

#pragma once

#include "headers.h"

extern int get_concave_counts(INodeSet* nodes);
extern int get_convex_counts(INodeSet* nodes);
extern int get_error_link_counts(ILinkSet* links);
extern void convert_depth(const char* inp_file, const char* new_inp_file, 
	const char* record_file, double length_multiplier);
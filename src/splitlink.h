/*
 *******************************************************************************
 * Project    ��PreProcess
 * Version    ��1.0.3
 * File       ���ֿ��غ�����
 * Date       ��05/11/2022
 * Author     ����С��
 * Copyright  ������ˮ�ֽڿƼ����޹�˾
 *******************************************************************************
 */

#pragma once

#include "headers.h"

extern int get_collineated_link_counts(INetwork* net, IGeoprocess* igp);
extern void split_collineated_link(const char* inp_file, 
	const char* new_inp_file, const char* tmp_geo_file, double f);
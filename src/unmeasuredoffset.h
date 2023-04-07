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

#pragma once

#include "headers.h"

extern int get_unmeasured_link_counts(INetwork* net, int dup_times);
extern void record_unmeasured_link(const char* inp_file,
	const char* record_file, int dup_times);
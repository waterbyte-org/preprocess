/*
 *******************************************************************************
 * Project    ��PreProcess
 * Version    ��1.0.3
 * File       ��Ѱ�Ҵ�ܽ�С��
 * Date       ��05/11/2022
 * Author     ����С��
 * Copyright  ������ˮ�ֽڿƼ����޹�˾
 *******************************************************************************
 */

#pragma once

#include "headers.h"

extern int get_large2small_counts(INodeSet* nodes);
extern void record_large2small(const char* inp_file, const char* record_file);
extern int get_discordant_counts(ILinkSet* links);
extern void record_discordant(const char* inp_file, const char* record_file);
extern int get_strict_discordant_counts(ILinkSet* links, int flag);
extern void adjust_strict_discordant(const char* inp_file, 
	const char* new_inp_file, const char* record_file, int flag);
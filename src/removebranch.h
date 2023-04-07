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

#pragma once

#include "headers.h"

extern int get_one_link_lateral_branch_counts(INodeSet* nodes);
extern void remove_one_link_lateral_branch(const char* inp_file,
	const char* new_inp_file, const char* record_file);
extern int get_one_link_terminal_branch_counts(INodeSet* nodes, 
	IGeoprocess* igp, double critical_cos, double critical_length);
extern void remove_one_link_terminal_branch(const char* inp_file,
	const char* new_inp_file, const char* record_file, 
	double critical_cos, double critical_length);
extern int get_two_link_lateral_branch_counts(INodeSet* nodes);
extern void remove_two_link_lateral_branch(const char* inp_file,
	const char* new_inp_file, const char* record_file);
/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.0.3
 * File       ：删除短管
 * Date       ：05/11/2022
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#pragma once

#include "headers.h"

extern int get_shortest_link_counts(ILinkSet* links, double shortest);
extern void remove_shortest_link(const char* inp_file,
	const char* new_inp_file, const char* record_file, double shortest);
extern int get_shorter_link_counts(
	ILinkSet* links, double shorter, double loss);
extern void remove_shorter_link(const char* inp_file,
	const char* new_inp_file, const char* record_file, double shorter, 
	double loss);
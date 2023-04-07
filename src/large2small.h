/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.0.3
 * File       ：寻找大管接小管
 * Date       ：05/11/2022
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
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
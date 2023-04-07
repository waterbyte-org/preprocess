/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.0.3
 * File       ：寻找非测量连接偏移
 * Date       ：05/11/2022
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#pragma once

#include "headers.h"

extern int get_unmeasured_link_counts(INetwork* net, int dup_times);
extern void record_unmeasured_link(const char* inp_file,
	const char* record_file, int dup_times);
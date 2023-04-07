/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.0.3
 * File       ：分开重合连接
 * Date       ：05/11/2022
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#pragma once

#include "headers.h"

extern int get_collineated_link_counts(INetwork* net, IGeoprocess* igp);
extern void split_collineated_link(const char* inp_file, 
	const char* new_inp_file, const char* tmp_geo_file, double f);
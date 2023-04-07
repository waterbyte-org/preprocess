/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.0.3
 * File       ：和流量单位换算有关的函数
 * Date       ：05/11/2022
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#pragma once

#include "headers.h"

extern void multiply_dwf_base(INodeSet* nodes, double multiplier);
extern void multiply_dwf_scale(INodeSet* nodes, double multiplier);
extern void multiply_hourly_pattern(IPatternSet* ips, double multiplier);
extern void set_daily_pattern(IPatternSet* ips, double value);
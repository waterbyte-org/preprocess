/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.0.3
 * File       ：全局函数
 * Date       ：05/13/2022
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#pragma once

#include "headers.h"

#include <sstream>

extern int get_total_degree(INode* node);
extern void sub_node_invert(INode* node, double suber);
extern void add_node_full_depth(INode* node, double adder);
extern void add_link_offset(INode* node, double adder);
extern void merge_node_inflow(INetwork* net, INode* del_node, INode* res_node);
extern int get_leaf_link_counts(INode* node);
extern void merge_file(const char* inp_file, const char* geo_file);
extern void record_node_inflow(INode* node, std::ostringstream& oss);
extern double get_ground_elevation(INode* node);
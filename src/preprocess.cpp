/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.0.3
 * File       ：预处理模块接口实现
 * Date       ：05/12/2022
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define DLL_EXPORT
#endif

#include "splitlink.h"
#include "removeshort.h"
#include "large2small.h"
#include "convertdepth.h"
#include "removebranch.h"
#include "unmeasuredoffset.h"
#include "../interface/preprocess.h"

DLL_API int __stdcall getConcaveCounts(INetwork* net)
{
	return get_concave_counts(net->getNodeSet());
}

DLL_API int __stdcall getConvexCounts(INetwork* net)
{
	return get_convex_counts(net->getNodeSet());
}

DLL_API int __stdcall getErrorLinkCounts(INetwork* net)
{
	return get_error_link_counts(net->getLinkSet());
}

DLL_API void __stdcall convertDepth(const char* inp_file, 
	const char* new_inp_file, const char* record_file, double length_multiplier)
{
	convert_depth(inp_file, new_inp_file, record_file, length_multiplier);
}

DLL_API int __stdcall getOneLinkLateralBranchCounts(INetwork* net)
{
	return get_one_link_lateral_branch_counts(net->getNodeSet());
}

DLL_API void __stdcall removeOneLinkLateralBranch(const char* inp_file,
	const char* new_inp_file, const char* record_file)
{
	remove_one_link_lateral_branch(inp_file, new_inp_file, record_file);
}

DLL_API int __stdcall getOneLinkTerminalBranchCounts(INetwork* net,
	IGeoprocess* igp, double critical_cos, double critical_length)
{
	return get_one_link_terminal_branch_counts(
		net->getNodeSet(), igp, critical_cos, critical_length);
}

DLL_API void __stdcall removeOneLinkTerminalBranch(const char* inp_file,
	const char* new_inp_file, const char* record_file, double critical_cos, 
	double critical_length)
{
	remove_one_link_terminal_branch(
		inp_file, new_inp_file, record_file, critical_cos, critical_length);
}

DLL_API int __stdcall getTwoLinkLateralBranchCounts(INetwork* net)
{
	return get_two_link_lateral_branch_counts(net->getNodeSet());
}

DLL_API void __stdcall removeTwoLinkLateralBranch(const char* inp_file,
	const char* new_inp_file, const char* record_file)
{
	remove_two_link_lateral_branch(inp_file, new_inp_file, record_file);
}

DLL_API int __stdcall getShortestLinkCounts(INetwork* net, double shortest)
{
	return get_shortest_link_counts(net->getLinkSet(), shortest);
}

DLL_API void __stdcall removeShortestLink(const char* inp_file,
	const char* new_inp_file, const char* record_file, double shortest)
{
	remove_shortest_link(inp_file, new_inp_file, record_file, shortest);
}

DLL_API int __stdcall getShorterLinkCounts(INetwork* net, double shorter,
	double loss) 
{
	return get_shorter_link_counts(net->getLinkSet(), shorter, loss);
}

DLL_API void __stdcall removeShorterLink(const char* inp_file,
	const char* new_inp_file, const char* record_file, double shorter,
	double loss) 
{
	remove_shorter_link(inp_file, new_inp_file, record_file, shorter, loss);
}

DLL_API int __stdcall getUnmeasuredLinkCounts(INetwork* net, int dup_times)
{
	return get_unmeasured_link_counts(net, dup_times);
}

DLL_API void __stdcall recordUnmeasuredLink(const char* inp_file,
	const char* record_file, int dup_times)
{
	record_unmeasured_link(inp_file, record_file, dup_times);
}

DLL_API int __stdcall getCollineatedLinkCounts(
	INetwork* net, IGeoprocess* igp)
{
	return get_collineated_link_counts(net, igp);
}

DLL_API void __stdcall splitCollineatedLink(const char* inp_file,
	const char* new_inp_file, const char* tmp_geo_file, double f)
{
	split_collineated_link(inp_file, new_inp_file, tmp_geo_file, f);
}

DLL_API int __stdcall getLargeToSmallCounts(INetwork* net)
{
	return get_large2small_counts(net->getNodeSet());
}

DLL_API void __stdcall recordLargeToSmall(const char* inp_file, 
	const char* record_file)
{
	record_large2small(inp_file, record_file);
}

DLL_API int __stdcall getDiscordantCounts(INetwork* net)
{
	return get_discordant_counts(net->getLinkSet());
}

DLL_API void __stdcall recordDiscordant(
	const char* inp_file, const char* record_file)
{
	record_discordant(inp_file, record_file);
}

DLL_API int __stdcall getStrictDiscordantCounts(INetwork* net, int flag)
{
	return get_strict_discordant_counts(net->getLinkSet(), flag);
}

DLL_API void __stdcall adjustStrictDiscordant(const char* inp_file,
	const char* new_inp_file, const char* record_file, int flag)
{
	adjust_strict_discordant(inp_file, new_inp_file, record_file, flag);
}
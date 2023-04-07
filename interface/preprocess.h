/*
 *******************************************************************************
 * Project    ：PreProcess
 * Version    ：1.0.3
 * File       ：预处理模块接口文件
 * Date       ：05/12/2022
 * Author     ：梁小光
 * Copyright  ：福州水字节科技有限公司
 *******************************************************************************
 */

#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#ifdef DLL_EXPORT
#define DLL_API _declspec(dllexport) 
#else                                                                           
#define DLL_API _declspec(dllimport) 
#endif 
#else
#define __stdcall
#define DLL_API __attribute__((visibility("default")))
#endif

struct INetwork;
struct IGeoprocess;

#ifdef __cplusplus
extern "C" {
#endif

	DLL_API int __stdcall getConcaveCounts(INetwork* net);
	DLL_API int __stdcall getConvexCounts(INetwork* net);
	DLL_API int __stdcall getErrorLinkCounts(INetwork* net);
	DLL_API void __stdcall convertDepth(const char* inp_file, 
		const char* new_inp_file, const char* record_file, 
		double length_multiplier);

	DLL_API int __stdcall getOneLinkLateralBranchCounts(INetwork* net);
	DLL_API void __stdcall removeOneLinkLateralBranch(const char* inp_file,
		const char* new_inp_file, const char* record_file);
	DLL_API int __stdcall getOneLinkTerminalBranchCounts(INetwork* net,
		IGeoprocess* igp, double critical_cos, double critical_length);
	DLL_API void __stdcall removeOneLinkTerminalBranch(const char* inp_file,
		const char* new_inp_file, const char* record_file,
		double critical_cos, double critical_length);
	DLL_API int __stdcall getTwoLinkLateralBranchCounts(INetwork* net);
	DLL_API void __stdcall removeTwoLinkLateralBranch(const char* inp_file,
		const char* new_inp_file, const char* record_file);

	DLL_API int __stdcall getShortestLinkCounts(
		INetwork* net, double shortest);
	DLL_API void __stdcall removeShortestLink(const char* inp_file,
		const char* new_inp_file, const char* record_file, double shortest);
	DLL_API int __stdcall getShorterLinkCounts(
		INetwork* net, double shorter, double loss);
	DLL_API void __stdcall removeShorterLink(const char* inp_file,
		const char* new_inp_file, const char* record_file, double shorter,
		double loss);

	DLL_API int __stdcall getUnmeasuredLinkCounts(INetwork* net, int dup_times);
	DLL_API void __stdcall recordUnmeasuredLink(const char* inp_file,
		const char* record_file, int dup_times);

	DLL_API int __stdcall getCollineatedLinkCounts(
		INetwork* net, IGeoprocess* igp);
	DLL_API void __stdcall splitCollineatedLink(const char* inp_file,
		const char* new_inp_file, const char* tmp_geo_file, double f);

	DLL_API int __stdcall getLargeToSmallCounts(INetwork* net);
	DLL_API void __stdcall recordLargeToSmall(
		const char* inp_file, const char* record_file);
	DLL_API int __stdcall getDiscordantCounts(INetwork* net);
	DLL_API void __stdcall recordDiscordant(
		const char* inp_file, const char* record_file);
	DLL_API int __stdcall getStrictDiscordantCounts(INetwork* net, int flag);
	DLL_API void __stdcall adjustStrictDiscordant(const char* inp_file,
		const char* new_inp_file, const char* record_file, int flag);

#ifdef __cplusplus
}
#endif
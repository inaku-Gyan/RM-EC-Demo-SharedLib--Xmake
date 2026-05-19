/**
 * @file ecx_config.hpp
 * @brief ECX 用户默认配置文件。本文件 作为测试的一部分，用于测试 ECX 的自动发现默认名称的用户配置文件的机制。
 */

#pragma once

#define ECX_USE_DEV_DEBUG 0
#define ECX_USE_USAGE_CHECK 1

#define ECX_USE_DCACHE 1
#define ECX_DCACHE_LINE_SIZE 32

// 下面这个定义只是用于测试。不会有实际作用
#define SELF_DEFINE_MACRO 1

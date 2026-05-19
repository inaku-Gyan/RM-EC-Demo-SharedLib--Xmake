# 默认 config 下的基础测试
ecx_add_test_executable(
    NAME    test_SpscZeroCopyAtomicQueue_default_config
    SOURCES test_SpscZeroCopyAtomicQueue.cpp
    PREFIX  "SpscZeroCopyAtomicQueue::DefaultConfig::"
)

# General config 下的基础测试（启用 USAGE_ASSERT + DEV_DEBUG）
ecx_add_test_executable(
    NAME    test_SpscZeroCopyAtomicQueue_general
    SOURCES test_SpscZeroCopyAtomicQueue.cpp
            test_SpscZeroCopyAtomicQueue_BadUsage.cpp
    CONFIG  "common/ecx_config_general.hpp"
    PREFIX  "SpscZeroCopyAtomicQueue::"
)

# 禁用 usage 检查的 config
ecx_add_test_executable(
    NAME    test_SpscZeroCopyAtomicQueue_no_usage_check
    SOURCES test_SpscZeroCopyAtomicQueue.cpp
    CONFIG  "common/ecx_config_no_usage_check.hpp"
    PREFIX  "SpscZeroCopyAtomicQueue::NoUsageCheck::"
)

# 启用 cache 支持的 config
ecx_add_test_executable(
    NAME    test_SpscZeroCopyAtomicQueue_use_cache
    SOURCES test_SpscZeroCopyAtomicQueue.cpp
            test_SpscZeroCopyAtomicQueue_BadUsage.cpp
    CONFIG  "common/ecx_config_use_cache.hpp"
    PREFIX  "SpscZeroCopyAtomicQueue::UseCache::"
)

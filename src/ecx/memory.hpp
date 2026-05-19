#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "_inc/configuration.hpp"

namespace ecx {

/**
 * 指定元素的对齐方式
 */
template <typename T, std::size_t Alignment>
struct alignas(T) alignas(Alignment) AlignedElement {
    T value;
};

/**
 * 使数组中的每个元素都具有指定的对齐方式。
 * 可以用于避免伪共享（false sharing）。
 */
template <typename T, std::size_t Alignment, std::size_t N>
using AlignedArray = std::array<AlignedElement<T, Alignment>, N>;

/**
 * 缓存操作策略
 * 用于指定在特定场景下对缓存进行何种操作，以优化性能和一致性。
 */
enum class CachePolicy : uint8_t {
    // 不执行任何缓存操作，适用于不关心缓存一致性的场景
    None,
    /**
     * Cache -> SRAM. 将数据写回内存，仅确保内存中的数据是最新的。
     * 用于数据流向: CPU -> 外设
     */
    Clean,
    /**
     * SRAM -> Cache. 使缓存行失效，下次访问时从内存中重新加载。
     * 用于数据流向: 外设 -> CPU
     */
    Invalidate,
    /**
     * 仅对齐，不执行任何缓存操作。
     * 手动在容器外部管理缓存
     */
    AlignOnly,
};

#if ECX_USE_DCACHE
/// 获取缓存策略对应的对齐方式（仅当启用 D-Cache 时有效）
template <CachePolicy Policy>
inline constexpr std::size_t AlignmentByCachePolicy =
    Policy == CachePolicy::None ? 1 : ECX_DCACHE_LINE_SIZE;
#else
/// 获取缓存策略对应的对齐方式（仅当启用 D-Cache 时有效）
template <CachePolicy Policy>
    requires(Policy == CachePolicy::None)  // 只有在不使用 D-Cache 时才允许 Policy 为 None
inline constexpr std::size_t AlignmentByCachePolicy = 1;
#endif

}  // namespace ecx

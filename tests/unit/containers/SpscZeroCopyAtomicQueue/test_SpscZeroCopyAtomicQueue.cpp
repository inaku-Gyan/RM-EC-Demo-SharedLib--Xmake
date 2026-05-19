#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <vector>

#include "ecx/containers/SpscZeroCopyAtomicQueue.hpp"

namespace {

using ecx::SpscZeroCopyAtomicQueue;

TEST(SpscZeroCopyAtomicQueueTest, EmptyOnConstruction) {
    SpscZeroCopyAtomicQueue<int, 4> q;
    EXPECT_TRUE(q.empty());
    EXPECT_FALSE(q.full());
    EXPECT_EQ(q.size(), 0U);
    EXPECT_EQ(q.read_acquire(), nullptr);
    EXPECT_EQ(decltype(q)::capacity(), 4U);
}

TEST(SpscZeroCopyAtomicQueueTest, SingleWriteRead) {
    SpscZeroCopyAtomicQueue<int, 4> q;

    auto* w = q.write_acquire();
    ASSERT_NE(w, nullptr);
    *w = 42;
    q.write_commit();

    EXPECT_FALSE(q.empty());
    EXPECT_EQ(q.size(), 1U);

    const auto* r = q.read_acquire();
    ASSERT_NE(r, nullptr);
    EXPECT_EQ(*r, 42);
    q.read_commit();

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0U);
}

TEST(SpscZeroCopyAtomicQueueTest, FifoOrdering) {
    SpscZeroCopyAtomicQueue<int, 8> q;

    for (int i = 0; i < 5; ++i) {
        auto* w = q.write_acquire();
        ASSERT_NE(w, nullptr);
        *w = i * 10;
        q.write_commit();
    }
    EXPECT_EQ(q.size(), 5U);

    for (int i = 0; i < 5; ++i) {
        const auto* r = q.read_acquire();
        ASSERT_NE(r, nullptr) << "at i=" << i;
        EXPECT_EQ(*r, i * 10);
        q.read_commit();
    }
    EXPECT_TRUE(q.empty());
}

TEST(SpscZeroCopyAtomicQueueTest, FullExactlyAtCapacity) {
    // capacity = 4 应能精确容纳 4 个元素
    SpscZeroCopyAtomicQueue<int, 4> q;
    for (int i = 0; i < 4; ++i) {
        auto* w = q.write_acquire();
        ASSERT_NE(w, nullptr) << "at i=" << i;
        *w = i;
        q.write_commit();
    }
    EXPECT_TRUE(q.full());
    EXPECT_EQ(q.size(), 4U);
    EXPECT_EQ(q.write_acquire(), nullptr);  // 第 5 次写应失败
}

TEST(SpscZeroCopyAtomicQueueTest, WrapAroundIndices) {
    SpscZeroCopyAtomicQueue<int, 4> q;

    // 写满 4 个
    for (int i = 0; i < 4; ++i) {
        auto* w = q.write_acquire();
        ASSERT_NE(w, nullptr);
        *w = i;
        q.write_commit();
    }
    // 读出 3 个，留 1 个
    for (int i = 0; i < 3; ++i) {
        const auto* r = q.read_acquire();
        ASSERT_NE(r, nullptr);
        EXPECT_EQ(*r, i);
        q.read_commit();
    }
    EXPECT_EQ(q.size(), 1U);

    // 再写 3 个，此时 writer_ 会绕回开头
    for (int i = 100; i < 103; ++i) {
        auto* w = q.write_acquire();
        ASSERT_NE(w, nullptr) << "at i=" << i;
        *w = i;
        q.write_commit();
    }
    EXPECT_EQ(q.size(), 4U);
    EXPECT_TRUE(q.full());

    // 按顺序读出剩余 4 个（1 个旧 + 3 个新）
    const std::array<int, 4> expected{3, 100, 101, 102};
    for (int const e : expected) {
        const auto* r = q.read_acquire();
        ASSERT_NE(r, nullptr);
        EXPECT_EQ(*r, e);
        q.read_commit();
    }
    EXPECT_TRUE(q.empty());
}

TEST(SpscZeroCopyAtomicQueueTest, SizeAndEmptyTrackOperations) {
    SpscZeroCopyAtomicQueue<int, 4> q;
    EXPECT_TRUE(q.empty());

    auto* w = q.write_acquire();
    ASSERT_NE(w, nullptr);
    *w = 1;
    EXPECT_TRUE(q.empty()) << "未 commit 前 size 不应变化";
    q.write_commit();
    EXPECT_FALSE(q.empty());
    EXPECT_EQ(q.size(), 1U);

    const auto* r = q.read_acquire();
    ASSERT_NE(r, nullptr);
    EXPECT_EQ(q.size(), 1U) << "未 commit 前 size 不应变化";
    q.read_commit();
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0U);
}

TEST(SpscZeroCopyAtomicQueueTest, ZeroCopyPayloadIntegrity) {
    // 验证较大 POD 类型直接在槽位上读写，内容完整
    using Payload = std::array<std::uint8_t, 32>;
    SpscZeroCopyAtomicQueue<Payload, 4> q;

    auto* w = q.write_acquire();
    ASSERT_NE(w, nullptr);
    for (std::size_t i = 0; i < w->size(); ++i) { (*w)[i] = static_cast<std::uint8_t>(i ^ 0x5A); }
    q.write_commit();

    const auto* r = q.read_acquire();
    ASSERT_NE(r, nullptr);
    for (std::size_t i = 0; i < r->size(); ++i) {
        EXPECT_EQ((*r)[i], static_cast<std::uint8_t>(i ^ 0x5A)) << "at i=" << i;
    }
    q.read_commit();
}

TEST(SpscZeroCopyAtomicQueueTest, ManyWrapAroundCycles) {
    // 反复写满-读空，验证多轮绕回不丢数据、不串数据
    SpscZeroCopyAtomicQueue<int, 4> q;
    constexpr int             kCycles      = 50;
    constexpr int             kPerCycle    = 4;
    int                       next_written = 0;
    int                       next_read    = 0;

    for (int cycle = 0; cycle < kCycles; ++cycle) {
        for (int i = 0; i < kPerCycle; ++i) {
            auto* w = q.write_acquire();
            ASSERT_NE(w, nullptr);
            *w = next_written++;
            q.write_commit();
        }
        for (int i = 0; i < kPerCycle; ++i) {
            const auto* r = q.read_acquire();
            ASSERT_NE(r, nullptr);
            EXPECT_EQ(*r, next_read++);
            q.read_commit();
        }
    }
    EXPECT_TRUE(q.empty());
}

TEST(SpscZeroCopyAtomicQueueTest, ConcurrentProducerConsumer) {
    // 并发 SPSC：生产者从一个线程持续写入，消费者从另一个线程持续读出，
    // 验证不丢数据、不重复、FIFO 顺序保持
    constexpr std::uint32_t              kN = 100'000;
    SpscZeroCopyAtomicQueue<std::uint32_t, 64> q;

    std::thread producer([&] {
        for (std::uint32_t i = 0; i < kN; ++i) {
            while (true) {
                if (auto* slot = q.write_acquire()) {
                    *slot = i;
                    q.write_commit();
                    break;
                }
                std::this_thread::yield();
            }
        }
    });

    std::vector<std::uint32_t> consumed;
    consumed.reserve(kN);
    std::thread consumer([&] {
        while (consumed.size() < kN) {
            if (const auto* slot = q.read_acquire()) {
                consumed.push_back(*slot);
                q.read_commit();
            } else {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();

    ASSERT_EQ(consumed.size(), kN);
    for (std::uint32_t i = 0; i < kN; ++i) {
        ASSERT_EQ(consumed[i], i) << "FIFO 顺序被破坏 at i=" << i;
    }
    EXPECT_TRUE(q.empty());
}

}  // namespace

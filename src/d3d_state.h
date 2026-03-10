#pragma once

#include <array>
#include <atomic>
#include <cstdint>

namespace ac6::d3d {

inline constexpr uint32_t kMaxRenderTargets = 5;
inline constexpr uint32_t kMaxTextures = 16;
inline constexpr uint32_t kMaxStreams = 16;

struct DrawStats {
    std::atomic<uint32_t> draw_calls{0};
    std::atomic<uint32_t> draw_calls_indexed{0};
    std::atomic<uint32_t> draw_calls_indexed_shared{0};
    std::atomic<uint64_t> total_indices{0};
    std::atomic<uint32_t> set_texture_calls{0};
    std::atomic<uint32_t> set_render_target_calls{0};
    std::atomic<uint32_t> set_depth_stencil_calls{0};
    std::atomic<uint32_t> set_vertex_decl_calls{0};
    std::atomic<uint32_t> set_index_buffer_calls{0};
    std::atomic<uint32_t> set_stream_source_calls{0};
    std::atomic<uint32_t> set_viewport_calls{0};
    std::atomic<uint32_t> clear_calls{0};
    std::atomic<uint32_t> resolve_calls{0};

    void Reset() {
        draw_calls.store(0, std::memory_order_relaxed);
        draw_calls_indexed.store(0, std::memory_order_relaxed);
        draw_calls_indexed_shared.store(0, std::memory_order_relaxed);
        total_indices.store(0, std::memory_order_relaxed);
        set_texture_calls.store(0, std::memory_order_relaxed);
        set_render_target_calls.store(0, std::memory_order_relaxed);
        set_depth_stencil_calls.store(0, std::memory_order_relaxed);
        set_vertex_decl_calls.store(0, std::memory_order_relaxed);
        set_index_buffer_calls.store(0, std::memory_order_relaxed);
        set_stream_source_calls.store(0, std::memory_order_relaxed);
        set_viewport_calls.store(0, std::memory_order_relaxed);
        clear_calls.store(0, std::memory_order_relaxed);
        resolve_calls.store(0, std::memory_order_relaxed);
    }
};

struct DrawStatsSnapshot {
    uint32_t draw_calls;
    uint32_t draw_calls_indexed;
    uint32_t draw_calls_indexed_shared;
    uint64_t total_indices;
    uint32_t set_texture_calls;
    uint32_t set_render_target_calls;
    uint32_t set_depth_stencil_calls;
    uint32_t set_vertex_decl_calls;
    uint32_t set_index_buffer_calls;
    uint32_t set_stream_source_calls;
    uint32_t set_viewport_calls;
    uint32_t clear_calls;
    uint32_t resolve_calls;
};

// All values are guest addresses into PPC address space.
struct ShadowState {
    uint32_t device{0};
    std::array<uint32_t, kMaxRenderTargets> render_targets{};
    uint32_t depth_stencil{0};
    std::array<uint32_t, kMaxTextures> textures{};
    uint32_t vertex_declaration{0};
    uint32_t index_buffer{0};
    std::array<uint32_t, kMaxStreams> stream_sources{};

    struct {
        uint32_t x{0};
        uint32_t y{0};
        uint32_t width{0};
        uint32_t height{0};
    } viewport;
};

}  // namespace ac6::d3d

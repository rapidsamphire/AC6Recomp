#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <vector>

namespace ac6::d3d {

inline constexpr uint32_t kMaxRenderTargets = 5;
inline constexpr uint32_t kMaxTextures = 16;
inline constexpr uint32_t kMaxStreams = 16;
inline constexpr uint32_t kMaxSamplers = 16;
inline constexpr uint32_t kMaxFetchConstants = 32;

struct DrawStats {
    std::atomic<uint32_t> draw_calls{0};
    std::atomic<uint32_t> draw_calls_indexed{0};
    std::atomic<uint32_t> draw_calls_indexed_shared{0};
    std::atomic<uint32_t> draw_calls_primitive{0};
    std::atomic<uint64_t> total_indices{0};
    std::atomic<uint64_t> total_vertices{0};
    std::atomic<uint32_t> set_texture_calls{0};
    std::atomic<uint32_t> set_render_target_calls{0};
    std::atomic<uint32_t> set_depth_stencil_calls{0};
    std::atomic<uint32_t> set_vertex_decl_calls{0};
    std::atomic<uint32_t> set_index_buffer_calls{0};
    std::atomic<uint32_t> set_stream_source_calls{0};
    std::atomic<uint32_t> set_viewport_calls{0};
    std::atomic<uint32_t> set_sampler_state_calls{0};
    std::atomic<uint32_t> set_texture_fetch_calls{0};
    std::atomic<uint32_t> clear_calls{0};
    std::atomic<uint32_t> resolve_calls{0};

    void Reset() {
        draw_calls.store(0, std::memory_order_relaxed);
        draw_calls_indexed.store(0, std::memory_order_relaxed);
        draw_calls_indexed_shared.store(0, std::memory_order_relaxed);
        draw_calls_primitive.store(0, std::memory_order_relaxed);
        total_indices.store(0, std::memory_order_relaxed);
        total_vertices.store(0, std::memory_order_relaxed);
        set_texture_calls.store(0, std::memory_order_relaxed);
        set_render_target_calls.store(0, std::memory_order_relaxed);
        set_depth_stencil_calls.store(0, std::memory_order_relaxed);
        set_vertex_decl_calls.store(0, std::memory_order_relaxed);
        set_index_buffer_calls.store(0, std::memory_order_relaxed);
        set_stream_source_calls.store(0, std::memory_order_relaxed);
        set_viewport_calls.store(0, std::memory_order_relaxed);
        set_sampler_state_calls.store(0, std::memory_order_relaxed);
        set_texture_fetch_calls.store(0, std::memory_order_relaxed);
        clear_calls.store(0, std::memory_order_relaxed);
        resolve_calls.store(0, std::memory_order_relaxed);
    }
};

struct DrawStatsSnapshot {
    uint32_t draw_calls;
    uint32_t draw_calls_indexed;
    uint32_t draw_calls_indexed_shared;
    uint32_t draw_calls_primitive;
    uint64_t total_indices;
    uint64_t total_vertices;
    uint32_t set_texture_calls;
    uint32_t set_render_target_calls;
    uint32_t set_depth_stencil_calls;
    uint32_t set_vertex_decl_calls;
    uint32_t set_index_buffer_calls;
    uint32_t set_stream_source_calls;
    uint32_t set_viewport_calls;
    uint32_t set_sampler_state_calls;
    uint32_t set_texture_fetch_calls;
    uint32_t clear_calls;
    uint32_t resolve_calls;
};

struct StreamBinding {
    uint32_t buffer{0};       // Guest address of D3DVertexBuffer
    uint32_t offset{0};       // Offset in bytes
    uint32_t stride{0};       // Vertex stride in bytes
};

struct SamplerBinding {
    uint32_t mag_filter{0};   // D3DTEXTUREFILTERTYPE
    uint32_t min_filter{0};   // Sampler state A
    uint32_t mip_filter{0};   // Sampler state B
    uint32_t mip_level{0};    // Max mip level
    uint32_t border_color{0}; // Sampler state C
};

// All values are guest addresses into PPC address space unless noted.
struct ShadowState {
    uint32_t device{0};
    std::array<uint32_t, kMaxRenderTargets> render_targets{};
    uint32_t depth_stencil{0};
    std::array<uint32_t, kMaxTextures> textures{};
    uint32_t vertex_declaration{0};
    uint32_t index_buffer{0};
    std::array<StreamBinding, kMaxStreams> streams{};
    std::array<SamplerBinding, kMaxSamplers> samplers{};
    std::array<uint32_t, kMaxFetchConstants> texture_fetch_ptrs{};
    uint32_t shader_gpr_alloc{0};

    struct {
        uint32_t x{0};
        uint32_t y{0};
        uint32_t width{0};
        uint32_t height{0};
    } viewport;
};

enum class DrawType : uint8_t {
    Indexed,
    IndexedShared,
    Primitive,
};

struct DrawCallCapture {
    uint32_t draw_id{0};       // Monotonic per-frame draw index
    DrawType type{DrawType::Indexed};
    uint32_t primitive_type{0};
    uint32_t start_index{0};   // Start index (indexed) or unused (primitive)
    uint32_t count{0};         // Index count or vertex count

    // Full binding snapshot (copied from ShadowState at draw time)
    std::array<uint32_t, kMaxRenderTargets> render_targets{};
    uint32_t depth_stencil{0};
    std::array<uint32_t, kMaxTextures> textures{};
    uint32_t vertex_declaration{0};
    uint32_t index_buffer{0};
    std::array<StreamBinding, kMaxStreams> streams{};
    std::array<SamplerBinding, kMaxSamplers> samplers{};
    std::array<uint32_t, kMaxFetchConstants> texture_fetch_ptrs{};
    uint32_t shader_gpr_alloc{0};

    struct {
        uint32_t x{0};
        uint32_t y{0};
        uint32_t width{0};
        uint32_t height{0};
    } viewport;
};

inline constexpr uint32_t kMaxDrawCapturesPerFrame = 4096;

struct FrameDrawLog {
    std::vector<DrawCallCapture> captures;
    uint32_t next_draw_id{0};

    void Reserve() { captures.reserve(kMaxDrawCapturesPerFrame); }

    void Reset() {
        captures.clear();
        next_draw_id = 0;
    }

    DrawCallCapture& Append(DrawType type, const ShadowState& state) {
        auto& c = captures.emplace_back();
        c.draw_id = next_draw_id++;
        c.type = type;

        c.render_targets = state.render_targets;
        c.depth_stencil = state.depth_stencil;
        c.textures = state.textures;
        c.vertex_declaration = state.vertex_declaration;
        c.index_buffer = state.index_buffer;
        c.streams = state.streams;
        c.samplers = state.samplers;
        c.texture_fetch_ptrs = state.texture_fetch_ptrs;
        c.shader_gpr_alloc = state.shader_gpr_alloc;
        c.viewport.x = state.viewport.x;
        c.viewport.y = state.viewport.y;
        c.viewport.width = state.viewport.width;
        c.viewport.height = state.viewport.height;

        return c;
    }
};

}  // namespace ac6::d3d

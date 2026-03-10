#include "d3d_hooks.h"

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ppc.h>

REXCVAR_DEFINE_BOOL(ac6_d3d_trace, false, "AC6/Render",
                    "Log every D3D device state change and draw call");

namespace {
const rex::LogCategoryId kLogGPU = rex::log::GPU;
}  // namespace

namespace {

ac6::d3d::ShadowState g_shadow{};
ac6::d3d::DrawStats g_live_stats{};
ac6::d3d::DrawStatsSnapshot g_snapshot{};

}  // namespace

PPC_EXTERN_FUNC(__imp__rex_sub_821DEF18);  // DrawIndexedVertices
PPC_EXTERN_FUNC(__imp__rex_sub_821DF300);  // DrawIndexedVertices_Shared
PPC_EXTERN_FUNC(__imp__rex_sub_821DD0A8);  // SetTexture
PPC_EXTERN_FUNC(__imp__rex_sub_821D95C8);  // SetRenderTarget
PPC_EXTERN_FUNC(__imp__rex_sub_821D9D38);  // SetDepthStencil
PPC_EXTERN_FUNC(__imp__rex_sub_821DE7D0);  // SetVertexDeclaration
PPC_EXTERN_FUNC(__imp__rex_sub_821DD1C8);  // SetIndexBuffer
PPC_EXTERN_FUNC(__imp__rex_sub_821DA698);  // SetViewport
PPC_EXTERN_FUNC(__imp__rex_sub_821E2BB8);  // Resolve

// D3DDevice_DrawIndexedVertices (0x821DEF18)
PPC_FUNC_IMPL(rex_sub_821DEF18) {
    PPC_FUNC_PROLOGUE();

    uint32_t index_count = ctx.r6.u32;

    g_live_stats.draw_calls.fetch_add(1, std::memory_order_relaxed);
    g_live_stats.draw_calls_indexed.fetch_add(1, std::memory_order_relaxed);
    g_live_stats.total_indices.fetch_add(index_count, std::memory_order_relaxed);

    if (REXCVAR_GET(ac6_d3d_trace)) {
        REXLOG_CAT_TRACE(kLogGPU,
            "DrawIndexedVertices: prim={} start={} count={}",
            ctx.r4.u32, ctx.r5.u32, index_count);
    }

    __imp__rex_sub_821DEF18(ctx, base);
}

// D3DDevice_DrawIndexedVertices_Shared (0x821DF300)
PPC_FUNC_IMPL(rex_sub_821DF300) {
    PPC_FUNC_PROLOGUE();

    uint32_t index_count = ctx.r7.u32;

    g_live_stats.draw_calls.fetch_add(1, std::memory_order_relaxed);
    g_live_stats.draw_calls_indexed_shared.fetch_add(1, std::memory_order_relaxed);
    g_live_stats.total_indices.fetch_add(index_count, std::memory_order_relaxed);

    if (REXCVAR_GET(ac6_d3d_trace)) {
        REXLOG_CAT_TRACE(kLogGPU,
            "DrawIndexedVertices_Shared: prim={} flags={} start={} count={}",
            ctx.r4.u32, ctx.r5.u32, ctx.r6.u32, index_count);
    }

    __imp__rex_sub_821DF300(ctx, base);
}

// D3DDevice_SetTexture (0x821DD0A8)
PPC_FUNC_IMPL(rex_sub_821DD0A8) {
    PPC_FUNC_PROLOGUE();

    uint32_t slot = ctx.r4.u32;
    uint32_t texture_ptr = ctx.r5.u32;

    if (slot < ac6::d3d::kMaxTextures) {
        g_shadow.textures[slot] = texture_ptr;
    }
    g_live_stats.set_texture_calls.fetch_add(1, std::memory_order_relaxed);

    if (REXCVAR_GET(ac6_d3d_trace)) {
        REXLOG_CAT_TRACE(kLogGPU,
            "SetTexture: slot={} texture=0x{:08X}",
            slot, texture_ptr);
    }

    __imp__rex_sub_821DD0A8(ctx, base);
}

// D3DDevice_SetRenderTarget (0x821D95C8)
PPC_FUNC_IMPL(rex_sub_821D95C8) {
    PPC_FUNC_PROLOGUE();

    uint32_t index = ctx.r4.u32;
    uint32_t surface = ctx.r5.u32;

    if (index < ac6::d3d::kMaxRenderTargets) {
        g_shadow.render_targets[index] = surface;
    }
    g_live_stats.set_render_target_calls.fetch_add(1, std::memory_order_relaxed);

    if (REXCVAR_GET(ac6_d3d_trace)) {
        REXLOG_CAT_TRACE(kLogGPU,
            "SetRenderTarget: index={} surface=0x{:08X}",
            index, surface);
    }

    __imp__rex_sub_821D95C8(ctx, base);
}

// D3DDevice_SetDepthStencil (0x821D9D38)
PPC_FUNC_IMPL(rex_sub_821D9D38) {
    PPC_FUNC_PROLOGUE();

    uint32_t surface = ctx.r4.u32;
    g_shadow.depth_stencil = surface;
    g_live_stats.set_depth_stencil_calls.fetch_add(1, std::memory_order_relaxed);

    if (REXCVAR_GET(ac6_d3d_trace)) {
        REXLOG_CAT_TRACE(kLogGPU,
            "SetDepthStencil: surface=0x{:08X}", surface);
    }

    __imp__rex_sub_821D9D38(ctx, base);
}

// D3DDevice_SetVertexDeclaration (0x821DE7D0)
PPC_FUNC_IMPL(rex_sub_821DE7D0) {
    PPC_FUNC_PROLOGUE();

    uint32_t decl = ctx.r4.u32;
    g_shadow.vertex_declaration = decl;
    g_live_stats.set_vertex_decl_calls.fetch_add(1, std::memory_order_relaxed);

    if (REXCVAR_GET(ac6_d3d_trace)) {
        REXLOG_CAT_TRACE(kLogGPU,
            "SetVertexDeclaration: decl=0x{:08X}", decl);
    }

    __imp__rex_sub_821DE7D0(ctx, base);
}

// D3DDevice_SetIndexBuffer (0x821DD1C8)
PPC_FUNC_IMPL(rex_sub_821DD1C8) {
    PPC_FUNC_PROLOGUE();

    uint32_t buffer = ctx.r4.u32;
    g_shadow.index_buffer = buffer;
    g_live_stats.set_index_buffer_calls.fetch_add(1, std::memory_order_relaxed);

    if (REXCVAR_GET(ac6_d3d_trace)) {
        REXLOG_CAT_TRACE(kLogGPU,
            "SetIndexBuffer: buffer=0x{:08X}", buffer);
    }

    __imp__rex_sub_821DD1C8(ctx, base);
}

// D3DDevice_SetViewport (0x821DA698)
PPC_FUNC_IMPL(rex_sub_821DA698) {
    PPC_FUNC_PROLOGUE();

    g_shadow.viewport.x      = ctx.r4.u32;
    g_shadow.viewport.y      = ctx.r5.u32;
    g_shadow.viewport.width  = ctx.r6.u32;
    g_shadow.viewport.height = ctx.r7.u32;
    g_live_stats.set_viewport_calls.fetch_add(1, std::memory_order_relaxed);

    if (REXCVAR_GET(ac6_d3d_trace)) {
        REXLOG_CAT_TRACE(kLogGPU,
            "SetViewport: {}x{} at ({},{})",
            g_shadow.viewport.width, g_shadow.viewport.height,
            g_shadow.viewport.x, g_shadow.viewport.y);
    }

    __imp__rex_sub_821DA698(ctx, base);
}

// D3DDevice_Resolve (0x821E2BB8)
PPC_FUNC_IMPL(rex_sub_821E2BB8) {
    PPC_FUNC_PROLOGUE();

    g_live_stats.resolve_calls.fetch_add(1, std::memory_order_relaxed);

    if (REXCVAR_GET(ac6_d3d_trace)) {
        REXLOG_CAT_TRACE(kLogGPU, "Resolve");
    }

    __imp__rex_sub_821E2BB8(ctx, base);
}

namespace ac6::d3d {

void OnFrameBoundary() {
    g_snapshot.draw_calls                = g_live_stats.draw_calls.load(std::memory_order_relaxed);
    g_snapshot.draw_calls_indexed        = g_live_stats.draw_calls_indexed.load(std::memory_order_relaxed);
    g_snapshot.draw_calls_indexed_shared = g_live_stats.draw_calls_indexed_shared.load(std::memory_order_relaxed);
    g_snapshot.total_indices             = g_live_stats.total_indices.load(std::memory_order_relaxed);
    g_snapshot.set_texture_calls         = g_live_stats.set_texture_calls.load(std::memory_order_relaxed);
    g_snapshot.set_render_target_calls   = g_live_stats.set_render_target_calls.load(std::memory_order_relaxed);
    g_snapshot.set_depth_stencil_calls   = g_live_stats.set_depth_stencil_calls.load(std::memory_order_relaxed);
    g_snapshot.set_vertex_decl_calls     = g_live_stats.set_vertex_decl_calls.load(std::memory_order_relaxed);
    g_snapshot.set_index_buffer_calls    = g_live_stats.set_index_buffer_calls.load(std::memory_order_relaxed);
    g_snapshot.set_stream_source_calls   = g_live_stats.set_stream_source_calls.load(std::memory_order_relaxed);
    g_snapshot.set_viewport_calls        = g_live_stats.set_viewport_calls.load(std::memory_order_relaxed);
    g_snapshot.clear_calls               = g_live_stats.clear_calls.load(std::memory_order_relaxed);
    g_snapshot.resolve_calls             = g_live_stats.resolve_calls.load(std::memory_order_relaxed);

    g_live_stats.Reset();
}

const DrawStatsSnapshot& GetDrawStats() {
    return g_snapshot;
}

const ShadowState& GetShadowState() {
    return g_shadow;
}

}  // namespace ac6::d3d

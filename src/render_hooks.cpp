#include "render_hooks.h"
#include "d3d_hooks.h"

#include <atomic>
#include <chrono>

#include <rex/cvar.h>
#include <rex/ppc/types.h>

REXCVAR_DEFINE_BOOL(ac6_unlock_fps, false, "AC6", "Unlock frame rate to 60fps");

using Clock = std::chrono::steady_clock;

namespace {

std::atomic<double> g_frame_time_ms{0.0};
std::atomic<double> g_fps{0.0};
std::atomic<uint64_t> g_frame_count{0};
Clock::time_point g_frame_start{};

}  // namespace

// Fallback flip interval bypass — rarely fires since vblank counter cycles 0→1→0.
bool ac6FlipIntervalHook() {
    return REXCVAR_GET(ac6_unlock_fps);
}

// Primary 60fps unlock — forces D3DPRESENT_INTERVAL to 1 (every VBlank).
bool ac6PresentIntervalHook(PPCRegister& r10) {
    if (REXCVAR_GET(ac6_unlock_fps)) {
        r10.u64 = 1;
        return true;
    }
    return false;
}

// Divisor=30 makes the delta time formula self-correct at any framerate.
void ac6DeltaDivisorHook(PPCRegister& r29) {
    r29.u64 = 30;
}

// Hooked before the device[21516] branch so it fires every frame, not just VdSwap frames.
void ac6PresentTimingHook() {
    ac6::d3d::OnFrameBoundary();

    auto now = Clock::now();
    if (g_frame_start.time_since_epoch().count() != 0) {
        double ms =
            std::chrono::duration<double, std::milli>(now - g_frame_start)
                .count();
        float fps_val = ms > 0.0 ? static_cast<float>(1000.0 / ms) : 0.0f;

        g_frame_time_ms.store(ms, std::memory_order_relaxed);
        g_fps.store(static_cast<double>(fps_val), std::memory_order_relaxed);
        g_frame_count.fetch_add(1, std::memory_order_relaxed);
    }
    g_frame_start = now;
}

namespace ac6 {

FrameStats GetFrameStats() {
    return FrameStats{
        g_frame_time_ms.load(std::memory_order_relaxed),
        g_fps.load(std::memory_order_relaxed),
        g_frame_count.load(std::memory_order_relaxed),
    };
}

}  // namespace ac6

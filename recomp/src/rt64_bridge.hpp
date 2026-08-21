#pragma once

#include <iostream>
#include <memory>
#include <cstdint>

#include "ultramodern/renderer_context.hpp"
#include "ultramodern/config.hpp"
#include "hle/rt64_application.h"

namespace N64 {

class RT64RendererContext : public ultramodern::renderer::RendererContext {
public:
    RT64RendererContext(uint8_t* rdram, ultramodern::renderer::WindowHandle window_handle, bool developer_mode)
        : rdramPtr(rdram)
    {
        std::cout << "[RT64 Bridge] Initializing RT64 DirectX 12 / Vulkan Renderer..." << std::endl;

        RT64::Application::Core core{};
#if defined(_WIN32)
        core.window.window = window_handle.window;
        core.window.threadId = window_handle.thread_id;
#else
        core.window = window_handle;
#endif
        core.RDRAM = rdram;
        core.HEADER = rdram;
        core.DMEM = nullptr;
        core.IMEM = nullptr;

        RT64::ApplicationConfiguration appConfig{};
        appConfig.appId = "ConkerRecompiled";
        appConfig.useConfigurationFile = true;

        rt64App = std::make_unique<RT64::Application>(core, appConfig);

        uint32_t threadId = 0;
#if defined(_WIN32)
        threadId = window_handle.thread_id != (DWORD)-1 ? window_handle.thread_id : GetCurrentThreadId();
#endif

        auto res = rt64App->setup(threadId);
        if (res == RT64::Application::SetupResult::Success) {
            setup_result = ultramodern::renderer::SetupResult::Success;
            chosen_api = ultramodern::renderer::GraphicsApi::D3D12;
            std::cout << "[RT64 Bridge] RT64 Direct3D 12 Engine successfully bound to Window HWND!" << std::endl;
        } else {
            setup_result = ultramodern::renderer::SetupResult::GraphicsAPINotFound;
            std::cerr << "[RT64 Bridge] Warning: RT64 setup reported code: " << static_cast<int>(res) << std::endl;
        }
    }

    ~RT64RendererContext() override {
        if (rt64App) {
            rt64App->end();
        }
    }

    bool valid() override {
        return setup_result == ultramodern::renderer::SetupResult::Success;
    }

    bool update_config(const ultramodern::renderer::GraphicsConfig& old_config, const ultramodern::renderer::GraphicsConfig& new_config) override {
        (void)old_config;
        (void)new_config;
        if (rt64App) {
            rt64App->updateUserConfig(false);
            return true;
        }
        return false;
    }

    void enable_instant_present() override {}

    void send_dl(const OSTask* task) override {
        if (!rt64App || !task) return;
        uint32_t dlStart = static_cast<uint32_t>(task->t.data_ptr);
        uint32_t dlEnd   = dlStart + static_cast<uint32_t>(task->t.data_size);
        rt64App->processDisplayLists(rdramPtr, dlStart, dlEnd, true);
    }

    void send_dummy_workload(uint32_t fb_address) override {
        (void)fb_address;
    }

    void update_screen() override {
        if (rt64App) {
            rt64App->updateScreen();
        }
    }

    void shutdown() override {
        if (rt64App) {
            rt64App->end();
        }
    }

    uint32_t get_display_framerate() const override {
        return 60;
    }

    float get_resolution_scale() const override {
        return 1.0f;
    }

private:
    uint8_t* rdramPtr = nullptr;
    std::unique_ptr<RT64::Application> rt64App;
};

// Función factoría para registrar en ultramodern
inline std::unique_ptr<ultramodern::renderer::RendererContext> createRT64Context(uint8_t* rdram, ultramodern::renderer::WindowHandle window_handle, bool developer_mode) {
    return std::make_unique<RT64RendererContext>(rdram, window_handle, developer_mode);
}

inline void registerRT64Renderer() {
    ultramodern::renderer::callbacks_t cbs{};
    cbs.create_render_context = createRT64Context;
    ultramodern::renderer::set_callbacks(cbs);
    std::cout << "[RT64 Bridge] Registered RT64 DirectX 12 / Vulkan factory callbacks with ultramodern!" << std::endl;
}

} // namespace N64

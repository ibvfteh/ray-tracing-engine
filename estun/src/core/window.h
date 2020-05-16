#pragma once

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.h>
#include <functional>
#include <vector>
#include <GLFW/glfw3.h>

namespace estun
{

struct WindowConfig
{
    std::string title;
    uint32_t width;
    uint32_t height;
    std::string icon;
    bool cursorDisabled;
    bool fullscreen;
    bool resizable;
};

class Window
{
public:
    Window(const Window &) = delete;
    Window(Window &&) = delete;
    
    Window &operator=(const Window &) = delete;
    Window &operator=(Window &&) = delete;

    explicit Window(const WindowConfig &config);
    ~Window();

    const WindowConfig &GetConfig() const { return config_; }
    GLFWwindow *GetWindow() const { return window_; }

    std::vector<const char *> GetRequiredInstanceExtensions() const;
    float ContentScale() const;
    double Time() const;
    
    VkExtent2D FramebufferSize() const;
    VkExtent2D WindowSize() const;

    std::function<void()> DrawFrame;

    std::function<void(int key, int scancode, int action, int mods)> OnKey;
    std::function<void(double xpos, double ypos)> OnCursorPosition;
    std::function<void(double xoffset, double yoffset)> OnScroll;
    std::function<void(int button, int action, int mods)> OnMouseButton;
    std::function<void(int width, int height)> OnResize;

    void Close() const;
    bool IsMinimized() const;
    void Run() const;
    void WaitForEvents() const;
    void ToggleCursor(bool cursor) const;

private:
    const WindowConfig config_;
    GLFWwindow *window_{};
};

} // namespace estun

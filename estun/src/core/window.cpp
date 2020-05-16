#include "core/window.h"
#include "core/core.h"
#include <stb_image.h>
#include <iostream>
#include <string>

namespace
{
	void GlfwErrorCallback(const int error, const char *const description)
	{
		ES_CORE_ASSERT(std::string("ERROR: GLFW: ") + std::string(description) + " (code: " + std::to_string(error) + ")");
	}

	void GlfwKeyCallback(GLFWwindow *window, const int key, const int scancode, const int action, const int mods)
	{
		const auto this_ = reinterpret_cast<estun::Window *>(glfwGetWindowUserPointer(window));
		if (this_->OnKey)
		{
			this_->OnKey(key, scancode, action, mods);
		}
	}

	void GlfwCursorPositionCallback(GLFWwindow *window, const double xpos, const double ypos)
	{
		const auto this_ = reinterpret_cast<estun::Window *>(glfwGetWindowUserPointer(window));
		if (this_->OnCursorPosition)
		{
			this_->OnCursorPosition(xpos, ypos);
		}
	}

	void GlfwScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
	{
		const auto this_ = reinterpret_cast<estun::Window *>(glfwGetWindowUserPointer(window));
		if (this_->OnScroll)
		{
			this_->OnScroll(xoffset, yoffset);
		}
	}

	void GlfwMouseButtonCallback(GLFWwindow *window, const int button, const int action, const int mods)
	{
		const auto this_ = reinterpret_cast<estun::Window *>(glfwGetWindowUserPointer(window));
		if (this_->OnMouseButton)
		{
			this_->OnMouseButton(button, action, mods);
		}
	}

	void GlfwFramebufferResizeCallback(GLFWwindow *window, int width, int height)
	{
		const auto this_ = reinterpret_cast<estun::Window *>(glfwGetWindowUserPointer(window));
		if (this_->OnResize)
		{
			this_->OnResize(width, height);
		}
	}
} // namespace

estun::Window::Window(const WindowConfig &config) : config_(config)
{
	glfwSetErrorCallback(GlfwErrorCallback);

	if (!glfwInit())
	{
		ES_CORE_ASSERT("glfwInit() failed");
	}

	if (!glfwVulkanSupported())
	{
		ES_CORE_ASSERT("glfwVulkanSupported() failed");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);

	const auto monitor = config.fullscreen ? glfwGetPrimaryMonitor() : nullptr;

	window_ = glfwCreateWindow(config.width, config.height, config.title.c_str(), monitor, nullptr);
	if (window_ == nullptr)
	{
		ES_CORE_ASSERT("Failed to create window");
	}

	GLFWimage icon;
	icon.pixels = stbi_load(config_.icon.c_str(), &icon.width, &icon.height, nullptr, 4);
	if (icon.pixels == nullptr)
	{
		ES_CORE_ASSERT("Failed to load icon");
	}

	glfwSetWindowIcon(window_, 1, &icon);
	stbi_image_free(icon.pixels);

	if (config.cursorDisabled)
	{
		glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	glfwSetWindowUserPointer(window_, this);
	glfwSetKeyCallback(window_, GlfwKeyCallback);
	glfwSetCursorPosCallback(window_, GlfwCursorPositionCallback);
	glfwSetMouseButtonCallback(window_, GlfwMouseButtonCallback);
	glfwSetMouseButtonCallback(window_, GlfwMouseButtonCallback);
	glfwSetMouseButtonCallback(window_, GlfwMouseButtonCallback);
}

estun::Window::~Window()
{
	if (window_ != nullptr)
	{
		glfwDestroyWindow(window_);
		window_ = nullptr;
	}

	glfwTerminate();
	glfwSetErrorCallback(nullptr);
}

std::vector<const char *> estun::Window::GetRequiredInstanceExtensions() const
{
	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	return std::vector<const char *>(glfwExtensions, glfwExtensions + glfwExtensionCount);
}

float estun::Window::ContentScale() const
{
	float xscale;
	float yscale;
	glfwGetWindowContentScale(window_, &xscale, &yscale);

	return xscale;
}

double estun::Window::Time() const
{
	return glfwGetTime();
}

VkExtent2D estun::Window::FramebufferSize() const
{
	int width, height;
	glfwGetFramebufferSize(window_, &width, &height);
	return VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

VkExtent2D estun::Window::WindowSize() const
{
	int width, height;
	glfwGetWindowSize(window_, &width, &height);
	return VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

void estun::Window::Close() const
{
	glfwSetWindowShouldClose(window_, 1);
}

bool estun::Window::IsMinimized() const
{
	const auto size = FramebufferSize();
	return size.height == 0 && size.width == 0;
}

void estun::Window::Run() const
{
	glfwSetTime(0.0);

	while (!glfwWindowShouldClose(window_))
	{
		glfwPollEvents();

		if (DrawFrame)
		{
			DrawFrame();
		}
	}
}

void estun::Window::WaitForEvents() const
{
	glfwWaitEvents();
}

void estun::Window::ToggleCursor(bool cursor) const
{
	if (cursor)
		glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	else
		glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}
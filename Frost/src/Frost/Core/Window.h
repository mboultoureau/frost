#pragma once

namespace Frost
{
	class Window
	{
	public:
		using WindowSizeUnit = unsigned int;
		using WindowTitle = const wchar_t*;

		Window(const WindowTitle title) {}
		virtual ~Window() = default;

		WindowSizeUnit GetWidth() const { return _width; }
		WindowSizeUnit GetHeight() const { return _height; }
		bool IsFullscreen() const { return _isFullscreen; }
		virtual void SetFullscreen(const bool isFullscreen) { _isFullscreen = isFullscreen; }

		virtual void MainLoop() = 0;

	protected:
		WindowSizeUnit _width = 640;
		WindowSizeUnit _height = 480;
		bool _isFullscreen = false;
	};
}
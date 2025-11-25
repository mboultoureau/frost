#pragma once

namespace Editor
{
	class EditorPanel
	{
	public:
		virtual void Draw() = 0;
		
	protected:
		bool _isOpen = true;
	};
}
#pragma once

#include <vector>

struct WindowInputStatus
{
	enum Key
	{
		kLeft,
		kRight,
		kUp,
		kDown,
		kCount
	};
	enum MouseButton
	{
		mbLeft,
		mbMiddle,
		mbRight,
		mbCount
	};

	bool keys[Key::kCount];
	bool mouseButtons[MouseButton::mbCount];
	int  mouseX;
	int  mouseY;

	enum EventType
	{
		etKeyUp,
		etKeyDown,
		etMouseButtonUp,
		etMouseButtonDown,
		etMouseButtonDoubleClick,
		etCount
	};
	struct Event
	{
		EventType type;
		Key key;
		MouseButton mouseButton;
		int  mouseX;
		int  mouseY;
	};

	void init()
	{
		memset(this, 0, sizeof(WindowInputStatus));
	}

};

typedef std::vector<WindowInputStatus::Event> WindowInputEventList;

struct WindowInputData
{
	WindowInputStatus mInputStatus;		/// 
	WindowInputEventList mInputEvents;	/// 

	void init()
	{
		mInputStatus.init();
		mInputEvents.clear();
		mInputEvents.reserve(16);
	}
};


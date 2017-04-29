#pragma once

#include <vector>

enum InputKey
{
	kLeft,
	kRight,
	kUp,
	kDown,
	kCount
};
enum InputMouseButton
{
	mbLeft,
	mbMiddle,
	mbRight,
	mbCount
};
enum InputEventType
{
	etKeyUp,
	etKeyDown,
	etMouseMoved,
	etMouseButtonUp,
	etMouseButtonDown,
	etMouseButtonDoubleClick,
	etCount
};

struct InputEvent
{
	InputEventType type;
	union 
	{
		InputKey key;
		InputMouseButton mouseButton;
	};
	int  mouseX;
	int  mouseY;
};

struct WindowInputStatus
{

	bool keys[kCount];
	bool mouseButtons[mbCount];
	int  mouseX;
	int  mouseY;

	void init()
	{
		memset(this, 0, sizeof(WindowInputStatus));
	}

};

typedef std::vector<InputEvent> WindowInputEventList;

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


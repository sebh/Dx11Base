#pragma once

#include "Dx11Base/WindowInput.h"

class Game
{
public:
	Game();
	~Game();

	void initialise();
	void shutdown();

	void update(const WindowInputData& inputData);
	void render();

private:

	/// Load/reload all shaders if compilation is succesful.
	/// @exitIfFail: calls exit(0) if any of the reload/compilation failed.
	void loadShaders(bool exitIfFail);
	/// release all shaders
	void releaseShaders();
};



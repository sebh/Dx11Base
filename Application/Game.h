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
	// TODO
};



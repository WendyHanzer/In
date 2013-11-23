#include "engine.h"

// program start
int main(int argc, char **argv) {
	// initialize game engine
	Engine::init(argc, argv);
	// run game
	return Engine::run();
}

#include <SFML/Graphics.hpp>
#include "Game.hpp"

int main(int argc, char* argv[])
{
	Game g("config.txt");
	g.run();
}
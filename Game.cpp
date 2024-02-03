#include "Game.hpp"
#include <iostream>
#include <fstream>

Game::Game(const std::string& config)
{
	init(config);
}

void Game::init(const std::string& path)
{
	// Reads in config file
	std::ifstream  fin(path);
	std::string type, fontAdd;

	while (fin >> type)
	{
		if (type == "Window")
		{
			fin >> m_windowConfig.W >> m_windowConfig.H >> m_windowConfig.FL >> m_windowConfig.FS;
		}
		else if (type == "Font")
		{
			fin >> fontAdd >> m_fontConfig.S >> m_fontConfig.R >> m_fontConfig.G >> m_fontConfig.B;

			// attempt to load the font from a file
			if (!m_font.loadFromFile(fontAdd))
			{
				// if we can't load the font, print an error to the error console and exit
				std::cerr << "Could not load font\n";
				exit(-1);
			}

			m_text.setCharacterSize(m_fontConfig.S);
			m_text.setFillColor(sf::Color(m_fontConfig.R, m_fontConfig.G, m_fontConfig.B));
			m_text.setFont(m_font);
		}
		else if (type == "Player")
		{
			fin >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB;
			fin >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V;
		}
		else if (type == "Enemy")
		{
			fin >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX >> m_enemyConfig.OR >> m_enemyConfig.OG;
			fin >> m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
		}
		else if (type == "Bullet")
		{
			fin >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB;
			fin >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
		}
		else
		{
			std::cerr << "File path '" << path << "' Object type '" << type << "' is unidentified!\n";
			exit(-2);
		}
	}

	
	// set up window parameters
	m_window.create(sf::VideoMode(m_windowConfig.W, m_windowConfig.H), "Shape Wars");
	m_window.setFramerateLimit(m_windowConfig.FL);

	spawnPlayer();
}

void Game::run()
{
	// TODO: add pause functionality in here
	//		 some systems should function while paused (rendering)
	//		 some system shouldn't (movement / input)
	while (m_running)
	{
		m_entityManager.update();

		if (!m_paused)
		{
			sEnemySpawner();	// if not paused these system should work
			sMovement();
			sCollision();
			sLifespan();
		}

		sUserInput();			// only get input of pause key
		sRender();				// even if paused this should still render the game

		// increment the current frame
		// may need to be moved when pause implemented
		m_currentFrame++;
	}
}

void Game::setPaused(bool paused)
{
	m_paused = paused;
}

// respawn the player in the middle of the screen
void Game::spawnPlayer()
{
	// We create every entity by calling EntityManager.addEntity(tag)
	// This returns a std::shared_ptr<Entity>, so we use "auto" to save typing
	auto entity = m_entityManager.addEntity("player");

	// Give this entity a Transform so it spawns at (x, y) with velocity (0, 0) and angle 0
	float mx = m_window.getSize().x / 2.0f;
	float my = m_window.getSize().y / 2.0f;

	entity->cTransform = std::make_shared<CTransform>(Vec2(mx, my), Vec2(0.0f, 0.0f), 0.0f);

	// The entity's shape will have radius , sides, fill, outline and thickness
	entity->cShape = std::make_shared<CShape>(m_playerConfig.SR, m_playerConfig.V, sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB),
		sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), m_playerConfig.OT);

	// Add an input component to the player so that we can use inputs
	entity->cInput = std::make_shared<CInput>();

	// Add a collision component to the player
	entity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);

	// Since we want this Entity to be our player, set our Game's player variable to be this Entity
	// This goes slighty against the EntityManager paradigm, but we use the player so much it's worth it
	m_player = entity;
}

// spawn an enemy at a random position
void Game::spawnEnemy()
{
	// TODO: make sure the enemy is spawned properly with the m_enemyConfig variables
	//		 the enemy must be spawned completely within the bounds of the window
	//

	auto entity = m_entityManager.addEntity("enemy");

	// Give this entity a Transform so it spawns at (ex, ey) with velocity and angle 
	float ex = m_enemyConfig.SR + (rand() % (1 + m_window.getSize().x - m_enemyConfig.SR));
	float ey = m_enemyConfig.SR + (rand() % (1 + m_window.getSize().y - m_enemyConfig.SR));

	// Give this entity  x and y velocity between SMIN and SMAX
	float velX = (rand() % (1 + ((int)m_enemyConfig.SMAX * 2))) - m_enemyConfig.SMAX;
	if (velX > (-m_enemyConfig.SMIN) && velX < 0)
	{
		velX -= m_enemyConfig.SMIN - 1 + (rand() % ((int)m_enemyConfig.SMAX - (int)m_enemyConfig.SMIN));
	}
	else if (velX < m_enemyConfig.SMIN && velX >= 0)
	{
		velX -= m_enemyConfig.SMIN + (rand() % ((int)m_enemyConfig.SMAX - (int)m_enemyConfig.SMIN - 1));
	}
	float velY = (rand() % (1 + ((int)m_enemyConfig.SMAX * 2))) - m_enemyConfig.SMAX;
	if (velY > (-m_enemyConfig.SMIN) && velY < 0)
	{
		velY -= m_enemyConfig.SMIN - 1 + (rand() % ((int)m_enemyConfig.SMAX - (int)m_enemyConfig.SMIN));
	}
	else if (velY < m_enemyConfig.SMIN && velY >= 0)
	{
		velY -= m_enemyConfig.SMIN + (rand() % ((int)m_enemyConfig.SMAX - (int)m_enemyConfig.SMIN - 1));
	}

	entity->cTransform = std::make_shared<CTransform>(Vec2(ex, ey), Vec2(velX, velY));

	//Give this entity a random vertices from VMIN to VMAX
	int vertice = m_enemyConfig.VMIN + (rand() % (1 + m_enemyConfig.VMAX - m_enemyConfig.VMIN));

	// The entity's shape will have radius, sides, fill, outline and thickness 
	float r = rand() % 255, g = rand() % 255, b = rand() % 255;
	entity->cShape = std::make_shared<CShape>(m_enemyConfig.SR, vertice, sf::Color(10, 10, 10), 
		sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB), m_enemyConfig.OT);

	// Add a collision component to the enemy
	entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);

	// Add a score component to the enemy
	entity->cScore = std::make_shared<CScore>(vertice*100);

	// record when the most recent enemy was spawned
	m_lastEnemySpawnTime = m_currentFrame;
}

// spawns the small enemies when a big one (input entity e) explodes
void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
	// TODO: spawn small enemies at the location of the input enemy e
	// when we create the smaller enemy, we have to read the values of the original enemy
	// - spawn a number of small enemies equal to the vertices of the original enemy
	// - set each small enemy to the same color as the original, half the size
	// - small enemies are worth double points of the original enemy

	float magnitude = e->cTransform->velocity.length();
	float angle = 0.0f;

	for (int i = 0, vertice = e->cShape->circle.getPointCount(); i < vertice; i++)
	{
		auto entity = m_entityManager.addEntity("small-enemy");
		float velX = magnitude * cos(angle);
		float velY = magnitude * sin(angle);
		entity->cTransform = std::make_shared<CTransform>(Vec2(e->cTransform->pos.x, e->cTransform->pos.y), Vec2(velX / 2, velY / 2), angle);
		entity->cShape = std::make_shared<CShape>((e->cShape->circle.getRadius() / 2), vertice, sf::Color(e->cShape->circle.getFillColor()),
			sf::Color(e->cShape->circle.getOutlineColor()), e->cShape->circle.getOutlineThickness());
		entity->cCollision = std::make_shared<CCollision>((e->cCollision->radius / 2));
		entity->cScore = std::make_shared<CScore>(e->cScore->score * 2);
		entity->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);
		angle += (360.0 / (vertice));
	}
}

// spawns a bullet from a given entity to a target location
void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2& target)
{
	// TODO: implement the spawning of a bullet which travels toward target
	//		 - bullet speed is given as a scalar speed
	//		 - you must set the velocity by using formula in notes

	auto bullet = m_entityManager.addEntity("bullet");
	Vec2 dist = entity->cTransform->pos.dist(target);
	dist /= (dist.length());
	dist *= (m_bulletConfig.S);

	bullet->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, dist);
	bullet->cShape = std::make_shared<CShape>(m_bulletConfig.SR, m_bulletConfig.V, sf::Color(m_bulletConfig.FR, m_bulletConfig.FG,
		m_bulletConfig.FB), sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB), m_bulletConfig.OT);
	bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);
	bullet->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);

}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity, const Vec2& target)
{
	// TODO: implement your own special weapon
	if (m_currentFrame - m_lastSpecialTime >= m_bulletConfig.L * 5)
	{
		auto special = m_entityManager.addEntity("special");
		Vec2 dist = entity->cTransform->pos.dist(target);
		dist /= (dist.length());
		dist *= 2;

		special->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, dist);
		special->cShape = std::make_shared<CShape>(m_playerConfig.SR, m_playerConfig.V, sf::Color(200, 0,
			0), sf::Color(255, 0, 0), m_bulletConfig.OT * 2);
		special->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);
		special->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L * 5);
		m_lastSpecialTime = m_currentFrame;
	}
}

void Game::sMovement()
{
	// TODO: implement all entity movement in this function
	//		 you should read the m_player->cInput component to determine if the player is moving
	//		 implement player movement
	if (m_player->cTransform->pos.y < m_playerConfig.SR)
	{
		m_player->cTransform->pos.y = m_playerConfig.SR + m_playerConfig.S;
	}
	else if (m_player->cTransform->pos.y > m_window.getSize().y - m_playerConfig.SR)
	{
		m_player->cTransform->pos.y = m_window.getSize().y - m_playerConfig.SR - m_playerConfig.S;
	}
	else
	{
		if (m_player->cInput->up)
		{
			m_player->cTransform->velocity.y = -m_playerConfig.S;
		}
		else if (m_player->cInput->down)
		{
			m_player->cTransform->velocity.y = m_playerConfig.S;
		}
		else { m_player->cTransform->velocity.y = 0.0f; }
	}

	if (m_player->cTransform->pos.x < m_playerConfig.SR)
	{
		m_player->cTransform->pos.x = m_playerConfig.SR + m_playerConfig.S;
	}
	else if (m_player->cTransform->pos.x > m_window.getSize().x - m_playerConfig.SR)
	{
		m_player->cTransform->pos.x = m_window.getSize().x - m_playerConfig.SR - m_playerConfig.S;
	}
	else
	{
		if (m_player->cInput->left)
		{
			m_player->cTransform->velocity.x = -m_playerConfig.S;
		}
		else if (m_player->cInput->right)
		{
			m_player->cTransform->velocity.x = m_playerConfig.S;
		}
		else { m_player->cTransform->velocity.x = 0.0f; }
	}

	// bounce enemies from window

	for (auto& e : m_entityManager.getEntities("enemy"))
	{
		if (e->cTransform->pos.x < m_enemyConfig.SR || e->cTransform->pos.x > m_windowConfig.W - m_enemyConfig.SR)
		{
			e->cTransform->velocity.bounceX();
		}
		if (e->cTransform->pos.y < m_enemyConfig.SR || e->cTransform->pos.y > m_windowConfig.H - m_enemyConfig.SR)
		{
			e->cTransform->velocity.bounceY();
		}
	}

	// movement update for entities
	for (auto& e : m_entityManager.getEntities())
	{
		e->cTransform->pos.x += e->cTransform->velocity.x;
		e->cTransform->pos.y += e->cTransform->velocity.y;
	}
}

void Game::sLifespan()
{

	// TODO: implement all lifespan functionality
	//
	// for all entities
	//	   if entity has no lifespan component, skip it
	//	   if entity has > 0 remaining lifespan, subtract 1
	//	   if it has lifespan and is alive
	//			scale its alpha channel properly
	//	   if it has lifespawn and its time is up
	//			destroy the entity

	for (auto& e : m_entityManager.getEntities())
	{
		if (!e->cLifespan)
		{
			continue;
		}

		if (e->cLifespan->remaining > 0)
		{
			e->cLifespan->remaining--;

			float alpha = ((float)e->cLifespan->remaining / (float)e->cLifespan->total) * 255;

			e->cShape->circle.setFillColor(sf::Color(e->cShape->circle.getFillColor().r, e->cShape->circle.getFillColor().g, 
				e->cShape->circle.getFillColor().b, alpha));
			e->cShape->circle.setOutlineColor(sf::Color(e->cShape->circle.getOutlineColor().r, e->cShape->circle.getOutlineColor().g,
				e->cShape->circle.getOutlineColor().b, alpha));
		}
		else
		{
			e->destroy();
		}
	}
}

void Game::sCollision()
{
	// TODO: implement all proper collisions between entities
	//		 be sure to use collision radius, NOT the shape radius

	for (auto& e : m_entityManager.getEntities("enemy"))
	{
		for (auto& b : m_entityManager.getEntities("bullet"))
		{
			if (e->cTransform->pos.dist(b->cTransform->pos).length() < (m_bulletConfig.CR + m_enemyConfig.CR))
			{
				m_score += e->cScore->score;
				spawnSmallEnemies(e);
				e->destroy();
				b->destroy();
			}
		}
		
		if (m_player->cTransform->pos.dist(e->cTransform->pos).length() < (m_playerConfig.CR + m_enemyConfig.CR))
		{
			e->destroy();
			m_player->cTransform->pos.x = m_window.getSize().x / 2.0f;
			m_player->cTransform->pos.y = m_window.getSize().y / 2.0f;
		}

		for (auto& s : m_entityManager.getEntities("special"))
		{
			if (e->cTransform->pos.dist(s->cTransform->pos).length() < (m_bulletConfig.CR + m_enemyConfig.CR))
			{
				m_score += e->cScore->score;
				spawnSmallEnemies(e);
				e->destroy();
			}
			else if (e->cTransform->pos.dist(s->cTransform->pos).length() < (m_bulletConfig.CR + m_enemyConfig.CR) * 5)
			{
				Vec2 dist = e->cTransform->pos.dist(s->cTransform->pos);
				e->cTransform->velocity = (dist / 50.0);
				if (s->cLifespan->remaining == 0)
				{
					m_score += e->cScore->score;
					spawnSmallEnemies(e);
					e->destroy();
				}
			}

		}
	}

	for (auto& b : m_entityManager.getEntities("bullet"))
	{
		for (auto& e : m_entityManager.getEntities("small-enemy"))
		{
			if (b->cTransform->pos.dist(e->cTransform->pos).length() < (m_bulletConfig.CR + m_enemyConfig.CR))
			{
				m_score += e->cScore->score;
				e->destroy();
				b->destroy();
			}
		}
	}
}

void Game::sEnemySpawner()
{
	// TODO: code which implements enemy spawning should go here
	//
	//		 (use m_currentFrame - m_lastEnemySpawnTime) to determine
	//		 how long it has been since last enemy spawned
	if (m_currentFrame - m_lastEnemySpawnTime >= m_enemyConfig.SI)
	{
		spawnEnemy();
	}
}

void Game::sRender()
{
	// TODO: change the code below to draw ALL of the entities
	//		 sample drawing of the player Entity that we have created
	m_window.clear();

	// set the position of the shape based on the entity's transform->pos
	m_player->cShape->circle.setPosition(m_player->cTransform->pos.x, m_player->cTransform->pos.y);

	// set the rotarion of the shape based on the entity's transform->angle
	m_player->cTransform->angle += 1.0f;
	m_player->cShape->circle.setRotation(m_player->cTransform->angle);

	//draw the entity's sf::CircleShape
	m_window.draw(m_player->cShape->circle);

	for (auto& e : m_entityManager.getEntities())
	{
		// set the position of the shape based on the entity's transform->pos
		e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);

		// set the rotarion of the shape based on the entity's transform->angle
		e->cTransform->angle += 1.0f;
		e->cShape->circle.setRotation(e->cTransform->angle);

		//draw the entity's sf::CircleShape
		m_window.draw(e->cShape->circle);
	}

	m_text.setString("Score : " + std::to_string(m_score));
	m_window.draw(m_text);

	m_window.display();
}

void Game::sUserInput()
{
	// TODO: handle user input here
	//		 note that you should only be setting player's input component variables here
	//		 you should not implement the player's movement logic here
	//		 the movement system will read the variables you set in this function

	sf::Event event;
	while (m_window.pollEvent(event))
	{
		// this event triggers when window is closed
		if (event.type == sf::Event::Closed)
		{
			m_running = false;
		}

		// this event is triggered when a key is pressed
		if (event.type == sf::Event::KeyPressed)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::W:
				// set player's input component "up" to true
				m_player->cInput->up = true;
				break;
			case sf::Keyboard::A:
				// set player's input component "left" to true
				m_player->cInput->left = true;
				break;
			case sf::Keyboard::S:
				// set player's input component "down" to true
				m_player->cInput->down = true;
				break;
			case sf::Keyboard::D:
				// set player's input component "right" to true
				m_player->cInput->right = true;
				break;
			case sf::Keyboard::Escape:
				// Pause the game
				if (!m_paused)
				{
					setPaused(true);
					std::cout << "Game is paused\n";
				}
				else
				{
					setPaused(false);
					std::cout << "Game is unpaused\n";
				}
				break;
			default: break;
			}
		}

		// this event is triggered when a key is released
		if (event.type == sf::Event::KeyReleased)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::W:
				// TODO: set player's input component "up" to false
				m_player->cInput->up = false;
				break;
			case sf::Keyboard::A:
				// set player's input component "left" to false
				m_player->cInput->left = false;
				break;
			case sf::Keyboard::S:
				// set player's input component "down" to false
				m_player->cInput->down = false;
				break;
			case sf::Keyboard::D:
				// set player's input component "right" to false
				m_player->cInput->right = false;
				break;
			default: break;
			}
		}

		if (event.type == sf::Event::MouseButtonPressed && !m_paused)
		{
			if (event.mouseButton.button == sf::Mouse::Left)
			{
				spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
			}

			if (event.mouseButton.button == sf::Mouse::Right)
			{
				spawnSpecialWeapon(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
			}
		}
	}
}
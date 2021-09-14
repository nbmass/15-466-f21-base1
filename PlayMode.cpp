#include "PlayMode.hpp"
#include "load_save_png.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

PlayMode::PlayMode() {
	//TODO:
	// you *must* use an asset pipeline of some sort to generate tiles.
	// don't hardcode them like this!
	// or, at least, if you do hardcode them like this,
	//  make yourself a script that spits out the code that you paste in here
	//   and check that script into your repository.

	//Also, *don't* use these tiles in your game:


	//load images
	auto load_image = [&](std::string file_name, int tile_index, int pallette_index) {
		// std::string file_name = "dist/test.png";
		glm::uvec2 size = glm::uvec2(8, 8);
		std::vector<glm::u8vec4> data;
		OriginLocation origin = OriginLocation::LowerLeftOrigin;
		load_png(file_name, &size, &data, origin);

		//set pallette
		for (int color = 0; color < 4; color++) {
			ppu.palette_table[pallette_index][color] = glm::uvec4(0, 0, 0, 0);
		}
		int current_color = 0;
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				bool found = false;
				for (int color = 0; color < 3; color++) {
					if (data[8*i+j] == ppu.palette_table[pallette_index][color]) {
						found = true;
					}
				}
				if (!found and current_color < 3) {
					ppu.palette_table[pallette_index][current_color] = data[8*i+j];
					current_color++;
				}
			}
		}
		
		//set tile
		for (int i = 0; i < 8; i++) {
			ppu.tile_table[tile_index].bit0[i] = 0;
			ppu.tile_table[tile_index].bit1[i] = 0;
			for (int j = 0; j < 8; j++) {
				for (int color = 0; color < 3; color++) {
					if (data[8*i+j] == ppu.palette_table[pallette_index][color]) {
						ppu.tile_table[tile_index].bit0[i] |= (color % 2) << j;
						ppu.tile_table[tile_index].bit1[i] |= (color / 2) << j;
					}
				}
			}
		}
	};

	//load player
	load_image("dist/player_up.png", 32, 7);
	load_image("dist/player_down.png", 33, 7);
	load_image("dist/player_left.png", 34, 7);
	load_image("dist/player_right.png", 35, 7);
	load_image("dist/player_upleft.png", 36, 7);
	load_image("dist/player_upright.png", 37, 7);
	load_image("dist/player_downleft.png", 38, 7);
	load_image("dist/player_downright.png", 39, 7);

	//load enemies
	load_image("dist/enemy.png", 31, 6);

	//load projectile images
	load_image("dist/laser11.png", 16, 2);
	load_image("dist/laser1-1.png", 17, 2);
	load_image("dist/laser01.png", 18, 2);
	load_image("dist/laser10.png", 19, 2);

	//background tiles
	load_image("dist/background_empty.png", 0, 0);
	load_image("dist/background_smallstar.png", 1, 0);
	load_image("dist/background_bigstar.png", 2, 0);
	load_image("dist/background_moon.png", 4, 0);
	load_image("dist/background_xstar.png", 3, 0);

	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			if ((std::rand() % 8) == 0) {
				ppu.background[x+PPU466::BackgroundWidth*y] = (std::rand() % 4);
			} else {
				ppu.background[x+PPU466::BackgroundWidth*y] = 0;
			}
		}
	}
	//make moon
	ppu.background[PPU466::BackgroundWidth * 19 + 24] = 4;

	for (int i = 1; i < 32; i++) {
			enemies[i].pos.x = std::rand() % 256;
			enemies[i].pos.y = std::rand() % 240;
			enemies[i].dir = directions[std::rand() % 8];
	}
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.downs += 1;
			space.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	// (will be used to set background color)
	background_fade += elapsed / 10.0f;
	background_fade -= std::floor(background_fade);

	constexpr float PlayerSpeed = 30.0f;
	if (left.pressed) player_at.x -= PlayerSpeed * elapsed;
	if (right.pressed) player_at.x += PlayerSpeed * elapsed;
	if (down.pressed) player_at.y -= PlayerSpeed * elapsed;
	if (up.pressed) player_at.y += PlayerSpeed * elapsed;

	if (player_at.x < 0) player_at.x += 256;
	if (player_at.x > 256) player_at.x -= 256;
	if (player_at.y < 0) player_at.y += 256;
	if (player_at.y > 256) player_at.y -= 256;

	//player direction
	if (left.pressed or right.pressed or down.pressed or up.pressed) {
		player_direction = glm::vec2(0, 0);
		if (left.pressed) player_direction.x = -1;
		if (right.pressed) player_direction.x = 1;
		if (down.pressed) player_direction.y = -1;
		if (up.pressed) player_direction.y = 1;
	}

	for (int i = 1; i < 32; i++) {
		Character &e = enemies[i];
		if (e.is_alive) {
			e.pos += elapsed * PlayerSpeed * e.dir;
			if (e.pos.x < 0) e.pos.x += 256;
			if (e.pos.x > 256) e.pos.x -= 256;
			if (e.pos.y < 0) e.pos.y += 256;
			if (e.pos.y > 256) e.pos.y -= 256;
			//change direction
			int d = std::rand() % 1000;
			if (d < 8) {
				e.dir = directions[d];
			}
			if (!e.fired and (projectiles.size() < 32) and (std::rand() % 2000 < score)) {
				projectiles.push_back(Projectile(e.pos.x, e.pos.y, e.dir.x, e.dir.y));
				projectiles.back().ignore_enemies = true;
			}
		}
	}

	//update and delete old projectiles
	constexpr float ProjectileSpeed = 100.0f;
	for (auto &p : projectiles) {
		if (p.pos.x < 0 or p.pos.x >= 255 or p.pos.y < 0 or p.pos.y >= 239) {
			projectiles.pop_front();
		} else {
			p.pos += ProjectileSpeed * p.dir * elapsed;
		}
	}
	//shoot
	if (space.downs > 0 and projectiles.size() < 32) {
		space.downs -= 1;
		projectiles.push_back(Projectile(player_at.x, player_at.y, player_direction.x, player_direction.y));
	}
	//check collisions
	for (auto &p : projectiles) {
		if (p.ignore_enemies) {
			if (glm::length(player_at - p.pos) <= 4.0f) {
				score = 0;
				//kill player
				player_at.y = 240;
				//kill projectile
				projectiles.pop_front();
				break;
			}
		} else {
			for (uint32_t i = 1; i < 32; i++) {
				if (!enemies[i].is_alive) continue;
				glm::vec2 dist = enemies[i].pos - p.pos;
				if (glm::length(dist) <= 4.0f) {
					score += 1;
					//kill enemy
					enemies[i].pos.y = 240;
					enemies[i].is_alive = false;
					//kill projectile
					projectiles.pop_front();
					break;
				}
			}
		}
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	space.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	//background color will be some hsv-like fade:
	// ppu.background_color = glm::u8vec4(
	// 	std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 0.0f / 3.0f) ) ) ))),
	// 	std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 0.0f / 3.0f) ) ) ))),
	// 	std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 0.0f / 3.0f) ) ) ))),
	// 	0xff
	// );

	//background scroll:
	// ppu.background_position.x = int32_t(-0.5f * player_at.x);
	// ppu.background_position.y = int32_t(-0.5f * player_at.y);

	//player sprite:
	ppu.sprites[0].x = int32_t(player_at.x);
	ppu.sprites[0].y = int32_t(player_at.y);
	if (player_direction == glm::vec2(0,1)) ppu.sprites[0].index = 32;
	if (player_direction == glm::vec2(0,-1)) ppu.sprites[0].index = 33;
	if (player_direction == glm::vec2(-1,0)) ppu.sprites[0].index = 34;
	if (player_direction == glm::vec2(1,0)) ppu.sprites[0].index = 35;
	if (player_direction == glm::vec2(-1,1)) ppu.sprites[0].index = 36;
	if (player_direction == glm::vec2(1,1)) ppu.sprites[0].index = 37;
	if (player_direction == glm::vec2(-1,-1)) ppu.sprites[0].index = 38;
	if (player_direction == glm::vec2(1,-1)) ppu.sprites[0].index = 39;
	ppu.sprites[0].attributes = 7;

	//some other misc sprites:
	for (uint32_t i = 1; i < 32; ++i) {
		ppu.sprites[i].x = enemies[i].pos.x;
		ppu.sprites[i].y = enemies[i].pos.y;
		ppu.sprites[i].index = 31;
		ppu.sprites[i].attributes = 6;
	}

	//projectiles:
	for (uint32_t i = 0; i < 32; i++) {
		if (i < projectiles.size()) {
			Projectile &p = projectiles[i];
			ppu.sprites[i+32].x = p.pos.x;
			ppu.sprites[i+32].y = p.pos.y;
			if (p.dir.x == p.dir.y) ppu.sprites[i+32].index = 16;
			else if (p.dir.x == -p.dir.y) ppu.sprites[i+32].index = 17;
			else if (p.dir.x == 0) ppu.sprites[i+32].index = 18;
			else if (p.dir.y == 0) ppu.sprites[i+32].index = 19;
			ppu.sprites[i+32].attributes = 2;
		} else {
			ppu.sprites[i+32].y = 255;
		}
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}

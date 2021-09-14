#include "PPU466.hpp"
#include "Mode.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, space;

	//some weird background animation:
	float background_fade = 0.0f;

	//player position:
	glm::vec2 player_at = glm::vec2(0.0f);
	glm::vec2 player_direction = glm::vec2(1, 1);

	std::vector<glm::vec2> directions{
		glm::vec2(0,1),
		glm::vec2(1,0),
		glm::vec2(0,-1),
		glm::vec2(-1,0),
		glm::vec2(-1,1),
		glm::vec2(-1,-1),
		glm::vec2(1,1),
		glm::vec2(1,-1)
	};

	//enemies:
	struct Character {
		glm::vec2 pos;
		glm::vec2 dir;
		bool is_alive = true;
	};
	std::vector<Character> enemies = std::vector<Character>(32);

	struct Projectile {
		glm::vec2 pos;
		glm::vec2 dir;
		bool ignore_enemies = false;
		Projectile(float x, float y, float dx, float dy) {
			pos = glm::vec2(x, y);
			dir = glm::vec2(dx, dy);
		}
	};

	std::deque<Projectile> projectiles;

	//score:
	uint32_t score = 0;

	//----- drawing handled by PPU466 -----

	PPU466 ppu;
};

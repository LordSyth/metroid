#include "freeglut.h"
#include <chrono>
#include <vector>
#include <map>
#include "xy.h"

class game {
public:
	struct hitbox : xyd {
		std::vector<xyd> vertices; //used as displacements from the parent's center xy
		double maxlifetime;
		double age;
		double damage;
		hitbox() : xyd(), vertices(), maxlifetime(), age(), damage() {}
	};
	struct movable : xyd {
		xyd velocity, dimensions;
		movable() : xyd(), velocity(), dimensions() {}
	};
	struct mover : movable {
		xyd acceleration, max_velocity;
		double health, damage;
		bool facingright;
		mover() : movable(), acceleration(), max_velocity(), health(), damage(), facingright() {}
	};
	struct fallable : mover {
		double gravity;
		bool grounded;
		fallable() : mover(), gravity(), grounded() {}
	};
	struct character : fallable {
		enum class state { standing, walking, crouching, jumping, falling, hurting };
		state actionstate;
		double invulnerability;
		int animindex; //is an index of which animation frame you're on
		std::vector<hitbox> attacks;
		double damage_modifier;
		character() : fallable(), actionstate(), invulnerability(), animindex(), attacks(), damage_modifier() {}
	};
	enum class tiletype {
		empty = 0
		,solidSquare
		,destroyableSquare
	};

	std::chrono::time_point<std::chrono::steady_clock> t;
	std::map<unsigned char, bool> keyboard;
	std::vector<hitbox> hitboxes;
	std::vector<fallable*> enemies;
	character player;
	tiletype map[50][50];

	void player_attack_test() {

		player.attacks.resize(3);
		player.attacks[0].vertices.emplace_back(player.dimensions.x * 1. / 2., player.dimensions.y * 1. / 4.);
		player.attacks[0].vertices.emplace_back(player.dimensions.x * 1. / 2., player.dimensions.y * 5. / 8.);
		player.attacks[0].vertices.emplace_back(player.dimensions.x * 7. / 4., player.dimensions.y * 5. / 8.);
		player.attacks[0].maxlifetime = 1. / 9.;
		player.attacks[0].age = -1. / 27.;
		player.attacks[0].damage = 0.;
		if (!player.facingright) {
			player.attacks[0].vertices[0].x *= -1.;
			player.attacks[0].vertices[1].x *= -1.;
			player.attacks[0].vertices[2].x *= -1.;
		}

		player.attacks[1].vertices.emplace_back(player.dimensions.x * 1. / 2., player.dimensions.y * 5. / 8.);
		player.attacks[1].vertices.emplace_back(player.dimensions.x * 1. / 2., player.dimensions.y);
		player.attacks[1].vertices.emplace_back(player.dimensions.x * 7. / 4., player.dimensions.y);
		player.attacks[1].vertices.emplace_back(player.dimensions.x * 7. / 4., player.dimensions.y * 5. / 8.);
		player.attacks[1].maxlifetime = 1. / 9.;
		player.attacks[1].age = -2. / 27.;
		player.attacks[1].damage = 0.;
		if (!player.facingright) {
			player.attacks[1].vertices[0].x *= -1.;
			player.attacks[1].vertices[1].x *= -1.;
			player.attacks[1].vertices[2].x *= -1.;
			player.attacks[1].vertices[3].x *= -1.;
		}

		player.attacks[2].vertices.emplace_back(player.dimensions.x * 1. / 2., player.dimensions.y);
		player.attacks[2].vertices.emplace_back(player.dimensions.x * 1. / 2., player.dimensions.y * 11. / 8.);
		player.attacks[2].vertices.emplace_back(player.dimensions.x * 7. / 4., player.dimensions.y);
		player.attacks[2].maxlifetime = 1. / 9.;
		player.attacks[2].age = -3. / 27.;
		player.attacks[2].damage = 0.;
		if (!player.facingright) {
			player.attacks[2].vertices[0].x *= -1.;
			player.attacks[2].vertices[1].x *= -1.;
			player.attacks[2].vertices[2].x *= -1.;
		}

	}
	void reset() {
		t = std::chrono::steady_clock::now();
		//reset the player
		{
			player.x = 6.1;
			player.y = 6.1;
			player.velocity.x = 0.;
			player.velocity.y = 0.;
			player.dimensions.x = 0.8;
			player.dimensions.y = 1.8;
			player.acceleration.x = 40.;
			player.acceleration.y = 20.;
			player.max_velocity.x = 40.;
			player.max_velocity.y = 60.;
			player.health = 10.;
			player.damage = 0.;
			player.facingright = true;
			player.gravity = -50.;
			player.grounded = false;
			player.actionstate = character::state::standing;
			player.invulnerability = 0.;
			player.animindex = 0;
			player.attacks.clear();
			player.damage_modifier = 1.;
		}
		//reset hitboxes
		hitboxes.clear();
		//reset enemies
		for (auto e : enemies) delete e;
		enemies.clear();
		{
			enemies.push_back(new fallable);
			enemies.front()->x = 16.1;
			enemies.front()->y = 5.1;
			enemies.front()->velocity.x = 0.;
			enemies.front()->velocity.y = 0.;
			enemies.front()->dimensions.x = 0.8;
			enemies.front()->dimensions.y = 0.8;
			enemies.front()->acceleration.x = 20.;
			enemies.front()->acceleration.y = 20.;
			enemies.front()->max_velocity.x = 20.;
			enemies.front()->max_velocity.y = 100.;
			enemies.front()->health = 4.;
			enemies.front()->damage = 0.;
			enemies.front()->facingright = false;
			enemies.front()->gravity = -50.;
			enemies.front()->grounded = false;
		}
		//reset tiles
		for (int i = 0; i < 50; ++i)
			map[i][3] = tiletype::solidSquare;
		for (int i = 7; i < 12; ++i)
			map[i][6] = tiletype::solidSquare;
	}

	game() : t(std::chrono::steady_clock::now()), keyboard(), hitboxes(), enemies(), player() {
		keyboard[' '] = keyboard['w'] = keyboard['a'] = keyboard['s'] = keyboard['d'] = false;
		reset();
	}
};
game g;

void display() {
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(g.player.x - 16., g.player.x + 16., g.player.y - 16., g.player.y + 16.);
	//draw tiles
	for (int x = 0; x < 50; ++x) {
		for (int y = 0; y < 50; ++y) {
			switch (g.map[x][y]) {
			case game::tiletype::solidSquare: {
				glColor4d(0., 0., 0., 1.);
				glBegin(GL_QUADS);
				glVertex3d(x, y, 0.);
				glVertex3d(x + 1., y, 0.);
				glVertex3d(x + 1., y + 1., 0.);
				glVertex3d(x, y + 1., 0.);
				glEnd();
			} break;
			}
		}
	}
	//draw enemies
	for (auto e : g.enemies) {
		glColor4f(.6f, .1f, .1f, 1.f);
		glBegin(GL_QUADS);
		glVertex3f(e->x - e->dimensions.x / 2.f, e->y, 1.f);
		glVertex3f(e->x + e->dimensions.x / 2.f, e->y, 1.f);
		glVertex3f(e->x + e->dimensions.x / 2.f, e->y + e->dimensions.y, 1.f);
		glVertex3f(e->x - e->dimensions.x / 2.f, e->y + e->dimensions.y, 1.f);
		glEnd();
	}
	//draw player
	{
		static bool blink = true;
		glColor4d(.1f, .2, .6, 1.);
		if (g.player.invulnerability > 0.)
			if (blink = !blink)
				glColor4d(.3, .6, 1., 1.);
			else
				glColor4d(.2, .4, .8, 1.);
		glBegin(GL_QUADS);
		glVertex3d(g.player.x - g.player.dimensions.x / 2., g.player.y, 1.);
		glVertex3d(g.player.x + g.player.dimensions.x / 2., g.player.y, 1.);
		glVertex3d(g.player.x + g.player.dimensions.x / 2., g.player.y + g.player.dimensions.y, 1.);
		glVertex3d(g.player.x - g.player.dimensions.x / 2., g.player.y + g.player.dimensions.y, 1.);
		glEnd();
	}
	//for now, we'll draw hitboxes as polygons
	for (auto h : g.hitboxes) if (h.age >= 0.) {
		glColor4d(1., .3, 1., .8);
		glBegin(GL_POLYGON);
		for (auto v : h.vertices)
			glVertex3d(v.x, v.y, 1.);
		glEnd();
	}
	for (auto a : g.player.attacks) if (a.age >= 0.) {
		glColor4d(.5, .25, 1., .8);
		glBegin(GL_POLYGON);
		for (auto v : a.vertices)
			glVertex3d(v.x, v.y, 1.);
		glEnd();
	}
	//draw health bar
	{
		glColor4d(.1, .6, .2, 1.);
		glBegin(GL_QUADS);
		glVertex3d(g.player.x - 15., g.player.y + 15., 0.);
		glVertex3d(g.player.x - 15. + g.player.health, g.player.y + 15., 0.);
		glVertex3d(g.player.x - 15. + g.player.health, g.player.y + 14., 0.);
		glVertex3d(g.player.x - 15., g.player.y + 14., 0.);
		glEnd();
	}
	glutSwapBuffers();
}
void idle() {
	//find delta t
	static double delta_t; //so far this is only used by idle()
	delta_t = std::chrono::duration<double>(std::chrono::steady_clock::now() - g.t).count();
	g.t = std::chrono::steady_clock::now();
	//subtract delta t from player invulnerability frames
	if (g.player.invulnerability > 0.) g.player.invulnerability -= delta_t;
	//advance hitboxes and then player pending hitboxes
	for (auto h = g.hitboxes.begin(); h != g.hitboxes.end(); ) {
		h->age += delta_t;
		if (h->age >= h->maxlifetime)
			h = g.hitboxes.erase(h);
		else ++h;
	}
	for (auto a = g.player.attacks.begin(); a != g.player.attacks.end(); ) {
		a->age += delta_t;
		if (a->age >= 0. && a->damage == 0.) {
			for (xyd& v : a->vertices)
				v += g.player;
			a->damage = 1. * g.player.damage_modifier;
		}
		if (a->age >= a->maxlifetime)
			a = g.player.attacks.erase(a);
		else ++a;
	}
	//add gravity to p.v.y
	g.player.velocity.y += g.player.gravity * delta_t;
	if (g.player.velocity.y > g.player.max_velocity.y)
		g.player.velocity.y = g.player.max_velocity.y;
	else if (-g.player.velocity.y > g.player.max_velocity.y)
		g.player.velocity.y = -g.player.max_velocity.y;
	//check keys and possibly add/subtract p.a.x and p.v.x
	if (g.keyboard['a'] && g.keyboard['d']) {
		//worry about this later, for now keep momentum
	}
	else if (g.keyboard['a']) {
		g.player.velocity.x -= g.player.acceleration.x * delta_t;
	}
	else if (g.keyboard['d']) {
		g.player.velocity.x += g.player.acceleration.x * delta_t;
	}
	else { //!keyboard['a'] && !keyboard['d']
		if (g.player.velocity.x < 0.) {
			if (-g.player.velocity.x < g.player.acceleration.x * delta_t) g.player.velocity.x = 0.;
			else g.player.velocity.x += g.player.acceleration.x * delta_t;
		}
		else if (g.player.velocity.x > 0.) {
			if (g.player.velocity.x < g.player.acceleration.x * delta_t) g.player.velocity.x = 0.;
			else g.player.velocity.x -= g.player.acceleration.x * delta_t;
		}
		//else do nothing
	}
	if (g.player.velocity.x > g.player.max_velocity.x)
		g.player.velocity.x = g.player.max_velocity.x;
	else if (-g.player.velocity.x > g.player.max_velocity.x)
		g.player.velocity.x = -g.player.max_velocity.x;
	if (g.keyboard['w'] && g.player.grounded) {
		g.player.velocity.y = g.player.acceleration.y;
		g.player.grounded = false;
	}
	if (g.keyboard['s'] && !g.player.grounded && g.player.velocity.y < 0.)
		g.player.velocity.y = -g.player.max_velocity.y;
	//check each direction to see if moving the player in will hit an impassable tile,
		//and stop them at the impassable
	if (g.player.velocity.y < 0.) {
		if (
			(g.map[int(g.player.x)][int(g.player.y + g.player.velocity.y * delta_t)] == game::tiletype::solidSquare) ||
			(g.map[int(g.player.x - g.player.dimensions.x / 2.)][int(g.player.y + g.player.velocity.y * delta_t)] == game::tiletype::solidSquare) ||
			(g.map[int(g.player.x + g.player.dimensions.x / 2.)][int(g.player.y + g.player.velocity.y * delta_t)] == game::tiletype::solidSquare)
			) {
			//this could be more elaborate and precise, but for now just teleport down
			g.player.y = floor(g.player.y);
			g.player.velocity.y = 0.;
			g.player.grounded = true;
		}
	}
	else if (g.player.velocity.y > 0.) {
		if (
			(g.map[int(g.player.x)][int(g.player.y + g.player.dimensions.y + g.player.velocity.y * delta_t)] == game::tiletype::solidSquare) ||
			(g.map[int(g.player.x - g.player.dimensions.x / 2.)][int(g.player.y + g.player.dimensions.y + g.player.velocity.y * delta_t)] == game::tiletype::solidSquare) ||
			(g.map[int(g.player.x + g.player.dimensions.x / 2.)][int(g.player.y + g.player.dimensions.y + g.player.velocity.y * delta_t)] == game::tiletype::solidSquare)
			) {
			//this could be more elaborate and precise, but for now just teleport down
			g.player.y = floor(g.player.y) + ceil(g.player.dimensions.y) - g.player.dimensions.y;
			g.player.velocity.y = 0.;
		}
	}
	if (g.player.velocity.x < 0.) {
		if (
			(g.map[int(g.player.x - g.player.dimensions.x / 2. + g.player.velocity.x * delta_t)][int(g.player.y)] == game::tiletype::solidSquare) ||
			(g.map[int(g.player.x - g.player.dimensions.x / 2. + g.player.velocity.x * delta_t)][int(g.player.y + g.player.dimensions.y / 2.)] == game::tiletype::solidSquare) ||
			(g.map[int(g.player.x - g.player.dimensions.x / 2. + g.player.velocity.x * delta_t)][int(g.player.y + g.player.dimensions.y)] == game::tiletype::solidSquare)
			) {
			g.player.x = floor(g.player.x) + g.player.dimensions.x / 2.;
			g.player.velocity.x = 0.;
		}
	}
	else if (g.player.velocity.x > 0.) {
		if (
			(g.map[int(g.player.x + g.player.dimensions.x / 2. + g.player.velocity.x * delta_t)][int(g.player.y)] == game::tiletype::solidSquare) ||
			(g.map[int(g.player.x + g.player.dimensions.x / 2. + g.player.velocity.x * delta_t)][int(g.player.y + g.player.dimensions.y / 2.)] == game::tiletype::solidSquare) ||
			(g.map[int(g.player.x + g.player.dimensions.x / 2. + g.player.velocity.x * delta_t)][int(g.player.y + g.player.dimensions.y)] == game::tiletype::solidSquare)
			) {
			g.player.x = ceil(g.player.x) - g.player.dimensions.x / 2.;
			g.player.velocity.x = 0.;
		}
	}
	if (g.player.velocity.y != 0.) g.player.grounded = false;
	//check to see if the player should be crouching
	if (g.player.actionstate != game::character::state::crouching && g.player.grounded && g.keyboard['s']) {
		g.player.actionstate = game::character::state::crouching;
		g.player.dimensions.y /= 2.;
	}
	if (g.player.actionstate == game::character::state::crouching && !(g.player.grounded && g.keyboard['s'])) {
		g.player.actionstate = game::character::state::standing;
		g.player.dimensions.y *= 2.;
	}
	g.player += g.player.velocity * delta_t;
	if (g.player.velocity.x > 0.) g.player.facingright = true;
	if (g.player.velocity.x < 0.) g.player.facingright = false;
	//go thru every enemy
	for (auto e : g.enemies) {
		//add gravity
		e->velocity.y += e->gravity * delta_t;
		if (e->velocity.y > e->max_velocity.y)
			e->velocity.y = e->max_velocity.y;
		else if (-e->velocity.y > e->max_velocity.y)
			e->velocity.y = -e->max_velocity.y;
		if (e->velocity.x > 0.) e->facingright = true;
		if (e->velocity.y > 0.) e->facingright = false;
		//collision detection
		if (e->velocity.y < 0.) {
			if (
				(g.map[int(e->x)][int(e->y + e->velocity.y * delta_t)] == game::tiletype::solidSquare) ||
				(g.map[int(e->x - e->dimensions.x / 2.)][int(e->y + e->velocity.y * delta_t)] == game::tiletype::solidSquare) ||
				(g.map[int(e->x + e->dimensions.x / 2.)][int(e->y + e->velocity.y * delta_t)] == game::tiletype::solidSquare)
				) {
				//this could be more elaborate and precise, but for now just teleport down
				e->y = floor(e->y);
				e->velocity.y = 0.;
				e->grounded = true;
			}
		}
		else if (e->velocity.y > 0.) {
			if (
				(g.map[int(e->x)][int(e->y + e->dimensions.y + e->velocity.y * delta_t)] == game::tiletype::solidSquare) ||
				(g.map[int(e->x - e->dimensions.x / 2.)][int(e->y + e->dimensions.y + e->velocity.y * delta_t)] == game::tiletype::solidSquare) ||
				(g.map[int(e->x + e->dimensions.x / 2.)][int(e->y + e->dimensions.y + e->velocity.y * delta_t)] == game::tiletype::solidSquare)
				) {
				//this could be more elaborate and precise, but for now just teleport down
				e->y = floor(e->y) + ceil(e->dimensions.y) - e->dimensions.y;
				e->velocity.y = 0.;
			}
		}
		if (e->velocity.x < 0.) {
			if (
				(g.map[int(e->x - e->dimensions.x / 2. + e->velocity.x * delta_t)][int(e->y)] == game::tiletype::solidSquare) ||
				(g.map[int(e->x - e->dimensions.x / 2. + e->velocity.x * delta_t)][int(e->y + e->dimensions.y / 2.)] == game::tiletype::solidSquare) ||
				(g.map[int(e->x - e->dimensions.x / 2. + e->velocity.x * delta_t)][int(e->y + e->dimensions.y)] == game::tiletype::solidSquare)
				) {
				e->x = floor(e->x) + e->dimensions.x / 2.;
				e->velocity.x = 0.;
			}
		}
		else if (e->velocity.x > 0.) {
			if (
				(g.map[int(e->x + e->dimensions.x / 2. + e->velocity.x * delta_t)][int(e->y)] == game::tiletype::solidSquare) ||
				(g.map[int(e->x + e->dimensions.x / 2. + e->velocity.x * delta_t)][int(e->y + e->dimensions.y / 2.)] == game::tiletype::solidSquare) ||
				(g.map[int(e->x + e->dimensions.x / 2. + e->velocity.x * delta_t)][int(e->y + e->dimensions.y)] == game::tiletype::solidSquare)
				) {
				e->x = ceil(e->x) - e->dimensions.x / 2.;
				e->velocity.x = 0.;
			}
		}
		if (e->velocity.y != 0.) e->grounded = false;
		e->x += e->velocity.x * delta_t; //possibly this should not be performed
		e->y += e->velocity.y * delta_t;
		//check for damage to the player, 5x5 checks per hitbox
		for (double y = 0.; y <= 4.; y += 1.) {
			if (g.player.invulnerability > 0.) break;
			for (double x = 0.; x <= 4.; x += 1.) {
				if (xyd(e->x - e->dimensions.x / 2. + e->dimensions.x / 4. * x,
					e->y + e->dimensions.y / 4. * y)
					.within(xyd(g.player.x - g.player.dimensions.x / 2., g.player.y),
						xyd(g.player.x + g.player.dimensions.x / 2., g.player.y + g.player.dimensions.y))) {
					g.player.health -= 1.1;
					g.player.invulnerability = .4;
					break;
				}
			}
		}
	}
	if (g.player.health <= 0.) g.reset();
	display();
}
template <bool down> void keydown(unsigned char key, int, int) {
	//TODO: save cursor location
	g.keyboard[key] = down;
	if (down) {
		if (key == VK_ESCAPE) g.reset();
		if (key == 'j')
			if (g.player.attacks.empty())
				g.player_attack_test();
	}
}

int main(int argc, char** argv) {
	//glut
	{
		glutInit(&argc, argv);
		glutInitWindowPosition(50, 50);
		glutInitWindowSize(500, 500);
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_ALPHA);
		glutCreateWindow("metroidvania");
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glClearColor(.8f, .8f, .8f, 1.f);
	}
	//func
	{
		glutDisplayFunc(display);
		glutIdleFunc(idle);
		glutKeyboardFunc(keydown<true>);
		glutKeyboardUpFunc(keydown<false>);
	}
	glutMainLoop();
	return 0;
}

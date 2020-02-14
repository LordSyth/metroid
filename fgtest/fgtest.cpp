#include "freeglut.h"
#include <chrono>
#include <vector>
#include <map>
#include "xy.h"

std::chrono::time_point<std::chrono::steady_clock> t;
double delta_t; //so far this is only used by idle()
std::map<unsigned char, bool> keyboard;

struct hitbox : xyd {
	std::vector<xyd> vertices; //all vertices stored here (in order); parent xyd can store whatever you want
	double maxlifetime;
	double age;
	double damage;
	bool playermade;
};
std::vector<hitbox> hitboxes;

struct movable : xyd { xyd velocity, dimensions; };
struct mover : movable { xyd acceleration, max_velocity; double health, damage; bool facingright; };
struct fallable : mover { double gravity; bool grounded; };
std::vector<fallable*> enemies;
struct character : fallable {
	enum class state { standing, walking, jumping, falling, hurting };
	state actionstate;
	double invulnerability;
	int animindex; //is an index of which animation frame you're on
	std::vector<hitbox> attacks;
};
character player;

enum class tiletype {
	empty = 0
	,solidSquare
	,destroyableSquare
};
tiletype map[50][50];

void player_attack_right_test() {
	player.attacks.resize(3);
	player.attacks[0].vertices.emplace_back(player.x + player.dimensions.x * 1. / 2., player.y + player.dimensions.y * 1. / 4.);
	player.attacks[0].vertices.emplace_back(player.x + player.dimensions.x * 1. / 2., player.y + player.dimensions.y * 5. / 8.);
	player.attacks[0].vertices.emplace_back(player.x + player.dimensions.x * 5. / 4., player.y + player.dimensions.y * 5. / 8.);
	player.attacks[0].maxlifetime = player.attacks[0].age = 1. / 3.;
	player.attacks[0].damage = 1.;
	player.attacks[0].playermade = true;
	player.attacks[1].vertices.emplace_back(player.x + player.dimensions.x * 1. / 2., player.y + player.dimensions.y * 5. / 8.);
	player.attacks[1].vertices.emplace_back(player.x + player.dimensions.x * 1. / 2., player.y + player.dimensions.y);
	player.attacks[1].vertices.emplace_back(player.x + player.dimensions.x * 5. / 4., player.y + player.dimensions.y);
	player.attacks[1].vertices.emplace_back(player.x + player.dimensions.x * 5. / 4., player.y + player.dimensions.y * 5. / 8.);
	player.attacks[1].maxlifetime = player.attacks[1].age = 2. / 3.;
	player.attacks[1].damage = 1.;
	player.attacks[1].playermade = true;
	player.attacks[2].vertices.emplace_back(player.x + player.dimensions.x * 1. / 2., player.y + player.dimensions.y);
	player.attacks[2].vertices.emplace_back(player.x + player.dimensions.x * 1. / 2., player.y + player.dimensions.y * 11. / 8.);
	player.attacks[2].vertices.emplace_back(player.x + player.dimensions.x * 5. / 4., player.y + player.dimensions.y);
	player.attacks[2].maxlifetime = player.attacks[2].age = 1.;
	player.attacks[2].damage = 1.;
	player.attacks[2].playermade = true;
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

void display() {
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(player.x - 16., player.x + 16., player.y - 16., player.y + 16.);
	//draw tiles
	for (int x = 0; x < 50; ++x) {
		for (int y = 0; y < 50; ++y) {
			switch (map[x][y]) {
			case tiletype::solidSquare: {
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
	for (auto e : enemies) {
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
		if (player.invulnerability > 0.)
			if (blink = !blink)
				glColor4d(.3, .6, 1., 1.);
			else
				glColor4d(.2, .4, .8, 1.);
		glBegin(GL_QUADS);
		glVertex3d(player.x - player.dimensions.x / 2., player.y, 1.);
		glVertex3d(player.x + player.dimensions.x / 2., player.y, 1.);
		glVertex3d(player.x + player.dimensions.x / 2., player.y + player.dimensions.y, 1.);
		glVertex3d(player.x - player.dimensions.x / 2., player.y + player.dimensions.y, 1.);
		glEnd();
	}
	//for now, we'll draw hitboxes as polygons
	for (auto h : hitboxes) {
		if (h.playermade) glColor4d(.5, .25, 1., .8);
		else glColor4d(1., .3, 1., .8);
		glBegin(GL_POLYGON);
		for (auto v : h.vertices)
			glVertex3d(v.x, v.y, 1.);
		glEnd();
	}
	//draw health bar
	{
		glColor4d(.1, .6, .2, 1.);
		glBegin(GL_QUADS);
		glVertex3d(player.x - 15., player.y + 15., 0.);
		glVertex3d(player.x - 15. + player.health, player.y + 15., 0.);
		glVertex3d(player.x - 15. + player.health, player.y + 14., 0.);
		glVertex3d(player.x - 15., player.y + 14., 0.);
		glEnd();
	}
	glutSwapBuffers();
}
void idle() {
	//find delta t
	delta_t = std::chrono::duration<double>(std::chrono::steady_clock::now() - t).count();
	t = std::chrono::steady_clock::now();
	//subtract delta t from player invulnerability frames
	if (player.invulnerability > 0.) player.invulnerability -= delta_t;
	//advance hitboxes and then player pending hitboxes
	for (auto h = hitboxes.begin(); h != hitboxes.end(); ) {
		h->age += delta_t;
		if (h->age >= h->maxlifetime)
			h = hitboxes.erase(h);
		else ++h;
	}
	for (auto h = player.attacks.begin(); h != player.attacks.end(); ) {
		h->age -= delta_t;
		if (h->age <= 0.) {
			hitboxes.push_back(*h);
			h = player.attacks.erase(h);
		}
		else ++h;
	}
	//add gravity to p.v.y
	player.velocity.y += player.gravity * delta_t;
	if (player.velocity.y > player.max_velocity.y)
		player.velocity.y = player.max_velocity.y;
	else if (-player.velocity.y > player.max_velocity.y)
		player.velocity.y = -player.max_velocity.y;
	//check keys and possibly add/subtract p.a.x and p.v.x
	if (keyboard['a'] && keyboard['d']) {
		//worry about this later, for now keep momentum
	}
	else if (keyboard['a']) {
		player.velocity.x -= player.acceleration.x * delta_t;
	}
	else if (keyboard['d']) {
		player.velocity.x += player.acceleration.x * delta_t;
	}
	else { //!keyboard['a'] && !keyboard['d']
		if (player.velocity.x < 0.) {
			if (-player.velocity.x < player.acceleration.x * delta_t) player.velocity.x = 0.;
			else player.velocity.x += player.acceleration.x * delta_t;
		}
		else if (player.velocity.x > 0.) {
			if (player.velocity.x < player.acceleration.x * delta_t) player.velocity.x = 0.;
			else player.velocity.x -= player.acceleration.x * delta_t;
		}
		//else do nothing
	}
	if (player.velocity.x > player.max_velocity.x)
		player.velocity.x = player.max_velocity.x;
	else if (-player.velocity.x > player.max_velocity.x)
		player.velocity.x = -player.max_velocity.x;
	if (keyboard['w'] && player.grounded) {
		player.velocity.y = player.acceleration.y;
		player.grounded = false;
	}
	if (keyboard['s'] && !player.grounded && player.velocity.y < 0.)
		player.velocity.y = -player.max_velocity.y;
	//check each direction to see if moving the player in will hit an impassable tile,
		//and stop them at the impassable
	if (player.velocity.y < 0.) {
		if (
			(map[int(player.x)][int(player.y + player.velocity.y * delta_t)] == tiletype::solidSquare) ||
			(map[int(player.x - player.dimensions.x / 2.)][int(player.y + player.velocity.y * delta_t)] == tiletype::solidSquare) ||
			(map[int(player.x + player.dimensions.x / 2.)][int(player.y + player.velocity.y * delta_t)] == tiletype::solidSquare)
			) {
			//this could be more elaborate and precise, but for now just teleport down
			player.y = floor(player.y);
			player.velocity.y = 0.;
			player.grounded = true;
		}
	}
	else if (player.velocity.y > 0.) {
		if (
			(map[int(player.x)][int(player.y + player.dimensions.y + player.velocity.y * delta_t)] == tiletype::solidSquare) ||
			(map[int(player.x - player.dimensions.x / 2.)][int(player.y + player.dimensions.y + player.velocity.y * delta_t)] == tiletype::solidSquare) ||
			(map[int(player.x + player.dimensions.x / 2.)][int(player.y + player.dimensions.y + player.velocity.y * delta_t)] == tiletype::solidSquare)
			) {
			//this could be more elaborate and precise, but for now just teleport down
			player.y = floor(player.y) + ceil(player.dimensions.y) - player.dimensions.y;
			player.velocity.y = 0.;
		}
	}
	if (player.velocity.x < 0.) {
		if (
			(map[int(player.x - player.dimensions.x / 2. + player.velocity.x * delta_t)][int(player.y)] == tiletype::solidSquare) ||
			(map[int(player.x - player.dimensions.x / 2. + player.velocity.x * delta_t)][int(player.y + player.dimensions.y / 2.)] == tiletype::solidSquare) ||
			(map[int(player.x - player.dimensions.x / 2. + player.velocity.x * delta_t)][int(player.y + player.dimensions.y)] == tiletype::solidSquare)
			) {
			player.x = floor(player.x) + player.dimensions.x / 2.;
			player.velocity.x = 0.;
		}
	}
	else if (player.velocity.x > 0.) {
		if (
			(map[int(player.x + player.dimensions.x / 2. + player.velocity.x * delta_t)][int(player.y)] == tiletype::solidSquare) ||
			(map[int(player.x + player.dimensions.x / 2. + player.velocity.x * delta_t)][int(player.y + player.dimensions.y / 2.)] == tiletype::solidSquare) ||
			(map[int(player.x + player.dimensions.x / 2. + player.velocity.x * delta_t)][int(player.y + player.dimensions.y)] == tiletype::solidSquare)
			) {
			player.x = ceil(player.x) - player.dimensions.x / 2.;
			player.velocity.x = 0.;
		}
	}
	if (player.velocity.y != 0.) player.grounded = false;
	player.x += player.velocity.x * delta_t; //possibly this should not be performed
	player.y += player.velocity.y * delta_t;
	//go thru every enemy
	for (auto e : enemies) {
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
				(map[int(e->x)][int(e->y + e->velocity.y * delta_t)] == tiletype::solidSquare) ||
				(map[int(e->x - e->dimensions.x / 2.)][int(e->y + e->velocity.y * delta_t)] == tiletype::solidSquare) ||
				(map[int(e->x + e->dimensions.x / 2.)][int(e->y + e->velocity.y * delta_t)] == tiletype::solidSquare)
				) {
				//this could be more elaborate and precise, but for now just teleport down
				e->y = floor(e->y);
				e->velocity.y = 0.;
				e->grounded = true;
			}
		}
		else if (e->velocity.y > 0.) {
			if (
				(map[int(e->x)][int(e->y + e->dimensions.y + e->velocity.y * delta_t)] == tiletype::solidSquare) ||
				(map[int(e->x - e->dimensions.x / 2.)][int(e->y + e->dimensions.y + e->velocity.y * delta_t)] == tiletype::solidSquare) ||
				(map[int(e->x + e->dimensions.x / 2.)][int(e->y + e->dimensions.y + e->velocity.y * delta_t)] == tiletype::solidSquare)
				) {
				//this could be more elaborate and precise, but for now just teleport down
				e->y = floor(e->y) + ceil(e->dimensions.y) - e->dimensions.y;
				e->velocity.y = 0.;
			}
		}
		if (e->velocity.x < 0.) {
			if (
				(map[int(e->x - e->dimensions.x / 2. + e->velocity.x * delta_t)][int(e->y)] == tiletype::solidSquare) ||
				(map[int(e->x - e->dimensions.x / 2. + e->velocity.x * delta_t)][int(e->y + e->dimensions.y / 2.)] == tiletype::solidSquare) ||
				(map[int(e->x - e->dimensions.x / 2. + e->velocity.x * delta_t)][int(e->y + e->dimensions.y)] == tiletype::solidSquare)
				) {
				e->x = floor(e->x) + e->dimensions.x / 2.;
				e->velocity.x = 0.;
			}
		}
		else if (e->velocity.x > 0.) {
			if (
				(map[int(e->x + e->dimensions.x / 2. + e->velocity.x * delta_t)][int(e->y)] == tiletype::solidSquare) ||
				(map[int(e->x + e->dimensions.x / 2. + e->velocity.x * delta_t)][int(e->y + e->dimensions.y / 2.)] == tiletype::solidSquare) ||
				(map[int(e->x + e->dimensions.x / 2. + e->velocity.x * delta_t)][int(e->y + e->dimensions.y)] == tiletype::solidSquare)
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
			if (player.invulnerability > 0.) break;
			for (double x = 0.; x <= 4.; x += 1.) {
				if (xyd(e->x - e->dimensions.x / 2. + e->dimensions.x / 4. * x,
					e->y + e->dimensions.y / 4. * y)
					.within(xyd(player.x - player.dimensions.x / 2., player.y),
						xyd(player.x + player.dimensions.x / 2., player.y + player.dimensions.y))) {
					player.health -= 1.1;
					player.invulnerability = .4;
					break;
				}
			}
		}
	}
	if (player.health <= 0.) reset();
	display();
}
template <bool down> void keydown(unsigned char key, int, int) {
	//TODO: save cursor location
	keyboard[key] = down;
	if (down) {
		if (key == VK_ESCAPE) reset();
		if (key == 'j') if (player.attacks.empty())
			player_attack_right_test();
	}
}

int main(int argc, char** argv) {
	//game
	{
		t = std::chrono::steady_clock::now();
		keyboard[' '] = keyboard['w'] = keyboard['a'] = keyboard['s'] = keyboard['d'] = false;
		reset();
	}
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

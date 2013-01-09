//=============================================================================================
// Szamitogepes grafika hazi feladat keret. Ervenyes 2012-tol.
// A //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// sorokon beluli reszben celszeru garazdalkodni, mert a tobbit ugyis toroljuk.
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni (printf is fajlmuvelet!)
// - new operatort hivni az onInitialization függvényt kivéve, a lefoglalt adat korrekt felszabadítása nélkül
// - felesleges programsorokat a beadott programban hagyni
// - tovabbi kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan gl/glu/glut fuggvenyek hasznalhatok, amelyek
// 1. Az oran a feladatkiadasig elhangzottak ES (logikai AND muvelet)
// 2. Az alabbi listaban szerepelnek:
// Rendering pass: glBegin, glVertex[2|3]f, glColor3f, glNormal3f, glTexCoord2f, glEnd, glDrawPixels
// Transzformaciok: glViewport, glMatrixMode, glLoadIdentity, glMultMatrixf, gluOrtho2D,
// glTranslatef, glRotatef, glScalef, gluLookAt, gluPerspective, glPushMatrix, glPopMatrix,
// Illuminacio: glMaterialfv, glMaterialfv, glMaterialf, glLightfv
// Texturazas: glGenTextures, glBindTexture, glTexParameteri, glTexImage2D, glTexEnvi,
// Pipeline vezerles: glShadeModel, glEnable/Disable a kovetkezokre:
// GL_LIGHTING, GL_NORMALIZE, GL_DEPTH_TEST, GL_CULL_FACE, GL_TEXTURE_2D, GL_BLEND, GL_LIGHT[0..7]
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Kriván Bálint
// Neptun : CBVOEN
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================

#include <math.h>
#include <stdlib.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
// MsWindows-on ez is kell
#include <windows.h>
#endif // Win32 platform
#include <GL/gl.h>
#include <GL/glu.h>
// A GLUT-ot le kell tolteni: http://www.opengl.org/resources/libraries/glut/
#include <GL/glut.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Innentol modosithatod...

const int screenWidth = 600;
const int screenHeight = 600;
const float TOLERANCE = 0.02f;
const int MAX_SLOPE_COUNT = 10;
const int MAX_SKIER_COUNT = 10;
const long MIN_TIME_RESOLUTION = 10;

struct Vector {
	float x, y, z;

	Vector() {
		x = y = z = 0;
	}
	Vector(float x0, float y0, float z0 = 0) {
		x = x0;
		y = y0;
		z = z0;
	}
	Vector operator*(float a) const {
		return Vector(x * a, y * a, z * a);
	}
	Vector operator/(float a) const {
		return Vector(x / a, y / a, z / a);
	}
	Vector operator+(const Vector& v) const {
		return Vector(x + v.x, y + v.y, z + v.z);
	}
	Vector& operator+=(const Vector& v) {
		x += v.x;
		y += v.y;
		return *this;
	}
	Vector operator-(const Vector& v) const {
		return Vector(x - v.x, y - v.y, z - v.z);
	}
	float operator*(const Vector& v) const {
		return (x * v.x + y * v.y + z * v.z);
	}
	Vector Normal() const {
		return (*this) / Length();
	}
	Vector Perpendicular() const {
		return Vector(-y, x);
	}
	float Length() const {
		return sqrtf(x * x + y * y + z * z);
	}
};

struct Color {
	float r, g, b;

	Color() {
		r = g = b = 0;
	}
	Color(float r0, float g0, float b0) {
		r = r0;
		g = g0;
		b = b0;
	}
	Color operator*(float a) const {
		return Color(r * a, g * a, b * a);
	}
	Color operator*(const Color& c) const {
		return Color(r * c.r, g * c.g, b * c.b);
	}
	Color operator+(const Color& c) const {
		return Color(r + c.r, g + c.g, b + c.b);
	}
};

float getHeight(const Vector& p) {
	float x = p.x;
	float y = p.y;

	return (-1 * sinf(1 / (x / 2500 + 2.2f)) + sinf(x / 2500 + y / 2500)
			+ 0.5f * sinf(2 * x * x / 2500 / 2500)
			+ 0.25f * sinf(3 * x / 2500 + y * y / 2500 / 2500)
			+ 0.1f * sinf(y * y * y / 2500 / 2500 / 2500) + 2.24934f)
			/ (2.07533f + 2.24934f) * 1000;
}

Vector convertPictureToWorld(int px, int py) {
	return Vector((float) px / 600 * 10000 - 5000,
			-(float) py / 600 * 10000 + 5000);
}

Vector convertWorldToGL(const Vector& w) {
	return w / 5000;
}

Vector directionAt(const Vector& p) {
	float wx = p.x;
	float wy = p.y;

	float dx = 3.2f * powf(10, -7) * wx * cosf(wx * wx / 3125000)
			+ 0.0003f * cosf((7500 * wx + wy * wy) / 6250000)
			+ (cosf((wx + wy) / 2500)) / 2500
			+ 0.0004f * cosf(1 / (0.0004f * wx + 2.2f))
					/ powf(0.0004f * wx + 2.2f, 2);
	float dy =
			8 * powf(10, -8) * wy * cosf((7500 * wx + wy * wy) / 6250000)
					+ (cosf((wx + wy) / 2500)) / 2500
					+ 1.92f * powf(10, -11) * wy * wy
							* cosf(wy * wy * wy / 15625000000);

	Vector grad = Vector(dx, dy);

	return grad * (-1000.0f / (2.07533f + 2.24934f));
}

class Slope {
	Vector points[21];
	int steepness[20];
	int sections;

	enum Difficulty {
		BLACK, RED, BLUE
	};

	float getSlope(const Vector& v) const {
		return atanf(v.Length()) / M_PI * 180;
	}

	bool addSection() {
		Vector direction = directionAt(points[sections]);

		if (getSlope(direction) < 1.0f)
			return false;

		steepness[sections] = getSlope(direction);
		points[sections + 1] = points[sections]
				+ direction * (100 / direction.Length());
		sections++;

		if (points[sections].x > 5000.0f || points[sections].x < -5000.0f
				|| points[sections].y > 5000.0f
				|| points[sections].y < -5000.0f) {
			return false;
		}

		return true;
	}

	Difficulty getDifficulty(float slope) const {
		if (slope - 40.0f > TOLERANCE)
			return BLACK;
		if (slope - 20.0f > TOLERANCE && slope - 40.0f < TOLERANCE)
			return RED;
		return BLUE;
	}

public:
	Slope() {
		sections = 0;
	}

	bool makeAt(const Vector& start) {
		sections = 0;
		points[0] = start;

		while (addSection() && sections < 20)
			;

		return sections > 0;
	}

	int getSectionsCount() const {
		return sections;
	}

	Vector& getStartPointOf(int section) {
		return points[section];
	}

	Vector getDirectionOf(int section) const {
		return (points[section + 1] - points[section]).Normal();
	}

	float getSpeedAt(int section) const {
		switch (getDifficulty(steepness[section])) {
		case BLACK:
			return 40.0f * 1000 / 3600;
		case RED:
			return 20.0f * 1000 / 3600;
		case BLUE:
			return 10.0f * 1000 / 3600;
		default:
			return 0;
		}
	}

	void draw() const {
		glBegin(GL_LINE_STRIP);
		Difficulty lastDiff;
		Difficulty currentDiff;
		for (int i = 0; i <= sections; i++) {
			Vector point = convertWorldToGL(points[i]);

			if (i < sections) {
				currentDiff = getDifficulty(steepness[i]);
				if (i != 0 && currentDiff != lastDiff) {
					glVertex2f(point.x, point.y);
					lastDiff = currentDiff;
				}

				switch (currentDiff) {
				case BLACK:
					glColor3f(0, 0, 0);
					break;
				case RED:
					glColor3f(1.0f, 0, 0);
					break;
				case BLUE:
					glColor3f(0, 0, 1.0f);
					break;
				}
			}

			glVertex2f(point.x, point.y);
		}
		glEnd();
	}
};

class Skier {
	Vector direction;
	Vector refPoint;
	int section;
	Slope slope;
	float distanceInSection;
	bool skiing;
	long advantage;

	void placeAtTheBeginningOfTheSection() {
		refPoint = slope.getStartPointOf(section);
		direction = slope.getDirectionOf(section);
		distanceInSection = 0;
	}

public:
	Skier() {
		skiing = false;
	}

	void putOn(const Slope& s) {
		skiing = true;
		section = 0;
		slope = s;
		placeAtTheBeginningOfTheSection();
	}

	bool isSkiing() const {
		return skiing;
	}

	void move(float elapsed) {
		float remainingTime = elapsed;

		float speed;
		while (remainingTime > 0) {
			speed = slope.getSpeedAt(section);

			if (distanceInSection + (speed * remainingTime / 1000) <= 100) {
				distanceInSection += (speed * remainingTime / 1000);
				refPoint += direction * (speed * remainingTime / 1000);
				remainingTime = 0;
			} else {
				distanceInSection = 0;
				section += 1;
				if (section == slope.getSectionsCount()) {
					skiing = false;
					continue;
				}
				remainingTime -= (100.0f - distanceInSection) / speed * 1000.0f;
				placeAtTheBeginningOfTheSection();
			}

			if (refPoint.x > 5000.0f || refPoint.x < -5000.0f
					|| refPoint.y > 5000.0f || refPoint.y < -5000.0f) {
				skiing = false;
			}
		}
	}

	void draw() const {
		Vector glDim = convertWorldToGL(Vector(100, 300));

		Vector tip = convertWorldToGL(refPoint);
		Vector halfOfWidth = direction.Perpendicular() * glDim.x;
		Vector heightOfTriangle = direction * glDim.y;

		Vector top = tip - heightOfTriangle + halfOfWidth;
		Vector bot = tip - heightOfTriangle - halfOfWidth;

		glColor3f(1.0f, 0.549f, 0);
		glBegin(GL_TRIANGLES);
		glVertex2f(tip.x, tip.y);
		glVertex2f(top.x, top.y);
		glVertex2f(bot.x, bot.y);
		glEnd();

		glColor3f(1.0f, 0, 0);
		glBegin(GL_LINE_LOOP);
		glVertex2f(tip.x, tip.y);
		glVertex2f(top.x, top.y);
		glVertex2f(bot.x, bot.y);
		glEnd();
	}
};

Color image[screenWidth * screenHeight];
Slope slopes[MAX_SLOPE_COUNT];
int slopeCount = 0;
int nextSlope = 0;
Skier skiers[MAX_SKIER_COUNT];
long lastSimulationAt;

void onInitialization() {
	glViewport(0, 0, screenWidth, screenHeight);

	for (int Y = 0; Y < screenHeight; Y++)
		for (int X = 0; X < screenWidth; X++) {
			Vector p = convertPictureToWorld(X, -Y + 600);
			float color = getHeight(p) / 1000;
			image[Y * screenWidth + X] = Color(color, color, color);
		}
}

void onDisplay() {
	glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDrawPixels(screenWidth, screenHeight, GL_RGB, GL_FLOAT, image);

	for (int i = 0; i < slopeCount; i++) {
		slopes[i].draw();
	}

	for (int i = 0; i < MAX_SKIER_COUNT; i++) {
		const Skier& s = skiers[i];
		if (s.isSkiing())
			s.draw();
	}

	glutSwapBuffers();
}

void onMouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT && state == GLUT_DOWN) {
		if (slopeCount == MAX_SLOPE_COUNT)
			return;

		if (slopes[slopeCount].makeAt(convertPictureToWorld(x, y))) {
			slopeCount++;
		}
		glutPostRedisplay();
	}
}

void doSimulation(long elapsed) {
	for (int i = 0; i < MAX_SKIER_COUNT; i++) {
		Skier& s = skiers[i];

		if (s.isSkiing()) {
			s.move(elapsed);
			glutPostRedisplay();
		}
	}
}

void onKeyboard(unsigned char key, int x, int y) {
	if (key == 's') {
		int i = 0;
		while (i < MAX_SKIER_COUNT && skiers[i].isSkiing())
			i++;

		if (i == MAX_SKIER_COUNT)
			return;

		long current = glutGet(GLUT_ELAPSED_TIME);
		long elapsed = current - lastSimulationAt;
		lastSimulationAt = current;
		doSimulation(elapsed);

		Skier& s = skiers[i];
		nextSlope = nextSlope % slopeCount;
		s.putOn(slopes[nextSlope]);
		nextSlope = nextSlope + 1;

		glutPostRedisplay();
	}
}

void onIdle() {
	long current = glutGet(GLUT_ELAPSED_TIME);
	long elapsed = current - lastSimulationAt;
	if (elapsed < MIN_TIME_RESOLUTION) {
		return;
	}

	lastSimulationAt = current;
	doSimulation(elapsed);
}

// ...Idaig modosithatod
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// A C++ program belepesi pontja, a main fuggvenyt mar nem szabad bantani
int main(int argc, char **argv) {
	glutInit(&argc, argv); // GLUT inicializalasa
	glutInitWindowSize(600, 600); // Alkalmazas ablak kezdeti merete 600x600 pixel
	glutInitWindowPosition(100, 100); // Az elozo alkalmazas ablakhoz kepest hol tunik fel
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH); // 8 bites R,G,B,A + dupla buffer + melyseg buffer

	glutCreateWindow("Grafika hazi feladat"); // Alkalmazas ablak megszuletik es megjelenik a kepernyon

	glMatrixMode(GL_MODELVIEW); // A MODELVIEW transzformaciot egysegmatrixra inicializaljuk
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION); // A PROJECTION transzformaciot egysegmatrixra inicializaljuk
	glLoadIdentity();

	onInitialization(); // Az altalad irt inicializalast lefuttatjuk

	glutDisplayFunc(onDisplay); // Esemenykezelok regisztralasa
	glutMouseFunc(onMouse);
	glutIdleFunc(onIdle);
	glutKeyboardFunc(onKeyboard);

	glutMainLoop(); // Esemenykezelo hurok

	return 0;
}

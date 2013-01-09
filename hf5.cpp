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

#define DEBUG
#if defined(DEBUG)
#include <iostream>
using namespace std;
#endif

const int screenWidth = 600;
const int screenHeight = 600;
const int WIDTH = 128;
const float CAMERA_DISTANCE_MIN = 20.0f;
const float CAMERA_DISTANCE_MAX = 40.0f;
const int MINES_COUNT = 5;
const int CIRCLE_RES = 20;
const float M_UNIT = 1.0f; // 1 méter hány egység
const float CRATER_SIZE = 1.0f * M_UNIT;
const float MINE_HEIGHT = 0.02f * M_UNIT;
const long KEY_DOWN_TRESHOLD = 200;

const float CAMERA_DISTANCE = 40.0f;
const float CAMERA_ANGLEY = 64.90f;
const float CAMERA_ANGLEX = 62.05f;

unsigned int textures[2];

struct Vector {
	float x, y, z;
	bool zero;

	Vector() {
		x = y = z = 0;
		zero = true;
	}
	Vector(float x0, float y0, float z0 = 0) {
		x = x0;
		y = y0;
		z = z0;
		zero = false;
	}
	Vector operator*(float a) const {
		return Vector(x * a, y * a, z * a);
	}
	Vector& operator*=(float a) {
		x *= a;
		y *= a;
		z *= a;
		return *this;
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
		z += v.z;
		return *this;
	}
	Vector& operator-=(const Vector& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}
	Vector operator-(const Vector& v) const {
		return Vector(x - v.x, y - v.y, z - v.z);
	}
	float operator*(const Vector& v) const {
		return (x * v.x + y * v.y + z * v.z);
	}
	Vector operator%(const Vector& c) const {
		const Vector& b = *this;

		return Vector(b.y * c.z - b.z * c.y, b.z * c.x - b.x * c.z,
				b.x * c.y - b.y * c.x);
	}
	Vector normal() const {
		return (*this) / length();
	}
	float length() const {
		return sqrtf(x * x + y * y + z * z);
	}
	bool isNull() const {
		return zero;
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
	Color operator/(float a) const {
		return Color(r / a, g / a, b / a);
	}
	Color& operator*=(float a) {
		r *= a;
		g *= a;
		b *= a;
		return *this;
	}
	Color operator*(const Color& c) const {
		return Color(r * c.r, g * c.g, b * c.b);
	}
	Color& operator*=(const Color& c) {
		*this = (*this) * c;
		return *this;
	}
	Color operator+(const Color& c) const {
		return Color(r + c.r, g + c.g, b + c.b);
	}
	Color& operator+=(const Color& c) {
		*this = (*this) + c;
		return *this;
	}

	Color normal() {
		float max;
		max = (r < g) ? g : r;
		if (max < b)
			max = b;

		if (max <= 1.0f) {
			return *this;
		} else {
			return Color(r / max, g / max, b / max);
		}
	}
};

void drawCylinder(float radius, float height) {
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	for (int i = 0; i <= CIRCLE_RES; i++) {
		float rad = 2 * M_PI / CIRCLE_RES * i;
		glNormal3f(0.0f, -1.0f, 0.0f);
		glVertex3f(sinf(rad) * radius, 0.0f, cosf(rad) * radius);
	}
	glEnd();

	for (int i = 0; i < CIRCLE_RES; i++) {
		float rad_1 = 2 * M_PI / CIRCLE_RES * i;
		float rad_2 = 2 * M_PI / CIRCLE_RES * (i + 1);
		glBegin(GL_QUADS);
		glNormal3f(sinf(rad_1) * radius, 0.0f, cosf(rad_1) * radius);
		glVertex3f(sinf(rad_1) * radius, 0.0f, cosf(rad_1) * radius);
		glNormal3f(sinf(rad_2) * radius, 0.0f, cosf(rad_2) * radius);
		glVertex3f(sinf(rad_2) * radius, 0.0f, cosf(rad_2) * radius);
		glNormal3f(sinf(rad_2) * radius, 0.0f, cosf(rad_2) * radius);
		glVertex3f(sinf(rad_2) * radius, height, cosf(rad_2) * radius);
		glNormal3f(sinf(rad_1) * radius, 0.0f, cosf(rad_1) * radius);
		glVertex3f(sinf(rad_1) * radius, height, cosf(rad_1) * radius);
		glEnd();
	}

	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, height, 0.0f);
	for (int i = 0; i <= CIRCLE_RES; i++) {
		float rad = 2 * M_PI / CIRCLE_RES * i;
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(cosf(rad) * radius, height, sinf(rad) * radius);
	}
	glEnd();
}

void drawCone(float radius, float height) {
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	for (int i = 0; i <= CIRCLE_RES; i++) {
		float rad = 2 * M_PI / CIRCLE_RES * i;
		glVertex3f(sinf(rad) * radius, 0.0f, cosf(rad) * radius);
	}
	glEnd();

	Vector point1, point2, center;
	Vector top(0.0f, height, 0.0f);
	Vector normal, normal1, normal2;

	for (int i = 0; i < CIRCLE_RES; i++) {
		float rad_1 = 2 * M_PI / CIRCLE_RES * i;
		float rad_2 = 2 * M_PI / CIRCLE_RES * (i + 1);
		glBegin(GL_TRIANGLES);

		point1 = Vector(sinf(rad_1) * radius, 0.0f, cosf(rad_1) * radius);
		normal1 = (Vector(-point1.z, 0, point1.x) % (point1 - top)).normal();
		glNormal3f(point1.x, point1.y, point1.z);
		glVertex3f(point1.x, point1.y, point1.z);

		point2 = Vector(sinf(rad_2) * radius, 0.0f, cosf(rad_2) * radius);
		normal2 = (Vector(-point2.z, 0, point2.x) % (point2 - top)).normal();
		glNormal3f(point2.x, point2.y, point2.z);
		glVertex3f(point2.x, point2.y, point2.z);

		center = Vector(sinf((rad_1 + rad_2) / 2.0f) * radius, 0.0f,
				cosf((rad_1 + rad_2) / 2.0f) * radius);
		normal = (Vector(-center.z, 0, center.x) % (center - top)).normal();
		glNormal3f(normal.x, normal.y, normal.z);
		glVertex3f(top.x, top.y, top.z);

		glEnd();

#if defined(NORMAL)
		glBegin(GL_LINES);
		glVertex3f(point1.x, point1.y, point1.z);
		glVertex3f(point1.x + normal1.x, point1.y + normal1.y, point1.z + normal1.z);
		glVertex3f(point2.x, point2.y, point2.z);
		glVertex3f(point2.x + normal2.x, point2.y + normal2.y, point2.z + normal2.z);
		glVertex3f(top.x, top.y, top.z);
		glVertex3f(top.x + normal.x, top.y + normal.y, top.z + normal.z);
		glEnd();
#endif
	}
}

Vector getVFromSC(float r, float theta, float phi) {
	return Vector(r * sinf(phi) * cosf(theta), r * cosf(phi),
			r * sinf(phi) * sinf(theta));
}

void drawSphere(float radius) {
	float theta, phi1, phi2;

	glBegin(GL_POINTS);
	for (int i = 0; i < CIRCLE_RES; i++) {
		phi1 = M_PI / CIRCLE_RES * i;
		phi2 = M_PI / CIRCLE_RES * (i + 1);
		for (int j = 0; j < CIRCLE_RES; j++) {
			theta = 2 * M_PI / CIRCLE_RES * j;
			glBegin(GL_QUADS);
			Vector v1 = getVFromSC(radius, theta, phi1);
			Vector v2 = getVFromSC(radius, theta, phi2);

			glTexCoord2f(theta / (2 * M_PI), phi1 / M_PI);
			glNormal3f(v1.x, v1.y, v1.z);
			glVertex3f(v1.x, v1.y, v1.z);
			glTexCoord2f(theta / (2 * M_PI), phi2 / M_PI);
			glNormal3f(v2.x, v2.y, v2.z);
			glVertex3f(v2.x, v2.y, v2.z);

			theta = 2 * M_PI / CIRCLE_RES * (j + 1);
			Vector v3 = getVFromSC(radius, theta, phi2);
			Vector v4 = getVFromSC(radius, theta, phi1);

			glTexCoord2f(theta / (2 * M_PI), phi2 / M_PI);
			glNormal3f(v3.x, v3.y, v3.z);
			glVertex3f(v3.x, v3.y, v3.z);
			glTexCoord2f(theta / (2 * M_PI), phi1 / M_PI);
			glNormal3f(v4.x, v4.y, v4.z);
			glVertex3f(v4.x, v4.y, v4.z);
			glEnd();
		}
	}
	glEnd();
}

bool drawNormals = false;

class SceneObjectHolder {
public:

	virtual void transformationsFor(int x, int z) = 0;
};

class SceneObject {
protected:
	int x, z;
	SceneObjectHolder * attachedTo;
	bool visible;
public:
	SceneObject(int x, int z, SceneObjectHolder * attachedTo) :
			x(x), z(z), attachedTo(attachedTo), visible(true) {
	}

	virtual int getX() const {
		return x;
	}

	virtual int getZ() const {
		return z;
	}

	void draw() const {
		if (visible) {
			if (attachedTo) {
				glPushMatrix();
				attachedTo->transformationsFor(getX(), getZ());
			}
			onDraw();
			if (attachedTo) {
				glPopMatrix();
			}
		}
	}

	void drawLight() const {
		if (visible) {
			if (attachedTo) {
				glPushMatrix();
				attachedTo->transformationsFor(getX(), getZ());
			}
			onDrawLight();
			if (attachedTo) {
				glPopMatrix();
			}
		}
	}

	bool isVisible() {
		return visible;
	}

	void hide() {
		visible = false;
	}

	void show() {
		visible = true;
	}

	virtual void onDraw() const = 0;

	virtual void onDrawLight() const {
	}
};

class Mine: public SceneObject {
	bool active;
	Color color;
public:
	Mine() :
			SceneObject(-1, -1, NULL), active(false) {
	}

	Mine(int x, int z, SceneObjectHolder * mf) :
			SceneObject(x, z, mf), active(false), color(1.0f, 0.0f, 0.0f) {
	}

	void onDraw() const {
		glColor3f(color.r, color.g, color.b);
		GLfloat kd[] = { color.r, color.g, color.b };
		glMaterialfv(GL_FRONT, GL_DIFFUSE, kd);
		glMaterialf(GL_FRONT, GL_SHININESS, 20);

		drawCylinder(0.2f * M_UNIT, MINE_HEIGHT);

		if (active) {
			glTranslatef(0.0f, MINE_HEIGHT + 0.05f * M_UNIT, 0.0f);

			GLfloat pos[] = { 0.0f, 0.0f, 0.0f, 1.0f };
			glLightfv(GL_LIGHT2, GL_POSITION, pos);
			glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.1f);
		}
	}

	void onDrawLight() const {
		glTranslatef(0.0f, MINE_HEIGHT + 0.05f * M_UNIT, 0.0f);

		GLfloat pos[] = { 0.0f, 1.0f, 0.0f, 1.0f };
		glLightfv(GL_LIGHT2, GL_POSITION, pos);
		glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.01f);
	}

	void activate() {
		active = true;
	}
};

class Robot: public SceneObject {
	float phi, theta1, theta2;
	Color color;
	Vector v;
	Vector a;
	float realX, realZ;
	long lastSpeedChange;

	void speed(const Vector& speed) {
		a += speed; // / 33.33f;
		lastSpeedChange = glutGet(GLUT_ELAPSED_TIME);
		//cout << "speed\n";
		//cout.flush();
	}

public:
	Robot(int x, int z, SceneObjectHolder * mf) :
			SceneObject(x, z, mf), color(0.0f, 0.0f, 1.0f) {
		phi = 0.0f;
		theta1 = 0.0f;
		v = Vector(0.0f, 0.0f, 0.0f);
		realX = (float) x;
		realZ = (float) z;
	}

	int getX() const {
		return realX;
	}

	int getZ() const {
		return realZ;
	}

	void onDraw() const {
		float ARM_WIDTH = 0.25f;

		glColor3f(color.r, color.g, color.b);
		GLfloat ks[] = { 0.5, 0.5, 0.5, 1.0 };
		glMaterialfv(GL_FRONT, GL_SPECULAR, ks);
		GLfloat kd[] = { color.r, color.g, color.b, 1.0 };
		glMaterialfv(GL_FRONT, GL_DIFFUSE, kd);
		glMaterialf(GL_FRONT, GL_SHININESS, 20);

		glPushMatrix();
		glTranslatef(0.0f, 0.1 * M_UNIT, 0.0f);
		drawCone(0.5f * M_UNIT, 1.0f * M_UNIT);

		glTranslatef(0.0f, 1.0f * M_UNIT, 0.0f);
		drawSphere(ARM_WIDTH * M_UNIT);

		glRotatef(phi, 0.0f, 1.0f, 0.0f);
		glRotatef(theta1, 1.0f, 0.0f, 0.0f);
		drawCylinder(ARM_WIDTH * M_UNIT, 4.0f * M_UNIT);

		glTranslatef(0.0f, 4.0f * M_UNIT, 0.0f);
		drawSphere(ARM_WIDTH * M_UNIT);

		glRotatef(theta2, 1.0f, 0.0f, 0.0f);
		drawCylinder(ARM_WIDTH * M_UNIT, 4.0f * M_UNIT);

		glPopMatrix();
	}

	void onDrawLight() const {
		glPushMatrix();

		glTranslatef(0.0f, 0.1 * M_UNIT, 0.0f);
		glTranslatef(0.0f, 1.0f * M_UNIT, 0.0f);
		glRotatef(phi, 0.0f, 1.0f, 0.0f);
		glRotatef(theta1, 1.0f, 0.0f, 0.0f);
		glTranslatef(0.0f, 4.0f * M_UNIT, 0.0f);
		glRotatef(theta2, 1.0f, 0.0f, 0.0f);

		glTranslatef(0.0f, 4.0f * M_UNIT, 0.0f);
		GLfloat pos[] = { 0.0f, 0.0f, 0.0f, 10.0f };
		glLightfv(GL_LIGHT1, GL_POSITION, pos);
		glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.01f);

		glPopMatrix();
	}

	void addPhi(float v) {
		phi += v;
	}

	void addTheta1(float v) {
		theta1 += v;
	}

	void addTheta2(float v) {
		theta2 += v;
	}

	void up() {
		speed(Vector(0.0f, 0.0f, 2.0f));
	}

	void down() {
		speed(Vector(0.0f, 0.0f, -2.0f));
	}

	void left() {
		speed(Vector(-2.0f, 0.0f, 0.0f));
	}

	void right() {
		speed(Vector(2.0f, 0.0f, 0.0f));
	}

	void debug() {
#if defined(DEBUG)
		cout << "x: " << x << "z: " << z << "\n";
		cout << phi << " " << theta1 << " " << theta2 << "\n";
#endif
	}

	void update(long elapsed) {
		v += a * elapsed / 1000.0f;

		cout << v.length();
		cout << "\n";
		cout.flush();

		realX += v.x * elapsed / 1000.0f;
		realZ += v.z * elapsed / 1000.0f;

		if (realX > WIDTH) {
			realX = WIDTH;
			v.x = 0.0f;
			a.x = 0.0f;
		}
		if (realX < 0) {
			realX = 0;
			v.x = 0.0f;
			a.x = 0.0f;
		}
		if (realZ > WIDTH) {
			realZ = WIDTH;
			v.z = 0.0f;
			a.z = 0.0f;
		}
		if (realZ < 0) {
			realZ = 0;
			v.z = 0.0f;
			a.z = 0.0f;
		}

		v *= powf(0.5f, (float) elapsed / 1000.0f);
	}
};

class Fireball: public SceneObject {
	Vector v;
public:
	Fireball() :
			SceneObject(-1, -1, NULL) {
		hide();
	}

	void showAt(Vector v) {
		this->v = v;

		show();
	}

	void onDraw() const {
		glColor3f(1.0f, 1.0f, 1.0f);

		//GLfloat ks[] = { 0, 0, 0, 1.0 };
		//glMaterialfv(GL_FRONT, GL_SPECULAR, ks);
		//GLfloat kd[] = { 1.0f, 165.0f / 255.0f, 0.0f, 1.0 };
		//glMaterialfv(GL_FRONT, GL_DIFFUSE, kd);
		//glMaterialf(GL_FRONT, GL_SHININESS, 0);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textures[1]);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		glPushMatrix();
		glTranslatef(v.x, v.y, v.z);
		drawSphere(2.0f * M_UNIT);
		glPopMatrix();

		glDisable(GL_TEXTURE_2D);

		glDisable(GL_BLEND);
	}
};

class Region {
	int low;
	int high;
	Color color;
public:
	Region(int low, int high, Color color) :
			low(low), high(high), color(color) {

	}

	Color getColor() const {
		return color;
	}

	float getWeight(int h) {
		float w = (high - low - fabsf(h - high)) / (high - low);
		if (w < 0.0f)
			w = 0.0f;
		return w;
	}
};

const int REGION_COUNT = 5;
Region regions[REGION_COUNT] = { Region(0, 50, Color(57, 97, 171)), Region(51,
		100, Color(208, 195, 157)), Region(101, 150, Color(97, 123, 67)),
		Region(151, 205, Color(157, 150, 162)), Region(206, 256,
				Color(226, 223, 208)) };

class MineField: public SceneObjectHolder {
	float X[WIDTH + 1];
	float Z[WIDTH + 1];
	float Y[WIDTH + 1][WIDTH + 1];
	Vector N[WIDTH + 1][WIDTH + 1];

public:
	Vector getV(int x, int z) const {
		if (x >= 0 && z >= 0 && x <= WIDTH && z <= WIDTH) {
			return Vector(X[x], Y[x][z], Z[z]);
		} else {
			return Vector();
		}
	}

	Vector getNormal(int x, int z) const {
		Vector a, b, c, d, e, f;

		a = getTriV(x - 1, z - 1, x, z);
		b = getTriV(x, z - 1, x, z);
		c = getTriV(x + 1, z, x, z);
		d = getTriV(x, z + 1, x, z);
		e = getTriV(x - 1, z + 1, x, z);
		f = getTriV(x - 1, z, x, z);

		Vector normal =
				(a % b + b % c + c % d + d % e + e % f + f % a).normal();

		return normal;
	}

private:
	float getHeight(int xi, int zi) const {
		float x = (float) xi;
		float z = (float) zi;

		x = (x / WIDTH - 0.5f) * 10000.0f;
		z = (z / WIDTH - 0.5f) * 10000.0f;

		return (-1 * sinf(1 / (x / 2500 + 2.2f)) + sinf(x / 2500 + z / 2500)
				+ 0.5f * sinf(2 * x * x / 2500 / 2500)
				+ 0.25f * sinf(3 * x / 2500 + z * z / 2500 / 2500)
				+ 0.1f * sinf(z * z * z / 2500 / 2500 / 2500) + 2.24934f)
				/ (2.07533f + 2.24934f) * 8.0f;
	}

	Vector getTriV(int x1, int z1, int x2, int z2) const {
		Vector v1 = getV(x1, z1);
		Vector v2 = getV(x2, z2);

		if (v1.isNull()) {
			return Vector();
		} else {
			return v1 - v2;
		}
	}

	void vertexFor(int x, int z) const {
		glTexCoord2f((float) x / WIDTH, (float) z / WIDTH);
		//cout << (float) x / WIDTH << " " << (float) z / WIDTH << "\n";
		glNormal3f(N[x][z].x, N[x][z].y, N[x][z].z);
		glVertex3f(X[x], Y[x][z], Z[z]);
	}

public:

	MineField() {
		for (int x = 0; x <= WIDTH; x++) {
			// 128 egység -> 32 méter, tehát 1 egység 0.25m.
			X[x] = ((float) x / WIDTH - 0.5f) * 32 * M_UNIT;
			Z[x] = (0.5f - (float) x / WIDTH) * 32 * M_UNIT;
		}
		for (int x = 0; x <= WIDTH; x++) {
			for (int z = 0; z <= WIDTH; z++) {
				Y[x][z] = getHeight(x, z);
			}
		}
		for (int z = 0; z <= WIDTH; z++) {
			for (int x = 0; x <= WIDTH; x++) {
				N[x][z] = getNormal(x, z);
			}
		}
	}

	void transformationsFor(int x, int z) {
		Vector v = getV(x, z);
		Vector n = getNormal(x, z);

		glTranslatef(v.x, v.y, v.z);

		Vector y = Vector(0.0f, 1.0f, 0.0f);
		Vector axis = (y % n).normal();

		glRotatef(acosf(n * y) / M_PI * 180.0f, axis.x, axis.y, axis.z);
	}

	void craterAt(int cx, int cz) {
		for (int x = 0; x <= WIDTH; x++) {
			for (int z = 0; z <= WIDTH; z++) {
				float dx = X[x] - X[cx];
				float dz = Z[z] - Z[cz];

				if (dx * dx + dz * dz < CRATER_SIZE * CRATER_SIZE) {
					Y[x][z] -= sqrtf(
							CRATER_SIZE * CRATER_SIZE - (dx * dx + dz * dz));
					N[x][z] = getNormal(x, z);
				}
			}
		}

		for (int x = 0; x <= WIDTH; x++) {
			for (int z = 0; z <= WIDTH; z++) {
				N[x][z] = getNormal(x, z);
			}
		}
	}

	// Az ebben a metódusban leírtakhoz innen merítettem ihletet: http://www.cprogramming.com/discussionarticles/texture_generation.html
	void getColorAt(int x, int z, Color& color) const {
		int c = (int) (Y[x][z] / 8.0f * 255.0f);

		color *= 0;
		for (int i = 0; i < REGION_COUNT; i++) {
			Region& r = regions[i];
			color += r.getColor() * r.getWeight(c);
		}
	}

	void draw() const {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textures[0]);
		glColor3f(1.0f, 1.0f, 1.0f);

		GLfloat ks[] = { 0.0, 0.0, 0.0, 1.0 };
		glMaterialfv(GL_FRONT, GL_SPECULAR, ks);
		GLfloat kd[] = { 1.0, 1.0, 1.0, 1.0 };
		glMaterialfv(GL_FRONT, GL_DIFFUSE, kd);

		glMaterialf(GL_FRONT, GL_SHININESS, 20);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		int MAX_Z = WIDTH; // 128
		int MAX_X = WIDTH; // 128

		int MIN_Z = 0; // 0
		int MIN_X = 0; // 0

		for (int z = MIN_Z; z <= MAX_Z - 1; z++) {
			glBegin(GL_TRIANGLE_STRIP);
			for (int x = MIN_X; x <= MAX_X; x++) {
				vertexFor(x, z);
				vertexFor(x, z + 1);
			}
			glEnd();
		}

		if (drawNormals) {
			glColor3f(0.0f, 1.0f, 0.0f);
			Vector n;
			for (int z = MIN_Z; z <= MAX_Z; z += 8) {
				for (int x = MIN_X; x <= MAX_X; x += 8) {
					n = N[x][z] * 0.1f;
					glBegin(GL_LINES);
					glVertex3f(X[x], Y[x][z], Z[z]);
					glVertex3f(X[x] + n.x, Y[x][z] + n.y, Z[z] + n.z);
					glEnd();
				}
			}
		}

		glDisable(GL_TEXTURE_2D);
	}
};

class Scene {
	MineField mineField;
	Robot robot;
	Fireball fireball;
	SceneObject * target;
	Mine mines[MINES_COUNT];
	int activeMine;
	long mineTick;

	float cameraAngleX;
	float cameraAngleY;
	float cameraDistance;

	void render() const {
		mineField.draw();

		for (int i = 0; i < MINES_COUNT; i++) {
			mines[i].draw();
		}

		robot.draw();
		fireball.draw();
	}

	void renderLights() const {
		mines[activeMine].drawLight();
		robot.drawLight();
	}

public:
	Scene() :
			robot(64, 64, &mineField), fireball() {
		cameraAngleX = CAMERA_ANGLEX;
		cameraAngleY = CAMERA_ANGLEY;
		cameraDistance = CAMERA_DISTANCE;

		mines[0] = Mine(64 + 4 * 4, 64, &mineField);
		mines[1] = Mine(64 - 8 * 4, 64, &mineField);
		mines[2] = Mine(64, 64 + 5 * 4, &mineField);
		mines[3] = Mine(64, 64 + 7 * 4, &mineField);
		mines[4] = Mine(64, 64 + 9 * 4, &mineField);

		getRobot().addPhi(-310);
		getRobot().addTheta1(30.0f);
		getRobot().addTheta2(120.0f);

		target = &robot;
	}

	MineField& getMineField() {
		return mineField;
	}

	Robot& getRobot() {
		return robot;
	}

	void addToCameraAngleY(float v) {
		cameraAngleY += v;
#if defined(DEBUG)
		cout << "angleY " << cameraAngleY << "\n";
		cout.flush();
#endif
	}

	void addToCameraAngleX(float v) {
		cameraAngleX += v;
#if defined(DEBUG)
		cout << "angleX " << cameraAngleX << "\n";
		cout.flush();
#endif
	}

	void addToCameraDistance(float v) {
		cameraDistance += v;
		if (cameraDistance < CAMERA_DISTANCE_MIN)
			cameraDistance = CAMERA_DISTANCE_MIN;
		if (cameraDistance > CAMERA_DISTANCE_MAX)
			cameraDistance = CAMERA_DISTANCE_MAX;
	}

	void detonate() {
		Mine& mine = mines[0];

		if (mine.isVisible()) {
			mine.hide();
			fireball.showAt(mineField.getV(mine.getX(), mine.getZ()));

			mineField.craterAt(mine.getX(), mine.getZ());
			target = &mine;
		} else {
			fireball.hide();
		}
	}

	void renderWorld() const {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45, (float) screenWidth / (float) screenHeight, 20, 70);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		Vector distance = getVFromSC(-cameraDistance,
				-cameraAngleY / 180.f * M_PI, -cameraAngleX / 180.f * M_PI);
		Vector v = mineField.getV(target->getX(), target->getZ());
		Vector from = v - distance;
		gluLookAt(from.x, from.y, from.z, v.x, v.y, v.z, 0, 1, 0);

		renderLights();
		render();
	}

	void renderMinimap() const {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		float dim = 32.0f * M_UNIT;
		glOrtho(-dim / 2.0f, dim / 2.0f, -dim / 2.0f, dim / 2.0f, 0.01f, 1000);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(0.0f, 20.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f);

		renderLights();
		render();
	}

	void update(long elapsed) {
		mineTick += elapsed;
		if (mineTick >= 1000) {
			mineTick -= 1000;
			activeMine = (activeMine + 1) % MINES_COUNT;
		}

		robot.update(elapsed);
	}
};

Scene scene;

void onInitialization() {
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	glEnable(GL_NORMALIZE);

	GLfloat ka[] = { 0.2, 0.2, 0.2, 1.0 };
	glMaterialfv(GL_FRONT, GL_AMBIENT, ka);

	GLfloat I0[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat pos[] = { 10, 20, 10, 0.0 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, I0);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);

	GLfloat I1[] = { 1.0, 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT1, GL_DIFFUSE, I1);
	GLfloat I2[] = { 1.0, 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT2, GL_DIFFUSE, I2);

	glGenTextures(2, textures);

	glBindTexture(GL_TEXTURE_2D, textures[0]);

	unsigned char image[WIDTH * WIDTH * 3];
	for (int z = 0; z < WIDTH; z++) {
		for (int x = 0; x < WIDTH; x++) {
			Color color;
			scene.getMineField().getColorAt(x, z, color);

			image[z * WIDTH * 3 + 3 * x + 0] = color.r;
			image[z * WIDTH * 3 + 3 * x + 1] = color.g;
			image[z * WIDTH * 3 + 3 * x + 2] = color.b;
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, WIDTH, 0, GL_RGB,
			GL_UNSIGNED_BYTE, &image[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, textures[1]);

	int width = 128, height = 128;
	unsigned char image2[width][height][4];
	for (int z = 0; z < height; z++) {
		for (int x = 0; x < width; x++) {
			image2[z][x][0] = 255;
			image2[z][x][1] = 165; //(x % 2 == 0 || z % 2 == 0) ? 165 : 140;
			image2[z][x][2] = 0;
			image2[z][x][3] = 255 / 2;
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, image2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void onDisplay() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, screenWidth, screenHeight);
	scene.renderWorld();

	glClear(GL_DEPTH_BUFFER_BIT);

	glViewport(450, 0, 150, 150);
	scene.renderMinimap();

	glutSwapBuffers();
}

int yOrigin = -1;
int xOrigin = -1;
bool rightButton = false;
bool leftButton = true;

void onMouse(int button, int state, int x, int y) {
	// only start motion if the left button is pressed
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_UP) {
			leftButton = false;
		} else { // state = GLUT_DOWN
			leftButton = true;
			xOrigin = x;
			yOrigin = y;
		}
	} else if (button == GLUT_RIGHT_BUTTON) {
		if (state == GLUT_UP) {
			rightButton = false;
		} else {
			rightButton = true;
			yOrigin = y;
		}
	}
}

void mouseMove(int x, int y) {
	// this will only be true when the left button is down
	if (leftButton) {
		scene.addToCameraAngleY((x - xOrigin) * 0.05f);
		scene.addToCameraAngleX((y - yOrigin) * 0.05f);
	}
	if (rightButton) {
		scene.addToCameraDistance((y - yOrigin) * 0.05f);
	}
}

void onKeyboard(unsigned char key, int x, int y) {
	if (key == 'q') {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else if (key == 'f') {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	} else if (key == 'n') {
		drawNormals = !drawNormals;
	} else if (key == '1') {
		scene.getRobot().addPhi(10.0f);
	} else if (key == '2') {
		scene.getRobot().addPhi(-10.0f);
	} else if (key == '3') {
		scene.getRobot().addTheta1(10.0f);
	} else if (key == '4') {
		scene.getRobot().addTheta1(-10.0f);
	} else if (key == '5') {
		scene.getRobot().addTheta2(10.0f);
	} else if (key == '6') {
		scene.getRobot().addTheta2(-10.0f);
	} else if (key == ' ') {
		scene.detonate();
	} else if (key == 'u') {
		scene.getRobot().right();
	} else if (key == 'j') {
		scene.getRobot().left();
	} else if (key == 'k') {
		scene.getRobot().up();
	} else if (key == 'h') {
		scene.getRobot().down();
	} else if (key == 'b') {
		scene.getRobot().debug();
	}
}

long time = 0;

void onIdle() {
	long current = glutGet(GLUT_ELAPSED_TIME);
	scene.update(current - time);
	time = current;

	glutPostRedisplay();
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

	glutMotionFunc(mouseMove);

	glutMainLoop(); // Esemenykezelo hurok

	return 0;
}

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
#include <stdio.h>
#endif

float min(float a, float b) {
	return (a < b) ? a : b;
}

const int screenWidth = 600;
const int screenHeight = 600;
const float STEPPING = 0.005f;
const float AMBIENT = 0.2f;

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
	Vector normal() const {
		return (*this) / length();
	}
	float length() const {
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

#if defined(DEBUG)
	friend std::ostream& operator<<(std::ostream& output, const Color& c) {
		output << c.r << " " << c.g << " " << c.b << "\n";
		output.flush();
		return output;
	}
#endif
};

class Ray {
	Vector point;
	Vector v;
public:
	Ray(Vector point, Vector v) {
		this->point = point;
		this->v = v.normal();
	}

	Vector getPoint() const {
		return point;
	}

	Vector getV() const {
		return v;
	}
};

class Light {
	Vector direction;
	Color color;
public:
	Light(Vector direction, Color color) :
			direction(direction), color(color) {
	}

	Vector getDirection() const {
		return direction;
	}

	Color getColor() const {
		return color;
	}
};

class PointOfView {
	Vector point;
public:
	PointOfView(float x, float y, float z) :
			point(x, y, z) {
	}

	Vector getPoint() const {
		return point;
	}
};

class SceneObject {
public:
	virtual Color getColor(const Vector& p) = 0;
	virtual float intersect(const Ray& ray) = 0;
	virtual float intersect(const Ray& ray, float t1, float t2) = 0;
	virtual Vector getNormal(const Vector& intersect) = 0;

	Vector getReflectDir(const Vector& in, const Vector& p) {
		Vector v = in.normal();
		Vector normal = getNormal(p);

		return v - normal * (normal * v) * 2;
	}
};

struct Bounds {
	float t1, t2;

	Bounds(float t1, float t2) :
			t1(t1), t2(t2) {
	}
};

class Atom {
	int type;
	Vector origo;
public:
	Atom(int type, Vector origo) :
			type(type), origo(origo) {

	}

	Vector getOrigo() const {
		return origo;
	}

	void rotateOrigoAroundY(float degree) {
		float rad = degree / 180.0f * M_PI;
		float z = origo.z;
		float x = origo.x;
		origo.z = z * cosf(rad) - x * sinf(rad);
		origo.x = z * sinf(rad) + x * cosf(rad);
	}

	float getRadius() const {
		float R = 0.0f;
		if (type == 1) {
			R = 38.0f;
		} else if (type == 6) {
			R = 77.0f;
		} else if (type == 7) {
			R = 75.0f;
		}
		return R / 75.0f;
	}

	Color getColor() const {
		if (type == 1) {
			return Color(1.0f, 1.0f, 1.0f);
		} else if (type == 6) {
			return Color(0.5f, 0.5f, 0.5f);
		} else if (type == 7) {
			return Color(1.0f, 0.0f, 0.0f);
		}

		return Color(0.0f, 0.0f, 0.0f);
	}
};

const int ATOMS_COUNT = 10;
Atom atoms[] = { Atom(7, Vector(1.7212f, -0.2736f, -0.0001f)), Atom(6,
		Vector(-0.6922f, 0.641f, -0.0001f)), Atom(6,
		Vector(-1.7086f, -0.4876f, 0.0f)), Atom(6,
		Vector(0.6796f, 0.1202f, 0.0002f)), Atom(1,
		Vector(-0.8485f, 1.2724f, 0.8814f)), Atom(1,
		Vector(-0.8483f, 1.272f, -0.8818f)), Atom(1,
		Vector(-1.5944f, -1.123f, -0.885f)), Atom(1,
		Vector(-1.5946f, -1.1227f, 0.8853f)), Atom(1,
		Vector(-2.7259f, -0.0834f, -0.0002f)), Atom(1,
		Vector(2.6666f, -0.6313f, -0.0004f)) };

class Sphere {
	Vector origo;
	float radius;
public:
	Sphere() {

	}

	Sphere(Vector origo, float radius) {
		this->origo = origo;
		this->radius = radius;
	}

	Bounds getBounds(const Ray& ray) const {
		double dx = ray.getV().x;
		double dy = ray.getV().y;
		double dz = ray.getV().z;
		double x0 = ray.getPoint().x;
		double y0 = ray.getPoint().y;
		double z0 = ray.getPoint().z;
		double cx = origo.x;
		double cy = origo.y;
		double cz = origo.z;
		double R = radius;
		double a = dx * dx + dy * dy + dz * dz;
		double b = 2 * dx * (x0 - cx) + 2 * dy * (y0 - cy) + 2 * dz * (z0 - cz);
		double c = cx * cx + cy * cy + cz * cz + x0 * x0 + y0 * y0 + z0 * z0
				- 2 * (cx * x0 + cy * y0 + cz * z0) - R * R;
		double d = b * b - 4 * a * c;
		if (d < 0) {
			return Bounds(-1.0f, -1.0f);
		}

		float t1 = (-1.0f * b - sqrtf(d)) / (2.0f * a);
		if (t1 < 0.0f)
			t1 = 0.0f;
		float t2 = (-1.0f * b + sqrtf(d)) / (2.0f * a);

		return Bounds(t1, t2);
	}

	Sphere merge(const Sphere& other) const {
		const Sphere& bigger = (radius > other.radius) ? (*this) : other;
		const Sphere& smaller = (radius < other.radius) ? (*this) : other;

		if ((origo - other.origo).length() + smaller.radius < bigger.radius) {
			return bigger;
		}

		Vector v = (origo - other.origo).normal();
		Vector start = origo + v * radius;
		Vector end = other.origo - v * other.radius;

		return Sphere((start + end) / 2.0f, (start - end).length() / 2);
	}
};

class Molecule: public SceneObject {
	Sphere bounding;
public:
	Molecule() {
		bounding = Sphere(atoms[0].getOrigo(), atoms[0].getRadius());
		for (int i = 1; i < 10; i++) {
			bounding = bounding.merge(
					Sphere(atoms[i].getOrigo(), atoms[i].getRadius()));
		}
	}

	float W(float r) {
		if (r <= 1.0f) {
			return 1
					- (4.0f * powf(r, 6.0f) - 17.0f * powf(r, 4.0f)
							+ 22.0f * powf(r, 2.0f)) / 9.0f;
		} else {
			return 0.0f;
		}
	}

	Color getColor(const Vector& p) {
		Color c(0.0f, 0.0f, 0.0f);
		float sum = 0.0f;
		for (int j = 0; j < ATOMS_COUNT; j++) {
			if ((p - atoms[j].getOrigo()).length() / atoms[j].getRadius() < 1) {
				float r2 = (powf(p.x - atoms[j].getOrigo().x, 2.0f)
						+ powf(p.y - atoms[j].getOrigo().y, 2.0f)
						+ powf(p.z - atoms[j].getOrigo().z, 2.0f))
						/ powf(atoms[j].getRadius(), 2.0f);
				sum += (-12.0f * r2 * r2 + 34.0f * r2 - 22.0f) * 2.0f / 9.0f
						/ powf(atoms[j].getRadius(), 2.0f);
			}
		}
		for (int j = 0; j < ATOMS_COUNT; j++) {
			if ((p - atoms[j].getOrigo()).length() / atoms[j].getRadius() < 1) {
				float r2 = (powf(p.x - atoms[j].getOrigo().x, 2.0f)
						+ powf(p.y - atoms[j].getOrigo().y, 2.0f)
						+ powf(p.z - atoms[j].getOrigo().z, 2.0f))
						/ powf(atoms[j].getRadius(), 2.0f);
				c += atoms[j].getColor()
						* (-12.0f * r2 * r2 + 34.0f * r2 - 22.0f) * 2.0f / 9.0f
						/ powf(atoms[j].getRadius(), 2.0f) / sum;
			}
		}
		return c;
	}

	float L(const Vector& x) {
		float A = 0.25f;
		float l = -A;
		for (int j = 0; j < ATOMS_COUNT; j++) {
			float w = W(
					(x - atoms[j].getOrigo()).length() / atoms[j].getRadius());
			if (w > 0.0f) {
				l += w;
			}
		}
		return l;
	}

	float intersect(const Ray& ray) {
		Bounds b = bounding.getBounds(ray);
		if (b.t1 >= 0) {
			return intersect(ray, b.t1, b.t2);
		} else {
			return -1.0f;
		}
	}

	float intersect(const Ray& ray, float t1, float t2) {
		return intersect(ray, t1, t2, NULL);
	}

	float intersect(const Ray& ray, float t1, float t2,
			SceneObject * lastIntersected) {
		Bounds b = bounding.getBounds(ray);
		if (b.t1 >= t1) {
			t1 = b.t1;
		}
		if (b.t2 <= t2) {
			t2 = b.t2;
		}

		float t = t1;
		float l;
		do {
			l = L(ray.getPoint() + (ray.getV() * t));
			t += STEPPING;
		} while (l < 0.005f && t < t2);

		if (l <= 0.0f) {
			return -1.0f;
		}

		if (fabsf(t - t1) < 2 * STEPPING && lastIntersected == this) {
			// megvárjuk míg átmegyünk rajta
			do {
				l = L(ray.getPoint() + (ray.getV() * t));
				t += STEPPING;
			} while (l >= 0.005f && t < t2);
			return intersect(ray, t, t2, NULL);
		} else {
			return t;
		}
	}

	Vector getNormal(const Vector& p) {
		float r2;
		Vector n = Vector(0.0f, 0.0f, 0.0f);
		for (int j = 0; j < ATOMS_COUNT; j++) {
			if (W((p - atoms[j].getOrigo()).length() / atoms[j].getRadius())
					> 0.0f) {
				r2 = (powf(p.x - atoms[j].getOrigo().x, 2.0f)
						+ powf(p.y - atoms[j].getOrigo().y, 2.0f)
						+ powf(p.z - atoms[j].getOrigo().z, 2.0f))
						/ powf(atoms[j].getRadius(), 2.0f);
				n += (p - atoms[j].getOrigo())
						* (-12.0f * r2 * r2 + 34.0f * r2 - 22.0f) * -2.0f / 9.0f
						/ powf(atoms[j].getRadius(), 2.0f);
			}
		}
		return n.normal();
	}
};

class Plane: public SceneObject {
	Vector point; // a sík egy pontja
	Vector normal; // a sík normálvektora
public:
	Plane(Vector p, Vector n) {
		point = p;
		normal = n;
	}

	float intersect(const Ray& ray) {
		float d = normal * ray.getV();
		{
			if (d == 0.0) {
				return -1.0;
			}
			double nx = normal.x;
			double ny = normal.y;
			double nz = normal.z;
			double Psx = point.x;
			double Psy = point.y;
			double Psz = point.z;
			double dvx = ray.getV().x;
			double dvy = ray.getV().y;
			double dvz = ray.getV().z;
			double Pex = ray.getPoint().x;
			double Pey = ray.getPoint().y;
			double Pez = ray.getPoint().z;
			double t = -1.0
					* ((nx * Pex - nx * Psx + ny * Pey - ny * Psy + nz * Pez
							- nz * Psz) / (nx * dvx + ny * dvy + nz * dvz));
			if (t > 0.01f)
				return t;
			if (t > 0)
				return 0;
			return -1;
		}
	}

	float intersect(const Ray& ray, float t1, float t2) {
		return intersect(ray);
	}

	Color getColor(const Vector& p) {
		float a = fabsf(p.z);
		float b = fabsf(p.x);

		if ((int) roundf(a) % 2 == 0 && (int) roundf(b) % 2 == 0) {
			return Color(1.0f, 1.0f, 1.0f);
		} else if ((int) roundf(a) % 2 == 1 && (int) roundf(b) % 2 == 1) {
			return Color(1.0f, 1.0f, 1.0f);
		} else {
			return Color(0.0f, 0.0f, 0.0f);
		}

		//return Color(0.0f, 0.0f, 0.0f);
	}

	Vector getNormal(const Vector& v) {
		return normal;
	}
};

int deg;

class Scene {
	Color image[screenWidth * screenHeight];
	Molecule mol;
	Light light;
	Plane plane;

	Color trace(const Ray& ray, int iterat) {
		Color color = Color(0.52f, 0.8f, 1.0f);
		double t;
		SceneObject * obj = NULL;

		if ((t = mol.intersect(ray)) >= 0) {
			obj = &mol;
		} else if ((t = plane.intersect(ray)) >= 0) {
			obj = &plane;
		}

		if (obj != NULL) {
			Vector intersection = ray.getPoint() + ray.getV() * t;
			color = obj->getColor(intersection);
			Vector normal = obj->getNormal(intersection);
			if (obj == &plane) {
				color *= min((ray.getV() * -1.0f) * normal * 10.0f, 1.0f);
			}
			Ray toLight(intersection + normal * STEPPING,
					light.getDirection().normal() * -1.0f);

			float shade = toLight.getV() * normal;
			if (shade < 0) {
				shade = 0;
			}

			bool inShadow = false;
			if (mol.intersect(toLight, 0, 9999999.99f, obj) >= 0) {
				shade *= 0.15f;
				inShadow = true;
			}

			color *= (AMBIENT + (1.0f - AMBIENT) * shade);

			// fenti volt a diffúz szín.

			float cosPhi = (ray.getV() * -1.0f).normal()
					* obj->getReflectDir(light.getDirection(), intersection);
			if (cosPhi < 0.0f)
				cosPhi = 0.0f;

			if (!inShadow) {
				color += light.getColor() * powf(cosPhi, 20.0f);
			}
		}

		return color.normal();
	}

public:
	Scene() :
			light(Vector(0.0f, -10.0f, 15.0f), Color(1.0f, 1.0f, 1.0f)), plane(
					Vector(0.0f, -4.1f, 0.0f), Vector(0.0f, 1.0f, 0.0f)) {
	}

	void init() {
		float mul = 4.0f;
		float minX = -1.0f * mul;
		float maxX = 1.0f * mul;
		float minY = -1.0f * mul;
		float maxY = 1.0f * mul;
		float near_ = -2.9f;

		PointOfView pov(0.0f, 0.0f, near_ - maxY);

		for (int i = 0; i < screenHeight; i++) {
			for (int j = 0; j < screenWidth; j++) {
				Vector p0(((float) j / screenWidth) * (maxX - minX) + minX,
						((float) i / screenHeight) * (maxY - minY) + minY,
						near_);
				Ray ray(p0, (p0 - pov.getPoint()).normal());
				image[i * screenWidth + j] = trace(ray, 0);
			}
		}

		FILE* f;
		char fname[32];
		sprintf(fname, "render-%04d.ppm", deg);
		f = fopen(fname, "w");
		fprintf(f, "P3\n%d %d\n255\n ", screenWidth, screenHeight);

		for (int i = screenHeight - 1; i >= 0; i--) {
			for (int j = 0; j < screenWidth; j++) {
				fprintf(f, "%d ", (int) (image[i * screenWidth + j].r * 255));
				fprintf(f, "%d ", (int) (image[i * screenWidth + j].g * 255));
				fprintf(f, "%d ", (int) (image[i * screenWidth + j].b * 255));
			}
#if defined(DEBUG)
			//std::cout << i << "\n";
			//std::cout.flush();
#endif
		}

		fclose(f);
	}

	void render() {
		glDrawPixels(screenWidth, screenHeight, GL_RGB, GL_FLOAT, image);
	}
};

Scene scene;

void onInitialization() {
	glViewport(0, 0, screenWidth, screenHeight);

	for (int d = 0; d < 360; d++) {
		deg = d;
		for (int i = 0; i < ATOMS_COUNT; i++) {
			Atom& a = atoms[i];
			a.rotateOrigoAroundY(1);
		}
		scene.init();
#if defined(DEBUG)
		std::cout << deg << "\n";
		std::cout.flush();
#endif
	}
}

void onDisplay() {
	scene.render();
	glutSwapBuffers();
}

void onMouse(int button, int state, int x, int y) {
}

void onKeyboard(unsigned char key, int x, int y) {
}

void onIdle() {
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

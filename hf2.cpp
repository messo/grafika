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
const int RESOLUTION = 20;
const long NO_LAST_CLICK_AT = -1;

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
	Vector& operator-=(const Vector& v) {
		x -= v.x;
		y -= v.y;
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
	void rotate(const Vector& c, float angle) {
		(*this) -= c;

		float rad = angle / 180 * M_PI;
		float newx = x * cosf(rad) - y * sinf(rad);
		float newy = x * sinf(rad) + y * cosf(rad);
		x = newx;
		y = newy;

		(*this) += c;
	}
};

// A következõ kódsorok apró módosításokkal a(z)
// http://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland oldalról származnak
typedef int OutCode;

const OutCode INSIDE = 0;
const OutCode LEFT = 1;
const OutCode RIGHT = 2;
const OutCode BOTTOM = 4;
const OutCode TOP = 8;

OutCode ComputeOutCode(const Vector& p, const Vector& topLeft,
		const Vector& bottomRight) {
	OutCode code;

	code = INSIDE;

	if (p.x < topLeft.x)
		code |= LEFT;
	else if (p.x > bottomRight.x)
		code |= RIGHT;
	if (p.y > bottomRight.y)
		code |= BOTTOM;
	else if (p.y < topLeft.y)
		code |= TOP;

	return code;
}

bool isCohenSutherlandLineClipping(Vector p0, Vector p1, const Vector& topLeft,
		const Vector& bottomRight) {

	OutCode outcode0 = ComputeOutCode(p0, topLeft, bottomRight);
	OutCode outcode1 = ComputeOutCode(p1, topLeft, bottomRight);
	bool accept = false;

	while (true) {
		if (!(outcode0 | outcode1)) {
			accept = true;
			break;
		} else if (outcode0 & outcode1) {
			break;
		} else {
			float x = 0.0f, y = 0.0f;

			OutCode outcodeOut = outcode0 ? outcode0 : outcode1;

			Vector dv = p1 - p0;

			if (outcodeOut & TOP) {
				x = p0.x + dv.x * (topLeft.y - p0.y) / dv.y;
				y = topLeft.y;
			} else if (outcodeOut & BOTTOM) {
				x = p0.x + dv.x * (bottomRight.y - p0.y) / dv.y;
				y = bottomRight.y;
			} else if (outcodeOut & RIGHT) {
				y = p0.y + dv.y * (bottomRight.x - p0.x) / dv.x;
				x = bottomRight.x;
			} else if (outcodeOut & LEFT) {
				y = p0.y + dv.y * (topLeft.x - p0.x) / dv.x;
				x = topLeft.x;
			}

			if (outcodeOut == outcode0) {
				p0.x = x;
				p0.y = y;
				outcode0 = ComputeOutCode(p0, topLeft, bottomRight);
			} else {
				p1.x = x;
				p1.y = y;
				outcode1 = ComputeOutCode(p1, topLeft, bottomRight);
			}
		}
	}

	return accept;
}
// Idáig származtak a fenti oldalról a kódok nagy része.

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

Vector convertScreenToWorld(const Vector& v, float visible) {
	float x = v.x / screenWidth * visible + (1 - visible) / 2;
	float y = v.y / screenHeight * visible + (1 - visible) / 2;
	return Vector(x, y);
}

double tAtControlPoint[100];

class Spline {
	Vector controlPoints[100];
	Vector v[100];
	int controlPointsCount;

	Vector getPointOnSegment(double t, int i) const {
		// Az itt található, vektorizáláshoz használt algoritmus (és a következõ függvény által tartalmazott
		// v értékeinek kiszámolásához használt algoritmus) nagy része a
		// Szirmay-Kalos László, Antal György, Csonka Ferenc: "Háromdimenziós grafika, amimáció és játékfejlesztés"
		// címû könyvbõl származik.

		double aix, aiy, bix, biy;
		Vector ci, di;

		double dt = tAtControlPoint[i + 1] - tAtControlPoint[i];
		Vector vsum = v[i + 1] + v[i];
		Vector dc = controlPoints[i + 1] - controlPoints[i];

		aix = (vsum.x - dc.x * 2 / dt) / dt / dt;
		aiy = (vsum.y - dc.y * 2 / dt) / dt / dt;
		bix = (dc.x * 3 / dt - v[i + 1].x - v[i].x * 2) / dt;
		biy = (dc.y * 3 / dt - v[i + 1].y - v[i].y * 2) / dt;
		ci = v[i];
		di = controlPoints[i];

		double ti = tAtControlPoint[i];

		double fx = aix * pow(t - ti, 3) + bix * pow(t - ti, 2)
				+ ci.x * (t - ti) + di.x;
		double fy = aiy * pow(t - ti, 3) + biy * pow(t - ti, 2)
				+ ci.y * (t - ti) + di.y;

		Vector f(fx, fy);

		return f;
	}

	Vector vAtControlPoint(int i) {
		return ((controlPoints[i] - controlPoints[i - 1])
				/ (tAtControlPoint[i] - tAtControlPoint[i - 1])
				+ (controlPoints[i + 1] - controlPoints[i])
						/ (tAtControlPoint[i + 1] - tAtControlPoint[i])) * 0.5f;
	}

public:
	Spline() {
		controlPointsCount = 0;
		v[0] = Vector(0.0f, 0.0f);
		v[1] = Vector(0.0f, 0.0f);
	}

	void addPoint(const Vector& p) {
		if (controlPointsCount == 100)
			return;

		controlPoints[controlPointsCount] = p;
		if (controlPointsCount >= 2) {
			int i = controlPointsCount - 1;
			v[i] = vAtControlPoint(i);

			v[controlPointsCount] = Vector(0.0f, 0.0f);
		}
		controlPointsCount++;
	}

	int getControlPointsCount() const {
		return controlPointsCount;
	}

	bool intersect(const Vector& tl, const Vector& br) {
		Vector last;
		bool first = true;

		for (int i = 0; i < (controlPointsCount - 1); i++) {
			double dt = (tAtControlPoint[i + 1] - tAtControlPoint[i])
					/ RESOLUTION;
			for (double t = tAtControlPoint[i]; t < tAtControlPoint[i + 1]; t +=
					dt) {
				Vector f = getPointOnSegment(t, i);

				if (!first) {
					if (isCohenSutherlandLineClipping(last, f, tl, br)) {
						return true;
					}
				}

				first = false;
				last = f;
			}
		}

		if (controlPointsCount == 1) {
			return tl.y <= controlPoints[0].y && controlPoints[0].y <= br.y
					&& tl.x <= controlPoints[0].x && controlPoints[0].x <= br.x;
		}

		return false;
	}

	void move(const Vector& dv) {
		for (int i = 0; i < controlPointsCount; i++) {
			controlPoints[i] += dv;
		}
	}

	void rotate(const Vector& c, float angle) {
		for (int i = 0; i < controlPointsCount; i++) {
			controlPoints[i].rotate(c, angle);
		}

		for (int i = 1; i < controlPointsCount - 1; i++) {
			v[i] = vAtControlPoint(i);
		}
	}

	void draw() const {
		if (controlPointsCount >= 2) {
			glBegin(GL_LINE_STRIP);
			for (int i = 0; i < (controlPointsCount - 1); i++) {
				double step = ((tAtControlPoint[i + 1] - tAtControlPoint[i]))
						/ RESOLUTION;
				for (double t = tAtControlPoint[i]; t < tAtControlPoint[i + 1];
						t += step) {
					Vector f = getPointOnSegment(t, i);
					glVertex2f(f.x, f.y);
				}
			}
			glVertex2f(controlPoints[controlPointsCount - 1].x,
					controlPoints[controlPointsCount - 1].y);
			glEnd();
		} else if (controlPointsCount == 1) {
			glBegin(GL_POINTS);
			glVertex2f(controlPoints[0].x, controlPoints[0].y);
			glEnd();
		}
	}
};

const Color colors[10] = { Color(0.0f, 0.0f, 0.0f), Color(0.0078f, 0.2941f,
		0.9921f), Color(0.9921f, 0.0078f, 0.8705f), Color(0.8705f, 0.5411f,
		0.0078f), Color(0.0078f, 0.8705f, 0.8274f), Color(0.8705f, 0.0078f,
		0.3372f), Color(0.9529f, 0.8862f, 0.4745f), Color(0.0f, 1.0f, 0.0f),
		Color(0.1176f, 0.6f, 0.2588f), Color(0.4705f, 0.1372f, 0.3921f) };

class Application {
	int mode;
	float visible;
	Spline splines[10];
	int splineCount;
	int currentSpline;
	bool moving;
	int fromx, fromy;
	bool rotating;
	int rotFromx, rotFromy;
	int selectedSplice;

	int selectSplice(int x, int y) {
		Vector tl = convertScreenToWorld(Vector(x - 5, y - 5), visible);
		Vector br = convertScreenToWorld(Vector(x + 5, y + 5), visible);
		for (int i = splineCount - 1; i >= 0; i--) {
			if (splines[i].intersect(tl, br)) {
				return i;
			}
		}

		return -1;
	}

public:
	const static int MODE_EDIT = 1;
	const static int MODE_SELECTION = 2;

	Application() {
		visible = 0.078f;
		splineCount = 0;
		currentSpline = 0;
		moving = false;
		rotating = false;

		tAtControlPoint[0] = 1.0;
		tAtControlPoint[1] = 2.0;
		for (int i = 2; i < 100; i++) {
			tAtControlPoint[i] = tAtControlPoint[i - 1]
					+ tAtControlPoint[i - 2];
		}
	}

	void init() {
		glLoadIdentity();
		gluOrtho2D((1.0 - visible) / 2, (1.0 + visible) / 2,
				(1.0 + visible) / 2, (1.0 - visible) / 2);
	}

	void setMode(int mode) {
		this->mode = mode;
	}

	int getMode() const {
		return this->mode;
	}

	void addNewPoint(int x, int y) {
		if (currentSpline == 10)
			return;

		if (splineCount <= currentSpline) {
			splineCount = currentSpline + 1;
		}

		Vector wv = convertScreenToWorld(Vector(x, y), visible);

		splines[currentSpline].addPoint(wv);
	}

	void finishSpline() {
		if (currentSpline < 10) {
			currentSpline++;
		}
	}

	void zoom() {
		visible *= 1.1f;
		if (visible > 1.0f)
			visible = 1.0f;

		glLoadIdentity();
		gluOrtho2D((1.0 - visible) / 2, (1.0 + visible) / 2,
				(1.0 + visible) / 2, (1.0 - visible) / 2);
	}

	bool isMoving() const {
		return moving;
	}

	void moveFrom(int x, int y) {
		fromx = x;
		fromy = y;

		int i = selectSplice(x, y);
		if (i != -1) {
			selectedSplice = i;
			moving = true;
		}
	}

	void moveTo(int x, int y) {
		if (!moving)
			throw "Nem volt mozgatás!";

		Vector dv = convertScreenToWorld(Vector(x, y), visible)
				- convertScreenToWorld(Vector(fromx, fromy), visible);

		splines[selectedSplice].move(dv);
		moving = false;
	}

	bool isRotating() const {
		return rotating;
	}

	void rotateFrom(int x, int y) {
		rotFromx = x;
		rotFromy = y;
		int i = selectSplice(x, y);
		if (i != -1) {
			selectedSplice = i;
			rotating = true;
		}
	}

	void rotateTo(int x, int y) {
		if (!rotating)
			throw "Nem volt forgatás!";

		int angle = x - rotFromx;

		Vector center = convertScreenToWorld(Vector(rotFromx, rotFromy),
				visible);
		splines[selectedSplice].rotate(center, angle);
		rotating = false;
	}

	void draw() const {
		glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (int i = 0; i < splineCount; i++) {
			glColor3f(colors[i].r, colors[i].g, colors[i].b);
			splines[i].draw();
		}
	}
};

Application application;
long lastClickAt = NO_LAST_CLICK_AT;
int lastClickX;
int lastClickY;

void onInitialization() {
	glViewport(0, 0, screenWidth, screenHeight);

	application.init();
}

void onDisplay() {
	application.draw();

	glutSwapBuffers();
}

void onMouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN
			&& application.getMode() == Application::MODE_EDIT) {
		long clickAt = glutGet(GLUT_ELAPSED_TIME);

		if (lastClickAt != NO_LAST_CLICK_AT && clickAt - lastClickAt <= 500
				&& x == lastClickX && y == lastClickY) {
			application.finishSpline();
			lastClickAt = NO_LAST_CLICK_AT;
		} else {
			application.addNewPoint(x, y);
			lastClickAt = clickAt;
			lastClickX = x;
			lastClickY = y;
		}

		glutPostRedisplay();
	} else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN
			&& application.getMode() == Application::MODE_SELECTION) {
		if (!application.isMoving()) {
			application.moveFrom(x, y);
			glutPostRedisplay();
		}
	} else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP
			&& application.getMode() == Application::MODE_SELECTION) {
		if (application.isMoving()) {
			application.moveTo(x, y);
			glutPostRedisplay();
		}
	} else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN
			&& application.getMode() == Application::MODE_SELECTION) {
		if (!application.isRotating()) {
			application.rotateFrom(x, y);
			glutPostRedisplay();
		}
	} else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP
			&& application.getMode() == Application::MODE_SELECTION) {
		if (application.isRotating()) {
			application.rotateTo(x, y);
			glutPostRedisplay();
		}
	}
}

void onKeyboard(unsigned char key, int x, int y) {
	if (key == 'e') {
		application.setMode(Application::MODE_EDIT);
	} else if (key == 'p') {
		application.setMode(Application::MODE_SELECTION);
	} else if (key == 'z') {
		application.zoom();
		glutPostRedisplay();
	}
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

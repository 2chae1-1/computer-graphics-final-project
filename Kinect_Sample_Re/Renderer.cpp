#include "Renderer.h"
#undef scale
#include <map>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void draw_center(void)
{
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f); /* R */
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.2f, 0.0f, 0.0f);
	glEnd();
	glRasterPos3f(0.2f, 0.0f, 0.0f);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'x');

	glBegin(GL_LINES);
	glColor3f(0.0f, 1.0f, 0.0f); /* G */
	glVertex3f(0.0f, 0.2f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();
	glRasterPos3f(0.0f, 0.2f, 0.0f);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'y');

	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 1.0f); /* B */
	glVertex3f(0.0f, 0.0f, -0.2f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();
	glRasterPos3f(0.0f, 0.0f, -0.2f);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'z');
}

void idle() {
	static GLuint previousClock = glutGet(GLUT_ELAPSED_TIME);
	static GLuint currentClock = glutGet(GLUT_ELAPSED_TIME);
	static GLuint deltaT;

	currentClock = glutGet(GLUT_ELAPSED_TIME);
	deltaT = currentClock - previousClock;
	if (deltaT < 1000.0 / 20.0) { return; }
	else { previousClock = currentClock; }

	glutPostRedisplay();
}

static void DeleteTextureIfCreated(GLuint& textureId)
{
	if (textureId != 0)
	{
		glDeleteTextures(1, &textureId);
		textureId = 0;
	}
}

static void ReleaseModelTextures(ObjModel& model)
{
	DeleteTextureIfCreated(model.textureId);
	DeleteTextureIfCreated(model.barkTextureId);
	DeleteTextureIfCreated(model.branchTextureId);
	model.hasTexture = false;
	model.hasBarkTexture = false;
	model.hasBranchTexture = false;
}

static void ReleaseAllTextures()
{
	DeleteTextureIfCreated(dispBindIndex);
	DeleteTextureIfCreated(groundTextureId);
	DeleteTextureIfCreated(mountainTextureId);
	ReleaseModelTextures(campfireModel);
	ReleaseModelTextures(lanternModel);
	ReleaseModelTextures(rockModel);
	ReleaseModelTextures(woodModel);
	ReleaseModelTextures(campingCarModel);
	ReleaseModelTextures(picnicTableModel);
	ReleaseModelTextures(butterflyModel);
	ReleaseModelTextures(watermelonModel);
	ReleaseModelTextures(treeModel);
}

void close()
{
	ReleaseAllTextures();
	glutLeaveMainLoop();
}

void add_quats(float q1[4], float q2[4], float dest[4])
{
	static int count = 0;
	float t1[4], t2[4], t3[4];
	float tf[4];

	vcopy(q1, t1);
	vscale(t1, q2[3]);

	vcopy(q2, t2);
	vscale(t2, q1[3]);

	vcross(q2, q1, t3);
	vadd(t1, t2, tf);
	vadd(t3, tf, tf);
	tf[3] = q1[3] * q2[3] - vdot(q1, q2);

	dest[0] = tf[0];
	dest[1] = tf[1];
	dest[2] = tf[2];
	dest[3] = tf[3];

	if (++count > RENORMCOUNT) {
		count = 0;
		normalize_quat(dest);
	}
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(58, (double)width / height, 0.1, 100);
	glMatrixMode(GL_MODELVIEW);
}

void motion(int x, int y)
{
	GLfloat spin_quat[4];
	float gain;
	gain = 2.0; /* trackball gain */

	if (drag_state == GLUT_DOWN)
	{
		if (button_state == GLUT_LEFT_BUTTON)
		{
			trackball(spin_quat,
				(gain * rot_x - 500) / 500,
				(500 - gain * rot_y) / 500,
				(gain * x - 500) / 500,
				(500 - gain * y) / 500);
			add_quats(spin_quat, quat, quat);
		}
		else if (button_state == GLUT_RIGHT_BUTTON)
		{
			t[0] -= (((float)trans_x - x) / 500);
			t[1] += (((float)trans_y - y) / 500);
		}
		else if (button_state == GLUT_MIDDLE_BUTTON)
			t[2] -= (((float)trans_z - y) / 500 * 4);
		else if (button_state == 3 || button_state == 4) // scroll
		{

		}
		//glutPostRedisplay();
	}

	rot_x = x;
	rot_y = y;

	trans_x = x;
	trans_y = y;
	trans_z = y;
}

void mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		if (button == GLUT_LEFT_BUTTON)
		{
			rot_x = x;
			rot_y = y;

			//t[0] = t[0] + 1;


		}
		else if (button == GLUT_RIGHT_BUTTON)
		{
			trans_x = x;
			trans_y = y;
		}
		else if (button == GLUT_MIDDLE_BUTTON)
		{
			trans_z = y;
		}
		else if (button == 3 || button == 4)
		{
			const float sign = (static_cast<float>(button)-3.5f) * 2.0f;
			t[2] -= sign * 500 * 0.00015f;
		}
	}

	drag_state = state;
	button_state = button;
}

void vzero(float* v)
{
	v[0] = 0.0f;
	v[1] = 0.0f;
	v[2] = 0.0f;
}

void vset(float* v, float x, float y, float z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

void vsub(const float *src1, const float *src2, float *dst)
{
	dst[0] = src1[0] - src2[0];
	dst[1] = src1[1] - src2[1];
	dst[2] = src1[2] - src2[2];
}

void vcopy(const float *v1, float *v2)
{
	register int i;
	for (i = 0; i < 3; i++)
		v2[i] = v1[i];
}

void vcross(const float *v1, const float *v2, float *cross)
{
	float temp[3];

	temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
	temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
	temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
	vcopy(temp, cross);
}

float vlength(const float *v)
{
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void vscale(float *v, float div)
{
	v[0] *= div;
	v[1] *= div;
	v[2] *= div;
}

void vnormal(float *v)
{
	vscale(v, 1.0f / vlength(v));
}

float vdot(const float *v1, const float *v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void vadd(const float *src1, const float *src2, float *dst)
{
	dst[0] = src1[0] + src2[0];
	dst[1] = src1[1] + src2[1];
	dst[2] = src1[2] + src2[2];
}

void trackball(float q[4], float p1x, float p1y, float p2x, float p2y)
{
	float a[3]; /* Axis of rotation */
	float phi;  /* how much to rotate about axis */
	float p1[3], p2[3], d[3];
	float t;

	if (p1x == p2x && p1y == p2y) {
		/* Zero rotation */
		vzero(q);
		q[3] = 1.0;
		return;
	}

	/*
	 * First, figure out z-coordinates for projection of P1 and P2 to
	 * deformed sphere
	 */
	vset(p1, p1x, p1y, tb_project_to_sphere(TRACKBALLSIZE, p1x, p1y));
	vset(p2, p2x, p2y, tb_project_to_sphere(TRACKBALLSIZE, p2x, p2y));

	/*
	 *  Now, we want the cross product of P1 and P2
	 */
	vcross(p2, p1, a);

	/*
	 *  Figure out how much to rotate around that axis.
	 */
	vsub(p1, p2, d);
	t = vlength(d) / (2.0f*TRACKBALLSIZE);

	/*
	 * Avoid problems with out-of-control values...
	 */
	if (t > 1.0) t = 1.0;
	if (t < -1.0) t = -1.0;
	phi = 2.0f * asin(t);

	axis_to_quat(a, phi, q);
}

void axis_to_quat(float a[3], float phi, float q[4])
{
	vnormal(a);
	vcopy(a, q);
	vscale(q, sin(phi / 2.0f));
	q[3] = cos(phi / 2.0f);
}

float tb_project_to_sphere(float r, float x, float y)
{
	float d, t, z;

	d = sqrt(x*x + y*y);
	if (d < r * 0.70710678118654752440f) {    /* Inside sphere */
		z = sqrt(r*r - d*d);
	}
	else {           /* On hyperbola */
		t = r / 1.41421356237309504880f;
		z = t*t / d;
	}
	return z;
}

void normalize_quat(float q[4])
{
	int i;
	float mag;

	mag = (q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
	for (i = 0; i < 4; i++) q[i] /= mag;
}

void build_rotmatrix(float m[4][4], float q[4])
{
	m[0][0] = 1.0f - 2.0f * (q[1] * q[1] + q[2] * q[2]);
	m[0][1] = 2.0f * (q[0] * q[1] - q[2] * q[3]);
	m[0][2] = 2.0f * (q[2] * q[0] + q[1] * q[3]);
	m[0][3] = 0.0f;

	m[1][0] = 2.0f * (q[0] * q[1] + q[2] * q[3]);
	m[1][1] = 1.0f - 2.0f * (q[2] * q[2] + q[0] * q[0]);
	m[1][2] = 2.0f * (q[1] * q[2] - q[0] * q[3]);
	m[1][3] = 0.0f;

	m[2][0] = 2.0f * (q[2] * q[0] - q[1] * q[3]);
	m[2][1] = 2.0f * (q[1] * q[2] + q[0] * q[3]);
	m[2][2] = 1.0f - 2.0f * (q[1] * q[1] + q[0] * q[0]);
	m[2][3] = 0.0f;

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
}

void InitializeWindow(int argc, char* argv[])
{
	// initialize glut settings
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(1000 / 2, 1000 / 2);

	glutInitWindowPosition(0, 0);

	dispWindowIndex = glutCreateWindow("3D Model");

	// Initial front-view camera setup: start without a forced trackball rotation.
	quat[0] = 0.0f;
	quat[1] = 0.0f;
	quat[2] = 0.0f;
	quat[3] = 1.0f;
	t[0] = 0.0f;
	t[1] = 0.0f;
	t[2] = 0.0f;

	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutMotionFunc(motion);
	glutMouseFunc(mouse);
	glutCloseFunc(close);
	//GLuint image = load   ("./my_texture.bmp");
	
	//glBindTexture(1,)

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	// bind textures
	glClearColor(0.30f, 0.34f, 0.52f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	reshape(1000, 1000);

	/*glGenTextures(1, &dispBindIndex);
	glBindTexture(GL_TEXTURE_2D, dispBindIndex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case '1':
		// Demo key 1: toggle the hand-written face-normal Lambert shading.
		showShadingDemo = !showShadingDemo;
		cout << "[Demo 1] Shading: " << (showShadingDemo ? "ON" : "OFF") << endl;
		break;

	case '2':
		// Demo key 2: switch between original dog and custom subdivided dog.
		showSubdivisionDemo = !showSubdivisionDemo;
		cout << "[Demo 2] Subdivision: "
			<< (showSubdivisionDemo ? "ON (subdivided dog)" : "OFF (original low-poly dog)") << endl;
		break;

	case '3':
		// Demo key 3: switch between original watermelon and clustered simplification.
		showSimplificationDemo = !showSimplificationDemo;
		cout << "[Demo 3] Simplification: "
			<< (showSimplificationDemo ? "ON (simplified watermelon)" : "OFF (original watermelon)")
			<< endl;
		break;

	case 'r':
	case 'R':
		quat[0] = 0.0f;
		quat[1] = 0.0f;
		quat[2] = 0.0f;
		quat[3] = 1.0f;
		t[0] = 0.0f;
		t[1] = 0.0f;
		t[2] = 0.0f;
		break;

	case 27:
		close();
		break;
	}

	glutPostRedisplay();
}

void ApplyShadingDemoState()
{
	// Manual Lambert shading will be calculated in DrawModel().
	// Therefore, fixed-function OpenGL lighting must stay off.
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHT2);
	glDisable(GL_NORMALIZE);

	// One brightness value per polygon face.
	glShadeModel(GL_FLAT);
}

void DrawDuskBackground()
{
	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT | GL_TEXTURE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glShadeModel(GL_SMOOTH);

	// Simple dusk sky behind the campsite.
	glBegin(GL_QUADS);
	glColor3f(0.94f, 0.58f, 0.34f);
	glVertex3f(-4.20f, -0.55f, -2.85f);
	glColor3f(0.94f, 0.58f, 0.34f);
	glVertex3f(4.20f, -0.55f, -2.85f);
	glColor3f(0.28f, 0.34f, 0.58f);
	glVertex3f(4.20f, 2.25f, -2.85f);
	glColor3f(0.28f, 0.34f, 0.58f);
	glVertex3f(-4.20f, 2.25f, -2.85f);
	glEnd();

	// Dark horizon band: hides warm sky leaking under the mountain silhouettes.
	glColor3f(0.08f, 0.12f, 0.12f);
	glBegin(GL_QUADS);
	glVertex3f(-4.20f, -0.70f, -2.80f);
	glVertex3f(4.20f, -0.70f, -2.80f);
	glVertex3f(4.20f, -0.42f, -2.80f);
	glVertex3f(-4.20f, -0.42f, -2.80f);
	glEnd();

	// Distant mountain silhouettes: separate textured peaks.
	if (mountainTextureId != 0)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, mountainTextureId);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}

	glBegin(GL_TRIANGLES);

	// Left small mountain: darker and cooler.
	if (mountainTextureId != 0)
		glColor3f(0.50f, 0.56f, 0.52f);
	else
		glColor3f(0.22f, 0.26f, 0.25f);

	glTexCoord2f(0.00f, 0.00f);
	glVertex3f(-4.20f, -0.48f, -2.79f);
	glTexCoord2f(0.16f, 0.80f);
	glVertex3f(-3.05f, 0.34f, -2.79f);
	glTexCoord2f(0.34f, 0.00f);
	glVertex3f(-1.85f, -0.48f, -2.79f);

	// Center-left mountain: slightly brighter gray-green.
	if (mountainTextureId != 0)
		glColor3f(0.64f, 0.68f, 0.56f);
	else
		glColor3f(0.30f, 0.35f, 0.30f);

	glTexCoord2f(0.22f, 0.00f);
	glVertex3f(-2.45f, -0.48f, -2.77f);
	glTexCoord2f(0.42f, 0.95f);
	glVertex3f(-0.95f, 0.48f, -2.77f);
	glTexCoord2f(0.62f, 0.00f);
	glVertex3f(0.65f, -0.48f, -2.77f);

	// Center-right mountain: medium tone, behind the trees.
	if (mountainTextureId != 0)
		glColor3f(0.56f, 0.62f, 0.54f);
	else
		glColor3f(0.26f, 0.31f, 0.28f);

	glTexCoord2f(0.48f, 0.00f);
	glVertex3f(0.10f, -0.48f, -2.75f);
	glTexCoord2f(0.68f, 0.90f);
	glVertex3f(1.55f, 0.42f, -2.75f);
	glTexCoord2f(0.86f, 0.00f);
	glVertex3f(3.00f, -0.48f, -2.75f);

	// Far-right mountain: darker again so the four peaks do not merge.
	if (mountainTextureId != 0)
		glColor3f(0.46f, 0.52f, 0.49f);
	else
		glColor3f(0.21f, 0.25f, 0.24f);

	glTexCoord2f(0.72f, 0.00f);
	glVertex3f(2.00f, -0.48f, -2.73f);
	glTexCoord2f(0.88f, 0.78f);
	glVertex3f(3.20f, 0.30f, -2.73f);
	glTexCoord2f(1.00f, 0.00f);
	glVertex3f(4.20f, -0.48f, -2.73f);

	glEnd();

	if (mountainTextureId != 0)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}

	// Small distant tree silhouettes removed for the textured mountain background.
	glShadeModel(GL_FLAT);
	glDepthMask(GL_TRUE);
	glPopAttrib();
}

void DrawFakeShadow(float x, float y, float z, float scaleX, float scaleZ, float alpha)
{
	// Soft transparent ellipse used as a simple contact shadow under scene objects.
	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_LIGHTING_BIT | GL_TEXTURE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	glColor4f(0.03f, 0.025f, 0.02f, alpha);

	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(x, y, z);
	for (int i = 0; i <= 32; i++)
	{
		float angle = 6.2831853f * i / 32.0f;
		glVertex3f(x + cos(angle) * scaleX, y, z + sin(angle) * scaleZ);
	}
	glEnd();

	glDepthMask(GL_TRUE);
	glPopAttrib();
}
void DrawGroundGlow(float x, float y, float z, float scaleX, float scaleZ, float alpha, float red, float green, float blue)
{
	// Flat blended glow on the ground; this is visible geometry, not GL lighting.
	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_LIGHTING_BIT | GL_TEXTURE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	glShadeModel(GL_SMOOTH);

	glBegin(GL_TRIANGLE_FAN);
	glColor4f(red, green, blue, alpha);
	glVertex3f(x, y, z);
	for (int i = 0; i <= 40; i++)
	{
		float angle = 6.2831853f * i / 40.0f;
		glColor4f(red, green, blue, 0.0f);
		glVertex3f(x + cos(angle) * scaleX, y, z + sin(angle) * scaleZ);
	}
	glEnd();

	glShadeModel(GL_FLAT);
	glDepthMask(GL_TRUE);
	glPopAttrib();
}

void DrawVerticalGlow(float x, float y, float z, float scaleX, float scaleY, float alpha, float red, float green, float blue)
{
	// Upright blended glow for warm light sources like the campfire and lantern.
	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_LIGHTING_BIT | GL_TEXTURE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	glShadeModel(GL_SMOOTH);

	glBegin(GL_TRIANGLE_FAN);
	glColor4f(red, green, blue, alpha);
	glVertex3f(x, y, z);
	for (int i = 0; i <= 40; i++)
	{
		float angle = 6.2831853f * i / 40.0f;
		glColor4f(red, green, blue, 0.0f);
		glVertex3f(x + cos(angle) * scaleX, y + sin(angle) * scaleY, z);
	}
	glEnd();

	glShadeModel(GL_FLAT);
	glDepthMask(GL_TRUE);
	glPopAttrib();
}

void DrawCampfireSmoke(float x, float y, float z, float sceneTime)
{
	// Small animated alpha puffs for the campfire bonus scene detail.
	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_LIGHTING_BIT | GL_TEXTURE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	glShadeModel(GL_SMOOTH);

	for (int puff = 0; puff < 4; puff++)
	{
		float phase = fmod(sceneTime * 0.18f + puff * 0.25f, 1.0f);
		float puffX = x + sin(sceneTime * 0.8f + puff) * 0.035f;
		float puffY = y + phase * 0.55f;
		float puffZ = z + cos(sceneTime * 0.6f + puff) * 0.025f;
		float puffScale = 0.045f + phase * 0.075f;
		float puffAlpha = (1.0f - phase) * 0.16f;

		glBegin(GL_TRIANGLE_FAN);
		glColor4f(0.72f, 0.70f, 0.66f, puffAlpha);
		glVertex3f(puffX, puffY, puffZ);
		for (int i = 0; i <= 24; i++)
		{
			float angle = 6.2831853f * i / 24.0f;
			glColor4f(0.72f, 0.70f, 0.66f, 0.0f);
			glVertex3f(puffX + cos(angle) * puffScale, puffY + sin(angle) * puffScale * 0.65f, puffZ);
		}
		glEnd();
	}

	glShadeModel(GL_FLAT);
	glDepthMask(GL_TRUE);
	glPopAttrib();
}
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1, 0.1, 200);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// Initial front-view camera setup: centered, level campsite overview.
	gluLookAt(
		-0.15, 0.45, 3.35,
		-0.25, -0.38, 0.35,
		0.0, 1.0, 0.0
	);
	glTranslatef(t[0], t[1], t[2]);

	GLfloat m[4][4];
	build_rotmatrix(m, quat);
	glMultMatrixf(&m[0][0]);

	DrawDuskBackground();
	ApplyShadingDemoState();

	// Shared Ground Alignment: place props relative to one visual ground plane.
	const float GROUND_Y = -0.50f;

	// Ground Contrast: slightly dim/desaturate the grass so tree foliage separates better.
	GLfloat ground_ambient[4] = { 0.42f, 0.46f, 0.30f, 1.0f };
	GLfloat ground_diffuse[4] = { 0.70f, 0.76f, 0.50f, 1.0f };
	GLfloat ground_specular[4] = { 0.02f, 0.02f, 0.02f, 1.0f };
	const float GROUND_TEX_REPEAT_S = 4.0f;
	const float GROUND_TEX_REPEAT_T = 3.5f;
	if (groundTextureId != 0)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, groundTextureId);
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ground_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, ground_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, ground_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 2.0f);
	glColor3f(0.88f, 0.92f, 0.78f);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-3.60f, GROUND_Y - 0.03f, -2.70f);
	glTexCoord2f(GROUND_TEX_REPEAT_S, 0.0f);
	glVertex3f(3.00f, GROUND_Y - 0.03f, -2.70f);
	glTexCoord2f(GROUND_TEX_REPEAT_S, GROUND_TEX_REPEAT_T);
	glVertex3f(3.00f, GROUND_Y - 0.03f, 3.40f);
	glTexCoord2f(0.0f, GROUND_TEX_REPEAT_T);
	glVertex3f(-3.60f, GROUND_Y - 0.03f, 2.20f);
	glEnd();
	if (groundTextureId != 0)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}

	// Tent: solid khaki material, scaled down so it no longer hides the campfire.
	GLfloat tent_ambient[4] = { 0.18f, 0.28f, 0.12f, 1.0f };
	GLfloat tent_diffuse[4] = { 0.36f, 0.59f, 0.11f, 1.0f };
	GLfloat tent_specular[4] = { 0.08f, 0.08f, 0.06f, 1.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, tent_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, tent_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, tent_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 12.0f);
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.76f, 0.86f, 0.48f);

	const float TENT_X = -0.65f;
	const float TENT_Y = GROUND_Y + 0.08f;
	const float TENT_Z = 0.05f;
	const float TENT_SCALE = 1.0f;

	DrawFakeShadow(TENT_X - 0.10f, GROUND_Y - 0.025f, TENT_Z - 0.08f, 0.60f, 0.38f, 0.14f);

	glPushMatrix();
	glTranslatef(TENT_X, TENT_Y, TENT_Z);

	// New campingTent.obj is already Y-up.
	// Do not rotate it around the X axis.
	// If the entrance faces the wrong direction, adjust only Y rotation.
	// glRotatef(180.0f, 0.0f, 1.0f, 0.0f);

	glScalef(TENT_SCALE, TENT_SCALE, TENT_SCALE);
	DrawTentModel(tentModel);
	glPopMatrix();

	// Butterfly and Dog: butterfly moves in a figure-eight path,
// and the dog smoothly watches it without sudden angle reset.
	const float sceneTime = glutGet(GLUT_ELAPSED_TIME) * 0.001f;

	// -------------------- Butterfly: figure-eight movement --------------------
	const float BUTTERFLY_BASE_X = -0.30f;
	const float BUTTERFLY_BASE_Y = GROUND_Y + 0.55f;
	const float BUTTERFLY_BASE_Z = 2.00f;

	const float butterflyPhase = sceneTime * 0.85f;

	// Figure-eight path:
	// X uses sin(t), Z uses sin(2t), so the path becomes an infinity shape.
	const float butterflyX = BUTTERFLY_BASE_X + sin(butterflyPhase) * 0.52f;
	const float butterflyY = BUTTERFLY_BASE_Y + sin(sceneTime * 1.8f) * 0.11f + sin(sceneTime * 8.0f) * 0.025f;
	const float butterflyZ = BUTTERFLY_BASE_Z + sin(butterflyPhase * 2.0f) * 0.26f;

	// Gentle visual rotation, not direct atan2, to avoid sudden flipping.
	const float butterflyRotateY = -20.0f + sin(butterflyPhase) * 22.0f + sin(sceneTime * 8.0f) * 4.0f;
	const float butterflyWingTiltX = sin(sceneTime * 10.0f) * 3.0f;

	float butterflyShadowAlpha = 0.14f - (butterflyY - (GROUND_Y + 0.50f)) * 0.10f;
	if (butterflyShadowAlpha < 0.06f)
		butterflyShadowAlpha = 0.06f;
	if (butterflyShadowAlpha > 0.14f)
		butterflyShadowAlpha = 0.14f;

	DrawFakeShadow(butterflyX, GROUND_Y - 0.025f, butterflyZ, 0.13f, 0.07f, butterflyShadowAlpha);

	DrawButterflyInstance(butterflyX, butterflyY, butterflyZ, 0.65f, butterflyRotateY, butterflyWingTiltX);


	// -------------------- Dog: fixed position + stronger continuous watching rotation --------------------
	const float DOG_BASE_X = 0.10f;
	const float DOG_BASE_Y = GROUND_Y + 0.02f;

	// Place the dog along the same center line as the butterfly infinity path.
	// This makes the dog feel aligned with the middle "-" part of the figure-eight.
	const float DOG_BASE_Z = 1.70f;

	const float DOG_SCALE = 0.45f;

	// Keep the dog fixed in place.
	const float dogX = DOG_BASE_X;
	const float dogZ = DOG_BASE_Z;

	// Make the turning much more obvious.
	// The butterfly X position tells whether it is on the left loop or right loop.
	const float DOG_BASE_ROTATE_Y = 45.0f;     // Dog generally faces toward the butterfly path.
	const float DOG_MAX_LOOK_YAW = 32.0f;      // Stronger left/right turning than before.

	// Normalize butterfly horizontal offset into roughly [-1, 1].
	float dogLookAmount = (butterflyX - BUTTERFLY_BASE_X) / 0.52f;
	if (dogLookAmount > 1.0f)
		dogLookAmount = 1.0f;
	if (dogLookAmount < -1.0f)
		dogLookAmount = -1.0f;

	// Main target yaw: clear turning to left/right depending on butterfly loop side.
	float dogTargetRotateY = DOG_BASE_ROTATE_Y + dogLookAmount * DOG_MAX_LOOK_YAW;

	// Add a continuous small motion so the dog keeps moving its head/body slightly
	// even near the center crossing of the infinity path.
	dogTargetRotateY += sin(sceneTime * 2.8f) * 4.0f;

	// Smooth rotation memory so it looks alive, not snapping.
	static float dogSmoothRotateY = DOG_BASE_ROTATE_Y;
	dogSmoothRotateY += (dogTargetRotateY - dogSmoothRotateY) * 0.10f;

	// Tiny extra idle motion.
	float dogIdleYaw = sin(sceneTime * 3.6f) * 1.5f;

	DrawFakeShadow(dogX - 0.02f, GROUND_Y - 0.025f, dogZ + 0.07f, 0.24f, 0.15f, 0.12f);
	DrawDogInstance(dogX, DOG_BASE_Y, dogZ, DOG_SCALE, dogSmoothRotateY + dogIdleYaw);

	// Camping Car Placement: keep it parked farther left/back so it does not cut into the tent.
	DrawFakeShadow(-1.75f, GROUND_Y - 0.025f, -1.35f, 0.62f, 0.30f, 0.10f);
	DrawCampingCarInstance(-1.75f, GROUND_Y, -1.35f, 0.85f, 105.0f);
	if (showShadingDemo)
		DrawVerticalGlow(-1.66f, GROUND_Y + 0.36f, -1.24f, 0.27f, 0.18f, 0.24f, 1.0f, 0.72f, 0.24f);
	
	// Table Placement: move slightly forward and left.
	const float TABLE_X = -0.95f;
	const float TABLE_Y = GROUND_Y + 0.08f;
	const float TABLE_Z = 1.50f;
	const float TABLE_SCALE = 0.70f;
	const float TABLE_ROTATE_Y = -20.0f;
	DrawFakeShadow(TABLE_X - 0.05f, GROUND_Y - 0.025f, TABLE_Z + 0.04f,	0.36f, 0.23f, 0.12f);
	DrawPicnicTableInstance(TABLE_X, TABLE_Y, TABLE_Z, TABLE_SCALE, TABLE_ROTATE_Y);

	// Table-top props: closer to the center and swapped left/right positions.
	const float TABLE_ROTATE_RAD = TABLE_ROTATE_Y * 3.14159265f / 180.0f;
	const float TABLE_LONG_DIR_X = cos(TABLE_ROTATE_RAD);
	const float TABLE_LONG_DIR_Z = -sin(TABLE_ROTATE_RAD);
		
	const float TABLE_PROP_OFFSET = TABLE_SCALE * 0.20f;
	const float TABLETOP_PROP_Y = TABLE_Y + 0.26f;

	// Swapped: watermelon moves to the previous lantern side.
	const float WATERMELON_X = TABLE_X + TABLE_LONG_DIR_X * TABLE_PROP_OFFSET;
	const float WATERMELON_Y = TABLETOP_PROP_Y;
	const float WATERMELON_Z = TABLE_Z + TABLE_LONG_DIR_Z * TABLE_PROP_OFFSET;
	const float WATERMELON_SCALE = 0.50f;
	const float WATERMELON_ROTATE_Y = TABLE_ROTATE_Y;
	DrawFakeShadow(WATERMELON_X + 0.03f, TABLETOP_PROP_Y - 0.02f, WATERMELON_Z + 0.02f, 0.08f, 0.05f, 0.10f);
	DrawWatermelonInstance(WATERMELON_X, WATERMELON_Y, WATERMELON_Z, WATERMELON_SCALE, WATERMELON_ROTATE_Y);
	
	// Campfire: textured model pulled forward from the tent.
	const float CAMPFIRE_X = 0.35f;
	const float CAMPFIRE_Y = GROUND_Y + 0.10f;
	const float CAMPFIRE_Z = 0.95f;
	const float CAMPFIRE_SCALE = 0.135f;
	DrawFakeShadow(CAMPFIRE_X - 0.02f, GROUND_Y - 0.025f, CAMPFIRE_Z - 0.03f,0.10f, 0.08f, 0.04f);
	float flicker = 0.85f + 0.15f * sin(sceneTime * 8.0f);
	if (showShadingDemo)
	{
		// The warm fire effect is drawn with glow geometry.
		// Fixed-function GL_LIGHT1 is not used because manual shading is required.
		DrawGroundGlow(CAMPFIRE_X, GROUND_Y - 0.020f, CAMPFIRE_Z, 0.62f, 0.44f, 0.27f * flicker, 1.0f, 0.38f, 0.08f);
		DrawVerticalGlow(CAMPFIRE_X, CAMPFIRE_Y + 0.24f, CAMPFIRE_Z, 0.18f, 0.28f, 0.25f * flicker, 1.0f, 0.46f, 0.10f);
	}
	GLfloat campfire_ambient[4] = { 0.55f, 0.42f, 0.30f, 1.0f };
	GLfloat campfire_diffuse[4] = { 0.95f, 0.72f, 0.42f, 1.0f };
	GLfloat campfire_specular[4] = { 0.15f, 0.12f, 0.08f, 1.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, campfire_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, campfire_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, campfire_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 8.0f);
	glColor3f(0.95f, 0.62f, 0.28f);

	glPushMatrix();
	glTranslatef(CAMPFIRE_X, CAMPFIRE_Y, CAMPFIRE_Z);
	glScalef(CAMPFIRE_SCALE, CAMPFIRE_SCALE, CAMPFIRE_SCALE);
	// If the campfire appears tilted, adjust with glRotatef(angle, x, y, z) here.
	DrawModel(campfireModel, true);
	glPopMatrix();
	if (showShadingDemo)
		DrawCampfireSmoke(CAMPFIRE_X, CAMPFIRE_Y + 0.22f, CAMPFIRE_Z, sceneTime);

	// Lantern on Table: derive its position from the table so it sits on the tabletop.
	// Swapped: lantern moves to the previous watermelon side.
	const float LANTERN_X = TABLE_X - TABLE_LONG_DIR_X * TABLE_PROP_OFFSET;
	const float LANTERN_Y = TABLETOP_PROP_Y;
	const float LANTERN_Z = TABLE_Z - TABLE_LONG_DIR_Z * TABLE_PROP_OFFSET;
	const float LANTERN_SCALE = 0.18f;
	// Lantern brightness comes from material color; fixed-function GL_LIGHT2 stays off
	// because the project uses manual face-normal shading.
	GLfloat lantern_ambient[4] = { 0.50f, 0.44f, 0.34f, 1.0f };
	GLfloat lantern_diffuse[4] = { 0.95f, 0.84f, 0.62f, 1.0f };
	GLfloat lantern_specular[4] = { 0.35f, 0.30f, 0.22f, 1.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, lantern_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, lantern_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, lantern_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 16.0f);
	glColor3f(0.95f, 0.82f, 0.55f);

	glPushMatrix();
	glTranslatef(LANTERN_X, LANTERN_Y, LANTERN_Z);
	glRotatef(TABLE_ROTATE_Y, 0.0f, 1.0f, 0.0f);
	glScalef(LANTERN_SCALE, LANTERN_SCALE, LANTERN_SCALE);
	DrawModel(lanternModel, true);
	glPopMatrix();

	if (showShadingDemo)
	{
		// Lantern glow: visible blended geometry, not fixed-function GL_LIGHT.
		// This keeps the manual Lambert shading demo consistent.
		DrawGroundGlow(
			LANTERN_X, LANTERN_Y - 0.035f, LANTERN_Z,
			0.18f, 0.11f, 0.12f,
			1.0f, 0.78f, 0.28f
		);

		DrawVerticalGlow(
			LANTERN_X, LANTERN_Y + 0.08f, LANTERN_Z,
			0.11f, 0.16f, 0.15f,
			1.0f, 0.82f, 0.32f
		);

		DrawFakeShadow(
			LANTERN_X, LANTERN_Y - 0.01f, LANTERN_Z,
			0.07f, 0.04f, 0.08f
		);
	}

	// Trees: subtle ground-contact shadows.
	// Keep them soft and not too large, because these are background trees.

	DrawFakeShadow(-0.30f, GROUND_Y - 0.025f, -1.42f, 0.14f, 0.08f, 0.07f);
	DrawTreeInstance(-0.35f, GROUND_Y + 0.05f, -1.35f, 0.28f, 0.0f);	// Tree 1

	DrawFakeShadow(0.29f, GROUND_Y - 0.025f, -1.78f, 0.10f, 0.06f, 0.06f);
	DrawTreeInstance(0.25f, GROUND_Y + 0.05f, -1.72f, 0.18f, -12.0f);     // Small tree

	DrawFakeShadow(0.96f, GROUND_Y - 0.025f, -1.52f, 0.13f, 0.08f, 0.07f);
	DrawTreeInstance(0.90f, GROUND_Y + 0.05f, -1.45f, 0.26f, 25.0f);      // Tree 2

	DrawFakeShadow(1.51f, GROUND_Y - 0.025f, -1.08f, 0.15f, 0.09f, 0.08f);
	DrawTreeInstance(1.45f, GROUND_Y + 0.05f, -1.00f, 0.30f, -20.0f);     // Tree 3

	// Rock Placement: three rocks with different sizes.
	DrawFakeShadow(0.95f + 0.06f, GROUND_Y - 0.025f, -0.22f - 0.06f,		0.16f, 0.10f, 0.07f);
	DrawRockInstance(0.95f, GROUND_Y + 0.12f, -0.22f, 1.25f, 0.55f, 1.10f, 35.0f);   // 큰 납작한 돌

	DrawFakeShadow(1.32f + 0.06f, GROUND_Y - 0.025f, -0.02f - 0.05f, 0.12f, 0.08f, 0.06f);
	DrawRockInstance(1.32f, GROUND_Y + 0.11f, -0.02f, 0.90f, 0.45f, 0.70f, -40.0f);   // 중간 돌

	DrawFakeShadow(1.55f + 0.05f, GROUND_Y - 0.025f, -0.02f - 0.04f, 0.10f, 0.07f, 0.05f);
	DrawRockInstance(1.55f, GROUND_Y + 0.10f, -0.02f, 0.65f, 0.35f, 0.50f, 15.0f);   // 작은 돌

	// Wood Log Placement: textured log pile near the campfire, but outside the fire itself.
	const float WOOD_X = 0.78f;
	const float WOOD_Y = GROUND_Y + 0.10f;
	const float WOOD_Z = 1.18f;
	const float WOOD_SCALE = 0.65f;
	const float WOOD_ROTATE_Y = -25.0f;
	DrawFakeShadow(WOOD_X + 0.08f, GROUND_Y - 0.025f, WOOD_Z + 0.06f, 0.22f, 0.10f, 0.11f);
	GLfloat wood_ambient[4] = { 0.38f, 0.26f, 0.16f, 1.0f };
	GLfloat wood_diffuse[4] = { 0.72f, 0.48f, 0.28f, 1.0f };
	GLfloat wood_specular[4] = { 0.12f, 0.09f, 0.06f, 1.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, wood_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, wood_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, wood_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 8.0f);
	glColor3f(0.72f, 0.48f, 0.28f);

	glPushMatrix();
	glTranslatef(WOOD_X, WOOD_Y, WOOD_Z);
	glRotatef(WOOD_ROTATE_Y, 0.0f, 1.0f, 0.0f);
	glScalef(WOOD_SCALE, WOOD_SCALE, WOOD_SCALE);
	DrawModel(woodModel, true);
	glPopMatrix();

	glutSwapBuffers();
}

void DrawObj()
{
	// Legacy template entry point kept for compatibility.
	// The current renderer draws loaded OBJ assets through DrawModel() and instance helpers.
}

void DrawMeshObj()
{
	// Legacy template entry point kept for compatibility.
	// The current renderer draws loaded OBJ assets through DrawModel() and instance helpers.
}

static FILE* OpenAssetWithFallback(const char* path, char* openedPath, int openedPathSize)
{
	FILE* fp = fopen(path, "r");
	if (fp != NULL)
	{
		if (openedPath != NULL)
			sprintf_s(openedPath, openedPathSize, "%s", path);
		return fp;
	}

	char fallbackPath[512];
	sprintf_s(fallbackPath, sizeof(fallbackPath), "../%s", path);
	fp = fopen(fallbackPath, "r");
	if (fp != NULL && openedPath != NULL)
		sprintf_s(openedPath, openedPathSize, "%s", fallbackPath);
	return fp;
}

static bool ParseFaceVertex(const char* token, int* v, int* vt, int* vn)
{
	*v = 0;
	*vt = 0;
	*vn = 0;

	if (sscanf(token, "%d/%d/%d", v, vt, vn) == 3)
		return true;
	if (sscanf(token, "%d//%d", v, vn) == 2)
		return true;
	if (sscanf(token, "%d/%d", v, vt) == 2)
		return true;
	if (sscanf(token, "%d", v) == 1)
		return true;

	return false;
}

static const int TREE_MATERIAL_NONE = 0;
static const int TREE_MATERIAL_BARK = 1;
static const int TREE_MATERIAL_BRANCH = 2;
static const int TENT_MATERIAL_EXTERIOR = 10;
static const int TENT_MATERIAL_EXTPART = 11;
static const int TENT_MATERIAL_FLOOR = 12;
static const int TENT_MATERIAL_HITCHES = 13;
static const int TENT_MATERIAL_INTERIOR = 14;
static const int TENT_MATERIAL_POCKET = 15;
static const int TENT_MATERIAL_STICKS = 16;
static const int TENT_MATERIAL_TIES = 17;
static const int DOG_MATERIAL_EAR = 30;
static const int DOG_MATERIAL_EAR_2 = 31;
static const int DOG_MATERIAL_EAR_3 = 32;
static const int DOG_MATERIAL_EYE_AREA = 33;
static const int DOG_MATERIAL_EYE_AREA_2 = 34;
static const int DOG_MATERIAL_EYE_AREA_NONE = 35;
static const int DOG_MATERIAL_JAW = 36;
static const int DOG_MATERIAL_JAW_NONE = 37;
static const int DOG_MATERIAL_PAWS = 38;
static const int DOG_MATERIAL_PAWS_2 = 39;
static const int DOG_MATERIAL_SEAT = 40;
static const int DOG_MATERIAL_SKIN = 41;
static const int DOG_MATERIAL_SKIN_NONE = 42;
static const int DOG_MATERIAL_TAIL_1 = 43;
static const int DOG_MATERIAL_TAIL_2 = 44;
static const int DOG_MATERIAL_UNDERSKIN = 45;

// OBJ Loader: reads v, vt, vn, f, and usemtl lines into ObjModel.
// Faces keep vertex/texcoord/normal indices so DrawModel can render them directly.
bool LoadObj(const char* path, ObjModel& model, float scaleValue)
{
	FILE* fp = OpenAssetWithFallback(path, NULL, 0);
	if (fp == NULL)
	{
		cout << "Failed to open OBJ: " << path << endl;
		return false;
	}

	model.vertices.clear();
	model.texcoords.clear();
	model.normals.clear();
	model.faces.clear();
	model.textureId = 0;
	model.barkTextureId = 0;
	model.branchTextureId = 0;
	model.hasTexture = false;
	model.hasBarkTexture = false;
	model.hasBranchTexture = false;
	model.hasNormals = false;

	int currentMaterialId = TREE_MATERIAL_NONE;
	char line[512];
	while (fgets(line, sizeof(line), fp) != NULL)
	{
		float x, y, z;

		if (line[0] == 'v' && (line[1] == ' ' || line[1] == '\t'))
		{
			if (sscanf(line, "v %f %f %f", &x, &y, &z) == 3)
			{
				Vertex vertexData;
				vertexData.X = x / scaleValue;
				vertexData.Y = y / scaleValue;
				vertexData.Z = z / scaleValue;
				vertexData.index_1 = vertexData.index_2 = vertexData.index_3 = 0;
				model.vertices.push_back(vertexData);
			}
		}
		else if (line[0] == 'v' && line[1] == 't' && (line[2] == ' ' || line[2] == '\t'))
		{
			z = 0.0f;
			if (sscanf(line, "vt %f %f %f", &x, &y, &z) >= 2)
			{
				Vertex texcoordData;
				texcoordData.X = x;
				texcoordData.Y = y;
				texcoordData.Z = z;
				texcoordData.index_1 = texcoordData.index_2 = texcoordData.index_3 = 0;
				model.texcoords.push_back(texcoordData);
			}
		}
		else if (line[0] == 'v' && line[1] == 'n' && (line[2] == ' ' || line[2] == '\t'))
		{
			if (sscanf(line, "vn %f %f %f", &x, &y, &z) == 3)
			{
				Vertex normalData;
				normalData.X = x;
				normalData.Y = y;
				normalData.Z = z;
				normalData.index_1 = normalData.index_2 = normalData.index_3 = 0;
				model.normals.push_back(normalData);
			}
		}
		else if (strncmp(line, "usemtl", 6) == 0)
		{
			// Tree material mapping
			if (strstr(line, "Trunk_bark") != NULL)
				currentMaterialId = TREE_MATERIAL_BARK;
			else if (strstr(line, "New_Material_4__simple_quad_") != NULL)
				currentMaterialId = TREE_MATERIAL_BRANCH;

			// Camping tent material mapping
			else if (strstr(line, "exteriorTent") != NULL)
				currentMaterialId = TENT_MATERIAL_EXTERIOR;
			else if (strstr(line, "extPart") != NULL)
				currentMaterialId = TENT_MATERIAL_EXTPART;
			else if (strstr(line, "floorTent") != NULL)
				currentMaterialId = TENT_MATERIAL_FLOOR;
			else if (strstr(line, "hitches") != NULL)
				currentMaterialId = TENT_MATERIAL_HITCHES;
			else if (strstr(line, "intTentPocket") != NULL)
				currentMaterialId = TENT_MATERIAL_POCKET;
			else if (strstr(line, "intTent") != NULL)
				currentMaterialId = TENT_MATERIAL_INTERIOR;
			else if (strstr(line, "sticks") != NULL)
				currentMaterialId = TENT_MATERIAL_STICKS;
			else if (strstr(line, "ties") != NULL)
				currentMaterialId = TENT_MATERIAL_TIES;

			// Dog material mapping
			else if (strstr(line, "Ear__3") != NULL)
				currentMaterialId = DOG_MATERIAL_EAR_3;
			else if (strstr(line, "Ear_2") != NULL)
				currentMaterialId = DOG_MATERIAL_EAR_2;
			else if (strstr(line, "Ear") != NULL)
				currentMaterialId = DOG_MATERIAL_EAR;
			else if (strstr(line, "Eye_Area_NONE") != NULL)
				currentMaterialId = DOG_MATERIAL_EYE_AREA_NONE;
			else if (strstr(line, "Eye_Area_2") != NULL)
				currentMaterialId = DOG_MATERIAL_EYE_AREA_2;
			else if (strstr(line, "Eye_Area") != NULL)
				currentMaterialId = DOG_MATERIAL_EYE_AREA;
			else if (strstr(line, "Jaw_NONE") != NULL)
				currentMaterialId = DOG_MATERIAL_JAW_NONE;
			else if (strstr(line, "Jaw") != NULL)
				currentMaterialId = DOG_MATERIAL_JAW;
			else if (strstr(line, "Paws_2.001") != NULL)
				currentMaterialId = DOG_MATERIAL_PAWS_2;
			else if (strstr(line, "Paws") != NULL)
				currentMaterialId = DOG_MATERIAL_PAWS;
			else if (strstr(line, "Seat") != NULL)
				currentMaterialId = DOG_MATERIAL_SEAT;
			else if (strstr(line, "UnderSkin") != NULL)
				currentMaterialId = DOG_MATERIAL_UNDERSKIN;
			else if (strstr(line, "Skin_NONE") != NULL)
				currentMaterialId = DOG_MATERIAL_SKIN_NONE;
			else if (strstr(line, "Skin") != NULL)
				currentMaterialId = DOG_MATERIAL_SKIN;
			else if (strstr(line, "Tail_1") != NULL)
				currentMaterialId = DOG_MATERIAL_TAIL_1;
			else if (strstr(line, "Tail_2") != NULL)
				currentMaterialId = DOG_MATERIAL_TAIL_2;


			else
				currentMaterialId = TREE_MATERIAL_NONE;
		}
		else if (line[0] == 'f' && (line[1] == ' ' || line[1] == '\t'))
		{
			const int MAX_FACE_VERTICES = 64;
			char workLine[512];
			char* context = NULL;
			char* token = NULL;
			char* tokens[MAX_FACE_VERTICES];
			int tokenCount = 0;

			strcpy_s(workLine, sizeof(workLine), line);
			token = strtok_s(workLine + 1, " \t\r\n", &context);
			while (token != NULL && tokenCount < MAX_FACE_VERTICES)
			{
				tokens[tokenCount] = token;
				tokenCount++;
				token = strtok_s(NULL, " \t\r\n", &context);
			}

			if (tokenCount >= 3)
			{
				int v[MAX_FACE_VERTICES] = { 0 };
				int vt[MAX_FACE_VERTICES] = { 0 };
				int vn[MAX_FACE_VERTICES] = { 0 };
				bool parsed = true;

				for (int i = 0; i < tokenCount; i++)
					parsed = parsed && ParseFaceVertex(tokens[i], &v[i], &vt[i], &vn[i]);

				if (parsed)
				{
					if (tokenCount == 3 || tokenCount == 4)
					{
						MMesh faceData;
						faceData.V1 = v[0]; faceData.V2 = v[1]; faceData.V3 = v[2]; faceData.V4 = v[3];
						faceData.T1 = vt[0]; faceData.T2 = vt[1]; faceData.T3 = vt[2]; faceData.T4 = vt[3];
						faceData.N1 = vn[0]; faceData.N2 = vn[1]; faceData.N3 = vn[2]; faceData.N4 = vn[3];
						faceData.VertexCount = tokenCount;
						faceData.MaterialId = currentMaterialId;
						model.faces.push_back(faceData);
					}
					else
					{
						// N-gon Face Handling: camping_car.obj has faces with more than 4 vertices.
						// Convert [0, 1, 2, 3, ...] into triangle fan faces: [0,1,2], [0,2,3], ...
						for (int i = 1; i < tokenCount - 1; i++)
						{
							MMesh faceData;
							faceData.V1 = v[0]; faceData.V2 = v[i]; faceData.V3 = v[i + 1]; faceData.V4 = 0;
							faceData.T1 = vt[0]; faceData.T2 = vt[i]; faceData.T3 = vt[i + 1]; faceData.T4 = 0;
							faceData.N1 = vn[0]; faceData.N2 = vn[i]; faceData.N3 = vn[i + 1]; faceData.N4 = 0;
							faceData.VertexCount = 3;
							faceData.MaterialId = currentMaterialId;
							model.faces.push_back(faceData);
						}
					}
				}
			}
		}
	}
	fclose(fp);

	model.hasNormals = !model.normals.empty();
	cout << "Loaded " << path << ": " << model.vertices.size() << " vertices, "
		<< model.texcoords.size() << " texcoords, "
		<< model.normals.size() << " normals, "
		<< model.faces.size() << " faces" << endl;
	return !model.vertices.empty() && !model.faces.empty();
}

struct ClusterKey
{
	int X;
	int Y;
	int Z;

	bool operator<(const ClusterKey& other) const
	{
		if (X != other.X)
			return X < other.X;
		if (Y != other.Y)
			return Y < other.Y;
		return Z < other.Z;
	}
};

struct VertexCluster
{
	float SumX;
	float SumY;
	float SumZ;
	int Count;
	int NewIndex;
};

static bool HasDuplicateFaceVertex(const MMesh& face)
{
	int v[4] = { face.V1, face.V2, face.V3, face.V4 };
	for (int i = 0; i < face.VertexCount; i++)
	{
		if (v[i] <= 0)
			return true;
		for (int j = i + 1; j < face.VertexCount; j++)
		{
			if (v[i] == v[j])
				return true;
		}
	}
	return false;
}

// Demo 3 simplification: cluster nearby vertices into grid cells, remap faces,
// and skip collapsed faces so the watermelon keeps a valid lightweight mesh.
bool BuildSimplifiedWatermelonModel(const ObjModel& source, ObjModel& result, float gridCellSize)
{
	result.vertices.clear();
	result.texcoords = source.texcoords;
	result.normals.clear();
	result.faces.clear();
	result.textureId = source.textureId;
	result.barkTextureId = source.barkTextureId;
	result.branchTextureId = source.branchTextureId;
	result.hasTexture = source.hasTexture;
	result.hasBarkTexture = source.hasBarkTexture;
	result.hasBranchTexture = source.hasBranchTexture;
	result.hasNormals = false;

	if (source.vertices.empty() || source.faces.empty())
	{
		cout << "[Demo 3] Watermelon simplification skipped: source model is empty." << endl;
		return false;
	}

	if (gridCellSize <= 0.0001f)
		gridCellSize = 0.035f;

	vector<int> vertexRemap(source.vertices.size() + 1, 0);
	map<ClusterKey, int> clusterLookup;
	vector<VertexCluster> clusters;

	for (size_t i = 0; i < source.vertices.size(); i++)
	{
		const Vertex& vertex = source.vertices[i];
		ClusterKey key;
		key.X = (int)floor(vertex.X / gridCellSize);
		key.Y = (int)floor(vertex.Y / gridCellSize);
		key.Z = (int)floor(vertex.Z / gridCellSize);

		map<ClusterKey, int>::iterator found = clusterLookup.find(key);
		int clusterIndex = 0;
		if (found == clusterLookup.end())
		{
			VertexCluster cluster;
			cluster.SumX = vertex.X;
			cluster.SumY = vertex.Y;
			cluster.SumZ = vertex.Z;
			cluster.Count = 1;
			cluster.NewIndex = 0;
			clusters.push_back(cluster);
			clusterIndex = (int)clusters.size();
			clusterLookup[key] = clusterIndex;
		}
		else
		{
			clusterIndex = found->second;
			VertexCluster& cluster = clusters[clusterIndex - 1];
			cluster.SumX += vertex.X;
			cluster.SumY += vertex.Y;
			cluster.SumZ += vertex.Z;
			cluster.Count++;
		}

		vertexRemap[i + 1] = clusterIndex;
	}

	result.vertices.reserve(clusters.size());
	for (size_t i = 0; i < clusters.size(); i++)
	{
		VertexCluster& cluster = clusters[i];
		float count = (float)cluster.Count;
		Vertex vertex;
		vertex.X = cluster.SumX / count;
		vertex.Y = cluster.SumY / count;
		vertex.Z = cluster.SumZ / count;
		vertex.index_1 = 0;
		vertex.index_2 = 0;
		vertex.index_3 = 0;
		result.vertices.push_back(vertex);
		cluster.NewIndex = (int)result.vertices.size();
	}

	result.faces.reserve(source.faces.size());
	for (size_t i = 0; i < source.faces.size(); i++)
	{
		MMesh face = source.faces[i];
		int* vertexIndex[4] = { &face.V1, &face.V2, &face.V3, &face.V4 };
		int* texcoordIndex[4] = { &face.T1, &face.T2, &face.T3, &face.T4 };
		int* normalIndex[4] = { &face.N1, &face.N2, &face.N3, &face.N4 };
		bool validFace = true;

		for (int j = 0; j < face.VertexCount; j++)
		{
			int oldVertexIndex = *vertexIndex[j];
			if (oldVertexIndex <= 0 || oldVertexIndex > (int)source.vertices.size())
			{
				validFace = false;
				break;
			}

			int clusterIndex = vertexRemap[oldVertexIndex];
			if (clusterIndex <= 0 || clusterIndex > (int)clusters.size())
			{
				validFace = false;
				break;
			}

			*vertexIndex[j] = clusters[clusterIndex - 1].NewIndex;

			if (*texcoordIndex[j] <= 0 || *texcoordIndex[j] > (int)result.texcoords.size())
				*texcoordIndex[j] = 0;
			*normalIndex[j] = 0;
		}

		if (validFace && !HasDuplicateFaceVertex(face))
			result.faces.push_back(face);
	}

	cout << "[Demo 3] Watermelon simplification: "
		<< source.vertices.size() << " vertices / " << source.faces.size() << " faces -> "
		<< result.vertices.size() << " vertices / " << result.faces.size() << " faces" << endl;

	return !result.vertices.empty() && !result.faces.empty();
}

GLuint LoadTexture(const char* path)
{
	char fallbackPath[512];
	const char* loadPath = path;
	int width = 0;
	int height = 0;
	int channels = 0;
	unsigned char* data = stbi_load(loadPath, &width, &height, &channels, 0);

	if (data == NULL)
	{
		sprintf_s(fallbackPath, sizeof(fallbackPath), "../%s", path);
		loadPath = fallbackPath;
		data = stbi_load(loadPath, &width, &height, &channels, 0);
	}

	if (data == NULL)
	{
		cout << "Failed to load texture: " << path << endl;
		return 0;
	}

	GLenum format = GL_RGB;
	if (channels == 4)
		format = GL_RGBA;
	else if (channels == 1)
		format = GL_LUMINANCE;

	GLuint textureId = 0;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(data);

	cout << "Loaded texture " << loadPath << " (" << width << "x" << height << ")" << endl;
	return textureId;
}

static bool GetVertexByObjIndex(const ObjModel& model, int objIndex, Vertex* outVertex)
{
	if (objIndex <= 0 || objIndex >(int)model.vertices.size())
		return false;
	*outVertex = model.vertices[objIndex - 1];
	return true;
}

static void ComputeFaceNormal(const Vertex& p1, const Vertex& p2, const Vertex& p3, float normal[3])
{
	float ux = p2.X - p1.X;
	float uy = p2.Y - p1.Y;
	float uz = p2.Z - p1.Z;
	float vx = p3.X - p1.X;
	float vy = p3.Y - p1.Y;
	float vz = p3.Z - p1.Z;

	normal[0] = uy * vz - uz * vy;
	normal[1] = uz * vx - ux * vz;
	normal[2] = ux * vy - uy * vx;

	float length = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
	if (length > 0.00001f)
	{
		normal[0] /= length;
		normal[1] /= length;
		normal[2] /= length;
	}
	else
	{
		normal[0] = 0.0f;
		normal[1] = 1.0f;
		normal[2] = 0.0f;
	}
}

static float ClampFloat(float value, float minValue, float maxValue)
{
	if (value < minValue)
		return minValue;
	if (value > maxValue)
		return maxValue;
	return value;
}

static float ComputeManualLambertBrightness(const float normal[3])
{
	// Directional light for the manual shading demo.
	// Change this vector slightly if you want the bright side to face another direction.
	float lightDir[3] = { -0.45f, 0.85f, 0.35f };

	float length = sqrt(lightDir[0] * lightDir[0] +
		lightDir[1] * lightDir[1] +
		lightDir[2] * lightDir[2]);

	if (length > 0.00001f)
	{
		lightDir[0] /= length;
		lightDir[1] /= length;
		lightDir[2] /= length;
	}

	float ndotl = normal[0] * lightDir[0] +
		normal[1] * lightDir[1] +
		normal[2] * lightDir[2];

	ndotl = ClampFloat(ndotl, 0.0f, 1.0f);

	// ambient + diffuse * max(dot(N, L), 0)
	return 0.25f + 0.75f * ndotl;
}

static void ApplyManualShadingToCurrentColor(const float normal[3])
{
	GLfloat baseColor[4];
	glGetFloatv(GL_CURRENT_COLOR, baseColor);

	float brightness = 1.0f;
	if (showShadingDemo)
		brightness = ComputeManualLambertBrightness(normal);

	glColor4f(
		ClampFloat(baseColor[0] * brightness, 0.0f, 1.0f),
		ClampFloat(baseColor[1] * brightness, 0.0f, 1.0f),
		ClampFloat(baseColor[2] * brightness, 0.0f, 1.0f),
		baseColor[3]
	);
}

static void ApplyTexCoord(const ObjModel& model, int texcoordIndex)
{
	if (texcoordIndex > 0 && texcoordIndex <= (int)model.texcoords.size())
	{
		const Vertex& texcoord = model.texcoords[texcoordIndex - 1];
		glTexCoord2f(texcoord.X, 1.0f - texcoord.Y);
	}
}

static void ApplyNormal(const ObjModel& model, int normalIndex, const float faceNormal[3])
{
	if (normalIndex > 0 && normalIndex <= (int)model.normals.size())
	{
		const Vertex& normal = model.normals[normalIndex - 1];
		glNormal3f(normal.X, normal.Y, normal.Z);
	}
	else
	{
		glNormal3f(faceNormal[0], faceNormal[1], faceNormal[2]);
	}
}

struct EdgeMidpoint
{
	int A;
	int B;
	int Midpoint;
};

static Vertex MakeVertex(float x, float y, float z)
{
	Vertex vertex;
	vertex.X = x;
	vertex.Y = y;
	vertex.Z = z;
	vertex.index_1 = 0;
	vertex.index_2 = 0;
	vertex.index_3 = 0;
	return vertex;
}

static int AddMidpointVertex(ObjModel& model, vector<EdgeMidpoint>& edges, int indexA, int indexB)
{
	if (indexA > indexB)
	{
		int temp = indexA;
		indexA = indexB;
		indexB = temp;
	}

	for (size_t i = 0; i < edges.size(); i++)
	{
		if (edges[i].A == indexA && edges[i].B == indexB)
			return edges[i].Midpoint;
	}

	if (indexA <= 0 || indexB <= 0 ||
		indexA >(int)model.vertices.size() || indexB >(int)model.vertices.size())
		return 0;

	const Vertex& a = model.vertices[indexA - 1];
	const Vertex& b = model.vertices[indexB - 1];
	model.vertices.push_back(MakeVertex(
		(a.X + b.X) * 0.5f,
		(a.Y + b.Y) * 0.5f,
		(a.Z + b.Z) * 0.5f));

	EdgeMidpoint edge;
	edge.A = indexA;
	edge.B = indexB;
	edge.Midpoint = (int)model.vertices.size();
	edges.push_back(edge);
	return edge.Midpoint;
}

static void AddTriangleFace(vector<MMesh>& faces, int v1, int v2, int v3, int materialId)
{
	MMesh face;
	face.V1 = v1; face.V2 = v2; face.V3 = v3; face.V4 = 0;
	face.T1 = 0; face.T2 = 0; face.T3 = 0; face.T4 = 0;
	face.N1 = 0; face.N2 = 0; face.N3 = 0; face.N4 = 0;
	face.VertexCount = 3;
	face.MaterialId = materialId;
	faces.push_back(face);
}

static void AddSubdividedTriangle(ObjModel& model, vector<EdgeMidpoint>& edges, vector<MMesh>& faces,
	int v1, int v2, int v3, int materialId)
{
	int m12 = AddMidpointVertex(model, edges, v1, v2);
	int m23 = AddMidpointVertex(model, edges, v2, v3);
	int m31 = AddMidpointVertex(model, edges, v3, v1);

	if (m12 == 0 || m23 == 0 || m31 == 0)
		return;

	AddTriangleFace(faces, v1, m12, m31, materialId);
	AddTriangleFace(faces, m12, v2, m23, materialId);
	AddTriangleFace(faces, m31, m23, v3, materialId);
	AddTriangleFace(faces, m12, m23, m31, materialId);
}

static void AddNeighbor(vector<int>& neighbors, int neighbor)
{
	for (size_t i = 0; i < neighbors.size(); i++)
	{
		if (neighbors[i] == neighbor)
			return;
	}
	neighbors.push_back(neighbor);
}

static void SmoothDogVertices(ObjModel& model, float smoothingFactor)
{
	vector<vector<int> > neighbors(model.vertices.size());

	for (size_t i = 0; i < model.faces.size(); i++)
	{
		const MMesh& face = model.faces[i];
		if (face.VertexCount != 3)
			continue;

		int v[3] = { face.V1, face.V2, face.V3 };
		for (int j = 0; j < 3; j++)
		{
			int current = v[j] - 1;
			if (current < 0 || current >= (int)neighbors.size())
				continue;

			AddNeighbor(neighbors[current], v[(j + 1) % 3] - 1);
			AddNeighbor(neighbors[current], v[(j + 2) % 3] - 1);
		}
	}

	vector<Vertex> smoothed = model.vertices;
	for (size_t i = 0; i < model.vertices.size(); i++)
	{
		if (neighbors[i].empty())
			continue;

		float avgX = 0.0f;
		float avgY = 0.0f;
		float avgZ = 0.0f;
		for (size_t j = 0; j < neighbors[i].size(); j++)
		{
			const Vertex& neighbor = model.vertices[neighbors[i][j]];
			avgX += neighbor.X;
			avgY += neighbor.Y;
			avgZ += neighbor.Z;
		}

		float count = (float)neighbors[i].size();
		avgX /= count;
		avgY /= count;
		avgZ /= count;

		smoothed[i].X = model.vertices[i].X + (avgX - model.vertices[i].X) * smoothingFactor;
		smoothed[i].Y = model.vertices[i].Y + (avgY - model.vertices[i].Y) * smoothingFactor;
		smoothed[i].Z = model.vertices[i].Z + (avgZ - model.vertices[i].Z) * smoothingFactor;
	}

	model.vertices = smoothed;
}

static bool SubdivideDogOnce(const ObjModel& source, ObjModel& result)
{
	if (source.vertices.empty() || source.faces.empty())
		return false;

	result = source;
	result.vertices = source.vertices;
	result.texcoords.clear();
	result.normals.clear();
	result.faces.clear();
	result.hasTexture = false;
	result.hasNormals = false;

	vector<EdgeMidpoint> edges;
	vector<MMesh> subdividedFaces;

	// Quads are split into two triangles, and each triangle becomes 4 triangles.
	// Therefore, reserve more than 4x to avoid repeated reallocations.
	subdividedFaces.reserve(source.faces.size() * 8);

	for (size_t i = 0; i < source.faces.size(); i++)
	{
		const MMesh& face = source.faces[i];

		if (face.VertexCount == 3)
		{
			AddSubdividedTriangle(result, edges, subdividedFaces,
				face.V1, face.V2, face.V3, face.MaterialId);
		}
		else if (face.VertexCount == 4)
		{
			// Split quad into two triangles first, then subdivide each triangle.
			AddSubdividedTriangle(result, edges, subdividedFaces,
				face.V1, face.V2, face.V3, face.MaterialId);

			AddSubdividedTriangle(result, edges, subdividedFaces,
				face.V1, face.V3, face.V4, face.MaterialId);
		}
	}

	result.faces = subdividedFaces;
	return !result.vertices.empty() && !result.faces.empty();
}

// Demo 2 subdivision: run two custom midpoint-subdivision passes, then smooth
// lightly so the dog shows a clearer high-vs-low polygon comparison.
bool BuildSubdividedDogModel(const ObjModel& source, ObjModel& result)
{
	if (source.vertices.empty() || source.faces.empty())
		return false;

	// Pass 1: original low-poly dog -> moderately subdivided dog.
	ObjModel pass1;
	if (!SubdivideDogOnce(source, pass1))
		return false;

	// Weak smoothing after the first pass.
	SmoothDogVertices(pass1, 0.18f);

	// Pass 2: subdivide again for a much denser polygon structure.
	ObjModel pass2;
	if (!SubdivideDogOnce(pass1, pass2))
		return false;

	// Even weaker smoothing after the second pass.
	// Keep this small to avoid melting or shrinking the dog too much.
	SmoothDogVertices(pass2, 0.12f);

	result = pass2;

	cout << "[Demo 2] Dog subdivision generated with 2 passes: "
		<< source.vertices.size() << " vertices / " << source.faces.size() << " faces -> "
		<< result.vertices.size() << " vertices / " << result.faces.size() << " faces" << endl;

	return true;
}

static void ApplyTentMaterial(int materialId)
{
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4] = { 0.08f, 0.08f, 0.09f, 1.0f };
	GLfloat shininess = 20.0f;

	switch (materialId)
	{
	case TENT_MATERIAL_EXTPART:
		// Cool blue-gray trim / front fabric parts
		diffuse[0] = 0.42f; diffuse[1] = 0.48f; diffuse[2] = 0.56f; diffuse[3] = 1.0f;
		shininess = 24.0f;
		break;

	case TENT_MATERIAL_EXTERIOR:
		// Muted slate blue outer tent fabric
		diffuse[0] = 0.18f; diffuse[1] = 0.24f; diffuse[2] = 0.34f; diffuse[3] = 1.0f;
		shininess = 24.0f;
		break;

	case TENT_MATERIAL_TIES:
		// Dark blue-gray ties
		diffuse[0] = 0.10f; diffuse[1] = 0.13f; diffuse[2] = 0.18f; diffuse[3] = 1.0f;
		shininess = 20.0f;
		break;

	case TENT_MATERIAL_FLOOR:
		// Dark floor
		diffuse[0] = 0.041291f; diffuse[1] = 0.041291f; diffuse[2] = 0.041291f; diffuse[3] = 1.0f;
		shininess = 60.0f;
		break;

	case TENT_MATERIAL_HITCHES:
		// Almost black details
		diffuse[0] = 0.008658f; diffuse[1] = 0.012038f; diffuse[2] = 0.007460f; diffuse[3] = 1.0f;
		shininess = 10.0f;
		break;

	case TENT_MATERIAL_INTERIOR:
		// Gray inner tent fabric
		diffuse[0] = 0.682775f; diffuse[1] = 0.682775f; diffuse[2] = 0.682775f; diffuse[3] = 1.0f;
		shininess = 5.0f;
		break;

	case TENT_MATERIAL_POCKET:
		// Light gray / white small parts
		diffuse[0] = 0.882625f; diffuse[1] = 0.882625f; diffuse[2] = 0.882625f; diffuse[3] = 1.0f;
		shininess = 5.0f;
		break;

	case TENT_MATERIAL_STICKS:
		// Dark poles / sticks
		diffuse[0] = 0.019379f; diffuse[1] = 0.023423f; diffuse[2] = 0.011077f; diffuse[3] = 1.0f;
		specular[0] = 0.28f; specular[1] = 0.28f; specular[2] = 0.28f;
		shininess = 60.0f;
		break;

	default:
		// Fallback muted blue-gray
		diffuse[0] = 0.24f; diffuse[1] = 0.28f; diffuse[2] = 0.34f; diffuse[3] = 1.0f;
		shininess = 20.0f;
		break;
	}

	ambient[0] = diffuse[0] * 0.65f;
	ambient[1] = diffuse[1] * 0.65f;
	ambient[2] = diffuse[2] * 0.65f;
	ambient[3] = 1.0f;

	glDisable(GL_TEXTURE_2D);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
	glColor3f(diffuse[0], diffuse[1], diffuse[2]);
}

static void ApplyDogMaterial(int materialId)
{
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4] = { 0.16f, 0.14f, 0.12f, 1.0f };
	GLfloat shininess = 16.0f;

	switch (materialId)
	{
	case DOG_MATERIAL_EAR:
		diffuse[0] = 0.202355f; diffuse[1] = 0.092221f; diffuse[2] = 0.034825f; diffuse[3] = 1.0f;
		break;

	case DOG_MATERIAL_EAR_2:
		diffuse[0] = 0.055983f; diffuse[1] = 0.055983f; diffuse[2] = 0.055983f; diffuse[3] = 1.0f;
		break;

	case DOG_MATERIAL_EAR_3:
		diffuse[0] = 0.321575f; diffuse[1] = 0.176565f; diffuse[2] = 0.066390f; diffuse[3] = 1.0f;
		break;

	case DOG_MATERIAL_EYE_AREA:
	case DOG_MATERIAL_EYE_AREA_NONE:
		diffuse[0] = 0.343381f; diffuse[1] = 0.123758f; diffuse[2] = 0.029477f; diffuse[3] = 1.0f;
		break;

	case DOG_MATERIAL_EYE_AREA_2:
		diffuse[0] = 0.113921f; diffuse[1] = 0.100329f; diffuse[2] = 0.094622f; diffuse[3] = 1.0f;
		break;

	case DOG_MATERIAL_JAW:
	case DOG_MATERIAL_JAW_NONE:
		diffuse[0] = 0.113921f; diffuse[1] = 0.113921f; diffuse[2] = 0.113921f; diffuse[3] = 1.0f;
		break;

	case DOG_MATERIAL_PAWS:
		diffuse[0] = 0.113921f; diffuse[1] = 0.069220f; diffuse[2] = 0.038529f; diffuse[3] = 1.0f;
		break;

	case DOG_MATERIAL_PAWS_2:
		diffuse[0] = 0.381592f; diffuse[1] = 0.106260f; diffuse[2] = 0.029554f; diffuse[3] = 1.0f;
		break;

	case DOG_MATERIAL_SEAT:
		diffuse[0] = 0.081979f; diffuse[1] = 0.071084f; diffuse[2] = 0.071084f; diffuse[3] = 1.0f;
		break;

	case DOG_MATERIAL_SKIN:
	case DOG_MATERIAL_SKIN_NONE:
		diffuse[0] = 0.672962f; diffuse[1] = 0.257981f; diffuse[2] = 0.078718f; diffuse[3] = 1.0f;
		break;

	case DOG_MATERIAL_TAIL_1:
		diffuse[0] = 0.548766f; diffuse[1] = 0.217401f; diffuse[2] = 0.063218f; diffuse[3] = 1.0f;
		break;

	case DOG_MATERIAL_TAIL_2:
		diffuse[0] = 0.800000f; diffuse[1] = 0.625192f; diffuse[2] = 0.314568f; diffuse[3] = 1.0f;
		break;

	case DOG_MATERIAL_UNDERSKIN:
		diffuse[0] = 0.800000f; diffuse[1] = 0.724220f; diffuse[2] = 0.301969f; diffuse[3] = 1.0f;
		break;

	default:
		diffuse[0] = 0.56f; diffuse[1] = 0.34f; diffuse[2] = 0.18f; diffuse[3] = 1.0f;
		break;
	}

	ambient[0] = diffuse[0] * 0.55f;
	ambient[1] = diffuse[1] * 0.55f;
	ambient[2] = diffuse[2] * 0.55f;
	ambient[3] = 1.0f;

	glDisable(GL_TEXTURE_2D);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
	glColor3f(diffuse[0], diffuse[1], diffuse[2]);
}

void DrawTentModel(const ObjModel& model)
{
	glDisable(GL_TEXTURE_2D);

	for (size_t i = 0; i < model.faces.size(); i++)
	{
		const MMesh& face = model.faces[i];
		int vIndex[4] = { face.V1, face.V2, face.V3, face.V4 };
		int tIndex[4] = { face.T1, face.T2, face.T3, face.T4 };
		int nIndex[4] = { face.N1, face.N2, face.N3, face.N4 };
		Vertex p[4];

		if (!GetVertexByObjIndex(model, vIndex[0], &p[0]) ||
			!GetVertexByObjIndex(model, vIndex[1], &p[1]) ||
			!GetVertexByObjIndex(model, vIndex[2], &p[2]))
			continue;

		if (face.VertexCount == 4 && !GetVertexByObjIndex(model, vIndex[3], &p[3]))
			continue;

		float faceNormal[3];
		ComputeFaceNormal(p[0], p[1], p[2], faceNormal);

		ApplyTentMaterial(face.MaterialId);
		ApplyManualShadingToCurrentColor(faceNormal);

		if (face.VertexCount == 3)
			glBegin(GL_TRIANGLES);
		else
			glBegin(GL_QUADS);

		for (int j = 0; j < face.VertexCount; j++)
		{
			ApplyNormal(model, nIndex[j], faceNormal);
			glVertex3f(p[j].X, p[j].Y, p[j].Z);
		}

		glEnd();
	}

	glDisable(GL_TEXTURE_2D);
}

void DrawDogModel(const ObjModel& model)
{
	glDisable(GL_TEXTURE_2D);

	for (size_t i = 0; i < model.faces.size(); i++)
	{
		const MMesh& face = model.faces[i];
		int vIndex[4] = { face.V1, face.V2, face.V3, face.V4 };
		int tIndex[4] = { face.T1, face.T2, face.T3, face.T4 };
		int nIndex[4] = { face.N1, face.N2, face.N3, face.N4 };
		Vertex p[4];

		if (!GetVertexByObjIndex(model, vIndex[0], &p[0]) ||
			!GetVertexByObjIndex(model, vIndex[1], &p[1]) ||
			!GetVertexByObjIndex(model, vIndex[2], &p[2]))
			continue;

		if (face.VertexCount == 4 && !GetVertexByObjIndex(model, vIndex[3], &p[3]))
			continue;

		float faceNormal[3];
		ComputeFaceNormal(p[0], p[1], p[2], faceNormal);

		ApplyDogMaterial(face.MaterialId);
		ApplyManualShadingToCurrentColor(faceNormal);

		if (face.VertexCount == 3)
			glBegin(GL_TRIANGLES);
		else
			glBegin(GL_QUADS);

		for (int j = 0; j < face.VertexCount; j++)
		{
			ApplyNormal(model, nIndex[j], faceNormal);
			glVertex3f(p[j].X, p[j].Y, p[j].Z);
		}

		glEnd();
	}

	glDisable(GL_TEXTURE_2D);
}

// Generic OBJ draw path for textured props. Object-specific functions set color,
// transforms, or material choices before calling this helper.
void DrawModel(const ObjModel& model, bool useTexture)
{
	bool bindTexture = useTexture && model.hasTexture && model.textureId != 0;

	// Save the color that was set by the caller before DrawModel().
	// Example: rock, wood, car, lantern each sets its own base color.
	GLfloat baseColor[4];
	glGetFloatv(GL_CURRENT_COLOR, baseColor);

	if (bindTexture)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, model.textureId);

		// Texture color * glColor.
		// This makes the manual brightness affect textured OBJ models.
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}

	for (size_t i = 0; i < model.faces.size(); i++)
	{
		const MMesh& face = model.faces[i];
		int vIndex[4] = { face.V1, face.V2, face.V3, face.V4 };
		int tIndex[4] = { face.T1, face.T2, face.T3, face.T4 };
		int nIndex[4] = { face.N1, face.N2, face.N3, face.N4 };
		Vertex p[4];

		if (!GetVertexByObjIndex(model, vIndex[0], &p[0]) ||
			!GetVertexByObjIndex(model, vIndex[1], &p[1]) ||
			!GetVertexByObjIndex(model, vIndex[2], &p[2]))
			continue;

		if (face.VertexCount == 4 && !GetVertexByObjIndex(model, vIndex[3], &p[3]))
			continue;

		float faceNormal[3];
		ComputeFaceNormal(p[0], p[1], p[2], faceNormal);

		float brightness = 1.0f;
		if (showShadingDemo)
			brightness = ComputeManualLambertBrightness(faceNormal);

		// Manual Lambert shading result.
		// Shading ON  : base color/texture is multiplied by brightness.
		// Shading OFF : brightness is 1.0, so the original color/texture is shown.
		glColor4f(
			ClampFloat(baseColor[0] * brightness, 0.0f, 1.0f),
			ClampFloat(baseColor[1] * brightness, 0.0f, 1.0f),
			ClampFloat(baseColor[2] * brightness, 0.0f, 1.0f),
			baseColor[3]
		);

		if (face.VertexCount == 3)
			glBegin(GL_TRIANGLES);
		else
			glBegin(GL_QUADS);

		for (int j = 0; j < face.VertexCount; j++)
		{
			ApplyNormal(model, nIndex[j], faceNormal);

			if (bindTexture)
				ApplyTexCoord(model, tIndex[j]);

			glVertex3f(p[j].X, p[j].Y, p[j].Z);
		}

		glEnd();
	}

	// Restore the caller's color so the next object is not affected by the last face.
	glColor4f(baseColor[0], baseColor[1], baseColor[2], baseColor[3]);

	if (bindTexture)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}
}

void DrawTreeModel(const ObjModel& model)
{
	// Background Trees / Material Handling:
	// fir.obj stores material names per face. LoadObj converts those names into
	// TREE_MATERIAL_BARK or TREE_MATERIAL_BRANCH so this renderer can bind the
	// correct texture without requiring fir.mtl to exist.
	for (size_t i = 0; i < model.faces.size(); i++)
	{
		const MMesh& face = model.faces[i];
		int vIndex[4] = { face.V1, face.V2, face.V3, face.V4 };
		int tIndex[4] = { face.T1, face.T2, face.T3, face.T4 };
		int nIndex[4] = { face.N1, face.N2, face.N3, face.N4 };
		Vertex p[4];

		if (!GetVertexByObjIndex(model, vIndex[0], &p[0]) ||
			!GetVertexByObjIndex(model, vIndex[1], &p[1]) ||
			!GetVertexByObjIndex(model, vIndex[2], &p[2]))
			continue;
		if (face.VertexCount == 4 && !GetVertexByObjIndex(model, vIndex[3], &p[3]))
			continue;

		float faceNormal[3];
		ComputeFaceNormal(p[0], p[1], p[2], faceNormal);

		// Texture Loading / Material Handling:
		// Bark uses bark.jpg. Branches use branch.png and alpha test to discard
		// transparent pixels around the leaf card geometry.
		bool useTexture = false;
		bool useAlphaTest = false;
		GLuint textureId = 0;
		if (face.MaterialId == TREE_MATERIAL_BARK && model.hasBarkTexture)
		{
			useTexture = true;
			textureId = model.barkTextureId;
			glColor3f(0.70f, 0.48f, 0.28f);
		}
		else if (face.MaterialId == TREE_MATERIAL_BRANCH && model.hasBranchTexture)
		{
			useTexture = true;
			useAlphaTest = true;
			textureId = model.branchTextureId;
			glColor3f(0.28f, 0.62f, 0.34f);
		}
		else if (face.MaterialId == TREE_MATERIAL_BRANCH)
		{
			glDisable(GL_TEXTURE_2D);
			glColor3f(0.22f, 0.50f, 0.28f);
		}
		else
		{
			glDisable(GL_TEXTURE_2D);
			glColor3f(0.52f, 0.34f, 0.18f);
		}

		if (useTexture)
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, textureId);

			// Texture color * manual shading color.
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}

		if (useAlphaTest)
		{
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.35f);
		}

		ApplyManualShadingToCurrentColor(faceNormal);

		if (face.VertexCount == 3)
			glBegin(GL_TRIANGLES);
		else
			glBegin(GL_QUADS);

		for (int j = 0; j < face.VertexCount; j++)
		{
			ApplyNormal(model, nIndex[j], faceNormal);
			if (useTexture)
				ApplyTexCoord(model, tIndex[j]);
			glVertex3f(p[j].X, p[j].Y, p[j].Z);
		}
		glEnd();

		if (useAlphaTest)
			glDisable(GL_ALPHA_TEST);
		if (useTexture)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
		}
	}
}

void DrawTreeInstance(float x, float y, float z, float scaleValue, float rotateY)
{
	// Background Trees: instance transform wrapper for reusing treeModel.
	// Tree Contrast: cooler foliage material separates branch.png from the grass plane.
	GLfloat tree_ambient[4] = { 0.24f, 0.40f, 0.22f, 1.0f };
	GLfloat tree_diffuse[4] = { 0.34f, 0.68f, 0.32f, 1.0f };
	GLfloat tree_specular[4] = { 0.08f, 0.08f, 0.06f, 1.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, tree_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, tree_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, tree_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 6.0f);

	glPushMatrix();
	glTranslatef(x, y, z);
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);		
	glScalef(scaleValue, scaleValue, scaleValue);
	DrawTreeModel(treeModel);
	glPopMatrix();
}

void DrawRockInstance(float x, float y, float z, float scaleX, float scaleY, float scaleZ, float rotateY)
{
	// Rock Instance: reuse rockModel with its Color 4K texture.
	// Normal 4K, Roughness 4K, and AO textures are intentionally unused in this renderer.
	GLfloat rock_ambient[4] = { 0.36f, 0.34f, 0.30f, 1.0f };
	GLfloat rock_diffuse[4] = { 0.72f, 0.68f, 0.60f, 1.0f };
	GLfloat rock_specular[4] = { 0.10f, 0.10f, 0.09f, 1.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, rock_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, rock_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, rock_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 5.0f);
	glColor3f(0.70f, 0.66f, 0.58f);

	glPushMatrix();
	glTranslatef(x, y, z);
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);
	glScalef(scaleX, scaleY, scaleZ);
	DrawModel(rockModel, true);
	glPopMatrix();
}

void DrawCampingCarInstance(float x, float y, float z, float scaleValue, float rotateY)
{
	// Camping Car Instance: this OBJ is already Y-up, so no X-axis conversion is needed.
	GLfloat car_ambient[4] = { 0.54f, 0.54f, 0.52f, 1.0f };
	GLfloat car_diffuse[4] = { 0.88f, 0.88f, 0.84f, 1.0f };
	GLfloat car_specular[4] = { 0.32f, 0.32f, 0.30f, 1.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, car_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, car_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, car_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 18.0f);
	glColor3f(0.88f, 0.88f, 0.84f);

	glPushMatrix();
	glTranslatef(x, y, z);
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);
	glScalef(scaleValue, scaleValue, scaleValue);
	DrawModel(campingCarModel, true);
	glPopMatrix();
}

void DrawPicnicTableInstance(float x, float y, float z, float scaleValue, float rotateY)
{
	// Picnic Table Instance: lightweight textured table placed near the campfire.
	GLfloat picnic_ambient[4] = { 0.42f, 0.29f, 0.18f, 1.0f };
	GLfloat picnic_diffuse[4] = { 0.78f, 0.53f, 0.31f, 1.0f };
	GLfloat picnic_specular[4] = { 0.16f, 0.11f, 0.07f, 1.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, picnic_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, picnic_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, picnic_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 8.0f);
	glColor3f(0.78f, 0.53f, 0.31f);

	glPushMatrix();
	glTranslatef(x, y, z);
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);
	glScalef(scaleValue, scaleValue, scaleValue);
	DrawModel(picnicTableModel, true);
	glPopMatrix();
}

void DrawButterflyInstance(float x, float y, float z, float scaleValue, float rotateY, float rotateX)
{
	// Butterfly Instance: MONARCH.OBJ is tiny, so LoadObj scales it up before this transform.
	GLfloat butterfly_ambient[4] = { 0.85f, 0.85f, 0.85f, 1.0f };
	GLfloat butterfly_diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat butterfly_specular[4] = { 0.08f, 0.08f, 0.08f, 1.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, butterfly_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, butterfly_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, butterfly_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 6.0f);
	glColor3f(1.0f, 1.0f, 1.0f);

	glPushMatrix();
	glTranslatef(x, y, z);
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);
	glRotatef(rotateX, 1.0f, 0.0f, 0.0f);
	glRotatef(-8.0f, 0.0f, 0.0f, 1.0f);
	glScalef(scaleValue, scaleValue, scaleValue);
	DrawModel(butterflyModel, true);
	glPopMatrix();
}

static void DrawWatermelonWireframeModel(const ObjModel& model, float alpha)
{
	if (model.vertices.empty() || model.faces.empty())
		return;

	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT |
		GL_POLYGON_BIT | GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT | GL_TEXTURE_BIT);

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(0.75f);
	glColor4f(0.93f, 0.98f, 0.88f, alpha);

	for (size_t i = 0; i < model.faces.size(); i++)
	{
		const MMesh& face = model.faces[i];
		int vIndex[4] = { face.V1, face.V2, face.V3, face.V4 };
		Vertex p[4];

		if (!GetVertexByObjIndex(model, vIndex[0], &p[0]) ||
			!GetVertexByObjIndex(model, vIndex[1], &p[1]) ||
			!GetVertexByObjIndex(model, vIndex[2], &p[2]))
			continue;

		if (face.VertexCount == 4 && !GetVertexByObjIndex(model, vIndex[3], &p[3]))
			continue;

		if (face.VertexCount == 3)
			glBegin(GL_TRIANGLES);
		else
			glBegin(GL_QUADS);

		for (int j = 0; j < face.VertexCount; j++)
			glVertex3f(p[j].X, p[j].Y, p[j].Z);

		glEnd();
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPopAttrib();
}

void DrawWatermelonInstance(float x, float y, float z, float scaleValue, float rotateY)
{
	const ObjModel& drawModel = (showSimplificationDemo && !simplifiedWatermelonModel.faces.empty())
		? simplifiedWatermelonModel
		: watermelonModel;

	if (drawModel.vertices.empty() || drawModel.faces.empty())
		return;

	GLfloat watermelon_ambient[4] = { 0.42f, 0.50f, 0.34f, 1.0f };
	GLfloat watermelon_diffuse[4] = { 0.88f, 0.96f, 0.72f, 1.0f };
	GLfloat watermelon_specular[4] = { 0.12f, 0.16f, 0.10f, 1.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, watermelon_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, watermelon_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, watermelon_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 8.0f);
	glColor3f(0.90f, 0.98f, 0.78f);

	glPushMatrix();
	glTranslatef(x, y, z);
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);
	glScalef(scaleValue, scaleValue, scaleValue);
	DrawModel(drawModel, true);

	if (showSimplificationDemo && !simplifiedWatermelonModel.faces.empty())
		DrawWatermelonWireframeModel(drawModel, 0.16f);

	glPopMatrix();
}
static void DrawDogWireframeModel(const ObjModel& model, float alpha)
{
	if (model.vertices.empty() || model.faces.empty())
		return;

	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT |
		GL_POLYGON_BIT | GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT | GL_TEXTURE_BIT);

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Draw wireframe slightly above the filled dog surface to reduce z-fighting.
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.0f, -1.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Low-poly mode uses thicker lines so the original large polygons are obvious.
	if (showSubdivisionDemo)
		glLineWidth(1.0f);
	else
		glLineWidth(1.5f);

	// Dark transparent wireframe overlay.
	glColor4f(0.12f, 0.08f, 0.04f, alpha);

	for (size_t i = 0; i < model.faces.size(); i++)
	{
		const MMesh& face = model.faces[i];
		int vIndex[4] = { face.V1, face.V2, face.V3, face.V4 };
		Vertex p[4];

		if (!GetVertexByObjIndex(model, vIndex[0], &p[0]) ||
			!GetVertexByObjIndex(model, vIndex[1], &p[1]) ||
			!GetVertexByObjIndex(model, vIndex[2], &p[2]))
			continue;

		if (face.VertexCount == 4 && !GetVertexByObjIndex(model, vIndex[3], &p[3]))
			continue;

		if (face.VertexCount == 3)
			glBegin(GL_TRIANGLES);
		else
			glBegin(GL_QUADS);

		for (int j = 0; j < face.VertexCount; j++)
			glVertex3f(p[j].X, p[j].Y, p[j].Z);

		glEnd();
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPopAttrib();
}

void DrawDogInstance(float x, float y, float z, float scaleValue, float rotateY)
{
	// Dog Orientation Fix: this OBJ already stands along scene Y, so yaw only.
	glDisable(GL_TEXTURE_2D);

	const ObjModel& drawModel = (showSubdivisionDemo && !dogSubdividedModel.faces.empty())
		? dogSubdividedModel
		: dogModel;

	glPushMatrix();
	glTranslatef(x, y, z);
	glRotatef(rotateY, 0.0f, 1.0f, 0.0f);
	glScalef(scaleValue, scaleValue, scaleValue);

	// First draw the filled dog with its existing material colors.
	DrawDogModel(drawModel);

	// Then draw wireframe overlay to make the subdivision difference visible.
	if (showSubdivisionDemo)
		DrawDogWireframeModel(drawModel, 0.12f);
	else
		DrawDogWireframeModel(drawModel, 0.38f);

	glPopMatrix();
}

int main(int argc, char* argv[])
{
	if (!LoadObj("assets/tent/campingTent.obj", tentModel, 250.0f))
		return -1;

	if (!LoadObj("assets/campfire/Campfire_clean.OBJ", campfireModel, 20.0f))
		cout << "Campfire OBJ failed to load; continuing with tent only." << endl;

	if (!LoadObj("assets/tree/fir.obj", treeModel, 1.0f))
		cout << "Tree OBJ failed to load; continuing without background trees." << endl;

	// Lantern Asset: load the OBJ with the same generic loader used by other models.
	if (!LoadObj("assets/lantern/lantern_obj.obj", lanternModel, 100.0f))
		cout << "Lantern OBJ failed to load; continuing without lantern." << endl;

	// Rock Asset: load only the mesh; Color 4K is attached after OpenGL is initialized.
	if (!LoadObj("assets/rock/rock.obj", rockModel, 10.0f))
		cout << "Rock OBJ failed to load; continuing without rocks." << endl;

	// Wood Log Asset: load the mesh only; texture is attached after OpenGL initializes.
	if (!LoadObj("assets/wood/Wood.obj", woodModel, 20.0f))
		cout << "Wood OBJ failed to load; continuing without wood log pile." << endl;

	// Camping Car Asset: background vehicle, loaded with normalized scale from the OBJ.
	if (!LoadObj("assets/camping_car/camping_car.obj", campingCarModel, 1.0f))
		cout << "Camping car OBJ failed to load; continuing without camping car." << endl;

	// Picnic Table Asset: lightweight textured replacement for the removed chair.
	if (!LoadObj("assets/picnic_table/picnic_table.obj", picnicTableModel, 200.0f))
		cout << "Picnic table OBJ failed to load; continuing without picnic table." << endl;

	// Butterfly Asset: MONARCH.OBJ uses very small coordinates, so use a tiny divisor.
	if (!LoadObj("assets/butterfly/MONARCH.OBJ", butterflyModel, 0.30f))
		cout << "Butterfly OBJ failed to load; continuing without butterfly." << endl;

	// Dog Asset: mesh only; the provided dog texture path is not usable in this project.
	if (!LoadObj("assets/dog/GermanShephardLowPoly.obj", dogModel, 9.0f))
		cout << "Dog OBJ failed to load; continuing without dog." << endl;
	else if (!BuildSubdividedDogModel(dogModel, dogSubdividedModel))
		cout << "Dog subdivision failed; using original dog model." << endl;

	// Demo 3 Watermelon Asset: textured prop and vertex-clustered simplified mesh.
	if (!LoadObj("assets/Watermelon/Watermelon_SF.obj", watermelonModel, 15.0f))
		cout << "[Demo 3] Watermelon OBJ failed to load; continuing without watermelon." << endl;
	else
	{
		cout << "[Demo 3] Loaded watermelon: " << watermelonModel.vertices.size()
			<< " vertices / " << watermelonModel.faces.size() << " faces" << endl;
	}
	InitializeWindow(argc, argv);

	// Ground Texture Loading: use only the simple diffuse/base color JPG.
	groundTextureId = LoadTexture("assets/ground/ground_grass.jpg");
	mountainTextureId = LoadTexture("assets/background/mountain_texture.jpg");

	campfireModel.textureId = LoadTexture("assets/campfire/Campfire_MAT_BaseColor_01.jpg");
	campfireModel.hasTexture = campfireModel.textureId != 0;

	// Lantern Texture Loading: use only the Base Color texture from the PBR set.
	lanternModel.textureId = LoadTexture("assets/lantern/lantern_Base_Color.jpg");
	lanternModel.hasTexture = lanternModel.textureId != 0;

	// Rock Texture Loading: use only rock_color.jpg for the current fixed-function renderer.
	rockModel.textureId = LoadTexture("assets/rock/rock_color.jpg");
	rockModel.hasTexture = rockModel.textureId != 0;

	// Wood Texture Loading: use WoodTexture.png directly instead of paths from the MTL file.
	woodModel.textureId = LoadTexture("assets/wood/WoodTexture.png");
	woodModel.hasTexture = woodModel.textureId != 0;

	// Camping Car Texture Loading: direct texture loading, no full MTL parser needed.
	campingCarModel.textureId = LoadTexture("assets/camping_car/camping_car_texture.png");
	campingCarModel.hasTexture = campingCarModel.textureId != 0;

	// Picnic Table Texture Loading: use the color texture directly, no full MTL parser needed.
	picnicTableModel.textureId = LoadTexture("assets/picnic_table/picnic-table4.jpg");
	picnicTableModel.hasTexture = picnicTableModel.textureId != 0;

	// Butterfly Texture Loading: use only the local MONARCH.JPG diffuse texture.
	butterflyModel.textureId = LoadTexture("assets/butterfly/MONARCH.JPG");
	butterflyModel.hasTexture = butterflyModel.textureId != 0;

	// Watermelon Texture Loading: use only the visible diffuse texture.
	watermelonModel.textureId = LoadTexture("assets/Watermelon/Watermelon.jpg");
	watermelonModel.hasTexture = watermelonModel.textureId != 0;
	if (!watermelonModel.vertices.empty() && !BuildSimplifiedWatermelonModel(watermelonModel, simplifiedWatermelonModel, 0.035f))
		cout << "[Demo 3] Simplification failed; original watermelon remains available." << endl;

	// Texture Loading: fir tree uses bark for trunk and branch PNG for leaves.
	treeModel.barkTextureId = LoadTexture("assets/tree/bark.jpg");
	treeModel.branchTextureId = LoadTexture("assets/tree/branch.png");
	treeModel.hasBarkTexture = treeModel.barkTextureId != 0;
	treeModel.hasBranchTexture = treeModel.branchTextureId != 0;

	glutMainLoop();
	ReleaseAllTextures();
	return 0;
}

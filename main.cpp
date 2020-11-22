#include <GL/freeglut.h>
#include "SIG2020_1D.h"


adpd g_simulation(300, 300, 0.006, 0.0);

#define WIDTH 600
#define TIMESTEP 0.0001

void displayFunc(void);
void idleFunc(void);
void keyboardFunc(unsigned char key, int x, int y);

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(WIDTH, WIDTH);
	glutInitWindowPosition(100, 100);
	int id = glutCreateWindow("SIGASIA2020_1D");

	if (id < 1) {
		exit(1);
	}

	glutKeyboardFunc(keyboardFunc);
	glutIdleFunc(idleFunc);
	glutDisplayFunc(displayFunc);
	glutMainLoop();
	exit(0);
}

void keyboardFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 's' :
		g_simulation.update();
		break;
	}
}

void displayFunc(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(2.5);
	glBegin(GL_POINTS);

	for (int i = 0; i < g_simulation._nx; i++)
	{
		for (int j = 0; j < g_simulation._ny; j++)
		{
			glVertex3f(g_simulation._x[i], g_simulation._y[j], 0);

			double color = g_simulation._phi[i][j] / 2.0;
			glColor3f(color, color, color);
		}
	}
	
	glEnd();


	glutSwapBuffers();
}

void idleFunc(void)
{
	g_simulation.update();
	glutPostRedisplay();
}


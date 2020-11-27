#include <GL/freeglut.h>
#include "Kobayashi.h"


Kobayashi kobayashi(300, 300, 0.006);


void displayFunc(void);
void idleFunc(void);
void keyboardFunc(unsigned char key, int x, int y);

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(100, 100);
	int id = glutCreateWindow("Kobayashi Formulation");

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
			kobayashi.update();
			break;
	}
}

void displayFunc(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(2.5);
	glBegin(GL_POINTS);

	for (int i = 0; i < kobayashi._nx; i++)
	{
		for (int j = 0; j < kobayashi._ny; j++)
		{
			glVertex3f(kobayashi._x[i], kobayashi._y[j], 0);

			double color = kobayashi._phi[i][j] / 2.0;
			glColor3f(color, color, color);
		}
	}
	
	glEnd();


	glutSwapBuffers();
}

void idleFunc(void)
{
	kobayashi.update();
	glutPostRedisplay();
}


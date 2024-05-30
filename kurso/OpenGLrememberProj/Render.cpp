#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"


bool textureMode = true;
bool lightMode = true;

//����� ��� ��������� ������
class CustomCamera : public Camera
{
public:
	//��������� ������
	double camDist;
	//���� �������� ������
	double fi1, fi2;

	
	//������� ������ �� ���������
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//������� ������� ������, ������ �� ����� ��������, ���������� �������
	void SetUpCamera()
	{
		//�������� �� ������� ������ ������
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//������� ��������� ������
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //������� ������ ������


//����� ��� ��������� �����
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//��������� ������� �����
		pos = Vector3(1, 1, 3);
	}

	
	//������ ����� � ����� ��� ���������� �����, ���������� �������
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//����� �� ��������� ����� �� ����������
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//������ ���������
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// ��������� ��������� �����
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// �������������� ����������� �����
		// ������� ��������� (���������� ����)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// ��������� ������������ �����
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// ��������� ���������� ������������ �����
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //������� �������� �����




//������ ���������� ����
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//������ ���� ������ ��� ������� ����� ������ ����
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//������� ���� �� ���������, � ����� ��� ����
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//����������� ����� ������ ��������
void initRender(OpenGL *ogl)
{
	//��������� �������

	//4 ����� �� �������� �������
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//��������� ������ ��������� �������
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//�������� ��������
	glEnable(GL_TEXTURE_2D);
	

	//������ ����������� ���������  (R G B)
	RGBTRIPLE *texarray;

	//������ ��������, (������*������*4      4, ���������   ����, �� ������� ������������ �� 4 ����� �� ������� �������� - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//���������� �� ��� ��������
	glGenTextures(1, &texId);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, texId);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//�������� ������
	free(texCharArray);
	free(texarray);

	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//������ � ���� ����������� � "������"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// ������������ �������� : �� ����� ����� ����� 1
	glEnable(GL_NORMALIZE);

	// ���������� ������������� ��� �����
	glEnable(GL_LINE_SMOOTH); 


	//   ������ ��������� ���������
	//  �������� GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  ������� � ���������� �������� ���������(�� ���������), 
	//                1 - ������� � ���������� �������������� ������� ��������       
	//                �������������� ������� � ���������� ��������� ����������.    
	//  �������� GL_LIGHT_MODEL_AMBIENT - ������ ������� ���������, 
	//                �� ��������� �� ���������
	// �� ��������� (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}

//===============================================================================================================
double* findv(double* a, double* b) // ���� ������ �� 2 ������
{
	double* v = new double[3];
	v[0] = a[0] - b[0];
	v[1] = a[1] - b[1];
	return v;
}

double* findvn(double* a, double* b) // ���� ������ ������� �� 2 �������� a � b
{
	double* v = new double[3];
	v[0] = a[1] * b[2] - b[1] * a[2];
	v[1] = -a[0] * b[2] + b[0] * a[2];
	v[2] = a[0] * b[1] - b[0] * a[1];
	double len_v;
	len_v = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] = v[0] / len_v;
	v[1] = v[1] / len_v;
	v[2] = v[2] / len_v;
	return v;
}
void Cccircle(double* c, double r) {
	double d = 0;
	while (d <= 360) {

		glVertex3dv(c);
		glVertex3d(r * cos(d) + c[0], r * sin(d) + c[1], c[2]);
		glVertex3d(r * cos(d + 0.1) + c[0], r * sin(d + 0.1) + c[1], c[2]);
		d += 0.1;
	}
}
void Rrround(double* a, double* b, double h) {
	glColor3d(0, 0.5, 0.4);
	glBegin(GL_TRIANGLES);
	double middle[] = { (a[0] + b[0]) / 2, (a[1] + b[1]) / 2, (a[2] + b[2]) / 2 };
	double radius = sqrt((a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]) + (a[2] - b[2]) * (a[2] - b[2])) / 2;
	glNormal3d(0, 0, -1);
	Cccircle(middle, radius);
	double middle1[] = { middle[0], middle[1], middle[2] + h };
	glNormal3d(0, 0, 1);
	Cccircle(middle1, radius);
	glEnd();
	glColor3d(0, 0.5, 0);
	glBegin(GL_QUADS);
	double d = 0;
	double Ver1[3], Ver2[3], Ver3[3];
	double* vnorm;
		while (d <= 360) {
		Ver1[0] = radius * cos(d) + middle[0];
		Ver1[1] = radius * sin(d) + middle[1];
		Ver1[2] = middle[2];
		Ver2[0] = radius * cos(d) + middle1[0];
		Ver2[1] = radius * sin(d) + middle1[1];
		Ver2[2] = middle1[2];
		Ver3[0] = radius * cos(d + 0.1) + middle[0];
		Ver3[1] = radius * sin(d + 0.1) + middle[1];
		Ver3[2] = middle[2];
		vnorm = findvn(findv(Ver1, Ver2), findv(Ver1, Ver3));
		glNormal3d(-vnorm[0],-vnorm[1],-vnorm[2]);
		glVertex3d(radius * cos(d) + middle[0], radius * sin(d) + middle[1], middle[2]);
		glVertex3d(radius * cos(d) + middle1[0], radius * sin(d) + middle1[1], middle1[2]);
		glVertex3d(radius * cos(d + 0.1) + middle1[0], radius * sin(d + 0.1) + middle1[1], middle1[2]);
		glVertex3d(radius * cos(d + 0.1) + middle[0], radius * sin(d + 0.1) + middle[1], middle[2]);
		d += 0.1;
	}
	glEnd();
}


//===============================================================================================================

void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//��������������
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//��������� ���������
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//�������
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//��������
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//����������
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//������ �����
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//���� ���� �������, ��� ����������� (����������� ���������)
	glShadeModel(GL_SMOOTH);
	//===================================
	//������� ���  


	//������ ��������� ���������� ��������
	double A[] = { 0,0,0 };
	double B[] = { 2,-5,0 };
	double C[] = { 9,-1,0 };
	double D[] = { 12,-4,0 };
	double E[] = { 18,2,0 };
	double F[] = { 12,5,0 };
	double G[] = { 9,1,0 };
	double H[] = { 4,4,0 };
	double A1[] = { 0,0,9 };
	double B1[] = { 2,-5,9 };
	double C1[] = { 9,-1,9 };
	double D1[] = { 12,-4,9 };
	double E1[] = { 18,2,9 };
	double F1[] = { 12,5,9 };
	double G1[] = { 9,1,9 };
	double H1[] = { 4,4,9 };



	glTranslated(0, 0, 0);

	glBegin(GL_TRIANGLES);

	glColor3d(0, 0, 1);
//	glNormal3dv(findvn(findv(A,B), findv(A,H)));
	glNormal3d(0,0,-1);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(H);
	//glNormal3dv(findvn(findv(H,B), findv(H,E)));
	glNormal3d(0, 0, -1);
	glVertex3dv(C);
	glVertex3dv(F);
	glVertex3dv(G);
	glEnd();

	glBegin(GL_QUADS);

	glColor3d(0, 0, 1);
//	glNormal3dv(findvn(findv(B,C), findv(B,E)));
	glNormal3d(0, 0, -1);
	glVertex3dv(B);
	glVertex3dv(C);
	glVertex3dv(G);
	glVertex3dv(H);
	//glNormal3dv(findvn(findv(E,F), findv(E,H)));
	glNormal3d(0, 0, -1);
	glVertex3dv(C);
	glVertex3dv(D);
	glVertex3dv(E);
	glVertex3dv(F);

	
	glEnd();

	//glTranslated(0, 0, 9);
	glBegin(GL_TRIANGLES);

	glColor3d(0, 0, 1);
//	glNormal3dv(findvn(findv(A1,B1), findv(A1,H1)));
	glNormal3d(0, 0, 1);
	glVertex3dv(A1);
	glVertex3dv(B1);
	glVertex3dv(H1);
//	glNormal3dv(findvn(findv(H1,B1), findv(H1,E1)));
	glNormal3d(0, 0, 1);
	glVertex3dv(C1);
	glVertex3dv(F1);
	glVertex3dv(G1);

	glEnd();

	glBegin(GL_QUADS);

	glColor3d(0, 0, 1);
	//glNormal3dv(findvn(findv(B1,C1), findv(B1,E1)));
	glNormal3d(0, 0, 1);
	glVertex3dv(B1);
	glVertex3dv(C1);
	glVertex3dv(G1);
	glVertex3dv(H1);
	//glNormal3dv(findvn(findv(E1,F1), findv(E1,H1)));
	glNormal3d(0, 0, 1);
	glVertex3dv(C1);
	glVertex3dv(D1);
	glVertex3dv(E1);
	glVertex3dv(F1);


	glEnd();
	glBegin(GL_QUADS);

	glColor3d(0, 0, 0.5);
	glNormal3dv(findvn(findv(A,B),findv(A,A1)));
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(B1);
	glVertex3dv(A1);
	glNormal3dv(findvn(findv(B, C), findv(B,B1)));
	glVertex3dv(B);
	glVertex3dv(C);
	glVertex3dv(C1);
	glVertex3dv(B1);
	glNormal3dv(findvn(findv(C,D), findv(D,D1)));
	glVertex3dv(C);
	glVertex3dv(D);
	glVertex3dv(D1);
	glVertex3dv(C1);
	glNormal3dv(findvn(findv(D,E), findv(D,D1)));
	glVertex3dv(D);
	glVertex3dv(E);
	glVertex3dv(E1);
	glVertex3dv(D1);
	glNormal3dv(findvn(findv(E,F), findv(E,E1)));
	glVertex3dv(E);
	glVertex3dv(F);
	glVertex3dv(F1);
	glVertex3dv(E1);
	glNormal3dv(findvn(findv(F,G), findv(F,F1)));
	glVertex3dv(F);
	glVertex3dv(G);
	glVertex3dv(G1);
	glVertex3dv(F1);
	glNormal3dv(findvn(findv(G,H), findv(G,G1)));
	glVertex3dv(G);
	glVertex3dv(H);
	glVertex3dv(H1);
	glVertex3dv(G1);
	glNormal3dv(findvn(findv(H,A), findv(H,H1)));
	glVertex3dv(H);
	glVertex3dv(A);
	glVertex3dv(A1);
	glVertex3dv(H1);

	glEnd();

	glBegin(GL_TRIANGLES);
	glColor3d(0, 0, 0.1);

	Rrround(F, E, 9);

	glEnd();
	//����� ��������� ���������� ��������


   //��������� ������ ������

	
	glMatrixMode(GL_PROJECTION);	//������ �������� ������� ��������. 
	                                //(���� ��������� ��������, ����� �� ������������.)
	glPushMatrix();   //��������� ������� ������� ������������� (������� ��������� ������������� ��������) � ���� 				    
	glLoadIdentity();	  //��������� ��������� �������
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //������� ����� ������������� ��������

	glMatrixMode(GL_MODELVIEW);		//������������� �� �����-��� �������
	glPushMatrix();			  //��������� ������� ������� � ���� (��������� ������, ����������)
	glLoadIdentity();		  //���������� �� � ������

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //������� ����� ��������� ��� ������� ������ � �������� ������.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - ���/���� �������" << std::endl;
	ss << "L - ���/���� ���������" << std::endl;
	ss << "F - ���� �� ������" << std::endl;
	ss << "G - ������� ���� �� �����������" << std::endl;
	ss << "G+��� ������� ���� �� ���������" << std::endl;
	ss << "�����. �����: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "�����. ������: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "��������� ������: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //��������������� ������� �������� � �����-��� �������� �� �����.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}
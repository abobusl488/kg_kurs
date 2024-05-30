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

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
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
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
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
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
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

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
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

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}

//===============================================================================================================
double* findv(double* a, double* b) // ищем вектор по 2 точкам
{
	double* v = new double[3];
	v[0] = a[0] - b[0];
	v[1] = a[1] - b[1];
	return v;
}

double* findvn(double* a, double* b) // ищем вектор нормали по 2 векторам a и b
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


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  


	//Начало рисования квадратика станкина
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
	//конец рисования квадратика станкина


   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}
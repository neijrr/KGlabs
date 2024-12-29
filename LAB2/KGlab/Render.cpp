#include "Render.h"

#include <Windows.h>

#include <GL\GL.h>
#include <GL\GLU.h>

#include <iostream>
#include <iomanip>
#include <cmath>
#include <tuple>
#include <algorithm>
#include <sstream>
#include "Vector3.h"
#include "GUItextRectangle.h"


#ifdef _DEBUG
#include <Debugapi.h> 

struct debug_print
{
	template<class C>
	debug_print& operator<<(const C& a)
	{
		OutputDebugStringA((std::stringstream() << a).str().c_str());
		return *this;
	}
} debout;
#else
struct debug_print
{
	template<class C>
	debug_print& operator<<(const C& a)
	{
		return *this;
	}
} debout;
#endif

//библиотека для разгрузки изображений
//https://github.com/nothings/stb
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//внутренняя логика "движка"
#include "MyOGL.h"
extern OpenGL gl;




//возвращает точку направление по 2д координатам окна в 3д мире
std::tuple<double, double, double, double, double, double> getLookRay(int wndX, int wndY)
{
	GLint    viewport[4];    // параметры viewport-a.
	GLdouble projection[16]; // матрица проекции.
	GLdouble modelview[16];  // видовая матрица.
	GLdouble wx, wy, wz;       // возвращаемые мировые координаты.

	glGetIntegerv(GL_VIEWPORT, viewport);           // узнаём параметры viewport-a.
	glGetDoublev(GL_PROJECTION_MATRIX, projection); // узнаём матрицу проекции.
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);   // узнаём видовую матрицу.
	
	
	// переводим оконные координаты курсора в систему координат viewport-a.

	double originX, originY, originZ; //точка в 3д мире под мышью
	double directionX, directionY, directionZ; //направление клика

	//Обратное проекцирование 2d->3d (wndX, wndY, 0) -> (wx,wy,wz)   0 - глубина внутрь экрана
	gluUnProject(wndX, wndY, 0, modelview, projection, viewport, &wx, &wy, &wz);
	originX = wx;
	originY = wy;
	originZ = wz;

	//Обратное проекцирование 2d->3d (wndX, wndY, 1) -> (wx,wy,wz)   1 - глубина внутрь экрана
	gluUnProject(wndX, wndY, 1, modelview, projection, viewport, &wx, &wy, &wz);
	directionX = wx;
	directionY = wy;
	directionZ = wz;

	directionX -= originX;
	directionY -= originY;
	directionZ -= originZ;

	double length = sqrt(directionX * directionX + directionY * directionY + directionZ * directionZ);
	
	directionX /= length;
	directionY /= length;
	directionZ /= length;

	return {originX, originY, originZ, directionX, directionY, directionZ};

}

class Camera
{
	double camDist = 5;

	int camNz = 1;

	double camX;
	double camY;
	double camZ;
	int mouseX=-1 , mouseY=-1;

	bool drag = false;

public:
	//начальные углы камеры
	double _fi1 = 1;
	double _fi2 = 0.5;

	Camera()
	{
		caclulateCameraPos();
	}

	double distance()
	{
		return camDist;
	}

	int nZ() const
	{
		return camNz;
	}
	double x() const
	{
		return  camX;
	}
	double y() const
	{
		return  camY;
	}
	double z() const
	{
		return  camZ;
	}
	double fi1() const
	{
		return  _fi1;
	}
	double fi2() const
	{
		return  _fi2;
	}
	
	void caclulateCameraPos()
	{
		camX = camDist * cos(_fi2) * cos(_fi1);
		camY = camDist * cos(_fi2) * sin(_fi1);
		camZ = camDist * sin(_fi2);
		if (cos(_fi2) <= 0)
			camNz = -1;
		else
			camNz = 1;
	}
	
	void Zoom(OpenGL *sender, MouseWheelEventArg arg)
	{
		if (arg.value < 0 && camDist <= 1)
			return;
		if (arg.value > 0 && camDist >= 100)
			return;
		
		camDist += 0.01*arg.value;

		caclulateCameraPos();
	}

	void MouseMovie(OpenGL* sender, MouseEventArg arg)
	{
		if (OpenGL::isKeyPressed('G'))
			return;

		if (mouseX == -1)
		{
			mouseX = arg.x;
			mouseY = arg.y;
			return;
		}
		int dx = mouseX - arg.x;
		int dy = mouseY - arg.y;
		mouseX = arg.x;
		mouseY = arg.y;		

		if (drag)
		{
			_fi1 = _fi1 + 0.01 * dx;
			_fi2 = _fi2 - 0.01 * dy;

			caclulateCameraPos();
		}
	}
	void MouseLeave(OpenGL* sender, MouseEventArg arg)
	{
		mouseX = -1;
	}

	void MouseStartDrag(OpenGL* sender, MouseEventArg arg)
	{
		drag = true;
	}

	void MouseStopDrag(OpenGL* sender, MouseEventArg arg)
	{
		drag = false;
		mouseX = -1;		
	}

	void SetUpCamera()	
	{
		//сообщаем openGL настройки нашей камеры,
		// где она находится и куда смотрит
		// https://learn.microsoft.com/ru-ru/windows/win32/opengl/glulookat
		gluLookAt(camX, camY, camZ, 0, 0, 0, 0, 0, camNz);
	}

} camera;

class Light
{
	double posX=1;
	double posY=1;
	double posZ=1;

	bool drag = false;
	bool from_camera = false;

public:

	double x() const
	{
		return  posX;
	}
	double y() const
	{
		return  posY;
	}
	double z() const
	{
		return  posZ;
	}

	void StartDrug(OpenGL* sender, KeyEventArg arg)
	{
		if (arg.key == 0x47) //клавиша G
		{
			drag = true;
		}

		if (arg.key == 0x46) //клавиша F
		{
			from_camera = true;
		}


	}

	void StopDrug(OpenGL* sender, KeyEventArg arg)
	{
		if (arg.key == 0x47) //клавиша G
		{
			drag = false;
		}
		if (arg.key == 0x46) //клавиша F
		{
			from_camera = false;
		}
	}

	void MoveLight(OpenGL* sender, MouseEventArg arg)
	{
		//двигаем свет по плоскости, в точку где мышь

		if (drag)
		{
			int _x = arg.x;
			int _y = gl.getHeight() - arg.y;

			auto [oX, oY, oZ, dX, dY, dZ] = getLookRay(_x, _y);

			if (!OpenGL::isKeyPressed(VK_LBUTTON)) //если не нажата левая кнопка мыши
			{
				double z = posZ;

				double k = 0, x = 0, y = 0;
				if (dZ == 0)
					k = 0;
				else
					k = (z - oZ) / dZ;
				
				x = k * dX + oX;
				y = k * dY + oY;

				if (x * x + y * y > 2500) //не даем свету улететь далеко
					return;

				posX = x;
				posY = y;
				posZ = z;
			}
			else //если нажата
			{
				Vector3 o{ oX,oY,oZ };
				Vector3 d{ dX,dY,dZ };
				Vector3 z{ 0,0,1 };

				Vector3 _top = d ^ Vector3(0, 0, camera.nZ()) ^ d;

				//уравнение плоскости Ax+By+Cz+D=0  _top = (A,B, C)

				//ищем D
				double D = -_top.x() * oX - _top.y() * oY - _top.z() * oZ;

				//ищем новый z света
				if (_top.z() == 0)
					posZ = 0;
				else
					posZ = std::clamp(-(_top.x() * posX + _top.y() * posY + D) / _top.z(), -20.0, 20.0);
			}			
		}
	}

	void SetUpLight()
	{
		if (from_camera)	//если нажата F, устанавливаем 
							//позицию света в точку, откуда смотрим
		{
			posX = camera.x();
			posY = camera.y();
			posZ = camera.z();
		}
				
		// массивы с параметрами источника света
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		GLfloat lamb[] = { 0.2, 0.2, 0.2, 0 };
		// диффузная составляющая света
		GLfloat ldif[] = { 0.7, 0.7, 0.7, 0 };
		// зеркально отражаемая составляющая света
		GLfloat lspec[] = { 1.0, 1.0, 1.0, 0 };
		//координаты
		GLfloat lposition[] = { posX, posY, posZ, 1. };

		//сообщаем эти значения openGL.
		glLightfv(GL_LIGHT0, GL_POSITION, lposition);
		glLightfv(GL_LIGHT0, GL_AMBIENT, lamb);		
		glLightfv(GL_LIGHT0, GL_DIFFUSE, ldif);		
		glLightfv(GL_LIGHT0, GL_SPECULAR, lspec);
		glEnable(GL_LIGHT0);
	}

	void DrawLightGizmo()
	{
		//рисуем точку, отквда идет свет

		//устанавливаем размер точки
		GLfloat pointSize;
		glGetFloatv(GL_POINT_SIZE, &pointSize);
		glPointSize(10);


		//отключаем тест глубины, чтобы точка рисовалась сквозь все.
		glDisable(GL_DEPTH_TEST);

		//отключаем свет и текстуры
		glDisable(GL_TEXTURE_2D);		
		glDisable(GL_LIGHTING);

		//рисуем точку
		glBegin(GL_POINTS);
		glColor3d(1, 0.7, 0.1);
		glVertex3d(posX, posY, posZ);
		glEnd();

		//возращаем размер точки как был до нам
		glPointSize(pointSize);


		//если нажата G - рисуем линии осей от света
		if (!drag) return;

		GLfloat lineWidth;
		glGetFloatv(GL_LINE_WIDTH, &lineWidth);

		glLineWidth(3.0);

		glBegin(GL_LINES);
			glColor3d(0, 0, 0.8);
			glVertex3d(posX, posY, posZ);
			glVertex3d(posX, posY, 0);

			glColor3d(0.8, 0, 0);
			glVertex3d(posX - 1, posY, 0);
			glVertex3d(posX + 1, posY, 0);

			glColor3d(0, 0.8, 0);
			glVertex3d(posX, posY-1, 0);
			glVertex3d(posX, posY+1, 0);

		glEnd();

		glLineWidth(lineWidth);
		
	}


} light;

bool texturing = true;
bool lightning = true;
bool alpha = false;


//переключение режимов освещения, текстурирования, альфаналожения
void switchModes(OpenGL *sender, KeyEventArg arg)
{
	//конвертируем код клавиши в букву
	auto key = LOWORD(MapVirtualKeyA(arg.key, MAPVK_VK_TO_CHAR));

	switch (key)
	{
	case 'L':
		lightning = !lightning;
		break;
	case 'T':
		texturing = !texturing;
		break;
	case 'A':
		alpha = !alpha;
		break;
	}
}

//Текстовый прямоугольничек в верхнем правом углу.
//OGL не предоставляет возможности для хранения текста
//внутри этого класса создается картинка с текстом (через виндовый GDI),
//в виде текстуры накладывается на прямоугольник и рисуется на экране.
//Это самый простой способ что то написать на экране
//но ооооочень не оптимальный
GuiTextRectangle text;

//айдишник для текстуры
GLuint texId;
//выполняется один раз перед первым рендером
void initRender()
{
	//==============НАСТРОЙКА ТЕКСТУР================
	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//просим сгенерировать нам Id для текстуры
	//и положить его в texId
	glGenTextures(1, &texId);

	//делаем текущую текстуру активной
	//все, что ниже будет применено texId текстуре.
	glBindTexture(GL_TEXTURE_2D, texId);


	int x, y, n;

	//загружаем картинку
	//см. #include "stb_image.h" 
	unsigned char* data = stbi_load("texture.png", &x, &y, &n, 4);
	//x - ширина изображения
	//y - высота изображения
	//n - количество каналов
	//4 - нужное нам количество каналов
	//пиксели будут хранится в памяти [R-G-B-A]-[R-G-B-A]-[..... 
	// по 4 байта на пиксель - по байту на канал
	//пустые каналы будут равны 255

	//Картинка хранится в памяти перевернутой 
	//так как ее начало в левом верхнем углу
	//по этому мы ее переворачиваем -
	//меняем первую строку с последней,
	//вторую с предпоследней, и.т.д.
	unsigned char* _tmp = new unsigned char[x * 4]; //времянка
	for (int i = 0; i < y / 2; ++i)
	{
		std::memcpy(_tmp, data + i * x * 4, x * 4);//переносим строку i в времянку
		std::memcpy(data + i * x * 4, data + (y - 1 - i) * x * 4, x * 4); //(y-1-i)я строка -> iя строка
		std::memcpy(data + (y - 1 - i) * x * 4, _tmp, x * 4); //времянка -> (y-1-i)я строка
	}
	delete[] _tmp;


	//загрузка изображения в видеопамять
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	//выгрузка изображения из опперативной памяти
	stbi_image_free(data);


	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
												  //GL_REPLACE -- полная замена политога текстурой
	//настройка тайлинга
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//настройка фильтрации
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//======================================================

	//================НАСТРОЙКА КАМЕРЫ======================
	camera.caclulateCameraPos();

	//привязываем камеру к событиям "движка"
	gl.WheelEvent.reaction(&camera, &Camera::Zoom);
	gl.MouseMovieEvent.reaction(&camera, &Camera::MouseMovie);
	gl.MouseLeaveEvent.reaction(&camera, &Camera::MouseLeave);
	gl.MouseLdownEvent.reaction(&camera, &Camera::MouseStartDrag);
	gl.MouseLupEvent.reaction(&camera, &Camera::MouseStopDrag);
	//==============НАСТРОЙКА СВЕТА===========================
	//привязываем свет к событиям "движка"
	gl.MouseMovieEvent.reaction(&light, &Light::MoveLight);
	gl.KeyDownEvent.reaction(&light, &Light::StartDrug);
	gl.KeyUpEvent.reaction(&light, &Light::StopDrug);
	//========================================================
	//====================Прочее==============================
	gl.KeyDownEvent.reaction(switchModes);
	text.setSize(512, 180);
	//========================================================

}

void Render(double delta_time)
{    
	glEnable(GL_DEPTH_TEST);
	
	//натройка камеры и света
	//в этих функциях находятся OGLные функции
	//которые устанавливают параметры источника света
	//и моделвью матрицу, связанные с камерой.
	camera.SetUpCamera();
	light.SetUpLight();


	//рисуем оси
	gl.DrawAxes();

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	

	//включаем режимы, в зависимости от нажания клавиш. см void switchModes(OpenGL *sender, KeyEventArg arg)
	if (lightning)
		glEnable(GL_LIGHTING);
	if (texturing)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0); //сбрасываем текущую текстуру
	}
		
	if (alpha)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
		
	//=============НАСТРОЙКА МАТЕРИАЛА==============


	//настройка материала, все что рисуется ниже будет иметь этот метериал.
	//массивы с настройками материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.2f * 256;

	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); 
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH); //закраска по Гуро

	//============ РИСОВАТЬ ТУТ ==============

	

	//квадратик станкина
	//так как расчет освещения происходит только в вершинах
	// (закраска по Гуро)
	//то рисуем квадратик из более маленьких квадратиков
	glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_QUADS);
	glNormal3d(0, 0, 1);
	double h = 0.025;
	for (double x = h; x<= 1; x+= h)
		for (double y = h; y <= 1; y += h)
		{
			glColor3d(1, 1, 0);

			glTexCoord2d(x, y);
			glVertex2d(x, y);

			glTexCoord2d(x-h, y);
			glVertex2d(x-h, y);

			glTexCoord2d(x - h, y-h);
			glVertex2d(x - h, y-h);

			glTexCoord2d(x, y - h);
			glVertex2d(x, y - h);
		}
	glEnd();


	//===============================================

	//рисуем источник света
	light.DrawLightGizmo();

	//================Сообщение в верхнем левом углу=======================

	//переключаемся на матрицу проекции
	glMatrixMode(GL_PROJECTION);
	//сохраняем текущую матрицу проекции с перспективным преобразованием
	glPushMatrix();
	//загружаем единичную матрицу в матрицу проекции
	glLoadIdentity();

	//устанавливаем матрицу паралельной проекции
	glOrtho(0, gl.getWidth() - 1, 0, gl.getHeight() - 1, 0, 1);

	//переключаемся на моделвью матрицу
	glMatrixMode(GL_MODELVIEW);
	//сохраняем матрицу
	glPushMatrix();
    //сбразываем все трансформации и настройки камеры загрузкой единичной матрицы
	glLoadIdentity();

	//отрисованное тут будет визуалзироватся в 2д системе координат
	//нижний левый угол окна - точка (0,0)
	//верхний правый угол (ширина_окна - 1, высота_окна - 1)

	
	std::wstringstream ss;
	ss << std::fixed << std::setprecision(3);
	ss << "T - " << (texturing ? L"[вкл]выкл  " : L" вкл[выкл] ") << L"текстур" << std::endl;
	ss << "L - " << (lightning ? L"[вкл]выкл  " : L" вкл[выкл] ") << L"освещение" << std::endl;
	ss << "A - " << (alpha ? L"[вкл]выкл  " : L" вкл[выкл] ") << L"альфа-наложение" << std::endl;
	ss << L"F - Свет из камеры" << std::endl;
	ss << L"G - двигать свет по горизонтали" << std::endl;
	ss << L"G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << L"Коорд. света: (" << std::setw(7) <<  light.x() << "," << std::setw(7) << light.y() << "," << std::setw(7) << light.z() << ")" << std::endl;
	ss << L"Коорд. камеры: (" << std::setw(7) << camera.x() << "," << std::setw(7) << camera.y() << "," << std::setw(7) << camera.z() << ")" << std::endl;
	ss << L"Параметры камеры: R=" << std::setw(7) << camera.distance() << ",fi1=" << std::setw(7) << camera.fi1() << ",fi2=" << std::setw(7) << camera.fi2() << std::endl;
	ss << L"delta_time: " << std::setprecision(5)<< delta_time << std::endl;

	
	text.setPosition(10, gl.getHeight() - 10 - 180);
	text.setText(ss.str().c_str());
	text.Draw();

	//восстанавливаем матрицу проекции на перспективу, которую сохраняли ранее.
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	

}   




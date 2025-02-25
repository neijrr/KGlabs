#include "Render.h"
#include <Windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>



//библиотека для разгрузки изображений
//https://github.com/nothings/stb
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//внутренняя логика "движка"
#include "MyOGL.h"
extern OpenGL gl;

#include "Camera.h"
Camera camera;


void initRender()
{
	//================НАСТРОЙКА КАМЕРЫ======================
	camera.caclulateCameraPos();

	//привязываем камеру к событиям "движка"
	gl.WheelEvent.reaction(&camera, &Camera::Zoom); //рекакия на колесико мыши
	gl.MouseMovieEvent.reaction(&camera, &Camera::MouseMovie);//рекакия на движение мыши
	gl.MouseLeaveEvent.reaction(&camera, &Camera::MouseLeave); //мышь покидает окно
	gl.MouseLdownEvent.reaction(&camera, &Camera::MouseStartDrag); //Левая кнопка мыши нажата
	gl.MouseLupEvent.reaction(&camera, &Camera::MouseStopDrag); //Левая кнопка мыши отпущена

	camera.setPosition(2, 1.5, 1.5); //начальная позиция камеры
}

void Render(double delta_time)
{    
	camera.SetUpCamera(); //применяем настройки камеры. эту функцию удалать нельзя
	gl.DrawAxes(); //рисуем оси координат
	
	//программировать тут

}   




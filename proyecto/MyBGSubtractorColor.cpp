#include "MyBGSubtractorColor.hpp"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/video/background_segm.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

using namespace cv;
using namespace std;



MyBGSubtractorColor::MyBGSubtractorColor(VideoCapture vc) {

	cap = vc;
	max_samples = MAX_HORIZ_SAMPLES * MAX_VERT_SAMPLES;

	lower_bounds = vector<Scalar>(max_samples);
	upper_bounds = vector<Scalar>(max_samples);
	means = vector<Scalar>(max_samples);

	h_low = 50;
  h_up = 0;
	l_low = 36;
	l_up = 0;
	s_low = 92;
	s_up = 31;


	namedWindow("Trackbars");

	createTrackbar("H low:", "Trackbars", &h_low, 100, &MyBGSubtractorColor::Trackbar_func);
	createTrackbar("H high:", "Trackbars", &h_up, 100, &MyBGSubtractorColor::Trackbar_func);
	createTrackbar("L low:", "Trackbars", &l_low, 100, &MyBGSubtractorColor::Trackbar_func);
	createTrackbar("L high:", "Trackbars", &l_up, 100, &MyBGSubtractorColor::Trackbar_func);
	createTrackbar("S low:", "Trackbars", &s_low, 100, &MyBGSubtractorColor::Trackbar_func);
	createTrackbar("S high:", "Trackbars", &s_up, 100, &MyBGSubtractorColor::Trackbar_func);


}

void MyBGSubtractorColor::Trackbar_func(int, void*)
{

}


void MyBGSubtractorColor::LearnModel() {





	Mat frame, tmp_frame, hls_frame;
	std::vector<cv::Point> samples_positions;

	cap >> frame;

	//almacenamos las posiciones de las esquinas de los cuadrados
	Point p;
	for (int i = 0; i < MAX_HORIZ_SAMPLES; i++) {
		for (int j = 0; j < MAX_VERT_SAMPLES; j++) {
			p.x = frame.cols / 2 + (-MAX_HORIZ_SAMPLES / 2 + i)*(SAMPLE_SIZE + DISTANCE_BETWEEN_SAMPLES);
			p.y = frame.rows / 2 + (-MAX_VERT_SAMPLES / 2 + j)*(SAMPLE_SIZE + DISTANCE_BETWEEN_SAMPLES);
			samples_positions.push_back(p);
		}
	}

	namedWindow("Cubre los cuadrados con la mano y pulsa espacio");

	for (;;) {

		flip(frame, frame, 1);

		frame.copyTo(tmp_frame);

		//dibujar los cuadrados

		for (int i = 0; i < max_samples; i++) {
			rectangle(tmp_frame, Rect(samples_positions[i].x, samples_positions[i].y,
				      SAMPLE_SIZE, SAMPLE_SIZE), Scalar(0, 255, 0), 2);
		}



		imshow("Cubre los cuadrados con la mano y pulsa espacio", tmp_frame);
		char c = cvWaitKey(40);
		if (c == ' ')
		{
			break;
		}
		cap >> frame;
	}


        // CODIGO 1.1
        // Obtener las regiones de interés y calcular la media de cada una de ellas
        // almacenar las medias en la variable means
        // ...
				Mat roi;
				cvtColor(tmp_frame, hls_frame, CV_BGR2HLS); //pasar a HLS la imagen con los cuadrados
				for (int i = 0; i < max_samples; i++) {
					roi = hls_frame(Rect(samples_positions[i].x, samples_positions[i].y, //pasar el recorte de cada cuadrado a una variable
						      SAMPLE_SIZE, SAMPLE_SIZE));
					means[i]=mean(roi); //meter la media de color de los cuadrados en el vector de medias means (declarado en el constructor)
															//No se hace push_back porque el tamaño está ya definido :)
				}


        destroyWindow("Cubre los cuadrados con la mano y pulsa espacio");

}
void  MyBGSubtractorColor::ObtainBGMask(cv::Mat frame, cv::Mat &bgmask) {

        // CODIGO 1.2
        // Definir los rangos máximos y mínimos para cada canal (HLS)
        // umbralizar las imágenes para cada rango y sumarlas para
        // obtener la máscara final con el fondo eliminado
        //...

				//frame es el capturado
				//bgmask es lo que devuelve
				//acc.copyTo(bgmask);

				//inRange(imagen_en_grises, low,up,dst);
				//Pinta en blanco los colores que están entre low y up y en negro el resto
				//inRange(src, Scalar(10,7,100),Scalar(100,30,150),dst);
				//h  80-h_low(20)-100-h_up(60)-160
				//l  0-l_low(6)-6-l_up(10)-16
				//s  60-s_low(20)-80-s_up(70)-150
				//lower_bounds upper_bounds (puntos altos y bajos para cada media)
				//si es menor que 0 se pone en 0 y si es mayor que 255 lo mismo


				//means[i][0]h
				//				[1]l
				//				[2]s


				Mat frame_hls;
				cvtColor(frame, frame_hls, CV_BGR2HLS);

				Mat acc=Mat(frame.rows,frame.cols,CV_8U);
				acc.setTo(Scalar(0));
				Mat dst;
				for (int i = 0; i < max_samples; i++) {
					if(means[i][0]-h_low<0)
						lower_bounds[i][0]=0;
					else
						lower_bounds[i][0]=means[i][0]-h_low;

					if(means[i][1]-l_low<0)
						lower_bounds[i][1]=0;
					else
						lower_bounds[i][1]=means[i][1]-l_low;

					if(means[i][2]-s_low<0)
						lower_bounds[i][2]=0;
					else
						lower_bounds[i][2]=means[i][2]-s_low;


					if(means[i][0]+h_up>255)
						upper_bounds[i][0]=255;
					else
						upper_bounds[i][0]=means[i][0]+h_up;

					if(means[i][1]+l_up>255)
						upper_bounds[i][1]=255;
					else
						upper_bounds[i][1]=means[i][1]+l_up;

					if(means[i][2]+s_up>255)
						upper_bounds[i][2]=255;
					else
						upper_bounds[i][2]=means[i][2]+s_up;

					inRange(frame_hls, lower_bounds[i],upper_bounds[i],dst);
					acc+=dst;
				}
				acc.copyTo(bgmask);
}

#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
IplImage *tmp_frame;
int thresHoldValue = 255;
bool  isrun = true;
bool isdetected = false;

int width, height;
int R = 0, G = 0, B = 0;  

pthread_mutex_t mut;

bool GetColor(IplImage* image, CvRect rect){

	int X_min = rect.x;
	int Y_min = rect.y;
	int X_max = rect.x + rect.width; 
	int Y_max = rect.y + rect.height;


	int value = 0;
	int r = 0, g = 0, b = 0; 	
	

	for( int y = Y_min; y < Y_max; y++ ) {
     	uchar* ptr = (uchar*) (image->imageData + y * image->widthStep);
	    	for( int x = X_min; x < X_max; x++ ) {
			b += (int)(ptr[3*x]);     // B - синий
	     	g += (int)(ptr[3*x+1]);   // G - зелёный
	       	r += (int)(ptr[3*x+2]); // R - красный
			value++;
			
	  	}
	}

	if((R == 0) && (G == 0) && (B == 0)){
		B = (int)(b/value);
		G = (int)(g/value);
		R = (int)(r/value);
		printf("R = %d G = %d B = %d\n", R, G, B);
		return true;
	}
	else{
		printf("R = %d G = %d B = %d\n", R, G, B);
		printf("r = %d g = %d b = %d\n", (int)(r/value), (int)(g/value), (int)(b/value));
		if((abs(R-(int)(r/value)) < 15) && (abs(G-(int)(g/value)) < 15) && (abs(B-(int)(b/value)) < 15)){

			printf("+\n");
			return true;
		}
		else{
			printf("-\n");
			return false;
		}	
	}
	
}


void CentrOfMass(IplImage * iplSrc, CvBox2D rect){
	CvPoint2D32f boxPoints[4];
	cvBoxPoints(rect, boxPoints);
	int x = ((int)boxPoints[0].x + ((int)boxPoints[2].x - (int)boxPoints[0].x)/2);
	int y = ((int)boxPoints[0].y + ((int)boxPoints[2].y - (int)boxPoints[0].y)/2);
	cvCircle(iplSrc, cvPoint(x,y), 4 ,CV_RGB(0,0,0), 1,8,0);
}

void DrawRotatedRect( IplImage * iplSrc,CvBox2D rect,CvScalar color, int thickness, int line_type = 8, int shift = 0 ){   
	CvPoint2D32f boxPoints[4];
	cvBoxPoints(rect, boxPoints);
	cvLine(iplSrc,cvPoint((int)boxPoints[0].x, (int)boxPoints[0].y), 
		cvPoint((int)boxPoints[1].x, (int)boxPoints[1].y), color, thickness, line_type, shift);
	cvLine(iplSrc,cvPoint((int)boxPoints[1].x, (int)boxPoints[1].y),
		cvPoint((int)boxPoints[2].x, (int)boxPoints[2].y), color, thickness, line_type, shift);
	cvLine(iplSrc,cvPoint((int)boxPoints[2].x, (int)boxPoints[2].y),
		cvPoint((int)boxPoints[3].x, (int)boxPoints[3].y), color, thickness, line_type, shift);
	cvLine(iplSrc,cvPoint((int)boxPoints[3].x, (int)boxPoints[3].y),
		cvPoint((int)boxPoints[0].x, (int)boxPoints[0].y), color, thickness, line_type, shift);  
}	

void* GetContour(void* param){
	
	IplImage * bin_frame;
	IplImage * con_image;
	bin_frame = cvCreateImage( cvSize( width, height ), IPL_DEPTH_8U, 1 );
	con_image = cvCreateImage( cvSize( width, height ), IPL_DEPTH_8U, 3 );
	

	cvNamedWindow("Binary", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Contour", CV_WINDOW_AUTOSIZE);

	CvBox2D b;
	CvSeq* contours = 0;

	while(isrun){	

		if(!isdetected){
			pthread_mutex_lock(&mut);
			cvCopy( tmp_frame, con_image, 0 );
			pthread_mutex_unlock(&mut);

			cvCvtColor( con_image, bin_frame, CV_BGR2GRAY );
			cvThreshold( bin_frame, bin_frame, thresHoldValue, 255, CV_THRESH_BINARY );
		
   	     	cvShowImage("Binary", bin_frame);

			CvMemStorage* storage = cvCreateMemStorage(0);


			cvFindContours( bin_frame, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, 					cvPoint(0,0)); 

			for( CvSeq* c = contours; c!=NULL; c=c->h_next){

				CvRect Rect = cvBoundingRect( c ); // Поиск ограничивающего прямоугольника
				b = cvMinAreaRect2( c );
		  		if ( (Rect.width > 70) && (Rect.height > 70) && (Rect.width < 150) && (Rect.height < 150)  && 
					GetColor(con_image, Rect)){
					
				DrawRotatedRect( con_image, b, CV_RGB(255,0,0), 2 );
				CentrOfMass(con_image,b);
					
				}
			}
			
	
			cvReleaseMemStorage( &storage);

			cvShowImage("Contour", con_image);
		
			isdetected = true;
		}
	}

	
	cvReleaseImage( &bin_frame );
	cvReleaseImage( &con_image );
	
	cvDestroyWindow( "Contour");
	cvDestroyWindow( "Binary");

	
}

IplImage* My_thresHold(IplImage * gray){
	
	IplImage* tmp_gray = cvCreateImage( cvGetSize( gray ), 8, 1 );
	bool flag = true;
	while(flag){
	thresHoldValue-=2;
	cvThreshold( gray, tmp_gray, thresHoldValue, 255, CV_THRESH_BINARY );	

	CvMemStorage* storage = cvCreateMemStorage(0);

	CvSeq* contours = 0;

	// Поиск контуров
	cvFindContours( tmp_gray, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_NONE, cvPoint(0,0) ); 

	CvBox2D b;

	for( CvSeq* c=contours; c!=NULL; c=c->h_next){
		CvRect Rect = cvBoundingRect( c ); // Поиск ограничивающего прямоугольника
		if((Rect.width > 100) && (Rect.height > 100)){
			b = cvMinAreaRect2( c );
			GetColor(tmp_frame, Rect);			
			flag = false;
		}
	}
	cvReleaseMemStorage( &storage);
	}
	
	cvNamedWindow("Bin_frame", CV_WINDOW_AUTOSIZE );//Создание окна
	cvShowImage("Bin_frame", tmp_gray );//Вывод изображения в окно
	cvWaitKey(0);//Ожидание нажатия любой клавиши
	
	cvReleaseImage( &tmp_gray );
	cvDestroyWindow("Bin_frame");

	return tmp_gray;
}

void ImageFromCapture(){
	IplImage *frame;
	
	CvCapture* capture = 0;
	capture = cvCreateCameraCapture(1);
	//Создаётся окно вывода изображения на экран
	if( capture ){
		for(;;){
			//Захватывается фрейм
			if( !cvGrabFrame( capture )){
				 break;
			}	
			//Получается указатель на изображение
			frame = cvQueryFrame( capture );
	
			cvNamedWindow("Image", CV_WINDOW_AUTOSIZE );//Создание окна
			cvShowImage("Image", frame );//Вывод изображения в окно			
			tmp_frame = cvCreateImage(cvSize(frame->width, frame->height),IPL_DEPTH_8U, frame->nChannels);	

			pthread_mutex_lock(&mut);	
			cvCopy( frame, tmp_frame, 0 );
			pthread_mutex_unlock(&mut);

			if( cvWaitKey( 10 ) >= 0 ){ 
				break;
			}
		isdetected = false;
		}
		cvWaitKey(0);
		cvReleaseCapture( &capture );
		cvDestroyWindow( "Image");
	}
}




int main(int argc, char** argv){
	int rc;
	pthread_t thread;
	IplImage * bin_frame;


	ImageFromCapture();
	
	width = tmp_frame->width;
	height = tmp_frame->height;

	bin_frame = cvCreateImage( cvSize( width, height ), IPL_DEPTH_8U, 1 );
//	thresHoldValue-=5;
	
	cvCvtColor( tmp_frame, bin_frame, CV_RGB2GRAY );	
	
	bin_frame = My_thresHold(bin_frame);
	
	rc = pthread_create(&thread, NULL, GetContour, NULL);
	if (rc) {
		printf("ERROR; return code from pthread_create() is %d\n", rc);
	     exit(-1);
	}
		
	ImageFromCapture();
	isrun = false;
	pthread_join(thread, NULL);
	
	
	

//	cvReleaseImage( &bin_frame );
//	cvReleaseImage( &tmp_frame );	
	


	return 0;

		

	

}



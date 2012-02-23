#include <iostream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

#include <gl\glut.h>

#include "kinect\nui\Kinect.h"
#include "kinect\nui\ImageFrame.h"
using namespace kinect::nui;

#include "kinect_utility.h"

#pragma region // --- Constants ---
const double MY_MINDISTANCE = 0.1; // カメラから[0.1, 100.0]の範囲しか描画しない
const double MY_MAXDISTANCE = 100000.0;
const double CAMERA_DISTANCE = 1.5; //カメラと座標系の距離
#pragma endregion


#pragma region // --- global variables ---
int a=10;
Kinect mykinect;
Mat camera_img;
Mat depth_img;
ColorFromDepthEngine CFDengine;
#pragma endregion

void loop(){

	ImageStream& video = mykinect.VideoStream();
	ImageStream& depth = mykinect.DepthStream();
	kinect::nui::SkeletonEngine& skeleton = mykinect.Skeleton();
	//ColorFromDepthEngine CFDengine;
	
	#pragma region // === kinect ===

	# pragma region // --- get data ---
		// データの更新を待つ
		mykinect.WaitAndUpdateAll();

		//Skeletonを取得
		kinect::nui::SkeletonFrame skeletonframe= skeleton.GetNextFrame();

		// 次のフレームのデータを取得する(OpenNIっぽく)
		ImageFrame image( video );
		DepthFrame depthMD( depth );
		// cv::Matへのデータのコピー			
		camera_img = Mat(camera_img.size(),	CV_8UC4, (BYTE *)image.Bits());
		depth_img = Mat(depth_img.size(), CV_16UC1, (BYTE *)depthMD.Bits());
		// adjust with utility
		cv::Mat adjusted_camera_img(depth_img.size(), CV_8UC4);
		adjusted_camera_img = CFDengine.getAdjustedImage(camera_img);

		#pragma endregion

	#pragma region // --- processing ---
	SkeletonDrawer skeletondrawer(skeletonframe);
	SkeletonPoints me = skeletondrawer.me(depth_img.cols, depth_img.rows); //Depth画面中のskeletonを取得
		
	if(me.IsTracked){ // 画面内に人がいたら
		me.Drawall(adjusted_camera_img); // Skeleton を描く

		// 左右の手
		circle(adjusted_camera_img, me.getPoint(NUI_SKELETON_POSITION_HAND_LEFT), 5, cv::Scalar(0,0,255), -1);
		circle(adjusted_camera_img, me.getPoint(NUI_SKELETON_POSITION_HAND_RIGHT), 5, cv::Scalar(0,255,0), -1);
	}

	#pragma endregion

	#pragma region // --- show ---
	cv::resize(adjusted_camera_img, camera_img, camera_img.size());
	cv::imshow("camera_window", camera_img);	
	#pragma endregion

	#pragma endregion

	#pragma region // === gl Draw ---

	#pragma region // --- 視点決定 ---
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30.0, 1.0, MY_MINDISTANCE, MY_MAXDISTANCE);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//カメラの座標, 原点, 座標系の向き
	/*gluLookAt(
		sqrt(CAMERA_DISTANCE/3.0), sqrt(CAMERA_DISTANCE/3.0), -sqrt(CAMERA_DISTANCE/3.0),
		0, 0, 0,
		0, 1.0, 0);*/
	gluLookAt(
		CAMERA_DISTANCE, CAMERA_DISTANCE, CAMERA_DISTANCE,
		0, 0, 0,
		0, 1.0, 0);

	#pragma endregion

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	cv::imshow("hoge", depth_img);
	#pragma region // --- point cloud ---
	glEnable(GL_DEPTH_TEST);
	glPointSize(1); // Pointの大きさ
	glBegin(GL_POINTS); // Pointモード
	for(int x=0; x<depth_img.cols; x++){
		for(int y=0; y<depth_img.rows; y++){
			if(depth_img.at<ushort>(y,x)!=0){
				Vec4b cv_color_vec = adjusted_camera_img.at<Vec4b>(y,x);
				glColor3d(cv_color_vec[2]/255.0, cv_color_vec[1]/255.0, cv_color_vec[0]/255.0);
				glVertex3d( (x-depth_img.cols/2.0)/500.0, -(y-depth_img.cols/2.0)/500.0, -depth_img.at<ushort>(y,x)/32000.0 );
			}
			//std::cout<<(GLdouble)depth_img.at<ushort>(y,x)<<std::endl;
		}
	}
	glEnd();

	
	glFlush();
	
	glDisable(GL_DEPTH_TEST);
	#pragma endregion

	glutSwapBuffers();


	#pragma endregion

	std::cout<<"myao"<<std::endl;
		
}

void glut_keyboard(unsigned char key, int x, int y){
	if ( key == 'q' ) {
		exit(0);
	}
	glutPostRedisplay();
}

void glut_idle(){
	glutPostRedisplay();
}

int main(int argc, char *argv[]){

	#pragma region // --- init ---
	//kinectクラスの宣言と初期化
	mykinect.Initialize(NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX );

	//ストリーム作る。画像用とデプス用
	ImageStream& video = mykinect.VideoStream();
	video.Open( NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480 );

	ImageStream& depth = mykinect.DepthStream();
	depth.Open( NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, NUI_IMAGE_RESOLUTION_320x240 );

	//skeletonを使う準備
	kinect::nui::SkeletonEngine& skeleton = mykinect.Skeleton();
	skeleton.Enable();

	//opencvのmatとwindowの準備
	namedWindow("camera_window");
	camera_img = Mat(Size(video.Width(), video.Height()), CV_8UC4);

	namedWindow("depth_window");
	depth_img = Mat(Size(depth.Width(), depth.Height()), CV_16UC1); //depthは16bitみたい

	//Depthとcameraの位置合わせ(kinect_utility.h)
	//ColorFromDepthEngine CFDengine;
	CFDengine = ColorFromDepthEngine();

	#pragma endregion

	#pragma region // --- init GL ---

	glutInit(&argc, argv);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(400, 400);
	glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE );

	glutCreateWindow("test window");
	glutDisplayFunc(loop);
	glutKeyboardFunc(glut_keyboard);
	glutIdleFunc(glut_idle);

	glClearColor(0.1, 0.1, 0.1, 0.5); //背景色

	#pragma endregion 

	glutMainLoop();

	return 0; // 正常終了
}
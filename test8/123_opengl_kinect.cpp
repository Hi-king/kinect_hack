#include <iostream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

#include <gl\glut.h>

#include "kinect\nui\Kinect.h"
#include "kinect\nui\ImageFrame.h"
using namespace kinect::nui;

#include "kinect_utility.h"


#pragma region // --- global variables
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

		cv::imshow("cam", camera_img);
		cv::imshow("dep", depth_img);

		// adjust with utility
		cv::Mat adjusted_camera_img(depth_img.size(), CV_8UC4);
		adjusted_camera_img = CFDengine.getAdjustedImage(camera_img);

		cv::imshow("adj", adjusted_camera_img);
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
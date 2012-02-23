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
		// �f�[�^�̍X�V��҂�
		mykinect.WaitAndUpdateAll();

		//Skeleton���擾
		kinect::nui::SkeletonFrame skeletonframe= skeleton.GetNextFrame();

		// ���̃t���[���̃f�[�^���擾����(OpenNI���ۂ�)
		ImageFrame image( video );
		DepthFrame depthMD( depth );
		// cv::Mat�ւ̃f�[�^�̃R�s�[			
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
		SkeletonPoints me = skeletondrawer.me(depth_img.cols, depth_img.rows); //Depth��ʒ���skeleton���擾
		
		if(me.IsTracked){ // ��ʓ��ɐl��������
			me.Drawall(adjusted_camera_img); // Skeleton ��`��

			// ���E�̎�
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
	//kinect�N���X�̐錾�Ə�����
	mykinect.Initialize(NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX );

	//�X�g���[�����B�摜�p�ƃf�v�X�p
	ImageStream& video = mykinect.VideoStream();
	video.Open( NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480 );

	ImageStream& depth = mykinect.DepthStream();
	depth.Open( NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, NUI_IMAGE_RESOLUTION_320x240 );

	//skeleton���g������
	kinect::nui::SkeletonEngine& skeleton = mykinect.Skeleton();
	skeleton.Enable();

	//opencv��mat��window�̏���
	namedWindow("camera_window");
	camera_img = Mat(Size(video.Width(), video.Height()), CV_8UC4);

	namedWindow("depth_window");
	depth_img = Mat(Size(depth.Width(), depth.Height()), CV_16UC1); //depth��16bit�݂���

	//Depth��camera�̈ʒu���킹(kinect_utility.h)
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

	glClearColor(0.1, 0.1, 0.1, 0.5); //�w�i�F

	#pragma endregion 

	glutMainLoop();

	return 0; // ����I��
}
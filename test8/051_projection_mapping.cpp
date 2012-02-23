#include <iostream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

#include "kinect\nui\Kinect.h"
#include "kinect\nui\ImageFrame.h"
using namespace kinect::nui;

#include "kinect_utility.h"


int main(int argc, char *argv[]){

	#pragma region // --- init ---
	//kinect�N���X�̐錾�Ə�����
	Kinect kinect;
	kinect.Initialize(NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX );

	//�X�g���[�����B�摜�p�ƃf�v�X�p
	ImageStream& video = kinect.VideoStream();
	video.Open( NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480 );

	ImageStream& depth = kinect.DepthStream();
	depth.Open( NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, NUI_IMAGE_RESOLUTION_320x240 );

	//skeleton���g������
	kinect::nui::SkeletonEngine& skeleton = kinect.Skeleton();
	skeleton.Enable();

	//opencv��mat��window�̏���
	namedWindow("camera_window");
	Mat camera_img = Mat(Size(video.Width(), video.Height()), CV_8UC4);

	namedWindow("depth_window");
	Mat depth_img = Mat(Size(depth.Width(), depth.Height()), CV_16UC1); //depth��16bit�݂���

	//Depth��camera�̈ʒu���킹(kinect_utility.h)
	ColorFromDepthEngine CFDengine;

	#pragma endregion


	const int DISPWIDTH = 1500;
	const int DISPHEIGHT = 860;

	#pragma region // --- window init ---
	cv::namedWindow("full");
	cv::setWindowProperty("full", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	cv::setWindowProperty("full", CV_WND_PROP_AUTOSIZE, CV_WINDOW_AUTOSIZE);
	std::cout<<cv::getWindowProperty("full", CV_WND_PROP_AUTOSIZE)<<CV_WINDOW_AUTOSIZE<<std::endl;
	std::cout<<cv::getWindowProperty("full", CV_WND_PROP_FULLSCREEN)<<CV_WINDOW_FULLSCREEN<<std::endl;

	cvMoveWindow("full", -10, -50);
	cv::Point colorpoint(100,100);
	mine::flag hogeflag;
	#pragma endregion

	while ( 1 ) {
		# pragma region // --- get data ---
		// �f�[�^�̍X�V��҂�
		kinect.WaitAndUpdateAll();

		//Skeleton���擾
		kinect::nui::SkeletonFrame skeletonframe= skeleton.GetNextFrame();

		// ���̃t���[���̃f�[�^���擾����(OpenNI���ۂ�)
		ImageFrame image( video );
		DepthFrame depthMD( depth );
		// cv::Mat�ւ̃f�[�^�̃R�s�[			
		camera_img = Mat(camera_img.size(),	CV_8UC4, (BYTE *)image.Bits());
		depth_img = Mat(depth_img.size(), CV_16UC1, (BYTE *)depthMD.Bits());

		// adjust with utility
		cv::Mat adjusted_camera_img(depth_img.size(), CV_8UC4);
		adjusted_camera_img = CFDengine.getAdjustedImage(camera_img);
		#pragma endregion

		#pragma region // --- processing ---
		SkeletonDrawer skeletondrawer(skeletonframe);
		SkeletonPoints me = skeletondrawer.me(depth_img.cols, depth_img.rows); //Depth��ʒ���skeleton���擾
		/*
		if(me.IsTracked){ // ��ʓ��ɐl��������
			//me.Drawall(adjusted_camera_img); // Skeleton ��`��

			// ���E�̎�
			circle(adjusted_camera_img, me.getPoint(NUI_SKELETON_POSITION_HAND_LEFT), 5, cv::Scalar(0,0,255), -1);
			circle(adjusted_camera_img, me.getPoint(NUI_SKELETON_POSITION_HAND_RIGHT), 5, cv::Scalar(0,255,0), -1);
		}
		*/
		#pragma endregion

		#pragma region // --- show ---
		cv::resize(adjusted_camera_img, camera_img, camera_img.size());
		
		cv::Mat proj = cv::Mat( DISPHEIGHT,DISPWIDTH, CV_8UC4, cv::Scalar(0));
		cv::circle(proj, colorpoint, 30, cv::Scalar(0, 0, 255), -1);
		cv::imshow("full", proj);
		
		std::vector<cv::Mat> planes;
		std::vector<cv::Mat> colors;
		for(int i=0;i<3;i++){colors.push_back(cv::Mat(adjusted_camera_img.size(), CV_8UC1));}
		cv::split(adjusted_camera_img, planes);
		
		//cv::threshold(planes[0], colors[0], 200, 255, THRESH_BINARY);
		cv::threshold(planes[0], colors[0], 200, 255, THRESH_BINARY_INV);
		//cv::imshow("B",colors[0]);	
		cv::threshold(planes[1], colors[1], 200, 255, THRESH_BINARY_INV);
		//cv::threshold(planes[1], colors[1], 10, 255, THRESH_BINARY_INV);
		//cv::imshow("G",colors[1]);		
		//cv::threshold(planes[2], colors[2], 200, 255, THRESH_BINARY);
		cv::threshold(planes[2], colors[2], 230, 255, THRESH_BINARY);
		//cv::imshow("R",colors[2]);

		cv::Mat temp = colors[0]&colors[1]&colors[2];

		bool isdetected=false;
		cv::Point redpoint = cv::Point(0,0);
		for(int x=1;x<adjusted_camera_img.cols-1;x++){
			for(int y=1;y<adjusted_camera_img.rows-1;y++){
				if(temp.at<uchar>(y,x)==255){
					if(temp.at<uchar>(y,x-1)==255&&temp.at<uchar>(y,x+1)==255&&
						temp.at<uchar>(y+1,x)==255&&temp.at<uchar>(y-1,x)==255){
							redpoint = cv::Point(x,y);
							isdetected = true;
					}
				}
			}
		}
		/*
		if(hogeflag.IsTrue && isdetected){
			colorpoint.x += (-cv::Point(200,200).x+redpoint.x)*(1.0/3.0);
			colorpoint.y += (cv::Point(200,200).y-redpoint.y)*(1.0/3.0);
			cv::circle(adjusted_camera_img, redpoint, 3, cv::Scalar(0,255,0), -1);
		}
		cv::circle(adjusted_camera_img, cv::Point(200, 200), 3, cv::Scalar(0,255,0), -1);
		*/
		if(isdetected && me.IsTracked){
			colorpoint.x += (-me.getPoint(NUI_SKELETON_POSITION_SPINE).x + redpoint.x)*(1.0/5.0); 
			colorpoint.y += (me.getPoint(NUI_SKELETON_POSITION_SPINE).y - redpoint.y)*(1.0/5.0); 
			cv::circle(adjusted_camera_img, redpoint, 3, cv::Scalar(0,255,0), -1);
			cv::circle(adjusted_camera_img, me.getPoint(NUI_SKELETON_POSITION_SPINE), 3, cv::Scalar(0,255,0), -1);
			
		}
		
		cv::imshow("Bdetect", temp);


		cv::imshow("camera_window", adjusted_camera_img);	

		#pragma endregion

		


		#pragma region // --- keyboard callback ---
		int key = waitKey(1);
		if ( key == 'q' ) {
			break;
		}
		if(key == 'g'){
			hogeflag.reverse();
		}
		#pragma endregion
	
	}
	return 0; // ����I��
}
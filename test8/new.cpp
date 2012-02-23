#include <iostream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

#include "kinect\nui\Kinect.h"
#include "kinect\nui\ImageFrame.h"
using namespace kinect::nui;

#include "kinect_utility.h"

#pragma region // --- constants ---
const double THRESH_SAMEPOS = 30.0; // ����x,y���W���Ƃ݂Ȃ��͈�
const double THRESH_SAMEDEPTH = 1500.0; // ����depth���Ƃ݂Ȃ��͈�
#pragma endregion


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

	#pragma region // --- my init ---
	mine::flag skeleton_flag;
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

		if(skeleton_flag.IsTrue){
			me.Drawall(adjusted_camera_img);
		}
		
		if(me.IsTracked){ // ��ʓ��ɐl��������
			Point prhand = me.getPoint(NUI_SKELETON_POSITION_HAND_RIGHT);
			Point plhand = me.getPoint(NUI_SKELETON_POSITION_HAND_LEFT);

			// �F�̌���
			Scalar color_rhand = Scalar(0, 0, 255);
			Scalar color_lhand = Scalar(0, 255, 0);
			if(norm(prhand-plhand) < THRESH_SAMEPOS){
				ushort drhand = depth_img.at<ushort>(prhand);
				ushort dlhand = depth_img.at<ushort>(plhand);
				if(abs(drhand-dlhand) < THRESH_SAMEDEPTH){
					Scalar mix = color_lhand;
					mix += color_rhand;
					color_rhand = mix;
					color_lhand = mix;
				}
			}

			// �摜�ɓh��
			circle(adjusted_camera_img, prhand, 5, color_rhand, -1);

			if(plhand.x-20>=0 && plhand.x+20<adjusted_camera_img.cols
				&& plhand.y-20>=0 && plhand.y+20 < adjusted_camera_img.rows){
				const float alpha = 0.5;
				for(int x =plhand.x-20; x<plhand.x+20;x++){
					for(int y=plhand.y-20; y<plhand.y+20; y++){
					//���H����������

						adjusted_camera_img.at<Vec4b>(y,x) =   alpha*adjusted_camera_img.at<Vec4b>(y,x);
						adjusted_camera_img.at<Vec4b>(y,x) += (1-alpha)*Vec4b(255,0,0,0);
					}
				}
			}

		}
		#pragma endregion

		#pragma region // --- show ---
		cv::resize(adjusted_camera_img, camera_img, camera_img.size());
		cv::imshow("camera_window", camera_img);	
		#pragma endregion

		#pragma region // --- keyboard callback ---
		int key = waitKey(1);
		if ( key == 'q' ) {
			break;
		}
		else if ( key == 's ') {
			skeleton_flag.reverse();
		}
		#pragma endregion
	
	}
	return 0; // ����I��
}
	
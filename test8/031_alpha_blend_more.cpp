#include <iostream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

#include "kinect\nui\Kinect.h"
#include "kinect\nui\ImageFrame.h"
using namespace kinect::nui;

#include "kinect_utility.h"

#pragma region // --- constants ---
const double THRESH_SAMEPOS = 30.0; // 同じx,y座標だとみなす範囲
const double THRESH_SAMEDEPTH = 1500.0; // 同じdepthだとみなす範囲
#pragma endregion


int main(int argc, char *argv[]){

	#pragma region // --- init ---
	//kinectクラスの宣言と初期化
	Kinect kinect;
	kinect.Initialize(NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX );

	//ストリーム作る。画像用とデプス用
	ImageStream& video = kinect.VideoStream();
	video.Open( NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480 );

	ImageStream& depth = kinect.DepthStream();
	depth.Open( NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, NUI_IMAGE_RESOLUTION_320x240 );

	//skeletonを使う準備
	kinect::nui::SkeletonEngine& skeleton = kinect.Skeleton();
	skeleton.Enable();

	//opencvのmatとwindowの準備
	namedWindow("camera_window");
	Mat camera_img = Mat(Size(video.Width(), video.Height()), CV_8UC4);

	namedWindow("depth_window");
	Mat depth_img = Mat(Size(depth.Width(), depth.Height()), CV_16UC1); //depthは16bitみたい

	//Depthとcameraの位置合わせ(kinect_utility.h)
	ColorFromDepthEngine CFDengine;

	#pragma endregion

	#pragma region // --- my init ---
	mine::flag skeleton_flag;
	
	// 一時的に絵を書くためのバッファ。透過色(黒)で塗りつぶしておく
	cv::Mat buf_img(depth_img.size(), CV_8UC4, cv::Scalar(0));
	#pragma endregion


	while ( 1 ) {
		# pragma region // --- get data ---
		// データの更新を待つ
		kinect.WaitAndUpdateAll();

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

		if(skeleton_flag.IsTrue){
			me.Drawall(adjusted_camera_img);
		}
		
		if(me.IsTracked){ // 画面内に人がいたら
			Point prhand = me.getPoint(NUI_SKELETON_POSITION_HAND_RIGHT);
			Point plhand = me.getPoint(NUI_SKELETON_POSITION_HAND_LEFT);

			// 色の決定
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

			#pragma region // --- painting ---

			circle(buf_img, prhand, 5, color_rhand, -1);
			circle(buf_img, plhand, 5, color_lhand, -1);

			AlphaCopy(buf_img, adjusted_camera_img, 0.5);

			// 残像
			Mat tempbuf_img(buf_img.size(), CV_8UC4, Scalar(0) );
			AlphaCopy(buf_img, tempbuf_img, 0.9);
			buf_img=tempbuf_img;

			#pragma endregion
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
		else if ( key == 's' ) {
			skeleton_flag.reverse();
		}
		#pragma endregion
	
	}
	return 0; // 正常終了
}
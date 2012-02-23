#include <iostream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

#include "kinect\nui\Kinect.h"
#include "kinect\nui\ImageFrame.h"
using namespace kinect::nui;

#include "kinect_utility.h"

#pragma region // --- constants ---
const double THRESH_SAMEPOS = 10.0; // 同じx,y座標だとみなす範囲
const double THRESH_SAMEDEPTH = 1500.0; // 同じdepthだとみなす範囲
#pragma endregion

namespace mine{
	class ThrownObject{
	private:
		Vec3i delta;
		Vec3i plast;
		Mat img;
	public:
		Vec3i pnow;
		bool IsExist;
		ThrownObject(){
			IsExist = false;
		}
		ThrownObject(Mat _img, Point point_start, int depth_start, Point point_delta, int depth_delta){
			img = _img;
			pnow[0] = point_start.x;
			pnow[1] = point_start.y;
			pnow[2] = depth_start;
			delta[0] = point_delta.x;
			delta[1] = point_delta.y;
			delta[2] = depth_delta;
			plast = pnow;
			IsExist = true;
		}
		void gonext(){
			plast = pnow;
			pnow[0] = plast[0] + delta[0];
			pnow[1] = plast[1] + delta[1];
			pnow[2] = plast[2] + delta[2];
			checkIsExist();
		}
		void checkIsExist(){
			std::cout<<pnow[2]<<std::endl;
			if(pnow[0]>=0 && pnow[1]>=0 && pnow[2]>=0 && pnow[0]<img.cols && pnow[1]<img.rows ){
				IsExist = true;
			}
			else IsExist = false;
		}
		void drawBall(Mat draw_img, Scalar color = Scalar(0,255,255) ){
			line(draw_img, Point(plast[0], plast[1]), Point(pnow[0], pnow[1]), color, 20);
			circle(draw_img, Point(pnow[0], pnow[1]), 20, color, -1);
		}
	};
}

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
	cv::Mat buf_img(depth_img.size(), CV_8UC4, Scalar(0));
	cv::Mat buf_depth_img(depth_img.size(), CV_16UC1, Scalar(0));

	// 右手のエフェクト
	float rballsize = 3.0;
	Point prhand_prev(0, 0);
	ushort drhand_prev = 0;
	mine::ThrownObject rball;
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

			Scalar color_rhand = Scalar(0, 0, 255);
			Scalar color_lhand = Scalar(0, 255, 0);
	
			#pragma region // --- shoot ---
			if(  prhand.inside(Rect(0, 0, depth_img.cols, depth_img.rows)) ){
				ushort drhand = depth_img.at<ushort>(prhand);
				if((norm(prhand-prhand_prev) < THRESH_SAMEPOS) && abs(drhand - drhand_prev) < THRESH_SAMEDEPTH){
					rballsize += 0.5;
					if(rballsize > 10){ // 十分大きくなったら
						rballsize = 20;
					}
				}
				else{
					if(rballsize == 20){ // チャージ後初めて動いた
						std::cout<<"start"<<drhand<<"delta"<<drhand-drhand_prev<<std::endl;
						rball = mine::ThrownObject(depth_img, prhand, drhand, prhand-prhand_prev, drhand-drhand_prev);
						//rball = mine::ThrownObject(depth_img, prhand, depth_img.at<ushort>(me.getPoint(NUI_SKELETON_POSITION_SPINE)), prhand-prhand_prev, 0);
					}
					rballsize = 3.0;
				}
				// 更新
				prhand_prev = prhand;
				drhand_prev = drhand;
			}

			// ボールの描画
			if(rball.IsExist){
				Mat rball_img(depth_img.size(), CV_8UC4, Scalar(0));
				Mat rball_depth_img(depth_img.size(), CV_16UC1, Scalar(0));

				rball.drawBall(rball_img);
				SetDepth(rball_img, rball_depth_img, rball.pnow[2]);

				DepthBlend(buf_img, buf_depth_img, rball_img, rball_depth_img);

				rball.gonext();
			}

			#pragma endregion
			
			#pragma region // --- painting ---


			Mat hands_img(depth_img.size(), CV_8UC4, Scalar(0));
			Mat hands_depth_img(depth_img.size(), CV_16UC1, Scalar(0) );

			circle(hands_img, prhand, rballsize, color_rhand, -1);
			circle(hands_img, plhand, 5, color_lhand, -1);
			circle(adjusted_camera_img, prhand, rballsize, color_rhand, -1);
			circle(adjusted_camera_img, plhand, 5, color_lhand, -1);

			SetDepth(hands_img, hands_depth_img, drhand_prev);
			cv::imshow("hands_depth", hands_depth_img);
			DepthBlend(buf_img, buf_depth_img, hands_img, hands_depth_img);

			Mat alpha_buf_img = adjusted_camera_img.clone();
			AlphaCopy(buf_img, alpha_buf_img, 0.5);
			cv::imshow("alphabuf", alpha_buf_img);

			cv::imshow("buf_depth", buf_depth_img);
			DepthBlend(adjusted_camera_img, depth_img, alpha_buf_img, buf_depth_img);



			// 残像
			Mat tempbuf_img(buf_img.size(), CV_8UC4, Scalar(0) );
			AlphaCopy(buf_img, tempbuf_img, 0.9);
		
			buf_img=tempbuf_img;
			DepthMasking(buf_img, buf_depth_img);
			cv::imshow("buf_depth", buf_depth_img);
			cv::imshow("buf", buf_img);

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
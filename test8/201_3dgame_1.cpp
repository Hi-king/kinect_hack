#include <iostream>
using namespace std;
 
#include <opencv2/opencv.hpp>
using namespace cv;

#include <gl\glut.h>
 
#include "kinect\nui\Kinect.h"
#include "kinect\nui\ImageFrame.h"
using namespace kinect::nui;
 
#include "kinect_utility.h"



namespace mine{
	class ThrownObject{
	public:
		Vec3i delta;
		Vec3i plast;
		Vec3i pnow;
		Mat img;

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
			pnow[0] += delta[0];
			pnow[1] += delta[1];
			pnow[2] += delta[2];
			plast = pnow;
			glTranslatef(pnow[0],pnow[1],pnow[2]); //drawBallへ
			checkIsExist();
		}
		void checkIsExist(){
			if(pnow[0]>0 && pnow[1]>0 && pnow[2]>0 && pnow[0]<img.cols && pnow[1]<img.rows ){
				IsExist = true;
			}
			else IsExist = false;
		}

		/* 　大垣コメント
		glutSolidSphereはglの関数なので、drawBallにdraw_imgを引数にとる必要はない。
		また、gonext()に書いてあるglTranslatefだが、
		gonext()のところに描画するのではなく、glutSolidSphereの直前に書いたほうがよい。
		gonext()からdrawBall()の間に他の物体を書こうとすると、その物体がx,y,zの位置に書かれてしまう。
		ちなみにpnowとglTranslatefは座標系が違うことに注意。右手系と左手系
		*/

		void drawBall(Mat draw_img, Scalar color = Scalar(0,255,255) ){
			glutSolidSphere(2,5,5);
			//line(draw_img, Point(plast[0], plast[1]), Point(pnow[0], pnow[1]), color, 20);
			//circle(draw_img, Point(pnow[0], pnow[1]), 20, color, -1);
		}
	};
	
}


#pragma region // --- constants ---
const double THRESH_SAMEPOS = 10.0; // 同じx,y座標だとみなす範囲
const double THRESH_SAMEDEPTH = 1500.0; // 同じdepthだとみなす範囲
const double MY_MINDISTANCE = 0.1; // カメラから[0.1, 100.0]の範囲しか描画しない
const double MY_MAXDISTANCE = 100000.0;
const double CAMERA_DISTANCE = 1.5; //カメラと座標系の距離
#pragma endregion
 

#pragma region // --- global variables
int a=10;
Kinect mykinect;
Mat camera_img;
Mat depth_img;
ColorFromDepthEngine CFDengine;
	
#pragma endregion
 
#pragma region // --- my init ---
	mine::flag skeleton_flag;
	
	// 一時的に絵を書くためのバッファ。透過色(黒)で塗りつぶしておく
	cv::Mat buf_img;
 
	// 右手のエフェクト
	float rballsize = 3.0;
	Point prhand_prev(0, 0);
	uchar drhand_prev = 0;
	mine::ThrownObject rball;
#pragma endregion

 

void loop(){
 
	#pragma region // --- init loop
	ImageStream& video = mykinect.VideoStream();
	ImageStream& depth = mykinect.DepthStream();
	kinect::nui::SkeletonEngine& skeleton = mykinect.Skeleton();
	//ColorFromDepthEngine CFDengine;
	#pragma endregion
 
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

		if(skeleton_flag.IsTrue){
			if(me.IsTracked) me.Drawall(adjusted_camera_img);
		}
		
		if(me.IsTracked){ // 画面内に人がいたら
			Point prhand = me.getPoint(NUI_SKELETON_POSITION_HAND_RIGHT);
			Point plhand = me.getPoint(NUI_SKELETON_POSITION_HAND_LEFT);
 
			Scalar color_rhand = Scalar(0, 0, 255);
			Scalar color_lhand = Scalar(0, 255, 0);
	
			#pragma region // --- shoot ---
			if(  prhand.inside(Rect(0, 0, depth_img.cols, depth_img.rows)) ){
				uchar drhand = depth_img.at<ushort>(prhand);
				if((norm(prhand-prhand_prev) < THRESH_SAMEPOS) && abs(drhand - drhand_prev) < THRESH_SAMEDEPTH){
					rballsize += 0.5;
					if(rballsize > 10){ // 十分大きくなったら
						rballsize = 20;
					}
				}
				else{
					if(rballsize == 20){ // チャージ後初めて動いた
						rball = mine::ThrownObject(depth_img, prhand, drhand, prhand-prhand_prev, 0);
					}
					rballsize = 3.0;
				}
				// 更新
				prhand_prev = prhand;
				drhand_prev = drhand;
			}
 
			/* GLのセクションへ移動(ogaki)
			// ボールの描画
			if(rball.IsExist){
				rball.drawBall(buf_img);
				rball.gonext();
			}
			*/
 
			#pragma endregion
			
			#pragma region // --- painting ---
 
			circle(buf_img, prhand, rballsize, color_rhand, -1);
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

	
	#pragma region // --- drawing ---
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable(GL_DEPTH_TEST);

	glColor3d(1.0, 0, 0);

	if(rball.IsExist){
		rball.gonext();
		rball.drawBall(buf_img);
	}

	glFlush();
	
	glDisable(GL_DEPTH_TEST);
	glutSwapBuffers();
	#pragma endregion	

	#pragma endregion

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


	#pragma region // --- my init ---
	mine::flag skeleton_flag;
	
	// 一時的に絵を書くためのバッファ。透過色(黒)で塗りつぶしておく
	buf_img = Mat(depth_img.size(), CV_8UC4, Scalar(0) );
 
	// 右手のエフェクト
	rballsize = 3.0;
	prhand_prev = Point(0, 0);
	drhand_prev = 0;
	#pragma endregion


	glutMainLoop();
 
	return 0; // 正常終了
}
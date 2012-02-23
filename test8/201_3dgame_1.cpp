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
			glTranslatef(pnow[0],pnow[1],pnow[2]); //drawBall��
			checkIsExist();
		}
		void checkIsExist(){
			if(pnow[0]>0 && pnow[1]>0 && pnow[2]>0 && pnow[0]<img.cols && pnow[1]<img.rows ){
				IsExist = true;
			}
			else IsExist = false;
		}

		/* �@��_�R�����g
		glutSolidSphere��gl�̊֐��Ȃ̂ŁAdrawBall��draw_img�������ɂƂ�K�v�͂Ȃ��B
		�܂��Agonext()�ɏ����Ă���glTranslatef�����A
		gonext()�̂Ƃ���ɕ`�悷��̂ł͂Ȃ��AglutSolidSphere�̒��O�ɏ������ق����悢�B
		gonext()����drawBall()�̊Ԃɑ��̕��̂��������Ƃ���ƁA���̕��̂�x,y,z�̈ʒu�ɏ�����Ă��܂��B
		���Ȃ݂�pnow��glTranslatef�͍��W�n���Ⴄ���Ƃɒ��ӁB�E��n�ƍ���n
		*/

		void drawBall(Mat draw_img, Scalar color = Scalar(0,255,255) ){
			glutSolidSphere(2,5,5);
			//line(draw_img, Point(plast[0], plast[1]), Point(pnow[0], pnow[1]), color, 20);
			//circle(draw_img, Point(pnow[0], pnow[1]), 20, color, -1);
		}
	};
	
}


#pragma region // --- constants ---
const double THRESH_SAMEPOS = 10.0; // ����x,y���W���Ƃ݂Ȃ��͈�
const double THRESH_SAMEDEPTH = 1500.0; // ����depth���Ƃ݂Ȃ��͈�
const double MY_MINDISTANCE = 0.1; // �J��������[0.1, 100.0]�͈̔͂����`�悵�Ȃ�
const double MY_MAXDISTANCE = 100000.0;
const double CAMERA_DISTANCE = 1.5; //�J�����ƍ��W�n�̋���
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
	
	// �ꎞ�I�ɊG���������߂̃o�b�t�@�B���ߐF(��)�œh��Ԃ��Ă���
	cv::Mat buf_img;
 
	// �E��̃G�t�F�N�g
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

		if(skeleton_flag.IsTrue){
			if(me.IsTracked) me.Drawall(adjusted_camera_img);
		}
		
		if(me.IsTracked){ // ��ʓ��ɐl��������
			Point prhand = me.getPoint(NUI_SKELETON_POSITION_HAND_RIGHT);
			Point plhand = me.getPoint(NUI_SKELETON_POSITION_HAND_LEFT);
 
			Scalar color_rhand = Scalar(0, 0, 255);
			Scalar color_lhand = Scalar(0, 255, 0);
	
			#pragma region // --- shoot ---
			if(  prhand.inside(Rect(0, 0, depth_img.cols, depth_img.rows)) ){
				uchar drhand = depth_img.at<ushort>(prhand);
				if((norm(prhand-prhand_prev) < THRESH_SAMEPOS) && abs(drhand - drhand_prev) < THRESH_SAMEDEPTH){
					rballsize += 0.5;
					if(rballsize > 10){ // �\���傫���Ȃ�����
						rballsize = 20;
					}
				}
				else{
					if(rballsize == 20){ // �`���[�W�㏉�߂ē�����
						rball = mine::ThrownObject(depth_img, prhand, drhand, prhand-prhand_prev, 0);
					}
					rballsize = 3.0;
				}
				// �X�V
				prhand_prev = prhand;
				drhand_prev = drhand;
			}
 
			/* GL�̃Z�N�V�����ֈړ�(ogaki)
			// �{�[���̕`��
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
 
			// �c��
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

	#pragma region // --- ���_���� ---
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30.0, 1.0, MY_MINDISTANCE, MY_MAXDISTANCE);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//�J�����̍��W, ���_, ���W�n�̌���
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


	#pragma region // --- my init ---
	mine::flag skeleton_flag;
	
	// �ꎞ�I�ɊG���������߂̃o�b�t�@�B���ߐF(��)�œh��Ԃ��Ă���
	buf_img = Mat(depth_img.size(), CV_8UC4, Scalar(0) );
 
	// �E��̃G�t�F�N�g
	rballsize = 3.0;
	prhand_prev = Point(0, 0);
	drhand_prev = 0;
	#pragma endregion


	glutMainLoop();
 
	return 0; // ����I��
}
#include <opencv2/opencv.hpp>
using namespace cv;

#include "kinect\nui\Kinect.h"
#include "kinect\nui\ImageFrame.h"
using namespace kinect::nui;


//デプスにあわせてカメラ画像を微調整するためのクラス
//これが無いと、デプスとカメラ画像が微妙にずれる
//内部的にはテーブルを呼び出してインデックスの対応付けを行う
//テーブルは実は静的に事前に計算できるため、クラスのコンストラクトのときに勝手にやってもらう
//微調整のためには、入力はカメラ画像(640, 480, CV_8UC4が必須)
//出力は自動的に(320, 240, CV_8UC4)の画像になる。デプスサイズに合わせるため画像サイズが小さくなる
class ColorFromDepthEngine{
private:
	Mat map_img;
public:
	ColorFromDepthEngine(){
		map_img = Mat(Size(320, 240), CV_32SC2);
		for(int j=0; j<240; j++){
			for(int i=0; i<320; i++){
				long temp_x = 0;
				long temp_y = 0;
				NuiImageGetColorPixelCoordinatesFromDepthPixel(NUI_IMAGE_RESOLUTION_640x480, new NUI_IMAGE_VIEW_AREA(), i, j, 0, &temp_x, &temp_y);
				map_img.at<Vec2i>(Point(i, j))[0] = (int)temp_x;
				map_img.at<Vec2i>(Point(i, j))[1] = (int)temp_y;
			}
		}
	}
	Mat getAdjustedImage(const Mat &src_img){
		try{
			CV_Assert(src_img.size() == Size(640, 480));
			CV_Assert(src_img.type() == CV_8UC4);
		}catch(Exception &e){
			cout << "getAdjustedIMage input error" <<e.msg<< endl;
		}
		Mat dst_img = Mat(Size(320, 240), CV_8UC4);
		for(int j=0; j<240; j++){
			for(int i=0; i<320; i++){
				Vec2i map_elem = map_img.at<Vec2i>(Point(i, j));
				if(map_elem[0]<640 && map_elem[1]<480){
					dst_img.at<Vec4b>(Point(i, j)) = src_img.at<Vec4b>(Point(map_elem[0], map_elem[1]));
				}
				else{
					dst_img.at<Vec4b>(Point(i, j)) = Scalar(0, 0, 0, 0);
				}
			}
		}
		return dst_img;
	}

};

#define HANDSCALE 1.5

class SkeletonPoints{
public:
	bool IsTracked;
	int me_index;
	std::vector<cv::Point> points;
	std::vector<Vector4> rawpoints;

	SkeletonPoints(){
		IsTracked = false;
		me_index=0;
	}	
	SkeletonPoints(bool _IsTracked){
		IsTracked = _IsTracked;
	}
	SkeletonPoints(std::vector<Vector4> _rawpoints, std::vector<cv::Point> _points, bool _IsTracked) : points(_points), rawpoints(_rawpoints){
		IsTracked= _IsTracked;
		me_index=0;
	}
	SkeletonPoints(std::vector<Vector4> _rawpoints, std::vector<cv::Point> _points, bool _IsTracked, int index) : points(_points), rawpoints(_rawpoints){
		IsTracked= _IsTracked;
		me_index=index;
	}

	void Drawall(cv::Mat img){
		cv::Scalar color = cv::Scalar(0,0,255);

		cv::line(img, points[NUI_SKELETON_POSITION_HIP_CENTER], points[NUI_SKELETON_POSITION_SPINE] , color);
		cv::line(img, points[NUI_SKELETON_POSITION_SPINE], points[NUI_SKELETON_POSITION_SHOULDER_CENTER] , color);
		cv::line(img, points[NUI_SKELETON_POSITION_SHOULDER_CENTER], points[NUI_SKELETON_POSITION_HEAD] , color);

		cv::line(img, points[NUI_SKELETON_POSITION_SHOULDER_CENTER], points[NUI_SKELETON_POSITION_SHOULDER_LEFT] , color);
		cv::line(img, points[NUI_SKELETON_POSITION_SHOULDER_LEFT], points[NUI_SKELETON_POSITION_ELBOW_LEFT] , color);
		cv::line(img, points[NUI_SKELETON_POSITION_ELBOW_LEFT], points[NUI_SKELETON_POSITION_WRIST_LEFT] , color);
		cv::line(img, points[NUI_SKELETON_POSITION_WRIST_LEFT], points[NUI_SKELETON_POSITION_HAND_LEFT] , color);

		cv::line(img, points[NUI_SKELETON_POSITION_SHOULDER_CENTER], points[NUI_SKELETON_POSITION_SHOULDER_RIGHT] , color);
		cv::line(img, points[NUI_SKELETON_POSITION_SHOULDER_RIGHT], points[NUI_SKELETON_POSITION_ELBOW_RIGHT] , color);
		cv::line(img, points[NUI_SKELETON_POSITION_ELBOW_RIGHT], points[NUI_SKELETON_POSITION_WRIST_RIGHT] , color);
		cv::line(img, points[NUI_SKELETON_POSITION_WRIST_RIGHT], points[NUI_SKELETON_POSITION_HAND_RIGHT] , color);

		cv::line(img, points[NUI_SKELETON_POSITION_HIP_CENTER], points[NUI_SKELETON_POSITION_HIP_LEFT] , color);
		cv::line(img, points[NUI_SKELETON_POSITION_HIP_LEFT], points[NUI_SKELETON_POSITION_KNEE_LEFT] , color);
		cv::line(img, points[NUI_SKELETON_POSITION_KNEE_LEFT], points[NUI_SKELETON_POSITION_ANKLE_LEFT] , color);
		cv::line(img, points[NUI_SKELETON_POSITION_ANKLE_LEFT], points[NUI_SKELETON_POSITION_FOOT_LEFT] , color);

		cv::line(img, points[NUI_SKELETON_POSITION_HIP_CENTER], points[NUI_SKELETON_POSITION_HIP_RIGHT] , color);
		cv::line(img, points[NUI_SKELETON_POSITION_HIP_RIGHT], points[NUI_SKELETON_POSITION_KNEE_RIGHT] , color);
		cv::line(img, points[NUI_SKELETON_POSITION_KNEE_RIGHT], points[NUI_SKELETON_POSITION_ANKLE_RIGHT] , color);
		cv::line(img, points[NUI_SKELETON_POSITION_ANKLE_RIGHT], points[NUI_SKELETON_POSITION_FOOT_RIGHT] , color);
	}

	cv::Point getPoint(int index){
		/*if(index == NUI_SKELETON_POSITION_HAND_LEFT){
			return cv::Point(
				(points.at(NUI_SKELETON_POSITION_WRIST_LEFT).x-points.at(NUI_SKELETON_POSITION_ELBOW_LEFT).x)*HANDSCALE + points.at(NUI_SKELETON_POSITION_ELBOW_LEFT).x,
				(points.at(NUI_SKELETON_POSITION_WRIST_LEFT).y-points.at(NUI_SKELETON_POSITION_ELBOW_LEFT).y)*HANDSCALE + points.at(NUI_SKELETON_POSITION_ELBOW_LEFT).y
				);
		}
		else if(index == NUI_SKELETON_POSITION_HAND_RIGHT){
			return cv::Point(
				(points.at(NUI_SKELETON_POSITION_WRIST_RIGHT).x-points.at(NUI_SKELETON_POSITION_ELBOW_RIGHT).x)*HANDSCALE + points.at(NUI_SKELETON_POSITION_ELBOW_RIGHT).x,
				(points.at(NUI_SKELETON_POSITION_WRIST_RIGHT).y-points.at(NUI_SKELETON_POSITION_ELBOW_RIGHT).y)*HANDSCALE + points.at(NUI_SKELETON_POSITION_ELBOW_RIGHT).y
				);
		}
		else{
			return points.at(index);
		
		}*/
		return points.at(index);
	}
	Vector4 getPoint3d(int index){
		return rawpoints.at(index);
	}

	cv::Vec3i getglPoint(int index, cv::Mat depth){
		return cv::Vec3i(points[index].x, -points[index].y, -depth.at<ushort>(points[index].y,points[index].x)/255 );
	}

	void Drawline(cv::Mat img, int start, int end){
		cv::Scalar color = cv::Scalar(0, 0, 255);
		cv::line(img, points.at(start), points.at(end), color);
	}
	void Drawline(cv::Mat img, int start, int end, cv::Scalar color, int thickness){
		cv::line(img, points.at(start), points.at(end), color, thickness);
	}

};


class SkeletonDrawer{
private:
	kinect::nui::SkeletonFrame skeletonMD;
public:
	SkeletonDrawer(kinect::nui::SkeletonFrame _skeletonMD) : skeletonMD(_skeletonMD){}
	int me_index;

	void Draw(cv::Mat img, float alpha, int width, int height){
		cv::Scalar color(0,0,255);
		//cv::Mat img_temp(img.rows, img.cols, img.type(), cv::Scalar(255,255,255));
	
		for(int i=0; i<skeletonMD.SKELETON_COUNT; i++){
			if(skeletonMD[i].TrackingState() == NUI_SKELETON_TRACKED){
			
				//point 取得
				cv::Point points[ kinect::nui::SkeletonData::POSITION_COUNT ];
				for(int j=0; j<kinect::nui::SkeletonData::POSITION_COUNT; j++){
					kinect::nui::SkeletonData::Point p = skeletonMD[i].TransformSkeletonToDepthImage(j);			
					points[j]=cv::Point(p.x * width + 0.5, p.y * height +0.5);
				}	

				cv::line(img, points[NUI_SKELETON_POSITION_HIP_CENTER], points[NUI_SKELETON_POSITION_SPINE] , color);
				cv::line(img, points[NUI_SKELETON_POSITION_SPINE], points[NUI_SKELETON_POSITION_SHOULDER_CENTER] , color);
				cv::line(img, points[NUI_SKELETON_POSITION_SHOULDER_CENTER], points[NUI_SKELETON_POSITION_HEAD] , color);

				cv::line(img, points[NUI_SKELETON_POSITION_SHOULDER_CENTER], points[NUI_SKELETON_POSITION_SHOULDER_LEFT] , color);
				cv::line(img, points[NUI_SKELETON_POSITION_SHOULDER_LEFT], points[NUI_SKELETON_POSITION_ELBOW_LEFT] , color);
				cv::line(img, points[NUI_SKELETON_POSITION_ELBOW_LEFT], points[NUI_SKELETON_POSITION_WRIST_LEFT] , color);
				cv::line(img, points[NUI_SKELETON_POSITION_WRIST_LEFT], points[NUI_SKELETON_POSITION_HAND_LEFT] , color);

				cv::line(img, points[NUI_SKELETON_POSITION_SHOULDER_CENTER], points[NUI_SKELETON_POSITION_SHOULDER_RIGHT] , color);
				cv::line(img, points[NUI_SKELETON_POSITION_SHOULDER_RIGHT], points[NUI_SKELETON_POSITION_ELBOW_RIGHT] , color);
				cv::line(img, points[NUI_SKELETON_POSITION_ELBOW_RIGHT], points[NUI_SKELETON_POSITION_WRIST_RIGHT] , color);
				cv::line(img, points[NUI_SKELETON_POSITION_WRIST_RIGHT], points[NUI_SKELETON_POSITION_HAND_RIGHT] , color);

				cv::line(img, points[NUI_SKELETON_POSITION_HIP_CENTER], points[NUI_SKELETON_POSITION_HIP_LEFT] , color);
				cv::line(img, points[NUI_SKELETON_POSITION_HIP_LEFT], points[NUI_SKELETON_POSITION_KNEE_LEFT] , color);
				cv::line(img, points[NUI_SKELETON_POSITION_KNEE_LEFT], points[NUI_SKELETON_POSITION_ANKLE_LEFT] , color);
				cv::line(img, points[NUI_SKELETON_POSITION_ANKLE_LEFT], points[NUI_SKELETON_POSITION_FOOT_LEFT] , color);

				cv::line(img, points[NUI_SKELETON_POSITION_HIP_CENTER], points[NUI_SKELETON_POSITION_HIP_RIGHT] , color);
				cv::line(img, points[NUI_SKELETON_POSITION_HIP_RIGHT], points[NUI_SKELETON_POSITION_KNEE_RIGHT] , color);
				cv::line(img, points[NUI_SKELETON_POSITION_KNEE_RIGHT], points[NUI_SKELETON_POSITION_ANKLE_RIGHT] , color);
				cv::line(img, points[NUI_SKELETON_POSITION_ANKLE_RIGHT], points[NUI_SKELETON_POSITION_FOOT_RIGHT] , color);
			}
		}
	}
	void Draw(cv::Mat img){
		Draw(img, 1.0, img.cols, img.rows);
	}
	
	SkeletonPoints me(int width, int height, int myindex){
		std::vector<cv::Point> points;
		me_index=100;
		if(skeletonMD[myindex].TrackingState() == NUI_SKELETON_TRACKED){
			me_index=myindex;
		}
		else{
			int j=me_index;
			for(int i=0; i<skeletonMD.SKELETON_COUNT; i++){
				if(skeletonMD[j].TrackingState() == NUI_SKELETON_TRACKED){
					me_index=j;
					break;
				}
				j++;
				if(j==skeletonMD.SKELETON_COUNT)j=0;
			}
		}
		if( me_index == 100){ // No skeleton
			//std::cerr<<"errror : can't get my skeleton"<<std::endl;
			return SkeletonPoints(false);		
		}
		
		for(int j=0; j<kinect::nui::SkeletonData::POSITION_COUNT; j++){
			kinect::nui::SkeletonData::Point p = skeletonMD[me_index].TransformSkeletonToDepthImage(j);
			cv::Point nowp = cv::Point(p.x * width + 0.5, p.y * height +0.5);
			
			if(nowp.y >= height)nowp.y = height-1;
			if(nowp.y < 0) nowp.y = 0;
			if(nowp.x >= width) nowp.x = width-1;
			if(nowp.x < 0) nowp.x =0;
			points.push_back(nowp);
		}			

		std::vector<Vector4> rawpoints;		
		for(int j=0; j<kinect::nui::SkeletonData::POSITION_COUNT; j++){
			rawpoints.push_back(skeletonMD[me_index][j]);
		}

		return SkeletonPoints(rawpoints, points, true, me_index);
		
	}
	SkeletonPoints me(int width, int height){
		return me(width, height, 1);
	}
};


void SetAlpha(cv::Mat img, cv::Mat alpha_img, float alpha, cv::Scalar invisible = cv::Scalar(255,255,255) ){
	for(int x=0; x<img.cols; x++){
		for(int y=0; y<img.rows; y++){
			cv::Vec4b nowvec = img.at<cv::Vec4b>(y,x);
			if(nowvec[0]==invisible[0] && nowvec[1]==invisible[1] && nowvec[2]==invisible[2]){ //透過色なら
				alpha_img.at<float>(y,x) = 0.0;
			}
			else{
				alpha_img.at<float>(y,x) = alpha;
			}
		}
	}
}
void AlphaBlend(cv::Mat srcimg, cv::Mat targetimg, cv::Mat srcalpha, cv::Mat targetalpha){
	//アルファブレンド　http://ja.wikipedia.org/wiki/アルファブレンド
	for(int x=0; x<targetimg.cols; x++){
		for(int y=0; y<targetimg.rows; y++){
			float alpha1 = srcalpha.at<float>(y,x);
			float alpha2 = targetalpha.at<float>(y,x);
			float nextalpha = alpha1 + alpha2*(1-alpha1);
			
			
			targetalpha.at<float>(y,x) = nextalpha;
			if(nextalpha > 0.05){
				cv::Vec4b vec1 = srcimg.at<cv::Vec4b>(y,x);
				cv::Vec4b vec2 = targetimg.at<cv::Vec4b>(y,x);
				
				targetimg.at<cv::Vec4b>(y,x)[0] = vec1[0]*(alpha1/nextalpha) + vec2[0]*((alpha2*(1-alpha1)/nextalpha));
				targetimg.at<cv::Vec4b>(y,x)[1] = vec1[1]*(alpha1/nextalpha) + vec2[1]*((alpha2*(1-alpha1)/nextalpha));
				targetimg.at<cv::Vec4b>(y,x)[2] = vec1[2]*(alpha1/nextalpha) + vec2[2]*((alpha2*(1-alpha1)/nextalpha));
			}else{
				targetimg.at<cv::Vec4b>(y,x) = cv::Scalar(255,255,255,0);
			}

		}
	}
}

void AlphaPlus(cv::Mat alphaimg, float plus){
	for(int x =0; x<alphaimg.cols; x++){
		for(int y=0; y<alphaimg.rows; y++){
			float now = alphaimg.at<float>(y,x);
			if(now+plus>1.0){
				alphaimg.at<float>(y,x) = 1.0;
			}
			else if(now+plus<0){
				alphaimg.at<float>(y,x) = 0.0;
			}
			else{
				alphaimg.at<float>(y,x) = now+plus;
			}
		}
	}	
}

void AlphaCopy(cv::Mat src_img, cv::Mat target_img, double alpha){
	cv::Mat tempgry_img(src_img.size(), CV_8UC1);
	cv::Mat mask_img(src_img.size(), CV_8UC1);
	cv::cvtColor(src_img, tempgry_img, CV_BGR2GRAY);
	cv::threshold(tempgry_img, mask_img, 5, 255, THRESH_BINARY);
	//cv::imshow("mask", mask_img);

	cv::Mat buf_img(target_img.size(), CV_8UC4);
	cv::addWeighted(src_img, alpha, target_img, 1.0-alpha, 0, buf_img);
	buf_img.copyTo(target_img, mask_img);

}

void SetDepth(cv::Mat img, cv::Mat depth_img, ushort depth){
	cv::Mat temp_depth(depth_img.size(), CV_16UC1, cv::Scalar(depth));

	cv::Mat tempgry_img(img.size(), CV_8UC1);
	cv::Mat mask_img(img.size(), CV_8UC1);
	cv::cvtColor(img, tempgry_img, CV_BGR2GRAY);
	cv::threshold(tempgry_img, mask_img, 5, 255, THRESH_BINARY);

	temp_depth.copyTo(depth_img, mask_img);
}

void DepthMasking(cv::Mat img, cv::Mat depth_img){
	cv::Mat temp_depth_img(depth_img.size(), CV_16UC1, cv::Scalar(0));

	cv::Mat tempgry_img(img.size(), CV_8UC1);
	cv::Mat mask_img(img.size(), CV_8UC1);
	cv::cvtColor(img, tempgry_img, CV_BGR2GRAY);
	cv::threshold(tempgry_img, mask_img, 5, 255, THRESH_BINARY);

	depth_img.clone().copyTo(temp_depth_img, mask_img);
	temp_depth_img.copyTo(depth_img);


}

void DepthBlend(cv::Mat target_img, cv::Mat target_depth_img, cv::Mat blend_img, cv::Mat blend_depth_img){
	for(int x=0; x<blend_img.cols; x++){
		for(int y=0; y<blend_img.rows; y++){
			ushort nowdepth = blend_depth_img.at<ushort>(y,x);
			if(nowdepth>0){
				if(nowdepth < target_depth_img.at<ushort>(y,x) || target_depth_img.at<ushort>(y,x)==0){
					target_depth_img.at<ushort>(y,x) = nowdepth;
					target_img.at<Vec4b>(y,x) = blend_img.at<Vec4b>(y,x);
				}									
			}
		}
	}
}

namespace mine{
	class flag{
	public:
		bool IsTrue;
		flag(){
			IsTrue = false;
		}
		flag(bool _IsTrue){
			IsTrue = _IsTrue;
		}
		bool reverse(){
			if(IsTrue)IsTrue = false;
			else IsTrue = true;
			return IsTrue;
		}
	};
}
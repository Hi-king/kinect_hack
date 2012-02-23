#include <opencv2/opencv.hpp>
#include "kinect\nui\Kinect.h"
#include "kinect\nui\ImageFrame.h"

//#define NUI_SKELETON_POSITION_HAND_LEFT 15
//#define NUI_SKELETON_POSITION_HAND_RIGHT 16
#define HANDSCALE 1.5

class SkeletonPoints{
private:
	//bool IsTracked;
public:
	bool IsTracked;
	std::vector<cv::Point> points;
	std::vector<Vector4> 3dpoints;
		
	SkeletonPoints(bool _IsTracked){
		IsTracked = _IsTracked;
	}
	SkeletonPoints(std::vector<Vector4> _3dpoints, std::vector<cv::Point> _points, bool _IsTracked) : points(_points), 3dpoints(_3dpoints){
		IsTracked= _IsTracked;
	}

	bool getIsTracked(){return IsTracked;};

	void Drawall(cv::Mat img, float alpha, int height, int width){
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
		return 3dpoints.at(index);
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
	int myindex = 1;
	kinect::nui::SkeletonFrame skeletonMD;
public:
	SkeletonDrawer(kinect::nui::SkeletonFrame _skeletonMD) : skeletonMD(_skeletonMD){}

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
	
	SkeletonPoints me(int width, int height){
		std::vector<cv::Point> points;
		int me_index=100;
		for(int i=0; i<skeletonMD.SKELETON_COUNT; i++){
			if(skeletonMD[i].TrackingState() == NUI_SKELETON_TRACKED){
				me_index=i;
				break;
			}
		}
		if( me_index == 100){ // No skeleton
			//std::cerr<<"errror : can't get my skeleton"<<std::endl;
			return SkeletonPoints(false);		
		}
		
		for(int j=0; j<kinect::nui::SkeletonData::POSITION_COUNT; j++){
			kinect::nui::SkeletonData::Point p = skeletonMD[me_index].TransformSkeletonToDepthImage(j);			
			points.push_back(cv::Point(p.x * width + 0.5, p.y * height +0.5));
		}			

		std::vector<Vector4> 3dpoints;		
		for(int j=0; j<kinect::nui::SkeletonData::POSITION_COUNT; j++){
			3dpoints.push_back(skeletonMD[me_index][j]);
		}

		return SkeletonPoints(points, true);
		
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
			/*if(nextalpha > 0.05){
				targetimg.at<cv::Vec4b>(y,x) = cv::Scalar(0,0,0,0);
				targetimg.at<cv::Vec4b>(y,x)+=vec1*(alpha1/nextalpha);
				targetimg.at<cv::Vec4b>(y,x)+=(vec2*((alpha2*(1-alpha1)/nextalpha)));
			}*/
			//speed up
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
	cv::threshold(tempgry_img, mask_img, 3, 255, THRESH_BINARY);
	cv::imshow("src", src_img);
	cv::imshow("mask", mask_img);

	cv::Mat buf_img(target_img.size(), CV_8UC4);
	cv::addWeighted(src_img, alpha, target_img, 1.0-alpha, 0, buf_img);
	buf_img.copyTo(target_img, mask_img);

}
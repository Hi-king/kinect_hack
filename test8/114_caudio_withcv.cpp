#include <iostream>
#include "C:\Users\ogaki\Downloads\cAudio2.1.0-Msvc_64\cAudio2.1.0-Win32-Msvc_64\include\caudio.h"

#include <opencv2\opencv.hpp>

#pragma comment(lib, "C:\\Users\\ogaki\\Downloads\\cAudio2.1.0-Msvc_64\\cAudio2.1.0-Win32-Msvc_64\\lib\\win32-visual\\cAudio.lib")

#pragma region // --- constants ---
const char* MUSICFILE = "C:\\Users\\ogaki\\Downloads\\mado.wav";
//const char* INSTFILE1 = "C:\\Users\\ogaki\\Downloads\\ToneScape_Animato\\ToneScape Animato\\Percussion\\Fat percussion\\TSA Fat Bd 04.wav";
const char* INSTFILE1 = "C:\\Users\\ogaki\\Downloads\\Bd3.wav";
const char* INSTFILE2 = "C:\\Users\\ogaki\\Downloads\\Sn.wav";
const char* INSTFILE3 = "C:\\Users\\ogaki\\Downloads\\Hh.wav";
#pragma endregion

int main(int argc, char** argv){

	#pragma region // --- caudio init ---
	cAudio::IAudioManager* manager = cAudio::createAudioManager(false);

	std::string defaultDeviceName = manager->getDefaultDeviceName();

	manager->initialize(manager->getAvailableDeviceName(0));
	
	cAudio::IAudioSource* music = manager->create("bling", MUSICFILE);
	cAudio::IAudioSource* inst1 = manager->create("bling", INSTFILE1);
	cAudio::IAudioSource* inst2 = manager->create("bling", INSTFILE2);
	cAudio::IAudioSource* inst3 = manager->create("bling", INSTFILE3);
	#pragma endregion

	#pragma region // --- cv init ---
	cv::namedWindow("test");
	#pragma endregion
	
	float pitch = 1.0; //pitch 2倍にしたらオクターブ
	music->play();
	while(1){

		#pragma region // --- keyboard callback ---
		char key = cv::waitKey(1);
		if(key=='q'){
			std::cout<<"good bye"<<std::endl;
			break;
		}
		else if(key=='p'){
			if(music->isPlaying()){
				music->pause();
			}
			else{
				music->play();
			}
		}
		else if(key=='1'){
			inst1->play();
		}
		else if(key=='2'){
			inst2->play();
		}
		else if(key=='3'){
			inst3->play();
		}
		else if(key=='u'){ // pitch up
			pitch+=0.2;
			music->setPitch(pitch);
		}
		else if(key=='d'){
			pitch-=0.2;
			music->setPitch(pitch);
		}
		#pragma endregion
	
	}


	#pragma region // --- exit ---
	manager->releaseAllSources();
	manager->shutDown();
	cAudio::destroyAudioManager(manager);

	return 0;
	#pragma endregion

}
#include <iostream>
#include "C:\Users\ogaki\Downloads\cAudio2.1.0-Msvc_64\cAudio2.1.0-Win32-Msvc_64\include\caudio.h"

#pragma comment(lib, "C:\\Users\\ogaki\\Downloads\\cAudio2.1.0-Msvc_64\\cAudio2.1.0-Win32-Msvc_64\\lib\\win32-visual\\cAudio.lib")


int main(int argc, char** argv){

#pragma region // --- caudio init ---
	cAudio::IAudioManager* manager = cAudio::createAudioManager(false);

	std::string defaultDeviceName = manager->getDefaultDeviceName();

	manager->initialize(manager->getAvailableDeviceName(0));
	
	cAudio::IAudioSource* mysound = manager->create("bling", "C:\\Users\\ogaki\\Downloads\\mado.wav");

#pragma endregion
	
	mysound->play();
	cAudio::cAudioSleep(10000);  //10000msŠÔ‚±‚±‚Å‚·‚Æ‚Á‚Õ
#pragma region // --- exit ---
	manager->releaseAllSources();
	manager->shutDown();
	cAudio::destroyAudioManager(manager);

	std::cout<<"enter to quit"<<std::endl;
	std::cin.get();
	return 0;

#pragma endregion

}
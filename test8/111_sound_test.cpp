#include <stdlib.h>
#include <AL/alut.h>

#include <opencv2\opencv.hpp>

#pragma comment(lib, "OpenAL32.lib")
#pragma comment(lib, "alut.lib")



const char* FILENAME = "C:\\Users\\ogaki\\Downloads\\mado.wav";
//const char* FILENAME = "C:\\Users\\ogaki\\Downloads\\Bd3.wav";
//const char* FILENAME = "C:\\Program Files\\fleealut\\freealut-1.1.0-bin\\examples\\file1.wav";

int main (int argc, char **argv){

#pragma region // --- my init ---
	float pitchnow = 1.0;
	alListener3f(AL_POSITION, 0.0, 0.0, 0.0);

#pragma endregion

#pragma region // --- al init ---
// alut の初期化
	
alutInit (&argc, argv);
  
// Hello World としゃべる音声の作成
ALuint helloBuffer = alutCreateBufferHelloWorld();
ALuint MusicBuffer = alutCreateBufferFromFile(FILENAME);
if(AL_NONE == helloBuffer){std::cerr<<"error:nofile"<<std::endl;exit(1);}
  
// ソースの作成
ALuint helloSource;
alGenSources (1, &helloSource);
ALuint MusicSource;
alGenSources (1, &MusicSource);
  

// ソースにバッファをバインド
alSourcei (helloSource, AL_BUFFER, helloBuffer);
alSourcei (MusicSource, AL_BUFFER, MusicBuffer);
#pragma endregion

  cv::namedWindow("hoge");
  alSourcePlay(MusicSource);
  
  alSource3f(MusicSource, AL_POSITION, 100.0, 0.0, 0.0);
  alSource3f(MusicSource, AL_VELOCITY, 10.0, 0.0, 0.0);

  while(1){
	
	  char key = cv::waitKey(1);
	  if(key=='s'){
		alSourcePlay(helloSource);
		//alutSleep(1);
	  }
	  if(key == 'p'){
		  int state;
		  alGetSourcei(MusicSource, AL_SOURCE_STATE, &state);
		  if(state ==AL_PAUSED)alSourcePlay(MusicSource);
			else alSourcePause(MusicSource);
	  }
	  else if(key == 'q'){
		  std::cout<<"good bye"<<std::endl;
		break;
	  }
	  else if(key == 'u'){
		  pitchnow *= 2;
		  alSourcef(MusicSource, AL_PITCH, pitchnow);
	  }
	  else if(key == 'd'){
		  pitchnow /= 2;
		  alSourcef(MusicSource, AL_PITCH, pitchnow);
	  }

	  // roop
	  int state;
	  alGetSourcei(MusicSource, AL_SOURCE_STATE, &state);
	  if(state != AL_PLAYING) alSourcePlay(MusicSource);
  
  }

  
	#pragma region --- release ---
	  // リソースを開放
	 alSourceStop(helloSource);
  alDeleteSources( 1, &helloSource );
  alDeleteBuffers( 1, &helloBuffer );
	 alSourceStop(MusicSource);
  alDeleteSources( 1, &MusicSource );
  alDeleteBuffers( 1, &MusicBuffer );
  alutExit ();
	#pragma endregion

  return 0;
}
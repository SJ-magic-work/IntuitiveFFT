#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( int argc, char** argv ){
	ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context

	/********************
	********************/
	int soundStream_Input_DeviceId = -1;
	int soundStream_Output_DeviceId = -1;

	/********************
	********************/
	printf("---------------------------------\n");
	printf("> parameters\n");
	printf("\t-i:Audio in(-1)\n");
	printf("\t-o:Audio out(-1)\n");
	printf("---------------------------------\n");
	
	for(int i = 1; i < argc; i++){
		if( strcmp(argv[i], "-i") == 0 ){
			if(i+1 < argc) soundStream_Input_DeviceId = atoi(argv[i+1]);
		}else if( strcmp(argv[i], "-o") == 0 ){
			if(i+1 < argc) soundStream_Output_DeviceId = atoi(argv[i+1]);
		}
	}
	
	ofRunApp(new ofApp(soundStream_Input_DeviceId, soundStream_Output_DeviceId));
}

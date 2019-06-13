/************************************************************
************************************************************/
#include "ofApp.h"

/************************************************************
************************************************************/

/******************************
******************************/
ofApp::ofApp(int _soundStream_Input_DeviceId, int _soundStream_Output_DeviceId)
: soundStream_Input_DeviceId(_soundStream_Input_DeviceId)
, soundStream_Output_DeviceId(_soundStream_Output_DeviceId)
, b_DispGui(true)
, png_id(0)
, col_Back(ofColor(30))
, col_Raw(ofColor(0, 120, 255, 255))
, col_Corrected(ofColor(0, 255, 0, 255))
{
	/********************
	********************/
	for(int i = 0; i < NUM_AUDIO_CHS; i++) { fft_thread[i] = new THREAD_FFT(); }
	
	/********************
	********************/
	font[FONT_S].load("font/RictyDiminished-Regular.ttf", 10, true, true, true);
	font[FONT_M].load("font/RictyDiminished-Regular.ttf", 12, true, true, true);
	font[FONT_L].load("font/RictyDiminished-Regular.ttf", 30, true, true, true);
	
	/********************
	********************/
	fp_Log			= fopen("../../../data/Log.csv", "w");
	fp_Log_main		= fopen("../../../data/Log_main.csv", "w");
	fp_Log_Audio 	= fopen("../../../data/Log_Audio.csv", "w");
	fp_Log_fft 		= fopen("../../../data/Log_fft.csv", "w");
}

/******************************
******************************/
ofApp::~ofApp()
{
	if(fp_Log)			fclose(fp_Log);
	if(fp_Log_main)		fclose(fp_Log_main);
	if(fp_Log_Audio)	fclose(fp_Log_Audio);
	if(fp_Log_fft)		fclose(fp_Log_fft);
}

/******************************
******************************/
void ofApp::exit(){
	/********************
	ofAppとaudioが別threadなので、ここで止めておくのが安全.
	********************/
	soundStream.stop();
	soundStream.close();
	
	/********************
	********************/
	for(int i = 0; i < NUM_AUDIO_CHS; i++){
		fft_thread[i]->exit();
		try{
			/********************
			stop済みのthreadをさらにstopすると、Errorが出るようだ。
			********************/
			while(fft_thread[i]->isThreadRunning()){
				fft_thread[i]->waitForThread(true);
			}
			
		}catch(...){
			printf("Thread exiting Error\n");
		}
		
		delete fft_thread[i];
	}
	
	
	/********************
	********************/
	printf("\n> Good bye\n");
}	

/******************************
******************************/
void ofApp::setup(){
	/********************
	********************/
	ofSetWindowTitle("intuitive FFT");
	
	ofSetWindowShape( WINDOW_WIDTH, WINDOW_HEIGHT );
	ofSetVerticalSync(true);
	ofSetFrameRate(60);
	ofSetEscapeQuitsApp(false);
	
	/********************
	********************/
	setup_Gui();
	
	/********************
	********************/
	for(int i = 0; i < NUM_AUDIO_CHS; i++){
		Vboset_fft_Raw[i].setup(AUDIO_BUF_SIZE/2 * 4); /* square */
		Vboset_fft_Raw[i].set_singleColor(ofColor(255, 255, 255, 110));
		
		Vboset_fft_Corrected[i].setup(AUDIO_BUF_SIZE/2 * 4); /* square */
		Vboset_fft_Corrected[i].set_singleColor(ofColor(255, 255, 255, 110));
	}
	
	/********************
	FBO_FFT_WIDTH		= 600,
	FBO_FFT_HEIGHT		= 340,
	
	FBO_ANALOGUE_METER_WIDTH	= 290,
	FBO_ANALOGUE_METER_HEIGHT	= 290,
	
	FBO_BAR_METER_WIDTH		= 290,
	FBO_BAR_METER_HEIGHT	= 10,
	********************/
	fbo[GRAPH__ANALOGUE_METER_RAW_L].allocate(FBO_ANALOGUE_METER_WIDTH, FBO_ANALOGUE_METER_HEIGHT, GL_RGBA, 4);
	fbo[GRAPH__ANALOGUE_METER_CORRECTED_L].allocate(FBO_ANALOGUE_METER_WIDTH, FBO_ANALOGUE_METER_HEIGHT, GL_RGBA, 4);
	fbo[GRAPH__BAR_METER_RAW_L].allocate(FBO_BAR_METER_WIDTH, FBO_BAR_METER_HEIGHT, GL_RGBA);
	fbo[GRAPH__BAR_METER_CORRECTED_L].allocate(FBO_BAR_METER_WIDTH, FBO_BAR_METER_HEIGHT, GL_RGBA);
	fbo[GRAPH__FFT_L].allocate(FBO_FFT_WIDTH, FBO_FFT_HEIGHT, GL_RGBA);

	fbo[GRAPH__ANALOGUE_METER_RAW_R].allocate(FBO_ANALOGUE_METER_WIDTH, FBO_ANALOGUE_METER_HEIGHT, GL_RGBA, 4);
	fbo[GRAPH__ANALOGUE_METER_CORRECTED_R].allocate(FBO_ANALOGUE_METER_WIDTH, FBO_ANALOGUE_METER_HEIGHT, GL_RGBA, 4);
	fbo[GRAPH__BAR_METER_RAW_R].allocate(FBO_BAR_METER_WIDTH, FBO_BAR_METER_HEIGHT, GL_RGBA);
	fbo[GRAPH__BAR_METER_CORRECTED_R].allocate(FBO_BAR_METER_WIDTH, FBO_BAR_METER_HEIGHT, GL_RGBA);
	fbo[GRAPH__FFT_R].allocate(FBO_FFT_WIDTH, FBO_FFT_HEIGHT, GL_RGBA);
	
	for(int i = 0; i < NUM_GRAPHS; i++) { Clear_fbo(fbo[i]); }
	
	/********************
	********************/
	AudioSample.resize(AUDIO_BUF_SIZE);

	for(int i = 0; i < NUM_AUDIO_CHS; i++) { fft_thread[i]->setup(); fft_thread[i]->startThread(); }
	
	/********************
	********************/
	Refresh_FFTVerts();
	
	/********************
	********************/
	// shader_Or.load( "shader/Or.vert", "shader/Or.frag" );
	
	/********************
	settings.setInListener(this);
	settings.setOutListener(this);
	settings.sampleRate = 44100;
	settings.numInputChannels = 2;
	settings.numOutputChannels = 2;
	settings.bufferSize = bufferSize;
	
	soundStream.setup(settings);
	********************/
	soundStream.printDeviceList();
	
	/********************
	soundStream.setup()の位置に注意:最後
		setup直後、audioIn()/audioOut()がstartする.
		これらのmethodは、fft_threadにaccessするので、start前にReStart()によって、fft_threadが初期化されていないと、不正accessが発生してしまう.
	********************/
	setup_SoundStream();
}

/******************************
******************************/
void ofApp::setup_SoundStream()
{
	/********************
	settings.setInListener(this);
	settings.setOutListener(this);
	settings.sampleRate = 44100;
	settings.numInputChannels = 2;
	settings.numOutputChannels = 2;
	settings.bufferSize = bufferSize;
	
	soundStream.setup(settings);
	********************/
	ofSoundStreamSettings settings;
	
	if( soundStream_Input_DeviceId == -1 ){
		ofExit();
		return;
		
	}else{
		vector<ofSoundDevice> devices = soundStream.getDeviceList();
		
		if( soundStream_Input_DeviceId != -1 ){
			settings.setInDevice(devices[soundStream_Input_DeviceId]);
			settings.setInListener(this);
			settings.numInputChannels = AUDIO_BUFFERS;
		}else{
			settings.numInputChannels = 0;
		}
		
		if( soundStream_Output_DeviceId != -1 ){
			if(devices[soundStream_Output_DeviceId].name == "Apple Inc.: Built-in Output"){
				printf("!!!!! prohibited to use [%s] for output ... by SJ for safety !!!!!\n", devices[soundStream_Output_DeviceId].name.c_str());
				fflush(stdout);
				
				settings.numOutputChannels = 0;
				
			}else{
				settings.setOutDevice(devices[soundStream_Output_DeviceId]);
				settings.numOutputChannels = AUDIO_BUFFERS;
				settings.setOutListener(this); /* Don't forget this */
			}
		}else{
			settings.numOutputChannels = 0;
		}
		
		settings.numBuffers = 4;
		settings.sampleRate = AUDIO_SAMPLERATE;
		settings.bufferSize = AUDIO_BUF_SIZE;
	}
	
	/********************
	soundStream.setup()の位置に注意:最後
		setup直後、audioIn()/audioOut()がstartする.
		これらのmethodは、fft_threadにaccessするので、start前にReStart()によって、fft_threadが初期化されていないと、不正accessが発生してしまう.
	********************/
	soundStream.setup(settings);
	// soundStream.start();
}

/******************************
description
	memoryを確保は、app start後にしないと、
	segmentation faultになってしまった。
******************************/
void ofApp::setup_Gui()
{
	/********************
	********************/
	Gui_Global = new GUI_GLOBAL;
	Gui_Global->setup("FFT", "gui.xml", 1000, 10);
}

/******************************
******************************/
void ofApp::Clear_fbo(ofFbo& fbo)
{
	fbo.begin();
	
	// Clear with alpha, so we can capture via syphon and composite elsewhere should we want.
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	ofClear(0, 0, 0, 0);
	
	fbo.end();
}

/******************************
******************************/
void ofApp::Refresh_FFTVerts()
{
	/********************
	********************/
	const float BarWidth = GRAPH_BAR_WIDTH__FFT_GAIN;
	const float BarSpace = GRAPH_BAR_SPACE__FFT_GAIN;
	
	const float CorrectionBarHeight = 5;
	
	
	/********************
	********************/
	/* */
	float Gui_DispGain = Gui_Global->Val_DispMax__FFTGain;
	for(int ch = 0; ch < NUM_AUDIO_CHS; ch++){
		for(int j = 0; j < AUDIO_BUF_SIZE/2; j++){
			double _Gain = fft_thread[ch]->getArrayVal_x_DispGain(j, Gui_DispGain, FBO_FFT_HEIGHT, true, false);
			Vboset_fft_Raw[ch].VboVerts[j * 4 + 0].set( BarSpace * j            , 0 );
			Vboset_fft_Raw[ch].VboVerts[j * 4 + 1].set( BarSpace * j            , _Gain );
			Vboset_fft_Raw[ch].VboVerts[j * 4 + 2].set( BarSpace * j  + BarWidth, _Gain );
			Vboset_fft_Raw[ch].VboVerts[j * 4 + 3].set( BarSpace * j  + BarWidth, 0 );
			
			_Gain = fft_thread[ch]->getArrayVal_x_DispGain(j, Gui_DispGain, FBO_FFT_HEIGHT, true, true);
			Vboset_fft_Corrected[ch].VboVerts[j * 4 + 0].set( BarSpace * j            , _Gain -  CorrectionBarHeight);
			Vboset_fft_Corrected[ch].VboVerts[j * 4 + 1].set( BarSpace * j            , _Gain );
			Vboset_fft_Corrected[ch].VboVerts[j * 4 + 2].set( BarSpace * j  + BarWidth, _Gain );
			Vboset_fft_Corrected[ch].VboVerts[j * 4 + 3].set( BarSpace * j  + BarWidth, _Gain -  CorrectionBarHeight );
		}
		
		Vboset_fft_Raw[ch].update();
		Vboset_fft_Corrected[ch].update();
	}
}

/******************************
******************************/
void ofApp::update(){
	/********************
	********************/
	static int t_LastINT = 0;
	int now = ofGetElapsedTimeMillis();
	
	/********************
	cal
	********************/
	for(int i = 0; i < NUM_AUDIO_CHS; i++) fft_thread[i]->update();
	
	update_VolcalGain(now - t_LastINT);
	
	/********************
	vbo
	********************/
	Refresh_FFTVerts();
	
	/********************
	draw to fbo
	********************/
	/* */
	drawFbo_FFT(fbo[GRAPH__FFT_L], Vboset_fft_Raw[AUDIO_CH_L], Vboset_fft_Corrected[AUDIO_CH_L], max_in_VocalZone__Raw[AUDIO_CH_L].Gain, max_in_VocalZone__Corrected[AUDIO_CH_L].Gain);
	drawFbo_FFT(fbo[GRAPH__FFT_R], Vboset_fft_Raw[AUDIO_CH_R], Vboset_fft_Corrected[AUDIO_CH_R], max_in_VocalZone__Raw[AUDIO_CH_R].Gain, max_in_VocalZone__Corrected[AUDIO_CH_R].Gain);
	
	/* */
	drawFbo_BarMeter(fbo[GRAPH__BAR_METER_RAW_L], max_in_VocalZone__Raw[AUDIO_CH_L].Gain, col_Raw.get_col(100));
	drawFbo_BarMeter(fbo[GRAPH__BAR_METER_CORRECTED_L], max_in_VocalZone__Corrected[AUDIO_CH_L].Gain, col_Corrected.get_col(120));
	
	drawFbo_BarMeter(fbo[GRAPH__BAR_METER_RAW_R], max_in_VocalZone__Raw[AUDIO_CH_R].Gain, col_Raw.get_col(100));
	drawFbo_BarMeter(fbo[GRAPH__BAR_METER_CORRECTED_R], max_in_VocalZone__Corrected[AUDIO_CH_R].Gain, col_Corrected.get_col(120));
	
	/* */
	drawFbo_AnalogueMeter(fbo[GRAPH__ANALOGUE_METER_RAW_L], max_in_VocalZone__Raw[AUDIO_CH_L].Gain, col_Raw.get_col(255), "RAW : L");
	drawFbo_AnalogueMeter(fbo[GRAPH__ANALOGUE_METER_CORRECTED_L], max_in_VocalZone__Corrected[AUDIO_CH_L].Gain, col_Corrected.get_col(180), "CORRECTED : L");
	
	drawFbo_AnalogueMeter(fbo[GRAPH__ANALOGUE_METER_RAW_R], max_in_VocalZone__Raw[AUDIO_CH_R].Gain, col_Raw.get_col(255), "RAW : R");
	drawFbo_AnalogueMeter(fbo[GRAPH__ANALOGUE_METER_CORRECTED_R], max_in_VocalZone__Corrected[AUDIO_CH_R].Gain, col_Corrected.get_col(180), "CORRECTED : R");
	
	
	/********************
	********************/
	t_LastINT = now;
}

/******************************
******************************/
void ofApp::update_VolcalGain(int dt_ms){
	for(int ch = 0; ch < NUM_AUDIO_CHS; ch++){
		double Gain;
		fft_thread[ch]->get_max_of_Gain(Vocal_FreqId_From, Vocal_FreqId_To, max_in_VocalZone__Raw[ch].FreqId, Gain, false);
		max_in_VocalZone__Raw[ch].Gain = LPF(max_in_VocalZone__Raw[ch].Gain, Gain, Gui_Global->LPFAlpha_dt__VovalGain, (float)dt_ms / 1000);
		
		fft_thread[ch]->get_max_of_Gain(Vocal_FreqId_From, Vocal_FreqId_To, max_in_VocalZone__Corrected[ch].FreqId, Gain, true);
		max_in_VocalZone__Corrected[ch].Gain = LPF(max_in_VocalZone__Corrected[ch].Gain, Gain, Gui_Global->LPFAlpha_dt__VovalGain, (float)dt_ms / 1000);
	}
}

/******************************
******************************/
void ofApp::draw(){
	/********************
	********************/
	ofDisableAlphaBlending();
	
	/********************
	********************/
	float now = ofGetElapsedTimef();
	
	/********************
	********************/
	// ofClear(0, 0, 0, 255);
	ofBackground(col_Back.get_col(255));
	ofSetColor(255, 255, 255, 255);
	
	for(int i = 0; i < NUM_GRAPHS; i++){
		drawFbo_toScreen( fbo[i], Fbo_DispPos[i], fbo[i].getWidth(), fbo[i].getHeight() );
	}
	
	/********************
	********************/
	if(b_DispGui) Gui_Global->gui.draw();
	
	/*
	if(b_DispFrameRate){
		ofSetColor(255, 0, 0, 255);
		
		char buf[BUF_SIZE_S];
		sprintf(buf, "%5.1f", ofGetFrameRate());
		
		font[FONT_M].drawString(buf, 30, 30);
	}
	*/
}

/******************************
******************************/
void ofApp::drawFbo_toScreen(ofFbo& _fbo, const ofPoint& Coord_zero, const int Width, const int Height)
{
	ofPushStyle();
	ofPushMatrix();
		/********************
		********************/
		ofTranslate(Coord_zero);
		
		_fbo.draw(0, 0, Width, Height);
		
	ofPopMatrix();
	ofPopStyle();
}

/******************************
******************************/
void ofApp::drawFbo_FFT(ofFbo& fbo, VBO_SET& _Vboset_fft_Raw, VBO_SET& _Vboset_fft_Corrected, double _max_in_VocalZone__Raw, double _max_in_VocalZone__Corrected)
{
 	float Screen_y_max = fbo.getHeight();
	float Val_Disp_y_Max = Gui_Global->Val_DispMax__FFTGain;
	
	/********************
	********************/
	fbo.begin();
		ofEnableAlphaBlending();
		// ofEnableBlendMode(OF_BLENDMODE_ADD);
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		
		ofClear(0, 0, 0, 0);
		ofSetColor(255, 255, 255, 255);
		
		ofPushStyle();
		ofPushMatrix();
			/********************
			********************/
			ofTranslate(0, fbo.getHeight() - 1);
			ofScale(1, -1, 1);
			
			/********************
			y目盛り
			********************/
			if(0 < Val_Disp_y_Max){
				const int num_lines = 5;
				const double y_step = Screen_y_max/num_lines;
				for(int i = 0; i < num_lines; i++){
					int y = int(i * y_step + 0.5);
					
					ofSetColor(ofColor(50));
					ofSetLineWidth(1);
					ofNoFill();
					ofDrawLine(0, y, fbo.getWidth(), y);
		
					/********************
					********************/
					char buf[BUF_SIZE_S];
					sprintf(buf, "%7.4f", Val_Disp_y_Max/num_lines * i);
					
					ofSetColor(ofColor(200));
					ofScale(1, -1, 1); // 文字が上下逆さまになってしまうので.
					font[FONT_S].drawString(buf, fbo.getWidth() - 1 - font[FONT_S].stringWidth(buf) - 10, -y); // y posはマイナス
					ofScale(1, -1, 1); // 戻す.
				}
			}
			
			/********************
			zoneを示す縦Line
			********************/
			for(int i = 0; i < NUM_FREQ_ZONES; i++){
				int _x = ZoneFreqId_From[i] * GRAPH_BAR_SPACE__FFT_GAIN;
				
				ofSetColor(ofColor(50));
				ofSetLineWidth(1);
				ofNoFill();
				
				ofDrawLine(_x, 0, _x, fbo.getHeight());
			}
			
			/********************
			********************/
			ofSetColor(255);
			glPointSize(1.0);
			glLineWidth(1);
			ofFill();
			
			_Vboset_fft_Raw.draw(GL_QUADS);
			_Vboset_fft_Corrected.draw(GL_QUADS);
			
			/********************
			********************/
			glPointSize(1.0);
			glLineWidth(2);
			ofFill();
			
			ofSetColor(col_Raw.get_col(120));
			float _height = ofMap(_max_in_VocalZone__Raw, 0.0, Gui_Global->Val_DispMax__FFTGain, 0.0, fbo.getHeight());
			ofDrawLine(GRAPH_BAR_SPACE__FFT_GAIN * Vocal_FreqId_From, _height, GRAPH_BAR_SPACE__FFT_GAIN * Vocal_FreqId_To, _height);
			
			ofSetColor(col_Corrected.get_col(120));
			_height = ofMap(_max_in_VocalZone__Corrected, 0.0, Gui_Global->Val_DispMax__FFTGain, 0.0, fbo.getHeight());
			ofDrawLine(GRAPH_BAR_SPACE__FFT_GAIN * Vocal_FreqId_From, _height, GRAPH_BAR_SPACE__FFT_GAIN * Vocal_FreqId_To, _height);
			
		ofPopMatrix();
		ofPopStyle();
	fbo.end();
}

/******************************
******************************/
void ofApp::drawFbo_BarMeter(ofFbo& fbo, double val, ofColor col)
{
	fbo.begin();
		ofClear(255, 255, 255, 255);
		
		ofEnableAlphaBlending();
		// ofEnableBlendMode(OF_BLENDMODE_ADD);
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		
		/********************
		********************/
		ofSetColor(50);
		ofSetLineWidth(1);
		ofFill();
		
		float Scale_Step = fbo.getWidth() / 10;
		int id = 0;
		for(float _x = 0; _x < fbo.getWidth(); _x += Scale_Step, id++){
			ofDrawLine(_x, 0, _x, fbo.getHeight());
			
			char buf[BUF_SIZE_S];
			sprintf(buf, "%2d", id);
			font[FONT_S].drawString(buf, _x, fbo.getHeight());
		}
		
		/********************
		********************/
		{
			float val_Width_On_Screen = ofMap(val, 0, Gui_Global->Val_DispMax__FFTGain, 0, fbo.getWidth());
			ofSetColor(col);
			ofSetLineWidth(1);
			ofFill();
			
			ofDrawRectangle(0, 0, val_Width_On_Screen, fbo.getHeight());
		}
		
	fbo.end();
}

/******************************
******************************/
void ofApp::drawFbo_AnalogueMeter(ofFbo& fbo, double val, ofColor col, string str_title)
{
	fbo.begin();
		// ofClear(0, 0, 0, 255);
		ofBackground(170);
		
		ofEnableAlphaBlending();
		// ofEnableBlendMode(OF_BLENDMODE_ADD);
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		ofEnableSmoothing();
		
		/********************
		********************/
		ofPushMatrix();
			ofTranslate(fbo.getWidth()/2, fbo.getHeight());
			
			/********************
			scale
			********************/
			ofPushMatrix();
			{
				ofSetColor(90);
				ofSetLineWidth(1);
				ofFill();
				
				ofRotateDeg(-60);
				
				{
					int NUM_SCALE = 7;
					double StepAngle = 120 / (NUM_SCALE - 1);
					int i = 0;
					do{
						const double _R = 140;
						const double _r = 3;
						
						ofDrawCircle(0, -_R, _r);
						ofRotateDeg(StepAngle);
						
						i++;
					}while(i < NUM_SCALE);
				}
			}
			ofPopMatrix();
			
			/********************
			text
			********************/
			// ofSetColor(120); // same color as scale.
			
			char buf[BUF_SIZE_S];
			sprintf(buf, "%s", str_title.c_str());
			font[FONT_M].drawString(buf, -font[FONT_M].stringWidth(buf)/2, -60);
			
			/********************
			meter
			********************/
			ofPushMatrix();
			{
				ofRotateDeg(-60);
				
				const double BarLength = 160;
				double Angle = ofMap(val, 0, Gui_Global->Val_DispMax__FFTGain, 0, 120, true);
				ofRotateDeg(Angle);
				
				ofSetColor(col);
				ofSetLineWidth(3);
				ofFill();
				
				ofDrawLine(0, 0, 0, -BarLength);
			}
			ofPopMatrix();
			
			/********************
			Black Dot
			********************/
			ofSetColor(col_Back.get_col(255));
			ofSetLineWidth(1);
			ofFill();
			
			ofDrawCircle(0, 0, 25);
			
		ofPopMatrix();
	fbo.end();
}

/******************************
******************************/
void ofApp::keyPressed(int key){
	switch(key){
		case 'd':
			b_DispGui = !b_DispGui;
			break;
			
		case ' ':
			{
				b_Log = true;
				
				char buf[BUF_SIZE_S];
				
				sprintf(buf, "image_%d.jpg", png_id);
				ofSaveScreen(buf);
				// ofSaveFrame();
				printf("> %s saved\n", buf);
				
				png_id++;
			}
			break;
	}
}

/******************************
audioIn/ audioOut
	同じthreadで動いている様子。
	また、audioInとaudioOutは、同時に呼ばれることはない(多分)。
	つまり、ofAppからaccessがない限り、変数にaccessする際にlock/unlock する必要はない。
	ofApp側からaccessする時は、threadを立てて、安全にpassする仕組みが必要
******************************/
void ofApp::audioIn(ofSoundBuffer & buffer)
{
    for (int i = 0; i < buffer.getNumFrames(); i++) {
        AudioSample.Left[i] = buffer[2*i];
		AudioSample.Right[i] = buffer[2*i+1];
    }
	
	/********************
	FFT Filtering
	1 process / block.
	********************/
	fft_thread[AUDIO_CH_L]->update__Gain(AudioSample.Left);
	fft_thread[AUDIO_CH_R]->update__Gain(AudioSample.Right);
}  

/******************************
******************************/
void ofApp::audioOut(ofSoundBuffer & buffer)
{
	/********************
	x	:input -> output
	o	:No output.
	********************/
    for (int i = 0; i < buffer.getNumFrames(); i++) {
		buffer[2*i  ] = AudioSample.Left[i];
		buffer[2*i+1] = AudioSample.Right[i];
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

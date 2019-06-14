/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "ofMain.h"
#include "ofxSyphon.h"

#include "sj_common.h"
#include "th_fft.h"


/************************************************************
************************************************************/

/**************************************************
**************************************************/
struct AUDIO_SAMPLE : private Noncopyable{
	vector<float> Left;
	vector<float> Right;
	
	void resize(int size){
		/*
		Left.resize(size);
		Right.resize(size);
		*/
		
		Left.assign(size, 0.0);
		Right.assign(size, 0.0);
	}
};

/**************************************************
**************************************************/
struct VBO_SET : private Noncopyable{
	ofVbo Vbo;
	vector<ofVec3f> VboVerts;
	vector<ofFloatColor> VboColor;
	
	void setup(int size){
		VboVerts.resize(size);
		VboColor.resize(size);
		
		Vbo.setVertexData(&VboVerts[0], VboVerts.size(), GL_DYNAMIC_DRAW);
		Vbo.setColorData(&VboColor[0], VboColor.size(), GL_DYNAMIC_DRAW);
	}
	
	void set_singleColor(const ofColor& color){
		for(int i = 0; i < VboColor.size(); i++) { VboColor[i].set( double(color.r)/255, double(color.g)/255, double(color.b)/255, double(color.a)/255); }
	}
	
	void set_Color(int id, int NumPerId, const ofColor& color){
		for(int i = 0; i < NumPerId; i++){
			VboColor[id * NumPerId + i].set( double(color.r)/255, double(color.g)/255, double(color.b)/255, double(color.a)/255);
		}
	}
	
	void update(){
		Vbo.updateVertexData(&VboVerts[0], VboVerts.size());
		Vbo.updateColorData(&VboColor[0], VboColor.size());
	}
	
	void draw(int drawMode){
		Vbo.draw(drawMode, 0, VboVerts.size());
	}
	
	void draw(int drawMode, int total){
		if(VboVerts.size() < total) total = VboVerts.size();
		Vbo.draw(drawMode, 0, total);
	}
};

/**************************************************
**************************************************/
class MY_COLOR{
private:
	ofColor col;
	
public:
	MY_COLOR(const ofColor& _col)
	: col(_col)
	{
	}
	
	ofColor get_col(double alpha)
	{
		ofColor _col = col;
		_col.a = alpha;
		
		return _col;
	}
};

/**************************************************
**************************************************/
class ofApp : public ofBaseApp{
private:
	/****************************************
	****************************************/
	struct FREQID_GAIN{
		int FreqId;
		double Gain;
		
		FREQID_GAIN()
		: FreqId(0), Gain(0)
		{
		}
	};
	
	/****************************************
	****************************************/
	enum{
		FONT_S,
		FONT_M,
		FONT_L,
		
		NUM_FONTSIZE,
	};
	
	/****************************************
	****************************************/
	/* */
	ofFbo fbo[NUM_GRAPHS];
	
	ofPoint Fbo_DispPos[NUM_GRAPHS] = {
		ofPoint(20, 50),	// GRAPH__ANALOGUE_METER_RAW_L,
		ofPoint(330, 50),	// GRAPH__ANALOGUE_METER_CORRECTED_L,
		ofPoint(20, 278),	// GRAPH__BAR_METER_CORRECTED_L,
		ofPoint(20, 294),	// GRAPH__BAR_METER_RAW_L,
		ofPoint(20, 330),	// GRAPH__FFT_L,
		
		ofPoint(660, 50),	// GRAPH__ANALOGUE_METER_RAW_R,
		ofPoint(970, 50),	// GRAPH__ANALOGUE_METER_CORRECTED_R,
		ofPoint(660, 278),	// GRAPH__BAR_METER_CORRECTED_R,
		ofPoint(660, 294),	// GRAPH__BAR_METER_RAW_R,
		ofPoint(660, 330),	// GRAPH__FFT_R,
	};
	
	/* */
	VBO_SET Vboset_fft_Raw[NUM_AUDIO_CHS];
	VBO_SET Vboset_fft_Corrected[NUM_AUDIO_CHS];
	
	/********************
	********************/
	int ZoneFreqId_From[NUM_FREQ_ZONES] = {2, 10, 24, 47, 117 };
	
	int Vocal_FreqId_From = 3;
	// int Vocal_FreqId_To = 15;
	
	
	FREQID_GAIN max_in_VocalZone__Raw[NUM_AUDIO_CHS];
	FREQID_GAIN max_in_VocalZone__Corrected[NUM_AUDIO_CHS];
	
	/********************
	********************/
	bool b_DispGui;
	
	/********************
	********************/
	MY_COLOR col_Back;
	MY_COLOR col_Raw;
	MY_COLOR col_Corrected;
	
	/********************
	********************/
	ofSoundStream soundStream;
	int soundStream_Input_DeviceId;
	int soundStream_Output_DeviceId;
	
	AUDIO_SAMPLE AudioSample;
	
	ofTrueTypeFont font[NUM_FONTSIZE];
	
	THREAD_FFT *fft_thread[NUM_AUDIO_CHS];
	
	/********************
	********************/
	ofShader shader_SetOpaque;
	
	/********************
	********************/
	int png_id;
	bool b_Log = false;
	
	
	/****************************************
	****************************************/
	void setup_Gui();
	void Clear_fbo(ofFbo& fbo);
	void Refresh_FFTVerts();
	void setup_SoundStream();
	void drawFbo_FFT(ofFbo& fbo, VBO_SET& _Vboset_fft_Raw, VBO_SET& _Vboset_fft_Corrected, double _max_in_VocalZone__Raw, double _max_in_VocalZone__Corrected);
	void drawFbo_toScreen(ofFbo& _fbo, const ofPoint& Coord_zero, const int Width, const int Height);
	void update_VolcalGain(int dt_ms);
	void drawFbo_BarMeter(ofFbo& fbo, double val, ofColor col);
	void drawFbo_AnalogueMeter(ofFbo& fbo, double val, ofColor col, string str_title);
	
public:
	/****************************************
	****************************************/
	ofApp(int _soundStream_Input_DeviceId, int _soundStream_Output_DeviceId);
	~ofApp();
	
	void exit();
 	void setup();
	void update();
	void draw();
	
	void audioIn(ofSoundBuffer & buffer);
	void audioOut(ofSoundBuffer & buffer);

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	
};

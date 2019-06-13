/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "stdio.h"

#include "ofMain.h"
#include "ofxGui.h"

/************************************************************
************************************************************/
#define ERROR_MSG(); printf("Error in %s:%d\n", __FILE__, __LINE__);

/************************************************************
************************************************************/
enum{
	WINDOW_WIDTH	= 1280,	// 切れの良い解像度でないと、ofSaveScreen()での画面保存が上手く行かなかった(真っ暗な画面が保存されるだけ).
	WINDOW_HEIGHT	= 720,
};

enum{
	FBO_FFT_WIDTH		= 600,
	FBO_FFT_HEIGHT		= 340,
	
	FBO_ANALOGUE_METER_WIDTH	= 290,
	FBO_ANALOGUE_METER_HEIGHT	= 200,
	
	FBO_BAR_METER_WIDTH		= 600,
	FBO_BAR_METER_HEIGHT	= 8,
};

enum{
	BUF_SIZE_S = 500,
	BUF_SIZE_M = 1000,
	BUF_SIZE_L = 6000,
};

enum{
	AUDIO_BUF_SIZE = 512,
	
	AUDIO_BUFFERS = 2,
	AUDIO_SAMPLERATE = 44100,
};

enum{
	GRAPH_BAR_WIDTH__FFT_GAIN = 5,
	GRAPH_BAR_SPACE__FFT_GAIN = 10,
};

enum{
	AUDIO_CH_L,
	AUDIO_CH_R,
	
	NUM_AUDIO_CHS,
};

enum{
	NUM_FREQ_ZONES = 5,
};

enum{
	GRAPH__ANALOGUE_METER_RAW_L,
	GRAPH__ANALOGUE_METER_CORRECTED_L,
	GRAPH__BAR_METER_CORRECTED_L,
	GRAPH__BAR_METER_RAW_L,
	GRAPH__FFT_L,
	
	GRAPH__ANALOGUE_METER_RAW_R,
	GRAPH__ANALOGUE_METER_CORRECTED_R,
	GRAPH__BAR_METER_CORRECTED_R,
	GRAPH__BAR_METER_RAW_R,
	GRAPH__FFT_R,
	
	NUM_GRAPHS,
};
	


/************************************************************
************************************************************/

/**************************************************
Derivation
	class MyClass : private Noncopyable {
	private:
	public:
	};
**************************************************/
class Noncopyable{
protected:
	Noncopyable() {}
	~Noncopyable() {}

private:
	void operator =(const Noncopyable& src);
	Noncopyable(const Noncopyable& src);
};


/**************************************************
**************************************************/
class GUI_GLOBAL{
private:
	/****************************************
	****************************************/
	
public:
	/****************************************
	****************************************/
	void setup(string GuiName, string FileName = "gui.xml", float x = 10, float y = 10);
	
	ofxGuiGroup Group_Graph;
		ofxFloatSlider LPFAlpha_dt__FFTGain;
		ofxFloatSlider LPFAlpha_dt__FFTPhase;

		ofxFloatSlider LPFAlpha_dt__VovalGain;
		
		ofxFloatSlider Val_DispMax__FFTGain;
		
	
	/****************************************
	****************************************/
	ofxPanel gui;
};

/************************************************************
************************************************************/
double LPF(double LastVal, double CurrentVal, double Alpha_dt, double dt);
double LPF(double LastVal, double CurrentVal, double Alpha);
double sj_max(double a, double b);

/************************************************************
************************************************************/
extern GUI_GLOBAL* Gui_Global;

extern FILE* fp_Log;
extern FILE* fp_Log_main;
extern FILE* fp_Log_Audio;
extern FILE* fp_Log_fft;

extern int GPIO_0;
extern int GPIO_1;


/************************************************************
************************************************************/


/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "ofMain.h"

#include "sj_common.h"

/************************************************************
************************************************************/

/**************************************************
**************************************************/
class THREAD_FFT : public ofThread, private Noncopyable{
private:
	/****************************************
	****************************************/
	/********************
	********************/
	double Gain_Raw[AUDIO_BUF_SIZE];
	double Gain_Corrected[AUDIO_BUF_SIZE];
	double Phase[AUDIO_BUF_SIZE];
	
	const int N;
	vector<float> fft_window;
	vector<double> sintbl;
	vector<int> bitrev;
	
	float LastInt;
	double BaseFreq;
	
	/****************************************
	function
	****************************************/
	/********************
	singleton
	********************/
	/*
	THREAD_FFT();
	~THREAD_FFT();
	THREAD_FFT(const THREAD_FFT&); // Copy constructor. no define.
	THREAD_FFT& operator=(const THREAD_FFT&); // コピー代入演算子. no define.
	*/
	
	/********************
	********************/
	void threadedFunction();
	
	int fft(double x[], double y[], int IsReverse = false);
	void make_bitrev(void);
	void make_sintbl(void);
	
	bool checkAndFix_AccessRangeOfGainArray(int& from, int& to);
	static int double_sort( const void * a , const void * b );
	void AudioSample_fft_LPF_saveToArray(const vector<float> &AudioSample, float dt);
	double CalPhase(double x, double y, int id);
	double cal_GainUpRatio(int Freq_id);
	
public:
	/****************************************
	****************************************/
	/********************
	********************/
	/*
	static THREAD_FFT* getInstance(){
		static THREAD_FFT inst;
		return &inst;
	}
	*/
	
	THREAD_FFT();
	~THREAD_FFT();
	
	void exit();
	void setup();
	void update();
	
	void update__Gain(const vector<float> &AudioSample);
	
	double getArrayVal(int id, bool b_Correction = false);
	double getArrayVal_x_DispGain(int id, float Gui_DispGain, float ScreenHeight, bool b_Clamp, bool b_Correction = false);
	double getPhase(int id);
	double get_max_of_Gain(int from, int to, int& ret_FreqId, double& ret_Gain, bool b_Correction = false);
	double get_min_of_Gain(int from, int to, int& ret_FreqId, double& ret_Gain, bool b_Correction = false);
	
	void Log();
};




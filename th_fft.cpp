/************************************************************
************************************************************/
#include "th_fft.h"
#include "stdlib.h"


/************************************************************
************************************************************/

/******************************
******************************/
THREAD_FFT::THREAD_FFT()
: N(AUDIO_BUF_SIZE)
, LastInt(0)
{
	/********************
	********************/
	/* 窓関数 */
	fft_window.resize(N);
	for(int i = 0; i < N; i++)	fft_window[i] = 0.5 - 0.5 * cos(2 * PI * i / N);
	
	sintbl.resize(N + N/4);
	bitrev.resize(N);
	
	make_bitrev();
	make_sintbl();
	
	/********************
	********************/
	setup();
	
	/********************
	********************/
	BaseFreq = (double)AUDIO_SAMPLERATE / (double)AUDIO_BUF_SIZE;
}

/******************************
******************************/
THREAD_FFT::~THREAD_FFT()
{
}

/******************************
******************************/
void THREAD_FFT::threadedFunction()
{
	while(isThreadRunning()) {
		lock();
		
		unlock();
		
		
		sleep(10);
	}
}

/******************************
******************************/
void THREAD_FFT::exit()
{
	this->lock();
	
	this->unlock();
}

/******************************
******************************/
void THREAD_FFT::setup()
{
	this->lock();
	for(int i = 0; i < AUDIO_BUF_SIZE; i++){
		Gain_Raw[i] = 0;
		Gain_Corrected[i] = 0;
		Phase[i] = 0;
	}
	this->unlock();
}

/******************************
******************************/
void THREAD_FFT::update()
{
	this->lock();
	
	this->unlock();
}


/******************************
******************************/
void THREAD_FFT::Log()
{
	/*
	for(int i = 0; i < AUDIO_BUF_SIZE/2; i++){
		fprintf(fp_Log, "%d,%f\n", i, ZeroCross__SlowSmoothedGain[i]);
	}
	*/
}

/******************************
description
	昇順
******************************/
int THREAD_FFT::double_sort( const void * a , const void * b )
{
	if(*(double*)a < *(double*)b){
		return -1;
	}else if(*(double*)a == *(double*)b){
		return 0;
	}else{
		return 1;
	}
}

/******************************
return
	true	: Bad Range.
	false	: Good Range.
******************************/
bool THREAD_FFT::checkAndFix_AccessRangeOfGainArray(int& from, int& to)
{
	/********************
	********************/
	if(to < from)	return true;
	
	/********************
	********************/
	if(from < 0)					from = 0;
	if(AUDIO_BUF_SIZE/2 <= from)	from = AUDIO_BUF_SIZE/2 - 1;
	
	if(to < 0)					to = 0;
	if(AUDIO_BUF_SIZE/2 <= to)	to = AUDIO_BUF_SIZE/2 - 1;
	
	/********************
	********************/
	if(to < from)	return true;
	else			return false;
}

/******************************
******************************/
double THREAD_FFT::get_max_of_Gain(int from, int to, int& ret_FreqId, double& ret_Gain, bool b_Correction)
{
	/********************
	********************/
	if(checkAndFix_AccessRangeOfGainArray(from, to)) return 0;
	
	/********************
	********************/
	lock();
	double* Gain;
	
	if(b_Correction)	Gain = Gain_Corrected;
	else				Gain = Gain_Raw;
	
	ret_Gain = Gain[from];
	ret_FreqId = from;
	
	int i;
	for(i = from + 1; i <= to; i++){
		if(ret_Gain < Gain[i]){
			ret_Gain = Gain[i];
			ret_FreqId = i;
		}
	}
	unlock();
	
	return ret_Gain;
}

/******************************
******************************/
double THREAD_FFT::get_min_of_Gain(int from, int to, int& ret_FreqId, double& ret_Gain, bool b_Correction)
{
	/********************
	********************/
	if(checkAndFix_AccessRangeOfGainArray(from, to)) return 0;
	
	/********************
	********************/
	lock();
	double* Gain;
	
	if(b_Correction)	Gain = Gain_Corrected;
	else				Gain = Gain_Raw;
	
	ret_Gain = Gain[from];
	ret_FreqId = from;
	
	int i;
	for(i = from + 1; i <= to; i++){
		if(Gain[i] < ret_Gain){
			ret_Gain = Gain[i];
			ret_FreqId = i;
		}
	}
	unlock();
	
	return ret_Gain;
}

/******************************
******************************/
double THREAD_FFT::getArrayVal(int id, bool b_Correction)
{
	if((id < 0) || (AUDIO_BUF_SIZE/2 <= id)) return 0;
	
	double ret = 0;
	
	this->lock();
	
	if(b_Correction)	ret = Gain_Corrected[id];
	else				ret = Gain_Raw[id];
	
	this->unlock();
	
	return ret;
}

/******************************
******************************/
double THREAD_FFT::getArrayVal_x_DispGain(int id, float Gui_DispGain, float ScreenHeight, bool b_Clamp, bool b_Correction)
{
	if((id < 0) || (AUDIO_BUF_SIZE/2 <= id)) return 0;
	
	double ret = 0;
	
	this->lock();
	if(b_Correction)	ret = ofMap(Gain_Corrected[id], 0, Gui_DispGain, 0, ScreenHeight, b_Clamp);
	else				ret = ofMap(Gain_Raw[id], 0, Gui_DispGain, 0, ScreenHeight, b_Clamp);
	this->unlock();
	
	return ret;
}

/******************************
******************************/
double THREAD_FFT::getPhase(int id)
{
	if((id < 0) || (AUDIO_BUF_SIZE/2 <= id)) return 0;
	
	double ret = 0;
	
	this->lock();
	ret = Phase[id];
	this->unlock();
	
	return ret;
}

/******************************
******************************/
void THREAD_FFT::update__Gain(const vector<float> &AudioSample)
{
	const float duration = 15.0;
	
	this->lock();
		/********************
		********************/
		float now = ofGetElapsedTimef();
		// if(fp_Log_fft != NULL) { if(now < duration) fprintf(fp_Log_fft, "%f,", now); }
			
		/********************
		********************/
		AudioSample_fft_LPF_saveToArray(AudioSample, now - LastInt);
		LastInt = now;
			
		/********************
		********************/
		/*
		if(fp_Log_fft != NULL){
			if(now < duration)	{ fprintf(fp_Log_fft, "%f\n", ofGetElapsedTimef()); }
			else				{ fclose(fp_Log_fft); fp_Log_fft = NULL; }
		}
		*/
	this->unlock();
}

/******************************
******************************/
double THREAD_FFT::cal_GainUpRatio(int Freq_id){
	double f = BaseFreq * Freq_id;
	
	double a = 1.0;
	
	return a * log10(f) - 2 * a + 1.0;
}

/******************************
******************************/
void THREAD_FFT::AudioSample_fft_LPF_saveToArray(const vector<float> &AudioSample, float dt)
{
	/********************
	********************/
	if( AudioSample.size() != N ) { ERROR_MSG(); std::exit(1); }
	
	/********************
	********************/
	double x[N], y[N];
	
	for(int i = 0; i < N; i++){
		x[i] = AudioSample[i] * fft_window[i];
		y[i] = 0;
	}
	
	fft(x, y);

	/********************
	********************/
	Gain_Raw[0] = 0;
	Gain_Corrected[0] = 0;
	for(int i = 1; i < N/2; i++){
		/********************
		********************/
		/* */
		double GainTemp = 2 * sqrt(x[i] * x[i] + y[i] * y[i]);
		
		Gain_Raw[i] = LPF(Gain_Raw[i], GainTemp, Gui_Global->LPFAlpha_dt__FFTGain, dt);
		Gain_Raw[N - i] = Gain_Raw[i]; // 共役(yの正負反転)だが、Gainは同じ
		
		/* */
		GainTemp = GainTemp * cal_GainUpRatio(i);
		
		Gain_Corrected[i] = LPF(Gain_Corrected[i], GainTemp, Gui_Global->LPFAlpha_dt__FFTGain, dt);
		Gain_Corrected[N - i] = Gain_Corrected[i]; // 共役(yの正負反転)だが、Gainは同じ
		
		/********************
		********************/
		double PhaseTemp = CalPhase(x[i], y[i], i);
		Phase[i] = LPF(Phase[i], PhaseTemp, Gui_Global->LPFAlpha_dt__FFTPhase, dt);
		Phase[N - i] = -Phase[i]; // 共役
	}
}

/******************************
******************************/
double THREAD_FFT::CalPhase(double x, double y, int id)
{
	if(x == 0){
		return 90.0;
	}else{
		double phase = atan(y/x);
		phase = phase * 180 / PI;
		
		if(phase < 0) phase += 180;
		
		phase /= id;
		
		return phase;
	}
}

/******************************
******************************/
int THREAD_FFT::fft(double x[], double y[], int IsReverse)
{
	/*****************
		bit反転
	*****************/
	int i, j;
	for(i = 0; i < N; i++){
		j = bitrev[i];
		if(i < j){
			double t;
			t = x[i]; x[i] = x[j]; x[j] = t;
			t = y[i]; y[i] = y[j]; y[j] = t;
		}
	}

	/*****************
		変換
	*****************/
	int n4 = N / 4;
	int k, ik, h, d, k2;
	double s, c, dx, dy;
	for(k = 1; k < N; k = k2){
		h = 0;
		k2 = k + k;
		d = N / k2;

		for(j = 0; j < k; j++){
			c = sintbl[h + n4];
			if(IsReverse)	s = -sintbl[h];
			else			s = sintbl[h];

			for(i = j; i < N; i += k2){
				ik = i + k;
				dx = s * y[ik] + c * x[ik];
				dy = c * y[ik] - s * x[ik];

				x[ik] = x[i] - dx;
				x[i] += dx;

				y[ik] = y[i] - dy;
				y[i] += dy;
			}
			h += d;
		}
	}

	/*****************
	*****************/
	if(!IsReverse){
		for(i = 0; i < N; i++){
			x[i] /= N;
			y[i] /= N;
		}
	}

	return 0;
}

/******************************
******************************/
void THREAD_FFT::make_bitrev(void)
{
	int i, j, k, n2;

	n2 = N / 2;
	i = j = 0;

	for(;;){
		bitrev[i] = j;
		if(++i >= N)	break;
		k = n2;
		while(k <= j)	{j -= k; k /= 2;}
		j += k;
	}
}

/******************************
******************************/
void THREAD_FFT::make_sintbl(void)
{
	for(int i = 0; i < N + N/4; i++){
		sintbl[i] = sin(2 * PI * i / N);
	}
}



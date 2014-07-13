/*
opensl_example.c:
OpenSL example module 
Copyright (c) 2012, Victor Lazzarini
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <android/log.h>
#include <stdio.h>
#include "opensl_io.h"

#define BUFFERFRAMES 	480	//(SR*10/1000)			//10ms
#define VECSAMPS_MONO 	480	//(SR*10/1000)			//10ms Mono
#define VECSAMPS_STEREO VECSAMPS_MONO*2				//10ms Stereo

#define FRAME_SIZE	BUFFERFRAMES

static int on;
int timestamp;
void start_process()
{
	FILE *RxInFile,*RxOutFile,*TxInFile,*TxOutFile;
	short int RxIn[FRAME_SIZE],TxIn[FRAME_SIZE],RxOut[FRAME_SIZE],TxOut[FRAME_SIZE];

	FILE *DelayAnalysisFile;
	short int MixIn[FRAME_SIZE*2];
	int silenceFilling = 0;

	OPENSL_STREAM  *p;
	int samps, i, j;
	float  inbuffer[VECSAMPS_MONO], outbuffer[VECSAMPS_STEREO];
	float  sample;
	p = android_OpenAudioDevice(SR,1,2,BUFFERFRAMES);
	if(p == NULL){
		LOGE("android_OpenAudioDevice(): failed.");
		return;
	}
	on = 1;

	RxInFile = fopen("/sdcard/rxin.pcm", "rb");
	TxInFile = fopen("/sdcard/txin.pcm", "wb");
	if ((RxInFile == NULL) || (TxInFile == NULL)){
		LOGE("File Operation Failed.");
		if(p == NULL){
			android_CloseAudioDevice(p);
		}
		return;
	}
	DelayAnalysisFile = fopen("/sdcard/mix.pcm", "wb");
	if (DelayAnalysisFile == NULL){
		LOGE("File delaymix.cpm Creation Failed.");
	}

	timestamp = 0;
	while(on)
	{
		/*--- Recording. Egress Direction (Tx) ---*/

		samps = android_AudioIn(p,inbuffer,VECSAMPS_MONO);

		for (i=0; i<samps; i++){
			sample = inbuffer[i]*32768.0f;
			sample = (sample>= 32767.0f?  32767: sample);
			sample = (sample<=-32768.0f? -32768: sample);

			TxIn[i] = (short int)sample;
		}
		fwrite(TxIn, sizeof(short int), samps, TxInFile);

		/*--- Playing. Ingress Direction (Rx) ---*/

		if (silenceFilling > 0){
			for (i=0; i<samps; i++) RxIn[i] = 0;
			silenceFilling -= samps;
		}
		else
		{
			if(fread(RxIn, sizeof(short int), samps,RxInFile) != samps){
				//Rewind to Loop
				fseek(RxInFile, 0L, SEEK_SET);
				for (i=0; i<samps; i++) RxIn[i] = 0;
				//Insert 5 seconds silent
				silenceFilling = 5 * SR;
			}
		}
		for (i = 0, j=0; i < samps; i++, j+=2){
			outbuffer[j] = outbuffer[j+1] = RxIn[i]*CONVMYFLT;
		}
		android_AudioOut(p,outbuffer,samps*2);

		/*--- For Delay Analysis ---*/
		if (DelayAnalysisFile != NULL){
			for (i=0,j=0; i < samps; i++, j+=2){
				MixIn[j  ] = RxIn[i];
				MixIn[j+1] = TxIn[i];
			}
			fwrite(MixIn, sizeof(short int), samps*2, DelayAnalysisFile);
		}

		timestamp += samps;
	}
	android_CloseAudioDevice(p);

	fclose(RxInFile);
	fclose(TxInFile);
	if (DelayAnalysisFile != NULL){ fclose(DelayAnalysisFile); }
	return;
}

void stop_process(){
	on = 0;
}

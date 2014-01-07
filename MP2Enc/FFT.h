#ifndef FFT_H
#define FFT_H

#include "Types.h"

void fft(float x_real[BLKSIZE],
         float energy[BLKSIZE],
         float phi[BLKSIZE],
         int N);

void MP2RealFour1024(float* pfData,float* pfEnergy);
//void MP3RealFour1024(float* pfData,float* pfEnergy,float* pfPhi);
//void MP3RealFour256(float* pfData,float* pfEnergy,float* pfPhi);


void fft_init();


#endif

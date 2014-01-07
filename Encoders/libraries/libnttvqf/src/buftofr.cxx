/* (c)Copyright 1996-2000 NTT Cyber Space Laboratories */
/*                Released on 2000.05.22 by N. Iwakami */

extern void buftofr(int, short[], float[], int, int);

void buftofr(int    blen,      /* Input  --- read data length */
			 short  bufin[],   /* Input  --- input data buffer array */
			 float in[],      /* Output --- output data frame */
			 int	frameSize,
			 int	numChannels)
{
    /*--- Variables ---*/
    int	ismp, ich, blen_ch;
	
    /*--- Initialization ---*/
    blen_ch = blen/numChannels;
    /*--- Main operation ---*/
    for ( ich=0; ich<numChannels; ich++ ){
		for ( ismp=0; ismp<blen_ch; ismp++ ){
			in[ich*frameSize+ismp] = (float)bufin[ismp*numChannels+ich];
		}
		for (ismp=blen_ch; ismp<frameSize; ismp++){
			in[ich*frameSize+ismp] = 0.;
		}
    }
}

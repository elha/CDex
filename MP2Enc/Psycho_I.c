/*
** Copyright (C) 2000 Albert L. Faber
** 
** Floating-Point processeing was added by DSPguru on March 2002.
** Modified Source-code can be found at http://DSPguru.doom9.net
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Leeser General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
#include "Psycho_I.h"
#include "tables.h"
#define LONDON                  /* enable "LONDON" modification */
#define MAKE_SENSE              /* enable "MAKE_SENSE" modification */
#define MI_OPTION               /* enable "MI_OPTION" modification */

#define POW10SPEED
 
 /**********************************************************************
*
*        This module implements the psychoacoustic model I for the
* MPEG encoder HEADER II. It uses simplified tonal and noise masking
* threshold analysis to generate SMR for the encoder bit allocation
* routine.
*
**********************************************************************/

int crit_band;
int *cbound;
int sub_size;

// this function reads in critical band boundaries
void read_cbound(int lay,int freq)
{
	switch (freq)
	{
		case 0:	cbound=table_2CB0;crit_band=sizeof(table_2CB0);	break;	// Freq 0
		case 1:	cbound=table_2CB1;crit_band=sizeof(table_2CB1);break;	// Freq 1
		case 2:	cbound=table_2CB2;crit_band=sizeof(table_2CB2);break;	// Freq 2
		case 4:	cbound=table_2CB4;crit_band=sizeof(table_2CB4);break;	// Freq 4
		case 5:	cbound=table_2CB5;crit_band=sizeof(table_2CB5);break;	// Freq 5
		case 6:	cbound=table_2CB6;crit_band=sizeof(table_2CB6);break;	// Freq 6
	
	}
	crit_band/=sizeof(table_2CB0[0]);
}        


// this function reads in frequency bands and bark values
void read_freq_band(g_ptr* ltg,int lay,int freq)
{
	g_thres* tables_THRESHOLD[]=
	{
		table_2TH0,		// Freq=0
		table_2TH1,		// Freq=1
		table_2TH2,		// Freq=2

		NULL,

		table_2TH4,		// Freq=4
		table_2TH5,		// Freq=5
		table_2TH6		// Freq=6
	
	};

	int	tables_SIZE[]=
	{
		sizeof(table_2TH0),		// Freq=0
		sizeof(table_2TH1),		// Freq=1
		sizeof(table_2TH2),		// Freq=2

		0,

		sizeof(table_2TH4),		// Freq=4
		sizeof(table_2TH5),		// Freq=5
		sizeof(table_2TH6)		// Freq=6
	
	};

	// read "Lay cb Freq" table
	if (lay==1)
	{
		MP2LibError("Layer1 not supported\n");
	}

	*ltg=tables_THRESHOLD[ freq ];

	sub_size=tables_SIZE[ freq ]/sizeof(table_2TH0[0]);
//	 sub_size=1;

}

// this function calculates the global masking threshold
void make_map(mask power[HAN_SIZE],g_thres* ltg)
{
	int i,j;

	for(i=1;i<sub_size;i++) 
	{
		for(j=ltg[i-1].line;j<=ltg[i].line;j++)
		{
			power[j].map = i;
		}
	}
}


DOUBLE add_db(DOUBLE a,DOUBLE b)
{
#ifdef POW10SPEED
	#define DBTAB 1000
	static int init=0;
	double diff;
	double x;
	int i;
	static double table[DBTAB];
	if (init==0) 
	{
        init++;
        for (i=0;i<DBTAB;i++) 
		{
          x=(double)i/10.0;
          table[i]=10 * log10 (1 + pow(10.0,x/10.0)) - x;
        // if (i%10==0)
           // printf("%2.1f %f  \n",x,table[i]);
        }
     }
	diff = (a-b);
   if (diff>=0.0) 
   {
        if (diff>99.0)
          return (a);
        return (a+table[(int)(10.0*fabs(diff))]);
   }
   if (diff<-0.0) 
   {
        if (diff<-99)
          return (b);
        return (b+table[(int)(10.0*fabs(diff))]);
   }

//   MP2LibError("add_db table overflow %lf %lf\n",a,b);
   a=pow(10.0,a/10.0);
   b=pow(10.0,b/10.0);
   return  10 * log10(a+b);
#else
	a = pow(10.0,a/10.0);
	b = pow(10.0,b/10.0);
	return 10 * log10(a+b);
#endif
}



/****************************************************************
*
*        Fast Fourier transform of the input samples.
*
****************************************************************/

// this function calculates an FFT analysis for the freq domain
void II_f_f_t(DOUBLE sample[FFT_SIZE],mask power[HAN_SIZE])      
{
	int i,j,k,L,l=0;
	int ip, le, le1;
	DOUBLE t_r, t_i, u_r, u_i;
	static int M, MM1, init = 0, N;
	DOUBLE *x_r, *x_i, *energy;
	static int *rev;
	static DOUBLE *w_r, *w_i;

 x_r = (DOUBLE *) calloc(1,sizeof(DFFT));
 x_i = (DOUBLE *) calloc(1,sizeof(DFFT));
 energy = (DOUBLE *) calloc(1,sizeof(DFFT));
 for(i=0;i<FFT_SIZE;i++) x_r[i] = x_i[i] = energy[i] = 0;
 if(!init){
    rev = (int *) calloc(1,sizeof(IFFT));
    w_r = (DOUBLE *) calloc(1,sizeof(D10));
    w_i = (DOUBLE *) calloc(1,sizeof(D10));
    M = 10;
    MM1 = 9;
    N = FFT_SIZE;
    for(L=0;L<M;L++){
       le = 1 << (M-L);
       le1 = le >> 1;
       w_r[L] = cos(PI/le1);
       w_i[L] = -sin(PI/le1);
    }
    for(i=0;i<FFT_SIZE;rev[i] = l,i++) for(j=0,l=0;j<10;j++){
       k=(i>>j) & 1;
       l |= (k<<(9-j));                
    }
    init = 1;
 }
 memcpy( (char *) x_r, (char *) sample, sizeof(DOUBLE) * FFT_SIZE);
 for(L=0;L<MM1;L++){
    le = 1 << (M-L);
    le1 = le >> 1;
    u_r = 1;
    u_i = 0;
    for(j=0;j<le1;j++){
       for(i=j;i<N;i+=le){
          ip = i + le1;
          t_r = x_r[i] + x_r[ip];
          t_i = x_i[i] + x_i[ip];
          x_r[ip] = x_r[i] - x_r[ip];
          x_i[ip] = x_i[i] - x_i[ip];
          x_r[i] = t_r;
          x_i[i] = t_i;
          t_r = x_r[ip];
          x_r[ip] = x_r[ip] * u_r - x_i[ip] * u_i;
          x_i[ip] = x_i[ip] * u_r + t_r * u_i;
       }
       t_r = u_r;
       u_r = u_r * w_r[L] - u_i * w_i[L];
       u_i = u_i * w_r[L] + t_r * w_i[L];
    }
 }
 for(i=0;i<N;i+=2){
    ip = i + 1;
    t_r = x_r[i] + x_r[ip];
    t_i = x_i[i] + x_i[ip];
    x_r[ip] = x_r[i] - x_r[ip];
    x_i[ip] = x_i[i] - x_i[ip];
    x_r[i] = t_r;
    x_i[i] = t_i;
    energy[i] = x_r[i] * x_r[i] + x_i[i] * x_i[i];
 }
 for(i=0;i<FFT_SIZE;i++) if(i<rev[i]){
    t_r = energy[i];
    energy[i] = energy[rev[i]];
    energy[rev[i]] = t_r;
 }
 for(i=0;i<HAN_SIZE;i++){    /* calculate power density spectrum */
    if (energy[i] < 1E-20) energy[i] = 1E-20;
    power[i].x = 10 * log10(energy[i]) + POWERNORM;
    power[i].next = STOP;
    power[i].type = FALSE;
 }
 mem_free((void **) &x_r);
 mem_free((void **) &x_i);
 mem_free((void **) &energy);
}

/****************************************************************
*
*         Window the incoming audio signal.
*
****************************************************************/

//.This function calculates a  Hann window for PCM (input)
// samples for a 1024-pt. FFT

void II_hann_win(DOUBLE sample[FFT_SIZE])
{
	register int	i;
	register DOUBLE sqrt_8_over_3;
	static int		init = 0;
	static DOUBLE	HanWindow[FFT_SIZE];

	// calculate window function for the Fourier transform
	if(!init)
	{  
		sqrt_8_over_3 = pow(8.0/3.0, 0.5);

		// Hann window formula
		for(i=0;i<FFT_SIZE;i++)
		{
			HanWindow[i]=sqrt_8_over_3*0.5*(1-cos(2.0*PI*i/(FFT_SIZE)))/FFT_SIZE;
		}
		init = 1;
	}

	// Apply Hanning window to data
	for(i=0;i<FFT_SIZE;i++)
	{
		sample[i] *= HanWindow[i];
	}
}


/*******************************************************************
*
*        This function finds the maximum spectral component in each
* subband and return them to the encoder for time-domain threshold
* determination.
*
*******************************************************************/
void II_pick_max(mask power[SBLIMIT],DOUBLE spike[HAN_SIZE])
{
#ifndef LONDON
	DOUBLE max;
	int i,j;

	// calculate the maximum spectral component in each subband from bound 4-16
	for(i=0;i<HAN_SIZE;spike[i>>4] = max, i+=16)
	{
		for(j=0, max = DBMIN;j<16;j++)
		{
			max = (max>power[i+j].x) ? max : power[i+j].x;
		}
	}
#else

	#define PMTAB 2200
	
	DOUBLE sum;
	int i,j;

	static int goober=0;
	static int init = 0;

	static double nlpow10[PMTAB];
	double y;

	if (init==0) 
	{
		init++;
		for (i=0;i<PMTAB;i++) 
		{
			y=(double)(i-PMTAB/2)/100;
			nlpow10[i]=pow(10.0, y);
			//if (i%10==0)
			//printf("%i %lf %lf\n",i,y,nlpow10[i]);
		}
	}

	// calculate the maximum spectral component in each subband from bound 4-16
	for(i=0;i<HAN_SIZE;spike[i>>4] = 10.0*log10(sum), i+=16)
	{
		for(j=0, sum = pow(10.0,0.1*DBMIN);j<16;j++)
		{
#ifdef POW10SPEED
			y=power[i+j].x*10;
			if ((y>-1099.0)&&(y<1099.0)) 
			{
				sum += nlpow10[(int)(y+1100)];
			}
			else 
			{
				sum += pow(10.0,0.1*power[i+j].x);
			}
#else
			sum += pow(10.0,0.1*power[i+j].x);
#endif
		}
	}
#endif
}

/****************************************************************
*
*        This function labels the tonal component in the power
* spectrum.
*
****************************************************************/

// this function extracts (tonal) sinusoidals from the spectrum
void II_tonal_label(mask power[HAN_SIZE],int* tone) 
{
 int i,j, last = LAST, first, run, last_but_one = LAST; /* dpwe */
 DOUBLE max;

 *tone = LAST;
 for(i=2;i<HAN_SIZE-12;i++){
    if(power[i].x>power[i-1].x && power[i].x>=power[i+1].x){
       power[i].type = TONE;
       power[i].next = LAST;
       if(last != LAST) power[last].next = i;
       else first = *tone = i;
       last = i;
    }
 }
 last = LAST;
 first = *tone;
 *tone = LAST;
 while(first != LAST){               /* the conditions for the tonal          */
    if(first<3 || first>500) run = 0;/* otherwise k+/-j will be out of bounds */
    else if(first<63) run = 2;       /* components in HEADER II, which         */
    else if(first<127) run = 3;      /* are the boundaries for calc.          */
    else if(first<255) run = 6;      /* the tonal components                  */
    else run = 12;
    max = power[first].x - 7;        /* after calculation of tonal   */
    for(j=2;j<=run;j++)              /* components, set to local max */
       if(max < power[first-j].x || max < power[first+j].x){
          power[first].type = FALSE;
          break;
       }
    if(power[first].type == TONE){   /* extract tonal components */
       int help=first;
       if(*tone==LAST) *tone = first;
       while((power[help].next!=LAST)&&(power[help].next-first)<=run)
          help=power[help].next;
       help=power[help].next;
       power[first].next=help;
       if((first-last)<=run){
          if(last_but_one != LAST) power[last_but_one].next=first;
       }
       if(first>1 && first<500){     /* calculate the sum of the */
          DOUBLE tmp;                /* powers of the components */
          tmp = add_db(power[first-1].x, power[first+1].x);
          power[first].x = add_db(power[first].x, tmp);
       }
       for(j=1;j<=run;j++){
          power[first-j].x = power[first+j].x = DBMIN;
          power[first-j].next = power[first+j].next = STOP;
          power[first-j].type = power[first+j].type = FALSE;
       }
       last_but_one=last;
       last = first;
       first = power[first].next;
    }
    else {
       int ll;
       if(last == LAST); /* *tone = power[first].next; dpwe */
       else power[last].next = power[first].next;
       ll = first;
       first = power[first].next;
       power[ll].next = STOP;
    }
 }
}

/****************************************************************
*
*        This function groups all the remaining non-tonal
* spectral lines into critical band where they are replaced by
* one single line.
*
****************************************************************/
        
void noise_label(mask* power,int* noise,g_thres* ltg)
{

#ifdef POW10SPEED

 int i,j, centre, last = LAST;
 double index, weight, sum;
 static int goober = 0;
 static int init = 0;
#define NLTAB 2200
 static double nlpow10[NLTAB];
 double y;

 if (init==0) {
   init++;
   for (i=0;i<NLTAB;i++) {
     y=(double)(i-NLTAB/2)/100;
     nlpow10[i]=pow(10.0, y);
     // if (i%10==0)
       // printf("%i %lf %lf\n",i,y,nlpow10[i]);
   }
 }
                              /* calculate the remaining spectral */
 for(i=0;i<crit_band-1;i++){  /* lines for non-tonal components   */
     for(j=cbound[i],weight = 0.0,sum = DBMIN;j<cbound[i+1];j++){
        if(power[j].type != TONE){
           if(power[j].x != DBMIN){
              sum = add_db(power[j].x,sum);
/* the line below and others under the "MAKE_SENSE" condition are an alternate
   interpretation of "geometric mean". This approach may make more sense but
   it has not been tested with hardware. */
#ifdef MAKE_SENSE
/* weight += pow(10.0, power[j].x/10.0) * (ltg[power[j].map].bark-i);
   bad code [SS] 21-1-93
 */
    y=power[j].x*10.0;
    if ((y>-1099.0)&&(y<1099.0)) {
    // printf("%lf %i : %lf\n",y, (int)(y+NLTAB/2),nlpow10[(int)(y+NLTAB/2)]);
    weight += nlpow10[(int)(y+NLTAB/2)] * (double) (j-cbound[i]) /
     (double) (cbound[i+1]-cbound[i]);  /* correction */
    }
    else {
    // printf("b:%lf\n",power[j].x/10.0);
    weight += pow(10.0,power[j].x/10.0) * (double) (j-cbound[i]) /
     (double) (cbound[i+1]-cbound[i]);  /* correction */
    }
#endif
              power[j].x = DBMIN;
           }
        }   /*  check to see if the spectral line is low dB, and if  */
     }      /* so replace the center of the critical band, which is */
            /* the center freq. of the noise component              */

#ifdef MAKE_SENSE
     if(sum <= DBMIN)  centre = (cbound[i+1]+cbound[i]) /2;
     else {
        index = weight/pow(10.0,sum/10.0);
        centre = cbound[i] + (int) (index * (double) (cbound[i+1]-cbound[i]) );
     } 
#else
     index = (double)( ((double)cbound[i]) * ((double)(cbound[i+1]-1)) );
     centre = (int)(pow(index,0.5)+0.5);
#endif

    /* locate next non-tonal component until finished; */
    /* add to list of non-tonal components             */
#ifdef MI_OPTION
     /* Masahiro Iwadare's fix for infinite looping problem? */
     if(power[centre].type == TONE) 
       if (power[centre+1].type == TONE) centre++; else centre--;
#else
     /* Mike Li's fix for infinite looping problem */
     if(power[centre].type == FALSE) centre++;

     if(power[centre].type == NOISE){
       if(power[centre].x >= ltg[power[i].map].hear){
         if(sum >= ltg[power[i].map].hear) sum = add_db(power[j].x,sum);
         else
         sum = power[centre].x;
       }
     }
#endif
     if(last == LAST) *noise = centre;
     else {
        power[centre].next = LAST;
        power[last].next = centre;
     }
     power[centre].x = sum;
     power[centre].type = NOISE;        
     last = centre;
 }        



#else


 int i,j, centre, last = LAST;
 DOUBLE index, weight, sum;
                              /* calculate the remaining spectral */
 for(i=0;i<crit_band-1;i++){  /* lines for non-tonal components   */
     for(j=cbound[i],weight = 0.0,sum = DBMIN;j<cbound[i+1];j++){
        if(power[j].type != TONE){
           if(power[j].x != DBMIN){
              sum = add_db(power[j].x,sum);
/* the line below and others under the "MAKE_SENSE" condition are an alternate
   interpretation of "geometric mean". This approach may make more sense but
   it has not been tested with hardware. */
#ifdef MAKE_SENSE
/* weight += pow(10.0, power[j].x/10.0) * (ltg[power[j].map].bark-i);
   bad code [SS] 21-1-93
 */
    weight += pow(10.0,power[j].x/10.0) * (DOUBLE) (j-cbound[i]) /
     (DOUBLE) (cbound[i+1]-cbound[i]);  /* correction */
#endif
              power[j].x = DBMIN;
           }
        }   /*  check to see if the spectral line is low dB, and if  */
     }      /* so replace the center of the critical band, which is */
            /* the center freq. of the noise component              */

#ifdef MAKE_SENSE
     if(sum <= DBMIN)  centre = (cbound[i+1]+cbound[i]) /2;
     else {
        index = weight/pow(10.0,sum/10.0);
        centre = cbound[i] + (int) (index * (DOUBLE) (cbound[i+1]-cbound[i]) );
     } 
#else
     index = (DOUBLE)( ((DOUBLE)cbound[i]) * ((DOUBLE)(cbound[i+1]-1)) );
     centre = (int)(pow(index,0.5)+0.5);
#endif

    /* locate next non-tonal component until finished; */
    /* add to list of non-tonal components             */
#ifdef MI_OPTION
     /* Masahiro Iwadare's fix for infinite looping problem? */
     if(power[centre].type == TONE) 
       if (power[centre+1].type == TONE) centre++; else centre--;
#else
     /* Mike Li's fix for infinite looping problem */
     if(power[centre].type == FALSE) centre++;

     if(power[centre].type == NOISE){
       if(power[centre].x >= ltg[power[i].map].hear){
         if(sum >= ltg[power[i].map].hear) sum = add_db(power[j].x,sum);
         else
         sum = power[centre].x;
       }
     }
#endif
     if(last == LAST) *noise = centre;
     else {
        power[centre].next = LAST;
        power[last].next = centre;
     }
     power[centre].x = sum;
     power[centre].type = NOISE;        
     last = centre;
 }   
#endif 
}

/****************************************************************
*
*        This function reduces the number of noise and tonal
* component for further threshold analysis.
*
****************************************************************/

void subsampling(mask power[HAN_SIZE],g_thres* ltg,int* tone,int* noise)
{
 int i, old;

 i = *tone; old = STOP;    /* calculate tonal components for */
 while(i!=LAST){           /* reduction of spectral lines    */
    if(power[i].x < ltg[power[i].map].hear){
       power[i].type = FALSE;
       power[i].x = DBMIN;
       if(old == STOP) *tone = power[i].next;
       else power[old].next = power[i].next;
    }
    else old = i;
    i = power[i].next;
 }
 i = *noise; 
 old = STOP;    /* calculate non-tonal components for */
 while(i!=LAST){            /* reduction of spectral lines        */
    if(power[i].x < ltg[power[i].map].hear){
       power[i].type = FALSE;
       power[i].x = DBMIN;
       if(old == STOP) *noise = power[i].next;
       else power[old].next = power[i].next;
    }
    else old = i;
    i = power[i].next;
 }
 i = *tone; old = STOP;
 while(i != LAST){                              /* if more than one */
    if(power[i].next == LAST)break;             /* tonal component  */
    if(ltg[power[power[i].next].map].bark -     /* is less than .5  */
       ltg[power[i].map].bark < 0.5) {          /* bark, take the   */
       if(power[power[i].next].x > power[i].x ){/* maximum          */
          if(old == STOP) *tone = power[i].next;
          else power[old].next = power[i].next;
          power[i].type = FALSE;
          power[i].x = DBMIN;
          i = power[i].next;
       }
       else {
          power[power[i].next].type = FALSE;
          power[power[i].next].x = DBMIN;
          power[i].next = power[power[i].next].next;
          old = i;
       }
    }
    else {
      old = i;
      i = power[i].next;
    }
 }
}

/****************************************************************
*
*        This function calculates the individual threshold and
* sum with the quiet threshold to find the global threshold.
*
****************************************************************/

void threshold(mask power[HAN_SIZE],g_thres* ltg,int* tone,int* noise,int bit_rate)
{
 int k, t;
 DOUBLE dz, tmps, vf;

 for(k=1;k<sub_size;k++){
    ltg[k].x = DBMIN;
    t = *tone;          /* calculate individual masking threshold for */
    while(t != LAST){   /* components in order to find the global     */
       if(ltg[k].bark-ltg[power[t].map].bark >= -3.0 && /*threshold (LTG)*/
          ltg[k].bark-ltg[power[t].map].bark <8.0){
          dz = ltg[k].bark-ltg[power[t].map].bark; /* distance of bark value*/
          tmps = -1.525-0.275*ltg[power[t].map].bark - 4.5 + power[t].x;
             /* masking function for lower & upper slopes */
          if(-3<=dz && dz<-1) vf = 17*(dz+1)-(0.4*power[t].x +6);
          else if(-1<=dz && dz<0) vf = (0.4 *power[t].x + 6) * dz;
          else if(0<=dz && dz<1) vf = (-17*dz);
          else if(1<=dz && dz<8) vf = -(dz-1) * (17-0.15 *power[t].x) - 17;
          tmps += vf;        
          ltg[k].x = add_db(ltg[k].x, tmps);
       }
       t = power[t].next;
    }

    t = *noise;        /* calculate individual masking threshold  */
    while(t != LAST){  /* for non-tonal components to find LTG    */
       if(ltg[k].bark-ltg[power[t].map].bark >= -3.0 &&
          ltg[k].bark-ltg[power[t].map].bark <8.0){
          dz = ltg[k].bark-ltg[power[t].map].bark; /* distance of bark value */
          tmps = -1.525-0.175*ltg[power[t].map].bark -0.5 + power[t].x;
             /* masking function for lower & upper slopes */
          if(-3<=dz && dz<-1) vf = 17*(dz+1)-(0.4*power[t].x +6);
          else if(-1<=dz && dz<0) vf = (0.4 *power[t].x + 6) * dz;
          else if(0<=dz && dz<1) vf = (-17*dz);
          else if(1<=dz && dz<8) vf = -(dz-1) * (17-0.15 *power[t].x) - 17;
          tmps += vf;
          ltg[k].x = add_db(ltg[k].x, tmps);
       }
       t = power[t].next;
    }
    if(bit_rate<96)ltg[k].x = add_db(ltg[k].hear, ltg[k].x);
    else ltg[k].x = add_db(ltg[k].hear-12.0, ltg[k].x);
 }
}

/****************************************************************
*
*        This function finds the minimum masking threshold and
* return the value to the encoder.
*
****************************************************************/

void II_minimum_mask(g_thres* ltg,DOUBLE ltmin[SBLIMIT],int sblimit)
{
	DOUBLE min;
	int i,j;

	j=1;
	// level of LTMIN for each subband check subband limit, and calculate the minimum masking
	for(i=0;i<sblimit;i++)
	{
		if(j>=sub_size-1)
		{
			ltmin[i] = ltg[sub_size-1].hear;
		}
		else 
		{
			min = ltg[j].x;

			while(ltg[j].line>>4 == i && j < sub_size)
			{
				if(min>ltg[j].x)  min = ltg[j].x;
					j++;
			}
			ltmin[i] = min;
		}
	}
}

/*****************************************************************
*
*        This procedure is called in musicin to pick out the
* smaller of the scalefactor or threshold.
*
*****************************************************************/

void II_smr(	DOUBLE	ltmin[SBLIMIT],
				DOUBLE	spike[SBLIMIT],
				DOUBLE	scale[SBLIMIT],
				int		sblimit
			)
{
	int		i;
	DOUBLE	max;

	// Determine the signal level for each subband
	// for the maximum scale factors
	for(i=0;i<sblimit;i++)
	{
		max = 20 * log10(scale[i] * 32768) - 10;
		if(spike[i]>max) max = spike[i];
			max -= ltmin[i];
		ltmin[i] = max;
	}
}
        
/****************************************************************
*
*        This procedure calls all the necessary functions to
* complete the psychoacoustic analysis.
*
****************************************************************/

void Psycho_I
	(
		PFLOAT	buffer[2],
		DOUBLE	scale[2][SBLIMIT], 
		DOUBLE	ltmin[2][SBLIMIT],
		PFRAME	pFrame
	)

{
	HEADER *pHeader = pFrame->pHeader;
	int   stereo = pFrame->nChannels;
	int   sblimit = pFrame->sblimit;
	int k,i, tone=0, noise=0;
	static char init = 0;
	static int off[2] = {256,256};
	DOUBLE *sample;
	DSBL *spike;
	static D1408 *fft_buf;
	static mask_ptr power;
	static g_ptr ltg;

 sample = (DOUBLE *) calloc(1,sizeof(DFFT));
 spike = (DSBL *) calloc(1,sizeof(D2SBL));
     /* call functions for critical boundaries, freq. */
 if(!init){  /* bands, bark values, and mapping */
    fft_buf = (D1408 *) calloc(1,sizeof(D1408) * 2);
    power = (mask_ptr ) calloc(1,sizeof(mask) * HAN_SIZE);
    if (pHeader->version == MPEG_AUDIO_ID) {
      read_cbound(pHeader->lay, pHeader->sampling_frequency);
      read_freq_band(&ltg, pHeader->lay, pHeader->sampling_frequency);
    } else {
      read_cbound(pHeader->lay, pHeader->sampling_frequency + 4);
      read_freq_band(&ltg, pHeader->lay, pHeader->sampling_frequency + 4);
    }
    make_map(power,ltg);
    for (i=0;i<1408;i++) fft_buf[0][i] = fft_buf[1][i] = 0;
    init = 1;
 }
 for(k=0;k<stereo;k++){  /* check pcm input for 3 blocks of 384 samples */
    for(i=0;i<MPEGFRAMESIZE;i++) fft_buf[k][(i+off[k])%1408]= (DOUBLE)buffer[k][i]/SCALE;
    for(i=0;i<FFT_SIZE;i++) sample[i] = fft_buf[k][(i+1216+off[k])%1408];
    off[k] += MPEGFRAMESIZE;
    off[k] %= 1408;
                            /* call functions for windowing PCM samples,*/
    II_hann_win(sample);    /* location of spectral components in each  */
    for(i=0;i<HAN_SIZE;i++) power[i].x = DBMIN;  /*subband with labeling*/
    II_f_f_t(sample, power);                     /*locate remaining non-*/
    II_pick_max(power, &spike[k][0]);            /*tonal sinusoidals,   */
    II_tonal_label(power, &tone);                /*reduce noise & tonal */
    noise_label(power, &noise, ltg);             /*components, find     */
    subsampling(power, ltg, &tone, &noise);      /*global & minimal     */
    threshold(power, ltg, &tone, &noise,         /*threshold, and sgnl- */
      bitrate[pHeader->version][pHeader->lay-1][pHeader->bitrate_index]/stereo); /*to-mask ratio*/
    II_minimum_mask(ltg, &ltmin[k][0], sblimit);
    II_smr(&ltmin[k][0], &spike[k][0], &scale[k][0], sblimit);        
 }
 mem_free((void **) &sample);
 mem_free((void **) &spike);
}

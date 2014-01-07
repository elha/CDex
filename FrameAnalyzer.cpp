/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2001 Albert L. Faber
** Copyright (C) 1999 - 2001 Marcel Müller (partly derived from FrameAnalyzer.cpp)
** http://cdexos.sourceforge.net/
** http://sourceforge.net/projects/cdexos 
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "StdAfx.h"
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <share.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <vector>
/*

//#define DEBUG
#undef DEBUG

#undef min
#undef max

template <class X>
inline const X min(const X &a, const X &b)
{  return (a>b)?b:a;
}
template <class X>
inline const X max(const X &a, const X &b)
{  return (a<b)?b:a;
}

// Error handling
struct Error
{  int err;
   char buf[1024];
   Error(int err, const char* fmt, ...);
};

Error::Error(int err, const char* fmt, ...)
:  err(err)
{
	// va_list va;
//   va_start((void*)va, fmt);
//   vsprintf(buf, fmt, va);
//   va_end(va);
}

// XING VBR header funcs ****************************************************
static int ExtractI4(const unsigned char* &buf)
{  // big endian extract
   int x = ( ( ( ( (buf[0] << 8) + buf[1] ) << 8) + buf[2] ) << 8) + buf[3];
   buf += 4;
   return x;
}

static void StoreI4(unsigned char* &buf, int i)
{  // big endian store
   buf[3] = (unsigned char)i;
   i >>= 8;
   buf[2] = (unsigned char)i;
   i >>= 8;
   buf[1] = (unsigned char)i;
   i >>= 8;
   buf[0] = (unsigned char)i;
   buf += 4;
}

#define FRAMES_FLAG     0x0001
#define BYTES_FLAG      0x0002
#define TOC_FLAG        0x0004
#define VBR_SCALE_FLAG  0x0008

#define FRAMES_AND_BYTES (FRAMES_FLAG | BYTES_FLAG)
// structure to receive extracted header
// toc may be NULL
struct XHEADDATA
{  int h_id;       // from MPEG header, 0=MPEG2, 1=MPEG1
   int samprate;   // determined from MPEG header
   int flags;      // from Xing header data
   int frames;     // total bit stream frames from Xing header data
   int bytes;      // total bit stream bytes from Xing header data
   int vbr_scale;  // encoded vbr scale from Xing header data
   unsigned char toc[100];// toc_buffer[100]
};

#ifdef DEBUG
void PrintXingHeader(const XHEADDATA* X)
{  printf("*** XING-Header ***\n"
          "ID            %i\n"
          "Samplingrate  %i\n"
          "Flags         %X\n"
          "Frames        %i\n"
          "Bytes         %i\n"
          "VBR scale     %i\n",
    X->h_id, X->samprate, X->flags, X->frames, X->bytes, X->vbr_scale);
   for (int i = 0; i < 100; ++i)
      printf("%3i ", X->toc[i]);
   puts("");
}
#endif

XHEADDATA* GetXingHeader(const unsigned char *buf)
// return 0=fail, 1=success
// X   structure to receive header data (output)
// buf bitstream input
{  XHEADDATA *X = new XHEADDATA;
   int head_flags;
   int h_id, h_mode, h_sr_index;
   static int sr_table[4] = { 44100, 48000, 32000, 99999 };

   // get Xing header data
   X->flags = 0;     // clear to null incase fail

   // get selected MPEG header data
   h_id       = (buf[1] >> 3) & 1;
   h_sr_index = (buf[2] >> 2) & 3;
   h_mode     = (buf[3] >> 6) & 3;

   // determine offset of header
   if (h_id)
      buf += h_mode != 3 ? 32+4 : 17+4;   // mpeg1
    else
      buf += h_mode != 3 ? 17+4 : 9+4;    // mpeg2

   if (buf[0] != 'X' || buf[1] != 'i' || buf[2] != 'n' || buf[3] != 'g')
      return NULL;                        // fail, header not found
   buf+=4;

   X->h_id = h_id;
   X->samprate = sr_table[h_sr_index];
   if (h_id == 0)
      X->samprate >>= 1;

   head_flags = X->flags = ExtractI4(buf);// get flags

   if (head_flags & FRAMES_FLAG)
      X->frames   = ExtractI4(buf);
   if (head_flags & BYTES_FLAG)
      X->bytes = ExtractI4(buf);

   if (head_flags & TOC_FLAG)
      memcpy(X->toc, buf, 100), buf+=100;

   X->vbr_scale = -1;
   if (head_flags & VBR_SCALE_FLAG)
      X->vbr_scale = ExtractI4(buf);

   //if( X->toc != NULL )
   //for(i=0;i<100;i++) {
   //    if( (i%10) == 0 ) printf("\n");
   //    printf(" %3d", (int)(X->toc[i]));
   //}

   return X;       // success
}

void PutXingHeader(unsigned char *buf, const XHEADDATA* X)
// X   structure to header data (input)
// buf bitstream input/output
{  int h_mode = (buf[3] >> 6) & 3;

   // determine offset of header
   if (X->h_id)
      buf += h_mode != 3 ? 32+4 : 17+4;   // mpeg1
    else
      buf += h_mode != 3 ? 17+4 : 9+4;    // mpeg2

   buf[0] = 'X';
   buf[1] = 'i';
   buf[2] = 'n';
   buf[3] = 'g';
   buf+=4;

   StoreI4(buf, X->flags);

   if (X->flags & FRAMES_FLAG)
      StoreI4(buf, X->frames);
   if (X->flags & BYTES_FLAG)
      StoreI4(buf, X->bytes);

   if (X->flags & TOC_FLAG)
      memcpy(buf, X->toc, 100), buf+=100;

   if (X->flags & VBR_SCALE_FLAG)
      StoreI4(buf, X->vbr_scale);
}

// mpeg structs (little endian) *********************************************
struct MPEG_Frame_Header
{  signed   FrameSync1    : 8;
   unsigned CRC           : 1;
   unsigned Layer         : 2;
   unsigned Version       : 2;
   signed   FrameSync2    : 3;
   unsigned Private       : 1;
   unsigned Padding       : 1;
   unsigned SamplingRate  : 2;
   unsigned BitRate       : 4;
   unsigned Emphasis      : 2;
   unsigned Copyright     : 2;
   unsigned ModeExtension : 2;
   unsigned Mode          : 2;
};

// mpeg tables
static const char sVersion[4][4] = {"2.5", {'�','0','1','�'}, "2", "1"};
static const char sLayer[4][4] = {{'�','0','0','�'}, "III", "II", "I"};
static const char sYesNo[2][4] = {"no", "yes"};
static const int iBitRate[2][4][16] = {
{  {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2},
   {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, -1},
   {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, -1},
   {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, -1}
},
{  {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2},
   {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, -1},
   {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, -1},
   {0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, -1}
} };
static const int iSampleRate[4][4] =
{  {11025, 12000, 8000, -1},
   {-2, -2, -2, -2},
   {22050, 24000, 16000, -1},
   {44100, 48000, 32000, -1}
};
static const char sMode[4][2] = {"S", {'J','S'}, {'D','C'}, {'S','C'}};
static const char sExtMode[2][4][8] =
{  {"4-31", "8-31", "12-31", "16-31"},
   {"off off", "on  off", "off on", "on  on"}
};
static const char sSExtMode[2][4][8] =
{  {"IS 4-31", "IS 8-31", {'I','S',' ','1','2','-','3','1'}, {'I','S',' ','1','6','-','3','1'}},
   {"normal", "IS", "MS-S", "IS+MS-S"}
};
static const char sCopyright[4][5] = {"none", "none", {'o','r','i','g','.'}, "copy"};
static const char sEmphasis[4][5] = {"none", {'5','0','/','1','5'}, "�10�", "CCIT"};

static const size_t iSideInfoLen[4] = {9, 17, 17, 32};

// control vars
//static const char* fname = NULL;
static int verbose = 0;
static int quiet = 0;
static int strip = 0;
static int nGainValue = 0;
static int z0 = 0;
static unsigned long gainnum = 0;

// internal vars
static FILE* fh;
static union
{  MPEG_Frame_Header hdr;
   unsigned char framebuf[4*65536];
   char strbuf[2048];
};
static MPEG_Frame_Header hdr_OK;
static unsigned long frame = 0;
static int bitrate;
static const int* brvect;
static int samprate;
static int framelen;
static int filelen;
static int bitrates[16] = {0};
static int smodes[4] = {0};
static int gainmin[2] = {256, 256};
static int gainmax[2] = {-1, -1};
static unsigned long gainavg[2] = {0};
static std::vector<size_t> frameloc;
static int strip1st = -1;
static int striplast = 0;
static XHEADDATA* xing = NULL;

// MP3 sideinfo funcs *******************************************************
static int handlegain(unsigned char* data, int shift, int ch, int nGainValue)
{  int raw = (data[0] << 8) | data[1];
   int val = (raw >> shift) & 0xFF;
   if (gainmin[ch] > val)
      gainmin[ch] = val;
   if (gainmax[ch] < val)
      gainmax[ch] = val;
   gainavg[ch] += val;
   ++gainnum;
   //printf("%i%3i ", ch, val);
   if (!val || !(nGainValue|z0)) // if val was 0 it is a good idea to leave val=0
      return 0;
   if (z0)
      val -= 64;
   if (nGainValue)
      val += nGainValue;
   if (val < 0)
   {  fputs("Gain underflow.\n", stderr);
      val = 0;
   } else if (val > 255)
   {  val = 255;
      fputs("Gain overflow.\n", stderr);
   }
   raw &= ~(0xFF << shift);
   raw |= val << shift;
   data[0] = (unsigned char)(raw >> 8);
   data[1] = (unsigned char)raw;
   return val;
}

static void handlesideinfo( int nGainValue )
{  if (hdr.Version < 2 || hdr.Layer != 1)
      return;  // currently only layer-III V1.0 and V2.0 supported
   int sitype = (hdr.Version & 1) | ((hdr.Mode != 3) << 1);
   unsigned char* sibuf = framebuf +4;
   // skip CRC
   if (!hdr.CRC)
      sibuf += 2;
   // read sideinfo
   size_t silen = iSideInfoLen[sitype];
   static const char zeros[32] = "";
   // ignore zero frames (i.e. VBR info's)
   if (memcmp(sibuf, zeros, silen) == 0)
   {  if (!xing)
         xing = GetXingHeader(framebuf);
      return;
   }
   int maxgain;
   //printf("-%i-", sitype);
   switch (sitype)
   {default:// mono, lsf
      maxgain = handlegain(sibuf+3, 2, 0, nGainValue);
      break;
    case 1: // mono
      maxgain = max(handlegain(sibuf+4, 1, 0, nGainValue ), handlegain(sibuf+12, 6, 0, nGainValue));
      break;
    case 2: // stereo, lsf
      maxgain = max(handlegain(sibuf+3, 1, 0, nGainValue ), handlegain(sibuf+11, 2, 1, nGainValue));
      break;
    case 3: // stereo
      maxgain = max( max(handlegain(sibuf+5, 7, 0, nGainValue), handlegain(sibuf+12, 4, 1, nGainValue)),
                     max(handlegain(sibuf+19, 1, 0, nGainValue), handlegain(sibuf+27, 6, 1, nGainValue)) );
   }
   // write sideinfo
   if (nGainValue|z0)
   {  long l = ftell(fh);
      if (fseek(fh, sibuf - framebuf - framelen, SEEK_CUR))
      {  fputs("Seek failed.\n", stderr);
         return;
      }
      if (fwrite(sibuf, silen, 1, fh) != 1)
         fputs("Failed to write sideinfo.\n", stderr);
      if (fseek(fh, l, SEEK_SET))
      {  fputs("Seek failed.\n", stderr);
         return;
      }
      if (z0)
         z0--;
   }
   // strip silence
   if (strip)
   {  int sum = 0;
      if (strip < 0 || (maxgain < 1 && strip1st == -1))
      {  // scan frame (because of bit reservoir)
         unsigned char* cp = sibuf + silen;
         unsigned char last = 0;
         int i = framebuf + framelen - cp;
         while (i--)
         {  sum += *cp != last;
            last = *cp++;
         }
      }  // store result
      if (sum > abs(strip) || (strip > 0 && maxgain >= 1))
      {  striplast = frame +1;
         if (strip1st == -1)
            strip1st = frame;
   }  }
}

static size_t calcframelen(const MPEG_Frame_Header hdr)
{  return (bitrate == 0 || samprate == 0)
    ? 0 : (hdr.Layer == 3)
      ? (12000 * bitrate / iSampleRate[3][hdr.SamplingRate] + hdr.Padding) << 2
      : 144000 * bitrate / iSampleRate[3][hdr.SamplingRate] + hdr.Padding;
}

static void printframeinfo(const MPEG_Frame_Header hdr)
{  printf("%-7li%-6i%-5.4s%-7.4s%-5.4s%3i   %5i  %-5.4s%-5i%-6.2s%-10.8s%-6.5s%.5s\n",
    frame, framelen, sVersion[hdr.Version], sLayer[hdr.Layer], sYesNo[!hdr.CRC],
    bitrate, max(iSampleRate[hdr.Version][hdr.SamplingRate], 0),
    sYesNo[hdr.Padding], hdr.Private, sMode[hdr.Mode],
    hdr.Mode == 1 ? sExtMode[hdr.Layer <= 1][hdr.ModeExtension] : "",
    sCopyright[hdr.Copyright], sEmphasis[hdr.Emphasis]);
}

static int parseoption(const char* cp)
{  switch (tolower(cp[0]))
   {default:
      throw Error(4, "Illegal option %s.", cp);
    case 'v':
      verbose = 1;
      break;
    case 'q':
      quiet = 1;
      break;
    case 's':
      if (cp[1] == 0)
         strip = -9;
       else
      {  size_t n = 0;
         sscanf(cp +1, "%i%ln", &strip, &n);
         if (n != strlen(cp +1))
            throw Error(4, "Integer constant expected (%s).", cp);
      }
      break;
    case 'l':
      {  size_t n = 0;
         sscanf(cp +1, "%i%ln", &nGainValue, &n);
         if (n != strlen(cp +1))
            throw Error(4, "Integer constant expected (%s).", cp);
      }
      break;
    case 'z':
      z0 = 1;
      sscanf(cp +1, "%i", &z0);
   }
   return 0;
}

int NormalizeMPEG(  const char* fname, int nGainValue )
{  // command line
   //if (argc < 2)
   //{  fputs("Usage: FrameAnalyzer <filename> [<options>]\n"
   //         "options: /l#   change gain by 1.5*# dB\n"
   //         "         /s[#] strip empty frames (threshold #)\n"
   //         "         /v    print info for every frame\n"
   //    , stderr);
   //   return 10;
   //}
   try
   {  

	   //for (int i = 1; i < argc; ++i)
    //  {  char* cp = argv[i];
    //     if (*cp == '/' || *cp == '-')
    //     {  int r = parseoption(cp+1);
    //        if (r)
    //           return r;
    //     } else if (fname)
    //        throw Error(10, "Syntax error in command line.");
    //      else
    //        fname = cp;
    //  }

      if (!fname)
         throw Error(10, "Filename required.");

      // open file
      int fhi = nGainValue|strip|z0


       ? _open(fname, O_RDWR|O_BINARY, SH_DENYWR, 0)
       : _open(fname, O_RDONLY|O_BINARY, SH_DENYNO, 0);

      if (fhi == -1)
         throw Error(5, "Error opening %s.n", fname);
      fh = fdopen(fhi, nGainValue|strip|z0 ? "r+b" : "rb");
      fprintf(stderr, "Scanning %s...\n", fname);

      // read tag
      char tag[128];
      if (fseek(fh, -128, SEEK_END))
         throw Error(4, "Seek to tag failed.");
      if (fread(&tag, 128, 1, fh) != 1)
         throw Error(4, "Read tag failed.");
      filelen = ftell(fh);
      rewind(fh);
      if (memcmp(tag, "TAG", 3) == 0)
      {  if (!quiet)
            printf("ID3 Tag:\n"
                   " Title:  %.30s\n"
                   " Artist: %.30s\n"
                   " Album:  %.30s\n"
                   " Year:   %.4s\n"
                   " Comment:%.30s\n\n", tag+3, tag+33, tag+63, tag+93, tag+97);
         filelen -= 128;
      }

      // scan file
      if (!quiet && verbose)
         puts("Frame  len.  ver. layer  CRC  kbps  sampl. pad. pri. mode  IS  MS-S  (C)   emph.");
      while(ftell(fh) < filelen)
      {  // read MPEG Header
         if (fread(&hdr, 4, 1, fh) != 1)
            break;
         // check header
         if ((hdr.FrameSync1 & hdr.FrameSync2) != -1)
         {  fprintf(stderr, "Illegal frame sync 0x%02X%X at %lX\n", hdr.FrameSync1&0xFF, hdr.FrameSync2&7, ftell(fh)-4);
            // try resync
          resync:
            fscanf(fh, "%*[^\377]\377");
          resync2:
            int c = getc(fh);
            if (c == EOF)
               break;
            if (c == 0xFF) // gcc runtime fix
               goto resync2;
            if (c < 0xE0)
               goto resync;
            c = getc(fh);
            if (c == EOF)
               break;
            if (c == 0xFF)
               goto resync2;
            // found possible sync
            fseek(fh, -3, SEEK_CUR);
            fprintf(stderr, "Resync at %lX\n", ftell(fh));
            continue;
         }
         brvect = iBitRate[hdr.Version != 3][hdr.Layer];
         bitrate = brvect[hdr.BitRate];
         switch (samprate)
         {case -1:
            fputs("bitrate bad (0xF)\n", stderr);
          case -2:
            bitrate = 0;
          default:;
         }
         samprate = iSampleRate[hdr.Version][hdr.SamplingRate];
         switch (samprate)
         {case -1:
            fputs("sampling rate bad (0xF)\n", stderr);
          case -2:
            samprate = 0;
          default:;
         }
         framelen = calcframelen(hdr);

         // print header
         if (verbose)
            printframeinfo(hdr);

         if (framelen > 4)
         {  // frame seems to be valid
            hdr_OK = hdr;
            int rem = ftell(fh) -4;
            frameloc.push_back(rem);
            rem = filelen - rem;
            if (framelen > rem)
            {  memset(framebuf +4, 0, framelen -4);
               framelen = rem;
               fputs("incomplete frame\n", stderr);
            }
            // statistic
            ++bitrates[hdr.BitRate];
            ++smodes[hdr.ModeExtension];

            // read frame
            if (fread(framebuf +4, framelen -4, 1, fh) != 1)
            {  fputs("Failed to read frame data.\n", stderr);
               break;
            }
            // analyze sideinfo
            handlesideinfo( nGainValue );

            // next frame
            ++frame;
         }
      }

      if (!frame)
         throw Error(1, "Warning: File %s contains no MPEG Frames.", fname);

      if (hdr_OK.Mode != 3) // stereo
         gainnum >>= 1;

      // restore last valid values
      brvect = iBitRate[hdr_OK.Version != 3][hdr_OK.Layer];
      bitrate = brvect[hdr_OK.BitRate];
      samprate = iSampleRate[hdr_OK.Version][hdr_OK.SamplingRate];
      framelen = calcframelen(hdr_OK);
      // last frame end
      frameloc.push_back(frameloc.back() + framelen);
      // print last header
      if (!verbose && !quiet)
      {  puts("Frame  len.  ver. layer  CRC  kbps  sampl. pad. pri. mode  IS  MS-S  (C)   emph");
         printframeinfo(hdr_OK);
      }
      // statistics
      if (!quiet)
      {  unsigned long hundrets = ((hdr_OK.Layer == 3 ? 384U : 1152U) * frame * 4) / (iSampleRate[3][hdr_OK.SamplingRate] / 25);
         printf("total: %2lu:%02lu:%05.2f [h:mm:ss.ss]\n"
          , hundrets / 360000U, hundrets / 6000U % 60U, hundrets % 6000U / 100.);
         if (hdr_OK.Mode == 1)
         {  puts("\nJS-Mode  frames     %");
            for (int i = 0; i < 4; ++i)
               if (smodes[i])
                  printf("%-8.8s %6i %5.1f\n"
                   , sSExtMode[hdr_OK.Layer == 1][i], smodes[i], 100.*smodes[i]/frame);
         }
         if (bitrates[hdr_OK.BitRate] != frame)
         {  char buf1[80] = "Rate", buf2[80] = "Frms", buf3[80] = "%   ";
            int n1 = 4, n2 = 4, n3 = 4;
            for (int i = 1; i < 15; ++i)
               if (bitrates[i])
               {  n1 += sprintf(buf1 + n1, " %4u", brvect[i]);
                  n2 += (bitrates[i] >= 10000)
                   ? sprintf(buf2 + n2, " %3uk", (bitrates[i]+500)/1000)
                   : sprintf(buf2 + n2, " %4u", bitrates[i]);
                  n3 += sprintf(buf3 + n3, "%5.1f", 100.*bitrates[i]/frame);
               }
            printf("\n%s\n%s\n%s\n", buf1, buf2, buf3);
            clearerr(fh);
            printf("average: %5.1f [kbps]\n"
             , ftell(fh) / (hdr_OK.Layer == 3 ? 48000. : 144000.) / frame * samprate);
         }
         if (gainnum)
         {  printf("\nGain  min.  max.  average\n"
                     "left  %-4i  %-4i  %.1f\n"
             , gainmin[0], gainmax[0], (double)gainavg[0]/gainnum);
            if (hdr_OK.Mode != 3)
               // stereo
               printf("right %-4i  %-4i  %.1f\n"
                , gainmin[1], gainmax[1], (double)gainavg[1]/gainnum);
      }  }

      // strip silence
      if (strip)
      {  size_t start = frameloc[max(strip1st, 0)];
         fprintf(stderr, "Strip: %i..%i  %lx..%lx\n", strip1st, striplast, start, frameloc[striplast]);
         size_t len = frameloc[striplast];
         // move
         if (start)
         {  len -= start;
            size_t c = len;

            if (xing)   // the xing header is never recognized as MP3 data
            {  // recreate xing header
               #ifdef DEBUG
               PrintXingHeader(xing);
               #endif
               if (fseek(fh, frameloc[0], SEEK_SET))
                  throw Error(3, "FATAL: Seek Xing failed.");
               int xlen = min(sizeof framebuf, frameloc[1] - frameloc[0]);
               if (fread(framebuf, xlen, 1, fh) != 1)
                  throw Error(3, "FATAL: Read Xing failed.");
               // update Xing header
               start -= xlen;
               xing->flags |= FRAMES_FLAG | BYTES_FLAG | TOC_FLAG;
               xing->frames = striplast - strip1st +1; // + Xing header
               xing->bytes = len + xlen;
               // recreate TOC
               for (int i = 0; i < 100; ++i)
                  xing->toc[i] = (unsigned char)( ( frameloc[(i * xing->frames + 50.0) /100.0 + strip1st] - start) * 256.0 / xing->bytes );
               #ifdef DEBUG
               PrintXingHeader(xing);
               #else
               // write Xing header
               PutXingHeader(framebuf, xing);
               rewind(fh);
               if (fwrite(framebuf, min(sizeof framebuf, frameloc[1] - frameloc[0]), 1, fh) != 1)
                  throw Error(1, "FATAL: Write Xing failed (%i).", errno);
               #endif
               len += xlen;
            } else
               rewind(fh);

            do
            {  int l = min(sizeof framebuf, c);
               if (fseek(fh, start, SEEK_CUR))
                  throw Error(3, "FATAL: Seek failed.");
               if (fread(framebuf, l, 1, fh) != 1)
                  throw Error(3, "FATAL: Read failed.");
               if (fseek(fh, -start-l, SEEK_CUR))
                  throw Error(3, "FATAL: Seek failed.");
               if (fwrite(framebuf, l, 1, fh) != 1)
                  throw Error(3, "FATAL: Write failed (%i).", errno);
               c -= l;
            } while (c);
         } else
         {  strip1st = 0;
            if (fseek(fh, frameloc[striplast], SEEK_SET))
               throw Error(3, "FATAL: Seek to tag failed.");
         }
         // write tag
         if (memcmp(tag, "TAG", 3) == 0)
         {  len += 128;
            if (fwrite(&tag, 128, 1, fh) != 1)
               throw Error(3, "FATAL: Write tag failed (%i).", errno);
         }
         fclose(fh);
         // who knows how to truncate a file in ANSI C
         sprintf(strbuf, "trunc \"%s\" %li", fname, len);
         system(strbuf);
      }
   } catch (const Error& err)
   {  fprintf(stderr, "%s\n", err.buf);
      delete xing;
      return err.err;
   }

   delete xing;
   return 0;
}
*/

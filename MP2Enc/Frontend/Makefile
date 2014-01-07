
CC = gcc

c_sources = \
	Main.c

mp2lib_sources = \
	../AvailableBits.c		\
	../Common.c			\
	../FFT.c				\
	../Layer2.c			\
	../MpegError.c			\
	../MP2Enc.c			\
	../Psycho_0.c			\
	../Psycho_I.c			\
	../Psycho_II.c			\
	../Subband.c			\
	../Tables.c			\
	../TimeEval.c			

mp2lib_obj = $(mp2lib_sources:.c=.o)
mp2lib_dep = $(mp2lib_sources:.c=.d)

OBJ = $(c_sources:.c=.o)
DEP = $(c_sources:.c=.d)


PG = -pg
#PG = -fomit-frame-pointer
OPTIM = -O9

#OPTIM = -O -g
REQUIRED = -DMFCMISC -DUNIX -DNDEBUG -DINLINE=inline
ARCH = -march=pentium 

INCLUDEDIR = ../
INCLUDE = -I $(INCLUDEDIR)

#Following TWEAKS are for gcc2.95.2 YMMV.
TWEAKS = -finline-functions -fexpensive-optimizations -ffast-math -malign-double \
	-mfancy-math-387 -funroll-loops -funroll-all-loops -pipe \
	-fschedule-insns2 -fno-strength-reduce
#EXTRATWEAKS = -DVBRDEBUG

WARNING = -Wall -Wstrict-prototypes -Wmissing-prototypes
WORKING = -DVBR -DIOFIX

#CC_SWITCHES = $(OPTIM) $(REQUIRED) $(ARCH) $(PG) $(TWEAKS) $(EXTRATWEAKS) $(WARNING) $(WORKING)
CC_SWITCHES = $(OPTIM) $(REQUIRED) $(ARCH) $(PG) $(INCLUDE)

PGM = mp2enc

LIBS =  -lm

UNAME = $(shell uname)
ifeq ($(UNAME),OS/2)
   SHELL=sh     
   PGM = mp2enc.exe
   LIBS =
endif

%.o: %.c 
	$(CC) $(CC_SWITCHES) -c $< -o $@

%.d: ../%.c
	$(SHELL) -ec '$(CC) -M $(CC_SWITCHES) ../$< | sed '\''s/$*.o/& $@/g'\'' > $@'

dep:../$(OBJ) Makefile
	$(CC) $(PG) -o $(PGM) $(OBJ) $(LIBS)

$(PGM):	../$(OBJ) Makefile
	$(CC) $(PG) -o $(PGM) ../$(OBJ) $(LIBS)

clean:
	-rm $(OBJ) $(DEP) $(mp2lib_obj) $(mp2lib_dep)


tags: TAGS


libmp2enc.a:  $(mp2lib_obj) Makefile
	$(AR) cr libmp2enc.a  $(mp2lib_obj)
	$(RANLIB) libmp2enc.a


TAGS: ${c_sources} ${mp2lib_sources}
	etags -T ${c_sources}

-include $(DEP)

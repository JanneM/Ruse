AM_CFLAGS = -std=gnu99 -Wall -O2 --openmp
AM_LDFLAGS = -lm -lrt 

bin_PROGRAMS = ruse_multiproc ruse_omp
ruse_multiproc_SOURCES = ruse_multiproc.c \
			 options_multiproc.c options_multiproc.h \
			 do_task.c do_task.h
ruse_omp_SOURCES = ruse_omp.c \
		   options_omp.c options_omp.h \
		   do_task.c do_task.h

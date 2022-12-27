# -*- Makefile -*-
# using Windows VScode
#	https://code.visualstudio.com/docs/languages/cpp
# using gfortran from MSYS2.org
#
# replicating instructions from https://irimodel.org/IRI-2016/00readme.txt
#
# gfortran $(OBJ) -o iri //sample output
#	

# fortran codes have local absolute path so everything in 1 directory
LPATH = .
LIB = iri
OBJ = irisub.o irifun.o iriflip.o iridreg.o iritec.o cira.o igrf.o
UOBJ = cassess1.o iritest.o
F77 = gfortran -std=legacy
CC = gcc		# using C compiler explicitly

all: assess1 libiri.a

assess1: $(UOBJ)
	$(CC) -o assess1 $(UOBJ) -L$(LPATH) -l$(LIB) -lgfortran

cassess1.o: cassess1.c
	$(CC) -c cassess1.c

iritest.o: iritest.for
	$(F77) -c iritest.for

libiri.a: $(OBJ)
	ar r libiri.a $(OBJ)

irisub.o: irisub.for
	$(F77) -c irisub.for

irifun.o: irifun.for
	$(F77) -c irifun.for

iriflip.o: iriflip.for
	$(F77) -c iriflip.for

iridreg.o: iridreg.for
	$(F77) -c iridreg.for

iritec.o: iritec.for
	$(F77) -c iritec.for

cira.o: cira.for
	$(F77) -c cira.for

igrf.o: igrf.for
	$(F77) -c igrf.for

# using del for windows os
clean:
	del -f *.o *.exe
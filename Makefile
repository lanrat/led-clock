CXXFLAGS=-Wall -O3 -g --std=c++11

RGB_INCDIR=matrix/include
RGB_LIBDIR=matrix/lib
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a
LDFLAGS+=-L$(RGB_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread

LDFLAGS+=`curl-config --cflags --libs` `xml2-config --cflags --libs`
LDFLAGS+=-lwiringPi


default: clock

$(RGB_LIBRARY): FORCE
	$(MAKE) -C $(RGB_LIBDIR)

clock: http.cc http.h muni.cc muni.h brightness.cc brightness.h clock.cc $(RGB_LIBRARY)
	$(CXX) -I$(RGB_INCDIR) $(CXXFLAGS) http.cc muni.cc brightness.cc clock.cc -o $@ $(LDFLAGS)

test: test.cc $(RGB_LIBRARY)
	$(CXX) -I$(RGB_INCDIR) $(CXXFLAGS) test.cc -o $@ $(LDFLAGS)

photo: photo.c
	$(CXX) $(CXXFLAGS) photo.c -o $@ $(LDFLAGS)

FORCE:
.PHONY: FORCE

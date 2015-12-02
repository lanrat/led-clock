CXXFLAGS=-Wall -O3 -g --std=c++11

RGB_INCDIR=matrix/include
RGB_LIBDIR=matrix/lib
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a
LDFLAGS+=-L$(RGB_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread

LDFLAGS+=`curl-config --cflags --libs` `xml2-config --cflags --libs`


default: clock

$(RGB_LIBRARY): FORCE
	$(MAKE) -C $(RGB_LIBDIR)

clock: http.cc muni.cc clock.cc $(RGB_LIBRARY)
	$(CXX) -I$(RGB_INCDIR) $(CXXFLAGS) http.cc muni.cc clock.cc -o $@ $(LDFLAGS)

test: test.cc $(RGB_LIBRARY)
	$(CXX) -I$(RGB_INCDIR) $(CXXFLAGS) test.cc -o $@ $(LDFLAGS)

FORCE:
.PHONY: FORCE

CXXFLAGS=-Wall -O3 -g --std=c++11

RGB_INCDIR=matrix/include
RGB_LIBDIR=matrix/lib
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a
RGB_LD_FLAGS=-L$(RGB_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread
RGB_INC_FLAGS=-I$(RGB_INCDIR)

XML_LD_FLAGS=`xml2-config --libs`
XML_INC_FLAGS=`xml2-config --cflags`

CURL_LD_FLAGS=`curl-config --libs`
CURL_INC_FLAGS=`curl-config --cflags`

GPIO_LD_FLAGS=-lwiringPi

default: clock

$(RGB_LIBRARY): FORCE
	$(MAKE) -C $(RGB_LIBDIR)

virtualcanvas.o: virtualcanvas.cc virtualcanvas.h $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) $(RGB_INC_FLAGS) -c virtualcanvas.cc

weather.o: weather.cc weather.h
	$(CXX) $(CXXFLAGS) $(XML_INC_FLAGS) -c weather.cc

brightness.o: brightness.cc brightness.h
	$(CXX) $(CXXFLAGS) -c brightness.cc

http.o: http.cc http.h
	$(CXX) $(CXXFLAGS) $(CURL_INC_FLAGS) -c http.cc

muni.o: muni.cc muni.h
	$(CXX) $(CXXFLAGS) $(XML_INC_FLAGS) -c muni.cc

clock: stop http.o muni.o brightness.o weather.o virtualcanvas.o clock.cc $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) $(RGB_INC_FLAGS) http.o weather.o virtualcanvas.o muni.o brightness.o clock.cc -o $@ $(RGB_LD_FLAGS) $(XML_LD_FLAGS) $(CURL_LD_FLAGS) $(GPIO_LD_FLAGS)

test: test.cc $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) $(RGB_INC_FLAGS) test.cc -o $@ $(RGB_LD_FLAGS)

photo: photo.c brightness.o
	$(CXX) $(CXXFLAGS) brightness.o photo.c -o $@ $(GPIO_LD_FLAGS)

clean:
	rm -f brightness.o http.o muni.o virtualcanvas.o weather.o clock photo test

start:
	systemctl start clock.service

stop:
	systemctl stop clock.service

install: clock.service
	ln -f -s /root/led-clock/clock.service /etc/systemd/system/clock.service
	ln -f -s /etc/systemd/system/clock.service /etc/systemd/system/multi-user.target.wants/clock.service
	systemctl daemon-reload


FORCE:
.PHONY: FORCE

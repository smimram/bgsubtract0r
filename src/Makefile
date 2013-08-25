NAME = bgsubtract0r

all: $(NAME).o
	gcc -shared -O3 -lc -o $(NAME).so $(NAME).o

test: all
	LIQ_FREI0R_PATH=. ./test.liq

help:
	LIQ_FREI0R_PATH=. liquidsoap -h video.frei0r.bgsubstract0r

noauto:
	uvcdynctrl -s "White Balance Temperature, Auto" 0

%.o: %.c
	gcc -c -fPIC -lm $^

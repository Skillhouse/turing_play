



turing: Turing.cpp
	gcc -o $@ $<


clean:
	rm turing frame-*.pgm out.mkv


video:	turing
	./turing -i 2000 --every 5
	avconv -i 'frame-%04d.pgm' -r 5 out.mkv

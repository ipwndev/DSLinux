CFLAGS=-g -Wall -O2
LDFLAGS=

all: improcess genps pbm2png clip generate_font arrow

clean:
	rm -f improcess gepns core pbm2png clip generate_font arrow

improcess: improcess.c
	gcc -o improcess $(CFLAGS) $(LDFLAFGS) -lpng improcess.c

pbm2png: pbm2png.c
	gcc -o pbm2png $(CFLAGS) $(LDFLAGS) -lpng pbm2png.c

genps: genps.c

clip: clip.c

generate_font: generate_font.c
	gcc -o generate_font -lm -lpng -lz $(CFLAGS) $(LDFLAGS) generate_font.c

arrow: arrow.c
	gcc -o arrow $(CFLAGS) $(LDFLAGS) arrow.c -lpng

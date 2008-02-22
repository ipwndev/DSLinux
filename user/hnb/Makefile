
src/hnb: src/*.c src/*.h src/*.inc src/hnbrc.inc
	(cd src;make)
#install: src/hnb
#	install -D src/hnb /usr/local/bin/hnb
#	install -D -m444 doc/hnb.1 /usr/local/man/man1/hnb.1
clean:
	(cd src;make clean)
	(cd util;make clean)
	rm -f *~

rcupdate: updaterc
updaterc: src/hnbrc.inc

src/hnbrc.inc: doc/hnbrc
	(cd util;make)
	echo -n "\"">src/hnbrc.inc
	cat doc/hnbrc | util/asc2c >> src/hnbrc.inc
	echo "\"">>src/hnbrc.inc


romfs:
	$(ROMFSINST) src/hnb /bin/hnb
	$(ROMFSINST) -m doc/hnb.1 /usr/share/man/hnb

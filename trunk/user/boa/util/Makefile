all:	cpsel	cp-test boa_indexer

clean:	
	rm cpsel cp-test boa_indexer

cpsel:	cpsel.c	cpsel.config.h
	$(CC)  $(LDFLAGS) -o cpsel cpsel.c

cp-test: cp-test.c
	$(CC)  $(LDFLAGS) -o cp-test cp-test.c
	
boa_indexer: index_dir.c
	$(CC) $(LDFLAGS) -o boa_indexer index_dir.c

romfs:
	$(ROMFSINST) boa_indexer /bin/

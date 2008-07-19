for i in *.c *.h; do echo $i; indent -br -brs -bad -bap -bbb -ncs -ce -nut -i4 -npcs -nbbo -cdb -sc -l80 "$i"; done; rm *.BAK *~

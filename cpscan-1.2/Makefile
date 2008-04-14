
CC=gcc
STRIP=strip
COPY=cp
CLS=clear
SAY=@echo
VIEW=less

default:
	$(SAY)  "make pt (for Portuguese)(Not maintained!)"
	$(SAY)  "make pl (for Polish)"
	$(SAY)  "make en (for english)"
	$(SAY)  "make no (for Norwegian)"
	$(SAY)  "make fr (for french)"
	$(SAY)	"make strip to strip binaries"
	$(SAY)	"make install to install in /usr/local/bin"
	$(SAY)  "make lic to view license."
pt: 
	$(CLS)
	$(CC) -o cpscan cpscan-pt.c
en:
	$(CLS)
	$(CC) -o cpscan cpscan.c
no:	
	$(CLS)
	$(CC) -o cpscan cpscan-no.c
fr:
	$(CLS)
	$(CC) -o cpscan cpscan-fr.c
fr:
	$(CLS)
	$(CC) -o cpscan cpscan-pl.c
install:
	$(COPY) cpscan /usr/local/bin
strip:	
	$(STRIP) cpscan
lic:
	$(VIEW) license.txt	

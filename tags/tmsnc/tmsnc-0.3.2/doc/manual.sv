                                                  Tmsnc
   Updated: November 2005
   Language: Svenska
   Translation: Sanoi <sanoix@gmail.com>
     ------------------------------------------------------------------------------------------------

    

NAMN

   tmsnc - Textbaserad MSN klient f�r UNIX  

SUMMERING

   tmsnc [-l adress] [-i status] [-uv]  

BESKRIVNING
   
   tmsnc �r en textbaserad chat-klient som anv�nder sig av MSN protokollet. Gr�nssnittet �r skrivet i
   Ncurses biblioteket och SSL delen i OpenSSL. Det g�r programmet v�ldigt portabelt och det borde vara
   m�jligt att anv�nda p� vilket UNIX-liknande system som helst.


VALM�JLIGHETER

   -l ADRESS
           Logga in med adressen <ADRESS>.
   -u
           Kolla efter uppdateringar.
   -i STATUS
           St�ll in s� att den status du har n�r du loggar in visas som STATUS, d�r STATUS kan
           vara "online", "away", "brb", "busy", "idle", "phone", "lunch" eller "hidden".
   -v
           Visa information om den aktuella versionen.

    

MENY

   Anv�nd TABB f�r att byta mellan de olika listorna och f�nstrena i gr�nssnittet.
   F�r att �ppna en meny, tryck p� returntangenten. F�r att v�lja ett av f�rem�len i
   menyn anv�nd piltangenterna och tryck sedan return-knappen f�r att k�ra kommandot.


TALKFILTER

   F�r att m�jligg�ra �vers�ttning av utg�ende meddelanden med hj�lp av talkfilters s� m�ste
   du f�rst installera libtalkfilters (http://www.hyperrealm.com/main.php?s=talkfilters) och sedan
   kompilera tmsnc igen. Sen kan du g� till "Actions->Set filter" i menyn. Skriv in ett av dessa
   filter: austro, b1ff, brooklyn, chef, cockney, drawl, dubya, fudd, funetak, jethro, jive,
   kraut, pansy, pirate, postmodern, redneck, valspeak eller warez. Du kan �ven st�nga av
   talkfilters genom att skriva in "none". Nu kommer alla dina utg�ende meddelanden att
   bli �versatta.
    

KONTAKTLISTAN

   Anv�nd piltangenterna f�r att v�lja en kontakt. Tryck p� returtangenten f�r att starta
   en konversation. N�r du har en kontakt markerad s� kan du g� till menyn och v�lja
   "Contacts->Set custom nick" f�r att ge den markerade kontakten ett nytt smeknamn.
   Du kan �ven ta bort/�terst�lla detta namn genom att v�lja "Contacts->Un-set custom nick".
   


KONVERSATIONSLISTAN

   F�r att byta f�nster till en �ppen konversation, markera konversationen med piltangenterna i listan
   och tryck p� returtangenten. En konversation kommer att bli markerad gr�n i listan om det finns
   ol�sta meddelanden.
   
   I kontaktlistan kan du anv�nda f�ljande kortkommandon.

   a      L�gg till den markerade personen (om personen redan finns p� kontaktlistan s� l�gg till en ny)

   d      Ta bort den markerade personen.

   b      Blockera den markerade personen.

   u      Ta bort blockeringen p� den markerade kontakten.


KORTKOMMANDON

   alt-c (eller F6)
           St�ng det aktiva f�nstret.
   alt-<nummer>
           Byt konversation till <nummer>. Om <nummer> �r 0 s� blir consol-f�nstret aktivt.
   alt-a
           Fokusera p� menyn.
   alt-s
           Fokusera p� kontaktlistan.
   alt-x
           Fokusera p� konversationslistan.
   alt-z
           Fokusera p� chat-f�nstret.
    

SUPERFUNKTIONER

   F�r att f� upp prompten f�r att skriva in ett superkommando, tryck ctrl-p.

   Tillg�nliga kommandon:
   block all
           Blockera alla kontakter.
   unblock all
           Ta bort blockeringen p� alla kontakter.
   remove all
           Ta bort alla kontakter (VARNING: detta kan ej �ngras).
   whoami
           Visa vem du �r inloggad som.
   export conlist
           Skriv kontaktlistan till en fil.
   import conlist
           L�s in en kontaktlista fr�n en fil.

    

AKTUELLT SPELADE L�T

   F�r att visa vilken l�t du spelar f�r tillf�llet s� m�ste du �ndra i ~/.tmsnc/tmsnc.conf f�rst.
   Ta bort '#' framf�r display_current_song alternativet. TMSNC uppdaterar den aktuellt spelade l�ten
   var femtonde sekund genom att k�ra skalmanuset ~/.tmsnc/current_song.sh. Resultatet av detta
   current_song.sh skickas som l�tens namn.

   Kolla h�r f�r att se exempel p� hur current_song.sh kan se ut.

   http://tmsnc.sourcefore.net/csong.html

    

FILER

   ~/.tmsnc/tmsnc.conf
           Konfigurationsfil
   ~/.tmsnc/aliases.conf
           Aliasfil
   ~/.tmsnc/logs/
           Konversationsloggar
    

UPPHOVSM�N

   tmsnc �r till st�rsta delen skrivet av Sanoi <sanoix@gmail.com>

   Tmsnc �r gratis mjukvara; du kan distribuera det och/eller modifiera det om du f�ljer formuleringarna
   i licensen IR Public Domain License som den �r publiserad av IR Group; antingen version 1.6 av
   licensen, eller (om du v�ljer det) en nyare verision. 
   
   Programmet �r distribuerat under f�rhoppningen att det kommer att vara anv�ndbart, men UTAN
   N�GON SOM HELST GARANTI.

   Du borde ha f�tt med en kopia av IR Public Domain Licensen med det h�r programmet; om inte,
   skriv till sanoix@gmail.com.

     ------------------------------------------------------------------------------------------------

                                                  Tmsnc
   Updated: November 2005
   Language: Svenska
   Translation: Sanoi <sanoix@gmail.com>
     ------------------------------------------------------------------------------------------------

    

NAMN

   tmsnc - Textbaserad MSN klient för UNIX  

SUMMERING

   tmsnc [-l adress] [-i status] [-uv]  

BESKRIVNING
   
   tmsnc är en textbaserad chat-klient som använder sig av MSN protokollet. Gränssnittet är skrivet i
   Ncurses biblioteket och SSL delen i OpenSSL. Det gör programmet väldigt portabelt och det borde vara
   möjligt att använda på vilket UNIX-liknande system som helst.


VALMÖJLIGHETER

   -l ADRESS
           Logga in med adressen <ADRESS>.
   -u
           Kolla efter uppdateringar.
   -i STATUS
           Ställ in så att den status du har när du loggar in visas som STATUS, där STATUS kan
           vara "online", "away", "brb", "busy", "idle", "phone", "lunch" eller "hidden".
   -v
           Visa information om den aktuella versionen.

    

MENY

   Använd TABB för att byta mellan de olika listorna och fönstrena i gränssnittet.
   För att öppna en meny, tryck på returntangenten. För att välja ett av föremålen i
   menyn använd piltangenterna och tryck sedan return-knappen för att köra kommandot.


TALKFILTER

   För att möjliggöra översättning av utgående meddelanden med hjälp av talkfilters så måste
   du först installera libtalkfilters (http://www.hyperrealm.com/main.php?s=talkfilters) och sedan
   kompilera tmsnc igen. Sen kan du gå till "Actions->Set filter" i menyn. Skriv in ett av dessa
   filter: austro, b1ff, brooklyn, chef, cockney, drawl, dubya, fudd, funetak, jethro, jive,
   kraut, pansy, pirate, postmodern, redneck, valspeak eller warez. Du kan även stänga av
   talkfilters genom att skriva in "none". Nu kommer alla dina utgående meddelanden att
   bli översatta.
    

KONTAKTLISTAN

   Använd piltangenterna för att välja en kontakt. Tryck på returtangenten för att starta
   en konversation. När du har en kontakt markerad så kan du gå till menyn och välja
   "Contacts->Set custom nick" för att ge den markerade kontakten ett nytt smeknamn.
   Du kan även ta bort/återställa detta namn genom att välja "Contacts->Un-set custom nick".
   


KONVERSATIONSLISTAN

   För att byta fönster till en öppen konversation, markera konversationen med piltangenterna i listan
   och tryck på returtangenten. En konversation kommer att bli markerad grön i listan om det finns
   olästa meddelanden.
   
   I kontaktlistan kan du använda följande kortkommandon.

   a      Lägg till den markerade personen (om personen redan finns på kontaktlistan så lägg till en ny)

   d      Ta bort den markerade personen.

   b      Blockera den markerade personen.

   u      Ta bort blockeringen på den markerade kontakten.


KORTKOMMANDON

   alt-c (eller F6)
           Stäng det aktiva fönstret.
   alt-<nummer>
           Byt konversation till <nummer>. Om <nummer> är 0 så blir consol-fönstret aktivt.
   alt-a
           Fokusera på menyn.
   alt-s
           Fokusera på kontaktlistan.
   alt-x
           Fokusera på konversationslistan.
   alt-z
           Fokusera på chat-fönstret.
    

SUPERFUNKTIONER

   För att få upp prompten för att skriva in ett superkommando, tryck ctrl-p.

   Tillgänliga kommandon:
   block all
           Blockera alla kontakter.
   unblock all
           Ta bort blockeringen på alla kontakter.
   remove all
           Ta bort alla kontakter (VARNING: detta kan ej ångras).
   whoami
           Visa vem du är inloggad som.
   export conlist
           Skriv kontaktlistan till en fil.
   import conlist
           Läs in en kontaktlista från en fil.

    

AKTUELLT SPELADE LÅT

   För att visa vilken låt du spelar för tillfället så måste du ändra i ~/.tmsnc/tmsnc.conf först.
   Ta bort '#' framför display_current_song alternativet. TMSNC uppdaterar den aktuellt spelade låten
   var femtonde sekund genom att köra skalmanuset ~/.tmsnc/current_song.sh. Resultatet av detta
   current_song.sh skickas som låtens namn.

   Kolla här för att se exempel på hur current_song.sh kan se ut.

   http://tmsnc.sourcefore.net/csong.html

    

FILER

   ~/.tmsnc/tmsnc.conf
           Konfigurationsfil
   ~/.tmsnc/aliases.conf
           Aliasfil
   ~/.tmsnc/logs/
           Konversationsloggar
    

UPPHOVSMÄN

   tmsnc är till största delen skrivet av Sanoi <sanoix@gmail.com>

   Tmsnc är gratis mjukvara; du kan distribuera det och/eller modifiera det om du följer formuleringarna
   i licensen IR Public Domain License som den är publiserad av IR Group; antingen version 1.6 av
   licensen, eller (om du väljer det) en nyare verision. 
   
   Programmet är distribuerat under förhoppningen att det kommer att vara användbart, men UTAN
   NÅGON SOM HELST GARANTI.

   Du borde ha fått med en kopia av IR Public Domain Licensen med det här programmet; om inte,
   skriv till sanoix@gmail.com.

     ------------------------------------------------------------------------------------------------

# Corona-Info#

## von fred_feuerstein (NI-Team)

In der aktuellen Situation mit Corona interessieren evt. einige von euch die aktuellen Fallzahlen von ausgew�hlten L�ndern.

Ich habe ein kleines Plugin gebaut, das aus der Quelle: https://corona.lmao.ninja/countries f�r ausgew�hlte L�nder die Fallzahlen holt.
Wenn euch noch weitere L�nder interessieren, gebt hier im Thread Bescheid.

Aktuelle L�nder in der �bersicht: Deutschland, Italien, Spanien, USA, Oesterreich, Frankreich, Schweiz, Niederlande, China, UK, ...

Die L�nder k�nnen in der Datei: corona.land im Plugin-Verzeichnis ge�ndert (hinzugef�gt und gel�scht) und anders sortiert werden! Bitte die Struktur der Datei so belassen.

Installation:

- Zip Datei entpacken und die 4 Dateien nach /var/tuxbox/plugins (oder euer entsprechendes anderes Plugin-Verzeichnis) kopieren
- Rechte der corona.so Datei auf 755 �ndern.
- Plugins neu laden im Men�, oder Box neu starten.
- wer Probleme mit dem WGET Abfruf hat kann am Anfang des Scripts die Variable von WGET auf CURL �ndern, dann wird statt WGET eben CURL im Script genutzt
- L�nder f�r die �bersicht k�nnen in der Datei corona.land editiert werden.

Das Plugin ist nun unter "Werkzeuge" auf der blauen Taste zu finden. �ber die Men�-Einstellungen kann man es auch an andere Stellen setzen, wie bei anderen Plugins auch.



Ampel
=========================

Aktueller Stand:

Ampel hängt an der Wand, Controller hat einfache Firmware.

Belegung des Steuerkabels:

schwarz – Masse
blau    – Blink-Attribut
gelb    – Rot
grün    – Grün

Die Steuerleitungen liegen per Pull-Up auf 5V und müssen zum Aktivieren nach Masse gezogen werden (Vorwiderstand nicht nötig).

Ansteuerung per Raspy: 
Derzeit GPIO 17 -> rot
        GPIO 18 -> grün

Rechnername wittgenstein

http://wittgenstein/{red, green, none}


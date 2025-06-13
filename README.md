Cilj ovog projekta je realizacije BLE perifernog uređaja za merenje temperature na bazi ESP32-S3 platforme. Uređaj treba da sadrži senzor temperature i dve LED i da implementira odgovarajući GATT servis, prema sledećim funkcionalnim zahtevima:
Prenos podataka o temperaturi: Podatak o temperaturi se prenosi BLE centrali na dva načina: putem čitanja (READ) i periodičnih notifikacija (NOTIFY).
Perioda notifikacija: Nakon uspostavljanja BLE konekcije, uređaj periodično šalje notifikacije o temperaturi na svakih 5 sekundi. Centrala treba da ima mogućnost podešavanja ove periode u opsegu od 1 do 255 sekundi, sa korakom 1 sekunda.
Pristup senzoru temperature: ESP32-S3 pristupa senzoru temperature samo kada centrala zahteva čitanje vrednosti temperature ili kada je potrebno poslati notifikaciju.
Indikatori statusa:
 Zelena LED signalizira status BLE konekcije (svetli dok je konekcija uspostavljena).
 Crvena LED signalizira razmenu podataka između BLE centrale i periferije: kratkotrajni treptaj (na primer, trajanja 50 𝑚𝑠) pri svakom slanju notifikacije ili pri pristiglom zahtevu za čitanje/upis neke karakteristike.
Za testiranje koristiti program nRF Connect.

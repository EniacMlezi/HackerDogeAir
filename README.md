# Secure Programming 
Verslaglegging van de secure programming maatregelen genomen gedurende de ontwikkeling van de HackerDoge webshop.

## 1.	Inleiding
Voor het vak Secure Programming dat wordt afgenomen gedurende het derde jaar van de opleiding technische informatica, gegeven aan Hogeschool InHolland, ontwikkelen de studenten een webportal gebasseerd op secure programming principes.
Deze rapportage heeft betrekking op de ontwikkeling van deze applicatie en de verscheidende maatregelen die zijn genomen om deze webapplicatie te beveiligen.
De maatregelen die zijn genomen zijn gebasseerd op de lessen gegeven door ITsec Security Services en het boek “24 Deadly sins of software security”.	

## 2.	Behaalde Doelstellingen	
### 2.1 Geïmplementeerde functionaliteiten	
Binnen dit hoofdstuk zal er per functionaliteit gekeken worden de geimplementeerde functinaliteiten.

#### 2.1.1 Inloggen
Het inloggen binnen de website kan via /login, hier wordt een email en wachtwoord gevraagt die gelijk moeten zijn aan een database resultaat. Hierna kan ingelogd worden.
	
#### 2.1.2 Accountinformatie bekijken	
Als gebruiker wil je natuurlijk je account informatie bekijken. Door naar /user/userdetails te gaan, kan informatie worden bekeken en zelfs worden aangepast.

#### 2.1.3 InhollandMiles (Doge Coin) toevoegen	
Een admin zou in staat moeten zijn om DogeCoins toe te voegen aan een gebruiker. Dit kan via /admin/userlist. Hier staat per user een link om voor deze user de valuta waarde aan te passen.

#### 2.1.4 Een vlucht kiezen op een bepaalde datum
Als iemand een vlucht wil boeken moet er eerst worden gekeken welke vluchten er bestaan. Door op /flight/flightsearch een datum in te vullen laat het systeem vluchten zien op deze specifieke datum. Per vlucht is er een link om de specifieke vlucht te boeken. Om een vlucht te kunnen boeken moet een gebruiker genoeg DogeCoins bezitten, dit wordt eerst gecheckt. Als de gebruiker genoeg DogeCoins bezit, wordt de vlucht voor deze gebruiker besteld.
	
#### 2.1.5 De vlucht betalen met InhollandMiles (Doge Coin)	
Alle vluchten die HackerDogeAir aan biedt zijn 50 DogeCoins per vlucht. Tijdens het boeken wordt er automatisch betaalt met DogeCoins, indien een gebruiker meer dan 50 DogeCoins in zijn bezit heeft natuurlijk.

#### 2.1.6 Een vlucht annuleren	
Een admin heeft de mogelijkheid een vlucht te annuleren. Bij HackerDogeAir wordt een geannuleerde vlucht dan ook verwijderd uit de database. Een gebruiker krijgt hiervoor zijn DogeCoins niet terug. Dit gebeurdt in /admin/flightlist, hier staan alle vluchten in een lijst. Per vlucht is er een link om de specifieke vlucht te verwijderen.

#### 2.1.7 Een vlucht toevoegen	
Deze functie bestaat binnen HackerDogeAir niet. Dit komt doordat de benodigde functionaliteiten die op BlackBoard staan niet overeen met de benodigde functionaliteiten die in de powerpoint staan gespecificeerd.

#### 2.1.8 Account aanmaken, registreren	
Om iets te kunnen doen binnen HackerDogeAir dient een gebruiker een account te hebben. Om een account aan te maken kan er naar /register worden gegaan. Hier is een formulier terug te vinden die volledig ingevuld dient te worden. Alleen dan kan er een account worden aangemaakt.

#### 2.1.9 Besttellingen inzien (user alleen de eigen vlucht)	
Een gebruiker wil natuurlijk zijn bestellingen kunnen inzien. Er kan hiervoor naar /user/bookings worden gegaan. Hier zijn alle bestellingen voor de gebruiker te zien. Een admin heeft de mogelijkheid om alle bestellingen van alle gebruikers te zien. Er kan hiervoor naar /admin/bookinglist worden gegaan.

#### 2.1.10 Optioneel: vergeten wachtwoord functie
HackerDogeAir ondersteund deze functionaliteit niet.
	
#### 2.1.11 Optioneel: vergeten gebruikersnaam functie	
HackerDogeAir ondersteund deze functionaliteit niet.

## 3. Genomen Beveiligingsmaatregelen	
### 3.1 Regex clientside validatie	
Om er voor te zorgen dat alle input van een gebruiker wordt gevalideerd, wordt er binnen HackerDogeAir met HTML5 validatie gewerkt. Hierbij wordt een <input> gebruikt. Hieraan wordt een pattern=”” meegegeven. Binnen deze pattern staat een regex die de waarde van de input valideert.

### 3.2 Serverside input validatie	
Aangezien je clientside validate kan uitzetten, wordt er ook nog eens gebruik gemaakt van serverside validatie. Hierbij wordt nogmaals de input door een regex string gehaald.

### 3.3 Pepared SQL query statements	
Door een database access layer te creëren waarbij alle SQL querys staan vastgesteld, worden SQL injections voorkomen.

### 3.4 XSS output statements	
Om ervoor te zorgen dat er binnen HackerDogeAir geen XSS kan plaatsvinden, wordt alle output geëncode.

### 3.5 Password hashing	
Om ervoor te zorgen dat er geen plain-tekst wachtwoorden binnen de database worden opgeslagen, worden alle wachtwoorden met behulp van libscript gehashed.

### 3.6 Bruteforce protective	
Wanneer een gebruiker vijf maal op rij met een verkeerd wachtwoord probeert in te loggen, wordt het account voor vijf minuten onbruikbaar gemaakt. Er kan dus voor vijf minuten niet mee worden ingelogt. Dit vootkomt bruteforce attacks.

### 3.7 Applicatie access log	
Het IP-adres van elke gebruiker wordt opgeslagen om bij te houden wie er op de website is geweest en waar.

### 3.8 Syslog applicatie logging	
De Kore IO loggin functionaliteiten maken in hun onderliggende logica gebruik van de Linux syslog functionaliteiten.

### 3.9 Gebruikte compiler protective flags	
De gebruikte compiler maakt gebruik van binary hardening en stack canaries om buffer overflow exploitatie te bemoeilijken.

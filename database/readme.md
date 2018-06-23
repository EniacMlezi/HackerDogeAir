# Creëren van de HackerDoge database
Om de de HackerDoge database te kunnen bouwen en creëren moeten een aantal stappen genomen worden.

## Stap 1: 
Creër de DogaAir database:
### Spawn de pgsql shell:
	$ sudo -u posgress pgsql
### Creër de hackerdoge user:
	$ sudo -u posgress createuser hackerdoge
### Creër de DogeAir database:
	$ sudo -u postgress createdb DogeAir
### Geef de hackerdoge user een passwoord en authorisatie:
	$ sudo -u postgress psql
	psql=# alter user hackerdoge with encyrpted password 'doge';
	psql=# grant all privileges on database DogeAir to hackerdoge;

## Stap 2:
Configureer de DogeAir database:
### Open de postgresql configuratie file:
	$ sudo vim /etc/postgresql/X.X/main/pg_hba.conf
### Voeg de volgende lijn toe:
	host all all 0.0.0.0/0 md5
### Open de posgresql configuratie file:
	$ sudo vim /etc/postgresql/X.X/main/postgresql.conf
### Uncomment de listen_addresses setting en set de waarde op *:
	listen_addresses='*'
### Restart de postgres service:
	$ /etc/init.d/postgresql restart

## Stap 3:
Voeg de queries toe aan de DogeAir database:
### Connect op de database:
	$ sudo -u postgres psql DogeAir
### Voer het bestand DogeAirDump.sql uit op de database:
	psql=# \i DogeAirDump.sql

## Sources:
Stap 1: <https://medium.com/coding-blocks/creating-user-database-and-adding-access-on-postgresql-8bfcd2f4a91e>

Stap 2: <https://devstudioonline.com/article/configure-postgresql-in-ubuntu-and-connect-with-datagrip>

Stap 3: <https://www.techportal.co.za/databases/52-pgsql/246-how-to-execute-sql-commands-from-a-file-in-postgresql.html>

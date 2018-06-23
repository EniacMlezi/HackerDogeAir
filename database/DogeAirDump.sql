--

-- PostgreSQL database dump

--



-- Dumped from database version 10.4 (Ubuntu 10.4-0ubuntu0.18.04)

-- Dumped by pg_dump version 10.4 (Ubuntu 10.4-0ubuntu0.18.04)



SET statement_timeout = 0;

SET lock_timeout = 0;

SET idle_in_transaction_session_timeout = 0;

SET client_encoding = 'UTF8';

SET standard_conforming_strings = on;

SELECT pg_catalog.set_config('search_path', '', false);

SET check_function_bodies = false;

SET client_min_messages = warning;

SET row_security = off;



DROP DATABASE IF EXISTS "DogeAir";

--

-- Name: DogeAir; Type: DATABASE; Schema: -; Owner: postgres

--



CREATE DATABASE "DogeAir" WITH TEMPLATE = template0 ENCODING = 'UTF8' LC_COLLATE = 'en_US.UTF-8' LC_CTYPE = 'en_US.UTF-8';





ALTER DATABASE "DogeAir" OWNER TO postgres;



\connect "DogeAir"



SET statement_timeout = 0;

SET lock_timeout = 0;

SET idle_in_transaction_session_timeout = 0;

SET client_encoding = 'UTF8';

SET standard_conforming_strings = on;

SELECT pg_catalog.set_config('search_path', '', false);

SET check_function_bodies = false;

SET client_min_messages = warning;

SET row_security = off;



--

-- Name: plpgsql; Type: EXTENSION; Schema: -; Owner: 

--



CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;





--

-- Name: EXTENSION plpgsql; Type: COMMENT; Schema: -; Owner: 

--



COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';





--

-- Name: bookflight(integer, integer); Type: FUNCTION; Schema: public; Owner: hackerdoge

--



CREATE FUNCTION public.bookflight(paramflightidentifier integer, paramuseridentifier integer) RETURNS integer

    LANGUAGE plpgsql

    AS $$

DECLARE

  flight_seatsavailable INTEGER;

  flight_cost           INTEGER;

  user_funds            INTEGER;

BEGIN

  SELECT

    "Flight".seatsavailable,

    "Flight".cost

  INTO flight_seatsavailable, flight_cost

  FROM "Flight"

  WHERE flightidentifier = paramflightidentifier;

  IF (flight_seatsavailable <= 0)

  THEN

    RETURN 1;

  END IF;



  SELECT "User".dogecoin

  INTO user_funds

  FROM "User"

  WHERE useridentifier = paramuseridentifier;

  IF (user_funds < flight_cost)

  THEN

    RETURN 2;

  END IF;



  UPDATE "Flight"

  SET seatsavailable = seatsavailable - 1

  WHERE flightidentifier = paramflightidentifier;

  UPDATE "User"

  SET dogecoin = dogecoin - flight_cost

  WHERE useridentifier = paramuseridentifier;

  INSERT INTO "Ticket" (flightidentifier, useridentifier, cost)

  VALUES (paramflightidentifier, paramuseridentifier, flight_cost);



  RETURN 0;

END

$$;





ALTER FUNCTION public.bookflight(paramflightidentifier integer, paramuseridentifier integer) OWNER TO hackerdoge;



SET default_tablespace = '';



SET default_with_oids = false;



--

-- Name: Airport; Type: TABLE; Schema: public; Owner: hackerdoge

--



CREATE TABLE public."Airport" (

    airportidentifier integer NOT NULL,

    name character varying(255) NOT NULL,

    location character varying(255) NOT NULL

);





ALTER TABLE public."Airport" OWNER TO hackerdoge;



--

-- Name: Flight; Type: TABLE; Schema: public; Owner: hackerdoge

--



CREATE TABLE public."Flight" (

    flightidentifier integer NOT NULL,

    departureairportidentifier integer NOT NULL,

    arrivalairportidentifier integer NOT NULL,

    departuredatetime timestamp without time zone NOT NULL,

    arrivaldatetime timestamp without time zone NOT NULL,

    distance double precision NOT NULL,

    seatsavailable integer NOT NULL,

    cost integer NOT NULL,

    CONSTRAINT departure_arrival CHECK ((departureairportidentifier <> arrivalairportidentifier)),

    CONSTRAINT positive_distance CHECK ((distance > (0)::double precision)),

    CONSTRAINT positive_seats CHECK ((seatsavailable >= 0)),

    CONSTRAINT positive_time CHECK ((arrivaldatetime > departuredatetime))

);





ALTER TABLE public."Flight" OWNER TO hackerdoge;



--

-- Name: LoginAttempt; Type: TABLE; Schema: public; Owner: hackerdoge

--



CREATE TABLE public."LoginAttempt" (

    useridentifier integer,

    datetime timestamp without time zone NOT NULL,

    success boolean NOT NULL

);





ALTER TABLE public."LoginAttempt" OWNER TO hackerdoge;



--

-- Name: Session; Type: TABLE; Schema: public; Owner: hackerdoge

--



CREATE TABLE public."Session" (

    sessionidentifier character varying NOT NULL,

    useridentifier integer NOT NULL,

    validuntil timestamp without time zone NOT NULL

);





ALTER TABLE public."Session" OWNER TO hackerdoge;



--

-- Name: Ticket; Type: TABLE; Schema: public; Owner: hackerdoge

--



CREATE TABLE public."Ticket" (

    ticketidentifier integer NOT NULL,

    flightidentifier integer NOT NULL,

    useridentifier integer NOT NULL,

    cost integer NOT NULL,

    CONSTRAINT positive_cost CHECK (((cost)::double precision > (0)::double precision))

);





ALTER TABLE public."Ticket" OWNER TO hackerdoge;



--

-- Name: user_useridentifier_seq; Type: SEQUENCE; Schema: public; Owner: postgres

--



CREATE SEQUENCE public.user_useridentifier_seq

    START WITH 1

    INCREMENT BY 1

    NO MINVALUE

    NO MAXVALUE

    CACHE 1;





ALTER TABLE public.user_useridentifier_seq OWNER TO postgres;



--

-- Name: User; Type: TABLE; Schema: public; Owner: hackerdoge

--



CREATE TABLE public."User" (

    useridentifier integer DEFAULT nextval('public.user_useridentifier_seq'::regclass) NOT NULL,

    username character varying(255) NOT NULL,

    emailaddress character varying(255) NOT NULL,

    firstname character varying(255) NOT NULL,

    lastname character varying(255) NOT NULL,

    telephonenumber character varying(255) NOT NULL,

    password character varying(255) NOT NULL,

    dogecoin integer NOT NULL,

    registrationtime character varying(255) NOT NULL,

    userrole integer NOT NULL,

    CONSTRAINT positive_coin CHECK ((dogecoin >= 0))

);





ALTER TABLE public."User" OWNER TO hackerdoge;



--

-- Name: ticket_ticketidentifier_seq; Type: SEQUENCE; Schema: public; Owner: hackerdoge

--



CREATE SEQUENCE public.ticket_ticketidentifier_seq

    START WITH 1

    INCREMENT BY 1

    NO MINVALUE

    NO MAXVALUE

    CACHE 1;





ALTER TABLE public.ticket_ticketidentifier_seq OWNER TO hackerdoge;



--

-- Name: ticket_ticketidentifier_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: hackerdoge

--



ALTER SEQUENCE public.ticket_ticketidentifier_seq OWNED BY public."Ticket".ticketidentifier;





--

-- Name: Ticket ticketidentifier; Type: DEFAULT; Schema: public; Owner: hackerdoge

--



ALTER TABLE ONLY public."Ticket" ALTER COLUMN ticketidentifier SET DEFAULT nextval('public.ticket_ticketidentifier_seq'::regclass);





--

-- Name: Airport Airport_name_key; Type: CONSTRAINT; Schema: public; Owner: hackerdoge

--



ALTER TABLE ONLY public."Airport"

    ADD CONSTRAINT "Airport_name_key" UNIQUE (name);





--

-- Name: Airport Airport_pkey; Type: CONSTRAINT; Schema: public; Owner: hackerdoge

--



ALTER TABLE ONLY public."Airport"

    ADD CONSTRAINT "Airport_pkey" PRIMARY KEY (airportidentifier);





--

-- Name: Flight Flight_pkey; Type: CONSTRAINT; Schema: public; Owner: hackerdoge

--



ALTER TABLE ONLY public."Flight"

    ADD CONSTRAINT "Flight_pkey" PRIMARY KEY (flightidentifier);





--

-- Name: Session Session_pkey; Type: CONSTRAINT; Schema: public; Owner: hackerdoge

--



ALTER TABLE ONLY public."Session"

    ADD CONSTRAINT "Session_pkey" PRIMARY KEY (sessionidentifier);





--

-- Name: Session Session_useridentifier_key; Type: CONSTRAINT; Schema: public; Owner: hackerdoge

--



ALTER TABLE ONLY public."Session"

    ADD CONSTRAINT "Session_useridentifier_key" UNIQUE (useridentifier);





--

-- Name: Ticket Ticket_pkey; Type: CONSTRAINT; Schema: public; Owner: hackerdoge

--



ALTER TABLE ONLY public."Ticket"

    ADD CONSTRAINT "Ticket_pkey" PRIMARY KEY (ticketidentifier);





--

-- Name: User User_emailaddress_key; Type: CONSTRAINT; Schema: public; Owner: hackerdoge

--



ALTER TABLE ONLY public."User"

    ADD CONSTRAINT "User_emailaddress_key" UNIQUE (emailaddress);





--

-- Name: User User_pkey; Type: CONSTRAINT; Schema: public; Owner: hackerdoge

--



ALTER TABLE ONLY public."User"

    ADD CONSTRAINT "User_pkey" PRIMARY KEY (useridentifier);





--

-- Name: User User_username_key; Type: CONSTRAINT; Schema: public; Owner: hackerdoge

--



ALTER TABLE ONLY public."User"

    ADD CONSTRAINT "User_username_key" UNIQUE (username);





--

-- Name: Flight Flight_arrivalairportidentifier_fkey; Type: FK CONSTRAINT; Schema: public; Owner: hackerdoge

--



ALTER TABLE ONLY public."Flight"

    ADD CONSTRAINT "Flight_arrivalairportidentifier_fkey" FOREIGN KEY (arrivalairportidentifier) REFERENCES public."Airport"(airportidentifier) ON UPDATE CASCADE ON DELETE CASCADE;





--

-- Name: Flight Flight_departureairportidentifier_fkey; Type: FK CONSTRAINT; Schema: public; Owner: hackerdoge

--



ALTER TABLE ONLY public."Flight"

    ADD CONSTRAINT "Flight_departureairportidentifier_fkey" FOREIGN KEY (departureairportidentifier) REFERENCES public."Airport"(airportidentifier) ON UPDATE CASCADE ON DELETE CASCADE;





--

-- Name: LoginAttempt LoginAttempt_useridentifier_fkey; Type: FK CONSTRAINT; Schema: public; Owner: hackerdoge

--



ALTER TABLE ONLY public."LoginAttempt"

    ADD CONSTRAINT "LoginAttempt_useridentifier_fkey" FOREIGN KEY (useridentifier) REFERENCES public."User"(useridentifier);





--

-- Name: Session Session_useridentifier_fkey; Type: FK CONSTRAINT; Schema: public; Owner: hackerdoge

--



ALTER TABLE ONLY public."Session"

    ADD CONSTRAINT "Session_useridentifier_fkey" FOREIGN KEY (useridentifier) REFERENCES public."User"(useridentifier);





--

-- Name: Ticket Ticket_flightidentifer_fkey; Type: FK CONSTRAINT; Schema: public; Owner: hackerdoge

--



ALTER TABLE ONLY public."Ticket"

    ADD CONSTRAINT "Ticket_flightidentifer_fkey" FOREIGN KEY (flightidentifier) REFERENCES public."Flight"(flightidentifier) ON UPDATE CASCADE ON DELETE CASCADE;





--

-- Name: Ticket Ticket_useridentifier_fkey; Type: FK CONSTRAINT; Schema: public; Owner: hackerdoge

--



ALTER TABLE ONLY public."Ticket"

    ADD CONSTRAINT "Ticket_useridentifier_fkey" FOREIGN KEY (useridentifier) REFERENCES public."User"(useridentifier);





--

-- Name: SCHEMA public; Type: ACL; Schema: -; Owner: postgres

--



GRANT ALL ON SCHEMA public TO PUBLIC;





--

-- Name: SEQUENCE user_useridentifier_seq; Type: ACL; Schema: public; Owner: postgres

--



GRANT SELECT,USAGE ON SEQUENCE public.user_useridentifier_seq TO hackerdoge;





--

-- PostgreSQL database dump complete

--


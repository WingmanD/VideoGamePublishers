--
-- PostgreSQL database dump
--

-- Dumped from database version 15.0
-- Dumped by pg_dump version 15.0

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: directors; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.directors (
    id integer NOT NULL,
    name character varying NOT NULL,
    surname character varying NOT NULL
);


ALTER TABLE public.directors OWNER TO postgres;

--
-- Name: directors_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.directors ALTER COLUMN id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.directors_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- Name: games; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.games (
    id integer NOT NULL,
    name character varying NOT NULL,
    "releaseDate" date,
    genre character varying,
    "idStudio" integer NOT NULL
);


ALTER TABLE public.games OWNER TO postgres;

--
-- Name: games_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.games ALTER COLUMN id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.games_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- Name: publishers; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.publishers (
    id integer NOT NULL,
    name character varying NOT NULL,
    "dateFounded" date,
    country character varying,
    description character varying,
    website character varying,
    "idDirector" integer NOT NULL
);


ALTER TABLE public.publishers OWNER TO postgres;

--
-- Name: publishers_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.publishers ALTER COLUMN id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.publishers_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- Name: studios; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.studios (
    id integer NOT NULL,
    name character varying NOT NULL,
    country character varying,
    "dateFounded" date,
    "numDevelopers" integer,
    "idPublisher" integer NOT NULL,
    "idDirector" integer
);


ALTER TABLE public.studios OWNER TO postgres;

--
-- Name: studios_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.studios ALTER COLUMN id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.studios_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- Data for Name: directors; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.directors (id, name, surname) FROM stdin;
1	Lars	Wingefors
2	Jim	Ryan
3	Neil	Ralley
4	Phil	Spencer
5	Shuntaro	Furukawa
6	Mark	Ren
7	Debbie	Bestwick
8	Douglas	Morin
9	Megan	Ellison
10	Alan	Patmore
11	Yves	Guillemot
12	Owen	Mahoney
13	Yosuke	Matsuda
14	Satoshi	Oshita
15	Strauss	Zelnick
16	Andrew	Wilson
35	Stefan	Hanna
36	S├©ren	Lundgaard
37	Angie	Smets
38	Yumi	Yang
39	Pierre	Hintze
40	Feargus	Urquhart
41	Minoru	Arakawa
42	Ethan	Yu
43	Roman	Ribari─ç
44	Swann	Martin-Raget
45	Roland	Primat
46	Thomas	Andr├®n
47	Christophe	Derennes
48	Bryant	Freitag
50	Daisuke	Uchiyama
51	Andrew	Semple
52	Phil	Hutcheon
53	David	Anfossi
\.


--
-- Data for Name: games; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.games (id, name, "releaseDate", genre, "idStudio") FROM stdin;
1	Satisfactory	2019-03-19	Factory Building Sim	8
2	Deep Rock Galactic	2018-02-28	CO-OP PvE FPS	9
3	Horizon Zero Dawn	2017-02-28	Open World RPG	10
4	God of War	2018-04-20	RPG	11
5	Control	2019-08-27	Paranormal RPG	12
6	Halo 5	2015-10-27	FPS	13
7	The Outer Worlds	2019-10-25	Open World RPG	14
8	Super Mario Odyssey	2017-10-27	Platformer	15
9	Ring of Elysium	2018-09-19	Battle Royale	16
10	Worms Rumble	2020-12-01	Arena shooter	17
11	Serious Sam: Siberian Mayhem	2022-01-25	FPS	18
12	Stray	2022-07-19	Platformer	19
13	Project Wingman	2020-12-01	Arcade Aircraft Combat	20
14	Tom Clancys The Division 2	2019-03-12	Open World RPG, Looter Shooter	21
15	Tom Clancys Rainbow Six Siege	2015-12-01	Tactical Shooter	22
16	DomiNations	2015-04-01	Mobile Strategy	23
17	Deus Ex: Mankind Divided	2016-08-23	Stealth FPS	24
18	Ace Combat 7: Skies Unknown	2019-01-18	Arcade Aircraft Combat	25
19	Grand Theft Auto V	2013-09-17	Open World RPG	26
20	Battlefield 2042	2021-11-12	FPS	27
21	Horizon Forbidden West	2022-02-18	Open World RPG	10
\.


--
-- Data for Name: publishers; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.publishers (id, name, "dateFounded", country, description, website, "idDirector") FROM stdin;
2	505 Games	2006-01-01	Italy	Medium sized publisher based in Milan.	https://505games.com/	3
3	Xbox Game Studios	2006-01-01	United States	Part of Microsoft Gaming division.	https://www.xbox.com/en-US/xbox-game-studios	4
4	Nintendo	1889-09-23	Japan	Game developer, publisher and hardware developer.	https://www.nintendo.com/	5
5	Tencent Games	2003-01-01	China	Large publisher focusing on mobile game market.	https://game.qq.com/	6
6	Team17	1990-12-07	United Kingdom	Game developer and publisher created by merger of 17-Bit Software and Team 7.	https://www.team17.com/	7
7	Devolver Digital	2009-06-25	United States	Indie game publisher.	https://www.devolverdigital.com/	8
8	Annapurna Interactive	2016-12-01	United States	Indie game publisher.	https://annapurnainteractive.com/	9
9	Humble Games	2010-10-24	United States	Digital storefront and publisher of indie games.	https://www.humblebundle.com/	10
10	Ubisoft	1986-03-28	France	Large publisher with many video game franchises released, such as Assassins Creed, Far Cry, Tom Clancys and Splinter Cell.	https://www.ubisoft.com/en-gb/	11
11	Nexon	1994-12-26	South Korea	Video game publisher that specializes in online virtual world games for PCs, consoles and mobile.	https://www.nexon.com/main/en	12
12	Square Enix	2003-04-01	Japan	Multinational holding company.	https://www.square-enix.com/	13
13	Bandai Namco Entertainment	2006-03-31	Japan	Multinational game publisher.	https://en.bandainamcoent.eu/	14
14	Take-Two Interactive	1993-09-30	United States	Large holding company that owns two large publishers, Rockstar Games and 2K.	https://www.take2games.com/	15
15	Electronic Arts	1982-05-27	United States	Large publisher with many famous franchises including Battlefield, Need for Speed, The Sims, Mass Effect and Command & Conquer	https://www.ea.com/en-gb	16
1	Embracer Group	2011-06-01	Sweden	Formerly known as Nordic Games Licensing AB and THQ Nordic AB.	https://embracer.com/	1
16	Sony Interactive Entertainment	1993-11-16	Japan	Handles research, development, production and sales of hardware and software for PlayStation consoles.	https://www.sie.com/en/index.html	2
\.


--
-- Data for Name: studios; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.studios (id, name, country, "dateFounded", "numDevelopers", "idPublisher", "idDirector") FROM stdin;
8	Coffee Stain Studios	Sweden	2010-01-01	123	1	35
9	Ghost Ship Games	Denmark	2016-01-01	32	1	36
10	Guerrilla Games	Netherlands	2010-01-01	360	16	37
11	Santa Monica Studio	United States	1999-01-01	250	16	38
12	505 Games	Italy	2006-01-01	66	2	3
13	343 Industries	United States	2007-01-01	450	3	39
14	Obsidian Entertainment	United States	2003-06-12	200	3	40
15	Nintendo Software Technology	United States	1998-01-01	50	4	41
16	Aurora Studio Group	China	2007-01-01	100	5	42
17	Team17	United Kingdom	1990-12-07	200	6	7
18	Croteam	Croatia	2010-01-01	38	7	43
19	BlueTwelve Studio	France	2016-01-01	28	8	44
20	Sector D2	United States	2010-01-01	10	9	45
21	Massive Entertainment	Sweden	1997-01-01	650	10	46
22	Ubisoft Montreal	Canada	1997-04-25	4000	10	47
23	Big Huge Games	United States	2000-02-01	109	11	48
25	Bandai Namco Studios	Japan	2012-04-02	1115	13	50
26	Rockstar North	United Kingdom	1987-01-01	650	14	51
27	DICE	Sweden	1992-05-01	714	15	52
24	Eidos-Montr├®al	Canada	2007-11-26	481	12	53
\.


--
-- Name: directors_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.directors_id_seq', 53, true);


--
-- Name: games_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.games_id_seq', 21, true);


--
-- Name: publishers_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.publishers_id_seq', 16, true);


--
-- Name: studios_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.studios_id_seq', 27, true);


--
-- Name: directors directors_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.directors
    ADD CONSTRAINT directors_pkey PRIMARY KEY (id);


--
-- Name: games games_id_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.games
    ADD CONSTRAINT games_id_key UNIQUE (id);


--
-- Name: games games_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.games
    ADD CONSTRAINT games_pkey PRIMARY KEY (id);


--
-- Name: publishers publishers_id_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.publishers
    ADD CONSTRAINT publishers_id_key UNIQUE (id);


--
-- Name: publishers publishers_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.publishers
    ADD CONSTRAINT publishers_pkey PRIMARY KEY (id);


--
-- Name: studios studios_id_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.studios
    ADD CONSTRAINT studios_id_key UNIQUE (id);


--
-- Name: studios studios_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.studios
    ADD CONSTRAINT studios_pkey PRIMARY KEY (id);


--
-- Name: games games_idStudio_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.games
    ADD CONSTRAINT "games_idStudio_fkey" FOREIGN KEY ("idStudio") REFERENCES public.studios(id) NOT VALID;


--
-- Name: publishers publishers_idDirector_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.publishers
    ADD CONSTRAINT "publishers_idDirector_fkey" FOREIGN KEY ("idDirector") REFERENCES public.directors(id) NOT VALID;


--
-- Name: studios studios_idDirector_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.studios
    ADD CONSTRAINT "studios_idDirector_fkey" FOREIGN KEY ("idDirector") REFERENCES public.directors(id) NOT VALID;


--
-- Name: studios studios_idPublisher_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.studios
    ADD CONSTRAINT "studios_idPublisher_fkey" FOREIGN KEY ("idPublisher") REFERENCES public.publishers(id) NOT VALID;


--
-- PostgreSQL database dump complete
--


-- Reset existing tables
DROP TABLE IF EXISTS users;
DROP TABLE IF EXISTS items;
DROP TABLE IF EXISTS loans;
DROP TABLE IF EXISTS holds;
DROP TABLE IF EXISTS policy;

-- Users
CREATE TABLE users (
    id TEXT PRIMARY KEY,
    username TEXT UNIQUE NOT NULL,
    role TEXT NOT NULL
);

-- Items (Catalogue)
CREATE TABLE items (
    id TEXT PRIMARY KEY,
    title TEXT NOT NULL,
    authorOrCreator TEXT,
    format TEXT NOT NULL,
    status TEXT NOT NULL,
    publicationYear INTEGER,
    isbn TEXT,
    deweyDecimal TEXT,
    genre TEXT,
    rating TEXT,
    issueNumber TEXT,
    publicationDate TEXT
);

-- Loans (active borrowing records)
CREATE TABLE loans (
    id TEXT PRIMARY KEY,
    patronId TEXT NOT NULL,
    itemId TEXT NOT NULL,
    checkoutDate TEXT NOT NULL,
    dueDate TEXT NOT NULL,
    FOREIGN KEY(patronId) REFERENCES users(id),
    FOREIGN KEY(itemId) REFERENCES items(id)
);

-- Holds (FIFO queue records)
CREATE TABLE holds (
    id TEXT PRIMARY KEY,
    patronId TEXT NOT NULL,
    itemId TEXT NOT NULL,
    queuePosition INTEGER NOT NULL,
    FOREIGN KEY(patronId) REFERENCES users(id),
    FOREIGN KEY(itemId) REFERENCES items(id)
);

-- Policy (system-wide settings)
CREATE TABLE policy (
    maxActiveLoansPerPatron INTEGER,
    loanPeriodDays INTEGER
);

-- Seed Users
INSERT INTO users VALUES ('U001','ahmed','Patron');
INSERT INTO users VALUES ('U002','sarah','Patron');
INSERT INTO users VALUES ('U003','michael','Patron');
INSERT INTO users VALUES ('U004','abdulrahman','Patron');
INSERT INTO users VALUES ('U005','aisha','Patron');
INSERT INTO users VALUES ('U006','lib','Librarian');
INSERT INTO users VALUES ('U007','admin','SysAdmin');

-- Seed Items (20 total)
-- Fiction
INSERT INTO items VALUES ('I001','Crime and Punishment','Fyodor Dostoevsky','Book','Available',1866,NULL,NULL,'Classic',NULL,NULL,NULL);
INSERT INTO items VALUES ('I002','White Nights','Fyodor Dostoevsky','Book','Available',1848,NULL,NULL,'Classic',NULL,NULL,NULL);
INSERT INTO items VALUES ('I003','The Hobbit','J. R. R. Tolkien','Book','Available',1937,NULL,NULL,'Fantasy',NULL,NULL,NULL);
INSERT INTO items VALUES ('I004','Pride and Prejudice','Jane Austen','Book','Available',1813,NULL,NULL,'Romance',NULL,NULL,NULL);
INSERT INTO items VALUES ('I005','To Kill a Mockingbird','Harper Lee','Book','Available',1960,NULL,NULL,'Classic',NULL,NULL,NULL);

-- Non-fiction
INSERT INTO items VALUES ('I006','Sapiens','Yuval Noah Harari','Book','Available',2011,NULL,'001.94',NULL,NULL,NULL,NULL);
INSERT INTO items VALUES ('I007','A Brief History of Time','Stephen Hawking','Book','Available',1988,NULL,'523.10',NULL,NULL,NULL,NULL);
INSERT INTO items VALUES ('I008','The Selfish Gene','Richard Dawkins','Book','Available',1976,NULL,'576.50',NULL,NULL,NULL,NULL);
INSERT INTO items VALUES ('I009','Guns, Germs, and Steel','Jared Diamond','Book','Available',1997,NULL,'303.48',NULL,NULL,NULL,NULL);
INSERT INTO items VALUES ('I010','Thinking, Fast and Slow','Daniel Kahneman','Book','Available',2011,NULL,'153.42',NULL,NULL,NULL,NULL);

-- Magazines
INSERT INTO items VALUES ('I011','The worm Runner''s Digest','Dr. James V. McConnell','Magazine','Available',1967,NULL,NULL,NULL,NULL,'1967-01','1967-01-15');
INSERT INTO items VALUES ('I012','Weekly World News','Spy Cat LLC','Magazine','Available',2025,NULL,NULL,NULL,NULL,'2025-02-07','2025-02-07');
INSERT INTO items VALUES ('I013','Meatpaper','Amy Standen & Sasha Wizansky','Magazine','Available',2025,NULL,NULL,NULL,NULL,'2025-03','2025-03-01');

-- Movies
INSERT INTO items VALUES ('I014','Her','Spike Jonze','Movie','Available',2013,NULL,NULL,'Drama/Romance','R',NULL,NULL);
INSERT INTO items VALUES ('I015','Dogville','Lars von Trier','Movie','Available',2003,NULL,NULL,'Drama','R',NULL,NULL);
INSERT INTO items VALUES ('I016','Arrival','Denis Villeneuve','Movie','Available',2016,NULL,NULL,'Sci-Fi','PG-13',NULL,NULL);

-- Video Games
INSERT INTO items VALUES ('I017','Detroit: Become Human','Quantic Dream','VideoGame','Available',2018,NULL,NULL,'Adventure','M',NULL,NULL);
INSERT INTO items VALUES ('I018','Angry Birds','Rovio','VideoGame','Available',2009,NULL,NULL,'Puzzle','E',NULL,NULL);
INSERT INTO items VALUES ('I019','Plants vs. Zombies','PopCap Games','VideoGame','Available',2009,NULL,NULL,'Tower Defense','E10+',NULL,NULL);
INSERT INTO items VALUES ('I020','Head Soccer','D&D Dream Corp','VideoGame','Available',2017,NULL,NULL,'Sports','E10+',NULL,NULL);

-- Policy
INSERT INTO policy VALUES (3,14);

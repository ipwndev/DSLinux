SET PASSWORD FOR root@localhost=PASSWORD('new_password');
SET PASSWORD FOR root@localhost=PASSWORD('root');
UPDATE user SET Password=PASSWORD('root')
WHERE user='root';
FLUSH PRIVILEGES;
UPDATE user SET Password=PASSWORD('pass') WHERE user='root';
FLUSH PRIVILEGES;
 REPLACE INTO user VALUES ('localhost','root','root','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y');
FLUSH PRIVILEGES;
FLUSH PRIVILEGES;
INSERT INTO user VALUES ('localhost','root','PASSWORD('root')','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y');
INSERT INTO user VALUES ('localhost','root',PASSWORD('root'),'Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y');
REPLACE INTO user VALUES ('localhost','root',PASSWORD('root'),'Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y');
show INTO user values;
show INTO user VALUES;
INSERT INTO user VALUES ('localhost','root1',PASSWORD('root'),'Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y');
FLUSH PRIVILEGES;
INSERT INTO user VALUES('localhost','root1',PASSWORD('root'),
'Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y');
INSERT INTO user VALUES('localhost','oluh',PASSWORD('vecrek'),
'Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y');
GRANT ALL PRIVILEGES ON *.* TO olu@localhost
IDENTIFIED BY 'root' WITH GRANT OPTION;
GRANT ALL PRIVILEGES ON *.* TO olu@192.168.1.1
IDENTIFIED BY 'root' WITH GRANT OPTION;
GRANT ALL PRIVILEGES ON *.* TO olu@"%"
IDENTIFIED BY 'root' WITH GRANT OPTION;
FLUSH PRIVILEGES;
GRANT ALL PRIVILEGES ON *.* TO wan@"%"
IDENTIFIED BY 'root' WITH GRANT OPTION;
GRANT ALL PRIVILEGES ON *.* TO wan@localhost
IDENTIFIED BY 'root' WITH GRANT OPTION;
print tables in mysql;
print tables in wan;
sho tables in wan;
show tables in wan;
show tables;
select * from sessions;
select * from users;
select * from pays;
select * from templates;
select * from template;
select * from template where ip between '192.168.2.2' and '192.168.2.3';
select * from template where ip between '192.168.2.0' and '192.168.2.5';
select * from template where ip between '192.168.2.0' and '192.168.2.20';
select * from template where ip between '192.168.2.0' and '192.168.2.21';
select * from template where ip between '192.168.2.0' and '192.168.2.22';
select * from template where ip between '192.168.2.0' and '192.168.2.25';
select * from template where ip between '192.168.2.0' and '192.168.2.50';
select * from template where ip between '192.168.2.0' and '192.168.2.35';
select * from template where ip between '192.168.2.0' and '192.168.2.25';
select * from users where date_format(date,'%Y-%m-%d') between '2002-02-01' and '2002-12-01';
select * from users where date_format(date,'%Y-%m-%d') between '2002-02-01' and '2002-12-01';
load data infile '/home/auth/log/table_counter' into table sessions fields terminated by '|';
select unix_timestamp(now());

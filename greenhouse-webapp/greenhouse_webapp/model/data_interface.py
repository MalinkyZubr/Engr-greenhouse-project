import mysql.connector
from mysql.connector.cursor import MySQLCursorPrepared

# configure db to only connect via localhost so that no password needed
class DatabaseInterface:
    def __init__(self, user, password, host, database):
        self.connector = mysql.connector.connect(
            user="greenhouse_api",
            host="127.0.0.1",
            database="greenhouse_db"
        )
        self.cursor = self.connector.cursor(cursor_class=MySQLCursorPrepared)
        
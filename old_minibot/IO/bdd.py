from time import sleep, time
import sqlite3
import datetime

conn = sqlite3.connect('../back/database.sqlite')

ref = time()

while True:
    with open('data_robot.txt', 'r') as f:
        cur = conn.cursor()

        b = time()
        line = f.readline().split(' ')
        print(line)
        da = str(datetime.datetime.now())
        d = da.split(' ')[0] + 'T' + da.split(' ')[1] + 'Z'
        cur.execute('INSERT INTO datas ("speedL", "speedR", "time", "name", "createdAt", "updatedAt") VALUES (' + str(line[1]) + ', ' + str(line[0]) + ' , ' + str(b - ref) +', "Test", "' + da + '", "' + da + '")')
        conn.commit()
        sleep(0.05)

    f.close()

cur.close()
conn.close()
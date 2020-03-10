import matplotlib.pyplot as plt
import numpy as np
from time import sleep, time

data_consigne = np.zeros((80, 1))
data_speed = np.zeros((80, 1))

fig = plt.figure()
ax = fig.add_subplot(111)
Ln1, = ax.plot(data_speed)
Ln2, = ax.plot(data_consigne, '-r')
ax.set_ylim([-50, 50])
plt.ion()
plt.show()

while True:
    try:
        with open('data_robot.txt', "r+") as f:
            f.seek(0)
            data = f.readline().split(" ")
            data_speed[:-1] = data_speed[1:]
            data_speed[-1] = float(data[0])

            data_consigne[:-1] = data_consigne[1:]
            data_consigne[-1] = float(data[1])
            Ln1.set_ydata(data_speed)
            Ln1.set_xdata(range(len(data_speed)))

            Ln2.set_ydata(data_consigne)
            Ln2.set_xdata(range(len(data_consigne)))
            plt.pause(0.01)
        f.close()
    except Exception:
        pass


#Justin Paladino
# A quick little script that can automate the proteus through the range of the servo

import proteus
import time

p = proteus.Proteus("Com3")

p.start()

for i in range(-160,430):
    temp = i / 1000.0
    p.move(0,temp)
    print temp
    p.readOdom()
    time.sleep(0.8)
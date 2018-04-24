import RPi.GPIO as gpio
import time
channel = 17#BCM编号方式的17对应树莓派的pin11
data=[]
j=0
gpio.setmode(gpio.BCM)#I/O口使用BCM编号方式
time.sleep(1)

gpio.setup(channel,gpio.OUT)#设置数据线为输出
gpio.output(channel,gpio.LOW)
time.sleep(0.02)
gpio.output(channel,gpio.HIGH)


gpio.setup(17,gpio.IN)#设置数据线为输入

while gpio.input(17)==gpio.LOW:
    continue

while gpio.input(17)==gpio.HIGH:
    continue

while j<40:
    k=0
    while gpio.input(17)==gpio.LOW:
        continue

    while gpio.input(17)==gpio.HIGH:
        k+=1
        if k>100:
            break

    if k<8:
        data.append(0)

    else:
        data.append(1)
    j+=1

print "Sensor is working"
print data

#读取数值
humidity_bit = data[0:8]
humidity_point_bit = data[8:16]
temperature_bit = data[16:24]
temperature_point_bit=data[24:32]
check_bit = data[32:40]

humidity = 0
humidity_point = 0
temperature = 0
temperature_point = 0
check = 0

#转换数值
for i in range(8):
    humidity+=humidity_bit[i]*2**(7-i)
    humidity_point+=humidity_point_bit[i]*2**(7-i)
    temperature+=temperature_bit[i]*2**(7-i)
    temperature_point+=temperature_point_bit[i]*2**(7-i)
    check+=check_bit[i]*2**(7-i)

tmp=humidity+humidity_point+temperature+temperature_point
#数据校验，并输出相应的温度湿度值
if check==tmp:
    print "temperature is:",temperature,"C,","humidity is:",humidity,"%"
    
else:
    print "Something is wrong the humidity,humidity,humidity_point,temperature,temperature_point,check is",humidity,humidity_point,temperature,temperature_point,check
gpio.cleanup()

#数据转换成JSON格式
mytemp='{"value":%f}'%temperature
myhumi='{"value":%f}'%humidity
#打开文件
tmp_output = open('/home/pi/mytest/DHT11/tmp_data.txt','w')
hud_output = open('/home/pi/mytest/DHT11/hud_data.txt','w')
#写数据到文本文件中
tmp_output.write(mytemp)
hud_output.write(myhumi)
#关闭文件
tmp_output.close
hud_output.close

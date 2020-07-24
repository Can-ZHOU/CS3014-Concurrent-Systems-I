# A file to calculate average time for each test.
import pandas as pd

f1 = open("16_16_1_32_32_20.txt")
f2 = open("16_16_1_32_32_50.txt")
f3 = open("16_16_1_32_32_100.txt")
f4 = open("16_16_1_32_32_400.txt")
f5 = open("16_16_1_32_32_800.txt")
f6 = open("16_16_1_32_32_1000.txt")

line = f1.readline()
line = f1.readline()
line = f1.readline()
line = f1.readline()
count = 0
sum_num = 0
r1 = [0.0,0.0,0.0]
i=0
while line:
    if(line.startswith(('0', '1', '2', '3', '4', '5', '6', '7', '8', '9'))):
        sum_num += int(line)
        count += 1
    else:
        r1[i] = float(sum_num) / float(count)
        count = 0
        sum_num = 0
        i += 1
    line = f1.readline()
r1[i] = float(sum_num) / float(count)


line = f2.readline()
line = f2.readline()
line = f2.readline()
line = f2.readline()
count = 0
sum_num = 0
r2 = [0.0,0.0,0.0]
i=0
while line:
    if(line.startswith(('0', '1', '2', '3', '4', '5', '6', '7', '8', '9'))):
        sum_num += int(line)
        count += 1
    else:
        r2[i] = float(sum_num) / float(count)
        count = 0
        sum_num = 0
        i += 1
    line = f2.readline()
r2[i] = float(sum_num) / float(count)


line = f3.readline()
line = f3.readline()
line = f3.readline()
line = f3.readline()
count = 0
sum_num = 0
r3 = [0.0,0.0,0.0]
i=0
while line:
    if(line.startswith(('0', '1', '2', '3', '4', '5', '6', '7', '8', '9'))):
        sum_num += int(line)
        count += 1
    else:
        r3[i] = float(sum_num) / float(count)
        count = 0
        sum_num = 0
        i += 1
    line = f3.readline()
r3[i] = float(sum_num) / float(count)


line = f4.readline()
line = f4.readline()
line = f4.readline()
line = f4.readline()
count = 0
sum_num = 0
r4 = [0.0,0.0,0.0]
i=0
while line:
    if(line.startswith(('0', '1', '2', '3', '4', '5', '6', '7', '8', '9'))):
        sum_num += int(line)
        count += 1
    else:
        r4[i] = float(sum_num) / float(count)
        count = 0
        sum_num = 0
        i += 1
    line = f4.readline()
r4[i] = float(sum_num) / float(count)


line = f5.readline()
line = f5.readline()
line = f5.readline()
line = f5.readline()
count = 0
sum_num = 0
r5 = [0.0,0.0,0.0]
i=0
while line:
    if(line.startswith(('0', '1', '2', '3', '4', '5', '6', '7', '8', '9'))):
        sum_num += int(line)
        count += 1
    else:
        r5[i] = float(sum_num) / float(count)
        count = 0
        sum_num = 0
        i += 1
    line = f5.readline()
r5[i] = float(sum_num) / float(count)


line = f6.readline()
line = f6.readline()
line = f6.readline()
line = f6.readline()
count = 0
sum_num = 0
r6 = [0.0,0.0,0.0]
i=0
while line:
    if(line.startswith(('0', '1', '2', '3', '4', '5', '6', '7', '8', '9'))):
        sum_num += int(line)
        count += 1
    else:
        r6[i] = float(sum_num) / float(count)
        count = 0
        sum_num = 0
        i += 1
    line = f6.readline()
r6[i] = float(sum_num) / float(count)


dataframe = pd.DataFrame({'20':r1, '50':r2, '100':r3, '400':r4, '800':r5, '1000':r6})
dataframe.to_csv("nz_ratio.csv",index=False,sep=',')

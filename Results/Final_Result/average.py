# A file to calculate average time for each test.
import pandas as pd

f1 = open("1.txt")
f2 = open("2.txt")
f3 = open("3.txt")
f4 = open("4.txt")
f5 = open("5.txt")
f6 = open("6.txt")

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


dataframe = pd.DataFrame({'1':r1, '2':r2, '3':r3, '4':r4, '5':r5, '6':r6})
dataframe.to_csv("nz_ratio.csv",index=False,sep=',')

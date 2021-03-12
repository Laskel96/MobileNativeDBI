f1 = open("call", "r")
lines = f1.readlines()

lines2 = []

lines2.append(lines[0])

loop=1
check=0

while loop < (len(lines)-1) :
	if check:
		check=0
		lines2.append(lines[loop])
		loop = loop+1
		continue

	l1 = lines[loop].split()
	l2 = lines[loop+1].split()

	if l1[3] != l2[5] or l1[5] != l2[3]:
		lines2.append(lines[loop])
		check=1
	
	loop = loop+1

f2 = open("callback", "w")
for line in lines2:
	f2.write(line)

f2.close()
f1.close()


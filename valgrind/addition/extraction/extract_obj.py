f = open("log", 'r')

lines=f.readlines()

lines2 = []

for line in lines:
	if(line.startswith("object")):
		lines2.append(line)

del lines2[0]

lib_list = []

for line in lines2:
	l = line.split()
	lib_list.append(l[3])
	lib_list.append(l[5])

s = list(set(lib_list))

f.close()

f = open("lib_list", "w")
for l in s:
	f.write(l+"\n")

f.close()

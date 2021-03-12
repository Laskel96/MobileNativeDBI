#!/usr/bin/python

import sys

fname = str(sys.argv[1])
dump_file = open(fname,"r")
db_s = open("DB_String", "w")
db_m = open("DB_method", "w")
db_offset = open("DB_offset", "w")

data_str = []
data_method_name = []
data_method_code = []

while True:
	line = dump_file.readline()
	if not line : break
	if 'string@' in line:
		data_str.append(line)
	if 'dex_method_idx' in line:
		data_method_name.append(line)
	if 'size_offset' in line:
		data_method_code.append(line)
	if 'EXECUTABLE OFFSET:' in line:
		offset = dump_file.readline()
		db_offset.write(offset)

result_str = dict()

tmp_method_name=[]
tmp_method_code=[]
result_method = dict()

for l in data_str:
	l = l.replace("\r\n","")#.replace("\"","")
	loc = l.find(",")
	l = l[loc+1:]
	l = l.split(" // ")
	name =  l[0].strip()
	num = l[1].split("@")[1]
	result_str[num] = name

for d in result_str.keys():
	s = "%s %s\n" % (d, result_str.get(d))
	db_s.write(s)

for l in data_method_name:
	l = l.replace("\r\n","").replace("\"","")
	l = l.split("(dex_method_idx")[0].split(":")[1].strip()
	tmp_method_name.append(l)

for l in data_method_code:
	l = l.replace("\r\n","").replace("\"","")
	l = l.split(" ")
	offset = l[5][15:]
	size = l[7][5:].replace(".","").replace(")","")
	base = int(offset,16) #need -1 & -0x29a000
	end = base + int(size, 10)
	s = "%s %s" % (hex(base), hex(end))
	tmp_method_code.append(s)

for i in range(0,len(tmp_method_name)):
#if "android.support" not in tmp_method_name[i] and "androidx." not in tmp_method_name[i]: 
#	if "android.support" not in tmp_method_name[i]: 
		s = "%s %s\n" % ( tmp_method_code[i], tmp_method_name[i])
		db_m.write(s)
	
dump_file.close()
db_s.close()
db_m.close()

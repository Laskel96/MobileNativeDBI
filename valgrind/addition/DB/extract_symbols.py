#!/usr/bin/python

import sys

targetInputOatFile = str(sys.argv[1])

targetOatFile = open(targetInputOatFile,"r")

dataBaseString = open("valgrind/addition/DB/DB_String", "w")
dataBaseMethod = open("valgrind/addition/DB/DB_method", "w")
dataBaseOffset = open("valgrind/addition/DB/DB_offset", "w")

stringDataLineList = []
methodNameLineList = []
methodCodeOffsetLineList = []

while True:
	line = targetOatFile.readline()
	if not line : break
	if 'string@' in line:
		stringDataLineList.append(line)
	if 'dex_method_idx' in line:
		methodNameLineList.append(line)
	if 'size_offset' in line:
		methodCodeOffsetLineList.append(line)
	if 'EXECUTABLE OFFSET:' in line:
		offset = targetOatFile.readline()
		dataBaseOffset.write(offset)

stringDataDictionary = dict()
methodDataDictionary = dict()

methodNameList=[]
methodCodeOffsetList=[]

for line in stringDataLineList:
	line = line.replace("\r\n","")
	loc = line.find(",")
	line = line[loc+1:]
	line = line.split(" // ")
	string =  line[0].strip()
	stringId = line[1].split("@")[1]
	stringDataDictionary[stringId] = string

for stringId in stringDataDictionary.keys():
	stringData = "%s %s\n" % (stringId, stringDataDictionary.get(stringId))
	dataBaseString.write(stringData)

for line in methodNameLineList:
	line = line.replace("\r\n","").replace("\"","")
	line = line.split("(dex_method_idx")[0].split(":")[1].strip()
	methodNameList.append(line)

for line in methodCodeOffsetLineList:
	line = line.replace("\r\n","").replace("\"","")
	line = line.split(" ")
	offset = line[5][15:]
	size = line[7][5:].replace(".","").replace(")","")
	base = int(offset,16) #need -1 & -0x29a000
	end = base + int(size, 10)
	methodRange = "%s %s" % (hex(base), hex(end))
	methodCodeOffsetList.append(methodRange)

for i in range(0,len(methodNameList)):
#if "android.support" not in tmp_method_name[i] and "androidx." not in tmp_method_name[i]: 
#	if "android.support" not in tmp_method_name[i]: 
		methodData = "%s %s\n" % ( methodCodeOffsetList[i], methodNameList[i])
		dataBaseMethod.write(methodData)
	
targetOatFile.close()
dataBaseString.close()
dataBaseMethod.close()
dataBaseOffset.close()

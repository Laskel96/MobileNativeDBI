import sys
from androguard.misc import AnalyzeAPK
from androguard.core.bytecodes.apk import APK
import shutil
import glob, os
import logging
import warnings

def get_pkg(apk_path) :
	a, d, dx = AnalyzeAPK(apk_path)
	return a.get_package()

def get_main_activity(apk_path) :
	a, d, dx = AnalyzeAPK(apk_path)
	return a.get_main_activity()

if __name__ == "__main__" :
	logging.basicConfig()
	warnings.filterwarnings(action='ignore')
	for f in os.listdir("./") :
		if f.endswith(".apk"):
			a, d, dx = AnalyzeAPK(f)
			name=a.get_package()
			package=a.get_main_activity()
			package=package.replace(name,"")
			print(f+":"+name+":"+ package)

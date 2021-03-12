
/*--------------------------------------------------------------------*/
/*--- Nulgrind: The minimal Valgrind tool.               nl_main.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Nulgrind, the minimal Valgrind tool,
   which does no instrumentation or analysis.

   Copyright (C) 2002-2017 Nicholas Nethercote
   njn@valgrind.org

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
 */

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"

#include "pub_tool_debuginfo.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_libcfile.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_aspacemgr.h"
#include "pub_tool_guest.h"
#include "pub_core_threadstate.h"
#include "pub_tool_threadstate.h"

#include "libvex_guest_x86.h"

#define MAX_LIB_LIST 76
#define TEST_DEBUG 0
#define BASIC_PRINT 1
#define CALL_PRINT 0

static char  * backup_objname = NULL;
//Lists of Excluded Library names. These are too low for logging.
const char * lib_list[MAX_LIB_LIST] = {"/system/lib/egl/libEGL_emulation.so", "/data/dalvik-cache/x86/system@framework@boot.oat","/system/lib/libart.so","/system/lib/libnativebridge.so","/system/lib/libc.so","/system/lib/libstdc++.so","/system/bin/app_process32","/system/lib/libEGL.so","/system/lib/libgui.so","/system/lib/libcommon_time_client.so","/system/lib/libskia.so","/system/lib/libpng.so","/system/lib/libwebviewchromium_loader.so","/system/lib/libm.so","/system/bin/linker","/system/lib/libinput.so","/system/lib/libcamera_client.so","/system/lib/libaudioeffect_jni.so","/system/lib/libbcinfo.so","/system/lib/libicuuc.so","/system/vendor/lib/libadreno_utils.so","/system/lib/librs_jni.so","/system/lib/libsqlite.so","/system/vendor/lib/egl/libEGL_adreno.so","/system/lib/libselinux.so","/system/lib/libandroid.so","/system/lib/libmedia.so","/system/lib/libcutils.so","/system/lib/libmemtrack.so","/system/lib/libcrypto.so","/system/lib/libui.so","/system/lib/libpowermanager.so","/system/lib/libmedia_jni.so","/system/lib/libstagefright_foundation.so","/system/lib/libGLESv2.so","/system/lib/libhwui.so","/system/lib/libutils.so","/system/vendor/lib/egl/eglsubAndroid.so","/system/lib/libc++.so","/system/lib/libandroid_runtime.so","/system/lib/libstagefright.so","/system/lib/libdrmframework.so","/system/lib/libbcc.so","/system/lib/libharfbuzz_ng.so","/system/lib/libmemalloc.so","/system/vendor/lib/libgsl.so","/system/lib/libminikin.so","/system/lib/libsoundtrigger.so","/system/lib/hw/gralloc.msm8974.so","/system/lib/liblog.so","/system/lib/libjavacrypto.so","/system/lib/libstlport.so","/system/lib/libhardware.so","/system/lib/libbinder.so","/system/lib/libicui18n.so","/system/lib/libimg_utils.so","/data/dalvik-cache/arm/system@framework@boot.oat","/system/lib/libLLVM.so","/system/vendor/lib/libsc-a3xx.so","/system/vendor/lib/egl/libGLESv1_CM_adreno.so","/system/lib/hw/memtrack.msm8974.so","/system/lib/libexpat.so","/system/lib/libnetd_client.so","/system/lib/libjavacore.so","/system/lib/libpdfium.so","/system/lib/libft2.so","/system/lib/libsoundpool.so","/system/lib/libandroidfw.so","/system/lib/libGLES_trace.so","/system/lib/libgabi++.so","/system/vendor/lib/egl/libGLESv2_adreno.so","/system/lib/libz.so","/system/lib/libqdutils.so","/system/lib/libssl.so","/system/lib/libnativehelper.so","/system/lib/lib0"};

//Check str is in lib_list. if found, return 0, else, return 1
int check_lib(char * str){
	int i;
	for(i = 0; i < MAX_LIB_LIST; i++){
		if(!VG_(strcmp)(lib_list[i], str))
			return 0;
	}
	return 1; //not found
}

int dynamicBufSize=512;

/*
   Read one Line From specified fd. Put \n and \0 at the end of line.
   If buffer is too small, then make it double. Base Buffer Size is above.
   VG_(read) function always return something cause it retuns number of byte that it read. When VG_(read) reaches EOF, it returns 0.
 */
int myReadLine(int fd, char ** buf){
	char * bp = *buf;
	char ch;
	int res;

	res = VG_(read)(fd, &ch, 1);
	//End of File
	if(res == 0){
		if(TEST_DEBUG)
			vex_printf("end of file\n");
		return 0;
	}

	*bp++ = ch;

	while(1){
		res = VG_(read)(fd,&ch, 1);
		if(bp == *buf+dynamicBufSize){ //if buffer is full
			char * tempBuf = VG_(malloc)("tempBuffer", dynamicBufSize * 2);
			VG_(strncpy)(tempBuf, *buf, dynamicBufSize);
			VG_(free)(*buf);
			*buf = tempBuf;
			bp = *buf+dynamicBufSize;
			dynamicBufSize = dynamicBufSize * 2;
			if(TEST_DEBUG)
				vex_printf("Buffer expanded\n");
		}
		if(ch == '\n'){
			*bp++ = '\n';
			*bp = '\0';
			break;
		}else{
			*bp++ = ch;
		}
	}
	if(TEST_DEBUG){
		vex_printf("buffer : %s\n",*buf);
		vex_printf("ReadLine End\n");
	}
	return 1;
}

static char * packageName=NULL;
void readPackageName(){
	char * buf = VG_(malloc)("myBuffer", dynamicBufSize);
	SysRes sr;
	int len = 0;
	sr = VG_(open)("/data/local/tmp/package_name", VKI_O_RDONLY, 777);
	if(0 < sr._val){
		myReadLine(sr._val, &buf);
		len = VG_(strlen)(buf);
		buf[len-1] = '\0';
		packageName = VG_(malloc)("pkgName",  len);
		VG_(strncpy)(packageName, buf, len);
		VG_(free)(buf);
		VG_(close)(sr._val);
		vex_printf("package name : %s\n", packageName);
	}
}


static long oat_offset=0;
void readOatOffset(){
	char * buf = VG_(malloc)("myBuffer", dynamicBufSize);
	SysRes sr;
	sr = VG_(open)("/data/local/tmp/DB_offset", VKI_O_RDONLY, 777);
	if(0 < sr._val){
		myReadLine(sr._val, &buf);
		oat_offset=VG_(strtoull16)(buf, NULL);
		vex_printf("oat offset : 0x%x\n", oat_offset);
	}
}

static int stringSignatureCount=0;
static char * signature_string[128];
void readSignature(){
	char * buf = VG_(malloc)("myBuffer", dynamicBufSize);
	SysRes sr;
	int len = 0;
	sr = VG_(open)("/data/local/tmp/signature_string", VKI_O_RDONLY, 777);
	if(0 < sr._val){
		while(myReadLine(sr._val, &buf)){
			len = VG_(strlen)(buf);
			buf[len-1] = '\0';
			signature_string[stringSignatureCount] = VG_(malloc)("string signature", len);
			VG_(strncpy)(signature_string[stringSignatureCount], buf, len);
			stringSignatureCount=stringSignatureCount+1;
		}
		VG_(free)(buf);
		VG_(close)(sr._val);
	}
}

int searchStringSignature(char * str){
	int i;
	for(i = 0; i < stringSignatureCount; i++){
		if(VG_(strstr)(str, signature_string[i]) != NULL)
			return i;
	}
	return -1;
}

/*
   Make my string table.
   String Table consist with {String index in oat, String}.
   Read one line from DB_String file, which produced at preprocessing, and put it into our sturcture.
   Program access string with string index, So we can easilly found which string is in use.
 */
static int str_count = 0;
static char *str_data[1024*8][2]; // {(int)String index , String}
void readString(){
	int len=0;
	char * buf = VG_(malloc)("myBuffer", dynamicBufSize);
	char *ch1, *ch2;
	long long int lli = 0;
	SysRes sr;
	sr = VG_(open)("/data/local/tmp/DB_String", VKI_O_RDONLY, 777);
	if( 0 < sr._val){	
		vex_printf("Parsing String\n");

		/*
		   Read One Line and Parsing it to string_data
		 */
		while(myReadLine(sr._val, &buf)){
			ch1 = ch2 = buf;
			lli = (long long int)VG_(strtoll10)(ch1, &ch2); //get index of string
			str_data[str_count][0]=lli; //put index to array
			ch1 = ch2+1;
			ch2 = VG_(strchr)(ch1, '\n'); //find end of line
			len = ch2-ch1+1; 
			str_data[str_count][1] = VG_(malloc)("string data", len);
			VG_(strncpy)(str_data[str_count][1], ch1, len);
			*(str_data[str_count][1]+len-1)='\0';
			ch1 = ch2+1;
			if(TEST_DEBUG)
				vex_printf("%d : %s\n", str_data[str_count][0], str_data[str_count][1]);
			str_count = str_count+1;
		}
		vex_printf("parsing String complete\n");
	}else{
		vex_printf("Problem with string data");
	}
	VG_(free)(buf);
	VG_(close)(sr._val);
}

/*
   Make my method table.
   Method Table consist with {base address, end address, method name}.
   Read one line from DB_method file, which produced at preprocessing, and put it into our sturcture.
   OAT File starts with their own base address that is not exactly match with memory map.
   So, We need to make it suitable. The actual Function's code will be inside [ Segment Base~ Segment Base + end ]. For this, we can easily specify which function is running.
 */
static int method_count=0;
static char * method_data[1024*32][3];//{base, end ,method name} address base = 29A000, OAT Addr = native + 1
void readMethod(){
	SysRes sr;
	char * buf = VG_(malloc)("myBuffer", dynamicBufSize);
	int len;
	char * ch1, *ch2;
	long long int lli = 0;
	sr = VG_(open)("/data/local/tmp/DB_method", VKI_O_RDONLY, 777);
	if( 0 < sr._val){
		vex_printf("Parsing method\n");
		/*
		   Read One Line and Parsing it to method_data
		 */
		while(myReadLine(sr._val, &buf)){
			ch1 = ch2 = buf;
			lli = VG_(strtoll16)(ch1, &ch2); // get base addr of method
			method_data[method_count][0] = lli; //put base addr of method to array
			ch1 = ch2+1;
			lli = VG_(strtoll16)(ch1, &ch2); // get end addr of method
			method_data[method_count][1] = lli; //put end addr of method
			ch1 = ch2+1;

			ch2 = VG_(strchr)(ch1, '\n'); //find end of line
			len = ch2-ch1+1;
			method_data[method_count][2] = VG_(malloc)("method name", len);
			VG_(strncpy)(method_data[method_count][2], ch1, len);
			*(method_data[method_count][2]+len-1)='\0';
			ch1 = ch2+1;
			if(TEST_DEBUG)
				vex_printf("%d ~ %d : %s\n", method_data[method_count][0], method_data[method_count][1], method_data[method_count][2]);
			method_count = method_count+1;
		}
		vex_printf("parsing method complete\n");
	}else{
		vex_printf("Problem with method data");
	}
	VG_(free)(buf);
	VG_(close)(sr._val);	
}

/*
   Search String in str_data.
   if found, return index of it, else, return -1
 */
int searchString(unsigned int sIndex){
	int i = 0;
	for(i = 0; i < str_count;  i++){
		if((unsigned int)str_data[i][0] == sIndex){
			return i;
		}
	}
	return -1;
}

/*
   Search method name in method_data.
   if found, return name of it , else, return -1
   The offset should be offset from segment.[ = Address - Segment base]
 */
char * searchMethod(int offset){//dex2oat starts with offset and code offset = actual_code+1  but in x86 emulator : offset and no +1
	int i = 0;
	for(i =0; i < method_count; i++){
		if(offset != 0 && offset >= (unsigned int)method_data[i][0]-oat_offset && offset <= (unsigned int)method_data[i][1]-oat_offset){
			return method_data[i][2];
		}

	}
	return NULL;
}

static void nl_post_clo_init(void)
{
}

/*
   Inject BB.
   Make
   MOV eax, 0
   ret
   Assembly set.
   It make return value 0, and exit function.
 */

void inject_branch_retOnly(IRSB * bb, int base, int end, int valRet){

	/* 
	   Make Return 0
	   Put 0 in EAX, RAX 
	 */   
	IRExpr * zeroConst = IRExpr_Const(IRConst_U32(valRet));//make return value
	IRStmt * putEaxZeroStmt = IRStmt_Put(8, zeroConst);
	ppIRStmt(putEaxZeroStmt);
	vex_printf("\n");
	addStmtToIRSB(bb, putEaxZeroStmt);

	IRTemp fourByteIRTemp;
	fourByteIRTemp = newIRTemp(bb->tyenv, 0x1104);

	/*
	   Find return instruction in object
	   For ARM, 0x4770 => bx lr
	   for Intel, 0xC3 => ret
	 */

	unsigned char * returnAddressInObject = base;
	int returnAddressInObjectIndex=0;

	//For ARM
	/*
	   for(findrti = 0; findrti < end-1; findrti++){
	   if((*findrt == 0x70 && *(findrt+1)== 0x47))
	   break;
	   findrt++;
	   }
	 */

	//For Intel
	for(returnAddressInObjectIndex = base+0x100; returnAddressInObjectIndex <= end; returnAddressInObjectIndex++){
		if(*returnAddressInObject == 0xc3)
			break;
		returnAddressInObject++;
	}


	/*
	   Make tmp Value for Store address about next BB
	   tx = (ret addr)
	   bb->next = tx
	 */
	if(returnAddressInObjectIndex == end)
		vex_printf("there is no bx lr instruction");
	else{
		//IRExpr * tt3 = IRExpr_Const(IRConst_U32((unsigned int)findrt+1));//why arm need +1? ... don't know
		IRExpr * returnAddressConst = IRExpr_Const(IRConst_U32((unsigned int)returnAddressInObject));// Intel don't need +1
		IRStmt * putReturnAddrStmt = IRStmt_WrTmp(fourByteIRTemp,returnAddressConst);
		ppIRStmt(putReturnAddrStmt);
		vex_printf("\n");
		addStmtToIRSB(bb, putReturnAddrStmt);
		bb->next = deepCopyIRExpr(IRExpr_RdTmp(fourByteIRTemp));
	}
}

int returnString = 0;
int resolveString = 0;
int isReturned=0;

/*
   Interface of Injecting IR.
   type mean inject value or inject branch.
 */
void inject_ir(IRSB* bb, IRStmt * st, int base, int end, int line, int type){ // type 0: value 1 : branch
	if(type == 0 && line == 11){ // inject value
		//inject = 0;
		vex_printf("Injected, changed in to : ");    
		st->Ist.WrTmp.data->Iex.Binop.arg2->Iex.Const.con->Ico.U32=1;
		ppIRStmt(st);
		vex_printf("\n");
	}    

	//if(type==1 && line == 26){ //Inject branch
	//if(type==1 && line == 0){ //Inject branch
	if(type==1){
		//inject = 0;
		vex_printf("Branch Injected\n");    
		inject_branch_retOnly(bb, base,end ,0);
	}
}

static int cstr=1;

	static
IRSB* nl_instrument ( VgCallbackClosure* closure,
		IRSB* bb_in,
		const VexGuestLayout* layout, 
		const VexGuestExtents* vge,
		const VexArchInfo* archinfo_host,
		IRType gWordTy, IRType hWordTy )
{
	IRStmt* st;
	int i = 0;

	IRSB* bb	= emptyIRSB();
	bb->tyenv	= deepCopyIRTypeEnv(bb_in->tyenv);
	bb->next	= deepCopyIRExpr(bb_in->next);
	bb->jumpkind = bb_in->jumpkind;
	bb->offsIP	= bb_in->offsIP;

	int native=0;
	const NSegment *seg;
	unsigned int base = 0, end = 0;
	char * fnname=NULL;

	if(cstr){
		cstr = 0;
		readOatOffset();
		readPackageName();
		readSignature();
		readString();
		readMethod();
		vex_printf("Initialized!\n");
	}

	for(i = 0 ; i < bb_in->stmts_used ; i++) {
		st = bb_in->stmts[i];

		if(!st)
			continue;

		char * str = NULL, * filename=NULL;

		if(isReturned && i == 0){
			int tid = VG_(get_running_tid)();
			VexGuestArchState * vex = &(VG_(get_ThreadState)(tid)->arch.vex);
			vex_printf("returned eax : %x\n", vex->guest_EAX);
			isReturned=0;
		}

		if((returnString||resolveString) && i==0){ //Check Resolved String
			int tid = VG_(get_running_tid)();
			VexGuestArchState * vex = &(VG_(get_ThreadState)(tid)->arch.vex);
			if(returnString){
				vex_printf("string returned\n");
				returnString=0;
			}else if(resolveString){
				vex_printf("string dynamic found\n");
				resolveString=0;
			}
			if(TEST_DEBUG){
				vex_printf("eax : %x\n", vex->guest_EAX);
				vex_printf("esi : %x\n", vex->guest_ESI);
				vex_printf("edi : %x\n", vex->guest_EDI);
			}
			int loop;

			if(vex->guest_EAX != 0 && vex->guest_EAX > 0x6000000){ //At least over 0x6000000, if less sometimes crash happends
				int * tPtr = vex->guest_EAX+8;
				unsigned int stringClassAddr = *tPtr;
				if(stringClassAddr > 0x600){
					int * lenPtr = vex->guest_EAX+12;
					int strLen = *lenPtr;
					int * strOffsetPtr = vex->guest_EAX+20;
					int strOffset = *strOffsetPtr;
					char * actualStr = stringClassAddr+12+strOffset*2;
					vex_printf("strlen : 0x%x\n", strLen);

					if(TEST_DEBUG){	//Dump address
						for(loop=0; loop < 24; loop++){
							vex_printf("%02x ", *((char *)vex->guest_EAX+loop));
							if(loop % 16 == 15)
								vex_printf("\n");
						}
						vex_printf("\n");
						for(loop=0; loop < 24; loop++){
							vex_printf("%02x ", *((char *)stringClassAddr+loop));
							if(loop % 16 == 15)
								vex_printf("\n");
						}
						vex_printf("\n");
					}
					if(strLen < 100){ //Mostly don't need that long string..
						int index=0;
						char stringArray[strLen+1];
						vex_printf("string data : \"");
						for(loop=0; loop < strLen*2; loop+=2){
							vex_printf("%s", actualStr+loop);
							stringArray[index++] = *(actualStr+loop);
						}
						vex_printf("\"\n");
						int sig = searchStringSignature(stringArray);
						vex_printf("sig : %d\n", sig);
						if(sig != -1){
							vex_printf("found signature!\n");
							*actualStr=",";
							vex_printf("%s changed to ",stringArray);
							for(loop=0; loop < strLen*2; loop+=2){
								vex_printf("%s", actualStr+loop);
							}
						}
						vex_printf("\n");
					}
				}
			}
		}

		if(st->tag == Ist_IMark){ //Check Library. Exclude if needed(for performance)
			VG_(get_objname)(VG_(current_DiEpoch)(), st->Ist.IMark.addr ,&str);
			if(str != NULL)
				if(check_lib(str))
					native=1;
				else
					native=0;
			/*
			   if(CALL_PRINT)
			   native=1;
			 */
		}

		if(native){ // print log
			if(str != NULL && i == 0){
				vex_printf("object : %s\n", str);
				if(VG_(strstr)(str, packageName) != NULL){
					seg = VG_(am_find_nsegment)(st->Ist.IMark.addr);
					if(seg != NULL){
						if(BASIC_PRINT)
							vex_printf("seg  %x ~ %x \n", seg->start, seg->end);
						base = seg->start;
						end = seg->end;
						fnname = searchMethod(st->Ist.IMark.addr-base);
					}else{
						vex_printf("no seg");
					}
					if(base != 0){ //Current Function
						if(fnname != NULL){
							vex_printf("Function : %s\n", fnname);
							if(TEST_DEBUG){//Inject Specific Function. Usually cannot used because of obfuscation.
								if(VG_(strstr)(fnname, "checkForSuBinary") != NULL){
									vex_printf("Found %s\n", fnname);
									inject_ir(bb, st, base, end ,i , 1); //inject branch
									//inject=1;
									break;
								}
							}
						}
					}
					if(BASIC_PRINT){ // Next Function
						vex_printf("Jump to : ");
						if(bb_in->next->tag == Iex_Const){ // In function
							char * fnname = searchMethod(bb_in->next->Iex.Const.con->Ico.U32-base);
							if(fnname != NULL)
								vex_printf("%s\n", fnname);
							else
								vex_printf("No function\n");
						}else if(bb_in->next->tag == Iex_RdTmp){ // to other function
							ppIRExpr(bb_in->next);
							vex_printf("\n");
						}
					}
				}
			}
			if(BASIC_PRINT || CALL_PRINT){ // Print IR
				vex_printf("%d th | ", i);
				ppIRStmt(st);
			}
			if(st->tag == Ist_IMark){ // Print Assembly. Check Instruction
				unsigned char * mark = st->Ist.IMark.addr;
				int j;
				//vex_printf("ASSM : ");
				//for ARM
				/*
				   if(st->Ist.IMark.len == 2){
				   vex_printf("%02x", *(mark+1));
				   vex_printf("%02x", *(mark));
				   }else if(st->Ist.IMark.len == 4){
				   vex_printf("%02x", *(mark+1));
				   vex_printf("%02x", *(mark));
				   vex_printf("%02x", *(mark+3));
				   vex_printf("%02x", *(mark+2));
				   }*/
				//for Intel
				if(BASIC_PRINT){	
					for(j = 0; j < st->Ist.IMark.len; j++){
						vex_printf("%02x", *(mark+j));
					}
				}
				mark = st->Ist.IMark.addr;
				if(st->Ist.IMark.len == 1 && *mark == 0xc3){ // Check it is string when return
					isReturned=1;
					if(fnname != NULL){
						char * stringClassArray = "java.lang.string[]";
						char * stringClass = "java.lang.String";
						if(VG_(strncmp)(stringClass, fnname, VG_(strlen)(stringClass)) == 0){
							if(VG_(strncmp)(stringClassArray, fnname, VG_(strlen)(stringClassArray)) != 0)
								returnString=1;
						}
					}
				}
			}else if(st->tag == Ist_Put){//check string
				if(st->Ist.Put.data->tag == Iex_Const && st->Ist.Put.offset == 12){
					int index = searchString(st->Ist.Put.data->Iex.Const.con->Ico.U32);
					if(index != -1){
						vex_printf("\nStatic String data = %d : ", st->Ist.Put.data->Iex.Const.con->Ico.U32);
					}
				}
			}
			else if(st->tag == Ist_WrTmp){
				if(st->Ist.WrTmp.data->tag == 0x190B){
					if(st->Ist.WrTmp.data->Iex.CCall.args[0] != NULL &&
							st->Ist.WrTmp.data->Iex.CCall.args[1] != NULL &&
							st->Ist.WrTmp.data->Iex.CCall.args[2] != NULL &&
							st->Ist.WrTmp.data->Iex.CCall.args[3] != NULL){
						if(st->Ist.WrTmp.data->Iex.CCall.args[3]->tag == 0x1909){
							if(st->Ist.WrTmp.data->Iex.CCall.args[3]->Iex.Const.con->Ico.U32 == 0x150){
								resolveString=1;
							}
						}
					}
				}
			}
			if(BASIC_PRINT || CALL_PRINT)
				vex_printf("\n");
		}
		addStmtToIRSB(bb, deepCopyIRStmt(st));
	}
	//inject = 0;
	return bb;
}

static void nl_fini(Int exitcode)
{
}

static void nl_pre_clo_init(void)
{
	VG_(details_name)            ("Nulgrind");
	VG_(details_version)         (NULL);
	VG_(details_description)     ("the minimal Valgrind tool");
	VG_(details_copyright_author)(
			"Copyright (C) 2002-2017, and GNU GPL'd, by Nicholas Nethercote.");
	VG_(details_bug_reports_to)  (VG_BUGS_TO);

	VG_(details_avg_translation_sizeB) ( 275 );

	VG_(basic_tool_funcs)        (nl_post_clo_init,
			nl_instrument,
			nl_fini);

	/* No needs, no core events to track */
}

VG_DETERMINE_INTERFACE_VERSION(nl_pre_clo_init)

	/*--------------------------------------------------------------------*/
	/*--- end                                                          ---*/
	/*--------------------------------------------------------------------*/

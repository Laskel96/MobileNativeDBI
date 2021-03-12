# Mobile_Native_DBI

Yonnggu Shin's Master Degree papaer  
Paper Name : Instruction-level OAT native code Instrumentation Scheme based on Process Wrapping  

## Setting
Ubuntu 14 to 18 is recommended for the host or VM operating system.  
This tool requires Android Studio or at least AVD Manager.  
The sample emulator was set up with Nexus 5 and Lillipop android 5.1 x86.  
> Note: Higher versions of the emulator may cause unexpected problems.  

## Install & Usage
When the emulator is ready, follow the steps below.  
First, You should edit run_nativeDBI_by_host.sh.  
Change target <application name> and <emulator name>  
After change, Just run below commands.  
```sh
mkdir native_dbi
cd native_dbi
git clone https://github.com/ssu-csec/code-Mobile_Native_DBI
./run_nativeDBI_by_host.sh
```

## Compile
If there need any changes to dbi, you may change code in valgrind/none/nl_main.c  
After Change run  
```sh
cd valgrind
./android.sh
```
This will compile your code into valgrind/Inst.  
If you are using 'real device' (or need ARM architecture) change android.sh file. (There already examples for it)  

#copied from sonLib...

# we do specific stuff for specific host for now.
HOSTNAME = $(shell hostname)
MACH = $(shell uname -m)
SYS =  $(shell uname -s)

platformCompileFlags=
platformLinkFlags=

#C compiler
ifeq (${SYS},FreeBSD)
    # default FreeBSD gcc (4.2.1) has warning bug
    #cxx = gcc46 -std=c99 -Wno-unused-but-set-variable
    cxx = gcc34 -std=c99 -Wno-unused-but-set-variable
    cpp = g++
else ifeq ($(SYS),Darwin) #This is to deal with the Mavericks replacing gcc with clang fully
	cxx = clang -std=c99 
	cpp = clang++
	platformCompileFlags = -I/opt/local/include
	platformLinkFlags g =  -L/opt/local/lib
else
    cxx = gcc -std=c99
    cpp = g++ 
endif

# -Wno-unused-result

# Compiler flags.
# DO NOT put static library -l options here. Those must be specified *after*
# linker input files. See <http://stackoverflow.com/a/8266512/402891>.

#Release compiler flags
cflags_opt = -O3 -g -Wall --pedantic -funroll-loops -DNDEBUG 
#-fopenmp
cppflags_opt = -O3 -g -Wall -funroll-loops -DNDEBUG 

#Debug flags (slow)
cflags_dbg = -Wall -Werror --pedantic -g -fno-inline
cppflags_dbg = -Wall -g -O0 -fno-inline 

#Ultra Debug flags (really slow)
cflags_ultraDbg = -Wall -Werror --pedantic -g -fno-inline 

#Profile flags
cflags_prof = -Wall -Werror --pedantic -pg -O3 

#for cpp code: don't use pedantic, or Werror
cppflags = ${cppflags_dbg} 

#Flags to use
cflags = ${cflags_dbg} 

binPath=${rootPath}
libPath=${rootPath}

sgExportPath=${rootPath}/sgExport
protobufPath=${rootPath}/protobuf
rapidJsonPath=${rootPath}/rapidjson

cflags +=  -I ${sgExportPath} ${platformCompileFlags}
cppflags +=  -I ${sgExportPath} -I ${rapidJsonPath}/include -I ${protobufPath}/build/include ${platformCompileFlags}
basicLibs = ${sgExportPath}/sgExport.a ${protobufPath}/libprotobuf.a -lz ${platformLinkFlags} -lcurl
basicLibsDependencies = ${sgExportPath}/sgExport.a ${protobufPath}/libprotobuf.a



# Makefile for COMP 150IDS RPC Samples
#
#    Copyright 2012 - Noah Mendelsohn
#
#
# Useful targets:
#
#
#    pingstreamclient  - demonstration message ping client using TCP streams
#    pingstreamserver  - demonstration message ping server
#    simplefunctionclient - sample RPC client
#    simplefunctionserver - sample RPC server
#    idldeclarationtst      - demonstrates the IDL parsing framework
#
#    clean       - clean out all compiled object and executable files
#    all         - (default target) make sure everything's compiled
#

# Do all C++ compies with g++
CPP = g++

# change following line if your rpgenerate is not in current directory
RPCGEN = ./rpcgenerate

# Where the COMP 150 shared utilities live, including c150ids.a and userports.csv
# Note that environment variable COMP150IDS must be set for this to work!

C150LIB = $(COMP150IDS)/files/c150Utils/
C150AR = $(C150LIB)c150ids.a

# RPC framework is shared archive
C150IDSRPC = $(COMP150IDS)/files/RPC.framework/
C150IDSRPCAR = $(C150IDSRPC)c150idsrpc.a

CPPFLAGS = -g -Wall -Werror -I$(C150IDSRPC) -I$(C150LIB) -std=c++11
# CPPFLAGS = -g -Wall -Werror  -I$(C150LIB)


LDFLAGS = 
INCLUDES = $(C150LIB)c150streamsocket.h $(C150LIB)c150network.h $(C150LIB)c150exceptions.h $(C150LIB)c150debug.h $(C150LIB)c150utility.h $(C150LIB)c150grading.h $(C150IDSRPC)IDLToken.h $(C150IDSRPC)tokenizeddeclarations.h  $(C150IDSRPC)tokenizeddeclaration.h $(C150IDSRPC)declarations.h $(C150IDSRPC)declaration.h $(C150IDSRPC)functiondeclaration.h $(C150IDSRPC)typedeclaration.h $(C150IDSRPC)arg_or_member_declaration.h rpcproxyhelper.h rpcstubhelper.h structs.idl 

all: rpcgenerate structsclient structsserver

structsclient: structsclient.o rpcproxyhelper.o basicTypeHandler.o structs.additionalTypeHandler.o structs.proxy.o  $(C150AR) $(C150IDSRPCAR) $(INCLUDES)
	$(CPP) -o structsclient structsclient.o rpcproxyhelper.o basicTypeHandler.o structs.additionalTypeHandler.o structs.proxy.o  $(C150AR) $(C150IDSRPCAR) 

# The following is NOT a mistake. The main program for any of the rpc servers
# is rpcserver.o.  This way, we can make a different one for each set 
# of functions, by linking the right specific stugs (in this case
# simplefunction.stub.o)
structsserver: structs.stub.o rpcserver.o rpcstubhelper.o basicTypeHandler.o structs.additionalTypeHandler.o structs.o  $(C150AR) $(C150IDSRPCAR) $(INCLUDES)
	$(CPP) -o structsserver rpcserver.o structs.stub.o basicTypeHandler.o structs.additionalTypeHandler.o structs.o rpcstubhelper.o $(C150AR) $(C150IDSRPCAR) 


########################################################################
#
#          Generate C++ source from IDL files
#
#     Once you have written an rpcgenerate program, you can uncomment
#     the following two lines to have .cpp generated automatically from idl.
#     (but be sure not to have other .cpp files with the same base name
#     as the idl, e.g. the ones with your functions!)
#
#     The proxies and stubs used here are hand generated, but eventually
#     your rpcgenerate program will (should) generate them automatically
#     from any idl
#
#     WARNING! this may cause your rpcgenerate program to attempt to
#     rebuild supplied samples like simplefunction.proxy.cpp and 
#     simplefunction.stub.cpp. You may want to save backups for comparison.
#
########################################################################

%.proxy.cpp %.stub.cpp %.additionalTypeHandler.cpp:%.idl $(RPCGEN)
	$(RPCGEN) $<

rpcgenerate: rpcgenerate.o $(C150AR) $(C150IDSRPCAR) $(INCLUDES)
	$(CPP) -o rpcgenerate rpcgenerate.o $(C150AR) $(C150IDSRPCAR)

########################################################################
#
#                   Housekeeping
#
########################################################################


# make .o from .cpp

%.o:%.cpp  $(INCLUDES)
	$(CPP) -c  $(CPPFLAGS) $< 



# clean up everything we build dynamically (probably missing .cpps from .idl)
clean:
	 rm -f rpcgenerate structsclient structsserver *.o



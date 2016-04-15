// --------------------------------------------------------------
//
//                        rpcgenerate.cpp
//
//        Author: Zhaokun Xue         
//   
//        This file builds the program for building proxy and stub
//        
//        COMMAND LINE: 
//              rpcgenerate idlfile1 [idlfile...]*
//
//
//
// --------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdio>
#include <cstring>
#include <string>
#include <algorithm>
#include "c150exceptions.h"
#include "declarations.h"
#include "functiondeclaration.h"
#include "typedeclaration.h"
using namespace std;

// function prototypes
void
generateRPCStub (FILE *stubFile, Declarations parseTree);
void
generateRPCProxy (FILE *proxyFile, Declarations parseTree);
void 
generateStubDispatch (FILE *stubFile, Declarations parseTree);
string 
getStubFunctionPrototype (FunctionDeclaration *functionp);
string
getProxyFunctionPrototype (FunctionDeclaration *functionp);
string 
getActualFunctionCallArgs (FunctionDeclaration *functionp);
string 
getFunctionArgs (FunctionDeclaration *functionp);
void 
generateAdditionalTypeFiles (FILE *additionalTypeHeader, FILE *additionalTypeFunc, Declarations parseTree);
void
structTypeHandler (FILE *additionalTypeHeader, FILE *additionalTypeFunc, TypeDeclaration* typep);
void 
arrayTypeHandler (FILE *additionalTypeHeader, FILE *additionalTypeFunc, TypeDeclaration* typep);
string
buildSendFunction(string sendFunctionName, string arg_or_member, string socket);
string
buildReadFunction (string readFunctionName, string socket);
string
buildReadFunction (string readFunctionName, string arg, string socket);
string
buildArrayFunctionType (TypeDeclaration* typep);
string
buildArrayArgType (TypeDeclaration* typep, string argName);
string 
getSendFunctionName (TypeDeclaration* typep);
string 
getReadFunctionName (TypeDeclaration* typep);
string
getFileBasename (const char *filename);
vector<string> 
&split(const string &s, char delim, vector<string> &elems);
vector<string> 
split(const string &s, char delim);
string 
fileheaders(string fileBasename);
string 
proxyHeaders (string headers);
string 
stubHeaders (string headers);
string
headerFileheaders ();


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                           main program
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
int 
main(int argc, char const *argv[])
{
	int argnum;

	//
  	// Check argument count
  	//
  	if (argc < 2) {
    	fprintf(stderr, "Syntax: %s  idlfile1 [idlfile...]*\n", argv[0]);
    	exit (8);
  	};

	for (argnum = 1; argnum < argc; argnum ++) {
  
		//
		// Open the file
		//
		ifstream idlFile(argv[argnum]);        // open

		if (!idlFile.is_open()) {
			stringstream ss;
			ss << "Could not open IDL file: " << argv[argnum];
			throw C150Exception(ss.str());
		}

		// parse IDL file into parseTree
		Declarations parseTree(idlFile);

		// get IDL file basename
		string fileBasename = getFileBasename(argv[argnum]);
		
		// create output filenames
		// additional helper functions file for handling structs and arrays 
		// which are used in the give IDL file
		string headerFileName = fileBasename + ".additionalTypeHandler.h";
		string functionFileName = fileBasename + ".additionalTypeHandler.cpp";
		// proxy and stub filenames
		string proxy = fileBasename + ".proxy.cpp";
		string stub = fileBasename + ".stub.cpp";

		// generate file headers
		string functionFileHeader = fileheaders(fileBasename);
		functionFileHeader = functionFileHeader + "#include \"" + headerFileName + "\"\n";
		string proxyFileHeaders = proxyHeaders(functionFileHeader);
		string stubFileHeaders = stubHeaders(functionFileHeader);
		string headFileheaders = headerFileheaders();

		// open files
		FILE* additionalTypeHeader = fopen(headerFileName.c_str(), "w+");
		FILE* additionalTypeFunc = fopen(functionFileName.c_str(), "w+");
		FILE* proxyFile = fopen(proxy.c_str(), "w+");
		FILE* stubFile = fopen(stub.c_str(), "w+");

		// write file headers to corresponding files
		fprintf(additionalTypeHeader, "%s\n", headFileheaders.c_str());
		fprintf(additionalTypeFunc, "%s\n", functionFileHeader.c_str());
		fprintf(proxyFile, "%s\n", proxyFileHeaders.c_str());
		fprintf(stubFile, "%s\n", stubFileHeaders.c_str());

		// generate additional function file, proxy file and stub file
		generateAdditionalTypeFiles(additionalTypeHeader, additionalTypeFunc, parseTree);
		generateRPCProxy(proxyFile, parseTree);
		generateRPCStub(stubFile, parseTree);

		fprintf(additionalTypeHeader, "%s\n", "#endif");

		// close files
		fclose(additionalTypeHeader);
		fclose(additionalTypeFunc);
		fclose(proxyFile);
		fclose(stubFile);

  	}
  	return 0;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     generateRPCStub
//
//        Generate .stub.cpp program
//        First, build the function calls for each function.
//        Then, build the dispatchfunction to handle data sent
//        from proxy
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 
void
generateRPCStub (FILE *stubFile, Declarations parseTree) {
	std::map<std::string, FunctionDeclaration*>::iterator fiter;  
	FunctionDeclaration *functionp;
  	string gradinglog = "";
	//
	//  Loop once for each function declaration
	//
	for (fiter = parseTree.functions.begin(); fiter != parseTree.functions.end(); ++fiter) {
		// get actual function declaration structures
		functionp = fiter -> second;
		// get function name
		string functionName = functionp -> getName();
  		// get function return type
  		string functionReturnType = functionp -> getReturnType() -> getName(); 
		// build function prototype which is prefixed with "__"
		string functionPrototye = getStubFunctionPrototype(functionp);
		// write to stub file and start to build the call to actual functions
		fprintf(stubFile, "%s{\n", functionPrototye.c_str());
    	gradinglog = "\t*GRADING << \" STUB: make actual function call for function, " + functionName + "\" << endl;\n";
    	fprintf(stubFile, "%s", gradinglog.c_str());
    	string sendToProxyArg = "";
		string functionCallArgs = getActualFunctionCallArgs (functionp);
		// if function return type is not void, build the return instance
		if (strcmp(functionReturnType.c_str(), "void") != 0){
			string functionCall = functionReturnType + " result = " + functionName + "(" + functionCallArgs + ")";
			fprintf(stubFile, "\t%s;\n", functionCall.c_str());
			sendToProxyArg = ", result";
		}

	  	// write to stub file
   	 	gradinglog = "\t*GRADING << \" STUB: send result back to proxy.\" << endl;\n";
    	fprintf(stubFile, "%s", gradinglog.c_str());

    	// send back the result to proxy
    	string sendToProxy = getSendFunctionName(functionp -> getReturnType()) + " " + "(RPCSTUBSOCKET" + sendToProxyArg + ");";
    	fprintf(stubFile, "\t%s\n", sendToProxy.c_str());

    	gradinglog = "\t*GRADING << \"----------------------------------------\" << endl;\n";
    	fprintf(stubFile, "%s}\n", gradinglog.c_str());
	}
	// function call for bad function name
	string badFunction = "void __badFunction(char *functionName){\n";
  	badFunction += "\t*GRADING << \"STUB: Bad Request Call!!!\" << endl;\n";
	badFunction += "\tchar doneBuffer[5] = \"BAD\";\n";
  	badFunction += "\t*GRADING << \"----------------------------------------\" << endl;\n";
	badFunction += "\tRPCSTUBSOCKET->write(doneBuffer, strlen(doneBuffer)+1);\n}\n";
	fprintf(stubFile, "%s", badFunction.c_str());

	// build the content in dispatchFunction call
	string dispatchFunction = "void dispatchFunction() {\n";
	dispatchFunction += "\tchar functionNameBuffer[50];\n";
	dispatchFunction += "\tstring functionName = readFunctionName(RPCSTUBSOCKET, functionNameBuffer, sizeof(functionNameBuffer));\n";
	dispatchFunction += "\tif (!RPCSTUBSOCKET-> eof()) {\n";
	fprintf(stubFile, "%s", dispatchFunction.c_str());

	// generate the content in dispatchfucntion for handling each function call
	generateStubDispatch (stubFile, parseTree);

	string badFunctionCall = " else {\n \t\t\t__badFunction(functionNameBuffer);\n\t\t}\n";
	fprintf(stubFile, "%s", badFunctionCall.c_str());
	string closedCurlyBrackets = "\t}\n";
	closedCurlyBrackets += "}\n";
	fprintf(stubFile, "%s", closedCurlyBrackets.c_str());
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     generateRPCProxy
//
//        Generate .proxy.cpp program
// 		  For each function call, send function name first.
//		  Then send the content of each argument
//		  to stub one by one in order. Finally, read the return
//        result from stub
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 
void
generateRPCProxy (FILE *proxyFile, Declarations parseTree) {
	unsigned int argnum;
	std::map<std::string, FunctionDeclaration*>::iterator fiter;  
  	FunctionDeclaration *functionp;
  	string gradinglog = "";
	//
	//  Loop once for each function declaration
	//
	for (fiter = parseTree.functions.begin(); fiter != parseTree.functions.end(); ++fiter) {
	    // get actual function declaration structures
	    functionp = fiter -> second;
	    // get function name
	    string functionName = functionp -> getName();
	    // get all arguments vector
	    ArgumentVector& args = functionp -> getArgumentVector();
	    // get return type
	    string functionReturnType = functionp -> getReturnType() -> getName(); 
	    // get function prototype used in proxy file
	    string functionPrototye = getProxyFunctionPrototype(functionp);
	    // write to proxy file
	    fprintf(proxyFile, "%s{\n", functionPrototye.c_str());
	    gradinglog = "\t*GRADING << \" PROXY: start sending request for function, " + functionName + "\" << endl;\n";
	    fprintf(proxyFile, "%s", gradinglog.c_str());
	    // send function name first
	    string sendFunctionNameToSub = "sendFunctionName(RPCPROXYSOCKET, \"" + functionName + "\");";
	    fprintf(proxyFile, "\t%s\n", sendFunctionNameToSub.c_str());

	  	//
	  	// Loop once for each argument
	  	//
	  	for(argnum = 0; argnum < args.size(); argnum++) {
	  		Arg_or_Member_Declaration* argp = args[argnum];
	  		// get arg name
	  		string argName = argp -> getName();
	      	gradinglog = "\t*GRADING << \" PROXY: send function argument, " + argName  + "\" << endl;\n";
	  		fprintf(proxyFile, "%s", gradinglog.c_str());
	      	// send arg to stub
	  		string sendArgToStub = getSendFunctionName(argp -> getType()) + " " + "(RPCPROXYSOCKET, " + argName + ");";
	  		fprintf(proxyFile, "\t%s\n", sendArgToStub.c_str());
	  	}
	  	// read return result from stub
	  	string returnResult = getReadFunctionName(functionp -> getReturnType()) + "(RPCPROXYSOCKET);";
	    gradinglog = "\t*GRADING << \" PROXY: read result from stub for function, " + functionName + "\" << endl;\n";
	    fprintf(proxyFile, "%s", gradinglog.c_str());
	  	// write to proxy file

	    gradinglog = "\t*GRADING << \"----------------------------------------\" << endl;\n";
	    fprintf(proxyFile, "%s", gradinglog.c_str());
	  	fprintf(proxyFile, "\treturn %s\n}\n", returnResult.c_str());
	} 
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     generateStubDispatch
//
//        Generate .stub.cpp program's dispatchFunction() content
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void 
generateStubDispatch (FILE *stubFile, Declarations parseTree) {
	unsigned int argnum;
	std::map<std::string, FunctionDeclaration*>::iterator fiter;  
	FunctionDeclaration *functionp;
  	string gradinglog = "";
	// handle the if, else if switch 
	string ifStatement = "\t\tif";

	//
	//  Loop once for each function declaration
	//
	for (fiter = parseTree.functions.begin(); fiter != parseTree.functions.end(); ++fiter) {
		// get actual function declaration structures
		functionp = fiter -> second;
		// get function name
		string functionName = functionp -> getName();
		// get arguments vector
  		ArgumentVector& args = functionp -> getArgumentVector();
  		// get function return type
  		string functionReturnType = functionp -> getReturnType() -> getName(); 
	  	// start the if statement switch
	  	string compareFunctionName = ifStatement + " (strcmp(functionName.c_str(), \"" + functionName + "\") == 0){\n";
	  	fprintf(stubFile, "%s", compareFunctionName.c_str());
	  
    	gradinglog = "\t\t\t*GRADING << \" STUB: start receiving request for function, " + functionName + "\" << endl;\n\t";
    	fprintf(stubFile, "%s", gradinglog.c_str());
  	
    	// tabs format handler
    	string readFunctiontabs = "\t\t";
    	// 
  		// Loop once for each argument, handler each arg received from proxy
  		//
  		for(argnum = 0; argnum < args.size(); argnum++) {
	  		Arg_or_Member_Declaration* argp = args[argnum];
	  		// get arg name
	  		string argName = argp -> getName();
	  		// get arg type
	  		string argType = argp -> getType() -> getName();
	  		// get function name sent from proxy
	  		string readFunctionName = getReadFunctionName(argp -> getType());
	  		string readFunction = "";
	  		// if arg type is array, init an array instance.
	  		// pass the array instance to read function and pop its values
	  		if (argp -> getType() -> isArray()) {
	  			string arrayArg = buildArrayArgType(argp -> getType(), argName) + ";\n\t\t\t";
	  			readFunction += arrayArg;
	  			readFunction += buildReadFunction(readFunctionName, argName, "RPCSTUBSOCKET");
	  		} else {
	  			readFunction += argType + " " + argName + " = " + buildReadFunction(readFunctionName, "RPCSTUBSOCKET");
	  		}
  			// build the read function
  			readFunction = readFunctiontabs + readFunction;
  			// write to stub file
      		gradinglog = readFunctiontabs + "*GRADING << \" STUB: read argument, " + argName + "\" << endl;\n\t";
      		fprintf(stubFile, "%s", gradinglog.c_str());
  			fprintf(stubFile, "%s", readFunction.c_str());
  		}
  		// make function calls
  		string functionCallArgs = getActualFunctionCallArgs (functionp);
  		string functionCall = "\t\t__" + functionName + "(" + functionCallArgs + ");\n";
  		ifStatement = "else if";
  		fprintf(stubFile, "%s\t\t} ", functionCall.c_str());
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     getStubFunctionPrototype
//
//        Generate function prototype prefix with "__" which is 
//		  used in stub file
//		  e.g. return "void __func1(int x, struct y)"
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 
string 
getStubFunctionPrototype (FunctionDeclaration *functionp) {
	string functionPrototye = "";
	string functionName = functionp -> getName();
 	functionPrototye += "void __" + functionName + "(";
 	string arguments = getFunctionArgs(functionp);
  	functionPrototye += arguments;
  	functionPrototye += ")";

	return functionPrototye;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     getProxyFunctionPrototype
//
//        Generate function prototype used in proxy file
//		  e.g. return "int func1(int x, struct y)"
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

string
getProxyFunctionPrototype (FunctionDeclaration *functionp) {
	string functionPrototye = "";
	string functionName = functionp -> getName();
 	string functionReturnType = functionp -> getReturnType() -> getName(); 
 	functionPrototye += functionReturnType + " " + functionName + "(";
 	string arguments = getFunctionArgs(functionp);
  	functionPrototye += arguments;
  	functionPrototye += ")";

	return functionPrototye;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     getActualFunctionCallArgs
//
//        Make actual fucntion call which does not have
//		  function return's type, arguments' types
//		  e.g. return "func1(x, y);"
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

string 
getActualFunctionCallArgs (FunctionDeclaration *functionp) {
	unsigned int argnum;
	string arguments = "";
  	ArgumentVector& args = functionp -> getArgumentVector();

  	//
  	// loop once for each argument
  	//
 	for(argnum = 0; argnum<args.size(); argnum++) {
  		Arg_or_Member_Declaration* argp = args[argnum];
  		string argName = argp -> getName();
  		string argType = "";
  		// if arg type is array, build array arg
  		if (argp -> getType() -> isArray()) {
  			argType = buildArrayArgType(argp -> getType(), argName);
  		} else {
  			argType = argp -> getType() -> getName();
  		}
  	
  		arguments +=  argName + ", ";
  	}
  	// remove the last ", "
  	arguments = arguments.substr(0, arguments.size() - 2);
  	return arguments;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     getFunctionArgs
//
//        Get all arguments for the given function,
//        which are used in building function prototypes 
//		  e.g. return "int x, struct y"
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

string 
getFunctionArgs (FunctionDeclaration *functionp) {
	unsigned int argnum;
	string arguments = "";
  	ArgumentVector& args = functionp -> getArgumentVector();
  
  	//
  	// loop once for each argument
  	//
 	for(argnum = 0; argnum<args.size(); argnum++) {
  		Arg_or_Member_Declaration* argp = args[argnum];
  		string argName = argp -> getName();
  		string argType = "";
  		// if arg type is array, build array arg
  		if (argp -> getType() -> isArray()) {
  			argType = buildArrayArgType(argp -> getType(), argName);
  			arguments +=  argType + ", ";
  		} else {
  			argType = argp -> getType() -> getName();
  			arguments +=  argType + " " + argName + ", ";
  		}
  	}
  	// remove the last ", "
  	arguments = arguments.substr(0, arguments.size() - 2);
  	return arguments;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     generateAdditionalTypeFiles
//
//        Generate additional helper functions based on given IDL file.
//		  These additional helper functions for handling 
//        send/read structs and arrays defined in IDL file
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void 
generateAdditionalTypeFiles (FILE *additionalTypeHeader, FILE *additionalTypeFunc, Declarations parseTree) {
	std::map<std::string, TypeDeclaration*>::iterator iter;

  	TypeDeclaration *typep;
  
  	//
  	// loop once for each type declaration
  	//
  	for (iter = parseTree.types.begin(); iter != parseTree.types.end(); ++iter) {
    	// get the actual type structs
    	typep = iter->second;
    	// if type is struct, call struct handler
    	if(typep->isStruct()) {
        	structTypeHandler(additionalTypeHeader, additionalTypeFunc, typep);
    	} else if(typep->isArray()) { // else if is array, call array handler
        	arrayTypeHandler(additionalTypeHeader, additionalTypeFunc, typep);
    	}
    	// all other types should be built basic types which are handled in basicTypeHandler
  	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     structTypeHandler
//
//        Build helper function for handling struct by 
//        sending/reading each field separately in order
//        
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void
structTypeHandler (FILE *additionalTypeHeader, FILE *additionalTypeFunc, TypeDeclaration* typep) {
	unsigned memberNum;
	// get type name
	string tyName = typep -> getName();

	// build read/send function name for the given struct
	string readFunctionName = getReadFunctionName(typep);
	string sendFunctionName = getSendFunctionName(typep);

	// build read/send function prototypes
	string readStructPrototype = tyName + " " + readFunctionName + "(C150StreamSocket *socket)";
	string sendStructPrototype = "void " + sendFunctionName + "(C150StreamSocket *socket, " + tyName + " structData)";
	
	// write to handler header file
	fprintf(additionalTypeHeader, "%s;\n", readStructPrototype.c_str());
	fprintf(additionalTypeHeader, "%s;\n", sendStructPrototype.c_str());
	
	// write to handler function file
	string readFunction = readStructPrototype + "{\n\t";
	string sendFunction = sendStructPrototype + "{\n\t";
	readFunction = readFunction + tyName + " result;\n\t";

	vector<Arg_or_Member_Declaration *>& members = typep -> getStructMembers();
	//
	// loop once for each member in the struct
	//
	for(memberNum=0; memberNum < members.size();memberNum++) {
		Arg_or_Member_Declaration* memp = members[memberNum];
		string mempName = memp -> getName();
		string mempType = memp -> getType() -> getName();

		// build send function name for each member
		sendFunctionName = getSendFunctionName(memp -> getType());
		string sendParam = "structData." + mempName;
		// build send function call
		sendFunction += buildSendFunction(sendFunctionName, sendParam, "socket");

		// build read name fucntion for each member
		readFunctionName = getReadFunctionName(memp -> getType());
		
		// if type is array call buildReadFunction for array type
		if (memp -> getType() -> isArray()){
			string arg = "result." + mempName;
			readFunction += buildReadFunction(readFunctionName, arg, "socket");
		}else{
			readFunction += "result." + mempName + " = " + buildReadFunction(readFunctionName, "socket");
		}
	}

	sendFunction += "\n}\n";
	readFunction += "return result;\n}\n";
	// write to handler function file
	fprintf(additionalTypeFunc, "%s\n", sendFunction.c_str());
	fprintf(additionalTypeFunc, "%s\n", readFunction.c_str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     arrayTypeHandler
//
//        Build helper function for handling array by 
//        sending/reading each element separately in order
//        
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void 
arrayTypeHandler (FILE *additionalTypeHeader, FILE *additionalTypeFunc, TypeDeclaration* typep) {
	// get type name
	string tyName = typep -> getName();
	
	// build read/send function names for array type
	string readFunctionName = getReadFunctionName(typep);
	string sendFunctionName = getSendFunctionName(typep);

	// build the array arg e.g. arrayArg[2][3][4]
	string arrayArgName = "arrayArg";
	string arrayArg = buildArrayArgType(typep, arrayArgName);

	// build read/send function call for array type
	string readArrayPrototype = "void " + readFunctionName + "(C150StreamSocket *socket, " + arrayArg + ")";
	string sendArrayPrototype = "void " + sendFunctionName + "(C150StreamSocket *socket, " + arrayArg + ")";
	
	// write to handler header file
	fprintf(additionalTypeHeader, "%s;\n", readArrayPrototype.c_str());
	fprintf(additionalTypeHeader, "%s;\n", sendArrayPrototype.c_str());
	
	// write to handler function file
	string readFunction = readArrayPrototype + "{\n";
	string sendFunction = sendArrayPrototype + "{\n";

	// make a temp variable for iterate array
	TypeDeclaration* temp = typep;
	int loop_counter = 0;
	string readItemIter = "";
	string sendItemIter = "";
	string tabs = "\t";
	string closedCurlyBrackets = "";
	// build the for loop for iterate the array element
	while (temp -> isArray()) {
		int bound = temp -> getArrayBound();
		closedCurlyBrackets = tabs + "}\n" + closedCurlyBrackets;
		string iterVar = "i_" + to_string(loop_counter);
		readFunction = readFunction + tabs + "for(int " + iterVar + 
						" = 0; " + iterVar + " < " + to_string(bound) + 
						"; " + iterVar + "++){\n";

		sendFunction = sendFunction + tabs + "for(int " + iterVar + 
						" = 0; " + iterVar + " < " + to_string(bound) + 
						"; " + iterVar + "++){\n";
		readItemIter = readItemIter + "[" + iterVar + "]";
		sendItemIter = sendItemIter + "[" + iterVar + "]";
		temp = temp -> getArrayMemberType();
		loop_counter++;
		tabs += "\t";
	}
	readItemIter = "arrayArg" + readItemIter;
	sendItemIter = "arrayArg" + sendItemIter;

	// build read/send functoin name for array's element
	string readArrayElementName = getReadFunctionName(temp);
	string sendArrayElementName = getSendFunctionName(temp);
	// build the read/send function call for array's element
	readFunction +=  tabs + readItemIter + " = " + buildReadFunction(readArrayElementName, "socket");
	sendFunction +=  tabs + buildSendFunction(sendArrayElementName, sendItemIter, "socket");
	
	// erase one tab 
	closedCurlyBrackets.erase(0,1);
	
	readFunction = readFunction + closedCurlyBrackets + "\n";
	sendFunction = sendFunction + closedCurlyBrackets + "\n";

	// write to addtional types handler file
	fprintf(additionalTypeFunc, "%s}\n\n", readFunction.c_str());
	fprintf(additionalTypeFunc, "%s}\n\n", sendFunction.c_str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     buildSendFunction
//
//        Construct the "send" function
//        e.g. sendstringType(C150StreamSocket *socket, string s);
//			   sendStruct_rectangle(C150StreamSocket *socket, rectangle r);
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

string
buildSendFunction(string sendFunctionName, string arg_or_member, string socket) {
	string sendFunction = sendFunctionName + "(";
	if (sendFunction == "sendvoidType") {
		sendFunction += socket + ");\n\t";
	} else {
		sendFunction += socket + ", " + arg_or_member + ");\n\t";
	}
	return sendFunction;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     buildReadFunction
//
//        Construct the "read" function which is for non-array type
//        e.g. readstringType(C150StreamSocket *socket);
//			   readStruct_rectangle(C150StreamSocket *socket);
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

string
buildReadFunction (string readFunctionName, string socket){
	string readFunction = readFunctionName + "(" + socket +");\n\t";
	return readFunction;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     buildReadFunction
//
//        Construct the "read" function which is for array type
//        e.g. readArray_rectangle_200(C150StreamSocket *socket, rectangle arrayArg[200]);
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

string
buildReadFunction (string readFunctionName, string arg, string socket){
	string readFunction = readFunctionName + "(" + socket + ", " + arg +");\n\t";
	return readFunction;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     buildArrayFunctionType
//
//        Construct format for part of array type function name
//		  e.g. return int_10_20
//        e.g. array help function name: sendArray_int_10_20
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

string
buildArrayFunctionType (TypeDeclaration* typep) {
	string tyName = "";
	TypeDeclaration* temp = typep;
	while (temp -> isArray()) {
		int bound = temp -> getArrayBound();
		tyName += "_" + to_string(bound);
		temp = temp -> getArrayMemberType();
	}
	tyName = temp -> getName() + tyName;
	return tyName;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     buildArrayArgType
//
//        Construct format for array type argument
//		  e.g. return int arrayArg[10][20]
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

string
buildArrayArgType (TypeDeclaration* typep, string argName) {
	string tyName = "";
	TypeDeclaration* temp = typep;
	while (temp -> isArray()) {
		int bound = temp -> getArrayBound();
		tyName += "[" + to_string(bound) + "]";
		temp = temp -> getArrayMemberType();
	}
	tyName = temp -> getName() + " " + argName + tyName;
	return tyName;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     getSendFunctionName
//
//        Construct "send" function's name based on different type
//		  e.g. return sendstringType/sendStruct_Person/sendArray_int_10
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

string 
getSendFunctionName (TypeDeclaration* typep) {
	string sendFunctionName = "send";
	string tyName = typep -> getName();
	if (typep -> isStruct()) {
		sendFunctionName += "Struct_" + tyName;
	} else if (typep -> isArray()) {
		tyName = buildArrayFunctionType(typep);
		sendFunctionName += "Array_" + tyName;
	} else {
		sendFunctionName += tyName + "Type";
	}
	return sendFunctionName;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     getReadFunctionName
//
//        Construct "read" function's name based on different type
//		  e.g. return readstringType/readStruct_Person/readArray_int_10
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

string 
getReadFunctionName (TypeDeclaration* typep) {
	string readFunctionName = "read";
	string tyName = typep -> getName();
	if (typep -> isStruct()) {
		readFunctionName += "Struct_" + tyName;
	} else if (typep -> isArray()) {
		tyName = buildArrayFunctionType(typep);
		readFunctionName += "Array_" + tyName;
	} else {
		readFunctionName += tyName + "Type";
	}
	return readFunctionName;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     getFileBasename
//
//			Get the base name for the given IDL file       
//			e.g. return structs for input structs.idl
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

string 
getFileBasename (const char *filename) {
	string stringFilename(filename);
 	vector<string> tokens = split(stringFilename, '.');
 	if(tokens.at(1) != "idl"){
 		fprintf(stderr, "Input file type should be .idl, wrong file type: %s\n", filename);
    	exit (0);
    }
	return tokens.at(0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     split
//		helper function for splitting string
//	    **REFERENCE from StackOverFlow**
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

vector<string> 
&split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}
vector<string> 
split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     fileheaders
//		build include headers for files
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

string 
fileheaders (string fileBasename) {
  string header = "";
  header.append("#include \"c150debug.h\"\n")
        .append("#include <fstream>\n")
        .append("#include <cstdio>\n")
     	  .append("#include <cstring>\n")
     	  .append("#include <string>\n")
     	  .append("#include <fstream>\n")
     	  .append("#include <arpa/inet.h>\n") 
     	  .append("using namespace std;\n")
     	  .append("using namespace C150NETWORK;\n")
     	  .append("#include \"" + fileBasename + ".idl\"\n")
     	  .append("#include \"basicTypeHandler.h\"\n");
  return header;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     proxyHeaders
//		append rpcproxyhelper.h to fileheaders
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

string 
proxyHeaders (string headers) {
	return headers.append("#include \"rpcproxyhelper.h\"\n");
} 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     stubHeaders
//		append rpcstubhelper.h to fileheaders
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

string 
stubHeaders (string headers) {
	return headers.append("#include \"rpcstubhelper.h\"\n");
} 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     headerFileheaders
//		build include headers for .h files
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

string
headerFileheaders () {
	string header = "";
	header.append("#ifndef __ADDITIONALTYPE_H_INCLUDED__\n")
		  .append("#define __ADDITIONALTYPE_H_INCLUDED__\n")
		  .append("#include <string>\n")
		  .append("#include \"c150streamsocket.h\"\n")
       	  .append("using namespace C150NETWORK;\n")
       	  .append("using namespace std;\n");
    return header;
}
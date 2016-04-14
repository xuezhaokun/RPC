#ifndef __ATOMICSOCKETUTILS_H__
#define __ATOMICSOCKETUTILS_H__

#include <string>
#include "c150streamsocket.h"

using namespace C150NETWORK;
using namespace std;

void sendstringType (C150StreamSocket *socket, string stringData);
string readstringType (C150StreamSocket *socket);

void sendintType (C150StreamSocket *socket, int intData);
int readintType (C150StreamSocket *socket);

void sendvoidType (C150StreamSocket *socket);
void readvoidType (C150StreamSocket *socket);

void sendfloatType (C150StreamSocket *socket, float floatData);
float readfloatType (C150StreamSocket *socket);

void sendFunctionName (C150StreamSocket *socket, const char  *functionName);
string readFunctionName(C150StreamSocket *socket, char *buffer, unsigned int bufSize);
#endif
// --------------------------------------------------------------
//
//                        strcuts.cpp
//
//        Author: Noah Mendelsohn         
//   
//
//        Trivial implementations of the routines declared
//        in structs.idl. These are for testing: they
//        just print messages.
//
//       Copyright: 2012 Noah Mendelsohn
//     
// --------------------------------------------------------------

// IMPORTANT! WE INCLUDE THE IDL FILE AS IT DEFINES THE INTERFACES
// TO THE FUNCTIONS WE'RE IMPLEMENTING. THIS MAKES SURE THE
// CODE HERE ACTUALLY MATCHES THE REMOTED INTERFACE

#include <string>
#include <cstring>
#include <iostream>
using namespace std;          // for C++ std library
#include "structs.idl"
#include "c150debug.h"

Person findPerson(ThreePeople tp){
	return tp.p1;
}

int area(rectangle r){
	return (r.x * r.y);
}

float multiply(float a, float b, float c, float d, float e) {
  return a*b*c*d*e;
}

float divide(float x, float y) {
  return x/y;
}

int add(int x, int y, int z) {
  return x+y+z;
}

int subtract(int x, int y) {
  return x-y;
}

rectangle searchRectangles(rectangle rects[4]) {
	return rects[2];
}

int sumSm(sm stest) {
	int sum = 0;
	for (int i = 0; i < 10000; i++){
		sum += stest.m1[i];
	}
	return sum;
}

int findSecondPersonAge(StructWithArrays persons){
	//Person p2 = persons.people[1];
	return persons.people[1].age;
}

void func1() {
  printf("func1() invoked\n");
  c150debug->printf(C150RPCDEBUG,"simplefunction.cpp: func1() invoked");
}

void func2() {
  printf("func2() invoked\n");				  
  c150debug->printf(C150RPCDEBUG,"simplefunction.cpp: func2() invoked");
}

void func3() {
  printf("func3() invoked\n");
  c150debug->printf(C150RPCDEBUG,"simplefunction.cpp: func3() invoked");
}
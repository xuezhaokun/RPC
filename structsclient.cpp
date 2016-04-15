// --------------------------------------------------------------
//
//                        structsclient.cpp
//
//        Author: Noah Mendelsohn         
//   
//
//        This is a test program designed to call a few demonstration
//        functions, after first enabling the COMP 150-IDS rpcproxyhelper.
//        (The purpose of the helper is to open a TCP stream connection
//        to the proper server, and to leave the socket pointer where
//        the generated proxies can find it.
//
//        NOTE: Although this example does nothing except test the
//        functions, we may test your proxies and stubs with client
//        applications that do real work. 
//
//        NOTE: When actually testing your RPC submission, you will use
//        a different client application for each set of functions. This
//        one is just to show a simple example.
//
//        NOTE: The only thing that makes this different from 
//        an ordinary local application is the call to
//        rpcproxyinitialize. If you commented that out, you could
//        link this with the local version of simplefunction.o
//        (which has the remotable function implementations)            
//
//        COMMAND LINE
//
//              structsclient <servername> 
//
//        OPERATION
//
//
//       Copyright: 2012 Noah Mendelsohn
//     
// --------------------------------------------------------------


// IMPORTANT! WE INCLUDE THE IDL FILE AS IT DEFINES THE INTERFACES
// TO THE FUNCTIONS WE'RE REMOTING. OF COURSE, THE PARTICULAR IDL FILE
// IS CHOSEN ACCORDING TO THE TEST OR APPLICATION
// 
// NOTE THAT THIS IS THE SAME IDL FILE INCLUDED WITH THE PROXIES
// AND STUBS, AND ALSO USED AS INPUT TO AUTOMATIC PROXY/STUB
// GENERATOR PROGRAM


#include "rpcproxyhelper.h"
#include "c150debug.h"
#include "c150grading.h"

#include <cstring>
#include <string>

using namespace std;          // for C++ std library
using namespace C150NETWORK;  // for all the comp150 utilities 
#include "structs.idl"
// forward declarations
void setUpDebugLogging(const char *logname, int argc, char *argv[]);


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                    Command line arguments
//
// The following are used as subscripts to argv, the command line arguments
// If we want to change the command line syntax, doing this
// symbolically makes it a bit easier.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

const int serverArg = 1;     // server name is 1st arg


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                           main program
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 
int 
main(int argc, char *argv[]) {

     //
     //  Set up debug message logging
     //
     setUpDebugLogging("simplefunctionclientdebug.txt",argc, argv);

     //
     // Make sure command line looks right
     //
     if (argc != 2) {
       fprintf(stderr,"Correct syntxt is: %s <servername> \n", argv[0]);
       exit(1);
     }

     //
     //  DO THIS FIRST OR YOUR ASSIGNMENT WON'T BE GRADED!
     //
     
     GRADEME(argc, argv);

     //
     //     Call the functions and see if they return
     //
     try {
       //float result; 
       Person p1;
       Person p2;
       Person p3;

       p1.firstname = string("john");
       p1.lastname = string("peter");
       p1.age = 20;
       cout << "p1 firstname: " << p1.firstname << " lastname: " << p1.lastname << " age: " << p1.age << endl;
       p2.firstname = string("hi");
       p2.lastname = string("hello");
       p2.age = 25;
       cout << "p2 firstname: " << p2.firstname << " lastname: " << p2.lastname << " age: " << p2.age << endl;

       p3.firstname = string("yes");
       p3.lastname = string("no");
       p3.age = 26;
       cout << "p3 firstname: " << p3.firstname << " lastname: " << p3.lastname << " age: " << p3.age << endl;

       ThreePeople tp;
       tp.p1 = p1;
       tp.p2 = p2;
       tp.p3 = p3;

       rectangle r;
       r.x = 3;
       r.y = 5;
       //
       // Set up the socket so the proxies can find it
       //
       rpcproxyinitialize(argv[serverArg]);

       // 
       // Call (possibly remote) add
       //
       printf("Calling findPerson\n");
       Person p = findPerson (tp);                          // remote call (we hope!)
       cout << "p's firstname is " << p.age << endl;

       // 
       // Call (possibly remote) subtract
       //
       printf("Calling area\n");
       int result = area(r);                          // remote call (we hope!)
       printf("Returned from area. Result=%d\n",result);
       // 
       // Call (possibly remote) multiply
       //
       printf("Calling multiply(10.0,3.5)\n");
       float f = multiply(10.0, 3.5, 2.3, 3.14, 1.6);                          // remote call (we hope!)
       printf("Returned from multiply(10.0, 3.5, 2.3, 3.14, 1.6). Result=%f\n",f);

       // 
       // Call (possibly remote) divide
       //
       printf("Calling divide(10.0,3.5)\n");
       f = divide(10.0,3.5);                          // remote call (we hope!)
       printf("Returned from divide(10.0,3.5). Result=%f\n",f);


       // 
       // Call (possibly remote) add
       //
       printf("Calling add(10,2,20)\n");
       int intResult = add(10,2,20);                          // remote call (we hope!)
       printf("Returned from add(10,2). Result=%d\n",intResult);

       // 
       // Call (possibly remote) subtract
       //
       printf("Calling subtract(10,2)\n");
       intResult = subtract(10,2);                          // remote call (we hope!)
       printf("Returned from subtract(10,2). Result=%d\n",intResult);

       //
       // Call (possibly remote) searchRectangle 
       //
       rectangle r1, r2, r3, r4;
       r1.x = 3;
       r1.y = 5;
       r2.x = 4;
       r2.y = 6;
       r3.x = 7;
       r3.y = 8;
       r4.x = 10;
       r4.y = 12;
       rectangle r_search [4] = { r1, r2, r3, r4 }; 

       rectangle r_result = searchRectangles(r_search);;
       printf("Returned from searchRectangle(r_search). Result.x=%d; Result.y=%d\n",r_result.x,r_result.y);

       //
       // Call (possibly remote) sumM1 
       //

       sm s1_test;
       for (int i = 0; i < 10000; i++){
          s1_test.m1[i] = i;
       }
       
       int sum_m1 = sumSm(s1_test);
       printf("Returned from sumM1(s1_test). Result=%d\n",sum_m1);


       //
       // Call (possibly remote) sumM1 
       //

       StructWithArrays persons;
       persons.people[0] = p1;
       persons.people[1] = p2;
       persons.people[2] = p3;
       persons.aNumber = 3;

       int personAge = findSecondPersonAge(persons);
       printf("Returned from findSecondPersonAge(s1_test). Result=%d\n",personAge);

       printf("Calling func1()\n");
       func1();                          // remote call (we hope!)
       printf("Returned from func1()\n");

       // 
       // Call (possibly remote) func2
       //
       printf("Calling func2()\n");
       func2();                          // remote call (we hope!)
       printf("Returned from func2()\n");

       // 
       // Call (possibly remote) func3
       //
       printf("Calling func3()\n");
       func3();                          // remote call (we hope!)
       printf("Returned from func3()\n");

     }

     //
     //  Handle networking errors -- for now, just print message and give up!
     //
     catch (C150Exception e) {
       // Write to debug log
       c150debug->printf(C150ALWAYSLOG,"Caught C150Exception: %s\n",
       e.formattedExplanation().c_str());
       // In case we're logging to a file, write to the console too
       cerr << argv[0] << ": caught C150NetworkException: " << e.formattedExplanation() << endl;
     }

     return 0;
}



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                     setUpDebugLogging
//
//        For COMP 150-IDS, a set of standards utilities
//        are provided for logging timestamped debug messages.
//        You can use them to write your own messages, but 
//        more importantly, the communication libraries provided
//        to you will write into the same logs.
//
//        As shown below, you can use the enableLogging
//        method to choose which classes of messages will show up:
//        You may want to turn on a lot for some debugging, then
//        turn off some when it gets too noisy and your core code is
//        working. You can also make up and use your own flags
//        to create different classes of debug output within your
//        application code
//
//        NEEDSWORK: should be factored into shared code w/pingstreamserver
//        NEEDSWORK: document arguments
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 
void setUpDebugLogging(const char *logname, int argc, char *argv[]) {

     //   
     //           Choose where debug output should go
     //
     // The default is that debug output goes to cerr.
     //
     // Uncomment the following three lines to direct
     // debug output to a file. Comment them
     // to default to the console.
     //
     // Note: the new DebugStream and ofstream MUST live after we return
     // from setUpDebugLogging, so we have to allocate
     // them dynamically.
     //
     //
     // Explanation: 
     // 
     //     The first line is ordinary C++ to open a file
     //     as an output stream.
     //
     //     The second line wraps that will all the services
     //     of a comp 150-IDS debug stream, and names that filestreamp.
     //
     //     The third line replaces the global variable c150debug
     //     and sets it to point to the new debugstream. Since c150debug
     //     is what all the c150 debug routines use to find the debug stream,
     //     you've now effectively overridden the default.
     //
     ofstream *outstreamp = new ofstream(logname);
     DebugStream *filestreamp = new DebugStream(outstreamp);
     DebugStream::setDefaultLogger(filestreamp);

     //
     //  Put the program name and a timestamp on each line of the debug log.
     //
     c150debug->setPrefix(argv[0]);
     c150debug->enableTimestamp(); 

     //
     // Ask to receive all classes of debug message
     //
     // See c150debug.h for other classes you can enable. To get more than
     // one class, you can or (|) the flags together and pass the combined
     // mask to c150debug -> enableLogging 
     //
     // By the way, the default is to disable all output except for
     // messages written with the C150ALWAYSLOG flag. Those are typically
     // used only for things like fatal errors. So, the default is
     // for the system to run quietly without producing debug output.
     //
     c150debug->enableLogging(C150ALLDEBUG | C150RPCDEBUG | C150APPLICATION | C150NETWORKTRAFFIC | 
            C150NETWORKDELIVERY); 
}

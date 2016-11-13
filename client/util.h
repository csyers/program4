#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <netdb.h>
#include <unistd.h>

using namespace std;

void print_usage_and_exit(){
    cout << "usage: myfrm server port" << endl;
    exit(1);
}

void print_error_and_exit(string message){
    cerr << "myfrm: " << message << endl;
    //exit(1);
}

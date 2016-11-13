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

void print_error_and_exit(string message, int s_udp, int s_tcp){
    cerr << "myfrm: " << message << endl;
    if(s_udp > 0){
        close(s_udp);
    }
    if(s_tcp > 0){
        close(s_tcp);
    }
    exit(1);
}

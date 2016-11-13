#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <netdb.h>
#include <unistd.h>

using namespace std;

void print_usage_and_exit(){
    cout << "usage: myfrmd port password" << endl;
    exit(1);
}

void print_error_and_exit(string message, int s_tcp, int s_udp){
    cerr << "myfrmd: " << message << endl;
    if(s_tcp > 0){
        close(s_tcp);
    }
    if(s_udp > 0){
        close(s_udp);
    }
    exit(1);
}

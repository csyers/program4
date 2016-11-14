#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <netdb.h>
#include <unistd.h>

#define MAX_LENGTH 4096

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

int send_string_udp(string message, int s, struct sockaddr_in &sin) {
    char buf[MAX_LENGTH];
    bzero(buf, sizeof(buf));
    strcpy(buf, message.c_str());
    int msg_len = strlen(buf) + 1;
    int bytes_sent;

    if((bytes_sent=sendto(s, buf, msg_len, 0, (struct sockaddr *) &sin, sizeof(sin))) < 0) {
        return -1;
    } else {
        return bytes_sent;
    }
}

int recv_string_udp(string &resp, int s_udp, struct sockaddr_in &sin) {
    char buf[MAX_LENGTH];
    bzero(buf, sizeof(buf));
    int bytes_received;
    socklen_t addr_len = sizeof(sin);

    if ((bytes_received=recvfrom(s_udp, buf, sizeof(buf), 0, (struct sockaddr *) &sin, &addr_len)) < 0) {
        return -1;
    } else {
        string temp(buf);
        resp = temp;
        return bytes_received;
    }
}


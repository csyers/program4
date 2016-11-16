#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <netdb.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <cstdio>
#include <fstream>
#include <algorithm>
#include <map>

#define MAX_LENGTH 4096

using namespace std;

struct bi {
    string creator;
    int line;
    fstream * os;

    bi() {
        creator = "";
        line = 0;
        os = 0;
    }

};

int recreate_file(unordered_map<string, bi> board_info, unordered_map<string, map<int,pair<string,string>>> board_contents, string board_name){
    // close and delete
    board_info[board_name].os->close();
    delete board_info[board_name].os;
    board_info[board_name].os = 0;

    // remove old file
    remove(board_name.c_str());

    // mkae new os
    board_info[board_name].os = new fstream(board_name, ios::in|ios::out|ios::app);

    if(!board_info[board_name].os || !*board_info[board_name].os) {
        return 0;
    }

    *board_info[board_name].os << board_info[board_name].creator << endl;

    for (auto const it: board_contents[board_name]){
        *board_info[board_name].os << it.first << " " << it.second.first << ": " << it.second.second << endl;
    }

    return 1;
}

void close_fp(unordered_map<string, bi> board_info){
    for (auto &it: board_info) {
        it.second.os->close();
        delete it.second.os;
        it.second.os = 0;
    }
}

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

int send_string_udp(string message, int s, struct sockaddr_in &sin) {
    char buf[MAX_LENGTH];
    bzero(buf, sizeof(buf));
    strcpy(buf, message.c_str());
    int msg_len = strlen(buf) ;
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

int recv_string_tcp(string &resp, int s_new) {
    char buf[MAX_LENGTH];
    bzero(buf, sizeof(buf));
    int bytes_received;

    if ((bytes_received=recv(s_new, buf, sizeof(buf),0))==-1) {
        return -1;
    } else {
        string temp(buf);
        resp = temp;
        return bytes_received;
    }
}

int send_string_tcp(string message, int s_new) {
    char buf[MAX_LENGTH];
    bzero(buf, sizeof(buf));
    strcpy(buf, message.c_str());
    int msg_len = strlen(buf) ;
    int bytes_sent;

    if ((bytes_sent=send(s_new, &buf, msg_len, 0)) < 0) {
        return -1;
    } else {
        return bytes_sent;
    }
}

int send_int_tcp(int msg, int s_new) {
    int bytes_sent;

    msg = htonl(msg);
    if ((bytes_sent=send(s_new, &msg, sizeof(msg), 0))==-1) {
        return -1;
    } else {
        return bytes_sent;
    }
}

int recv_int_tcp(int &msg, int s_new) {
    int bytes_recv;

    if ((bytes_recv=recv(s_new, &msg, sizeof(int), 0))==-1) {
        return -1;
    } else {
        msg = ntohl(msg);
        return bytes_recv;
    }
}

int recv_file_tcp(string &resp, int s_tcp) {
    int bytes_recv;
    char buf[MAX_LENGTH];
    bzero(buf, sizeof(buf));

    if ((bytes_recv=recv(s_tcp, buf, sizeof(buf), 0))==-1) {
        return -1;
    } 

    string temp(buf);
    resp = temp;
    return bytes_recv;
}

int send_file_tcp(fstream &os, int s_new) {
    char buf[MAX_LENGTH];
    int bytes_sent = 0;
    int nsent;
    streampos pos = os.tellg();
    os.seekg(0, os.beg);

    while (!os.eof()) {
        bzero(buf, sizeof(buf));
        os.read(buf, MAX_LENGTH);
        if(!os.eof() && os.fail()) {
            cout << "fail to read" << endl;
            return -1;
        }
        if ((nsent=send(s_new, &buf, strlen(buf), 0)) < 0) {
            return -1;
        } else {
            bytes_sent += nsent;
        }
    }
    os.clear();
    os.seekg(pos);
    return bytes_sent;
}

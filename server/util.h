/* Name: Tim Chang (tchang2), Christopher Syers (csyers)
 * Date: November 1, 2016
 * CSE 30264: Computer Networks
 * Programming Assignment 4: Program a Prototype of a Message Board Forum application
 */

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

// board info struct with info about creator, the next line to be written, and a stream to write to it
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

    // write the creator to the file
    *board_info[board_name].os << board_info[board_name].creator << endl;

    // write each line in the contents to the file
    for (auto const it: board_contents[board_name]){
        *board_info[board_name].os << it.first << " " << it.second.first << ": " << it.second.second << endl;
    }

    return 1;
}

// close all streams that may be open 
void close_fp(unordered_map<string, bi> board_info){
    for (auto &it: board_info) {
        it.second.os->close();
        delete it.second.os;
        it.second.os = 0;
    }
}

// print usage message and exit the program
void print_usage_and_exit(){
    cout << "usage: myfrmd port password" << endl;
    exit(1);
}

// print erorr message, perform cleanup, and exit the program
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

// send a string over a udp link
int send_string_udp(string message, int s, struct sockaddr_in &sin) {
    // local varialbes needed for sending
    char buf[MAX_LENGTH];
    bzero(buf, sizeof(buf));
    strcpy(buf, message.c_str());
    int msg_len = strlen(buf) ;
    int bytes_sent;

    /// send the message to the sin address
    if((bytes_sent=sendto(s, buf, msg_len, 0, (struct sockaddr *) &sin, sizeof(sin))) < 0) {
        // if error, return -1
        return -1;
    } else {
        // else, return the number of bytes sent
        return bytes_sent;
    }
}

// receive a string over a udp link
int recv_string_udp(string &resp, int s_udp, struct sockaddr_in &sin) {
    // local variables needed for receiving
    char buf[MAX_LENGTH];
    bzero(buf, sizeof(buf));
    int bytes_received;
    socklen_t addr_len = sizeof(sin);

    // receive the message from the s_udp port
    if ((bytes_received=recvfrom(s_udp, buf, sizeof(buf), 0, (struct sockaddr *) &sin, &addr_len)) < 0) {
    // receive the message from the s_udp port
        return -1;
    } else {
        // else, populate response string and returns the number of bytes received
        string temp(buf);
        resp = temp;
        return bytes_received;
    }
}

// receive a string over a tcp link
int recv_string_tcp(string &resp, int s_new) {
    // local variables needed
    char buf[MAX_LENGTH];
    bzero(buf, sizeof(buf));
    int bytes_received;

    // receive from the tcp port
    if ((bytes_received=recv(s_new, buf, sizeof(buf),0))==-1) {
        return -1;
    } else {
        // populate response string and return bytes received
        string temp(buf);
        resp = temp;
        return bytes_received;
    }
}

// send a string over a tcp link
int send_string_tcp(string message, int s_new) {
    // local variables
    char buf[MAX_LENGTH];
    bzero(buf, sizeof(buf));
    strcpy(buf, message.c_str());
    int msg_len = strlen(buf) ;
    int bytes_sent;

    // send the string, return -1 on error
    if ((bytes_sent=send(s_new, &buf, msg_len, 0)) < 0) {
        return -1;
    } else {
        return bytes_sent;
    }
}

// send an integer over a tcp link
int send_int_tcp(int msg, int s_new) {
    int bytes_sent;

    // convert to network endianess
    msg = htonl(msg);
    // send the integer, return -1 on error
    if ((bytes_sent=send(s_new, &msg, sizeof(msg), 0))==-1) {
        return -1;
    } else {
        return bytes_sent;
    }
}

// recieve an integer over a tcp link
int recv_int_tcp(int &msg, int s_new) {
    int bytes_recv;

    // receive the integer, return -1 on error
    if ((bytes_recv=recv(s_new, &msg, sizeof(int), 0))==-1) {
        return -1;
    } else {
        msg = ntohl(msg);
        return bytes_recv;
    }
}

// receive file from tcp link
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

// send file over tcp link
int send_file_tcp1(fstream &os, int s_new) {
    char buf[MAX_LENGTH];
    int bytes_sent = 0;
    int nsent;
    streampos pos = os.tellg();
    os.seekg(0, os.beg);
    
    // while there are still charcters to read,
    while (!os.eof()) {
        bzero(buf, sizeof(buf));
        os.read(buf, MAX_LENGTH);
        if(!os.eof() && os.fail()) {
            cout << "fail to read" << endl;
            return -1;
        }
        // send a piece of the file
        if ((nsent=send(s_new, &buf, strlen(buf), 0)) < 0) {
            return -1;
        } else {
            bytes_sent += nsent;
        }
    }
    // reset stream
    os.clear();
    os.seekg(pos);
    return bytes_sent;
}

int send_file_tcp(string filename, int s_new) {
    char buf[MAX_LENGTH];
    int bytes_sent = 0;
    int nsent;
    FILE *fp;

    if (!(fp = fopen(filename.c_str(), "r"))) {
        return -1;
    }

    while(1) {
    bzero((char *)buf, sizeof(buf));
    // read some bytes from the file
    int nred = fread(buf, 1, MAX_LENGTH, fp);
    buf[nred] = '\0';

    // if some bytes were read, send them to the client
    if (nred > 0) {
        if((nsent=send(s_new, &buf, nred, 0))==-1)
        {
            return -1;
        }
        bytes_sent += nsent;
    }
    // if fewer than MAX_LINE bytes were read, there could be an error
    if (nred < MAX_LENGTH) {
        // if there was an error, exit
        // else break out of the while look
        if(ferror(fp)) {
            return -1;
        }
        break;
    }
    }
    return bytes_sent;
}

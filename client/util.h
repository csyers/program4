#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <netdb.h>
#include <unistd.h>

#define MAX_LENGTH 4096

using namespace std;

bool has_only_spaces(const std::string& str) {
   return str.find_first_not_of (" \t\n") == str.npos;
}

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

    if((bytes_sent=send(s_new, &buf, msg_len, 0)) < 0) {
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

int send_file_tcp1(fstream os, int s_new) {
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
            cout << "adding: " << nsent << endl;
            bytes_sent += nsent;
        }
    }
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

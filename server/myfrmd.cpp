/* Name: Tim Chang (tchang2), Christopher Syers (csyers)
 * Date: November 1, 2016
 * CSE 30264: Computer Networks
 * Programming Assignment 4: Program a Prototype of a Message Board Forum application
 */

#include "util.h"

#define MAX_PENDING 5

void print_usage_and_exit();
void print_error_and_exit(string, int, int);

int main(int argc, char* argv[]){
    string port_temp, password;
    int port;
    int s_udp = -1;
    int s_tcp = -1;

    // check the nubmer of agurmnets
    if(argc != 3){
        // if there are the wrong number of arguments, exit
        print_usage_and_exit();
    } else{
        // get the port number and the admin password
        port_temp = argv[1];
        password = argv[2];
        // check if the port only contains digits
        bool has_only_digits = (port_temp.find_first_not_of( "0123456789" ) == string::npos);
        if(has_only_digits) {
            // set port variable
            port = atoi(port_temp.c_str());
        } else{
            print_error_and_exit("port argument must be postiive integer", s_udp, s_tcp);
        }
    }

    // local variables for socket programming
    struct sockaddr_in sin;
    int s_new;
    int opt = 1;
    int len = sizeof(sin);

    //etup the sockaddr_in
    bzero((char *)&sin,sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = port;

    // open a tcp socket, exit if there is an error
    if((s_tcp=socket(PF_INET,SOCK_STREAM,0)) < 0){
        print_error_and_exit("error in tcp socket creation", s_udp, s_tcp);
    }

    // open a udp socket, exit if there is an error
    if((s_udp=socket(PF_INET,SOCK_DGRAM,0)) < 0){
        print_error_and_exit("error in udp socket creation", s_udp, s_tcp);
    }

    // allow socket to be reused after a closed connection, exit on error
    if((setsockopt(s_tcp,SOL_SOCKET,SO_REUSEADDR,(char *)&opt,sizeof(int)))<0){
        print_error_and_exit("error in setsocketopt", s_udp, s_tcp);
    }

    // bind the tcp socket to the port passed in on the command line
    if((bind(s_tcp,(struct sockaddr *)&sin,sizeof(sin)))<0){
        print_error_and_exit("error in bind", s_udp, s_tcp);
    }   

    // bind the udp socket to the port passed in on the command line
    if((bind(s_udp,(struct sockaddr *)&sin,sizeof(sin)))<0){
        print_error_and_exit("error in bind", s_udp, s_tcp);
    }   

    // begin listening for client connections on the socket
    if((listen(s_tcp,MAX_PENDING))<0)
    {
        print_error_and_exit("error in listen", s_udp, s_tcp);
    }

    // enter infinte loop wating for connections
    while(1){
        // accept a client connection on a new socket (s_new), exit on error
        if((s_new=accept(s_tcp,(struct sockaddr *)&sin,(socklen_t *)&len))<0){
            print_error_and_exit("error in accept", s_udp, s_tcp);
        }
        // infinite loop while in connection with client
        while(1){
            cout << " in connection..." << endl;
            // do something
            sleep(10);

        }
        close(s_new);
    }

    close(s_udp);
    close(s_tcp);
    return 0;
}

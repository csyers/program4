/* Name: Tim Chang (tchang2), Christopher Syers (csyers)
 * Date: November 1, 2016
 * CSE 30264: Computer Networks
 * Programming Assignment 4: Program a Prototype of a Message Board Forum application
 */

#include "util.h"

void print_usage_and_exit();
void print_error_and_exit(string message);

int main(int argc, char* argv[]){
    string host, port_temp;
    int port;
    
    // check the number of arguemnts
    if(argc != 3){
        // if there are the wrong number of arguments, exit
        print_usage_and_exit();
    } else{
        // get the host name and port number
        host = argv[1];
        port_temp = argv[2];
        // check if the port only contains digits
        bool has_only_digits = (port_temp.find_first_not_of( "0123456789" ) == string::npos);
        if(has_only_digits) {
            // set port variable
            port = atoi(port_temp.c_str());
        } else{
            // print error if there are non-digits in the port argument
            print_error_and_exit("port argument must be positive integer");
        }
    }

    // local variables for socket programming
    struct hostent *hp;
    struct sockaddr_in sin;
    int s_udp, s_tcp;

    // get host name and if it is not resolvable, print error and exit
    if(!(hp =  gethostbyname(host.c_str()))){
        print_error_and_exit("unknown host");
    }

    // setup the sockaddr_in
    bzero((char *)&sin,sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr,(char *)&sin.sin_addr,hp->h_length);
    sin.sin_port = port;

    // open a UDP socket, exit if there is an error
    if((s_udp=socket(PF_INET,SOCK_DGRAM,0))<0){
        print_error_and_exit("could not open UDP socket");
    }

    // open a TCP socket, exit if there is an error
    if((s_tcp=socket(PF_INET,SOCK_STREAM,0))<0){
        print_error_and_exit("could not open TCP socket");
    }

    return 0;
}

/* Name: Tim Chang (tchang2), Christopher Syers (csyers)
 * Date: November 1, 2016
 * CSE 30264: Computer Networks
 * Programming Assignment 4: Program a Prototype of a Message Board Forum application
 */

#include "util.h"

void print_usage_and_exit();
void print_error_and_exit(string message, int s_udp, int s_tcp);

int main(int argc, char* argv[]){
    string host, port_temp, password;
    int port;
    int s_udp = -1, s_tcp = -1;
    int bytes_sent, bytes_received;
    
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
            print_error_and_exit("port argument must be positive integer", s_udp, s_tcp);
        }
    }

    // local variables for socket programming
    struct hostent *hp;
    struct sockaddr_in sin;

    // get host name and if it is not resolvable, print error and exit
    if(!(hp =  gethostbyname(host.c_str()))){
        print_error_and_exit("unknown host", s_udp, s_tcp);
    }

    // setup the sockaddr_in
    bzero((char *)&sin,sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr,(char *)&sin.sin_addr,hp->h_length);
    sin.sin_port = port;

    // open a UDP socket, exit if there is an error
    if((s_udp=socket(PF_INET,SOCK_DGRAM,0))<0){
        print_error_and_exit("could not open UDP socket", s_udp, s_tcp);
    }

    // open a TCP socket, exit if there is an error
    if((s_tcp=socket(PF_INET,SOCK_STREAM,0))<0){
        print_error_and_exit("could not open TCP socket", s_udp, s_tcp);
    }

    // connect tcp connection
    if(connect(s_tcp,(struct sockaddr *)&sin,sizeof(sin))<0){
        close(s_udp);
        close(s_tcp);
        print_error_and_exit("error in tcp connect call", s_udp, s_tcp);
    }

    // local variables for operations passing
    string operation;

    cout << "Enter operation (CRT, LIS, MSG, DLT, RDB, EDT, APN, DWN, DST, XIT, SHT): ";
    while(getline(cin,operation)){
        if(operation == "CRT"){

        } else if(operation == "LIS"){
            // case: list boards operation
        } else if(operation == "MSG"){
            // case: leave message operation
        } else if(operation == "DLT"){
            // delete message operation
        } else if(operation == "RDB"){
            // case read baord operation
        } else if(operation == "EDT"){
            // case: edit file operation
        } else if(operation == "APN"){
            // case: append file operation
        } else if(operation == "DWN"){
            // case: download file operation
        } else if(operation == "DST"){
            // case: destroy board operation
        } else if(operation == "XIT"){
            // case: exit client connection operation
            if (bytes_sent < 0) {
                print_error_and_exit("error sending operation XIT", s_udp, s_tcp);
            } else {
                cout << "Goodbye" << endl;
                break;
            }
        } else if(operation == "SHT"){
            // case: shutdown receiver operation
            bytes_sent = send_string_udp(operation, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending operation SHT", s_udp, s_tcp);
            } else {
                cout << "password: ";
                getline(cin, password);
                bytes_sent = send_string_udp(password, s_udp, sin);
                if (bytes_sent < 0) {
                    print_error_and_exit("error sending password", s_udp, s_tcp);
                } else {
                    string resp;
                    bytes_received = recv_string_udp(resp, s_udp, sin);
                    if (bytes_received < 0) {
                        print_error_and_exit("error in receiving confirmation", s_udp, s_tcp);
                    }
                    if (resp == "success") {
                        cout << "Server shutdown. Goodbye" << endl;
                        break;
                    } else {
                        cout << "invalid password" << endl;
                    }
                }
            }
        } else {
            // case: invalid command: send error message and prompt again
            cout << "Invalid operation: " << operation << endl;
        }
        cout << "Enter operation (CRT, LIS, MSG, DLT, RDB, EDT, APN, DWN, DST, XIT, SHT): ";
    }

    close(s_tcp);
    close(s_udp);
    return 0;
}

/* Name: Tim Chang (tchang2), Christopher Syers (csyers)
 * Date: November 1, 2016
 * CSE 30264: Computer Networks
 * Programming Assignment 4: Program a Prototype of a Message Board Forum application
 */

#include "util.h"

void print_usage_and_exit();
void print_error_and_exit(string message, int s_udp, int s_tcp);

int main(int argc, char* argv[]){
    string host, port_temp;
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

    string user_request, user, password_message, password, ack;

    bytes_sent = send_string_udp("ready", s_udp, sin);
    if (bytes_sent < 0) {    
        print_error_and_exit("error in send", s_udp, s_tcp);
    }
    
    while(1){
        bytes_received = recv_string_udp(user_request, s_udp, sin);
        if (bytes_received < 0) {
        print_error_and_exit("error in receiving request ", s_udp, s_tcp);
        }
        cout << user_request;
        getline(cin,user);

        bytes_sent = send_string_udp(user, s_udp, sin);
        if (bytes_sent < 0) {    
            print_error_and_exit("error sending username", s_udp, s_tcp);
        }
    
        bytes_received = recv_string_udp(password_message, s_udp, sin);
        if (bytes_received < 0) {
           print_error_and_exit("error in receiving password request", s_udp, s_tcp);
        }
        cout << password_message;
        getline(cin,password);
    
        bytes_sent = send_string_udp(password, s_udp, sin);
        if (bytes_sent < 0) {    
            print_error_and_exit("error sending password", s_udp, s_tcp);
        }
    
        bytes_received = recv_string_udp(ack, s_udp, sin);
        if (bytes_received < 0) {
           print_error_and_exit("error in receiving confirmation", s_udp, s_tcp);
        }
        if(ack == "success"){
            break;            
        } else if (ack == "failure"){
            cout << "wrong password" << endl;
        }
    }
    // local variables for operations passing
    string operation;
    string board_name, message, message_number, resp;

    cout << "Enter operation (CRT, LIS, MSG, DLT, RDB, EDT, APN, DWN, DST, XIT, SHT): ";
    while(getline(cin,operation)){
        if(operation == "CRT"){
            // case: create board
            bytes_sent = send_string_udp(operation, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending operation CRT", s_udp, s_tcp);
            } else {
                cout << "new board name: ";
                board_name = "";
                getline(cin,board_name);
                if(board_name.size() == 0 || has_only_spaces(board_name)){
                    while(board_name.size() == 0 || has_only_spaces(board_name)){
                        cout << "please enter at least one character in board name: ";
                        getline(cin,board_name);
                    }
                }
                bytes_sent = send_string_udp(board_name, s_udp, sin);
                if (bytes_sent < 0) {
                    print_error_and_exit("error sending board_name", s_udp, s_tcp);
                } else {
                    string resp;
                    bytes_received = recv_string_udp(resp, s_udp, sin);
                    if (bytes_received < 0) {
                        print_error_and_exit("error in receiving confirmation", s_udp, s_tcp);
                    }
                    cout << resp << endl;
                }
            }
        } else if(operation == "LIS"){
            // case: list boards operation
            bytes_sent = send_string_udp(operation, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending operation LIS", s_udp, s_tcp);
            }
            string listing;
            bytes_received = recv_string_udp(listing, s_udp, sin);
            if (bytes_received < 0) {
                print_error_and_exit("error in receiving listing", s_udp, s_tcp);
            }
            cout << listing;

        } else if(operation == "MSG"){
            // case: leave message operation
            bytes_sent = send_string_udp(operation, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending operation MSG", s_udp, s_tcp);
            }
            cout << "Enter board name to post on: ";
            getline(cin, board_name);
            cout << "Enter message: ";
            getline(cin, message);

            bytes_sent = send_string_udp(board_name, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending board_name", s_udp, s_tcp);
            }
            bytes_sent = send_string_udp(message, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending msg", s_udp, s_tcp);
            }
            bytes_received = recv_string_udp(resp, s_udp, sin);
            if (bytes_received < 0) {
                print_error_and_exit("error in receiving response", s_udp, s_tcp);
            }
            cout << resp << endl;
        } else if(operation == "DLT"){
            // delete message operation
            bytes_sent = send_string_udp(operation, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending operation DLT", s_udp, s_tcp);
            }
            cout << "Enter board name to delete from: ";
            getline(cin, board_name);
            cout << "Enter message number to be deleted: ";
            getline(cin, message_number);
            bytes_sent = send_string_udp(board_name, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending board_name", s_udp, s_tcp);
            }
            bytes_sent = send_string_udp(message_number, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending msg", s_udp, s_tcp);
            }
            bytes_received = recv_string_udp(resp, s_udp, sin);
            if (bytes_received < 0) {
                print_error_and_exit("error in receiving response", s_udp, s_tcp);
            }
            cout << resp << endl;
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
            bytes_sent = send_string_udp(operation, s_udp, sin);
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

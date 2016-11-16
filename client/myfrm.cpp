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
    int filesize;
    
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
    // wait for correct login
    while(1){
        // receive request for user name
        bytes_received = recv_string_udp(user_request, s_udp, sin);
        if (bytes_received < 0) {
        print_error_and_exit("error in receiving request ", s_udp, s_tcp);
        }

        // ask user for username
        cout << user_request;
        getline(cin,user);

        // send user name to server
        bytes_sent = send_string_udp(user, s_udp, sin);
        if (bytes_sent < 0) {    
            print_error_and_exit("error sending username", s_udp, s_tcp);
        }
    
        // receive request for password
        bytes_received = recv_string_udp(password_message, s_udp, sin);
        if (bytes_received < 0) {
           print_error_and_exit("error in receiving password request", s_udp, s_tcp);
        }

        // ask user for password
        cout << password_message;
        getline(cin,password);
    
        // send server the password
        bytes_sent = send_string_udp(password, s_udp, sin);
        if (bytes_sent < 0) {    
            print_error_and_exit("error sending password", s_udp, s_tcp);
        }
    
        // receive acknowledgment from server
        bytes_received = recv_string_udp(ack, s_udp, sin);
        if (bytes_received < 0) {
           print_error_and_exit("error in receiving confirmation", s_udp, s_tcp);
        }
        // if successful password, exit loop
        if(ack == "success"){
            break;            
        } else if (ack == "failure"){
            // else, try again
            cout << "wrong password" << endl;
        }
    }
    // local variables for operations passing
    string operation;
    string board_name, message, message_number, resp, new_message;

    cout << "Enter operation (CRT, LIS, MSG, DLT, RDB, EDT, APN, DWN, DST, XIT, SHT): ";
    while(getline(cin,operation)){
        if(operation == "CRT"){
            // case: create board
            // send CRT operation
            bytes_sent = send_string_udp(operation, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending operation CRT", s_udp, s_tcp);
            } else {
                // get board name to create
                cout << "new board name: ";
                board_name = "";
                getline(cin,board_name);
                // keep getting board name until it contains a character
                if(board_name.size() == 0 || has_only_spaces(board_name)){
                    while(board_name.size() == 0 || has_only_spaces(board_name)){
                        cout << "please enter at least one character in board name: ";
                        getline(cin,board_name);
                    }
                }
                // send board name to server
                bytes_sent = send_string_udp(board_name, s_udp, sin);
                if (bytes_sent < 0) {
                    print_error_and_exit("error sending board_name", s_udp, s_tcp);
                } else {
                    // receive confirmation from server
                    bytes_received = recv_string_udp(resp, s_udp, sin);
                    if (bytes_received < 0) {
                        print_error_and_exit("error in receiving confirmation", s_udp, s_tcp);
                    }
                    cout << resp << endl;
                }
            }
        } else if(operation == "LIS"){
            // case: list boards operation
            // send LIS to server
            bytes_sent = send_string_udp(operation, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending operation LIS", s_udp, s_tcp);
            }
            string listing;
            // receive and print the listing response from server
            bytes_received = recv_string_udp(listing, s_udp, sin);
            if (bytes_received < 0) {
                print_error_and_exit("error in receiving listing", s_udp, s_tcp);
            }
            cout << listing;

        } else if(operation == "MSG"){
            // case: leave message operation
            // send MSG operation to server
            bytes_sent = send_string_udp(operation, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending operation MSG", s_udp, s_tcp);
            }
            // prompt user for board_name and message
            cout << "Enter board name to post on: ";
            getline(cin, board_name);
            cout << "Enter message: ";
            getline(cin, message);

            // send board name
            bytes_sent = send_string_udp(board_name, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending board_name", s_udp, s_tcp);
            }
            // send message
            bytes_sent = send_string_udp(message, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending msg", s_udp, s_tcp);
            }

            // receive response from server and print it out
            bytes_received = recv_string_udp(resp, s_udp, sin);
            if (bytes_received < 0) {
                print_error_and_exit("error in receiving response", s_udp, s_tcp);
            }
            cout << resp << endl;
        } else if(operation == "DLT"){
            // delete message operation
            // send DLT to server
            bytes_sent = send_string_udp(operation, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending operation DLT", s_udp, s_tcp);
            }
            // prompt user for board and message number to delete
            cout << "Enter board name to delete from: ";
            getline(cin, board_name);
            cout << "Enter message number to be deleted: ";
            getline(cin, message_number);
            // send board_name
            bytes_sent = send_string_udp(board_name, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending board_name", s_udp, s_tcp);
            }
            // send message number
            bytes_sent = send_string_udp(message_number, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending msg", s_udp, s_tcp);
            }
            // receive response and print it
            bytes_received = recv_string_udp(resp, s_udp, sin);
            if (bytes_received < 0) {
                print_error_and_exit("error in receiving response", s_udp, s_tcp);
            }
            cout << resp << endl;
        } else if(operation == "RDB"){
            // case read baord operation
            // send RDB to server
            bytes_sent = send_string_udp(operation, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending operation RDB", s_udp, s_tcp);
            }

            // prompt for board name
            cout << "Enter board name to read: ";
            getline(cin, board_name);
            // send board name to server
            bytes_sent = send_string_udp(board_name, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending board name", s_udp, s_tcp);
            }

            // recv filesize
            bytes_received = recv_int_tcp(filesize, s_tcp);
            if (bytes_received < 0) {
                print_error_and_exit("error in receiving response", s_udp, s_tcp);
            }

            // case where server said file does not exist
            if (filesize < 0) {
                cout << "Board " << board_name << " does not exist" << endl;
            } else {
                while (filesize > 0) {
                    // receive file from client piece by piece and print it out
                    bytes_received = recv_file_tcp(message, s_tcp);
                    cout << message;
                    //message = "";
                    filesize -= bytes_received;
                }
            } 
        } else if(operation == "EDT"){
            // case: edit file operation
            // send EDT operation to server
            bytes_sent = send_string_udp(operation, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending operation EDT", s_udp, s_tcp);
            }
            // prompt user for board name, message number, and new message
            cout << "Enter board name to modify post on: ";
            getline(cin, board_name);
            cout << "Enter message number to modify: ";
            getline(cin, message_number);
            cout << "Enter new message: ";
            getline(cin,new_message);

            // send board_name
            bytes_sent = send_string_udp(board_name, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending board name", s_udp, s_tcp);
            }
            // send message number
            bytes_sent = send_string_udp(message_number, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending message number", s_udp, s_tcp);
            }
            // send new message
            bytes_sent = send_string_udp(new_message, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending message number", s_udp, s_tcp);
            }
            // receive server response and print it
            bytes_received = recv_string_udp(resp, s_udp, sin);
            if (bytes_received < 0) {
                print_error_and_exit("error in receiving response", s_udp, s_tcp);
            }
            cout << resp << endl;

        } else if(operation == "APN"){
            // case: append file operation
            string message, filename;
            int flag;
            // read baord operation
            bytes_sent = send_string_udp(operation, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending operation APN", s_udp, s_tcp);
            }

            // prompt for board name
            cout << "Enter board name: ";
            getline(cin, board_name);
            bytes_sent = send_string_udp(board_name, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending board name", s_udp, s_tcp);
            }

            // prompt for file name
            cout << "Enter file to append: ";
            getline(cin, filename);
            // send file name
            bytes_sent = send_string_udp(filename, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending file name", s_udp, s_tcp);
            }

            // recv flag
            bytes_received = recv_int_tcp(flag, s_tcp);
            if (bytes_received < 0) {
                print_error_and_exit("error in receiving flag", s_udp, s_tcp);
            }

            // recv msg
            bytes_received = recv_string_tcp(message, s_tcp);
            if (bytes_received < 0) {
                print_error_and_exit("error in receiving message", s_udp, s_tcp);
            }

            if (flag == -1) {
                // error on server side
                cout << message << endl;
                // send -1 filesize
                bytes_sent = send_int_tcp(-1, s_tcp);
                if (bytes_sent < 0) {
                    print_error_and_exit("error sending filesize -1", s_udp, s_tcp);
                } 
            // file can be appended
            } else if (flag == 0){
                cout << filename << endl;
                if(access(filename.c_str(), F_OK) == -1) {
                    // if the file doesn't exist on the client size, send error of -1 to server
                    cout << filename << " does not exist" << endl;
                    bytes_sent = send_int_tcp(-1, s_tcp);
                    if (bytes_sent < 0) {
                        print_error_and_exit("error sending filesize -1", s_udp, s_tcp);
                    } 
                } else {
                    // if the file exists, send it over
                    cout << message << endl;
                    fstream os;
                    os.open(filename.c_str(), ios::binary|ios::in|ios::app);
                    os.seekp(0, os.end);
                    filesize = os.tellp();
                    os.seekp(0, os.beg);

                    
                    //cout << filesize << endl;

                    // send filesize
                    bytes_sent = send_int_tcp(filesize, s_tcp);
                    if (bytes_sent < 0) {
                        print_error_and_exit("error sending filesize", s_udp, s_tcp);
                    } 

                    bytes_sent = send_file_tcp(filename, s_tcp);
                    if (bytes_sent < 0) {
                        print_error_and_exit("error sending file", s_udp, s_tcp);
                    }
                }
            } else {
                cout << "unknown flag" << endl;
                print_error_and_exit("error sending file", s_udp, s_tcp);
            }
        } else if(operation == "DWN"){
            // case: download file operation
            string message, filename;
            // case read baord operation
            bytes_sent = send_string_udp(operation, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending operation DWN", s_udp, s_tcp);
            }

            // prompt for board name
            cout << "Enter board name: ";
            getline(cin, board_name);

            // send board to read from
            bytes_sent = send_string_udp(board_name, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending board name", s_udp, s_tcp);
            }

            cout << "Enter file to read: ";
            getline(cin, filename);
            // send file name to read
            bytes_sent = send_string_udp(filename, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending file name", s_udp, s_tcp);
            }

            // recv filesize
            bytes_received = recv_int_tcp(filesize, s_tcp);
            if (bytes_received < 0) {
                print_error_and_exit("error in receiving response", s_udp, s_tcp);
            }

            // error condition 1
            if (filesize == -1) {
                cout << "File " << filename << " is not appended to board " << board_name << endl;
            // error condition 2
            } else if (filesize == -2) {
                cout << "Board " << board_name << " does not exist" << endl;
            // else, receive the file and write it
            } else {
                ofstream os;
                filename = board_name + "-" + filename;
                os.open(filename.c_str(), ios::app);
                // receive the file piece by peice
                while (filesize > 0) {
                    bytes_received = recv_file_tcp(message, s_tcp);
                    if (bytes_received < 0) {
                        print_error_and_exit("error in receiving response", s_udp, s_tcp);
                    }
                    //cout << "bytes recv: " << bytes_received << endl;
                    os << message;
                    filesize -= bytes_received;
                }
                // close os and print success message
                os.close();
                cout << "file " << filename << " successfully downloaded" << endl;
            }
        } else if(operation == "DST"){
            // case: destroy board operation
            // send DST to server
            bytes_sent = send_string_udp(operation, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending operation DST", s_udp, s_tcp);
            }
            // prompt user for board to destry
            cout << "Enter board name to destroy: ";
            getline(cin, board_name);
            //send board to destroy to server
            bytes_sent = send_string_udp(board_name, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending board name", s_udp, s_tcp);
            }
            // receive response and print it
            bytes_received = recv_string_udp(resp, s_udp, sin);
            if (bytes_received < 0) {
                print_error_and_exit("error in receiving response", s_udp, s_tcp);
            }
            cout << resp << endl;

        } else if(operation == "XIT"){
            // case: exit client connection operation
            // send XIT to server and break out of infinite loop to conclude program
            bytes_sent = send_string_udp(operation, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending operation XIT", s_udp, s_tcp);
            } else {
                cout << "Goodbye " << endl;
                break;
            }
        } else if(operation == "SHT"){
            // case: shutdown receiver operation
            // send SHT to server
            bytes_sent = send_string_udp(operation, s_udp, sin);
            if (bytes_sent < 0) {
                print_error_and_exit("error sending operation SHT", s_udp, s_tcp);
            } else {
            // prompt user for password
                cout << "password: ";
                getline(cin, password);
            // send password
                bytes_sent = send_string_udp(password, s_udp, sin);
                if (bytes_sent < 0) {
                    print_error_and_exit("error sending password", s_udp, s_tcp);
                } else {
                // receive response from server
                    bytes_received = recv_string_udp(resp, s_udp, sin);
                    if (bytes_received < 0) {
                        print_error_and_exit("error in receiving confirmation", s_udp, s_tcp);
                    }
                // if correct password, break out of infinite loop
                    if (resp == "success") {
                        cout << "Server shutdown. Goodbye" << endl;
                        break;
                // if incorrect password, say password is invlaid and return to prompt for next operation
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

    // end program
    close(s_tcp);
    close(s_udp);
    return 0;
}

/* Name: Tim Chang (tchang2), Christopher Syers (csyers)
 * Date: November 1, 2016
 * CSE 30264: Computer Networks
 * Programming Assignment 4: Program a Prototype of a Message Board Forum application
 */

#include "util.h"

#define MAX_PENDING 5
#define MAX_LINE 4096

    void print_usage_and_exit();
    void print_error_and_exit(string, int, int);
    int recreate_file(unordered_map<string, bi>, unordered_map<string, map<int,pair<string,string> > >, string);
    int main(int argc, char* argv[]){
    string port_temp, password, operation;
    string board_name, message, message_number, new_message;
    string username;

    // map from board name to its info (creator, stream, current line)
    unordered_map<string, bi> board_info;
    // map from board name to map of line number to author and message
    unordered_map<string, map<int,pair<string,string> > > board_contents;
    // map from username to passwords
    unordered_map<string,string> users;
    // map from board name to file appended to that board
    unordered_map<string, vector<string> > file_info;
    int port;
    int s_udp = -1;
    int s_tcp = -1;
    int bytes_received, bytes_sent;
    int filesize;

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
            close_fp(board_info);
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
    sin.sin_port = htons(port);

    // open a tcp socket, exit if there is an error
    if((s_tcp=socket(PF_INET,SOCK_STREAM,0)) < 0){
        close_fp(board_info);
        print_error_and_exit("error in tcp socket creation", s_udp, s_tcp);
    }

    // open a udp socket, exit if there is an error
    if((s_udp=socket(PF_INET,SOCK_DGRAM,0)) < 0){
        close_fp(board_info);
        print_error_and_exit("error in udp socket creation", s_udp, s_tcp);
    }

    // allow socket to be reused after a closed connection, exit on error
    if((setsockopt(s_tcp,SOL_SOCKET,SO_REUSEADDR,(char *)&opt,sizeof(int)))<0){
        close_fp(board_info);
        print_error_and_exit("error in setsocketopt", s_udp, s_tcp);
    }

    // bind the tcp socket to the port passed in on the command line
    if((bind(s_tcp,(struct sockaddr *)&sin,sizeof(sin)))<0){
        close_fp(board_info);
        print_error_and_exit("error in bind", s_udp, s_tcp);
    }   

    // bind the udp socket to the port passed in on the command line
    if((bind(s_udp,(struct sockaddr *)&sin,sizeof(sin)))<0){
        close_fp(board_info);
        print_error_and_exit("error in bind", s_udp, s_tcp);
    }   

    // begin listening for client connections on the socket
    if((listen(s_tcp,MAX_PENDING))<0) {
        print_error_and_exit("error in listen", s_udp, s_tcp);
    }

    // enter infinte loop wating for connections
    while(1){
        // accept a client connection on a new socket (s_new), exit on error
        if((s_new=accept(s_tcp,(struct sockaddr *)&sin,(socklen_t *)&len))<0){
            close_fp(board_info);
            print_error_and_exit("error in accept", s_udp, s_tcp);
        }
       // send prelim message
        string flag;
        bytes_received = recv_string_udp(flag, s_udp, sin);
        if (bytes_received < 0) {
            close(s_new);
            close_fp(board_info);
            print_error_and_exit("error receiving operation", s_udp, s_tcp);
        }
        // infinite loop for user login
        while(1){
            // request username
            bytes_sent = send_string_udp("please enter username: ", s_udp, sin);
            if (bytes_sent < 0) {
                close(s_new);
                close_fp(board_info);
                print_error_and_exit("error sending confirmation", s_udp, s_tcp);
            }
            // receive username
            bytes_received = recv_string_udp(username, s_udp, sin);
            if (bytes_received < 0) {
                close(s_new);
                close_fp(board_info);
                print_error_and_exit("error receiving operation", s_udp, s_tcp);
            }
            string user_password;
            if(users.find(username) == users.end()){
                // new password for new user
                bytes_sent = send_string_udp("please enter new password for new user: ", s_udp, sin);
                if (bytes_sent < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                }
                // get password
                bytes_received = recv_string_udp(user_password, s_udp, sin);
                if (bytes_received < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error receiving operation", s_udp, s_tcp);
                }
                // set password in map and send success message
                users[username] = user_password;
                bytes_sent = send_string_udp("success", s_udp, sin);
                if (bytes_sent < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                }
                break;
            } else {
                // get password for existing user
                bytes_sent = send_string_udp("please enter password for existing user: ", s_udp, sin);
                if (bytes_sent < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error sending confirmation", s_udp, s_tcp);

                }
                // receive password entry from user
                bytes_received = recv_string_udp(user_password, s_udp, sin);
                if (bytes_received < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error receiving operation", s_udp, s_tcp);
                }
                if(users[username] == user_password){
                    // successful password
                    bytes_sent = send_string_udp("success", s_udp, sin);
                    if (bytes_sent < 0) {
                        close(s_new);
                        close_fp(board_info);
                        print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                    }
                    break;
                } else {
                    // unsuccesful password
                    bytes_sent = send_string_udp("failure", s_udp, sin);
                    if (bytes_sent < 0) {
                        close(s_new);
                        close_fp(board_info);
                        print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                    }
                }
            }
        }
        // infinite loop while in connection with client
        while(1){
            // get the operation
            bytes_received = recv_string_udp(operation, s_udp, sin);
            if (bytes_received < 0) {
                close(s_new);
                close_fp(board_info);
                print_error_and_exit("error receiving operation", s_udp, s_tcp);
            }

            if (operation == "CRT") {
                // case: client wants to create a board
                // get board name
                bytes_received = recv_string_udp(board_name, s_udp, sin);
                if (bytes_received < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error receiving operation", s_udp, s_tcp);
                }
                if(access(board_name.c_str(), F_OK) == -1){
                    // file doesn't exist
                    bi bi1;
                    bi1.os = new fstream(board_name, ios::in|ios::out|ios::app);
                    
                    if(!bi1.os || !*bi1.os) {
                        close(s_new);
                        close_fp(board_info);
                        print_error_and_exit("error in opening file",s_udp,s_tcp);
                    } else {
                        // populate info struct
                        bi1.creator = username;
                        bi1.line = 0;
                        // map a blank map for new board
                        map<int,pair<string,string> > line_map;
                        board_info.insert(pair<string, bi>(board_name, bi1));
                        board_contents[board_name] = line_map;
                        // print username to first line of file
                        *bi1.os << username << endl;
                        bytes_sent = send_string_udp("successfully created board", s_udp, sin);
                        if (bytes_sent < 0) {
                            close(s_new);
                            close_fp(board_info);
                            print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                        }
                    }
                } else {
                    if(board_info.find(board_name) == board_info.end()){
                        // file is not in control
                        bytes_sent = send_string_udp("error: file exists but is not an active board", s_udp, sin);
                        if (bytes_sent < 0) {
                            close(s_new);
                            close_fp(board_info);
                            print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                        }
                    } else {
                        // file is in control
                        bytes_sent = send_string_udp("error: board already exists", s_udp, sin);
                        if (bytes_sent < 0) {
                            close(s_new);
                            close_fp(board_info);
                            print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                        }
                    }
                }
            } else if (operation == "MSG") {
                //case: client wants to post a MSG on a board
                // get board name from client
                bytes_received = recv_string_udp(board_name, s_udp, sin);
                if (bytes_received < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error receiving operation", s_udp, s_tcp);
                }

                // get message from client
                bytes_received = recv_string_udp(message, s_udp, sin);
                if (bytes_received < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error receiving operation", s_udp, s_tcp);
                }

                if(board_info.find(board_name) != board_info.end()){
                    // board exists and in control
                    // write to the board
                    *board_info[board_name].os << board_info[board_name].line << " " << username << ": " << message << endl;

                    // keep track of newly added line
                    int line_num = board_info[board_name].line;
                    board_contents[board_name][line_num] = make_pair(username, message);
                    board_info[board_name].line++;

                    // send success message back
                    message = "Message appended to board " + board_name;
                    bytes_sent = send_string_udp(message, s_udp, sin);
                    if (bytes_sent < 0) {
                        close(s_new);
                        close_fp(board_info);
                        print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                    }
                } else {
                    // board does not exists, send error message back
                    message = "Message board " + board_name + " does not exist";
                    bytes_sent = send_string_udp(message, s_udp, sin);
                    if (bytes_sent < 0) {
                        close(s_new);
                        close_fp(board_info);
                        print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                    }
                    
                }
            } else if (operation == "DLT") {
                // case: user wants to delete message
                // get baord name
                bytes_received = recv_string_udp(board_name, s_udp, sin);
                if (bytes_received < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error receiving board name", s_udp, s_tcp);
                }

                // get message number to delete
                bytes_received = recv_string_udp(message_number, s_udp, sin);
                if (bytes_received < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error message number", s_udp, s_tcp);
                }

                if (board_info.find(board_name) != board_info.end()) {
                    // board in control of this server
                    // check to see if message_number is in this board
                    map<int,pair<string,string> > temp_map = board_contents[board_name];
                    int line_num = atoi(message_number.c_str());
                    if(temp_map.find(line_num) != temp_map.end()){
                        // if it is, check to see if current user posted that message
                        if(temp_map[line_num].first == username){
                            // if it is, delte it and recreate file
                            temp_map.erase(line_num);
                            board_contents[board_name] = temp_map;
                            if(!recreate_file(board_info, board_contents, board_name)){
                                close(s_new);
                                close_fp(board_info);
                                print_error_and_exit("error receiving operation", s_udp, s_tcp);
                                
                            }
                            message = "Message " + message_number + " successfully deleted";
                            bytes_sent = send_string_udp(message, s_udp, sin);
                            if (bytes_sent < 0) {
                                close(s_new);
                                close_fp(board_info);
                                print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                            }
                        }else {
                            // if it isn't, send message saying that you can't delete it
                            message = "Message " + message_number + " was not wrriten by you, so you cannot delte it";
                            bytes_sent = send_string_udp(message, s_udp, sin);
                            if (bytes_sent < 0) {
                                close(s_new);
                                close_fp(board_info);
                                print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                            }
                        }
                    } else {
                        // if it isn't, send message saying that there is no such message in the board
                        message = "Message " + message_number + " does not exist in board " + board_name;
                        bytes_sent = send_string_udp(message, s_udp, sin);
                        if (bytes_sent < 0) {
                            close(s_new);
                            close_fp(board_info);
                            print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                        }
                    }
                } else {
                    // if the board does not exits, tell use
                    message = "Message board " + board_name + " does not exist";
                    bytes_sent = send_string_udp(message, s_udp, sin);
                    if (bytes_sent < 0) {
                        close(s_new);
                        close_fp(board_info);
                        print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                    }
                }
            } else if (operation == "EDT") {
                // case: client wants to EDIT a message
                // receive board_name from client
                bytes_received = recv_string_udp(board_name, s_udp, sin);
                if (bytes_received < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error receiving board name", s_udp, s_tcp);
                }

                // get message_number from client
                bytes_received = recv_string_udp(message_number, s_udp, sin);
                if (bytes_received < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error receiving message number", s_udp, s_tcp);
                }

                // get new message from client
                bytes_received = recv_string_udp(new_message, s_udp, sin);
                if (bytes_received < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error receiving new message", s_udp, s_tcp);
                }
                if (board_info.find(board_name) != board_info.end()) {
                    // board in control of this server
                    // check to see if message_number is in this board
                    map<int,pair<string,string> > temp_map = board_contents[board_name];
                    int line_num = atoi(message_number.c_str());
                    if(temp_map.find(line_num) != temp_map.end()){
                        // if it is, check to see if current user posted that message
                        if(temp_map[line_num].first == username){
                            // if it is, update it and recreate the file
                            temp_map[line_num].second = new_message;
                            board_contents[board_name] = temp_map;
                            if(!recreate_file(board_info, board_contents, board_name)){
                                close(s_new);
                                close_fp(board_info);
                                print_error_and_exit("error recreating the board", s_udp, s_tcp);
                                
                            }
                            message = "Message " + message_number + " successfully edited";
                            bytes_sent = send_string_udp(message, s_udp, sin);
                            if (bytes_sent < 0) {
                                close(s_new);
                                close_fp(board_info);
                                print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                            }
                        }else {
                            // if it isn't, send message saying that you can't delete it
                            message = "Message " + message_number + " was not wrriten by you, so you cannot delte it";
                            bytes_sent = send_string_udp(message, s_udp, sin);
                            if (bytes_sent < 0) {
                                close(s_new);
                                close_fp(board_info);
                                print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                            }
                        }
                    } else {
                        // if it isn't, send message saying that there is no such message in the board
                        message = "Message " + message_number + " does not exist in board " + board_name;
                        bytes_sent = send_string_udp(message, s_udp, sin);
                        if (bytes_sent < 0) {
                            close(s_new);
                            close_fp(board_info);
                            print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                        }
                    }
                } else {
                    // tell user that the board doesn't exist
                    message = "Message board " + board_name + " does not exist";
                    bytes_sent = send_string_udp(message, s_udp, sin);
                    if (bytes_sent < 0) {
                        close(s_new);
                        close_fp(board_info);
                        print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                    }
                }
                
            } else if (operation == "LIS") {
                // case: use wants to list boards
                // make a string with all of the board names
                string listing;
                for(auto const it: board_info){
                    listing += it.first + "\n";
                }

                // send that listing over udp
                bytes_sent = send_string_udp(listing, s_udp, sin);
                if (bytes_sent < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error sending listing", s_udp, s_tcp);
                }
            } else if (operation == "RDB") {
                // case: client wants to see the contents of a board
                // receive board to read
                bytes_received = recv_string_udp(board_name, s_udp, sin);
                if (bytes_received < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error receiving board name", s_udp, s_tcp);
                }

                if (board_info.find(board_name)!=board_info.end()) {
                    // board exist 
                    bi temp = board_info[board_name];
                    temp.os->seekp(0, temp.os->end);
                    filesize = temp.os->tellp();
                    bytes_sent = send_int_tcp(filesize, s_new);
                    if (bytes_sent < 0) {
                        close(s_new);
                        close_fp(board_info);
                        print_error_and_exit("error sending -1", s_udp, s_tcp);
                    } 

                    // send file
                    bytes_sent = send_file_tcp1(*board_info[board_name].os, s_new);
                    if (bytes_sent < 0) {
                        close(s_new);
                        close_fp(board_info);
                        print_error_and_exit("error sending -1", s_udp, s_tcp);
                    }
                } else {
                    // board dne
                    bytes_sent = send_int_tcp(-1, s_new);
                    if (bytes_sent < 0) {
                        close(s_new);
                        close_fp(board_info);
                        print_error_and_exit("error sending -1", s_udp, s_tcp);
                    } 
                }
            } else if (operation == "APN") {
                int flag;
                string msg;
                string filename, old_filename;
                // receive board to read
                bytes_received = recv_string_udp(board_name, s_udp, sin);
                if (bytes_received < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error receiving board name", s_udp, s_tcp);
                }

                // receive file to read
                bytes_received = recv_string_udp(filename, s_udp, sin);
                if (bytes_received < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error receiving file name", s_udp, s_tcp);
                }

                // does board exist?
                if (board_info.find(board_name)!=board_info.end()) {
                    // board exist 
                    old_filename = filename;
                    filename = board_name + "-" + filename;
                    vector<string> fvector = file_info[board_name];
                    if (find(fvector.begin(), fvector.end(), filename) == fvector.end()) {
                        // file has not yet been appended 
                        if(access(filename.c_str(), F_OK) == -1){
                            // file doesn't exist
                            flag = 0;
                            msg = "board " + board_name + " exists and file " + filename + " can be appended. Appending";
                        } else {
                            // file already exists
                            flag = -1;
                            msg = "file " + filename + " already exists elsewhere";
                        }
                    } else {
                        // file already appended to board
                        flag = -1;
                        msg = "file " + filename + " already appended to board";
                    } 
                } else {
                    // board dne
                    flag = -1;
                    msg = "board " + board_name + " doest not exist";
                }

                // send flag
                bytes_sent = send_int_tcp(flag, s_new);
                if (bytes_sent < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error sending flag", s_udp, s_tcp);
                } 

                // send message
                bytes_sent = send_string_tcp(msg, s_new);
                if (bytes_sent < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error sending message", s_udp, s_tcp);
                } 

                // receive filesize
                bytes_received = recv_int_tcp(filesize, s_new);
                if (bytes_received < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error in receiving filesize", s_udp, s_tcp);
                }


                // recv file
                if (filesize > 0) {
                    FILE *fp;
                    int remaining_filesize = filesize;
                    unsigned int bytesreceived;
                    char buf[MAX_LENGTH];
                    if(!(fp = fopen(filename.c_str(), "w+"))) {
                            close(s_new);
                            close_fp(board_info);
                            print_error_and_exit("error in receiving file", s_udp, s_tcp);
                    }
                    while (remaining_filesize > 0) {
                        bytesreceived = recv(s_new, buf, MAX_LINE > remaining_filesize ? remaining_filesize: MAX_LINE, 0);
                        if(bytesreceived < 0) {
                            close(s_new);
                            close_fp(board_info);
                            print_error_and_exit("error in receiving file", s_udp, s_tcp);
                        }
                        remaining_filesize -= bytesreceived;
                        if(fwrite(buf, sizeof(char), bytesreceived, fp) < bytesreceived) {
                            close(s_new);
                            close_fp(board_info);
                            print_error_and_exit("error in receiving file", s_udp, s_tcp);
                        }
                    }
                    fclose(fp);
                    file_info[board_name].push_back(filename);

                    // add message to board
                    string msg = "appended " + old_filename + " to the board";
                    *board_info[board_name].os << board_info[board_name].line << " " << username << ": " << msg << endl;
                    // keep track of line
                    int line_num = board_info[board_name].line;
                    board_contents[board_name][line_num] = make_pair(username, msg);
                    board_info[board_name].line++;
                }
            } else if (operation == "DWN") {
                string filename;
                // receive board to read
                bytes_received = recv_string_udp(board_name, s_udp, sin);
                if (bytes_received < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error receiving board name", s_udp, s_tcp);
                }

                // receive file to read
                bytes_received = recv_string_udp(filename, s_udp, sin);
                if (bytes_received < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error receiving file name", s_udp, s_tcp);
                }

                // does board exist?
                if (board_info.find(board_name)!=board_info.end()) {
                    // board exist 
                    filename = board_name + "-" + filename;
                    vector<string> fvector = file_info[board_name];
                    if (find(fvector.begin(), fvector.end(), filename) != fvector.end()) {
                        // file is appended to this board
                        fstream os (filename, ios::in|ios::app);
                        os.seekp(0, os.end);
                        filesize = os.tellp();
                    } else {
                        // file is not appended to board
                        filesize = -1;
                    } 
                } else {
                    // board dne
                    filesize = -2;
                }

                // send filesize
                bytes_sent = send_int_tcp(filesize, s_new);
                if (bytes_sent < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error sending -1", s_udp, s_tcp);
                } 

                // send file
                if (filesize > 0) {
                    bytes_sent = send_file_tcp(filename, s_new);
                    if (bytes_sent < 0) {
                        close(s_new);
                        close_fp(board_info);
                        print_error_and_exit("error sending -1", s_udp, s_tcp);
                    }
                }
            } else if (operation == "DST") {
                // case: user wants to destroy the a board
                // get board name
                bytes_received = recv_string_udp(board_name, s_udp, sin);
                if (bytes_received < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error receiving board name", s_udp, s_tcp);
                }
                if(board_info.find(board_name) != board_info.end()){
                    // if the baord is in control of the server
                    if(board_info[board_name].creator == username){
                        // if the creator of the board is the current user
                        board_info[board_name].os->close();
                        delete board_info[board_name].os;
                        board_info[board_name].os = 0;
                        board_info.erase(board_name);
                        remove(board_name.c_str());
                        // send success message
                        message = "Board " + board_name + " successfully destroyed";
                        bytes_sent = send_string_udp(message, s_udp, sin);
                        if (bytes_sent < 0) {
                            close(s_new);
                            close_fp(board_info);
                            print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                        }

                    } else {
                        // send error saying creator board is not the same as current user
                        message = "Board " + board_name + " was not created by you, so you cannot destroy it";
                        bytes_sent = send_string_udp(message, s_udp, sin);
                        if (bytes_sent < 0) {
                            close(s_new);
                            close_fp(board_info);
                            print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                        }
                    }
                } else {
                    // send error saying board is not in control of this server
                    message = "Message board " + board_name + " does not exist";
                    bytes_sent = send_string_udp(message, s_udp, sin);
                    if (bytes_sent < 0) {
                        close(s_new);
                        close_fp(board_info);
                        print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                    }
                }
            } else if (operation == "XIT") {
                // close curent connection and wait for another
                close(s_new);
                break;
            } else if (operation == "SHT") {
                // get the admin password attemp from user
                string client_password;
                bytes_received = recv_string_udp(client_password, s_udp, sin);
                if (bytes_received < 0) {
                    close(s_new);
                    close_fp(board_info);
                    print_error_and_exit("error receiving operation", s_udp, s_tcp);
                }

                // if it is correct:
                if (password == client_password) {
                    // send success message to user
                    bytes_sent = send_string_udp("success", s_udp, sin);
                    if (bytes_sent < 0) {
                        close(s_new);
                        close_fp(board_info);
                        print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                    } else {
                        // deletes files
                        unordered_map<string, bi>::iterator it;
                        for (it=board_info.begin(); it!=board_info.end(); it++) {
                            remove(it->first.c_str());
                        }
                        unordered_map<string, vector<string> >::iterator its;
                        for (its=file_info.begin(); its!=file_info.end(); its++) {
                            for (auto const it: its->second) {
                                remove(it.c_str());
                            }
                        }
                        // close all sockets and exit
                        close(s_new);
                        close(s_udp);
                        close(s_tcp);
                        close_fp(board_info);
                        return 0;
                    }
                // if the password is incorrect
                } else {
                    // send failure indication
                    bytes_sent = send_string_udp("failure", s_udp, sin);
                    if (bytes_sent < 0) {
                        close(s_new);
                        close_fp(board_info);
                        print_error_and_exit("error sending confirmation", s_udp, s_tcp);
                    }
                }
            } else {
                // other
            }
        }
        close(s_new);
    }

    close(s_udp);
    close(s_tcp);
    close_fp(board_info);
    return 0;
}

/* Name: Tim Chang (tchang2), Christopher Syers (csyers)
 * Date: November 1, 2016
 * CSE 30264: Computer Networks
 * Programming Assignment 4: Program a Prototype of a Message Board Forum application
 */

#include "util.h"

void print_usage_and_exit();

int main(int argc, char* argv[]){
    string server, port_temp;
    int port;
    if(argc != 3){
        print_usage_and_exit();
    } else{
        server = argv[1];
        port_temp = argv[2];
        bool has_only_digits = (port_temp.find_first_not_of( "0123456789" ) == string::npos);
        if(has_only_digits) {
            port = atoi(port_temp.c_str());
        } else{
            cerr << "myfrm: port argument can only contain numbers" << endl;
            exit(1);
        }
    }

    cout << "port: " << port << "\tserver: " << server << endl;
    return 0;
}

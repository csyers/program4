/* Name: Tim Chang (tchang2), Christopher Syers (csyers)
 * Date: November 1, 2016
 * CSE 30264: Computer Networks
 * Programming Assignment 4: Program a Prototype of a Message Board Forum application
 */

#include "util.h"

using namespace std;

void print_usage_and_exit();

int main(int argc, char* argv[]){
    // check usage
    int port;
    string port_temp, password;
    if(argc != 3){
        print_usage_and_exit();
    } else{
        string port_temp = argv[1];
        password = argv[2];
        bool has_only_digits = (port_temp.find_first_not_of( "0123456789" ) == string::npos);
        if(has_only_digits) {
            port = atoi(argv[1]);
        } else{
            cerr << "myfrmd: port argument can only contain numbers" << endl;
            exit(1);
        }
    }

    cout << "port: " << port << "\tpassword: " << password << endl;
    return 0;
}

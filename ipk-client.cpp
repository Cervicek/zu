#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

/****************************** Vypis chyby ***********************************/
void error ( const char *message )
{
    perror ( message ) ;
    exit(0);
}

/****************************** Vytvoreni socketu *****************************/
int new_socket ( int s ) {
    s = socket ( AF_INET, SOCK_STREAM, 0 );
    return s;
}

/*********************************** Pripojeni ********************************/
void connect_to_server ( int s, sockaddr_in sa ) {
    if ( connect ( s, (struct sockaddr *) &sa , sizeof ( sa ) ) < 0 ) {
        error("Chyba pripojeni.");
    }
}

/************************************* Zapis **********************************/
int write_to_socket ( int s, char * b ) {
    int i;
    i = write ( s , b , strlen ( b ) );
    if ( i < 0 ) {
        error ( "Zapis se nezdaril." );
    }
    return i;
}

/************************************* Cteni **********************************/
int read_from_socket ( int s, char * b ) {
    int i;
    i = read ( s, b, 255 );
    if ( i < 0 ) {
        error ( "Cteni se nezdarilo." );
    }
    return i;
}

/********************************* Pocitani ***********************************/
double count_it ( double num1, double num2, string oper) {
    string o ("+");
    string o2 ("-");
    string o3 ("*");
    string o4 ("/");
    double result;
    int tmp;
    if ( oper.compare(o) == 0 ) {
        result = num1 + num2;
    }
    else if ( oper.compare(o2) == 0 ) {
        result = num1 - num2;
    }
    else if ( oper.compare(o3) == 0 ) {
        result = num1 * num2;
    }
    else if ( oper.compare(o4) == 0 ) {
        result = num1 / num2;

    }
    // omezeni na dve desetinna mista
    result = result * 100;
    tmp = result;
    result = tmp / 100.00;
    return result;
}

/*********************************** Main *************************************/
int main ( int argc, char *argv[] )
{
    int sock, n, fs_block_size;
    int rc = 0;
    struct hostent *server;
    char buffer[256], sdbuf[512];
    char * file_name;
    string addres = argv[1];
    // overeni poctu argumentu
    if ( argc < 2 ) {
       error ( "Spatne zadane argumenty!" );
    }

    // IPv6
    if (addres.find(":") != string::npos) {

        struct sockaddr_in6 serv_addr;
        sock = socket ( AF_INET6, SOCK_STREAM,0 );
        if (sock < 0) {
            error("Chyba vytvoreni socketu.");
        }

        else {

            bzero ( ( char * ) &serv_addr, sizeof ( serv_addr ) );
            server = gethostbyname2 ( argv[1], AF_INET6 );

            memset((char *) &serv_addr, 0, sizeof(serv_addr));
            serv_addr.sin6_flowinfo = 0;
            serv_addr.sin6_family = AF_INET6;


            memmove (  ( char * ) &serv_addr.sin6_addr.s6_addr, ( char * ) server->h_addr, server->h_length );
            serv_addr.sin6_port = htons ( 55555 );
            // pripojuji se na server
            if ( connect ( sock, (struct sockaddr *) &serv_addr , sizeof ( serv_addr ) ) < 0 ) {

                error("Chyba pripojeni.");
            }

        }
    }
    // IPv4
    else {
        sock = new_socket ( sock );
        struct sockaddr_in serv_addr;
        server = gethostbyname ( argv[1] );

        if ( server == NULL ) {
            error ( "Server nenazelen." );
        }

        bzero ( ( char * ) &serv_addr, sizeof ( serv_addr ) );

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons ( 55555 );

        bcopy ( ( char * ) server->h_addr, ( char * ) &serv_addr.sin_addr.s_addr, server->h_length );

        // pripojuji se na server
        connect_to_server ( sock, serv_addr );
    }


    bzero ( buffer,256 );

    n = write_to_socket ( sock, "HELLO 8497fb7dfc66377a203e78a6c8f21020\n" );
    bzero ( buffer,256 );
    while (read_from_socket(sock, buffer) > 0) {

        cout << buffer;

        char *ch, *ch2;
        ch = strstr (buffer, "SOLVE");
        ch2 = strstr (buffer, "BYE 8497fb7dfc66377a203e78a6c8f21020\n");
        
        // dostala jsem SOLVE
        if(ch != NULL){
            double number1, number2, result;
            string str1, oper, str;
            char res[256];
            string o ("/");
            stringstream ss;
            ss << buffer;

            ss >> str1 >> number1 >> oper >> number2;
            ss >> str1 >> number1 >> oper >> number2;

            if (number2 == 0 && oper == "/" ) {
                write_to_socket ( sock, "RESULT ERROR\n" );
                cout << "RESULT ERROR\n" << endl;
                bzero ( res,256 ) ;
            }
            else {
                result = count_it( number1, number2, oper);
                snprintf (res, sizeof(res), "RESULT %0.2f\n",  result);
                cout << res << endl;
                write_to_socket ( sock, res );
                bzero ( res,256 ) ;
                //read_from_socket ( sock, res );

            }
        }
        if (ch2 != NULL) {
            cout << "BYE 8497fb7dfc66377a203e78a6c8f21020\n" << endl;
        }
        bzero ( buffer,256 );
    }

    close ( sock );

    return 0;
}

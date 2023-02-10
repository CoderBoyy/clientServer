#include <iostream>
#include <sys/types.h> //it is necessary to include
#include <unistd.h>
#include <sys/socket.h>
// sys.socket.h has operations of sockets which are controlled by socket level options.
#include <netdb.h> //defines max len for host and server name i.e NI_MAXSERV and NI_MAXHOST.
#include <arpa/inet.h>
#include <string.h> //memset lib.
#include <string>

using namespace std;

int main()
{
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    // socket is having 3 arguments here. 1) domain(specifies domain within which communication will take place), 2) type(semantics of communication), 3) protocol (specifies particular protocol to be used with the socket. normally only a single protocol exist to support a particular socket.)
    // 1) AF_INET is address family provides interprocess communication between processes that run on the same system or on different systems
    // 2) SOCK_STREAM is reliable 2 way connection based. we have DGRAM which is unreliable and connectionless and then we have RAW which provides access to interneal network protocols and is available to super-user.
    // 3) 0 is Hop by hop option HOPOPT IPv6. Hop-by-Hop Options header is used to specify delivery parameters at each hop on the path to the destination.
    if (listening == -1)
    {
        cerr << "Can't create a socket! Quitting" << endl;
        return -1;
    }
    else
    {
        cout << "Socket opened successfully on " << listening << endl;
    }

    sockaddr_in hint; // The SOCKADDR_IN contain the details about the family, the port where we will be listening and ip address
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000);                  // The htons function takes a 16-bit number in host byte order and returns a 16-bit number in network byte order used in TCP/IP networks. Host byte order sorts bytes in the manner which is most natural to the host software and hardware. There are two common host byte order methods: Little-endian byte ordering and big endian byte ordering. network byte order, defines the bit-order of network addresses as they pass through the network.
    hint.sin_addr.s_addr = INADDR_ANY;             // since i want to use my machine itself as a server so i am giving IP of my machine itself. This will itself pick my system IP. In order to provide IP by ourself, we can write inet_addr("IP we want to call").
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr); // inet_pton() function converts an Internet address in its standard text format into its numeric binary form.

    int j = bind(listening, (sockaddr *)&hint, sizeof(hint)); // bind() method which binds it to a specific IP and port so that it can listen to incoming requests on that IP and port.
    if (j < 0)
    {
        cout << "failed to bind the local port" << endl;
        return -1;
    }
    else
    {
        cout << "successfully bind to local port" << endl;
    }
    j = listen(listening, SOMAXCONN); // listen() method puts the server into listen mode. This allows the server to listen to incoming connections. SOMAXCONN tells us how many requests at a time our server can pool, basically how many requests can be in the active queue for the server, we can enter any integer as well. Listening here is my socket name.
    if (j < 0)
    {
        cout << "failed to listen to local port" << endl;
        return -1;
    }
    else
    {
        cout << "successfully listening to local port" << endl;
    }

    sockaddr_in client;
    socklen_t clientSize = sizeof(client);
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

    int clientSocket = accept(listening, (sockaddr *)&client, &clientSize);
    // listening is the listener socket, using it we are just recieving the request from a new client.But we can not use it to comunicate to the new client. For that we have another new socket which we have named clientSocket and created using accept function. Using clientSocket, we can communicate with the client.

    if (clientSocket == -1)
    {
        cout << "Error on accepting request" << endl;
        return -1;
    }

    close(listening);

    // MEMSET
    // The memset() function writes len bytes of value c (converted to an unsigned char) to the string b. It returns its first argument.
    memset(host, 0, NI_MAXHOST);
    memset(service, 0, NI_MAXSERV);
    // GET NAMEINFO:
    // used to convert a sockaddr structure to a pair of hostname and service strings. The SOCKADDR_IN structure specifies a transport address and port for the AF_INET address family. The sockaddr structure sa should point to either a sockaddr_in, sockaddr_in6 or sockaddr_dl structure (for IPv4, IPv6 or link-layer respectively).
    // The host and service names associated with sa are stored in host and serv which have length parameters hostlen and servlen.  The maximum value for hostlen is NI_MAXHOST and the maximum value for servlen is NI_MAXSERV, as defined by ⟨netdb.h⟩.enough space must be provided to store the host name or service string plus a byte for the NUL terminator. getnameinfo() returns zero on success or one of the error codes listed in gai_strerror(3) if an error occurs.

    if (getnameinfo((sockaddr *)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
    {
        cout << host << " connected on port " << service << endl;
    }
    else
    {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST); // The inet_ntop() function converts from an Internet address in binary format, specified by src, to standard text format, and places the result in dst, when size, the space available in dst, is sufficient. The argument af specifies the family of the Internet address. This can be AF_INET or AF_INET6.
        cout << host << " connected on port " << ntohs(client.sin_port) << endl;
    }

    char buf[4096];

    while (true)
    {
        memset(buf, 0, 4096);

        int bytesReceived = recv(clientSocket, buf, 4096, 0); // recv function is used to read incoming data on connection-oriented sockets, or connectionless sockets
        if (bytesReceived == -1)
        {
            cerr << "Error in recv(). Quitting" << endl;
            break;
        }

        if (bytesReceived == 0)
        {
            cout << "Client disconnected " << endl;
            break;
        }

        cout << string(buf, 0, bytesReceived) << endl; // printing the message recieved.

        send(clientSocket, buf, bytesReceived + 1, 0);
    }

    // Close the socket
    close(clientSocket);

    return 0;
}

// socket descriptor is something which is pointing to a particular kernel object which is opaque means we cant see/access through it directly.
// blocking vs non-blocking sockets: in blocking, client sends a request and waits for the response before proceeding with the next request and therefore we need multiple threads in this while in non blocking, it doesnt wait for the response but proceeds with the next request.  We can declare our socket as blocking or non blocking just above the line where we are binding our socket.By default every socket is a blocking socket.
// setscokopt(), needs to be called before bind function.

// Leftover tasks:
// checkout select system calls and socket decriptors.
// checkout setsockopt.
// what is a read state socket descriptor?//checkout fdset
//
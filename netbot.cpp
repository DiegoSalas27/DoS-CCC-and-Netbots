#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

int main()
{
	int sock = 0, valread, client_fd;
	struct sockaddr_in server_address;
	char *hello = "Hello from client";
	char buffer[1024] = { 0 };
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		cout << "\n Socket creation error \n";
		return -1;
	}

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("10.0.2.4");
	server_address.sin_port = htons(8080);

	start:
	while(connect(sock, (struct sockaddr *) &server_address, sizeof(server_address)) != 0) {
		sleep(10);
		goto start;
	}

	send(sock, hello, strlen(hello), 0);
	cout << "Hello message sent\n";
	valread = read(sock, buffer, 1024);
	cout << buffer << "\n";
}

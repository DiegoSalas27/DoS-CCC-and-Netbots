# DoS-CCC-and-Netbots

## Summary

DDoS Framework comprised of a Command Control Center (CCC) and botnets in a controlled environment using virtual machines to conduct DDoS attacks on Linux servers.

## Technical details

This project was developed in C++. Both botnets and the CCC communicate with each other through TCP sockets in order to send and receive packages containing information about the type of attacks the CCC is requesting from the botnets. 

## Main libraries used

* sys/socket.h: makes available the type socklen_t which is an unsigned integral type of length of at least 32 bits. The main purpose is not to worry about the size an int can take on any operating system.
  * socklen_t distinguishes itself from other int types for this reason. It was used to get the client's address length which is used for accepting a new connection on the server socket. Functions like accept, bind, connect, listen, recv, shutdown, and socket are also provided by this library.

* netinet/in.h: defines different types and structs like sockadd_in which is used in socket programming to allow binding a socket to a desired address for the server to be able to listen to clients' connection request. (https://linuxhint.com/sockaddr-in-structure-usage-c/)

* arpa/inet.h: provides definition for internet operations such as inet_addr which was used to convert an IP in the standard dot notation to an integer suitable for use as an Internet address (https://pubs.opengroup.org/onlinepubs/7908799/xns/inet_addr.html), it also provides the htons function to convert an unsigned short integer to a network byte order (https://www.ibm.com/docs/en/zos/2.4.0?topic=lf-htons-translate-unsigned-short-integer-into-network-byte-order)

* thread: was used to create threads for every single botnet so that they could execute commands concurrently in a multithreading environment.
  * detach method was used to allow threads to execute independently from each other. This also helped the CCC not being blocked when new connections from botnets where stablished.
 
* signal.h: used to interrupt current execution of a DoS attack from a botnet when the pause command or a new attack from CCC was issued.

## Replicate DDoS Environment

* You will have to place all virtual machines you are working with (The one containing the CCC, botnets, and the victim's machine) in the same network. By default NAT adapter option is assigned to newly created VMs so they won't be able to communicate with each other.
* Use a NAT Network adapter option in the network settings of your VM. This will place VMs in one virtual router (in the same network). With this the CCC will be able to issue commands to the botnets and the DoS attacks will reach the victim's machine.

DDoS Demo: https://www.youtube.com/watch?v=V0aIa173xyg

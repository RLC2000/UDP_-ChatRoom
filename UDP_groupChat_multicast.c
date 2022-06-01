#include <stdio.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <netinet/udp.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <arpa/inet.h> 
#include <pthread.h> 
#include <string.h>

#define MULTICAST_PORT 1234 
#define MULTICAST_GROUP "225.0.0.1"

char username[50]; 

void *sender(void *arg) { 
	
	int cid = *((int *)arg); 
	struct sockaddr_in c; 
	c.sin_family = AF_INET; 
	c.sin_port = htons(MULTICAST_PORT); 
	
	// Convert Internet host address from numbers-and-dots notation in CP 
	// into binary data and store the result in the structure INP 
	inet_aton(MULTICAST_GROUP, &(c.sin_addr));
	
	while (1) { 
	
		char msg[2][50]; 
		strcpy(msg[0], username); 
		fgets(msg[1], 50, stdin); 
		int l = sizeof(c); 
		sendto(cid, msg, sizeof(msg), 0, (struct sockaddr *)&c, sizeof(c)); 
		
		if (strcmp(msg[1], "bye") == 0) { 
			pthread_exit(NULL);
			break; 
		} 
	} 
}

void *reciever(void *arg) { 
	
	int cid = *((int *)arg); 
	struct sockaddr_in c;
	
	while (1) {
		
		char msg[2][50];
		char SIP[16]; 
		short SPORT; 
		
		int l = sizeof(c); 
		
		recvfrom(cid, msg, sizeof(msg), 0, (struct sockaddr *)&c, &l); 
		
		strcpy(SIP, inet_ntoa(c.sin_addr)); 
		SPORT = ntohs(c.sin_port); 
		
		fprintf(stderr, "\nMessage from %s: %s\n", msg[0], msg[1]); 
		
		if (strcmp(msg[1], "bye") == 0 && strcmp(msg[0], username) == 0) { 
			pthread_exit(NULL); 
			break; 
		} 
	} 
}

int main() { 

	int sid = socket(AF_INET, SOCK_DGRAM, 0); 
	perror("socket"); 
	
	int reuse = 1; 
	// This tells the system to receive packets on the network whose destination is the group address (but not its own) 
	if (setsockopt(sid, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse, sizeof(reuse)) < 0) 
		perror("SO_REUSEADDR"); 
	
	struct sockaddr_in s;
	s.sin_family = AF_INET; 
	s.sin_addr.s_addr = htonl(INADDR_ANY); 
	s.sin_port = htons(MULTICAST_PORT); 
	
	bind(sid, (struct sockaddr *)&s, sizeof(s)); 
	perror("bind");
	
	// IPv4 multicast request. 
	struct ip_mreq m; 
	
	// add an IP group membership 
	m.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP); 
	m.imr_interface.s_addr = htonl(INADDR_ANY); 
	
	// IP_ADD_MEMBERSHIP /* add an IP group membership */ 
	setsockopt(sid, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&m, sizeof(m)); 
	perror("IP_ADD_MEMBERSHIP"); 
	
	printf("Enter your name: "); 
	scanf("%s", username); 
	printf("In the chat\n"); 
	
	pthread_t s1, r; pthread_create(&s1, NULL, sender, &sid); 
	pthread_create(&r, NULL, reciever, &sid); 
	pthread_join(s1, NULL); 
	pthread_join(r, NULL); 
	
	return 0; 
}
	
	





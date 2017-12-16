#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include <stdint.h>
//#pragma comment(lib,"ws2_32.lib") //Winsock Library
typedef unsigned long  crc;

#define POLYNOMIAL 0xD8
#define WIDTH  (8 * sizeof(crc))
#define TOPBIT (1 << (WIDTH - 1))

typedef struct header {
	int velkostF;
	int poradie;
	crc check;
} HEADER;

crc  crcTable[256];

void crcInit(void)
{
	crc  remainder;
	for (int dividend = 0; dividend < 256; ++dividend)
{		
		remainder = dividend << (WIDTH - 8);
		for (uint8_t bit = 8; bit > 0; --bit){
			
			if (remainder & TOPBIT){
				remainder = (remainder << 1) ^ POLYNOMIAL;
			}
			else{
				remainder = (remainder << 1);
			}
		}
		crcTable[dividend] = remainder;
	}
}

crc crcFast(uint8_t const message[], int nBytes){
	uint8_t data;
	crc remainder = 0;

	for (int byte = 0; byte < nBytes; ++byte){
		data = message[byte] ^ (remainder >> (WIDTH - 8));
		remainder = crcTable[data] ^ (remainder << 8);
	}
	return (remainder);
} 

int wsaFunction(){
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}
}

int checkPort(int cisloPortu) {
	if ((cisloPortu < 0) || (cisloPortu > 65535)) {
		printf("Invalid port number. Terminating.\n");
		return 1;
	}
}

int checkFragmentSize(int velkostFragmentu){
	while (velkostFragmentu < 13) {
		printf("Moc mala velkost, musi byt vacsia ako 12, zvolte znovu.\n");
		scanf("%d", &velkostFragmentu);
		getchar();
	}
}

void setHeader(HEADER *hlavicka, int velkostFragmentu) {
	hlavicka->poradie = -1;
	hlavicka->check = 0;
	hlavicka->velkostF = velkostFragmentu;
}

int tryToSocket() {
	int sockfd;
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("%d\n", sockfd);
		perror("Failed to create a socket.");
		return 2;
	}
	else return sockfd;
}

int recievedHeader(HEADER *prijataHlavicka, char *bafer, int checksum, int *i, int *pos) {
	if (prijataHlavicka->check == checksum)
		printf("Uspesne odoslany %d. fragment, velkost %dB\n", i + 1, prijataHlavicka->velkostF + sizeof(HEADER));
	else {
		printf("Nespravne odoslany %d. fragment, velkost %dB. Znovuposielanie.\n\n", i + 1, prijataHlavicka->velkostF + sizeof(HEADER));
		i--;
		pos = 0;
	}
}

int prepnutie(int sockfd, char *c) {
	printf("Chces prepnut klient/server? (y/n) ");
	c = getchar();
	getchar();
	if (c == 'y') {
		return 1;
	}
	else return 0;
	
}

int main() {
	crcInit();
	char c = "";
	
	printf("k - klient\ns - server\n q - quit\n");

	while ((c = getchar()) != 'q') {
		switch (c) {
		case 'k': { klientMain(); printf("k - klient\ns - server\n q - quit\n");  break; }
		case 's': { serverMain(); printf("k - klient\ns - server\n q - quit\n"); break; }
		//default: printf("unresolved symbol.\n");
		}
	}
	return 0;
}
int klientMain(){
	HEADER *hlavicka, *prijataHlavicka;

	struct sockaddr_in mojaAdresa;   // address of the client
	struct sockaddr_in serverAdresa; // server's address
	int cisloPortu, buf[512], dlzkaSpravy;
	int sockfd = 0, slen = sizeof(mojaAdresa), velkostFragmentu = 13, recvlen, flag = 0, checksum = 0, pos = 0;
	char sprava[60000], *bafer, *castspravy, server[20];
	char **fragmentovanaSprava;
	
	wsaFunction();

	hlavicka =(HEADER *) malloc(sizeof(HEADER));

	printf("Enter port:");
	scanf("%d", &cisloPortu);
	checkPort(cisloPortu);
	sockfd = tryToSocket();
	printf("Som v klientovi.\n");
	//printf("%d\n", sockfd);

	memset((char *)&mojaAdresa, 0, sizeof(mojaAdresa));

	//strcpy(server, "127.0.0.1");

	mojaAdresa.sin_family = AF_INET;
	printf("Enter server (format IPv4): ");
	scanf("%s", server);
	mojaAdresa.sin_addr.S_un.S_addr= inet_addr(server);
	mojaAdresa.sin_port = htons(cisloPortu);

	printf("Zadajte max velkost fragmentu: ");
	scanf("%d", &velkostFragmentu);
	getchar();

	checkFragmentSize(velkostFragmentu);
	setHeader(hlavicka,velkostFragmentu);

	sendto(sockfd, hlavicka, sizeof(HEADER), 0, (struct sockaddr *)&mojaAdresa, slen);

	while (1) {
		printf("Prajes si odoslat chybny ramec? (y/n) ");
		char c = getchar();

		if (c == 'y') {
			printf("Vnasam chybu.\n");
			flag = 1;
		}
		getchar();

		printf("Enter expression:");
		gets(sprava);
		dlzkaSpravy = strlen(sprava);

		int fragmentFinalSize = velkostFragmentu - sizeof(HEADER);

		int pocetFragmentovvPoli = dlzkaSpravy / (fragmentFinalSize);

		if ((dlzkaSpravy % (fragmentFinalSize)) != 0) {
			pocetFragmentovvPoli++;
		}

		//printf("%d\n", pocetFragmentovvPoli);
		int velkostPoslednehoFragmentu = dlzkaSpravy % (fragmentFinalSize);
		if (velkostPoslednehoFragmentu == 0)
			velkostPoslednehoFragmentu = fragmentFinalSize;

		fragmentovanaSprava = (int**)malloc(pocetFragmentovvPoli * sizeof(int*));
		int currentSize;

		for (int i = 0; i < pocetFragmentovvPoli; i++) {
			if ((i + 1) == pocetFragmentovvPoli) {
				currentSize = velkostPoslednehoFragmentu;
			}
			else {
				currentSize = fragmentFinalSize;
			}
			fragmentovanaSprava[i] = (int*)malloc((currentSize + sizeof(HEADER) + 1) * sizeof(int));
			for (int j = 0; j < currentSize; j++) {
				fragmentovanaSprava[i][j] = sprava[pos + j];
			}
			pos = pos + currentSize;
			bafer = (char *)malloc(currentSize + sizeof(HEADER) + 1);
			hlavicka = (HEADER *)bafer;
			castspravy = bafer + sizeof(HEADER);
			strncpy(castspravy, fragmentovanaSprava[i], currentSize);
			castspravy[currentSize] = '\0';
			hlavicka->poradie = i + 1;
			if (flag == 1) {
				printf("Vnasam chybu.\n");
				hlavicka->check = -2;
				flag = 0;
			}
			else
			{
				hlavicka->check = crcFast(castspravy, currentSize);
			}
			checksum = hlavicka->check;

			hlavicka->velkostF = currentSize;
			//printf("%s\n", castspravy);
			//printf("%d", hlavicka->check);


			if (sendto(sockfd, bafer, (currentSize + sizeof(HEADER)), 0, (struct sockaddr *)&mojaAdresa, slen) < 0) {
				perror("Cannot sendto()");
				return 4;
			}

			// PRIJIMANIE POTVRDENIA OD SERVERU
			recvlen = recvfrom(sockfd, bafer, 500, 0, (struct sockaddr *)&serverAdresa, &slen);
			if (recvlen == -1) {
				perror("cannot recvfrom()");
			}
			else {
				prijataHlavicka = (HEADER *)bafer;
				recievedHeader(prijataHlavicka, bafer, checksum, &i, &pos);
			}


		}

		sendto(sockfd, "Success", strlen("Success") + 1, 0, (struct sockaddr *)&mojaAdresa, slen);
		pos = 0;


		if (prepnutie(sockfd, &c) == 1) {
			closesocket(sockfd);
			WSACleanup();
			break;
		}
	//closesocket(sockfd);
	//WSACleanup();
	//break;
	}
	return 0;
}

int serverMain() {
	HEADER *hlavicka;
	struct sockaddr_in mojaAdresa;  // address of the server
	struct sockaddr_in klientAdresa;  // address of the client
	char buf[60000], *fragment, *sprava,c = "";
	int sockfd, slen = sizeof(klientAdresa), recvlen, cisloPortu, velkostHlavicky, poradieHlavicky, spracovanieHlavicky;
	int max = 0;
	char *ackMessage;
	WSADATA wsa;

	wsaFunction();
	printf("Som v serveri.\n");

	sockfd = tryToSocket();
	
	printf("Enter port:");
	scanf("%ld", &cisloPortu);
	checkPort(cisloPortu);

	sprava = malloc(1);
	
	memset((char *)&mojaAdresa, 0, sizeof(mojaAdresa));
	mojaAdresa.sin_family = AF_INET;
	mojaAdresa.sin_addr.s_addr = INADDR_ANY;
	mojaAdresa.sin_port = htons(cisloPortu);

	if (bind(sockfd, (struct sockaddr*)&mojaAdresa , sizeof(mojaAdresa)) == -1)
	{
		perror("Could not bind socket.");
	}

	memset(buf, '\0', sizeof(buf));
	printf("Cakam na hlavicku...");
	spracovanieHlavicky = recvfrom(sockfd, buf, 500, 0, (struct sockaddr *)&klientAdresa, &slen);
	printf(" \nmam\n");
	hlavicka = (HEADER *)buf;
	max = hlavicka->velkostF;
	sprava[0] = '\0';

	while (c != 'y' ) {
		
		printf("Cakam na spravu... ");
		recvlen = recvfrom(sockfd, buf, 500, 0, (struct sockaddr *)&klientAdresa, &slen);
		
		if (recvlen == -1) {
			perror("cannot recvfrom()");
		}
		else
		{
			if (strcmp(buf, "Success") == 0) {
				sprava = (char *)realloc(sprava, strlen(sprava) + strlen("_END_") + 1);
				sprava[strlen(sprava) + strlen("_END_") + 1] = '\0';
				//buf[recvlen] = '\0';
				strcat(sprava, "_END_");
				printf("Cela sprava : %s\n\n", sprava);
				sprava[0] = '\0';
			
				if (prepnutie(sockfd, &c) == 1) {
					c = 'y';
					closesocket(sockfd);
					WSACleanup();
					break;
				}
				break;
			}
			else {
				fragment = "";
				hlavicka = (HEADER *)buf;
				fragment = buf + sizeof(HEADER);
				
				int aktualnyFragment = hlavicka->poradie;
				int velkostF = hlavicka->velkostF;
				int checksum = hlavicka->check;			
				buf[recvlen] = '\0';
				printf(" Prijaty fragment %d. s velkostou %dB\n", aktualnyFragment, velkostF + sizeof(HEADER));
				ackMessage = (char *)malloc(sizeof(HEADER));

					hlavicka = (HEADER *)ackMessage;
					hlavicka->check = crcFast(fragment, velkostF);
					hlavicka->poradie = aktualnyFragment;
					hlavicka->velkostF = velkostF;
					
					if (checksum != -2) {
						sprava = (char *)realloc(sprava, strlen(sprava) + velkostF + 1);
						sprava[strlen(sprava) + velkostF + 1] = '\0';
						//buf[recvlen] = '\0';
						strcat(sprava, fragment);
					}
					else
						printf("Chybne odoslany paket. Znovuvyziadanie.\n");

					sendto(sockfd, ackMessage, sizeof(HEADER), 0, (struct sockaddr *)&klientAdresa, slen);
					fragment = "";
			}
		}	
	}
	closesocket(sockfd);
	WSACleanup();
	return 0;
}
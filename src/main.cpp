#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

const int MAX_CLIENTS = 9;

void handleClient(int clientSocket, std::string serverPassword) {
    (void)serverPassword;
    // Gestisci la comunicazione con il client qui.
    // In questa implementazione di base, inviamo un messaggio di benvenuto.
    std::string welcomeMessage = "Benvenuto al server IRC!\n";
    send(clientSocket, welcomeMessage.c_str(), strlen(welcomeMessage.c_str()), 0);

    char buffer[1024];
    int bytesRead;

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        // Gestisci i comandi e le comunicazioni del client qui.
        // In questa implementazione di base, stampiamo i messaggi ricevuti.
        write(1, buffer, bytesRead);  // Stampa i dati ricevuti sul terminale del server.
    }

    close(clientSocket);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error in socket creation");
        return 2;
    }

    int port = std::atoi(argv[1]);
    std::string password = argv[2];

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Error in binding");
        return 3;
    }

    if (listen(serverSocket, MAX_CLIENTS) == -1) {
        perror("Error in listening");
        return 4;
    }

    std::cout << "Server IRC running in port: " << port << std::endl;

    while (true) {
        struct sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);

        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);

        if (clientSocket == -1) {
            perror("Error in accepting the connection");
            continue;
        }

        // Gestisci il client in un nuovo processo o thread.
        handleClient(clientSocket, password);
    }

    close(serverSocket);

    return 0;
}
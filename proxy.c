#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/socket.h>
#include  <netdb.h>
#include  <string.h>
#include  <unistd.h>
#include  <stdbool.h>
#include "./simpleSocketAPI.h"
#include <poll.h>

#define SERVADDR "127.0.0.1"        // Définition de l'adresse IP d'écoute
#define SERVPORT "0"                // Définition du port d'écoute, si 0 port choisi dynamiquement
#define LISTENLEN 1                 // Taille de la file des demandes de connexion
#define MAXBUFFERLEN 8192           // Taille du tampon pour les échanges de données
#define MAXBUFFERLENDATA 65536      // Taille du tampon pour les gros échanges de données
#define MAXHOSTLEN 64               // Taille d'un nom de machine
#define MAXPORTLEN 64               // Taille d'un numéro de port
#define DELAI 300                   //Delai maximum client en secondes
#define REMOTEPORT "21"             //Port du serveur distant


int calculPort (char port[], char* adresse, char delim[]); // Fonction calcul du port
int checkDelay(int descSock,int delai); //Fonction delai client

int main(){
    int ecode;                         // Code retour des fonctions
    char serverAddr[MAXHOSTLEN];       // Adresse du serveur
    char serverPort[MAXPORTLEN];       // Port du server
    int descSockRDV;                   // Descripteur de socket de rendez-vous
    int descSockCOM;                   // Descripteur de socket de communication
    struct addrinfo hints;             // Contrôle la fonction getaddrinfo
    struct addrinfo *res;              // Contient le résultat de la fonction getaddrinfo
    struct sockaddr_storage myinfo;    // Informations sur la connexion de RDV
    struct sockaddr_storage from;      // Informations sur le client connecté
    socklen_t len;                     // Variable utilisée pour stocker les 
				                       // longueurs des structures de socket
    char buffer[MAXBUFFERLEN];         // Tampon de communication entre le client et le serveur
    char bufferDATA[MAXBUFFERLENDATA]; // Tampon de communication de données entre le client et le serveur
    
    // Initialisation de la socket de RDV IPv4/TCP
    descSockRDV = socket(AF_INET, SOCK_STREAM, 0);
    if (descSockRDV == -1) {
         perror("Erreur création socket RDV\n");
         exit(2);
    }
    // Publication de la socket au niveau du système
    // Assignation d'une adresse IP et un numéro de port
    // Mise à zéro de hints
    memset(&hints, 0, sizeof(hints));
    // Initialisation de hints
    hints.ai_flags = AI_PASSIVE;      // mode serveur, nous allons utiliser la fonction bind
    hints.ai_socktype = SOCK_STREAM;  // TCP
    hints.ai_family = AF_INET;        // seules les adresses IPv4 seront présentées par 
				                      // la fonction getaddrinfo

     // Récupération des informations du serveur
     ecode = getaddrinfo(SERVADDR, SERVPORT, &hints, &res);
     if (ecode) {
         fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(ecode));
         exit(1);
     }
     // Publication de la socket
     ecode = bind(descSockRDV, res->ai_addr, res->ai_addrlen);
     if (ecode == -1) {
         perror("Erreur liaison de la socket de RDV");
         exit(3);
     }
     // Nous n'avons plus besoin de cette liste chainée addrinfo
     freeaddrinfo(res);

     // Récuppération du nom de la machine et du numéro de port pour affichage à l'écran
     len=sizeof(struct sockaddr_storage);
     ecode=getsockname(descSockRDV, (struct sockaddr *) &myinfo, &len);
     if (ecode == -1)
     {
         perror("SERVEUR: getsockname");
         exit(4);
     }
     ecode = getnameinfo((struct sockaddr*)&myinfo, sizeof(myinfo), serverAddr,MAXHOSTLEN, 
                         serverPort, MAXPORTLEN, NI_NUMERICHOST | NI_NUMERICSERV);
     if (ecode != 0) {
             fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(ecode));
             exit(4);
     }
     printf("L'adresse d'ecoute est: %s\n", serverAddr);
     printf("Le port d'ecoute est: %s\n", serverPort);

     // Definition de la taille du tampon contenant les demandes de connexion
     ecode = listen(descSockRDV, LISTENLEN);
     if (ecode == -1) {
         perror("Erreur initialisation buffer d'écoute");
         exit(5);
     }

	len = sizeof(struct sockaddr_storage);
     // Attente connexion du client
     // Lorsque demande de connexion, creation d'une socket de communication avec le client
    int pid = 0;
    while (pid ==0) {
     descSockCOM = accept(descSockRDV, (struct sockaddr *) &from, &len);
     if (descSockCOM == -1){
         perror("Erreur accept\n");
         exit(6);
     }

    // Echange de données avec le client connecté
    /** Testez de mettre 220 devant BLABLABLA ... **/
    strcpy(buffer, "220 BLABLABLA CAR je sais c'est pas drole\n");
    write(descSockCOM, buffer, strlen(buffer));
    
    /*********************************/
    /***** A vous de continuer ! *****/
    bool commandOK = false;
    char  servername [MAXHOSTLEN];
    char username [MAXHOSTLEN]  ;
    char* pointeurUser;
    char* pointeurServ;
    char* separateurs = " @";
    int descSockServer;
    int descSockDataClient;
    int descSockDataServeur;
    int source, destination;
    
    //Etablir la connection 
    printf("LECTURE\n");
    while (!commandOK) {
        ecode = checkDelay(descSockCOM,DELAI);
        // Vérifier si le délai est dépassé, si oui fermer la connexion
    	if (ecode == 0) {
           strcpy(buffer,"Erreur: 421 Delai depasse, fin de la connexion\n");
           write(descSockCOM, buffer, strlen(buffer));
           close(descSockCOM);
           exit(1);
    	}
        //Lecture du message envoyé par le client
        ecode = read(descSockCOM,buffer,MAXBUFFERLEN);
        buffer[ecode] = '\0';
        printf("Client : %s",buffer);
        //Extraire les informations utilisateur et serveur à partir de la commande envoyée par le client
        strtok(buffer,separateurs);
        pointeurUser = strtok(NULL,separateurs);
        pointeurServ = strtok(NULL,separateurs);
        //Gérer les cas d'erreur 
        if (pointeurUser == NULL||pointeurServ == NULL) {
           strcpy(buffer, "520 Erreur Connectez-vous avec USER utilisateur@servername\n");
           write(descSockCOM, buffer, strlen(buffer));
           continue;
        }
        else {
           // Extraction de l'utilisateur et du nom de serveur
           strcpy (username,pointeurUser);
           printf("utilisateur : %s\n",username);
           strncpy (servername,pointeurServ,strlen(pointeurServ)-2) ; 
           printf("serveur : %s\n",servername);
           commandOK = true;
        }
        // Vérification si la commande saisie est correcte
        if (commandOK) {
            // Tentative de connexion au serveur FTP
            ecode = connect2Server(servername,"21",&descSockServer);
            printf("taille %s %d\n", servername, strlen(servername));
        }
        // Si la connexion échoue
        if (ecode ==-1) {
            strcpy(buffer, "520 Erreur Impossible de joindre le serveur\n");
            write(descSockCOM, buffer, strlen(buffer));
            commandOK = false;
            continue;
        }
        // Si la connexion réussit
        else {
            printf("Connexion reussie!\n");
            commandOK = true;
        }
    }
    // Affichage des informations de connexion
    printf("utilisateur : %s\n",username);
    printf("serveur :  %s\n",servername);

    //Lecture message bienvenue serveur
    ecode = read(descSockServer,buffer,MAXBUFFERLEN);
    //Authentification 
    strcpy(buffer, "USER ");
    strcat(buffer, username);
    strcat(buffer,"\n");
    printf("%s",buffer);
    // Envoi de la commande au serveur
    write(descSockServer, buffer, strlen(buffer));
    // Lecture de la réponse du serveur
    ecode = read(descSockServer,buffer,MAXBUFFERLEN);
    buffer[ecode] = '\0';
    // Affichage de la réponse du serveur
    printf("Serveur: %s\n",buffer);
    //Verification de demande de mot de passe
    commandOK = (strncmp (buffer,"331",3) !=0);
    if (!commandOK) {
        // Envoi de la demande de mot de passe au client
        write(descSockCOM, buffer, strlen(buffer));
    }
    while (!commandOK) {
        //Lecture PASS du client
        ecode = checkDelay(descSockCOM,DELAI);
        if (ecode == 0) {
            strcpy(buffer,"421 Delai depasse, fin de la connexion\n");
            write(descSockCOM, buffer, strlen(buffer));
            close(descSockCOM);
            exit(1);
        }
        //Lecture du mot de passe du client
        ecode = read(descSockCOM,buffer,MAXBUFFERLEN);
        buffer[ecode] = '\0';
        printf("Client : %s",buffer);

        //Envoi du mot de passe au serveur
        write(descSockServer,buffer,strlen(buffer));

        //Lecture reponse serveur
        ecode = read(descSockServer,buffer,MAXBUFFERLEN);
        buffer[ecode] = '\0';
        printf("Serveur: %s\n",buffer);

        if (strncmp (buffer,"530",3) ==0) {
            strcpy(buffer, "530 Mot de passe incorrect\n");
            write(descSockCOM, buffer, strlen(buffer));
            continue;
        }
        if (strncmp (buffer,"230",3) ==0) {
            strcpy(buffer, "230 Mot de passe correct\n");
            write(descSockCOM, buffer, strlen(buffer));
            commandOK = true;
            continue;
        }
        //Renvoi du message serveur
        else  {
        write(descSockCOM, buffer, strlen(buffer));
        }
    }

    //Boucle jusqu'a fermeture explicite du client
    while (strncmp (buffer,"QUIT",4) !=0) {
        //lecture client de la commande PORT
        ecode = checkDelay(descSockCOM,DELAI);
        if (ecode == 0) {
            strcpy(buffer,"421 Delai depasse, fin de la connexion\n");
            write(descSockCOM, buffer, strlen(buffer));
            close(descSockCOM);
            exit(1);
        }
        //Lecture reponse serveur
        ecode = read(descSockCOM,buffer,MAXBUFFERLEN);
        buffer[ecode] = '\0';
        printf("Client : %s",buffer);
       
        commandOK = (strncmp (buffer,"PORT",4) ==0 || strncmp (buffer,"QUIT",4) ==0);

        if (!commandOK) {
            write(descSockServer, buffer, strlen(buffer));
        }

        while (!commandOK) {
            //Envoi de la commande au serveur si pas port
            sleep(1);
            ecode = recv(descSockServer,buffer,MAXBUFFERLEN,0);
            buffer[ecode] = '\0';
            printf("Serveur : %s",buffer);
            ecode = send(descSockCOM, buffer, strlen(buffer),0);
            printf("Données renvoyées (%d  octets)\n",ecode);
	    //Verifier le delai
            ecode = checkDelay(descSockCOM,DELAI);
            if (ecode == 0) {
                strcpy(buffer,"421 Delai depasse, fin de la connexion\n");
                write(descSockCOM, buffer, strlen(buffer));
                close(descSockCOM);
                exit(1);
            }
            //Lecture de la reponse client
            ecode = read(descSockCOM,buffer,MAXBUFFERLEN);
            buffer[ecode] = '\0';
            printf("Client: %s\n",buffer);

            if (strncmp (buffer,"PORT",4) ==0 || strncmp (buffer,"QUIT",4) ==0) {
                commandOK = true;
                continue;
            }
            else {
            write(descSockServer,buffer,strlen(buffer));
            }
        }

        //Cas d'arret deconnexion client
        if (strncmp (buffer,"QUIT",4) ==0) {
            continue;
        }
        //Traduction du port Client
        printf("---PORT---\n");
        char chainePORT[MAXBUFFERLEN];
        strcpy(chainePORT,buffer);
	    // Création de variables pour stocker le numéro de port et l'adresse traduit
        char portDataClient[6];
        char adresseDataClient[MAXBUFFERLEN];
        // Définition des délimiteurs pour la fonction strtok
        char delim[] = " ,"; 
	    // Appel de la fonction calculPort pour traduire le port client
        int portDataTraduit = calculPort(chainePORT,adresseDataClient,delim);
        // Conversion du numéro de port en chaîne de caractères
        sprintf(portDataClient,"%d",portDataTraduit);
        printf("Adresse = [%s] Port = [%s] \n",adresseDataClient,portDataClient);
        printf("CREATION DE LA SOCKET DONNEES CLIENT \n");
        // Initialisation des variables pour la connexion de données
        char addresseclientbordel[15];
        // Copie de l'adresse IP du client pour utilisation ultérieure
        strncpy(addresseclientbordel,adresseDataClient,strlen(adresseDataClient));
        ecode = connect2Server(addresseclientbordel,portDataClient,&descSockDataClient);
        // En cas d'échec de la connexion
        if (ecode == -1) {
            perror("erreur socket données");exit(2);
        }
        printf("Success de la sock donnees\n");
        
        //Passage en mode PASSIF cote SERVEUR
        printf("PASV \n");
        strcpy(buffer, "PASV\n");
        write(descSockServer, buffer, strlen(buffer));
        printf("REPONSE SERVEUR\n");
        ecode = read(descSockServer,buffer,MAXBUFFERLEN);
        buffer[ecode] = '\0';
        printf("Serveur: %s\n",buffer);

        //Traduction du port Serveur
        char portSERV[6];
        char adresseSERV[MAXBUFFERLEN];
        //Délimiteur pour extraire l'adresse et le numéro de port
        strncpy(delim,"(,",2);
        //Copie le message du serveur dans une variable pour le traitement
        strcpy(chainePORT,buffer);
        //Appel de la fonction pour calculer le numéro de port final
        int portdonneesSERV = calculPort(chainePORT,adresseSERV,delim);
        printf("port traduit = %d\n",portdonneesSERV);
        printf("Adresse = [%s] taille = %d\n",adresseSERV,strlen(adresseSERV));
        sprintf(portSERV,"%d",portdonneesSERV);
        printf("Port en string [%s]",portSERV);
        buffer[ecode] = '\0';
        printf("Serveur: %s\n",buffer);
        printf("CREATION SOCKET DONNEES SERVEUR\n");
        //Création du socket pour les données avec l'adresse et le port obtenus
        ecode = connect2Server(adresseSERV,portSERV,&descSockDataServeur);
        if (ecode == -1) {
            perror("erreur socket données serveur");exit(2);
        }
        //Confirmation de la création de la socket pour les données côté serveur
        printf("Success de la sock donnees serveur\n");
        printf("TYPE I \n");
        strcpy(buffer, "TYPE I\n");
        write(descSockServer, buffer, strlen(buffer));

        // Lecture de la reponse client
        ecode = read(descSockServer,buffer,MAXBUFFERLEN);
        buffer[ecode] = '\0';
        printf("Serveur: %s\n",buffer);
        
	    // Creation Copie
        strcpy(buffer, "200 Commande PORT effectue avec succes\n");
        write(descSockCOM, buffer, strlen(buffer));

        //Connection de données réussie
        ecode = checkDelay(descSockCOM,DELAI);
        if (ecode == 0) {
            strcpy(buffer,"421 Delai depasse, fin de la connexion\n");
            write(descSockCOM, buffer, strlen(buffer));
            close(descSockCOM);
            exit(1);
        }
        //Lecture de la reponse client
        ecode = read(descSockCOM,buffer,MAXBUFFERLEN);
        buffer[ecode] = '\0';
        printf("Client: %s\n",buffer);

        if (strncmp,(buffer,"QUIT",3) == 0) {
            continue;
        }

        //Renvoi commande au serveur
        write(descSockServer, buffer, strlen(buffer));
	    // Lecture de la réponse du serveur
        ecode = read(descSockServer,buffer,MAXBUFFERLEN);

        // Définition du sens du transfert : source = serveur, destination = client
        source = descSockDataServeur;
        destination = descSockDataClient;
        // Boucle pour gérer les commandes du client tant qu'une erreur n'est pas rencontrée ou que le client n'envoie pas la commande QUIT 
        while ((strncmp(buffer,"150",3)) != 0 && (strncmp(buffer,"5",1)) != 0) {
            write(descSockCOM, buffer, strlen(buffer));
	        // Verification du delai
            ecode = checkDelay(descSockCOM,DELAI);
            if (ecode == 0) {
                strcpy(buffer,"421 Delai depasse, fin de la connexion\n");
                write(descSockCOM, buffer, strlen(buffer));
                close(descSockCOM);
                exit(1);
            }
            ecode = read(descSockCOM,buffer,MAXBUFFERLEN);
            buffer[ecode] = '\0';
            printf("Client: %s\n",buffer);
            if (strncmp,(buffer,"QUIT",4) == 0) {
            break;
            }
            //Si le client veut stocker, inversion du sens
            if (strncmp,(buffer,"STOR",4) == 0) {
            destination = descSockDataServeur;
            source = descSockDataClient;
            }
            write(descSockServer, buffer, strlen(buffer));
	        //Lecture de la reponse client
            ecode = read(descSockServer,buffer,MAXBUFFERLEN);
            buffer[ecode] = '\0';
            printf("Serveur: %s\n",buffer);
        }
	    // Si le client envoie la commande QUIT, on ferme les sockets de données et on continue
        if (strncmp,(buffer,"QUIT",4) == 0) {
            close(descSockDataClient);
            close(descSockDataServeur);
            continue;
            }
        //Erreur de la part du serveur
        if (strncmp(buffer,"5",1) == 0) {
                write(descSockCOM, buffer, strlen(buffer));
                close(descSockDataClient);
                close(descSockDataServeur);
                continue;
            }
        write(descSockCOM, "150 Ouverture de la connexion\n", 31);
        
        //Lecture non bloquante
        ecode = recv(source,bufferDATA,MAXBUFFERLENDATA,MSG_DONTWAIT);
        bufferDATA[ecode] = '\0';
        int erreur = 0;
        // Boucle de lecture des données depuis la source
        while (ecode != 0 && erreur <= 3) {
            //Envoi des données au client
            ecode = send(destination, bufferDATA, strlen(bufferDATA),0);
            printf("Données renvoyées (%d  octets)\n",ecode);
            //Récupération des données depuis le serveur
            ecode = recv(source,bufferDATA,MAXBUFFERLENDATA,MSG_DONTWAIT);
            printf("Recuperation donnees depuis serveur (%d octets) \n", ecode);
            bufferDATA[ecode] = '\0';
            if (ecode ==-1) {
                usleep(500000);
                erreur++;
            }
            else {
                erreur = 0;
            }
            //Gestion de la fin de la connexion
            if (ecode ==0) {
                printf("Connexion fermée \n");
            }
        }

	    //Fermeture des descripteurs de socket utilisés pour l'échange de données
        close(descSockDataClient);
        close(descSockDataServeur);
        //Envoi du message de confirmation de fin de transfert au client
        strcpy(buffer,"226 Transfert termine\n");
        write(descSockCOM, buffer, strlen(buffer));
        ecode = read(descSockServer,buffer,MAXBUFFERLEN);
    }

    strcpy(buffer,"221 Connexion terminee\n");
    write(descSockCOM,buffer,strlen(buffer));

    //Fermeture de la connexion
    close(descSockCOM);
    close(descSockRDV);
    close(descSockServer);
    exit(0);
}

int calculPort(char port[],char *adresse,char delim[]) {
    // Initialisation des variables
    char *ptr = strtok(port,delim);
    ptr = strtok(NULL,delim);
    adresse[0] = '\0';
    	// Boucle pour extraire l'adresse IP
        for (size_t i = 0; i < 3; i++)
        {
            // Concaténation des octets de l'adresse IP
            strcat(adresse,ptr);
            strcat(adresse,".");
            ptr = strtok(NULL,delim);
        }
        // Ajout de l'octet restant
        strcat(adresse,ptr);
        // Extraction des numéros de ports
        ptr = strtok(NULL,delim);
        int port1 = strtol(ptr,NULL,10);
        printf("port1 = %d ",port1);
        ptr = strtok(NULL,delim);
        int port2 = strtol(ptr,NULL,10);
        printf("port2 = %d \n",port2);
	    // Calcul du numéro de port final
        return port1*256 + port2;
}

int checkDelay(int descSock, int delai) {
    struct pollfd fds;
    fds.fd = descSock;
    fds.events = POLLIN;
    fds.revents = 0;
    // Utiliser la fonction poll pour vérifier si un événement est survenu sur la socket donnée dans un délai donné
    int res = poll(&fds,1,delai*1000);
    // retourne le nombre d'événements survenus
    return res;
}

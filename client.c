#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "cJSON.h"

void printResponse(cJSON *profiles_array, char* print_type){ // response dealer

    printf("\n--------------------------RESULTADOS-----------------------------------\n");

    int num_profiles = cJSON_GetArraySize(profiles_array);
    if (num_profiles == 0) { // verifies if there is a least one profile
        printf("\nNenhum perfil encontrado!\n");
    }

    for(int i = 0; i < num_profiles; i++){ // iterates over every profile and gathers required informations 

        if (i > 0){
            printf("\n-------------------------------------------------------------\n");
        }

        cJSON *profile = cJSON_GetArrayItem(profiles_array, i);
        cJSON *name = cJSON_GetObjectItem(profile, "nome");
        cJSON *email = cJSON_GetObjectItem(profile, "email");

        // generic information
        printf("\nNOME: %s\n", name->valuestring);
        printf("EMAIL: %s\n", email->valuestring);


        if (strcmp(print_type, "year") == 0) { // info of year of graduation
            
            cJSON *course = cJSON_GetObjectItem(profile, "formacao");
            printf("FORMAÇÃO: %s\n", course->valuestring);


        } else if (strcmp(print_type, "all") == 0) { // all info

            cJSON *course = cJSON_GetObjectItem(profile, "formacao");
            cJSON *sobrenome = cJSON_GetObjectItem(profile, "sobrenome");
            cJSON *cidade = cJSON_GetObjectItem(profile, "cidade");
            cJSON *ano_formatura = cJSON_GetObjectItem(profile, "ano_formatura");

            printf("SOBRENOME: %s\n", sobrenome->valuestring);
            printf("CIDADE: %s\n", cidade->valuestring);
            printf("FORMAÇÃO: %s\n", course->valuestring);
            printf("ANO DE FORMAÇÃO: %d\n", ano_formatura->valueint);
            printf("HABILIDADES:\n");

            cJSON *skills_array = cJSON_GetObjectItem(profile, "habilidades");
            int num_skills = cJSON_GetArraySize(skills_array);
            
            for(int j = 0; j < num_skills; j++) { // printing skills one by one
                cJSON *skill = cJSON_GetArrayItem(skills_array, j);
                printf("    %s\n", skill->valuestring);
            }
        }
    }
}

int main() {

    typedef struct { //struct containing profiles' info
        char email[50];
        char nome[50];
        char sobrenome[50];
        char cidade[50];
        char formacao[50];
        int ano_formatura;
        char habilidades[100];
    } Profile;

    typedef struct { //struct containing the messages' payload 
        char action[50];
        char message[500];
    } Payload;

    char *ip = "127.0.0.1"; // local address
    int port = 5564; 

    struct sockaddr_in client_address; 
    int client_socket = socket(AF_INET, SOCK_STREAM, 0); // defining the client as a TCP socket with IPv4

    if (client_socket < 0) { // verifies if there is an error in the socket's creation
        perror("Erro ao criar socket do cliente");
        exit(1);
    }

    struct sockaddr_in server_address; 
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = port;

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) { // connecting the client and the server
        perror("Erro ao conectar ao servidor");
        return 1;
    }

    char buffer[10000];
    bzero(buffer, 10000);
    int read_size = 0;
    printf("  _________\n /         \\\n |  /\\ /\\  |\n |    -    |\n |  \\___/  |\n \\_________/");
    printf("\n\nBEM VINDO!");

    while (1) { // loop of requests for the client

        int opcao;
        // options of request
        printf("\n--------------------- MENU INICIAL --------------------------\n");
        printf("\nEscolha uma opcao:\n");
        printf("1 - Cadastro\n");
        printf("2 - Coletar perfis através do curso\n");
        printf("3 - Coletar perfis através das habilidades\n");
        printf("4 - Coletar perfis através do ano de formação\n");
        printf("5 - Coletar todos os perfis\n");
        printf("6 - Coletar informações de um perfil\n");
        printf("7 - Remover perfil\n");
        printf("8 - Desconectar e encerrar\n");

        printf("\nDigite a sua escolha (1-8): ");
        scanf("%d", &opcao);

        Payload payload;
        cJSON *profiles_array; 
        cJSON *jsonPayload;

        switch(opcao) {
            case 1: // register a new profile
                Profile profile;
                strcpy(payload.action, "register");

                printf("\nPara realizar o cadastro precisaremos de algumas informações: \n");

                printf("Digite o seu email: ");
                scanf("%s", profile.email);

                printf("Digite o seu nome: ");
                scanf("%s", profile.nome);

                printf("Digite o seu sobrenome: ");
                scanf("%s", profile.sobrenome);

                printf("Digite a sua cidade: ");
                scanf("%s", profile.cidade);

                printf("Digite a sua formacao: ");
                scanf("%s", profile.formacao);

                printf("Digite o ano de formatura: ");
                scanf("%d", &profile.ano_formatura);

                char input_message[1000];
                printf("Digite as suas habilidades separados por vírgula: ");
                scanf("%s", input_message);
                
                char *current_skill; // creating skills' array
                char skills[1000] = "[";

                current_skill = strtok(input_message, ",");
                int skill_counter = 0;

                while (current_skill != NULL) { //

                    if (skill_counter > 0){
                        strcat(skills, ",");
                    }
                    strcat(skills, "\"");
                    strcat(skills, current_skill);
                    strcat(skills, "\"");

                    current_skill = strtok(NULL, ",");
                    skill_counter++;
                }

                strcat(skills, "]");
                strcpy(profile.habilidades, skills);

                sprintf(payload.message, "{\"email\": \"%s\", \"nome\": \"%s\", \"sobrenome\": \"%s\", \"cidade\": \"%s\", \"formacao\": \"%s\", \"ano_formatura\": %d, \"habilidades\": %s}",
                profile.email, profile.nome, profile.sobrenome, profile.cidade, profile.formacao, profile.ano_formatura, profile.habilidades); // defining message structure

                sprintf(buffer, "{\"action\": \"%s\", \"message\": %s}",
                payload.action, payload.message);
                send(client_socket, buffer, strlen(buffer),0); // sending message and action

                bzero(buffer, 10000);
                read_size = recv(client_socket, buffer, sizeof(buffer),0); // receiving result

                if (read_size <= 0) {
                    break;
                }

                buffer[read_size] = '\0';
                printf("\n%s\n", buffer);
                break;

            case 2: // search profiles by course
                
                strcpy(payload.action, "getAllProfilesByCourse");
                printf("\nDigite o curso selecionado: ");
                scanf("%s", payload.message);
                
                // SEND
                sprintf(buffer, "{\"action\": \"%s\", \"message\": \"%s\"}",
                payload.action, payload.message);
                send(client_socket, buffer, strlen(buffer),0);

                // RESPONSE
                bzero(buffer, 10000);
                read_size = recv(client_socket, buffer, sizeof(buffer),0);

                if (read_size <= 0) {
                    break;
                }


                buffer[read_size] = '\0';
                jsonPayload = cJSON_Parse(buffer);
                if (jsonPayload == NULL) {
                    printf("Erro ao fazer o parse do JSON.\n");
                    break;
                }

                profiles_array = cJSON_GetObjectItem(jsonPayload, "profiles");

                printResponse(profiles_array, "course");

                break;

            case 3: // search profiles by skill
                
                strcpy(payload.action, "getAllProfilesBySkill");

                printf("\nDigite a habilidade selecionada: ");
                scanf("%s", payload.message);
                
                // SEND
                sprintf(buffer, "{\"action\": \"%s\", \"message\": \"%s\"}",
                payload.action, payload.message);
                send(client_socket, buffer, strlen(buffer),0);

                // RESPONSE
                bzero(buffer, 10000);
                read_size = recv(client_socket, buffer, sizeof(buffer),0);

                if (read_size <= 0) {
                    break;
                }

                buffer[read_size] = '\0';
                jsonPayload = cJSON_Parse(buffer);
                if (jsonPayload == NULL) {
                    printf("\nErro ao fazer o parse do JSON.\n");
                    break;
                }

                profiles_array = cJSON_GetObjectItem(jsonPayload, "profiles");

                printResponse(profiles_array, "skill");

                break;

            case 4: // seach profiles by year of graduation
                
                strcpy(payload.action, "getAllProfilesByYear");

                int year_value;
                printf("\nDigite o ano de formatura selecionado: ");

                while (scanf("%d", &year_value) != 1) {
                    printf("Entrada inválida -> digite o ano de formação (ex: 2023)");
                    while (getchar() != '\n');
                    printf("\nDigite o ano de formatura selecionado: ");
                }
                
                // SEND
                sprintf(buffer, "{\"action\": \"%s\", \"message\": %d}",
                payload.action, year_value);
                
                send(client_socket, buffer, strlen(buffer),0);

                // RESPONSE
                bzero(buffer, 10000);
                read_size = recv(client_socket, buffer, sizeof(buffer),0);

                if (read_size <= 0) {
                    break;
                }

                buffer[read_size] = '\0';
                jsonPayload = cJSON_Parse(buffer);
                if (jsonPayload == NULL) {
                    printf("Erro ao fazer o parse do JSON.\n");
                    break;
                }

                profiles_array = cJSON_GetObjectItem(jsonPayload, "profiles");

                printResponse(profiles_array, "year");
                               
                break;

            case 5: // get all profiles
                
                strcpy(payload.action, "getAllProfiles");
                strcpy(payload.message, "");

                // SEND
                sprintf(buffer, "{\"action\": \"%s\", \"message\": \"%s\"}",
                payload.action, payload.message);
                
                send(client_socket, buffer, strlen(buffer),0);

                // RESPONSE
                bzero(buffer, 10000);
                read_size = recv(client_socket, buffer, sizeof(buffer),0);

                if (read_size <= 0) {
                    break;
                }

                buffer[read_size] = '\0';
                jsonPayload = cJSON_Parse(buffer);
                if (jsonPayload == NULL) {
                    printf("Erro ao fazer o parse do JSON.\n");
                    break;
                }

                profiles_array = cJSON_GetObjectItem(jsonPayload, "profiles");

                printResponse(profiles_array, "all");

                break;

            case 6: // get the profile of an email
                
                strcpy(payload.action, "getProfile");

                printf("\nDigite o email do perfil desejado: ");
                scanf("%s", payload.message);
                
                // SEND
                sprintf(buffer, "{\"action\": \"%s\", \"message\":  \"%s\"}",
                payload.action, payload.message);
                
                send(client_socket, buffer, strlen(buffer),0);

                // RESPONSE
                bzero(buffer, 10000);
                read_size = recv(client_socket, buffer, sizeof(buffer),0);

                if (read_size <= 0) {
                    break;
                }

                buffer[read_size] = '\0';
                jsonPayload = cJSON_Parse(buffer);
                if (jsonPayload == NULL) {
                    printf("Erro ao fazer o parse do JSON.\n");
                    break;
                }

                profiles_array = cJSON_GetObjectItem(jsonPayload, "profiles");

                printResponse(profiles_array, "all");
                break;

            case 7: // remove a profile
                
                strcpy(payload.action, "removeProfile");

                printf("\nDigite o email do perfil a ser removido: ");
                scanf("%s", payload.message);
                
                // SEND
                sprintf(buffer, "{\"action\": \"%s\", \"message\":  \"%s\"}",
                payload.action, payload.message);
                
                send(client_socket, buffer, strlen(buffer),0);

                // RESPONSE
                bzero(buffer, 10000);
                read_size = recv(client_socket, buffer, sizeof(buffer),0);

                if (read_size <= 0) {
                    break;
                }

                buffer[read_size] = '\0';
                printf("\n%s\n", buffer);
                break;

            case 8: // exit program and close connection

                close(client_socket);
                printf("\nVocê foi desconectado.\n");
                printf("  _________\n /         \\\n |  /\\ /\\  |\n |    -    |\n |  \\___/  |\n \\_________/");
                printf("\n\nVOLTE SEMPRE!\n");
                return 1;

            default: // non-existing option
                printf("\nOpção inválida.\n");
                break;
        }
 }

    close(client_socket);

    return 0;
}

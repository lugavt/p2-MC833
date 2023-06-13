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
    int port = 5555; 
    
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0); // defining the client as a UDP socket with IPv4
    struct sockaddr_in client_address; 
    char buffer[10000];
    socklen_t client_address_size; 

    memset(&client_address, 0, sizeof(client_address));
    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = inet_addr(ip);
    client_address.sin_port = port;

    int read_size = 0;
    printf("  _________\n /         \\\n |  /\\ /\\  |\n |    -    |\n |  \\___/  |\n \\_________/");
    printf("\n\nBEM VINDO!");


while(1){    
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
    printf("8 - Buscar imagem dado um email\n");
    printf("9 - Desliga cliente\n");

    printf("\nDigite a sua escolha (1-9): ");
    scanf("%d", &opcao);

    Payload payload;
    cJSON *profiles_array; 
    cJSON *jsonPayload;
            
    bzero(buffer, 10000);

    switch (opcao) {
        case 1: // register a new profile
        {
            Profile profile;
            strcpy(payload.action, "register");

            printf("\nPara realizar o cadastro precisaremos de algumas informações: \n");
            
            getchar();

            printf("Digite o seu email: ");
            //    scanf("%s", profile.email);
            fgets(profile.email, sizeof(profile.email), stdin);
            
            size_t length = strlen(profile.email);
            if (length > 0 && profile.email[length - 1] == '\n') profile.email[length - 1] = '\0';  // Substitui o '\n' por '\0'

            printf("Digite o seu nome: ");
            fgets(profile.nome, sizeof(profile.nome), stdin);
            length = strlen(profile.nome);
            if (length > 0 && profile.nome[length - 1] == '\n') profile.nome[length - 1] = '\0';  // Substitui o '\n' por '\0'

            printf("Digite o seu sobrenome: ");
            fgets(profile.sobrenome, sizeof(profile.sobrenome), stdin);
            // scanf("%s", profile.sobrenome);
            length = strlen(profile.sobrenome);
            if (length > 0 && profile.sobrenome[length - 1] == '\n') profile.sobrenome[length - 1] = '\0';  // Substitui o '\n' por '\0'

            printf("Digite a sua cidade: ");
            fgets(profile.cidade, sizeof(profile.cidade), stdin);
            length = strlen(profile.cidade);
            if (length > 0 && profile.cidade[length - 1] == '\n') profile.cidade[length - 1] = '\0';  // Substitui o '\n' por '\0'

            printf("Digite a sua formacao: ");
            fgets(profile.formacao, sizeof(profile.formacao), stdin);
            length = strlen(profile.formacao);
            if (length > 0 && profile.formacao[length - 1] == '\n') profile.formacao[length - 1] = '\0';  // Substitui o '\n' por '\0'

            printf("Digite o ano de formatura: ");
            scanf("%d", &profile.ano_formatura);

            getchar();

            char input_message[1000];
            printf("Digite as suas habilidades separados por vírgula: ");
            fgets(input_message, sizeof(input_message), stdin);
            length = strlen(input_message);
            if (length > 0 && input_message[length - 1] == '\n') input_message[length - 1] = '\0';  // Substitui o '\n' por '\0'
            
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

            sendto(client_socket, buffer, 10000, 0, (struct sockaddr*)&client_address, sizeof(client_address)); // sending profile
        
            bzero(buffer, 10000);
            client_address_size = sizeof(client_address);
            read_size = recvfrom(client_socket, buffer, 10000, 0, (struct sockaddr*)&client_address, &client_address_size); // reading result

            if (read_size <= 0) {
                printf("Erro na mensagem recebida");
            }

            buffer[read_size] = '\0';
            printf("\n%s\n", buffer);
            break;
        }
        case 2: // search profiles by course
        {   strcpy(payload.action, "getAllProfilesByCourse");
            printf("\nDigite o curso selecionado: ");
            scanf("%s", payload.message);
            
            // SEND
            sprintf(buffer, "{\"action\": \"%s\", \"message\": \"%s\"}",
            payload.action, payload.message);
            sendto(client_socket, buffer, 10000, 0, (struct sockaddr*)&client_address, sizeof(client_address));

            // RESPONSE
            bzero(buffer, 10000);
            read_size = recvfrom(client_socket, buffer, 10000, 0, (struct sockaddr*)&client_address, &client_address_size); // reading result

            if (read_size <= 0) {
                printf("Erro na mensagem recebida");
            }


            buffer[read_size] = '\0';
            jsonPayload = cJSON_Parse(buffer);
            if (jsonPayload == NULL) {
                printf("Erro ao fazer o parse do JSON.\n");
                break;
            }

            profiles_array = cJSON_GetObjectItem(jsonPayload, "profiles");

            printResponse(profiles_array, "course");

            break;}

        case 3: // search profiles by skill          
        {   strcpy(payload.action, "getAllProfilesBySkill");

            printf("\nDigite a habilidade selecionada: ");
            scanf("%s", payload.message);
            
            // SEND
            sprintf(buffer, "{\"action\": \"%s\", \"message\": \"%s\"}",
            payload.action, payload.message);
            sendto(client_socket, buffer, 10000, 0, (struct sockaddr*)&client_address, sizeof(client_address));;

            // RESPONSE
            bzero(buffer, 10000);
            read_size = recvfrom(client_socket, buffer, 10000, 0, (struct sockaddr*)&client_address, &client_address_size); // reading result

            if (read_size <= 0) {
                printf("Erro na mensagem recebida");
            }

            buffer[read_size] = '\0';
            jsonPayload = cJSON_Parse(buffer);
            if (jsonPayload == NULL) {
                printf("\nErro ao fazer o parse do JSON.\n");
                break;
            }

            profiles_array = cJSON_GetObjectItem(jsonPayload, "profiles");

            printResponse(profiles_array, "skill");

            break;}

        case 4: // seach profiles by year of graduation         
            {strcpy(payload.action, "getAllProfilesByYear");

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
            
            sendto(client_socket, buffer, 10000, 0, (struct sockaddr*)&client_address, sizeof(client_address));

            // RESPONSE
            bzero(buffer, 10000);
            read_size = recvfrom(client_socket, buffer, 10000, 0, (struct sockaddr*)&client_address, &client_address_size); // reading result

            if (read_size <= 0) {
                printf("Erro na mensagem recebida");
            }

            buffer[read_size] = '\0';
            jsonPayload = cJSON_Parse(buffer);
            if (jsonPayload == NULL) {
                printf("Erro ao fazer o parse do JSON.\n");
                break;
            }

            profiles_array = cJSON_GetObjectItem(jsonPayload, "profiles");

            printResponse(profiles_array, "year");
                            
            break;}
            
        case 5: // get all profiles           
        {   strcpy(payload.action, "getAllProfiles");
            strcpy(payload.message, "");

            // SEND
            sprintf(buffer, "{\"action\": \"%s\", \"message\": \"%s\"}",
            payload.action, payload.message);
            
            sendto(client_socket, buffer, 10000, 0, (struct sockaddr*)&client_address, sizeof(client_address));

            // RESPONSE
            bzero(buffer, 10000);
            read_size = recvfrom(client_socket, buffer, 10000, 0, (struct sockaddr*)&client_address, &client_address_size); // reading result

            if (read_size <= 0) {
                printf("Erro na mensagem recebida");
            }

            buffer[read_size] = '\0';
            jsonPayload = cJSON_Parse(buffer);
            if (jsonPayload == NULL) {
                printf("Erro ao fazer o parse do JSON.\n");
                break;
            }

            profiles_array = cJSON_GetObjectItem(jsonPayload, "profiles");

            printResponse(profiles_array, "all");

            break;}

        case 6: // get the profile of an email         
        {   
            strcpy(payload.action, "getProfile");

            printf("\nDigite o email do perfil desejado: ");
            scanf("%s", payload.message);
            
            // SEND
            sprintf(buffer, "{\"action\": \"%s\", \"message\":  \"%s\"}",
            payload.action, payload.message);
            
            sendto(client_socket, buffer, 10000, 0, (struct sockaddr*)&client_address, sizeof(client_address));

            // RESPONSE
            bzero(buffer, 10000);
            read_size = recvfrom(client_socket, buffer, 10000, 0, (struct sockaddr*)&client_address, &client_address_size); // reading result

            if (read_size <= 0) {
                printf("Erro na mensagem recebida");
            }

            buffer[read_size] = '\0';
            jsonPayload = cJSON_Parse(buffer);
            if (jsonPayload == NULL) {
                printf("Erro ao fazer o parse do JSON.\n");
                break;
            }

            profiles_array = cJSON_GetObjectItem(jsonPayload, "profiles");

            printResponse(profiles_array, "all");
            break;}
        
        case 7: // remove a profile    
        {    strcpy(payload.action, "removeProfile");

            printf("\nDigite o email do perfil a ser removido: ");
            scanf("%s", payload.message);
            
            // SEND
            sprintf(buffer, "{\"action\": \"%s\", \"message\":  \"%s\"}",
            payload.action, payload.message);
            
            sendto(client_socket, buffer, 10000, 0, (struct sockaddr*)&client_address, sizeof(client_address));

            // RESPONSE
            bzero(buffer, 10000);
            read_size = recvfrom(client_socket, buffer, 10000, 0, (struct sockaddr*)&client_address, &client_address_size); // reading result

            if (read_size <= 0) {
                printf("Erro na mensagem recebida");
            }

            buffer[read_size] = '\0';
            printf("\n%s\n", buffer);
            break;}
        case 8: // search an image
        {
            strcpy(payload.action, "seachImage");
            printf("\nDigite o email do perfil para imagem: ");
            scanf("%s", payload.message);

            // SEND
            sprintf(buffer, "{\"action\": \"%s\", \"message\":  \"%s\"}",
            payload.action, payload.message);
            sendto(client_socket, buffer, 10000, 0, (struct sockaddr*)&client_address, sizeof(client_address));

            // RESPONSE
            char profileImagePath[100] = "./imagens-client/";
            strcat(profileImagePath, payload.message);
            strcat(profileImagePath, ".jpg");

            int bytesRead;
            int totalReceived = 0;
            int imagemCriada = 0;

            FILE* file;

            while (1) {
                bzero(buffer,10000);
                ssize_t receivedBytes = recvfrom(client_socket, buffer, 10000, 0, (struct sockaddr*)&client_address, &client_address_size);

                if (strncmp(buffer, "Email não encontrado\n", sizeof("Email não encontrado\n")) == 0) {
                    printf("Email não encontrado\n");
                    break;
                }

                else if (strncmp(buffer, "finished", sizeof("finished")) == 0) {
                    printf("Imagem recebida com sucesso!\nArquivo está localizado no seguinte caminho: %s\n", profileImagePath);
                    break;
                }

                else if (receivedBytes < 0) {
                    printf("Falha ao receber dados da imagem\n");
                    break;
                }

                else if (strncmp(buffer, "Falha ao abrir o arquivo de imagem\n", sizeof("Falha ao abrir o arquivo de imagem\n")) == 0) {
                    printf("Erro no servidor: Falha ao abrir o arquivo de imagem\n");
                    break;
                }
                else if (strncmp(buffer, "Falha ao enviar os dados da imagem\n", sizeof("Falha ao enviar os dados da imagem\n")) == 0) {
                    printf("Erro no servidor: Falha ao enviar os dados da imagem\n");
                    break;
                }
                else if (strncmp(buffer, "Erro ao gerar imagem de perfil\n", sizeof("Erro ao gerar imagem de perfil\n")) == 0) {
                    printf("Erro no servidor: Erro ao gerar imagem de perfil\n");
                    break;
                }
                else{
                    if(imagemCriada == 0){
                        imagemCriada = 1;
                        file = fopen(profileImagePath, "wb");
                        if (file == NULL) {
                            printf("Falha ao criar o arquivo de imagem: %s\n", profileImagePath);
                            break;
                        }           
                    }
                }

                size_t writtenBytes = fwrite(buffer, 1, receivedBytes, file);
                if (writtenBytes < receivedBytes) {
                    printf("Falha ao escrever os dados no arquivo da imagem\n");
                    break;
                }

                totalReceived += receivedBytes;
            }

            if(imagemCriada == 1)  fclose(file);

            break;
        }
        case 9:
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
    return 0;
}

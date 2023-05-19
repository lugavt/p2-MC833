#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "cJSON.h"
#include <stdbool.h>

void* handle_client(void* arg) { // worker to handle client request
    int client_socket = *(int*)arg; // get client socket

    int read_size;    
    char buffer[10000];
    while (1) {
        
        bzero(buffer, 10000); // clear buffer
        read_size = recv(client_socket, buffer, sizeof(buffer),0); // read request message
        
        if (read_size > 0){
            buffer[read_size] = '\0';
            cJSON *jsonPayload = cJSON_Parse(buffer); // parse message to get a JSON object
            if (jsonPayload == NULL) {
                printf("Erro ao fazer o parse do JSON.\n");
                break;
            }

            // get action and message from payload
            cJSON *action = cJSON_GetObjectItem(jsonPayload, "action");
            cJSON *message = cJSON_GetObjectItem(jsonPayload, "message");
            
            // get data from JSON file
            FILE *fp = fopen("data.json", "r");

            char fileBuffer[10000];
            fread(fileBuffer, 1, 10000, fp);
            fclose(fp);

            cJSON *data_json = cJSON_Parse(fileBuffer);
            cJSON *profiles_array = cJSON_GetObjectItem(data_json, "profiles");

            int num_profiles = cJSON_GetArraySize(profiles_array);

            // server actions
            if (strcmp(action->valuestring, "register") == 0){

                int existeId = 0;
                cJSON *inputEmail = cJSON_GetObjectItem(message, "email");

                for(int i = 0; i < num_profiles; i++){ 
                    cJSON *profile = cJSON_GetArrayItem(profiles_array, i);
                    cJSON *email = cJSON_GetObjectItem(profile, "email");
                    if (strcmp(email->valuestring, inputEmail->valuestring) == 0){ // verify if email is already in our database
                        existeId = 1;
                        break;  
                    } 
                }
                if(existeId == 0){
                    // update data file with new profile
                    cJSON_AddItemToArray(profiles_array, message);
                    fp = fopen("data.json", "w");
                    fprintf(fp, "%s", cJSON_PrintUnformatted(data_json));
                    fclose(fp);
                    bzero(buffer, 10000);
                    strcpy(buffer, "Usuário cadastrado com sucesso.\n");
                    send(client_socket, buffer, strlen(buffer), 0);
                }
                else{
                    // error response -> email already in our database
                    bzero(buffer, 10000);
                    strcpy(buffer, "Falha ao cadastrar usuário. Email em uso.\n");
                    send(client_socket, buffer, strlen(buffer), 0);
                }
            }
            else if (strcmp(action->valuestring, "getAllProfilesByCourse") == 0){

                // response in JSON format {"profiles": []}
                char payload[10000];
                char profileJson[200];
                strcpy(payload, "{\"profiles\":[");
                int profilesCounter = 0;

                for(int i = 0; i < num_profiles; i++){
                    cJSON *profile = cJSON_GetArrayItem(profiles_array, i);
                    cJSON *course = cJSON_GetObjectItem(profile, "formacao");
                    if (strcmp(course->valuestring, message->valuestring) == 0){
                        profilesCounter += 1;
                        cJSON *email = cJSON_GetObjectItem(profile, "email");
                        cJSON *name = cJSON_GetObjectItem(profile, "nome");
                        if (profilesCounter > 1){
                            strcat(payload, ",");
                        }
                        sprintf(profileJson, "{\"email\": \"%s\", \"nome\": \"%s\"}",
                        email->valuestring, name->valuestring);
                        strcat(payload, profileJson);
                    } 
                }

                strcat(payload, "]}");
                bzero(buffer, 10000);
                sprintf(buffer, payload);
                send(client_socket, buffer, strlen(buffer), 0);
            }
            else if (strcmp(action->valuestring, "getAllProfilesBySkill") == 0){
                // response in JSON format {"profiles": []}

                char payload[10000];
                char profileJson[200];
                strcpy(payload, "{\"profiles\":[");
                int profilesCounter = 0;

                for(int i = 0; i < num_profiles; i++){

                    cJSON *profile = cJSON_GetArrayItem(profiles_array, i);
                    cJSON *skills_array = cJSON_GetObjectItem(profile, "habilidades");
                    int num_skills = cJSON_GetArraySize(skills_array);

                    for(int j = 0; j < num_skills; j++) {
                        cJSON *skill = cJSON_GetArrayItem(skills_array, j);
                        if (strcmp(skill->valuestring, message->valuestring) == 0){
                            profilesCounter += 1;
                            cJSON *email = cJSON_GetObjectItem(profile, "email");
                            cJSON *name = cJSON_GetObjectItem(profile, "nome");
                            if (profilesCounter > 1){
                                strcat(payload, ",");
                            }
                            sprintf(profileJson, "{\"email\": \"%s\", \"nome\": \"%s\"}",
                            email->valuestring, name->valuestring);
                            strcat(payload, profileJson);
                        }
                    }
                }

                strcat(payload, "]}");
                bzero(buffer, 10000);
                sprintf(buffer, payload);
                send(client_socket, buffer, strlen(buffer), 0);
            }
            else if (strcmp(action->valuestring, "getAllProfilesByYear") == 0){
                // response in JSON format {"profiles": []}

                char payload[10000];
                char profileJson[200];
                strcpy(payload, "{\"profiles\":[");
                int profilesCounter = 0;

                for(int i = 0; i < num_profiles; i++){
                    cJSON *profile = cJSON_GetArrayItem(profiles_array, i);
                    cJSON *year = cJSON_GetObjectItem(profile, "ano_formatura");
                    if (year->valueint == message->valueint){
                        profilesCounter += 1;
                        cJSON *email = cJSON_GetObjectItem(profile, "email");
                        cJSON *name = cJSON_GetObjectItem(profile, "nome");
                        cJSON *course = cJSON_GetObjectItem(profile, "formacao");
                        if (profilesCounter > 1){
                            strcat(payload, ",");
                        }
                        sprintf(profileJson, "{\"email\": \"%s\", \"nome\": \"%s\", \"formacao\": \"%s\"}",
                        email->valuestring, name->valuestring, course->valuestring);
                        strcat(payload, profileJson);
                    } 
                }

                strcat(payload, "]}");
                bzero(buffer, 10000);
                sprintf(buffer, payload);
                send(client_socket, buffer, strlen(buffer), 0);
            }

            else if (strcmp(action->valuestring, "getAllProfiles") == 0){
                // return the entire data file

                bzero(buffer, 10000);
                char *json_str = cJSON_PrintUnformatted(data_json);
                strcpy(buffer, json_str);
                send(client_socket, buffer, strlen(buffer), 0);
                free(json_str);
            }

            else if (strcmp(action->valuestring, "getProfile") == 0){
                // response in JSON format {"profiles": []}

                char payload[10000];
                strcpy(payload, "{\"profiles\":[");

                for(int i = 0; i < num_profiles; i++){
                    cJSON *profile = cJSON_GetArrayItem(profiles_array, i);
                    cJSON *email = cJSON_GetObjectItem(profile, "email");
                    if (strcmp(email->valuestring, message->valuestring) == 0){
                        
                        char *json_str = cJSON_PrintUnformatted(profile);
                        strcat(payload, json_str);
                        free(json_str);  

                    }
                }

                strcat(payload, "]}");
                bzero(buffer, 10000);
                strcpy(buffer, payload);
                send(client_socket, buffer, strlen(buffer), 0);

            }
            else if (strcmp(action->valuestring, "removeProfile") == 0){

                bool profile_found = false;

                for(int i = 0; i < num_profiles; i++){
                    cJSON *profile = cJSON_GetArrayItem(profiles_array, i);
                    cJSON *email = cJSON_GetObjectItem(profile, "email");
                    if (strcmp(email->valuestring, message->valuestring) == 0){

                        profile_found = true;

                        cJSON_DeleteItemFromArray(profiles_array, i);
                        cJSON *removedProfile = cJSON_GetArrayItem(profiles_array, i);

                        fp = fopen("data.json", "w");
                        fprintf(fp, "%s", cJSON_PrintUnformatted(data_json));
                        fclose(fp);
        
                        cJSON_Delete(removedProfile);

                        break;
                    }                
                }

                // RESPONSE
                if (profile_found) {strcpy(buffer, "Perfil removido com sucesso!");} else {strcpy(buffer, "Erro: perfil não encontrado!");}
                send(client_socket, buffer, strlen(buffer), 0);    
            }

            cJSON_Delete(jsonPayload);

            }
        }
    }


int main() {

    char *ip = "127.0.0.1";
    int port = 5564;
    socklen_t addr_size;

    int server_socket = socket(AF_INET, SOCK_DGRAM, 0); //IPv4, UDP
    if (server_socket < 0) { // error handler
        perror("Erro ao criar socket do servidor");
        exit(1);
    }

    struct sockaddr_in server_address, client_address; //socket address settings
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = port;

    // bind socket and address settings
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Bind error");
        exit(1);
    }

    // start listening
    listen(server_socket, 10);

    printf("Servidor escutando\n");
    int client_socket;

    // waiting for client requests
    while (1) {
        client_socket = recvfrom(server_socket, NULL, NULL); // accept client attempt to connect
        printf("Cliente conectado\n");
        handle_client((void*)&client_socket);
    }
    
    close(client_socket); // close client connection
    printf("Cliente desconectado\n");

    return 0;
}


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "builtin.h"
#include "tests/syscall_mock.h"

#define TOTAL_COMMANDS 3 /* {"cd", "help", "exit"} */
#define BUFFER_SIZE 1024

bool builtin_is_internal(scommand cmd) {
    assert(cmd != NULL);
    bool result = false;
    char * intern_command[TOTAL_COMMANDS] = {"cd", "help", "exit"};
    char * command = scommand_front(cmd);
    
    for (unsigned int i = 0; i < TOTAL_COMMANDS && !result; i++) { 
    
        if (strcmp(command, intern_command[i]) == 0) {
            result = true;
        }
    }
    
    command = NULL;
    return result;
}


bool builtin_alone(pipeline p){
    assert(p != NULL);
    bool result = (pipeline_length(p) == 1 &&
                   builtin_is_internal(pipeline_front(p)));
    
    return result;
}


void builtin_run(scommand cmd) {
    assert(builtin_is_internal(cmd));
    char * intern_command[TOTAL_COMMANDS] = {"cd", "help", "exit"};
    char * command = scommand_front(cmd);
    unsigned int ind_command = 0;
    
    for (unsigned int i = 0; i < TOTAL_COMMANDS; i++) { 
    
        if (strcmp(command, intern_command[i]) == 0) {
            ind_command = i;
            break;
        }
    }
    
    if (ind_command == 0) {
        
        if (scommand_length(cmd) == 1) {
            printf("Error: No se ingreso la ruta destino\n");
            return;
        }
        
        scommand_pop_front(cmd);
        char * route = scommand_front(cmd);
        int res = chdir(route);
        
        if (res != 0) {      
            printf("Error: No existe el archivo o el directorio\n");
        }
        
        route = NULL;
        
    } else if (ind_command == 1) {
    
        printf("2² Easy Pieces Shell\n\n"
           "Autores:\n"
           "  - Agustin Reyna\n"
           "  - Luciano Lopez\n"
           "  - Rosella Nardi\n"
           "  - Tomas Iannaccone\n\n"
           "Comandos disponibles:\n\n"
           "# cd\n"
           "  Permite cambiar del directorio actual al de la ruta especificada. Se utiliza para navegar entre directorios en la línea de comandos.\n\n"
           "  Uso básico:\n"
           "    - cd <ruta>: Cambia al directorio especificado por <ruta>. La ruta puede ser absoluta (ej. /home/user/docs) o relativa (ej. ../ para subir un nivel).\n"
           "    - cd ..: Sube un nivel en la jerarquía de directorios.\n"
           "    - cd ~: Cambia al directorio home del usuario actual.\n\n"
           "# help\n"
           "  Proporciona, mediante un mensaje por la salida estándar, información sobre los comandos disponibles en el shell y su uso." 
           " Especifica una descripción breve o detalles sobre un comando particular.\n\n"
           "# exit\n"
           "  Finaliza la sesión de shell actual o termina la ejecución de un script. Puede devolver un valor de salida al sistema operativo que indica el estado de la terminación (código de salida).\n\n"
           "  Uso básico:\n"
           "    - exit: Sale del shell actual o del script con el código de salida por defecto (generalmente 0, que indica éxito).\n"
           "    - exit <número>: Sale del shell o script con el código de salida especificado por <número>. Los códigos de salida suelen indicar diferentes estados de éxito o error.\n");
    
    } else if (ind_command == 2) {    
        exit(EXIT_SUCCESS);
    }
}

void print_current_information(void) {

    char dir[BUFFER_SIZE];
    char hostname[BUFFER_SIZE];
    char *user;
    
    printf("\033[0;32m");
    
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        perror("Error al obtener el nombre del host");
        return;
    }

    if (getcwd(dir, sizeof(dir)) == NULL) {
        perror("Error al obtener el directorio actual");
        return;
    }
    
    user = getlogin();
    
    if (user == NULL) {
        perror("Error al obtener el nombre de usuario");
        return;
    }
    
    printf("%s@%s:~%s> ", user, hostname, dir);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////// DIRECTIVES /////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
# include <getopt.h>
# include <ctype.h>
# include <stdlib.h>
# include <stdio.h>
# include <unistd.h>
# include <string.h>

# include "../header/simdsort.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////// FUNCTIONS //////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - INPUTS: 
// - OUTPUTS: 
// - DESCRIPTION:

void getParams (int argc, char** argv, char* iValue, char* oValue, char* nValue, char* dValue) {

    int c;
    strcpy(dValue, "0");

    // i: Archivo binario con la lista de entrada desordenados (string)
    // o: Archivo binario de salida con la lista ordenada (string)
    // n: Largo de la lista (int)
    // d: Bandera (Mostrar resultados por pantalla (1) o no (0)) (binario)
    // Ejecutar como:
    //      ./simdsort -i desordenada.raw -o ordenada.raw -N num_elementos -d debug
    while ( (c = getopt (argc, argv, "i:o:N:d")) != -1) {

        switch (c) {
            case 'i':
                strcpy(iValue, optarg);
                // Verificacion que el archivo de entrada existe 
                if (!exist(iValue)) {
                    printf ("%s\n", "-------------------------------------------------------------------------");
                    printf (" => El argumento de -%c debe ser un ARCHIVO EXISTENTE.\n", c);
                    printf (" => Programa abortado\n");
                    printf ("%s\n", "-------------------------------------------------------------------------");
                    abort ();
                }       

                printf(" => Ruta del archivo de entrada (-i): %s\n", iValue);
                break;
            
            case 'o':
                strcpy(oValue, optarg);
                printf(" => Ruta de la mascara (-o): %s\n", oValue);
                break;
            
            case 'N':
                strcpy(nValue, optarg);
                // Verificacion de que se ingresa un numero entero positivo
                if (!isInteger(nValue)) {
                    printf ("%s\n", "-------------------------------------------------------------------------");
                    printf (" => El argumento de -%c debe ser un ENTERO POSITIVO.\n", c);
                    printf (" => Programa abortado\n");
                    printf ("%s\n", "-------------------------------------------------------------------------");
                    abort ();
                }
            
                printf(" => Largo de la lista (-N): %s\n", nValue);
                break;
            
            case 'd':
                strcpy(dValue, "1");

                printf(" => Opcion debug (-d): %s\n", dValue);
                break;
            
            case '?':
                // Verificacion de existencia de argumentos
                if ( (optopt == 'i') || (optopt == 'i') || (optopt == 'N') ) { 
                    printf ("%s\n", "-------------------------------------------------------------------------");
                    printf (" => La opcion -%c requiere un argumento.\n", optopt);
                    printf (" => Programa abortado\n");
                    printf ("%s\n", "-------------------------------------------------------------------------");
                    abort ();
                }
                // Verificacion de la validez de las banderas
                else if (isprint (optopt)) {
                    printf ("%s\n", "-------------------------------------------------------------------------");
                    printf (" => Opcion -%c desconocida.\n", optopt);
                    printf (" => Programa abortado\n");
                    printf ("%s\n", "-------------------------------------------------------------------------");
                    abort ();
                }

            default:
                break;
            }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - INPUTS: - fileName: Nombre del archivo a leer
// - OUTPUTS: Valor booleano 1 si el archivo existe, 0 en caso contrario
// - DESCRIPTION: Verifica que el archivo con el nombre "fileName" existe y devuelve la verificacion.

int exist (char* fileName) {

    FILE* f = fopen(fileName, "r");

    if (f != NULL) {
        fclose(f);
        return 1;
    }
      
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - INPUTS: - input: Cadena de caracteres a evaluar si corresponde a un numero entero positivo o no
// - OUTPUTS: Valor booleano 1 si es entero positivo, 0 en caso contrario
// - DESCRIPTION: Verifica si una cadena de caracteres de entrada posee en cada una de sus posiciones un caracter que es
//                digito y es positivo

int isInteger (char* input) {

    int c;

    for (c = 0; c < strlen(input); c++) {

        if (!isdigit(input[c]))
            return 0;
    }

    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
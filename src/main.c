/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////// DIRECTIVES /////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
# include <getopt.h>
# include <ctype.h>
# include <stdlib.h>
# include <stdio.h>
# include <unistd.h>
# include <string.h>
# include <omp.h>
# include <pmmintrin.h>

# include "../header/simdsort.h"
# include "../header/structs.h"

int main(int argc, char** argv) {

    int N, numLevels, numThreads;
    char* iValue = (char*)malloc(sizeof(char)); // Archivo binario con la lista de entrada desordenados
    char* oValue = (char*)malloc(sizeof(char)); // Archivo binario de salida con la lista ordenada
    char* nValue = (char*)malloc(sizeof(char)); // Largo de la lista
    char* dValue = (char*)malloc(sizeof(char)); // Bandera para mostrar por consola los resultados
    char* lValue = (char*)malloc(sizeof(char)); // Numero de niveles de recursividad a generar
    char* hValue = (char*)malloc(sizeof(char)); // Numero de hebras a generar

    // Obtencion de los parametros de entrada y verificacion.
    getParams (argc, argv, iValue, oValue, nValue, dValue, lValue, hValue);
    N = atoi(nValue);
    numLevels = atoi(lValue);
    numThreads = atoi(hValue);
    
    float* sequence = chargeSequence (iValue, N);
    vectorThreadSIMD (sequence, N, numLevels, numThreads);

    //if (strcmp(dValue, "1") == 0)
      //  printSequence (sortedSequence, N);
    
    // Liberacion de memoria reservada durante el programa.
    //freeMemory (iValue, oValue, nValue, dValue, sequence, sorted16Sequence, sortedSequence, L);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////// END ////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



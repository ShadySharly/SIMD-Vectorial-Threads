/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////// DIRECTIVES /////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
# include <getopt.h>
# include <ctype.h>
# include <stdlib.h>
# include <stdio.h>
# include <unistd.h>
# include <string.h>
# include <pmmintrin.h>

# include "../header/simdsort.h"

int main(int argc, char** argv) {

    int N, L;
    __m128 A, B, C, D, W_IR, X_IR, Y_IR, Z_IR, W_BMN, X_BMN, Y_BMN, Z_BMN, W_MSIMD, X_MSIMD, Y_MSIMD, Z_MSIMD;  // Registros MMX a utilizar en el programa
    char* iValue = (char*)malloc(sizeof(char)); // Archivo binario con la lista de entrada desordenados
    char* oValue = (char*)malloc(sizeof(char)); // Archivo binario de salida con la lista ordenada
    char* nValue = (char*)malloc(sizeof(char)); // Largo de la lista
    char* dValue = (char*)malloc(sizeof(char)); // Bandera para mostrar por consola los resultados
    
    // Obtencion de los parametros de entrada y verificacion.
    if ( (iValue != NULL) && (oValue != NULL) && (nValue != NULL) && (dValue != NULL) ) {
        getParams (argc, argv, iValue, oValue, nValue, dValue);
        N = atoi(nValue);
        L = N / 16;
    }

    float** sequence = chargeSequence (iValue, N);
    float** sorted16Sequence = sequenceMalloc (N);

    // Por cada secuencia de 16 elementos de la secuencia total de entrada se ejecuta la fase SIMD, formando al final de cada iteracion una secuencia de 
    // 16 elementos ordenados de forma creciente.
    for (int i = 0; i < L; i++) {
        //////////////////////////////////
        //         FASE SIMD            //
        //////////////////////////////////
        loadSequence (sequence[i], &A, &B, &C, &D);                                     // Carga de secuencias en los registros MMX
        inRegister (A, B, C, D, &W_IR, &X_IR, &Y_IR, &Z_IR);                            // Ordenamiento In Register
        BMN (W_IR, X_IR, &W_BMN, &X_BMN);                                               // BMN para la primera secuencia de 8
        BMN (Y_IR, Z_IR, &Y_BMN, &Z_BMN);                                               // BMN para la segunda secuencia de 8
        mergeSIMD (W_BMN, X_BMN, Y_BMN, Z_BMN, &W_MSIMD, &X_MSIMD, &Y_MSIMD, &Z_MSIMD); // Merge entre las dos secuencias de 8 generadas en las BMN anteriores
        storeSequence (sorted16Sequence[i], W_MSIMD, X_MSIMD, Y_MSIMD, Z_MSIMD);        // Almacenar en memoria la secuencia de 16 generada anteriormente
    }
        //////////////////////////////////
        //        FASE NO SIMD          //
        //////////////////////////////////
    float* sortedSequence = MWMS (sorted16Sequence, N, L);                              // Multiway Merge SIMD
    writeSequence (sortedSequence, oValue, N);                                          // Escritura de la secuencia ordenada

    // Dependiendo del valor del debug de entrada, se muestran los archivos por consola o no
    if (strcmp(dValue, "1") == 0)
        printSequence (sortedSequence, N);
    
    // Liberacion de memoria reservada durante el programa.
    freeMemory (iValue, oValue, nValue, dValue, sequence, sorted16Sequence, sortedSequence, L);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////// END ////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



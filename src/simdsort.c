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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////// FUNCTIONS //////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - INPUTS: - argc: Largo del arreglo de argumentos argv.
//           - argv: Arreglo con los argumentos de entrada incluyendo en nombre del archivo.
//           - iValue: Nombre del archivo de entrada que contiene el archivo en formato binario (RAW).
//           - oValue: Nombre del archivo de salida con las secuencia ordenada en formato binario (RAW).
//           - nValue: Largo de la secuencia contenida en el archivo de entrada (Numero entero multiplo de 16).
//           - dValue: Bandera que controla el debug para imprimir los resultados por consola (1) o no (0).
// - OUTPUTS: -
// - DESCRIPTION: Procedimiento que obtiene los parametros entregados por consola y almacenados en la variable "argv", y los deposita en las variables
//                iValue, oValue, nValue y dValue, en cada caso verificando la validez del valor entragado para cada bandera. Si alguna de estas banderas
//                no cumple con los formatos especificados el programa es interrumpido.

void getParams (int argc, char** argv, char* iValue, char* oValue, char* nValue, char* dValue) {

    int c;
    // i: Archivo binario con la lista de entrada desordenados (string)
    // o: Archivo binario de salida con la lista ordenada (string)
    // n: Largo de la lista (int)
    // d: Bandera (Mostrar resultados por pantalla (1) o no (0)) (binario)
    // Ejecutar como:
    //      ./simdsort -i desordenada.raw -o ordenada.raw -N num_elementos -d debug
    while ( (c = getopt (argc, argv, "i:o:N:d:")) != -1) {

        switch (c) {
            case 'i':
                strcpy(iValue, optarg);
                // Verificacion que el archivo de entrada existe 
                if (!exist(iValue)) {
                    printf ("%s\n", "-------------------------------------------------------------------------");
                    printf (" => El argumento de -%c debe ser un ARCHIVO EXISTENTE.\n", c);
                    printf (" => Programa abortado\n");
                    printf ("%s\n", "-------------------------------------------------------------------------");
                    exit(EXIT_FAILURE);
                }       

                //printf(" => Ruta del archivo de entrada (-i): %s\n", iValue);
                break;
            
            case 'o':
                // Obtencion de el nombre del archivo de salida
                strcpy(oValue, optarg);
                //printf(" => Ruta del archivo de salida (-o): %s\n", oValue);
                break;
            
            case 'N':
                strcpy(nValue, optarg);
                // Verificacion de que el N entregado es un entero positivo.
                if (!isInteger(nValue)) {
                    printf ("%s\n", "-------------------------------------------------------------------------");
                    printf (" => El argumento de -%c debe ser un ENTERO POSITIVO.\n", c);
                    printf (" => Programa abortado\n");
                    printf ("%s\n", "-------------------------------------------------------------------------");
                    exit(EXIT_FAILURE);
                }
            
                //printf(" => Largo de la lista (-N): %s\n", nValue);
                break;
            
            case 'd':
                strcpy(dValue, optarg);
                // Verificacion de que se ingresa un valor binario (1 o 0)
                if ( (strcmp(dValue, "0") != 0) && (strcmp(dValue, "1") != 0) ) {
                    printf ("%s\n", "-------------------------------------------------------------------------");
                    printf (" => El argumento de -%c debe ser un VALOR BINARIO (1 o 0).\n", c);
                    printf (" => Programa abortado\n");
                    printf ("%s\n", "-------------------------------------------------------------------------");
                    exit(EXIT_FAILURE);
                }

                //printf(" => Opcion debug (-d): %s\n", dValue);
                break;
            
            case '?':
                // Verificacion de existencia de argumentos
                if ( (optopt == 'i') || (optopt == 'o') || (optopt == 'N') || (optopt == 'd')) { 
                    printf ("%s\n", "-------------------------------------------------------------------------");
                    printf (" => La opcion -%c requiere un argumento.\n", optopt);
                    printf (" => Programa abortado\n");
                    printf ("%s\n", "-------------------------------------------------------------------------");
                    exit(EXIT_FAILURE);
                }
                // Verificacion de la validez de las banderas
                else if (isprint (optopt)) {
                    printf ("%s\n", "-------------------------------------------------------------------------");
                    printf (" => Opcion -%c desconocida.\n", optopt);
                    printf (" => Programa abortado\n");
                    printf ("%s\n", "-------------------------------------------------------------------------");
                    exit(EXIT_FAILURE);
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
    // Recorrer el argumento entragado en cadena de caracteres, verificando que cada uno de estos corresponde a un numero.
    for (c = 0; c < strlen(input); c++) {

        // Si no se cumple para alguno de los caracteres, significa que el argumento no corresponde a un entero positivo y retorna 0.
        if (!isdigit(input[c]))
            return 0;
    }
    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - INPUTS: - inputName: Nombre del archivo de entrada que contiene el archivo en formato binario (RAW).
//           - N: Largo de la secuencia contenida en el archivo de entrada (Numero entero multiplo de 16).
// - OUTPUTS: Retorna un arreglo de vectores de 16 elementos, correspondiente a los vectores o secuencias de 16 numeros que pasaran por cada una de las
//            etapas SIMD paara finalmente unirse en la etapa NO SIMD
// - DESCRIPTION: Toma el nombre del archivo de entrada y se abre en formato binario, luego se lee el contenido del archivo almacenando este cada 16
//                elementos en un arreglo de largo L = N / 16, y se retorna estem arreglo.

float** chargeSequence (char* inputName, int N) {

    int index = 0;
    FILE* inputFile = fopen(inputName, "rb");
    
    if (inputFile != NULL) {
        // Primero se reserva memoria dependiendo del largo de secuencia, reservando memoria cada 16 elemntos.
        float** sequences = sequenceMalloc (N);

        // Mientras queden datos que leer en el archivo de entrada se almacena la informacion en el arreglo anteriormente creado.
        while (!feof(inputFile)) {
            fread(sequences[index], sizeof(float), 16, inputFile);
            index++;
        }
        fclose(inputFile);
        return sequences;
    }

    // En caso de que el archivo de entrada no existe el programa es abortado.
    else{
        printf("ERROR AL LEER ARCHIVO DE ENTRADA\n");
        printf (" => Programa abortado\n");
        exit(EXIT_FAILURE);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - INPUTS: - sequence: Vector o secuencia de 16 numeros a ordenar.
//           - A: Registro MMX a almacenar los primeros 4 numeros del vector "sequence"
//           - B: Registro MMX a almacenar los segundos 4 numeros del vector "sequence"
//           - C: Registro MMX a almacenar los terceros 4 numeros del vector "sequence"
//           - D: Registro MMX a almacenar los ultimos 4 numeros del vector "sequence"
// - OUTPUTS: -
// - DESCRIPTION: Procedimiento que toma un vector de 16 elementos, y los distribuye de forma equitativa entre 4 registros MMX.

void loadSequence (float* sequence, __m128* A, __m128* B, __m128* C, __m128* D) {

    // Se almacenan los numeros en arreglos estaticos, los primeros 4 en a, los siguientes en b, los siguientes en c y los ultimos 4 en d.
    float a[4] __attribute__((aligned(16))) = { sequence[0], sequence[1], sequence[2], sequence[3] };
    float b[4] __attribute__((aligned(16))) = { sequence[4], sequence[5], sequence[6], sequence[7] };
    float c[4] __attribute__((aligned(16))) = { sequence[8], sequence[9], sequence[10], sequence[11] };
    float d[4] __attribute__((aligned(16))) = { sequence[12], sequence[13], sequence[14], sequence[15] };

    // Se cargan los 4 numeros (floats) en los registros MMX correspondientes desde los arreglos a, b, d y d.
    *A = _mm_load_ps (a);
    *B = _mm_load_ps (b);
    *C = _mm_load_ps (c);
    *D = _mm_load_ps (d);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - INPUTS: - A: Primer registro MMX de entrada, con los 4 primeros elementos de la secuencia de 16 de entrada.
//           - B: Segundo registro MMX de entrada, con los 4 siguientes elementos de la secuencia de 16 de entrada.
//           - C: Tercer registro MMX de entrada, con los 4 siguientes elementos de la secuencia de 16 de entrada.
//           - D: Cuarto registro MMX de entrada, con los 4 ultimos elementos de la secuencia de 16 de entrada.
//           - W_F: Primer registro MMX de salida, con sus elementos ordenados de forma ascendente
//           - X_F: Segundo registro MMX de salida, con sus elementos ordenados de forma ascendente
//           - Y_F: Tercero registro MMX de salida, con sus elementos ordenados de forma ascendente
//           - Z_F: cuarto registro MMX de salida, con sus elementos ordenados de forma ascendente
// - OUTPUTS: -
// - DESCRIPTION: Procedimeinto que aplica el ordenamiento In Register sobre una secuencia de 16 numeros distribuidos en 4 registros MMX de entrada,
//                aplicando funciones SIMD, min-max para ordenar dos registros y shuffle para permutar dos registros distintos en uno de salida. Como
//                salida se obtiene el proceso de ordenar los resgistros de entrada de forma descendente por cada indice de cada registro,
//                para posteriormente trasponerlos y obtener 4 registros ordenados en forma ascendente W_F, X_F, Y_F y Z_F.

void inRegister (__m128 A, __m128 B, __m128 C, __m128 D, __m128* W_F, __m128* X_F, __m128* Y_F, __m128* Z_F) {
 
    // Definicion de registros a utilizar en la etapa
    __m128 A_1, C_1, B_2, D_2, A_3, B_3, C_3, D_3, B_4, C_4;    // Regsitros para la red MIN-MAX inicial.
    __m128 W_1, W_2, X_1, X_2;  // Primer conjunto de registros para la red de shuffles.
    __m128 Y_1, Y_2, Z_1, Z_2;  // Segundo conjunto de registros para la red de shuffles.

    // Implementacion de la primera red de ordenamiento del procedimiento in register, dividiendola en 4 etapas dependiendo de como se ubican en 
    // la red, obteniendose el registro con los minimos y maximos valores para cada par de registros de entrada. Esto se repite por cada paso
    // de la red y tomando como entrada registros anteriormente calculados.
    
    // Primer paso de la red
    A_1 = _mm_min_ps (A, C);  
    C_1 = _mm_max_ps (A, C);

    // Segundo paso de la red
    B_2 = _mm_min_ps (B, D);
    D_2 = _mm_max_ps (B, D);

    // Tercer paso de la red
    A_3 = _mm_min_ps (A_1, B_2); // Primer registro de salida de la red
    B_3 = _mm_max_ps (A_1, B_2);

    // Cuarto paso de la red
    C_3 = _mm_min_ps (C_1, D_2);
    D_3 = _mm_max_ps (C_1, D_2); // Cuarto registro de salida de la red

    // Ultimo paso de la red
    B_4 = _mm_min_ps (B_3, C_3); // Segundo registro de salida de la red
    C_4 = _mm_max_ps (B_3, C_3); // Tercer registro de salida de la red

    // El siguiente par de bloques se encarga de realizar la trasposicion de los registros de salida anteriores, vale decir tomar los elementos para 
    // cada indice de los 4 registros y juntarlos en un nuevo registro final.
    
    // Trasposicion de la primera y segunda columna de la matriz de registros
    W_1 = _mm_shuffle_ps (A_3, B_4, _MM_SHUFFLE (1, 0, 1, 0));
    X_1 = _mm_shuffle_ps (C_4, D_3, _MM_SHUFFLE (1, 0, 1, 0));
    W_2 = _mm_shuffle_ps (W_1, W_1, _MM_SHUFFLE (3, 1, 2, 0));
    X_2 = _mm_shuffle_ps (X_1, X_1, _MM_SHUFFLE (3, 1, 2, 0));
    *W_F = _mm_shuffle_ps (W_2, X_2, _MM_SHUFFLE (1, 0, 1, 0)); // Primer registro ordenado de forma ascendente
    *X_F = _mm_shuffle_ps (W_2, X_2, _MM_SHUFFLE (3, 2, 3, 2)); // Segundo registro ordenado de forma ascendente

    // Trasposicion de la tercera y cuarta columna de la matriz de registros
    Y_1 = _mm_shuffle_ps (A_3, B_4, _MM_SHUFFLE (3, 2, 3, 2));
    Z_1 = _mm_shuffle_ps (C_4, D_3, _MM_SHUFFLE (3, 2, 3, 2));
    Y_2 = _mm_shuffle_ps (Y_1, Y_1, _MM_SHUFFLE (3, 1, 2, 0));
    Z_2 = _mm_shuffle_ps (Z_1, Z_1, _MM_SHUFFLE (3, 1, 2, 0));
    *Y_F = _mm_shuffle_ps (Y_2, Z_2, _MM_SHUFFLE (1, 0, 1, 0)); // Tercer registro ordenado de forma ascendente
    *Z_F = _mm_shuffle_ps (Y_2, Z_2, _MM_SHUFFLE (3, 2, 3, 2)); // Cuarto registro ordenado de forma ascendente
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - INPUTS: - A: Primer registro MMX ordenado de forma creciente
//           - B: Segundo registro MMX ordenado de forma creciente
// - OUTPUTS: -
// - DESCRIPTION: Procedimiento que aplica la Bitonic Merge Network (BMN) a dos registros MMX A y B de entrada, aplicando una serie de funciones SIMD
//                min-max y shuffle. Como salida se obtienen dos registros W y X que forman un vector de 8 elementos ordenados de forma creciente.

void BMN (__m128 A, __m128 B, __m128* W, __m128* X) {
    
    // Registros de salida para cada operacion min-max y shuffle de la BMN
    __m128 A_1, A_2, A_3, A_4, A_5, A_6, A_7, A_8;
    __m128 BI_1, BI_2, BI_3, BI_4, BI_5, BI_6, BI_7, BI_8, BI;

    //Shuffle 1, se reordena el registro B de entrada para que quede de forma decreciente y formar la secuencia bitonica.
    BI = _mm_shuffle_ps(B, B, _MM_SHUFFLE(0, 1, 2, 3));

    //min max paso 1
    A_1 = _mm_min_ps(A, BI);
    BI_1 = _mm_max_ps(A, BI);

    //Shuffle paso 2
    A_2 = _mm_shuffle_ps(A_1, BI_1, _MM_SHUFFLE(1, 0, 1, 0));
    BI_2 = _mm_shuffle_ps(A_1, BI_1 , _MM_SHUFFLE(3, 2, 3, 2));

    //Shufle paso 3
    A_3 = _mm_shuffle_ps(A_2, A_2, _MM_SHUFFLE(3, 1, 2, 0));
    BI_3 = _mm_shuffle_ps(BI_2, BI_2, _MM_SHUFFLE(3, 1, 2, 0));

    //min max paso 4 
    A_4 = _mm_min_ps(A_3, BI_3);
    BI_4 = _mm_max_ps(A_3, BI_3);

    //Shuffle paso 5 
    A_5 = _mm_shuffle_ps(A_4, BI_4, _MM_SHUFFLE(1, 0, 1, 0));
    BI_5 = _mm_shuffle_ps(A_4, BI_4, _MM_SHUFFLE(3, 2, 3, 2));

    //Shuffle paso 6 
    A_6 = _mm_shuffle_ps(A_5, A_5, _MM_SHUFFLE(3, 1, 2, 0));
    BI_6 = _mm_shuffle_ps(BI_5, BI_5, _MM_SHUFFLE(3, 1, 2, 0));

    //min max paso 7
    A_7 =  _mm_min_ps(A_6, BI_6);
    BI_7 = _mm_max_ps(A_6, BI_6);

    //Shuffle paso 8 
    A_8 = _mm_shuffle_ps(A_7, BI_7, _MM_SHUFFLE(1, 0, 1, 0));
    BI_8 = _mm_shuffle_ps(A_7, BI_7, _MM_SHUFFLE(3, 2, 3, 2));

    //Shuffle paso 9
    *W = _mm_shuffle_ps(A_8, A_8, _MM_SHUFFLE(3,1,2,0)); // Primer registro ordenado de forma creciente
    *X = _mm_shuffle_ps(BI_8, BI_8, _MM_SHUFFLE(3,1,2,0)); // Segundo registro ordenado de forma creciente y a continuacion del anterior.
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - INPUTS: - A: Primer registro ordenado del primer vector de 8 elementos.
//           - B: Segundo registro ordenado del primer vector de 8 elementos
//           - C: Primer registro ordenado del segundo vector de 8 elementos
//           - D: Segundo registro ordenado del segundo vector de 8 elementos
// - OUTPUTS: -
// - DESCRIPTION: Procedimiento que aplica el Merge SIMD sobre dos secuencias de 8 elementos ordenados de forma creciente, domde la primera secuencia
//                esta representada por los registros A y B, y la segunda por los registros C y D, en ambos casos de forma consecutiva. El funcionamiento
//                consiste en pasar estos cuatro registros a traves de la BMN hasta obtener una sola secuencia de 16 elementos ordenada de forma 
//                creciente, osea que los cuatro registros de 4 se esten ordenados de forma consecutiva.

void mergeSIMD (__m128 A, __m128 B, __m128 C, __m128 D, __m128* W, __m128* X, __m128* Y, __m128* Z) {

    // Declaracion de registros intermedios
    __m128 O1, O2;
    // Se declaran dos arregos para poder manejar el valor de los primeros elementos de los registros B y D.
    float b[4] __attribute__((aligned(16))) = { 0.0, 0.0, 0.0, 0.0 };  
    float d[4] __attribute__((aligned(16))) = { 0.0, 0.0, 0.0, 0.0 };

    _mm_store_ps(b, B);
    _mm_store_ps(d, D);

    // Se aplica la BMN para los registros A y C y que corresponden a los primeros cuatro elementos de las dos secuencias de 8, y su resultado es 
    // almacenado en los registros O1 y O2
    BMN (A, C, &O1, &O2);
    *W = O1;    // El primer registro resultante de la BMN anterior es pasado directamente a la secuencia final, correspondiente al registro W

    // Se comparan los primeros valores de los dos registros que aun no se consideran, en este caso B y D, y dependiendo del que tenga el minimo 
    // primer valor, se toma para aplicar la BMN junto con el O2 de la BMN anterior.
    if (b[0] < d[0]) {
        // Se aplica la BMN por segunda vez con O2 y B, y se obtiene un nuevo O1 que es pasado al registro final nuevamente
        // en este caso al registro X
        BMN (O2, B, &O1, &O2);  
        *X = O1;

        // Se aplica la BMN con los dos registros restantes, osea el O2 anterior y el registro D. Como no hay mas registros que comprar el resultado
        // de esta BMN es pasado directamente  a los registros siguientes de la secuencia final, osea O1 a Y, y O2 a Z.
        BMN (O2, D, &O1, &O2);
        *Y = O1;
        *Z = O2;
    }

    // El mismo procedimiento anterior considerando que D tiene el minimo primer valor en comparacion con B.
    else if (b[0] > d[0]) {
        BMN (O2, D, &O1, &O2);
        *X = O1;

        BMN (O2, B, &O1, &O2);
        *Y = O1;
        *Z = O2;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - INPUTS: - sequence: Arreglo de floats de 16 elementos en los que se almacenan los valores provientes desde los registros MMX
//           - WF: Primer registro MMX de 4 elementos ordenado de forma creciente.
//           - XF: Segundo registro MMX de 4 elementos ordenado de forma creciente despues del anterior.
//           - YF: Tercer registro MMX de 4 elementos ordenado de forma creciente despues del anterior.
//           - ZF: Ultimo registro MMX de 4 elementos ordenado de forma creciente despues del anterior.
// - OUTPUTS: -
// - DESCRIPTION: Procedimiento que almacena una secuencia de 16 elementos en memoria, especificamente en el arreglo "sequence", para esto se 
//                utiliza la funcion "store".

void storeSequence (float* sequence, __m128 WF, __m128 XF, __m128 YF, __m128 ZF) {

    // Se definen 4 arreglos de 4 elementos cada uno inicializados todos con 0, para almacenar cada uno de los valores de los registros MMX de entrada.
    float w[4] __attribute__((aligned(16))) = { 0.0, 0.0, 0.0, 0.0 };
    float x[4] __attribute__((aligned(16))) = { 0.0, 0.0, 0.0, 0.0 };
    float y[4] __attribute__((aligned(16))) = { 0.0, 0.0, 0.0, 0.0 };
    float z[4] __attribute__((aligned(16))) = { 0.0, 0.0, 0.0, 0.0 };

    // Se almacenan los registros MMX en memoria.
    _mm_store_ps(w, WF);
    _mm_store_ps(x, XF);
    _mm_store_ps(y, YF);
    _mm_store_ps(z, ZF);

    // Se traspasan los valores de los arreglos estaticos al arreglo dinamico de salida "sequence"
    sequence[0] = w[0];
    sequence[1] = w[1];
    sequence[2] = w[2];
    sequence[3] = w[3];
    sequence[4] = x[0];
    sequence[5] = x[1];
    sequence[6] = x[2];
    sequence[7] = x[3];
    sequence[8] = y[0];
    sequence[9] = y[1];
    sequence[10] = y[2];
    sequence[11] = y[3];
    sequence[12] = z[0];
    sequence[13] = z[1];
    sequence[14] = z[2];
    sequence[15] = z[3];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - INPUTS: - sequence: Arreglo de L secuencias ordenadas de forma creciente independientes unas de otras. 
//           - N: Numero de elementos de la secuencia de entrada entregada por el archivo de entrada.
//           - L: Numero de secuencias de 16 elementos cada una, corresponde a L = N / 16.
// - OUTPUTS: Como salida se obtiene una secuencia (puntero de floats) de largo N, todos los numeros ordenados de forma creciente.
// - DESCRIPTION: Funcion que aplica la Multiway Merge Sort (MWMS), tomando el valor minimo de cada secuencia de 16 y agregandolos a un arreglo final
//                el cual se va creando de forma creciente. En terminos generales la funcion compara los primeros elementos de cada secuencia de largo
//                16 y "extrae" el menor y lo agrega al arreglo de salida. Este proceso itera hasta que no quedan elementos por visitar, o mas bien
//                el largo de la secuencia de salida alcanza el valor N. Esta etapa corresponde a la NO SIMD de todo el proceso.

float* MWMS (float** sequence, int N, int L) {

    float min;  // Variable en al que se almacenan los minimos valores en cada iteracion
    int seqIndex, minIndex, firstIndex, outIndex;   // Variables para almacenar indices de utilidad, como el indice que contiene al minimo, y el de la
                                                    // secuencia final.
    float* outSequence = (float*)malloc(N * sizeof(float)); // Se reserva memoria para la secuancia final, reservando la necesaria para soportar N 
                                                            // numeros en formato float.
    int* indexArray = initIndexArray (L);   // Se crea un arreglo de largo N para mantener el registro de los indices visitados para cada secuencia de
                                            // 16 elementos.

    outIndex = 0;   // Se inicializa el primer elemento de la secuencia final en 0
    firstIndex = 1; // Variable para saber si existe un minimo actual, 1 si el indice actual corresponde al primero de entre las L secuencias de 16 
                    // elementos, y 0 si es que ya se ha visitado secuencias enteriores a la actual. Recordar que para cada iteracion, el elemento minimo
                    // comenzara siendo el primero de la iteracion sin considerar secuencias que ya han sido visitadas en su totalidad.
    
    // Verificacion de reserva de memoria
    if (outSequence != NULL) {
        // Cuando el arreglo de salida alcance el largo N, significa que ya se han agregado todos los elementos del arreglo "sequence" de entrada
        while (outIndex < N) {
            // Ciclo que recorre todas las secuencias de 16 y agregando el minimo de cada iteracion al arreglo de salida
            for (seqIndex = 0; seqIndex < L; seqIndex++) {
                int numIndex = indexArray[seqIndex];    // Indice del cursor de cada secuencia de 16 elementos, se utiliza para sabre cuando una 
                                                        // secuencia de 16 ya ha sido agregada al arreglo final en su totalidad

                // No se consideran secuencias que ya han sido agregadas en su totalidad al arreglo de salida
                if (numIndex < 16) {
                    // Si es la primera seuencia de 16 comparada, el elemento actual de esta secuencia sera el minimo actual.
                    if (firstIndex == 1) {
                        minIndex = seqIndex;
                        min = sequence[seqIndex][numIndex];
                        firstIndex = 0; // Se pone la variable en 0.
                    }

                    // Si el elemento visitado actual es menor al minimo, este pasara a ser el minimo y se almacena el indice correspondiente
                    if (sequence[seqIndex][numIndex] < min) {
                        minIndex = seqIndex;
                        min = sequence[seqIndex][numIndex];
                    }
                }
            }
        
            outSequence[outIndex] = min;    // Se almacena el minimo de la iteracion en el arreglo de salida.
            indexArray[minIndex] = indexArray[minIndex] + 1;    // Se incrementa el indice de la secuencia que ha sido agregada al arreglo de salida, 
                                                                // para evitar que se lea el mismo numero en las siguientes iteraciones
            firstIndex = 1; // Se resetea el valor
            outIndex++;     // Se corre el indice del arreglo de salida en 1.
        }
        
        free(indexArray);
        return outSequence;
    }

    else {
        printf("ERROR AL ALOJAR MEMORIA PARA LA SECUENCIA FINAL\n");
        printf (" => Programa abortado\n");
        exit(EXIT_FAILURE);
    }   
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - INPUTS: - L: Largo del arreglo a generar
// - OUTPUTS: Arreglo de enteros de largo L inicializados en 0.
// - DESCRIPTION: Toma un valor L para generar un arreglo de ese largo con cada uno de sus valores inicializados en 0, en su contexto es utiliza para
//                crear el arreglo de indices y llevar un registro de los numeros visitados para cada secuencia de largo 16.

int* initIndexArray (int L) {

    int index;
    int* array = (int*)malloc(L * sizeof(int));

    if (array != NULL) {
        for (index = 0; index < L; index++)
            array[index] = 0;
        
        return array;
    }

    else {
        printf("ERROR AL ALOJAR MEMORIA PARA LA SECUENCIA DE INDICES\n");
        printf (" => Programa abortado\n");
        exit(EXIT_FAILURE);
    } 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - INPUTS: - sequence: Arreglo de largo N ordenado de forma creciente.
//           - outputName: Nombre del archivo de salida en el cual se escribiran los datos de "sequence" en formato binario
//           - N: Largo de la secuencia.
// - OUTPUTS: -
// - DESCRIPTION: Toma una secuencia de numero de largo N, y la escribe en un archivo binario (RAW).

void writeSequence (float* sequence, char* outputName, int N) {

    FILE* outputFile = fopen(outputName, "wb");
    
    // Verificacion de que se abre el archivo en modo escritura binaria.
    if (outputFile != NULL) {
        fwrite(sequence, sizeof(float), N, outputFile);     // Se escriben N elementos de tamaño "float" contenidas en "sequence"
        fclose(outputFile);
    }

    else{
        printf("ERROR AL CREAR ARCHIVO DE SALIDA\n");
        printf (" => Programa abortado\n");
        exit(EXIT_FAILURE);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - INPUTS: - sequence: Secuencia de largo 16
// - OUTPUTS: -
// - DESCRIPTION: Mostrar una secuencia de 16 elementos por consola

void print16Sequence (float* sequence) {

    for (int i = 0; i < 16; i = i + 4) {
        printf("%f %f %f %f\n", sequence[i], sequence[i + 1], sequence[i + 2], sequence[i + 3]);
    }
    printf("\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - INPUTS: - sequence: Secuencia de largo N
// - OUTPUTS: -
// - DESCRIPTION: Mostrar una secuencia de N elementos por consola

void printSequence (float* sequence, int N) {

    for (int i = 0; i < N; i++) {
        printf("%f\n", sequence[i]);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - INPUTS: - N: Numero entero correspondiente al largo de la secuencia.
// - OUTPUTS: Arreglo de N secuencias, cada una de largo 16.
// - DESCRIPTION: Reservar memoria para un arreglo de N elementos, cada uno de los cuales de tamaño 16.

float** sequenceMalloc (int N) {

    int L, index;
    L = N / 16;

    float** sequences = (float**)malloc(L * sizeof(float*));

    if (sequences != NULL) {
        for (index = 0; index < L; index++) {
            sequences[index] = (float*)malloc(16 * sizeof(float));

            if (sequences[index] == NULL) {
                printf("ERROR AL ALOJAR MEMORIA PARA LAS SECUENCIAS\n");
                printf (" => Programa abortado\n");
                exit(EXIT_FAILURE);
            }
        }
        return sequences;
    }

    else {
        printf("ERROR AL ALOJAR MEMORIA PARA LAS SECUENCIAS\n");
        printf (" => Programa abortado\n");
        exit(EXIT_FAILURE);
    }    
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - INPUTS: Punteros referenciando direcciones de variables utilizadas en el programa
// - OUTPUTS: -
// - DESCRIPTION: Liberar punteros utilizados en el programa.

void freeMemory (char* iValue, char* oValue, char* nValue, char* dValue, float** sequence, float** sorted16sequence, float* sortedSequence, int L) {

    int index;

    for (index = 0; index < L; index++) {
        free(sequence[index]);
        free(sorted16sequence[index]);
    }

    free(sequence);
    free(sorted16sequence);
    free(sortedSequence);
    free(iValue);
    free(oValue);
    free(nValue);
    free(dValue);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////// END ////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
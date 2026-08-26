#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constantes definidas para el tamaño del buffer y los modos de operación
#define TAM_BUFFER 2048
#define CIFRAR 1
#define DESCIFRAR 0

// Andrés Huertas - 202420560
// Andrés Javier Sanabria Garzón -  202411507

__declspec(naked) int ajustarRangoByte(int valor) {
    
    __asm {
        mov eax, [esp + 4]

        cmp eax, 255
        jle minimo
        mov eax, 255
        jmp fin

        minimo:
            cmp eax, 0 
            jge fin
            mov eax, 0

        fin:
            ret
    }

    }
              



/* ---------------------------------------
   Subrutina: ajustarRangoByte a traducir. Parámetros por registro
--------------------------------------- */
int  ajustarRangoByte(int valor) {

    if (valor > 255) {
        valor = valor - 256;
    }

    if (valor < 0) {
        valor = valor + 256;
    }

    return valor;
}


/* ---------------------------------------
   Rutina principal a traducir. Parámetros por pila
--------------------------------------- */

__declspec(naked) int transformarBytesClave(unsigned char *buffer,
                         int longitud,
                         unsigned char *clave,
                         int tamClave,
                         int indiceClave,
                         int modo) {
    /*
        TO DO: Escriba en este comentario cómo se encuentra la pila
    */
    __asm {
        /*
            TO DO:  Traduzca a ASM usando la pila y direccionamiento basado en EBP.
                    Además, defina el prólogo y el epílogo.

            Se tienen 6 parametros y 4 variables locales

            LOCALES:
             int i; [ebp-4]
             int valorByte; [epb-8]
             int valorClave; [ebp-12]
             int temp; [ebp-16]

            Direccion de retorno [ebp+4]
            ebp del llamador [ebp+0]

            PARAMETROS:
            unsigned char *buffer [ebp+8]
            int longitud [ebp+12]
            unsigned char *clave [ebp+16]
            int tamClave [ebp+20]
            int indiceClave [ebp+24]
            int modo [ebp+28]



        */

        ; Variables que no cambian
        ; - int modo = [ebp+28]
        ; - int tamClave [ebp+20]
        ; - int longitud [ebp+12]

        ; Variables que cambian
        ; - int i = [ebp-4]
        ; - int valorByte = [ebp-8]
        ; - int valorClave = [ebp-12]
        ; - int temp = [ebp-16] 
        ; - unsigned char *buffer = [epb+8]
        ; - unsigned char *clave = [ebp+16]
        ; - int indiceClave = [ebp+24]

        push ebp
        mov ebp, esp
        sub esp, 16

        push ebx
        push esi
        push edi 

        mov dword ptr[ebp-4], 0
        
        InicioFor:
            mov esi, dword ptr[ebp - 4] ; registro esi tiene contador
            cmp esi, dword ptr[ebp + 12] ; i >= longitud
            jge finLoop

            mov ecx, [ebp + 8] ; ecx = buffer
            movzx eax, byte ptr [ecx + esi] ; eax = buffer[i]
            mov [ebp - 8], eax ; valorByte = buffer[i]

            
            mov ebx, [ebp + 24] ; ebx = indiceClave

            mov ecx, [ebp + 16] ; ecx = clave
            movzx edx, byte ptr [ecx + ebx]  ; eax = clave[indiceClave]
            mov [ebp - 12], edx ; valorClave = clave[indiceClave]

            cmp dword ptr [ebp + 28], CIFRAR
            jne noIgual
                add eax, edx
                mov [ebp - 16], eax
                jmp finIf    
            noIgual:
                sub eax, edx
                mov [ebp - 16], eax

            finIf:
            mov ecx, [ebp - 16]
            push ecx
            call ajustarRangoByte
            add esp, 4

            mov ecx, [ebp + 8]
            mov esi, dword ptr [ebp - 4]
            mov byte ptr [ecx + esi], al

            inc dword ptr [ebp + 24]

            inc dword ptr [ebp - 4]

            mov edi, [ebp + 20]
            cmp dword ptr [ebp - 4], edi
            jne noEsIgual

            noEsIgual:
                mov dword ptr [ebp + 24], 0
                jmp InicioFor


            jmp InicioFor

        finLoop:
            pop edi
            pop esi 
            pop ebx
            
            mov esp, ebp
            pop epb
            ret
        
    }
}



int transformarBytesClave(unsigned char *buffer,
                         int longitud,
                         unsigned char *clave,
                         int tamClave,
                         int indiceClave,
                         int modo) {

    int i;
    int valorByte;
    int valorClave;
    int temp;

    for (i = 0; i < longitud; i++) {

        valorByte = buffer[i];
        valorClave = clave[indiceClave];

        if (modo == CIFRAR) {
            temp = valorByte + valorClave;
        } else {
            temp = valorByte - valorClave;
        }

        /* llamada a subrutina */
        temp = ajustarRangoByte(temp);

        buffer[i] = (unsigned char) temp;

        indiceClave++;

        if (indiceClave == tamClave) {
            indiceClave = 0;
        }
    }

    return indiceClave;
}

/* ---------------------------------------
   Procesamiento del archivo
--------------------------------------- */
int procesarArchivo(const char *archivoEntrada,
                    const char *archivoSalida,
                    unsigned char *clave,
                    int tamClave,
                    int modo) {

    FILE *in;
    FILE *out;

    unsigned char buffer[TAM_BUFFER];

    int leidos;
    int total = 0;
    int indiceClave = 0;

    in = fopen(archivoEntrada, "rb");
    if (in == NULL) {
        printf("Error abriendo archivo de entrada\n");
        return -1;
    }

    out = fopen(archivoSalida, "wb");
    if (out == NULL) {
        printf("Error abriendo archivo de salida\n");
        fclose(in);
        return -1;
    }

    /*
    buffer: puntero al destino de bloque de memoria donde se almacenarán los datos leídos del archivo de entrada.
    1: tamaño de cada elemento a leer (en bytes)
    TAM_BUFFER: número máximo de elementos a leer (en este caso, el tamaño del buffer).
    in: puntero al archivo de entrada desde el cual se leerán los datos.
    */
    
    leidos = fread(buffer, 1, TAM_BUFFER, in);

    // Ciclo para procesar cada elemento del archivo y cifrarlo o descifrarlo según el modo especificado
    while (leidos > 0) {

        indiceClave = transformarBytesClave(
            buffer,
            leidos,
            clave,
            tamClave,
            indiceClave,
            modo
        );

        fwrite(buffer, 1, leidos, out);

        total += leidos;

        leidos = fread(buffer, 1, TAM_BUFFER, in);
    }

    fclose(in);
    fclose(out);

    return total;
}

/* ---------------------------------------
   Programa principal
--------------------------------------- */
int main(int argc, char *argv[]) {

    // Punteros para almacenar los argumentos de la línea de comandos
    char *operacion;
    char *archivoEntrada;
    char *archivoSalida;
    char *claveTexto;

    /* Variables para almacenar la clave, su tamaño, el modo de operación 
    y el número de bytes procesados  */

    unsigned char *clave;
    int tamClave;
    int modo;
    int procesados;

    if (argc != 5) {
        printf("Uso:\n");
        printf("  programa cifrar entrada salida clave\n");
        printf("  programa descifrar entrada salida clave\n");
        return 1;
    }

    // Asignación de los argumentos a las variables correspondientes
    operacion = argv[1];
    archivoEntrada = argv[2];
    archivoSalida = argv[3];
    claveTexto = argv[4];

    // Determinación del modo de operación según el argumento proporcionado
    if (strcmp(operacion, "cifrar") == 0) {
        modo = CIFRAR;
    } else if (strcmp(operacion, "descifrar") == 0) {
        modo = DESCIFRAR;
    } else {
        printf("Operacion invalida\n");
        return 1;
    }


    // Convierte la variable claveTexto a un puntero
    // tipo unsigned char y lo guarda en la variable clave.
    clave = (unsigned char *) claveTexto;
    tamClave = 0;

    // aumentar el tamaño del caracter mientras
    // no se encuentre en la clave del texto el caracter nulo '\0'
    while (claveTexto[tamClave] != '\0') {
        tamClave++;
    }


    // Varificar si la clave esta vacia
    // en dado caso, terminar programa 
    if (tamClave == 0) {
        printf("Clave vacia\n");
        return 1;
    }


    // Ingresar a la función procesarArchivo con los parámetros correspondientes
    // archivoEntrada: Nombre de archivo que se va a cifrar o decifrar
    // archivoSalida: Nombre de archivo que debe imprimir 
    // clave: Puntero a la clave que se va a utilizar para cifrar o descifrar
    // tamClave: Tamaño de la clave
    // modo: Modo de operación
    procesados = procesarArchivo(
        archivoEntrada,
        archivoSalida,
        clave,
        tamClave,
        modo
    );

    if (procesados >= 0) {
        printf("Archivo procesado correctamente\n");
        printf("Bytes procesados: %d\n", procesados);
    }

    return 0;
}
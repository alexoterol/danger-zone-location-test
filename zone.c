#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>

#define URL "http://ip-api.com/json"

struct MemoryStruct {
    char *memory;
    size_t size;
};

// Función de escritura para almacenar los datos obtenidos por curl
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, struct MemoryStruct *mem) {
    size_t realsize = size * nmemb;
    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        printf("No se puede asignar memoria\n");
        return 0;
    }
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

// Función principal para obtener la ubicación
void obtener_ubicacion() {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    chunk.memory = malloc(1);  // inicializamos la memoria
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, URL);  // URL de la API
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        // Realizamos la solicitud
        res = curl_easy_perform(curl);

        // Comprobamos si la solicitud fue exitosa
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() falló: %s\n", curl_easy_strerror(res));
        } else {
            // Procesamos la respuesta JSON
            printf("Respuesta obtenida: %s\n", chunk.memory);
        }

        // Limpiamos recursos
        curl_easy_cleanup(curl);
    }
    free(chunk.memory);
    curl_global_cleanup();
}

int main() {
    obtener_ubicacion();
    return 0;
}

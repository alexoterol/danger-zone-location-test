#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <math.h>

#define URL "http://ip-api.com/json"

// Estructura para almacenar la respuesta de la API
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Coordenadas de las zonas peligrosas
#define NUM_ZONAS 2
double zonas_lat[NUM_ZONAS] = {40.4168, 19.4326};  // Latitudes de las zonas peligrosas
double zonas_lon[NUM_ZONAS] = {-3.7038, -99.1332}; // Longitudes de las zonas peligrosas
double radios_peligro_m[NUM_ZONAS] = {500.0, 1000.0}; // Radios de zonas peligrosas en metros

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

// Función para calcular la distancia entre dos puntos geográficos (en metros)
double distancia_m(double lat1, double lon1, double lat2, double lon2) {
    double R = 6371000; // Radio de la Tierra en metros
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;

    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) *
               sin(dLon / 2) * sin(dLon / 2);

    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return R * c;
}

// Función para obtener la ubicación desde una API pública (ip-api.com) y parsear la respuesta JSON
void obtener_ubicacion(double *lat, double *lon) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    chunk.memory = malloc(1);  // Inicializamos la memoria
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
            // Procesamos la respuesta JSON para obtener latitud y longitud
            char *lat_str = strstr(chunk.memory, "\"lat\":");
            char *lon_str = strstr(chunk.memory, "\"lon\":");
            if (lat_str && lon_str) {
                sscanf(lat_str, "\"lat\":%lf", lat);
                sscanf(lon_str, "\"lon\":%lf", lon);
            }
        }

        // Limpiamos recursos
        curl_easy_cleanup(curl);
    }
    free(chunk.memory);
    curl_global_cleanup();
}

// Función para verificar si una ubicación está dentro de una zona peligrosa
int esta_en_zona(double lat, double lon) {
    for (int i = 0; i < NUM_ZONAS; i++) {
        double d = distancia_m(lat, lon, zonas_lat[i], zonas_lon[i]);
        if (d <= radios_peligro_m[i]) {
            return 1; // Está en la zona peligrosa
        }
    }
    return 0; // No está en la zona peligrosa
}

int main() {
    double lat_actual, lon_actual;
    int en_zona_peligrosa;

    // Obtener la ubicación actual
    obtener_ubicacion(&lat_actual, &lon_actual);

    // Mostrar la ubicación obtenida
    printf("Ubicación obtenida: Lat: %f, Lon: %f\n", lat_actual, lon_actual);

    // Verificar si la ubicación está dentro de una zona peligrosa
    en_zona_peligrosa = esta_en_zona(lat_actual, lon_actual);

    // Mostrar el mensaje dependiendo de si está en zona peligrosa o no
    if (en_zona_peligrosa) {
        printf("⚠️ ¡Estás en una zona peligrosa! (Lat: %f, Lon: %f)\n", lat_actual, lon_actual);
    } else {
        printf("✔ Estás en una zona segura. (Lat: %f, Lon: %f)\n", lat_actual, lon_actual);
    }

    return 0;
}

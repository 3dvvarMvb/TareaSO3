#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <vector>
#include <queue>
#include <mutex>
#include <algorithm>

using namespace std;

// Estructura para representar una página
struct Pagina {
    int id_proceso;
    int num_pagina;
    bool en_RAM;
};

// Estructura para representar un proceso
struct Proceso {
    int id_proceso;
    int tam_proceso_kb;
    int num_paginas;
    vector<Pagina> tabla_paginas;
};

// Estructura para representar la memoria swap
struct Memoria_Swap {
    int tam_memoria_swap_kb;
    vector<Pagina> paginas_swap;
};

vector<Pagina> memoria_RAM;
Memoria_Swap memoriaSwap;
vector<Proceso> lista_procesos;

int tam_memoria_fisica_kb;
int tam_pagina_kb;
int num_marcos_RAM;
int num_marcos_swap;

mutex mtx_memoria;
bool terminar_programa = false;

// Generar un valor aleatorio entre 1.5 y 4.5
float generarValorAleatorio() {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(1.5, 4.5);
    return dis(gen);
}

// Generar tamaño de proceso aleatorio entre min y max
int generarTamanioProceso(int min_kb, int max_kb) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(min_kb, max_kb);
    return dis(gen);
}

// Función para generar procesos
void generarProcesos(int tam_proceso_min_kb, int tam_proceso_max_kb) {
    int id_proceso_actual = 1;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis_pagina(1, tam_pagina_kb); // largo de pagina random entre 1 y tam_pagina_kb

    while (!terminar_programa) {
        this_thread::sleep_for(chrono::seconds(2));

        Proceso nuevo_proceso;
        nuevo_proceso.id_proceso = id_proceso_actual++;
        nuevo_proceso.tam_proceso_kb = generarTamanioProceso(tam_proceso_min_kb, tam_proceso_max_kb);
        nuevo_proceso.num_paginas = (nuevo_proceso.tam_proceso_kb + tam_pagina_kb - 1) / tam_pagina_kb;

        // Crear las páginas del proceso
        for (int i = 0; i < nuevo_proceso.num_paginas; ++i) {
            Pagina nueva_pagina;
            nueva_pagina.id_proceso = nuevo_proceso.id_proceso;
            nueva_pagina.num_pagina = dis_pagina(gen); // Assign random page number
            nueva_pagina.en_RAM = false;
            nuevo_proceso.tabla_paginas.push_back(nueva_pagina);
        }

        // Asignar páginas a RAM o Swap
        {
            lock_guard<mutex> lock(mtx_memoria);

            for (auto& pagina : nuevo_proceso.tabla_paginas) {
                if (memoria_RAM.size() < num_marcos_RAM) {
                    memoria_RAM.push_back(pagina);
                    pagina.en_RAM = true;
                }
                else if (memoriaSwap.paginas_swap.size() < num_marcos_swap) {
                    memoriaSwap.paginas_swap.push_back(pagina);
                    pagina.en_RAM = false;
                }
                else {
                    cout << "No hay espacio en RAM ni en Swap. Terminando simulación." << endl;
                    terminar_programa = true;
                    return;
                }
            }
            lista_procesos.push_back(nuevo_proceso);
            cout << "Proceso " << nuevo_proceso.id_proceso << " creado con tamaño " << nuevo_proceso.tam_proceso_kb << " KB (" << nuevo_proceso.num_paginas << " páginas)" << endl;
        }
    }
}

// Función para terminar procesos aleatoriamente
void terminarProcesoAleatorio() {
    random_device rd;
    mt19937 gen(rd());

    while (!terminar_programa) {
        this_thread::sleep_for(chrono::seconds(5));

        lock_guard<mutex> lock(mtx_memoria);

        if (!lista_procesos.empty()) {
            uniform_int_distribution<> dis(0, lista_procesos.size() - 1);
            int index = dis(gen);
            Proceso proceso_a_terminar = lista_procesos[index];

            // Liberar páginas de RAM y Swap
            memoria_RAM.erase(remove_if(memoria_RAM.begin(), memoria_RAM.end(),
                [proceso_a_terminar](Pagina& p) { return p.id_proceso == proceso_a_terminar.id_proceso; }), memoria_RAM.end());

            memoriaSwap.paginas_swap.erase(remove_if(memoriaSwap.paginas_swap.begin(), memoriaSwap.paginas_swap.end(),
                [proceso_a_terminar](Pagina& p) { return p.id_proceso == proceso_a_terminar.id_proceso; }), memoriaSwap.paginas_swap.end());

            lista_procesos.erase(lista_procesos.begin() + index);

            cout << "Proceso " << proceso_a_terminar.id_proceso << " terminado y sus páginas liberadas." << endl;
        }
    }
}

// Función para acceder a direcciones virtuales aleatoriamente
void accederDireccionAleatoria() {
    random_device rd;
    mt19937 gen(rd());

    while (!terminar_programa) {
        this_thread::sleep_for(chrono::seconds(5));

        lock_guard<mutex> lock(mtx_memoria);

        if (!lista_procesos.empty()) {
            uniform_int_distribution<> dis_proc(0, lista_procesos.size() - 1);
            int index_proceso = dis_proc(gen);
            Proceso& proceso = lista_procesos[index_proceso];

            uniform_int_distribution<> dis_pagina(0, proceso.num_paginas - 1);
            int num_pagina = dis_pagina(gen);

            Pagina& pagina = proceso.tabla_paginas[num_pagina];

            cout << "Accediendo a página " << num_pagina << " del proceso " << proceso.id_proceso << endl;

            if (pagina.en_RAM) {
                cout << "La página " << num_pagina << " del proceso " << proceso.id_proceso << " está en RAM. Acceso exitoso." << endl;
            }
            else{
                cout << "Page fault: La página " << num_pagina << " del proceso " << proceso.id_proceso << " no está en RAM." << endl;

                // Mover página a RAM
                if (memoria_RAM.size() < num_marcos_RAM) {
                    memoria_RAM.push_back(pagina);
                    pagina.en_RAM = true;

                    // Eliminar de Swap
                    memoriaSwap.paginas_swap.erase(remove_if(memoriaSwap.paginas_swap.begin(), memoriaSwap.paginas_swap.end(),
                        [pagina](Pagina& p) { return p.id_proceso == pagina.id_proceso && p.num_pagina == pagina.num_pagina; }), memoriaSwap.paginas_swap.end());

                    cout << "Página " << num_pagina << " del proceso " << proceso.id_proceso << " cargada en RAM." << endl;
                } else {
                    // Reemplazo FIFO
                    Pagina pagina_a_reemplazar = memoria_RAM.front();
                    memoria_RAM.erase(memoria_RAM.begin());
                    pagina_a_reemplazar.en_RAM = false;
                    memoriaSwap.paginas_swap.push_back(pagina_a_reemplazar);

                    memoria_RAM.push_back(pagina);
                    pagina.en_RAM = true;

                    cout << "Se reemplazó la página " << pagina_a_reemplazar.num_pagina << " del proceso " << pagina_a_reemplazar.id_proceso
                         << " por la página " << pagina.num_pagina << " del proceso " << pagina.id_proceso << " en RAM." << endl;
                }
            }
        }
    }
}

int main() {
    // Solicitar memoria física y tamaño de página
    float memoriaFisica_MB;
    cout << "Ingrese el tamaño de la memoria física en MB: ";
    cin >> memoriaFisica_MB;

    tam_memoria_fisica_kb = memoriaFisica_MB * 1024; // Convertir a KB

    cout << "Ingrese el tamaño de las páginas en KB: ";
    cin >> tam_pagina_kb;

    if (tam_memoria_fisica_kb <= 0 || tam_pagina_kb <= 0) {
        cerr << "Error: El tamaño de la memoria física y el tamaño de las páginas deben ser positivos." << endl;
        return 1;
    }

    // Solicitar tamaño mínimo y máximo de los procesos
    int tam_proceso_min_kb, tam_proceso_max_kb;
    cout << "Ingrese el tamaño mínimo de los procesos en KB: ";
    cin >> tam_proceso_min_kb;
    cout << "Ingrese el tamaño máximo de los procesos en KB: ";
    cin >> tam_proceso_max_kb;

    if (tam_proceso_min_kb <= 0 || tam_proceso_max_kb <= 0 || tam_proceso_min_kb > tam_proceso_max_kb) {
        cerr << "Error: Los tamaños de los procesos deben ser positivos y el tamaño mínimo debe ser menor o igual al tamaño máximo." << endl;
        return 1;
    }

    // Calcular memoria virtual
    float factor = generarValorAleatorio();
    int tam_memoria_virtual_kb = tam_memoria_fisica_kb * factor;

    // Calcular número de marcos
    num_marcos_RAM = tam_memoria_fisica_kb / tam_pagina_kb;
    int num_marcos_virtual = tam_memoria_virtual_kb / tam_pagina_kb;
    num_marcos_swap = num_marcos_virtual - num_marcos_RAM;

    memoriaSwap.tam_memoria_swap_kb = tam_memoria_virtual_kb - tam_memoria_fisica_kb;

    cout << "Memoria física: " << tam_memoria_fisica_kb << " KB (" << num_marcos_RAM << " marcos)" << endl;
    cout << "Memoria virtual: " << tam_memoria_virtual_kb << " KB (" << num_marcos_virtual << " marcos)" << endl;
    cout << "Memoria swap: " << memoriaSwap.tam_memoria_swap_kb << " KB (" << num_marcos_swap << " marcos)" << endl;

    // Iniciar hilos
    thread hilo_generador([tam_proceso_min_kb, tam_proceso_max_kb]() {
        generarProcesos(tam_proceso_min_kb, tam_proceso_max_kb);
    });
    thread hilo_terminador(terminarProcesoAleatorio);
    thread hilo_acceso(accederDireccionAleatoria);

    // Esperar a que el usuario presione una tecla para terminar
    cout << "Presione Enter para terminar la simulación..." << endl;
    cin.ignore();
    cin.get();
    terminar_programa = true;

    // Unir hilos
    hilo_generador.join();
    hilo_terminador.join();
    hilo_acceso.join();

    cout << "Simulación terminada." << endl;

    return 0;
}
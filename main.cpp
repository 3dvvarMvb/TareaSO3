#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <vector>
#include <queue>

using namespace std;

struct Proceso {
    float Tam_proceso;
    int pagina;
};

struct Memoria_Swap {
    float Tam_Memoria_Swap;
    vector<Proceso> Paginas_Swap;
};

vector<Proceso> Memoria;
Memoria_Swap MemoriaSwap;

float generarValorAleatorio() {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(1.5, 4.5);
    return dis(gen);
}

float generarTamanioProceso(int min, int max) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

void generarProcesos(vector<Proceso>& memoria, float Min_Proceso, float Max_Proceso, int Cantidad_frames, int Cantidad_paginas) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, Cantidad_paginas - 1);

    queue<Proceso> colaProcesos;

    while (true) {
        if (memoria.size()>=Cantidad_frames && (Memoria_Swap().Paginas_Swap).size()>=Cantidad_frames) {
            cout << "Memoria llena" << endl;
            exit(0);
        }

        Proceso nuevoProceso;
        nuevoProceso.Tam_proceso = generarTamanioProceso(Min_Proceso, Max_Proceso);
        nuevoProceso.pagina = dis(gen);
        colaProcesos.push(nuevoProceso);

        cout << "Nuevo proceso generado con tamaño: " << nuevoProceso.Tam_proceso << endl;

        // FIFO Paging Algorithm
        while (!colaProcesos.empty() && memoria.size() < Cantidad_frames) {
            Proceso proceso = colaProcesos.front();
            colaProcesos.pop();
            memoria.push_back(proceso);
            cout << "Proceso asignado a la memoria con tamaño: " << proceso.Tam_proceso << endl;
        }

        this_thread::sleep_for(chrono::seconds(2));
    }
}

void terminarProcesoAleatorio(vector<Proceso>& memoria) {
    random_device rd;
    mt19937 gen(rd());
    while (true) {
        this_thread::sleep_for(chrono::seconds(5));
        if (!memoria.empty()) {
            uniform_int_distribution<> dis(0, memoria.size() - 1);
            int index = dis(gen);
            cout << "Proceso terminado con tamaño: " << memoria[index].Tam_proceso << endl;
            memoria.erase(memoria.begin() + index);
        }
    }
}

void accederDireccionAleatoria(vector<Proceso>& memoria, Memoria_Swap& swap, int TamFrames, int Cantidad_paginas) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, Cantidad_paginas - 1);

    while (true) {
        this_thread::sleep_for(chrono::seconds(5));
        int paginaSolicitada = dis(gen);
        bool estaEnRAM = false;

        for (const Proceso& p : memoria) {
            if (p.pagina == paginaSolicitada) {
                estaEnRAM = true;
                cout << "Página " << paginaSolicitada << " accedida en RAM." << endl;
                break;
            }
        }

        if (!estaEnRAM) {
            cout << "Page fault: Página " << paginaSolicitada << " no está en RAM." << endl;
            // Mover página de swap a RAM
            if (memoria.size() >= TamFrames) {
                Proceso reemplazado = memoria.front(); // Política FIFO
                memoria.erase(memoria.begin());
                swap.Paginas_Swap.push_back(reemplazado);
                cout << "Página " << reemplazado.pagina << " reemplazada de RAM a Swap." << endl;
            }

            Proceso nuevaPagina;
            nuevaPagina.pagina = paginaSolicitada;
            memoria.push_back(nuevaPagina);
            cout << "Página " << paginaSolicitada << " cargada en RAM desde Swap." << endl;
        }
    }
}


int main() {
    float memoriaFisica, memoriaVirtual, TamanioPagina, TamFrames;
    cout << "Ingrese el tamaño de la memoria fisica en MB" << endl;
    cin >> memoriaFisica;

    memoriaVirtual = memoriaFisica * generarValorAleatorio();

    cout << "Ingresse el tamaño de las paginas" << endl;
    cin >> TamanioPagina;

    TamFrames = TamanioPagina;

    if (memoriaFisica <= 0 || TamanioPagina <= 0) {
        cerr << "Error: El tamaño de la memoria física y el tamaño de las páginas deben ser positivos." << endl;
        return 1;
    }

    int Cantidad_frames = memoriaFisica / TamFrames;
    int Cantidad_paginas = memoriaVirtual / TamanioPagina;

    float Min_Proceso, Max_Proceso;

    cout << "Ingrese el tamaño minimo y maximo del proceso en ese orden:" << endl;
    cin >> Min_Proceso;
    cin >> Max_Proceso;

    if (Min_Proceso > Max_Proceso) {
        cerr << "Error: El tamaño mínimo del proceso no puede ser mayor que el tamaño máximo." << endl;
        return 1;
    }

    Memoria.resize(Cantidad_paginas);

    MemoriaSwap.Tam_Memoria_Swap = memoriaVirtual;
    MemoriaSwap.Paginas_Swap.resize(Cantidad_paginas);

    thread generador(generarProcesos, ref(Memoria), Min_Proceso, Max_Proceso, Cantidad_frames);
    thread terminador(terminarProcesoAleatorio, ref(Memoria));

    // Para detener el programa después de un tiempo, por ejemplo, 20 segundos
    this_thread::sleep_for(chrono::seconds(20));
    generador.detach();
    terminador.detach();

    return 0;
}
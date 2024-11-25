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

void generarProcesos(vector<Proceso>& memoria, float Min_Proceso, float Max_Proceso, int Cantidad_frames) {
    queue<Proceso> colaProcesos;
    while (true) {
        Proceso nuevoProceso;
        nuevoProceso.Tam_proceso = generarTamanioProceso(Min_Proceso, Max_Proceso);
        colaProcesos.push(nuevoProceso);
        cout << "Nuevo proceso generado con tamaño: " << nuevoProceso.Tam_proceso << endl;

        // FCFS Paging Algorithm
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

int main() {
    float memoriaFisica, memoriaVirtual, TamanioPagina, TamFrames;
    cout << "Ingrese el tamaño de la memoria fisica en MB" << endl;
    cin >> memoriaFisica;

    memoriaVirtual = memoriaFisica * generarValorAleatorio();

    cout << "Ingresse el tamaño de las paginas" << endl;
    cin >> TamanioPagina;

    TamFrames = TamanioPagina;

    int Cantidad_frames = memoriaFisica / TamFrames;
    int Cantidad_paginas = memoriaVirtual / TamanioPagina;

    float Min_Proceso, Max_Proceso;

    cout << "Ingrese el tamaño minimo y maximo del proceso en ese orden:" << endl;
    cin >> Min_Proceso;
    cin >> Max_Proceso;

    vector<Proceso> Memoria;
    Memoria.resize(Cantidad_paginas);

    Memoria_Swap MemoriaSwap;
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
#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <vector>

using namespace std;

struct Proceso {
    float Tam_proceso;
};

struct Memoria_Swap {
    float Tam_Swap;
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

void generarProcesos(vector<Proceso>& procesos, float Min_Proceso, float Max_Proceso) {
    while (true) {
        Proceso nuevoProceso;
        nuevoProceso.Tam_proceso = generarTamanioProceso(Min_Proceso,Max_Proceso);
        procesos.push_back(nuevoProceso);
        cout << "Nuevo proceso generado con tamaño: " << nuevoProceso.Tam_proceso << endl;
        this_thread::sleep_for(chrono::seconds(2));
    }
}

int main() {
    float memoriaFisica, memoriaVirtual, TamañoPagina, TamFrames;
    cout << "Ingrese el tamaño de la memoria fisica en MB" << endl;
    cin >> memoriaFisica;

    memoriaVirtual = memoriaFisica * generarValorAleatorio();

    cout << "Ingresse el tamaño de las paginas" << endl;
    cin >> TamañoPagina;

    TamFrames = TamañoPagina;

    float Min_Proceso,Max_Proceso;

    cin>>Min_Proceso;
    cin>>Max_Proceso;

    vector<Proceso> procesos;
    thread generador(generarProcesos, ref(procesos));

    // Para detener el programa después de un tiempo, por ejemplo, 10 segundos
    this_thread::sleep_for(chrono::seconds(10));
    generador.detach(); // Detach the thread to allow the program to exit

    return 0;
}
// Kalman.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include "EKF.h"
#include "Orbit.h"
#include "InertialData.h"
#include "ReadADCS.h"
#include "ADCSDinamics.h"
	
int main(int argc,char** argv) {
    //Pass to main function number of cicles and time step [ms]
    int cicles=1,dt=1000;
    if (argc>1){
        cicles=atol(argv[1]);
    }
    printf("cicles: %d\n",cicles);
    if (argc>2){
        dt=atol(argv[2]);
    }
    printf("time step: %d ms\n",dt);
    //Tiempo global MJD = JD - 2400000.5 (25 ene 2024 00:00:00)
    double MJD_UTC = 60333;
    //Inertial models
    float R_i[3]{};
    float V_i[3]{};
    float LLA_i[3]{};
    float B_i[3]{};
    float S_i[3]{};
    //Kalman
    float Q_kalman[4]{};
    float Beta_kalman[3]{};
    float B_id[3]{};
    float S_id[3]{};
    float B_meas[3]{};
    float S_meas[3]{};
    float w_meas[3]{};
    //Matrices kalman constantes
    Matrix6TypeDef Gamma{};
    Matrix6TypeDef Q_k{};
    Matrix6TypeDef R_k{};
    Matrix6TypeDef P_k{};
    Init_kalman((dt/1000),Q_kalman,&Gamma,&Q_k,&R_k,&P_k);
    fstream fout;
    fout.open("DatosKalman.txt",ios::out);
    fout << "Log de Datos Kalman C++\n\n";
    fout.close();
    for(int i=0;i<cicles;i++){
        Cicle = i;
        GetOrbitData(MJD_UTC,R_i,V_i,LLA_i);
        getInertialSun(MJD_UTC,S_i);
        GetInertialMag(MJD_UTC,LLA_i,B_i);
        GetModelData(i,B_id,S_id,B_meas,S_meas,w_meas);
        kalman(dt/1000, Q_kalman, Beta_kalman, &P_k, B_i, S_i, B_meas, S_meas, w_meas,&Gamma,&Q_k,&R_k);
        /*cout << "Q:" << endl;
        for (int i = 0; i < 4; i++) {
            cout << Q_kalman[i] << " ";
        }
        cout << endl;
        cout << "Beta:" << endl;
        for (int i = 0; i < 3; i++) {
            cout << Beta_kalman[i] << " ";
        }
        cout << endl;
        cout << "P:" << endl;
        imprimirmatriz(&P_k);*/
        EscribirDatos(B_i,S_i,B_meas,S_meas,w_meas,Q_kalman, Beta_kalman);
        MJD_UTC += 1.157407407e-5;
    }
}


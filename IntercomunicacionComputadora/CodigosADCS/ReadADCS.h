/**
 ******************************************************************************
 * @file    InertialData.h
 * @author  Eduardo Munoz
 * @brief   Header file of inertial models of sun and magnetic field
 ******************************************************************************
   */
#ifndef READADCS_H
#define READADCS_H
#include <fstream>
#include <sstream>
#include <string>
#include <random>

void GetModelData(long int EpSec,double b_i[3],double s_i[3],double b_meas[3], double s_meas[3], double w_meas[3]) {
    fstream fin{};
    string line, valor;
    fin.open("DatosInercialesMatlab.csv", ios::in);
    for (int i = 0; i < EpSec; i++)
        getline(fin, line);
    getline(fin,line);
    stringstream stream(line);
    for (int i = 0; i < 3; i++) {
        getline(stream, valor, ',');
        b_i[i] = stof(valor);
    }
    for (int i = 0; i < 3; i++) {
        getline(stream, valor, ',');
        s_i[i] = stof(valor);
    }
    fin.close();
    fin.open("DatosSensoresMatlab.csv", ios::in);
    for(int i=0;i < EpSec;i++)
        getline(fin, line);
    getline(fin, line);
    stringstream stream2(line);
    for (int i = 0; i < 3; i++) {
        getline(stream2, valor, ',');
        b_meas[i] = stof(valor);
    }
    for (int i = 0; i < 3; i++) {
        getline(stream2, valor, ',');
        s_meas[i] = stof(valor);
    }
    for (int i = 0; i < 3; i++) {
        getline(stream2, valor, ',');
        w_meas[i] = stof(valor);
    }
    fin.close();
}

void GetModelData(long int EpSec, float b_i[3], float s_i[3], float b_meas[3], float s_meas[3], float w_meas[3]) {
    fstream fin{};
    string line, valor;
    fin.open("DatosInercialesMatlab.csv", ios::in);
    for (int i = 0; i < EpSec; i++)
        getline(fin, line);
    getline(fin, line);
    stringstream stream(line);
    for (int i = 0; i < 3; i++) {
        getline(stream, valor, ',');
        b_i[i] = stof(valor);
    }
    for (int i = 0; i < 3; i++) {
        getline(stream, valor, ',');
        s_i[i] = stof(valor);
    }
    fin.close();
    fin.open("DatosSensoresMatlab.csv", ios::in);
    for (int i = 0; i < EpSec; i++)
        getline(fin, line);
    getline(fin, line);
    stringstream stream2(line);
    for (int i = 0; i < 3; i++) {
        getline(stream2, valor, ',');
        b_meas[i] = stof(valor);
    }
    for (int i = 0; i < 3; i++) {
        getline(stream2, valor, ',');
        s_meas[i] = stof(valor);
    }
    for (int i = 0; i < 3; i++) {
        getline(stream2, valor, ',');
        w_meas[i] = stof(valor);
    }
    fin.close();
}

void GetBodyMag(long int EpSec, double q[4], double B_meas[3]) {
    fstream fin{};
    string line, valor;
    double B_i[3]{}, B_b[3]{};
    fin.open("Koto_MagFieldJ2000-25Jan24.csv", ios::in);
    getline(fin, line);
    // used for breaking words 
    for (long int i = 0; i < (EpSec + 1); i++)
        getline(fin, line);
    stringstream stream(line);
    getline(stream, valor, ',');
    for (int i = 0; i < 3; i++) {
        getline(stream, valor, ',');
        B_i[i] = stof(valor);
    }
    fin.close();
    const double mean = 0.0;
    const double stddev = 40;
    std::default_random_engine generator;
    std::normal_distribution<double> dist(mean, stddev);
    Matrix3TypeDef R{};
    RotateVec(q, &R);
    MatVecMult(&R, B_i, B_b);
    for (int i = 0; i < 3; i++)
        B_meas[i] = (B_b[i] + dist(generator))*1e-9;
}

void GetBodySun(long int EpSec, double q[4], double S_meas[3]) {
    fstream fin{};
    string line, valor;
    double S_i[3]{}, S_b[3]{};
    fin.open("Koto_SunJ2000-25Jan24.csv", ios::in);
    getline(fin, line);
    // used for breaking words 
    for (long int i = 0; i < (EpSec + 1); i++)
        getline(fin, line);
    stringstream stream(line);
    getline(stream, valor, ',');
    for (int i = 0; i < 3; i++) {
        getline(stream, valor, ',');
        S_i[i] = stof(valor);
    }
    fin.close();
    const double mean = 0.0;
    const double stddev = 0.002;
    std::default_random_engine generator;
    std::normal_distribution<double> dist(mean, stddev);
    Matrix3TypeDef R{};
    RotateVec(q, &R);
    MatVecMult(&R, S_i, S_b);
    for (int i = 0; i < 3; i++)
        S_meas[i] = S_b[i] + dist(generator);
}

void GetAngVel(double w_b[3], double movbias[3], double dt, double w_meas[3], double biasreal[3]) {
    const double mean = 0.0;
    const double stddev = 0.000017;
    std::default_random_engine generator;
    std::normal_distribution<double> dist(mean, stddev);
    float constbias[3] = { 0.0017, - 0.0034, -0.0017 };
    // Add Gaussian noise
    for (int i = 0; i < 3; i++) {
        movbias[i] = movbias[i] + dist(generator) * dt;
        biasreal[i] = constbias[i] + movbias[i] + dist(generator);
        w_meas[i] = w_b[i] + biasreal[i];
    }
}

#endif

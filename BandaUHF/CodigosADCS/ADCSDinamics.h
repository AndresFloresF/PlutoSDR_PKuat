/**
 ******************************************************************************
 * @file    InertialData.h
 * @author  Eduardo Munoz
 * @brief   Header file of inertial models of sun and magnetic field
 ******************************************************************************
   */
#ifndef ADCSDINAMICS_H
#define ADCSDINAMICS_H

#include "ADCSMath.h"
#include <fstream>
#include <sstream>
#include <string>

long int Cicle = 0;

void DinamicaSatelite(Matrix3TypeDef* J, double w_i[3], double T[3], double dt) {
	double J_w[3]{};
	MatVecMult(J, w_i,J_w);
	double cross_w_Jw[3]{};
	cross(w_i, J_w, cross_w_Jw);
	double Sum[3]{};
	for (int i = 0; i < 3; i++) {
		Sum[i]=T[i] - cross_w_Jw[i];
	}
	Matrix3TypeDef invJ{};
	inverse3(J, &invJ);
	double accang[3]{};
	MatVecMult(&invJ, Sum, accang);
	for (int i = 0; i < 3; i++)
		w_i[i] = w_i[i] + accang[i] * dt;
}

void CinematicaSatelite(double w_i[3], double q_i[4], double dt) {
	Matrix4TypeDef Omega{};
	Omega.data[0][1] = 0.5 * w_i[2];
	Omega.data[0][2] = 0.5 * -w_i[1];
	Omega.data[0][3] = 0.5 * w_i[0];
	Omega.data[1][0] = 0.5 * -w_i[2];
	Omega.data[1][2] = 0.5 * w_i[0];
	Omega.data[1][3] = 0.5 * w_i[1];
	Omega.data[2][0] = 0.5 * w_i[1];
	Omega.data[2][1] = 0.5 * -w_i[0];
	Omega.data[2][3] = 0.5 * w_i[2];
	Omega.data[3][0] = 0.5 * -w_i[0];
	Omega.data[3][1] = 0.5 * -w_i[1];
	Omega.data[3][2] = 0.5 * -w_i[2];
	double qdot[4]{};
	MatQuatMult(&Omega, q_i, qdot);
	for (int i = 0; i < 4; i++)
		q_i[i] = q_i[i] + qdot[i] * dt;
	normalize(q_i, 4);
}

void EscribirDatos(double B_i[3], double S_i[3], double B_meas[3], double S_meas[3], double W_meas[3], double Q_kalman[4], double Bias_kalman[3]) {
	fstream fout;
	string line, valor;
	fout.open("DatosC.csv", ios::out | ios::app);
	// Read the input 
		// Insert the data to file 
	fout << B_i[0] << ", "
		<< B_i[1] << ", "
		<< B_i[2] << ", "
		<< S_i[0] << ", "
		<< S_i[1] << ", "
		<< S_i[2] << ", "
		<< B_meas[0] << ", "
		<< B_meas[1] << ", "
		<< B_meas[2] << ", "
		<< S_meas[0] << ", "
		<< S_meas[1] << ", "
		<< S_meas[2] << ", "
		<< W_meas[0] << ", "
		<< W_meas[1] << ", "
		<< W_meas[2] << ", "
		<< Q_kalman[0] << ", "
		<< Q_kalman[1] << ", "
		<< Q_kalman[2] << ", "
		<< Q_kalman[3] << ", "
		<< Bias_kalman[0] << ", "
		<< Bias_kalman[1] << ", "
		<< Bias_kalman[2]
		<< "\n";
	fout.close();
}

void EscribirDatos(float B_i[3], float S_i[3], float B_meas[3], float S_meas[3], float W_meas[3], float Q_kalman[4], float Bias_kalman[3]) {
	fstream fout;
	string line, valor;
	fout.open("DatosC.csv", ios::out | ios::app);
	// Read the input 
		// Insert the data to file 
	fout << B_i[0] << ", "
		<< B_i[1] << ", "
		<< B_i[2] << ", "
		<< S_i[0] << ", "
		<< S_i[1] << ", "
		<< S_i[2] << ", "
		<< B_meas[0] << ", "
		<< B_meas[1] << ", "
		<< B_meas[2] << ", "
		<< S_meas[0] << ", "
		<< S_meas[1] << ", "
		<< S_meas[2] << ", "
		<< W_meas[0] << ", "
		<< W_meas[1] << ", "
		<< W_meas[2] << ", "
		<< Q_kalman[0] << ", "
		<< Q_kalman[1] << ", "
		<< Q_kalman[2] << ", "
		<< Q_kalman[3] << ", "
		<< Bias_kalman[0] << ", "
		<< Bias_kalman[1] << ", "
		<< Bias_kalman[2]
		<< "\n";
	fout.close();
}

void LogKalman(Matrix6TypeDef* M,string name) {
	fstream fout;
	string line, valor;
	fout.open("DatosKalman.txt", ios::out | ios::app);
	fout << Cicle<<"  "<< name << ":\n";
	for (int i = 0; i < 6; i++) {
		fout << "  ";
		for (int j = 0; j < 6; j++)
			fout << M->data[i][j] << " ";
		fout << "\n";
	}
	fout << "\n";
}

void LogKalman(Matrix4TypeDef* M, string name) {
	fstream fout;
	string line, valor;
	fout.open("DatosKalman.txt", ios::out | ios::app);
	fout << Cicle << "  " << name << ":\n";
	for (int i = 0; i < 4; i++) {
		fout << "  ";
		for (int j = 0; j < 4; j++)
			fout << M->data[i][j] << " ";
		fout << "\n";
	}
	fout << "\n";
}

void LogKalman(Matrix3TypeDef* M, string name) {
	fstream fout;
	string line, valor;
	fout.open("DatosKalman.txt", ios::out | ios::app);
	fout << Cicle << "  " << name << ":\n";
	for (int i = 0; i < 3; i++) {
		fout << "  ";
		for (int j = 0; j < 3; j++)
			fout << M->data[i][j] << " ";
		fout << "\n";
	}
	fout << "\n";
}

void LogKalman(double* v, int size, string name) {
	fstream fout;
	string line, valor;
	fout.open("DatosKalman.txt", ios::out | ios::app);
	fout << Cicle << "  " << name << ":\n";
	fout << "  ";
	for (int i = 0; i < size; i++)
			fout << v[i] << " ";
	fout << "\n";
}

void LogKalman(float* v, int size, string name) {
	fstream fout;
	string line, valor;
	fout.open("DatosKalman.txt", ios::out | ios::app);
	fout << Cicle << "  " << name << ":\n";
	fout << "  ";
	for (int i = 0; i < size; i++)
		fout << v[i] << " ";
	fout << "\n";
}

#endif

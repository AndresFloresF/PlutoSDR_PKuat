/**
 ******************************************************************************
 * @file    GlobalConfig.h
 * @author  Eduardo Mu�oz Arredondo
 * @brief   Header file of global config of ADCS
 ******************************************************************************
   */
#ifndef EKF_H
#define EKF_H

#include <iostream>
using namespace std;
#include "ADCSMath.h"
#include "ADCSDinamics.h"

//Varianzas sensores
constexpr auto sigma_m = 2e-6;
constexpr auto sigma_s = 0.03;
constexpr auto sigma_u = 0.001;
constexpr auto sigma_v = 0.003;

void S(float a[3], Matrix3TypeDef* m);
void A(float q[4], Matrix3TypeDef* R);
void imprimirmatriz(Matrix6TypeDef* m);
void imprimirmatriz(Matrix3TypeDef* m);
void imprimirmatriz(Matrix4TypeDef* m);

void Init_kalman(float dt, float Q_kalman[4], Matrix6TypeDef* Gamma, Matrix6TypeDef* Q_k, Matrix6TypeDef* R_k, Matrix6TypeDef* P_k) {
	//Inicializar matrices en cero
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			Gamma->data[i][j] = 0;
			Q_k->data[i][j] = 0;
			R_k->data[i][j] = 0;
		}
	}
	//Asignar valores de matrices constantes
	for (int i = 0; i < 3; i++) {
		//Gamma
		Gamma->data[i][i] = -1;
		Gamma->data[i + 3][i + 3] = 1;
		//Q_k
		Q_k->data[i][i] = sigma_v * sigma_v * dt + (1 / 3) * sigma_u * sigma_u * dt * dt * dt;
		Q_k->data[i][i + 3] = 0.5 * sigma_u * sigma_u * dt * dt;
		Q_k->data[i + 3][i] = 0.5 * sigma_u * sigma_u * dt * dt;
		Q_k->data[i + 3][i + 3] = sigma_u * sigma_u * dt;
		//R_k
		R_k->data[i][i] = sigma_m * sigma_m;
		R_k->data[i + 3][i + 3] = sigma_s * sigma_s;
	}
	//Orientacion inicial
	Q_kalman[0] = 1;
	Q_kalman[1] = 0;
	Q_kalman[2] = 0;
	Q_kalman[3] = 0;
	//Inicializar P
	P_k->data[0][0] = 0.025;
	P_k->data[1][1] = 0.025;
	P_k->data[2][2] = 0.025;
	P_k->data[3][3] = 0.01;
	P_k->data[4][4] = 0.01;
	P_k->data[5][5] = 0.01;
}

int kalman(float dt, float q_km1_km1[4], float Beta_km1[3], Matrix6TypeDef* P_km1_km1,
	float b_i[3], float s_i[3], float b_k_meas[3], float s_k_meas[3], float w_km1_meas[3], 
	Matrix6TypeDef* Gamma, Matrix6TypeDef* Q_k, Matrix6TypeDef* R_k) {
	LogKalman(w_km1_meas, 3, "w_km1_meas");
	LogKalman(Beta_km1, 3, "Beta_km1");
	/*Paso 1: Actualización de paso de tiempo*********************************************/
	float q_k_km1[4]{};
	Matrix6TypeDef P_k_km1{};
	{
		float w_k_km1[3]{};
		//w_k_km1
		for (int i = 0; i < 3; i++) {
			w_k_km1[i] = w_km1_meas[i] - Beta_km1[i];
		}
		LogKalman(w_k_km1, 3, "w_k_km1");
		//Omega_w (Ya se le hacen las operaciones para multiplicar con q_km1_km1)
		Matrix4TypeDef Omega_w{};
		Omega_w.data[0][0] = 1;
		Matrix3TypeDef S_w_k_km1{};
		S(w_k_km1, &S_w_k_km1);
		LogKalman(&S_w_k_km1, "S_w_k_km1");
		for (int i = 0; i < 3; i++) {
			Omega_w.data[0][i + 1] = -w_k_km1[i] * (dt / 2);
			Omega_w.data[i + 1][0] = w_k_km1[i] * (dt / 2);
			for (int j = 0; j < 3; j++) {
				if(i==j)
					Omega_w.data[i + 1][j + 1] = 1 + S_w_k_km1.data[j][i] * (dt / 2);
				else
				Omega_w.data[i + 1][j + 1] = S_w_k_km1.data[j][i] * (dt / 2);
			}
		}
		//Estimacion q_k_km1
		MatQuatMult(&Omega_w, q_km1_km1, q_k_km1);
		LogKalman(&Omega_w, "Omega");
		LogKalman(q_km1_km1,4, "q_km1_km1");
		LogKalman(q_k_km1, 4, "q_k_km1");
		//Phi_k
		{
			Matrix6TypeDef Phi_k{};
			Matrix3TypeDef S_w_k_km1_2{};
			Mat3MatMult(&S_w_k_km1, &S_w_k_km1, &S_w_k_km1_2);
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					if (i == j) {
						Phi_k.data[i][j] = 1 - dt * S_w_k_km1.data[i][j] + (dt * dt / 2) * S_w_k_km1_2.data[i][j];
						Phi_k.data[i][j + 3] = -dt + (dt / 2) * S_w_k_km1.data[i][j] - (dt * dt * dt / 6) * S_w_k_km1_2.data[i][j];
						Phi_k.data[i + 3][j + 3] = 1;
					}
					else {
						Phi_k.data[i][j] = -dt * S_w_k_km1.data[i][j] + (dt * dt / 2) * S_w_k_km1_2.data[i][j];
						Phi_k.data[i][j + 3] = (dt / 2) * S_w_k_km1.data[i][j] - (dt * dt * dt / 6) * S_w_k_km1_2.data[i][j];
						Phi_k.data[i + 3][j + 3] = 0;
					}
					Phi_k.data[i + 3][j] = 0;
				}
			}
			//Estimacion matriz de covarianza P
			//Phi*P*Phi^T
			Matrix6TypeDef Phi_P{};
			Mat6MatMult(&Phi_k, P_km1_km1, &Phi_P);
			Matrix6TypeDef Phi_k_T{};
			TransposeMat6(&Phi_k, &Phi_k_T);
			Matrix6TypeDef Phi_P_PhiT{};
			Mat6MatMult(&Phi_P, &Phi_k_T, &Phi_P_PhiT);
			//Gamma*Q*Gamma^T
			Matrix6TypeDef Gamma_Q;
			Mat6MatMult(Gamma, Q_k, &Gamma_Q);
			Matrix6TypeDef Gamma_T;
			TransposeMat6(Gamma, &Gamma_T);
			Matrix6TypeDef Gamma_Q_GammaT;
			Mat6MatMult(&Gamma_Q, &Gamma_T, &Gamma_Q_GammaT);
			for (int i = 0; i < 6; i++)
				for (int j = 0; j < 6; j++)
					P_k_km1.data[i][j] = Phi_P_PhiT.data[i][j] + Gamma_Q_GammaT.data[i][j];
			LogKalman(&P_k_km1, "P_k_km1");
		}
	}
	/*Paso 2: Actualización de la medicion**************************************/
	//r_k
	float r_k[6]{};
	Matrix6TypeDef H_k{};
	{
		float A_q_b_i[3]{};
		float A_q_s_i[3]{};
		{
			//z_k
			float z_k[6]{};
			for (int i = 0; i < 3; i++) {
				z_k[i] = b_k_meas[i];
				z_k[i + 3] = s_k_meas[i];
			}
			//h_k
			float h_k[6]{};
			{
				Matrix3TypeDef A_q{};
				A(q_k_km1, &A_q);
				LogKalman(q_k_km1,4, "q_k_km1");
				LogKalman(&A_q, "A_q");
				MatVecMult(&A_q, b_i, A_q_b_i);
				MatVecMult(&A_q, s_i, A_q_s_i);
				for (int i = 0; i < 3; i++) {
					h_k[i] = A_q_b_i[i];
					h_k[i + 3] = A_q_s_i[i];
				}
			}
			for (int i = 0; i < 6; i++)
				r_k[i] = z_k[i] - h_k[i];
		}
		//Calculo H
		{
			Matrix3TypeDef S_A_bi{};
			Matrix3TypeDef S_A_si{};
			S(A_q_b_i, &S_A_bi);
			S(A_q_s_i, &S_A_si);
			LogKalman(&S_A_bi, "S_A_bi");
			LogKalman(&S_A_si, "S_A_si");
			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 3; j++) {
					H_k.data[i][j] = S_A_bi.data[i][j];
					H_k.data[i + 3][j] = S_A_si.data[i][j];
					H_k.data[i][j + 3] = 0;
					H_k.data[i + 3][j + 3] = 0;
				}
			LogKalman(&H_k, "H_k");
		}
	}
	//Calculo K
	Matrix6TypeDef K_k;
	{
		Matrix6TypeDef H_kT{};
		TransposeMat6(&H_k, &H_kT);
		Matrix6TypeDef P_HT{};
		Mat6MatMult(&P_k_km1, &H_kT, &P_HT);
		Matrix6TypeDef H_P{};
		Mat6MatMult(&H_k, &P_k_km1, &H_P);
		Matrix6TypeDef H_P_HT{};
		Mat6MatMult(&H_P, &H_kT, &H_P_HT);
		Matrix6TypeDef H_P_HT_R{};
		for (int i = 0; i < 6; i++)
			for (int j = 0; j < 6; j++)
				H_P_HT_R.data[i][j] = H_P_HT.data[i][j] + R_k->data[i][j];
		Matrix6TypeDef Inv_H_P_HT_R{};
		inverse6(&H_P_HT_R, &Inv_H_P_HT_R);
		Mat6MatMult(&P_HT, &Inv_H_P_HT_R, &K_k);
		LogKalman(&K_k, "K_k");
	}
	//
	//error attitude y bias
	float error_attitude[3]{};
	float error_bias[3]{};
	{//error_x
		float error_x[6]{};
		Mat6VecMult(&K_k, r_k, error_x);
		LogKalman(error_x, 6, "error_x");
		for (int i = 0; i < 3; i++) {
			error_attitude[i] = error_x[i];
			error_bias[i] = error_x[i + 3];
		}
	}
	LogKalman(error_attitude, 3, "error_attitude");
	//error_q_k
	float error_q_k[4]{};
	error_q_k[0] = 1 / sqrt(1 - (norm(error_attitude, 3) * norm(error_attitude, 3) / 4));
	for (int i = 0; i < 3; i++) {
		error_q_k[i + 1] = error_q_k[0] * error_attitude[i] / 2;
	}
	LogKalman(q_k_km1, 4, "q_k_km1");
	LogKalman(error_q_k, 4, "error_q_k");
	//q_k_k (Aqui se sobreescribe el valor de q_km1_km1)
	QuatMult(q_k_km1, error_q_k, q_km1_km1);
	normalize(q_km1_km1,4);
	LogKalman(q_km1_km1,4, "q_k_k");
	//Beta_k_k (Aqui se sobreescribe el valor de Beta_km1)
	for (int i = 0; i < 3; i++)
		Beta_km1[i] = Beta_km1[i] + error_bias[i];
	LogKalman(Beta_km1, 3, "Beta_k");
	//Calculo P_k_k (Aqui se sobreescribe el valor de P_km1_km1)
	{
		Matrix6TypeDef K_H;
		Mat6MatMult(&K_k, &H_k, &K_H);
		for (int i = 0; i < 6; i++) {
			for (int j = 0; j < 6; j++) {
				if (i == j)
					K_H.data[i][j] = 1 - K_H.data[i][j];
				else
					K_H.data[i][j] = -K_H.data[i][j];
			}
		}
		Matrix6TypeDef K_H_P;
		Mat6MatMult(&K_H, &P_k_km1, &K_H_P);
		Matrix6TypeDef K_H_T;
		TransposeMat6(&K_H, &K_H_T);
		Matrix6TypeDef KH_P_KHT;
		Mat6MatMult(&K_H_P, &K_H_T, &KH_P_KHT);
		Matrix6TypeDef K_R;
		Mat6MatMult(&K_k, R_k, &K_R);
		Matrix6TypeDef K_T;
		TransposeMat6(&K_k, &K_T);
		Matrix6TypeDef K_R_KT;
		Mat6MatMult(&K_R, &K_T, &K_R_KT);
		for (int i = 0; i < 6; i++)
			for (int j = 0; j < 6; j++)
				P_km1_km1->data[i][j] = KH_P_KHT.data[i][j] + K_R_KT.data[i][j];
		LogKalman(P_km1_km1, "P_k_k");
	}
	return 0;
}

void imprimirmatriz(Matrix6TypeDef* m) {
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			cout << m->data[i][j] << "  ";
		}
		cout << endl;
	}
}

void imprimirmatriz(Matrix3TypeDef* m) {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			cout << m->data[i][j] << "  ";
		}
		cout << endl;
	}
}

void imprimirmatriz(Matrix4TypeDef* m) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			cout << m->data[i][j] << "  ";
		}
		cout << endl;
	}
}

#endif


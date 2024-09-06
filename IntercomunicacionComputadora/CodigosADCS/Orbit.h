
#ifndef ORBIT_H
#define ORBIT_H	

//TLE por defecto
const char ISSTLE1[] = "1 25544U 98067A   24024.30477502  .00027907  00000+0  49769-3 0  9992";
const char ISSTLE2[] = "2 25544  51.6419 313.2651 0004862 125.7509  18.8175 15.49851456436087";
#include "Sgp4.h"


void LoadOrbitParameters() {
	// read Earth orientation parameters
	//eopdata = fscanf(fid,'%i %d %d %i %f %f %f %f %f %f %f %f %i',[13 inf]);
	//float** eopdata2;
	//eopdata2 = new float* [735];
	//for (int i = 0; i < 735; i++)
	//	eopdata2[i] = new float[13]{};
	//llenar_arreglos_T("eopl2020.txt", eopdata);
	//csv2matrix("eop20202022.txt", eopdata2);
	//for (int i = 0; i < 10;i++) {
	//	for (int j = 0; j < 13; j++) {
	//		cout << eopdata2[i][j] << "   ";
	//	}
	//	cout << endl;
	//}
	//for (int i = 0; i < 735;i++) {
	//	for (int j = 0; j < 13; j++) {
	//		eopdata[i][j] = eopdata2[i][j];
	//	}
	//}

	//Read precesion and nutation parameters
	
	//iar80 = new int* [106];
	//for (int i = 0; i < 106; i++)
	//	iar80[i] = new int[4]{};
	
	//rar80 = new float* [106];
	//for (int i = 0; i < 106; i++)
	//	rar80[i] = new float[4]{};
	//readNutdata("nut80.dat", iar80, rar80);
}

void GetOrbitData(double MJD_UTC, float* R_i, float* V_i, float* LLA) {

	float R_teme[3]{};
	float V_teme[3]{};
	float R_ecef[3]{};
	float V_ecef[3]{};
	SatData Satdata;
	//TLEread("ISSTLE.txt", &Satdata);
	TLEread(&Satdata);
	double tsince = (MJD_UTC - Satdata.MJD_epoch) * 1440;
	//printf("tsince: %1.10f\n", tsince);
	sgp4(R_teme, V_teme, &Satdata, tsince);
	EOPDataTypeDef EOPData{};
	EOP(MJD_UTC,&EOPData);
	teme2eci(R_teme, V_teme, R_i, V_i, EOPData.T, EOPData.dpsi, EOPData.deps);
	//Vector R_ecef, V_ecef;
	teme2ecef(R_teme, V_teme, R_ecef, V_ecef, EOPData.T, EOPData.JD_UT1, EOPData.LOD, EOPData.x_pole, EOPData.y_pole, 0);
	eceftoLLA(R_ecef, LLA);
}
/// <summary>
/// Get rotation matrix from inertial to body frame
/// </summary>
/// <param name="S_b"></param>
/// <param name="B_b"></param>
/// <param name="S_i"></param>
/// <param name="B_i"></param>
/// <param name="A_bi"></param>
void TRIAD(float* S_b, float* B_b, float* S_i, float* B_i, Matrix3TypeDef* A_bi) {
	if (dot(S_b, B_b) == 1) {
		//Los vectores son paralelos
		A_bi->data[0][0] = 1;
		A_bi->data[0][1] = 0;
		A_bi->data[0][2] = 0;
		A_bi->data[1][0] = 0;
		A_bi->data[1][1] = 1;
		A_bi->data[1][2] = 0;
		A_bi->data[2][0] = 0;
		A_bi->data[2][1] = 0;
		A_bi->data[2][2] = 1;
	}
	else {
		//Xbody=S_b (se usa directo para ahorrar memoria)
		float Cross[3]{};
		cross(S_b, B_b,Cross);
		float crossNorm = norm(Cross,3);

		float Y_body[3]{};
		Y_body[0]= 1 / crossNorm * Cross[0];
		Y_body[1]= 1 / crossNorm * Cross[1];
		Y_body[2]= 1 / crossNorm * Cross[2];

		float Z_body[3]{};
		cross(S_b, Y_body,Z_body);

		//Xinertial=S_i (se usa directo para ahorrar memoria)
		cross(S_i, B_i, Cross);

		float Y_inertial[3]{};
		crossNorm = norm(Cross,3);
		Y_inertial[0]= 1 / crossNorm * Cross[0];
		Y_inertial[1]= 1 / crossNorm * Cross[1];
		Y_inertial[2]= 1 / crossNorm * Cross[2];

		float Z_inertial[3]{};
		cross(S_i, Y_inertial,Z_inertial);

		//Matrices de rotacion
		Matrix3TypeDef C_b;
		C_b.data[0][0] = S_b[0];
		C_b.data[1][0] = S_b[1];
		C_b.data[2][0] = S_b[2];
		C_b.data[0][1] = Y_body[0];
		C_b.data[1][1] = Y_body[1];
		C_b.data[2][1] = Y_body[2];
		C_b.data[0][2] = Z_body[0];
		C_b.data[1][2] = Z_body[1];
		C_b.data[2][2] = Z_body[2];

		Matrix3TypeDef C_i;
		C_i.data[0][0] = S_i[0];
		C_i.data[0][1] = S_i[1];
		C_i.data[0][2] = S_i[2];
		C_i.data[1][0] = Y_inertial[0];
		C_i.data[1][1] = Y_inertial[1];
		C_i.data[1][2] = Y_inertial[2];
		C_i.data[2][0] = Z_inertial[0];
		C_i.data[2][1] = Z_inertial[1];
		C_i.data[2][2] = Z_inertial[2];

		Mat3MatMult(&C_b,&C_i,A_bi);
	}
}

/// <summary>
/// Create rotation matrix and angular velocity vector from inertial frame to orbital frame
/// from position and velocity vectors in ECI, inertial to body matrix and angular velocity from sensor
/// </summary>
/// <param name="R_i"></param>
/// <param name="V_i"></param>
/// <param name="A_io"></param>
void LVLHframe(float* R_i, float* V_i, Matrix3TypeDef* A_bi, Matrix3TypeDef* A_oi) {
	float g2,g3;

	g3 = 1 / norm(R_i,3);

	float o3I[3]{};
	o3I[0] = -g3 * R_i[0];
	o3I[1] = -g3 * R_i[1];
	o3I[2] = -g3 * R_i[2];

	float Cross[3]{};
	cross(R_i,V_i,Cross);

	g2 = 1 / norm(Cross,3);

	float o2I[3]{};
	o2I[0] = -g2 * Cross[0];
	o2I[1] = -g2 * Cross[1];
	o2I[2] = -g2 * Cross[2];

	float Norm[3]{};
	Norm[0] = g2 * g3 * norm(R_i,3) * norm(R_i,3) * V_i[0];
	Norm[1] = g2 * g3 * norm(R_i,3) * norm(R_i,3) * V_i[1];
	Norm[2] = g2 * g3 * norm(R_i,3) * norm(R_i,3) * V_i[2];

	float Dot[3]{};
	Dot[0] = g2 * g3 * dot(R_i, V_i) * R_i[0];
	Dot[1] = g2 * g3 * dot(R_i, V_i) * R_i[1];
	Dot[2] = g2 * g3 * dot(R_i, V_i) * R_i[2];

	float o1I[3]{};
	restVec(Norm,Dot,o1I);

	A_oi->data[0][0] = o1I[0];
	A_oi->data[0][1] = o1I[1];
	A_oi->data[0][2] = o1I[2];
	A_oi->data[1][0] = o2I[0];
	A_oi->data[1][1] = o2I[1];
	A_oi->data[1][2] = o2I[2];
	A_oi->data[2][0] = o3I[0];
	A_oi->data[2][1] = o3I[1];
	A_oi->data[2][2] = o3I[2];

	//float w_oi_o[3]{};

	//Only second term has value
	//w_oi_o[0] = 0;
	//w_oi_o[1] = -norm(Cross,3) / (norm(R_i,3) * norm(R_i,3));
	//w_oi_o[2] = 0;

	Matrix3TypeDef A_io;
	TransposeMat3(A_oi, &A_io);

	Matrix3TypeDef A_bo;
	//A_bo = A_bi*A_oi';
	Mat3MatMult(A_bi, &A_io, &A_bo);

	//float w_oi_b[3]{};
	//w_oi_b = A_bo*w_oi_o
	//MatVecMult(&A_bo, w_oi_o, w_oi_b);
	//w_bo_b = w_bi_b - w_oi_b
	//restVec(w_bi_b,w_oi_b,w_bo_b);
}

/// <summary>
/// Transform from rotation matrix to quaternions
/// </summary>
/// <param name="A"></param>
/// <param name="q"></param>
void DCMtoQ(Matrix3TypeDef* A, float* q) {
	float t, mult;
	if (A->data[2][2] < 0) {
		if (A->data[0][0] > A->data[1][1]) {
			t = 1 + A->data[0][0] - A->data[1][1] - A->data[2][2];
			mult = 0.5 / sqrt(t);
			q[0] = -mult * (t);
			q[1] = -mult * (A->data[0][1] + A->data[1][0]);
			q[2] = -mult * (A->data[2][0] + A->data[0][2]);
			q[3] = -mult * (A->data[1][2] - A->data[2][1]);
		}
		else {
			t = 1 - A->data[0][0] + A->data[1][1] - A->data[2][2];
			mult = 0.5 / sqrt(t);
			q[0] = -mult * (A->data[0][1] + A->data[1][0]);
			q[1] = -mult * (t);
			q[2] = -mult * (A->data[1][2] + A->data[2][1]);
			q[3] = -mult * (A->data[2][0] - A->data[0][2]);
		}

	}
	else {
		if (A->data[0][0] < -A->data[1][1]) {
			t = 1 - A->data[0][0] - A->data[1][1] + A->data[2][2];
			mult = 0.5 / sqrt(t);
			q[0] = -mult * (A->data[2][0] + A->data[0][2]);
			q[1] = -mult * (A->data[1][2] + A->data[2][1]);
			q[2] = -mult * (t);
			q[3] = -mult * (A->data[0][1] - A->data[1][0]);
		}
		else {
			t = 1 + A->data[0][0] + A->data[1][1] + A->data[2][2];
			mult = 0.5 / sqrt(t);
			q[0] = -mult * (A->data[1][2] - A->data[2][1]);
			q[1] = -mult * (A->data[2][0] - A->data[0][2]);
			q[2] = -mult * (A->data[0][1] - A->data[1][0]);
			q[3] = -mult * (t);
		}
	}
}

#endif

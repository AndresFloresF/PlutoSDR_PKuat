/**
 ******************************************************************************
 * @file    ADCSMath.h
 * @author  Eduardo Munoz
 * @brief   Header file of static methods for vector/matrix operations
 ******************************************************************************
   */
#ifndef ADCSMATH_H
#define ADCSMATH_H	
#include <math.h>

#define vec_col		true
#define vec_row		false

typedef struct {
	float data[3][3];
}Matrix3TypeDef;

typedef struct {
	float data[6][6];
}Matrix6TypeDef;

typedef struct {
	float data[4][4];
}Matrix4TypeDef;

//Matt Pharr solution of differences of floats
inline float DifferenceOfProducts(float a, float b, float c, float d) {
	float cd = c * d;
	float err = std::fma(-c, d, cd);
	float dop = std::fma(a, b, -cd);
	return dop + err;
}

inline float SumOfProducts(float a, float b, float c, float d) {
	float cd = c * d;
	float err = std::fma(-c, d, cd);
	float dop = std::fma(a, b, cd);
	return dop + err;
}

void MatVecMult(Matrix3TypeDef* m, float v[3], float mult[3]) {
	for (int i = 0; i < 3; i++)
		mult[i] = 0;
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			mult[i] += m->data[i][j] * v[j];
}

void MatVecMult(Matrix3TypeDef* m, double v[3], double mult[3]) {
	for (int i = 0; i < 3; i++)
		mult[i] = 0;
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			mult[i] += m->data[i][j] * v[j];
}

void MatQuatMult(Matrix4TypeDef* m, float q[4], float mult[4]) {
	for (int i = 0; i < 4; i++)
		mult[i] = 0;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			mult[i] += m->data[i][j] * q[j];
}

void MatQuatMult(Matrix4TypeDef* m, double q[4], double mult[4]) {
	for (int i = 0; i < 4; i++)
		mult[i] = 0;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			mult[i] += m->data[i][j] * q[j];
}

void Mat6VecMult(Matrix6TypeDef* m, float v[6], float mult[6]) {
	for (int i = 0; i < 6; i++)
		mult[i] = 0;
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 6; j++)
			mult[i] += m->data[i][j] * v[j];
}

void Mat6VecMult(Matrix6TypeDef* m, double v[6], double mult[6]) {
	for (int i = 0; i < 6; i++)
		mult[i] = 0;
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 6; j++)
			mult[i] += m->data[i][j] * v[j];
}

void Mat3MatMult(Matrix3TypeDef* m1, Matrix3TypeDef* m2, Matrix3TypeDef* mult) {
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			mult->data[i][j] = 0;
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			for (int k = 0; k < 3; ++k)
				mult->data[i][j] += m1->data[i][k] * m2->data[k][j];
}

void Mat6MatMult(Matrix6TypeDef* m1, Matrix6TypeDef* m2, Matrix6TypeDef* mult) {
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 6; j++)
			mult->data[i][j] = 0;
	for (int i = 0; i < 6; ++i)
		for (int j = 0; j < 6; ++j)
			for (int k = 0; k < 6; ++k)
				mult->data[i][j] += m1->data[i][k] * m2->data[k][j];
}

void Mat34Mult(Matrix3TypeDef* m1, float m2[3][4], float mult[3][4]) {
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 4; ++j)
			for (int k = 0; k < 3; ++k)
			{
				mult[i][j] += m1->data[i][k] * m2[k][j];
			}
}

void Mat34VecMult(float m[3][4], float v[4], float mult[3]) {
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			mult[i] += m[i][j] * v[j];
		}
	}
}

void TransposeMat3(Matrix3TypeDef* m, Matrix3TypeDef* T) {
	T->data[0][0] = m->data[0][0];
	T->data[1][0] = m->data[0][1];
	T->data[2][0] = m->data[0][2];
	T->data[0][1] = m->data[1][0];
	T->data[1][1] = m->data[1][1];
	T->data[2][1] = m->data[1][2];
	T->data[0][2] = m->data[2][0];
	T->data[1][2] = m->data[2][1];
	T->data[2][2] = m->data[2][2];
}

void TransposeMat6(Matrix6TypeDef* m, Matrix6TypeDef* T) {
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 6; j++)
			T->data[j][i] = m->data[i][j];
}

void inverse6(Matrix6TypeDef* matriz, Matrix6TypeDef* inv)
{
	double inverse[36]{};
	double x[36]{};
	double s=0;
	double smax=0;
	int a = 0;
	int b_i=0;
	int b_tmp=0;
	int i=0;
	int j=0;
	int jA=0;
	int jp1j=0;
	int k=0;
	int kAcol=0;
	int mmj_tmp=0;
	int x_tmp=0;
	signed char ipiv[6];
	signed char p[6];
	signed char i1;
	for (i = 0; i < 6; i++) {
		for (j = 0; j < 6; j++) {
			inverse[a] = 0.0;
			x[a] = matriz->data[i][j];
			a++;
		}
	}
	for (i = 0; i < 6; i++) {
		ipiv[i] = (signed char)(i + 1);
	}
	for (j = 0; j < 5; j++) {
		mmj_tmp = 4 - j;
		b_tmp = j * 7;
		jp1j = b_tmp + 2;
		jA = 6 - j;
		kAcol = 0;
		smax = fabs(x[b_tmp]);
		for (k = 2; k <= jA; k++) {
			s = fabs(x[(b_tmp + k) - 1]);
			if (s > smax) {
				kAcol = k - 1;
				smax = s;
			}
		}
		if (x[b_tmp + kAcol] != 0.0F) {
			if (kAcol != 0) {
				jA = j + kAcol;
				ipiv[j] = (signed char)(jA + 1);
				for (k = 0; k < 6; k++) {
					kAcol = j + k * 6;
					smax = x[kAcol];
					x_tmp = jA + k * 6;
					x[kAcol] = x[x_tmp];
					x[x_tmp] = smax;
				}
			}
			i = (b_tmp - j) + 6;
			for (b_i = jp1j; b_i <= i; b_i++) {
				x[b_i - 1] /= x[b_tmp];
			}
		}
		jA = b_tmp;
		for (kAcol = 0; kAcol <= mmj_tmp; kAcol++) {
			smax = x[(b_tmp + kAcol * 6) + 6];
			if (smax != 0.0F) {
				i = jA + 8;
				jp1j = (jA - j) + 12;
				for (x_tmp = i; x_tmp <= jp1j; x_tmp++) {
					x[x_tmp - 1] += x[((b_tmp + x_tmp) - jA) - 7] * -smax;
				}
			}
			jA += 6;
		}
	}
	for (i = 0; i < 6; i++) {
		p[i] = (signed char)(i + 1);
	}
	for (k = 0; k < 5; k++) {
		i1 = ipiv[k];
		if (i1 > k + 1) {
			jA = p[i1 - 1];
			p[i1 - 1] = p[k];
			p[k] = (signed char)jA;
		}
	}
	for (k = 0; k < 6; k++) {
		x_tmp = 6 * (p[k] - 1);
		inverse[k + x_tmp] = 1.0F;
		for (j = k + 1; j < 7; j++) {
			i = (j + x_tmp) - 1;
			if (inverse[i] != 0.0F) {
				jp1j = j + 1;
				for (b_i = jp1j; b_i < 7; b_i++) {
					jA = (b_i + x_tmp) - 1;
					inverse[jA] -= inverse[i] * x[(b_i + 6 * (j - 1)) - 1];
				}
			}
		}
	}
	for (j = 0; j < 6; j++) {
		jA = 6 * j;
		for (k = 5; k >= 0; k--) {
			kAcol = 6 * k;
			i = k + jA;
			smax = inverse[i];
			if (smax != 0.0F) {
				inverse[i] = smax / x[k + kAcol];
				for (b_i = 0; b_i < k; b_i++) {
					x_tmp = b_i + jA;
					inverse[x_tmp] -= inverse[i] * x[b_i + kAcol];
				}
			}
		}
	}
	a = 0;
	for (i = 0; i < 6; i++) {
		for (j = 0; j < 6; j++) {
			inv->data[i][j] = inverse[a];
			a++;
		}
	}
}

float dot(float* v1, float* v2) {
	float dot = 0;
	for (int i = 0; i < 3; i++)
		dot += v1[i] * v2[i];
	return dot;
}

void cross(float* v1, float* v2, float* cross) {
	cross[0] = DifferenceOfProducts(v1[1], v2[2], v1[2], v2[1]);
	cross[1] = DifferenceOfProducts(v1[2], v2[0], v1[0], v2[2]);
	cross[2] = DifferenceOfProducts(v1[0], v2[1], v1[1], v2[0]);
}

void cross(double* v1, double* v2, double* cross) {
	cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
	cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
	cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

void sumVec(float* v1, float* v2, float* sum) {
	sum[0] = v1[0] + v2[0];
	sum[1] = v1[1] + v2[1];
	sum[2] = v1[2] + v2[2];
}

void restVec(float* v1, float* v2, float* rest) {
	rest[0] = v1[0] - v2[0];
	rest[1] = v1[1] - v2[1];
	rest[2] = v1[2] - v2[2];
}

float norm(float* v, int size) {
	float n = 0;
	for (int i = 0; i < size; i++)
		n += v[i] * v[i];
	n = sqrtf(n);
	return n;
}

double norm(double* v, int size) {
	double n = 0;
	for (int i = 0; i < size; i++)
		n += v[i] * v[i];
	n = sqrt(n);
	return n;
}

void normalize(float* v, int size) {
	float n = norm(v,size);
	if (n != 0)
		for (int i = 0; i < size; i++)
			v[i] = v[i] / n;
}

void normalize(double* v, int size) {
	double n = norm(v, size);
	if (n != 0)
		for (int i = 0; i < size; i++)
			v[i] = v[i] / n;
}

//float round(float a){
//	if((a - (int)a) >= 0.5)
//		a=(int)a + 1;
//	else
//		a=(int)a;
//}

void QuatMult(float q1[4], float q2[4], float mult[4]) {
	//mult[0] = q1[0] * q2[0] - q1[1] * q2[1] - q1[2] * q2[2] - q1[3] * q2[3];
	//mult[1] = q1[0] * q2[1] + q1[1] * q2[0] - q1[2] * q2[3] + q1[3] * q2[2];
	//mult[2] = q1[0] * q2[2] + q1[1] * q2[3] + q1[2] * q2[0] - q1[3] * q2[1];
	//mult[3] = q1[0] * q2[3] - q1[1] * q2[2] + q1[2] * q2[1] + q1[3] * q2[0];

	mult[0] = DifferenceOfProducts(q2[0], q1[0], q2[1], q1[1]) + DifferenceOfProducts(-q2[2], q1[2], q2[3], q1[3]);
	mult[1] = SumOfProducts(q2[0], q1[1], q2[1], q1[0]) + SumOfProducts(-q2[2], q1[3], q2[3], q1[2]);
	mult[2] = SumOfProducts(q2[0], q1[2], q2[1], q1[3]) + DifferenceOfProducts(q2[2], q1[0], q2[3], q1[1]);
	mult[3] = DifferenceOfProducts(q2[0], q1[3], q2[1], q1[2]) + SumOfProducts(q2[2], q1[1], q2[3], q1[0]);
}

void QuatMult(double q1[4], double q2[4], double mult[4]) {
	mult[0] = q2[0] * q1[0] - q2[1] * q1[1] - q2[2] * q1[2] - q2[3] * q1[3];
	mult[1] = q2[0] * q1[1] + q2[1] * q1[0] - q2[2] * q1[3] + q2[3] * q1[2];
	mult[2] = q2[0] * q1[2] + q2[1] * q1[3] + q2[2] * q1[0] - q2[3] * q1[1];
	mult[3] = q2[0] * q1[3] - q2[1] * q1[2] + q2[2] * q1[1] + q2[3] * q1[0];
}

void QuatError(float qtarget[4], float qactual[4], float qerror[4]) {
	float conjq[4]{};
	float invq[4]{};
	for (int i = 0; i < 3; i++)
		conjq[i] = -qtarget[i];
	conjq[3] = qtarget[3];
	float normconjq = norm(conjq, 4);
	for (int i = 0; i < 4; i++)
		invq[i] = conjq[i] / normconjq;
	QuatMult((float*)invq, (float*)qactual, (float*)qerror);
}

int CA216(unsigned char msb, unsigned char lsb) {
	int n = 0;
	unsigned char n1 = ~msb;
	unsigned char n2 = ~lsb;
	n = (-1) * ((n1 << 8 | n2) + 1);
	return n;
}

//Esta funcion se usa para la parte de calibracion[1x3][3x3], es diferente a la de MatVecMult[3x3][3x1]
int matrixmult(float vec[3], const float mat[3][3], float mult[3]) {
	for (int i = 0; i < 3; i++)
		mult[i] = 0;
	for (int j = 0; j < 3; j++)
		for (int l = 0; l < 3; l++)
			mult[j] += vec[l] * mat[l][j];
	return 0;
}


double roundc(double a) {
	if ((a - (int16_t)a) < 0.5)
		return (int16_t)a;
	else
		return (int16_t)a + 1;
}

void inverse3(Matrix3TypeDef* m, Matrix3TypeDef* minv) {
	//double det = m->data[0][0] * (m->data[1][1] * m->data[2][2] - m->data[2][1] * m->data[1][2]) -
	//	m->data[0][1] * (m->data[1][0] * m->data[2][2] - m->data[1][2] * m->data[2][0]) +
	//	m->data[0][2] * (m->data[1][0] * m->data[2][1] - m->data[1][1] * m->data[2][0]);

	double det = m->data[0][0] * (DifferenceOfProducts(m->data[1][1], m->data[2][2], m->data[2][1], m->data[1][2])) -
		m->data[0][1] * (DifferenceOfProducts(m->data[1][0], m->data[2][2], m->data[1][2], m->data[2][0])) +
		m->data[0][2] * (DifferenceOfProducts(m->data[1][0], m->data[2][1], m->data[1][1], m->data[2][0]));
	double invdet = 1 / det;

	/*minv->data[0][0] = (m->data[1][1] * m->data[2][2] - m->data[2][1] * m->data[1][2]) * invdet;
	minv->data[0][1] = (m->data[0][2] * m->data[2][1] - m->data[0][1] * m->data[2][2]) * invdet;
	minv->data[0][2] = (m->data[0][1] * m->data[1][2] - m->data[0][2] * m->data[1][1]) * invdet;
	minv->data[1][0] = (m->data[1][2] * m->data[2][0] - m->data[1][0] * m->data[2][2]) * invdet;
	minv->data[1][1] = (m->data[0][0] * m->data[2][2] - m->data[0][2] * m->data[2][0]) * invdet;
	minv->data[1][2] = (m->data[1][0] * m->data[0][2] - m->data[0][0] * m->data[1][2]) * invdet;
	minv->data[2][0] = (m->data[1][0] * m->data[2][1] - m->data[2][0] * m->data[1][1]) * invdet;
	minv->data[2][1] = (m->data[2][0] * m->data[0][1] - m->data[0][0] * m->data[2][1]) * invdet;
	minv->data[2][2] = (m->data[0][0] * m->data[1][1] - m->data[1][0] * m->data[0][1]) * invdet;*/

	minv->data[0][0] = DifferenceOfProducts(m->data[1][1], m->data[2][2], m->data[2][1], m->data[1][2]) * invdet;
	minv->data[0][1] = DifferenceOfProducts(m->data[0][2], m->data[2][1], m->data[0][1], m->data[2][2]) * invdet;
	minv->data[0][2] = DifferenceOfProducts(m->data[0][1], m->data[1][2], m->data[0][2], m->data[1][1]) * invdet;
	minv->data[1][0] = DifferenceOfProducts(m->data[1][2], m->data[2][0], m->data[1][0], m->data[2][2]) * invdet;
	minv->data[1][1] = DifferenceOfProducts(m->data[0][0], m->data[2][2], m->data[0][2], m->data[2][0]) * invdet;
	minv->data[1][2] = DifferenceOfProducts(m->data[1][0], m->data[0][2], m->data[0][0], m->data[1][2]) * invdet;
	minv->data[2][0] = DifferenceOfProducts(m->data[1][0], m->data[2][1], m->data[2][0], m->data[1][1]) * invdet;
	minv->data[2][1] = DifferenceOfProducts(m->data[2][0], m->data[0][1], m->data[0][0], m->data[2][1]) * invdet;
	minv->data[2][2] = DifferenceOfProducts(m->data[0][0], m->data[1][1], m->data[1][0], m->data[0][1]) * invdet;
}

void S(float a[3], Matrix3TypeDef* m) {
	m->data[0][0] = 0;
	m->data[0][1] = -a[2];
	m->data[0][2] = a[1];
	m->data[1][0] = a[2];
	m->data[1][1] = 0;
	m->data[1][2] = -a[0];
	m->data[2][0] = -a[1];
	m->data[2][1] = a[0];
	m->data[2][2] = 0;
}

void S(double a[3], Matrix3TypeDef* m) {
	m->data[0][0] = 0;
	m->data[0][1] = -a[2];
	m->data[0][2] = a[1];
	m->data[1][0] = a[2];
	m->data[1][1] = 0;
	m->data[1][2] = -a[0];
	m->data[2][0] = -a[1];
	m->data[2][1] = a[0];
	m->data[2][2] = 0;
}

void A(float q[4], Matrix3TypeDef* R) {
	//Funcion para kalman (q0 q1 q2 q3)
	/*R->data[0][0] = q[1] * q[1] - q[2] * q[2] - q[3] * q[3] + q[0] * q[0];
	R->data[0][1] = 2 * (q[1] * q[2] + q[3] * q[0]);
	R->data[0][2] = 2 * (q[1] * q[3] - q[2] * q[0]);
	R->data[1][0] = 2 * (q[1] * q[2] - q[3] * q[0]);
	R->data[1][1] = -q[1] * q[1] + q[2] * q[2] - q[3] * q[3] + q[0] * q[0];
	R->data[1][2] = 2 * (q[2] * q[3] + q[1] * q[0]);
	R->data[2][0] = 2 * (q[1] * q[3] + q[2] * q[0]);
	R->data[2][1] = 2 * (q[2] * q[3] - q[1] * q[0]);
	R->data[2][2] = -q[1] * q[1] - q[2] * q[2] + q[3] * q[3] + q[0] * q[0];*/

	R->data[0][0] = DifferenceOfProducts(q[1], q[1], q[2], q[2]) + SumOfProducts(-q[3], q[3], q[0], q[0]);
	R->data[0][1] = 2 * SumOfProducts(q[1], q[2], q[3], q[0]);
	R->data[0][2] = 2 * DifferenceOfProducts(q[1], q[3], q[2], q[0]);
	R->data[1][0] = 2 * DifferenceOfProducts(q[1], q[2], q[3], q[0]);
	R->data[1][1] = SumOfProducts(-q[1], q[1], q[2], q[2]) + SumOfProducts(-q[3], q[3], q[0], q[0]);
	R->data[1][2] = 2 * SumOfProducts(q[2], q[3], q[1], q[0]);
	R->data[2][0] = 2 * SumOfProducts(q[1], q[3], q[2], q[0]);
	R->data[2][1] = 2 * DifferenceOfProducts(q[2], q[3], q[1], q[0]);
	R->data[2][2] = DifferenceOfProducts(-q[1], q[1], q[2], q[2]) + SumOfProducts(q[3], q[3], q[0], q[0]);
}

void A(double q[4], Matrix3TypeDef* R) {
	//Funcion para kalman (q0 q1 q2 q3)
	R->data[0][0] = q[1] * q[1] - q[2] * q[2] - q[3] * q[3] + q[0] * q[0];
	R->data[0][1] = 2 * (q[1] * q[2] + q[3] * q[0]);
	R->data[0][2] = 2 * (q[1] * q[3] - q[2] * q[0]);
	R->data[1][0] = 2 * (q[1] * q[2] - q[3] * q[0]);
	R->data[1][1] = -q[1] * q[1] + q[2] * q[2] - q[3] * q[3] + q[0] * q[0];
	R->data[1][2] = 2 * (q[2] * q[3] + q[1] * q[0]);
	R->data[2][0] = 2 * (q[1] * q[3] + q[2] * q[0]);
	R->data[2][1] = 2 * (q[2] * q[3] - q[1] * q[0]);
	R->data[2][2] = -q[1] * q[1] - q[2] * q[2] + q[3] * q[3] + q[0] * q[0];
}

void RotateVec(float q[4], Matrix3TypeDef* R) {
	//Funcion para resto de funciones (q1 q2 q3 q4)
	/*R->data[0][0] = q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3];
	R->data[0][1] = 2 * (q[0] * q[1] + q[2] * q[3]);
	R->data[0][2] = 2 * (q[0] * q[2] - q[1] * q[3]);
	R->data[1][0] = 2 * (q[0] * q[1] - q[2] * q[3]);
	R->data[1][1] = -q[0] * q[0] + q[1] * q[1] - q[2] * q[2] + q[3] * q[3];
	R->data[1][2] = 2 * (q[1] * q[2] + q[0] * q[3]);
	R->data[2][0] = 2 * (q[0] * q[2] + q[1] * q[3]);
	R->data[2][1] = 2 * (q[1] * q[2] - q[0] * q[3]);
	R->data[2][2] = -q[0] * q[0] - q[1] * q[1] + q[2] * q[2] + q[3] * q[3];*/

	R->data[0][0] = DifferenceOfProducts(q[0], q[0], q[1], q[1]) + SumOfProducts(-q[2], q[2], q[3], q[3]);
	R->data[0][1] = 2 * SumOfProducts(q[0], q[1], q[2], q[3]);
	R->data[0][2] = 2 * DifferenceOfProducts(q[0], q[2], q[1], q[3]);
	R->data[1][0] = 2 * DifferenceOfProducts(q[0], q[1], q[2], q[3]);
	R->data[1][1] = SumOfProducts(-q[0], q[0], q[1], q[1]) + SumOfProducts(-q[2], q[2], q[3], q[3]);
	R->data[1][2] = 2 * SumOfProducts(q[1], q[2], q[0], q[3]);
	R->data[2][0] = 2 * SumOfProducts(q[0], q[2], q[1], q[3]);
	R->data[2][1] = 2 * DifferenceOfProducts(q[1], q[2], q[0], q[3]);
	R->data[2][2] = DifferenceOfProducts(-q[0], q[0], q[1], q[1]) + SumOfProducts(q[2], q[2], q[3], q[3]);
}

void RotateVec(double q[4], Matrix3TypeDef* R) {
	//Funcion para resto de funciones (q1 q2 q3 q4)
	R->data[0][0] = q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3];
	R->data[0][1] = 2 * (q[0] * q[1] + q[2] * q[3]);
	R->data[0][2] = 2 * (q[0] * q[2] - q[1] * q[3]);
	R->data[1][0] = 2 * (q[0] * q[1] - q[2] * q[3]);
	R->data[1][1] = -q[0] * q[0] + q[1] * q[1] - q[2] * q[2] + q[3] * q[3];
	R->data[1][2] = 2 * (q[1] * q[2] + q[0] * q[3]);
	R->data[2][0] = 2 * (q[0] * q[2] + q[1] * q[3]);
	R->data[2][1] = 2 * (q[1] * q[2] - q[0] * q[3]);
	R->data[2][2] = -q[0] * q[0] - q[1] * q[1] + q[2] * q[2] + q[3] * q[3];
}

/*class Matrix;
class MiscOperators {
public:
	static float abs(float n) {
		if (n < 0)
			n = -n;
		return n;
	}
	int CA216(unsigned char msb, unsigned char lsb) {
		int n = 0;
		unsigned char n1 = ~msb;
		unsigned char n2 = ~lsb;
		n = (-1) * ((n1 << 8 | n2) + 1);
		return n;
	}
};
class Byte {
public:
	static int CA2(unsigned char msb, unsigned char lsb) {
		unsigned char n1 = ~msb;
		unsigned char n2 = ~lsb;
		return (-1) * ((n1 << 8 | n2) + 1);
	}
};*/


/*class Matrix {
private:
	float** data;
public:
	int Rows;
	int Cols;

	/// <summary>
	/// Constructor without size
	/// </summary>
	Matrix() {
		Rows = 0;
		Cols = 0;
		data = new float* [Rows];
		for (int i = 0; i < Rows; i++) {
			data[i] = new float[Cols];
		}
	}

	/// <summary>
	/// Constructor with given matrix size
	/// </summary>
	Matrix(int rows, int cols) {
		Rows = rows;
		Cols = cols;
		//allocate memory
		data = new float[Rows * Cols];
		for (int i = 0; i < Rows * Cols; i++)
			data[i] = 0;
		*/
		//AllocateMemory(rows, cols);
	//}

	/// <summary>
	/// Constructor with given vector object
	/// </summary>
	/*Matrix(Vector* vec) {
		if (vec->Column) {
			Rows = vec->Size;
			Cols = 1;
		}
		else {
			Rows = 1;
			Cols = vec->Size;
		}
		//allocate memory
		AllocateMemory(Rows, Cols);
		for (int i = 0; i < Rows; i++)
			for (int j = 0; j < Cols; j++)
				data[i][j] = vec->Components[i];
	}*/

	/// <summary>
	/// Constructor with given size and data in form of array 1D
	/// </summary>
	/*Matrix(int rows, int cols, float** arrayData) {
		SetDataArray(arrayData, rows, cols);
	}

	/// <summary>
	/// Destructor with dynamic array data deallocating
	/// </summary>
	~Matrix() {
		//dealocate dynamic array
		for (int i = 0; i < Rows; i++)
			delete[] data[i];
		delete[] data;
		data = NULL;
		//cout << "Matriz destruida" << endl;
	}

	/// <summary>
	/// Allocate dynamic memory for given size
	/// </summary>
	void AllocateMemory(int rows, int cols) {
		Rows = rows;
		Cols = cols;
		data = new float* [Rows];
		for (int i = 0; i < Rows; i++) {
			data[i] = new float[Cols] {};
		}
	}

	/// <summary>
	/// Allocate dynamic memory for given size and save data array
	/// </summary>
	void SetDataArray(float** arrayData, int rows, int cols) {
		AllocateMemory(rows, cols);
		for (int i = 0; i < Rows; i++)
			for (int j = 0; j < Cols; j++)
				data[i][j] = arrayData[i][j];
	}

	/// <summary>
	/// Save values to matrix data in given index position if exist
	/// </summary>
	/// <returns>Operation executed</returns>
	bool SetValue(float value, int row, int column) {
		if (row < Rows && column < Cols) {
			data[row][column] = value;
			return true;
		}
		else
			return false;
	}

	float** Data() {
		return data;
	}

	float Data(int row, int column) {
		if (row < Rows && column < Cols) {
			return data[row][column];
		}
		else
			return 0;
	}

	/// <summary>
	/// Perform a sum of two given matrix objects if size is the same
	/// </summary>
	/// <returns>Operation executed</returns>
	bool Sum(Matrix* m1, Matrix* m2) {
		if (m1->Rows == m2->Rows && m1->Cols == m2->Cols) {
			if (Rows != m1->Rows || Cols != m1->Cols) {
				AllocateMemory(m1->Rows, m1->Cols);
			}
			for (int i = 0;i < m1->Rows; i++) {
				for (int j = 0;j < m1->Cols; j++)
					data[i][j] = m1->data[i][j] + m2->data[i][j];
			}
			return true;
		}
		else
			return false;

	}


	/// <summary>
	/// Perform a rest of two given matrix objects if size is the same
	/// </summary>
	/// <returns>Operation executed</returns>
	bool Rest(Matrix* m1, Matrix* m2) {
		if (m1->Rows == m2->Rows && m1->Cols == m2->Cols) {
			if (Rows != m1->Rows || Cols != m1->Cols) {
				AllocateMemory(m1->Rows, m1->Cols);
			}
			for (int i = 0;i < m1->Rows; i++) {
				for (int j = 0;j < m1->Cols; j++)
					data[i][j] = m1->data[i][j] - m2->data[i][j];
			}
			return true;
		}
		else
			return false;
	}

	/// <summary>
	/// Perform a multiplication of two given matrix objects if sizes are correct
	/// </summary>
	/// <returns>Operation executed</returns>
	bool Mult(Matrix* m1, Matrix* m2) {
		if (m1->Cols == m2->Rows) {

			AllocateMemory(m1->Rows, m2->Cols);

			for (int i = 0; i < m1->Rows; ++i)
				for (int j = 0; j < m2->Cols; ++j)
					for (int k = 0; k < m1->Cols; ++k)
					{
						data[i][j] += m1->Data(i, k) * m2->Data(k, j);
					}
			return true;
		}
		else
			return false;
	}

	/// <summary>
	/// Get a new matrix from transposing a given matrix object
	/// </summary>
	void Transpose(Matrix* m) {

		AllocateMemory(m->Cols, m->Rows);

		for (int i = 0;i < Rows;i++) {
			for (int j = 0; j < Cols; j++) {
				data[i][j] = m->data[j][i];
			}
		}
	}

	/// <summary>
	/// Get a new matrix from inversing a given 3x3 matrix object
	/// if matrix is invertible
	/// </summary>
	/// <returns>Operation executed</returns>
	bool Inverse3(Matrix* m) {
		if (m->Rows == 3 && m->Cols == 3) {

			AllocateMemory(m->Rows, m->Cols);

			float determinant = 0;
			for (int i = 0; i < 3; i++)
				determinant += (m->Data(0, i) * (m->Data(1, (i + 1) % 3) * m->Data(2, (i + 2) % 3) - m->Data(1, (i + 2) % 3) * m->Data(2, (i + 1) % 3)));
			if (determinant != 0) {
				for (int i = 0; i < 3; i++) {
					for (int j = 0; j < 3; j++)
						data[i][j] = ((m->Data((i + 1) % 3, (j + 1) % 3) * m->Data((i + 2) % 3, (j + 2) % 3)) - (m->Data((i + 1) % 3, (j + 2) % 3) * m->Data((i + 2) % 3, (j + 1) % 3))) / determinant;
				}
				return true;
			}
			else
				return false;
		}
		else
			return false;
	}

	/// <summary>
	/// Get a new matrix from inversing a given 7x7 matrix object
	/// if matrix is invertible
	/// </summary>
	/// <returns>Operation executed</returns>
	bool Inverse7(Matrix* m) {
		if (m->Rows == 7 && m->Cols == 7) {

			AllocateMemory(7, 7);

			float* x = new float[49]{};
			float* inv = new float[49]{};
			float smax, s;
			int b_i = 0, c_i, j, jA, jp1j, k, kAcol, x_tmp, b_tmp, mmj_tmp;
			signed char ipiv[7], p[7], i1;
			for (int i = 0; i < 7; i++) {
				for (int j = 0; j < 7; j++) {
					x[b_i] = m->Data(i, j);
					b_i++;
				}
			}
			for (b_i = 0; b_i < 7; b_i++)
				ipiv[b_i] = static_cast<signed char>(b_i + 1);
			for (j = 0; j < 6; j++) {
				mmj_tmp = 5 - j;
				b_tmp = j << 3;
				jp1j = b_tmp + 2;
				jA = 7 - j;
				kAcol = 0;
				smax = MiscOperators::abs(x[b_tmp]);
				for (k = 2; k <= jA; k++) {
					s = MiscOperators::abs(x[(b_tmp + k) - 1]);
					if (s > smax) {
						kAcol = k - 1;
						smax = s;
					}
				}
				if (x[b_tmp + kAcol] != 0.0) {
					if (kAcol != 0) {
						jA = j + kAcol;
						ipiv[j] = static_cast<signed char>(jA + 1);
						for (k = 0; k < 7; k++) {
							kAcol = j + k * 7;
							smax = x[kAcol];
							x_tmp = jA + k * 7;
							x[kAcol] = x[x_tmp];
							x[x_tmp] = smax;
						}
					}
					b_i = (b_tmp - j) + 7;
					for (c_i = jp1j; c_i <= b_i; c_i++)
						x[c_i - 1] /= x[b_tmp];
				}
				jA = b_tmp;
				for (kAcol = 0; kAcol <= mmj_tmp; kAcol++) {
					smax = x[(b_tmp + kAcol * 7) + 7];
					if (smax != 0.0) {
						b_i = jA + 9;
						jp1j = (jA - j) + 14;
						for (x_tmp = b_i; x_tmp <= jp1j; x_tmp++)
							x[x_tmp - 1] += x[((b_tmp + x_tmp) - jA) - 8] * -smax;
					}
					jA += 7;
				}
			}
			for (b_i = 0; b_i < 7; b_i++)
				p[b_i] = static_cast<signed char>(b_i + 1);
			for (k = 0; k < 6; k++) {
				i1 = ipiv[k];
				if (i1 > k + 1) {
					jA = p[i1 - 1];
					p[i1 - 1] = p[k];
					p[k] = static_cast<signed char>(jA);
				}
			}
			for (k = 0; k < 7; k++) {
				x_tmp = 7 * (p[k] - 1);
				inv[k + x_tmp] = 1.0;
				for (j = k + 1; j < 8; j++) {
					b_i = (j + x_tmp) - 1;
					if (inv[b_i] != 0.0) {
						jp1j = j + 1;
						for (c_i = jp1j; c_i < 8; c_i++) {
							jA = (c_i + x_tmp) - 1;
							inv[jA] -= inv[b_i] * x[(c_i + 7 * (j - 1)) - 1];
						}
					}
				}
			}
			for (j = 0; j < 7; j++) {
				jA = 7 * j;
				for (k = 6; k >= 0; k--) {
					kAcol = 7 * k;
					b_i = k + jA;
					smax = inv[b_i];
					if (smax != 0.0) {
						inv[b_i] = smax / x[k + kAcol];
						for (c_i = 0; c_i < k; c_i++) {
							x_tmp = c_i + jA;
							inv[x_tmp] -= inv[b_i] * x[c_i + kAcol];
						}
					}
				}
			}
			b_i = 0;
			for (int i = 0; i < 7; i++) {
				for (int j = 0; j < 7; j++) {
					data[i][j] = inv[b_i];
					b_i++;
				}
			}
			return true;
		}
		else
			return false;
	}*/

	/*void Print() {
		int j = 0;
		for (int i = 0;i < Rows; i++) {
			for (int j = 0; j < Cols; j++)
				cout << data[i][j] << " ";
			cout << endl;
		}
	}*/

	//};


	/*class Vector {
	public:
		float* Components;
		int Size;
		bool Column;

		Vector() {
			Size = 0;
			Components = new float[Size] {};
			Column = vec_row;
		}
		/// <summary>
		/// Constructor with given size and column condition
		/// </summary>
		Vector(int size, bool column) {
			Size = size;
			Components = new float[Size] {};
			Column = column;
		}

		/// <summary>
		/// Constructor with given size and column condition
		/// and given data array
		/// </summary>
		Vector(int size, bool column, float* vec) {
			Size = size;
			Components = new float[Size];
			for (int i = 0; i < Size; i++) {
				Components[i] = vec[i];
			}
			Column = column;
		}

		/// <summary>
		/// Destructor with dynamic data deallocating
		/// </summary>
		~Vector() {
			delete[] Components;
			Components = 0;
		}

		/// <summary>
		/// Dynamic data allocating with given size and condition for column/row vector
		/// </summary>
		void Initialize(int size, bool column) {
			if (Size > 0) {
				delete[] Components;
			}
			Size = size;
			Components = new float[Size] {};
			Column = column;
		}

		/// <summary>
		/// Perform a vectorial sum of two given vector objects,
		/// size must be the same
		/// </summary>
		static void Sum(float* v1, float* v2, int size, float* sum) {
			for (int i = 0; i < size; i++)
				sum[i] = v1[i] + v2[i];
		}

		/// <summary>
		/// Perform a vectorial rest of two given vector objects,
		/// size must be the same
		/// </summary>
		static void Rest(float* v1, float* v2, int size, float* rest) {
			for (int i = 0; i < size; i++)
				rest[i] = v1[i] - v2[i];
		}

		bool Sum(Vector* v1, Vector* v2) {
			if (v1->Size == v2->Size) {

				Initialize(v1->Size, v1->Column);
				for (int i = 0; i < Size; i++)
					Components[i] = v1->Components[i] + v2->Components[i];
				return true;
			}
			else
				return false;
		}

		bool Rest(Vector* v1, Vector* v2) {
			if (v1->Size == v2->Size) {

				Initialize(v1->Size, v1->Column);
				for (int i = 0; i < Size; i++)
					Components[i] = v1->Components[i] - v2->Components[i];
				return true;
			}
			else
				return false;
		}

		/// <summary>
		/// Perform a elementwise multiplicacion
		/// </summary>
		/// <returns>Operation executed</returns>
		bool Mult(Vector* v1, Vector* v2) {
			if (v1->Size == v2->Size) {

				Initialize(v1->Size, vec_col);
				for (int i = 0; i < Size; i++)
					Components[i] = v1->Components[i] * v2->Components[i];
				return true;
			}
			else
				return false;
		}

		/// <summary>
		/// Perform a matrix and vector multiplication
		/// </summary>
		/// <returns>Operation executed</returns>
		bool Mult(Matrix* m, Vector* v) {
			if (m->Cols == v->Size) {

				Initialize(v->Size, vec_col);
				for (int i = 0; i < m->Rows; i++)
				{
					for (int j = 0; j < m->Cols; j++)
					{
						Components[i] += m->Data(i, j) * v->Components[j];
					}
				}
				return true;
			}
			else
				return false;
		}

		/// <summary>
		/// Perform a vector and matrix multiplication
		/// </summary>
		/// <returns>Operation executed</returns>
		bool Mult(Vector* v, Matrix* m) {
			if (v->Size == m->Rows) {

				Initialize(v->Size, vec_row);
				for (int i = 0; i < m->Cols; i++)
				{
					for (int j = 0; j < m->Rows; j++)
					{
						Components[i] += v->Components[j] * m->Data(i, j);
					}
				}
				return true;
			}
			else
				return false;
		}

		/// <summary>
		/// Perform a scalar and vector multiplication
		/// </summary>
		bool Mult(float a, Vector* v) {
			if (v->Size > 0) {
				Initialize(v->Size, v->Column);
				for (int i = 0; i < v->Size; i++) {
					Components[i] = a * v->Components[i];
				}
				return true;
			}
			else
				return false;
		}

		static float dot(Vector* v1, Vector* v2) {
			float dot=0;
			if (v1->Size == v2->Size) {
				for (int i = 0; i < v1->Size; i++)
					dot += v1->Components[i] * v2->Components[i];
				return dot;
			}
			else
			{
				return 0;
				//Error Handling: Vector size error
			}
		}

		bool cross(Vector* v1, Vector* v2) {
			if (v1->Size == v2->Size) {
				Initialize(v1->Size, vec_col);
				Components[0] = v1->Components[1] * v2->Components[2] - v1->Components[2] * v2->Components[1];
				Components[1] = v1->Components[2] * v2->Components[0] - v1->Components[0] * v2->Components[2];
				Components[2] = v1->Components[0] * v2->Components[1] - v1->Components[1] * v2->Components[0];
				return true;
			}
			else
				return false;
		}

		/// <summary>
		/// Get a new vector normalized from given vector
		/// </summary>
		static void Normalize(float* vec, int size, float* vecnormalized) {
			float vecnorm = Norm(vec, size);
			for (int i = 0; i < size; i++)
				vecnormalized[i] = vec[i] / vecnorm;
		}

		/// <summary>
		/// Get the norm magnitude of a given vector
		/// </summary>
		static float Norm(float* vec, int size) {
			float n = 0;
			for (int i = 0;i < size;i++)
				n += vec[i] * vec[i];
			n = sqrtf(n);
			return n;
		}

		/// <summary>
		/// Get a new vector normalized from given vector
		/// </summary>
		void Normalize(Vector* vec) {
			Initialize(vec->Size, vec->Column);
			float vecnorm = vec->Norm();
			for (int i = 0; i < vec->Size; i++)
				Components[i] = vec->Components[i] / vecnorm;
		}

		/// <summary>
		/// Get the current vector normalized
		/// </summary>
		void Normalize() {
			float vecnorm = Norm();
			for (int i = 0; i < Size; i++)
				Components[i] = Components[i] / vecnorm;
		}

		/// <summary>
		/// Get the norm magnitude of current vector
		/// </summary>
		float Norm() {
			float n = 0;
			for (int i = 0;i < Size;i++)
				n += Components[i] * Components[i];
			n = sqrtf(n);
			return n;
		}

		/// <summary>
		/// Transpose vector
		/// </summary>
		void Transpose() {
			Column = !Column;
		}

		void Print() {
			if (!Column) {
				for (int i = 0;i < Size;i++) {
					cout << Components[i] << " ";
				}
				cout << endl;
			}
			else {
				for (int i = 0;i < Size;i++) {
					cout << Components[i] << endl;
				}
			}
		}*/
		//};

#endif

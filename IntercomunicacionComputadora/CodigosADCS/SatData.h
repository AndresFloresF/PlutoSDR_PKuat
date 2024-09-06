/**
 ******************************************************************************
 * @file    ADCSMath.h
 * @author  Eduardo Munoz
 * @brief   Header file of SPG4 propagator
 ******************************************************************************
   */
#ifndef SATDATA_H
#define SATDATA_H	
#include <math.h>
#include "CONST.h"

typedef struct {
	float x_pole;
	float y_pole;
	double UT1_UTC;
	double LOD;
	float dpsi;
	float deps;
	float dx_pole;
	float dy_pole;
	float TAI_UTC;
}IERSDataTypeDef;

typedef struct {
	double T;
	double JD_UT1;
	float LOD;
	float x_pole;
	float y_pole;
	float dpsi;
	float deps;
}EOPDataTypeDef;

/// <summary>
/// Convert from (year,dayofyear) to (month,day,hour,minute,sec) 
/// </summary>
/// <param name="year"></param>
/// <param name="days"></param>
/// <param name="date[5]"></param>
void days2mdh(int year, double days, double date[]) {
	// --------------- set up array of days in month  --------------
	double dayofyr, mon, day, hr, minute, sec, inttemp;
	double temp, lmonth[12]{};
	for (int i = 1; i <= 12; i++) {
		lmonth[i - 1] = 31;
		if (i == 2)
			lmonth[i - 1] = 28;
		switch (i) {
		case 4:
		case 6:
		case 9:
		case 11:
			lmonth[i - 1] = 30;
			break;
		}
	}

	dayofyr = floor(days);

	// ----------------- find month and day of month ---------------
	if (((year - 1900) % 4) == 0)
		lmonth[1] = 29;

	int i = 1;
	inttemp = 0;
	while ((dayofyr > (inttemp + lmonth[i - 1])) && (i < 12)) {
		inttemp += lmonth[i - 1];
		i++;
	}

	mon = i;
	day = dayofyr - inttemp;

	// ----------------- find hours minutes and seconds ------------
	temp = (days - dayofyr) * 24.0;
	hr = (int)temp;
	temp = (temp - hr) * 60.0;
	minute = (int)temp;
	sec = (temp - minute) * 60.0;

	date[0] = mon;
	date[1] = day;
	date[2] = hr;
	date[3] = minute;
	date[4] = sec;
}

/// <summary>
/// Return modified julian date
/// </summary>
/// <param name="year"></param>
/// <param name="date(month,day,hr,min,sec)"></param>
/// <returns></returns>
double Mjday(int year, double date[]) {
	/*if (nargin < 4) //numero de argumentos
		hour = 0;
		min  = 0;
		sec  = 0;
	end*/
	double Mjd;
	int a = 0;
	int y = year;
	int m = date[0];
	int b = 0;
	float c = 0;

	if (m <= 2) {
		y -= 1;
		m += 12;
	}

	if (y < 0)
		c = -0.75;

	// check for valid calendar date
	if (year > 1582) {
		a = (int)(y / 100);
		b = 2 - a + floor((float)(a / 4));
	}
	else if (date[0] < 10) {

	}
	else if (date[0] > 10) {
		a = (int)(y / 100);
		b = 2 - a + floor((float)(a / 4));
	}
	else if (date[1] <= 4) {

	}
	else if (date[1] > 14) {
		a = (int)(y / 100);
		b = 2 - a + floor((float)(a / 4));
	}
	//else
		//Hadle errorcout << "\n\n  this is an invalid calendar date!!\n";

	Mjd = ((int)(365.25 * y + c)) + ((int)(30.6001 * (m + 1)));
	Mjd = Mjd + date[1] + b + 1720994.5;
	//Mjd = Mjd + (date[2] + date[3]/60 + date[4] / 3600) / 24;
	Mjd = Mjd + (date[2] + date[3] * 0.0166666666666666 + date[4] * 0.000277777777777777) * 0.0416666666666666;
	Mjd = Mjd - 2400000.5;
	return Mjd;
}

class SatData {
	public: //Atributos (properties)
		double MJD_epoch, eo, xno;
		float xmo, xnodeo, omegao, xincl, bstar;
		//int norad_number, bulletin_number, revolution_number, ephemeris_type;
		//string classification;
		
	public: //Métodos
		SatData();
		SatData (float, int, int, /*string, int, int, */float, float, float, float, float, float, /*float, float, */float);
		void getMJD_epoch(int, double);
};

SatData::SatData() {
	MJD_epoch = 0;
	xmo = 0;
	xnodeo = 0;
	omegao = 0;
	xincl = 0;
	eo = 0;
	xno = 0;
	bstar = 0;
}

SatData::SatData (float ep, int Cnum, int ID, /*string SC, int rNo, int Etype, */float M, float raan, float omega, float i, float e, float no, /*float TD1, float TD2, */float BStar) {
	//MJD_epoch = days2mdh(year);
	//norad_number = Cnum;
	//bulletin_number = ID;
	//classification = SC;
	//revolution_number = rNo;
	//ephemeris_type = Etype;
	xmo = M * Rad;
	xnodeo = raan * Rad;
	omegao = omega * Rad;
	xincl = i * Rad;
	eo = e;
	xno = no * TWOPI / MINUTES_PER_DAY;
	//xndt2o = TD1 * exp(-8) * TWOPI / MINUTES_PER_DAY_SQUARED;
	//xndd6o = TD2 * TWOPI / MINUTES_PER_DAY_CUBED;
	bstar = BStar;
}

void SatData::getMJD_epoch(int year, double doy) {
	double date[5]{};
	days2mdh(year, doy, date);
	MJD_epoch = Mjday(year, date);
}

#endif

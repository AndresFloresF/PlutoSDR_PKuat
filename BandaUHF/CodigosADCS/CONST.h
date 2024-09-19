#include <cmath>
#include <math.h>

//
#define ge 398600.8   // Earth gravitational constant
#define MINUTES_PER_DAY  1440
#define MINUTES_PER_DAY_SQUARED pow(MINUTES_PER_DAY, 2)
#define MINUTES_PER_DAY_CUBED pow(MINUTES_PER_DAY, 3)

// Mathematical constants
#define M_PI        3.14159265358979323846  /* pi */
#define TWOPI  		2*M_PI
#define pi2   		2*M_PI               			// 2M_PI
#define Rad   		M_PI/180             			// Radians per degree
#define Deg   		180/M_PI              			// Degrees per radian
//#define Arcs  		3600*180/M_PI         			// Arcseconds per radian
#define Arcs  		0.00000484813         			// radian per arcseconds

// General
//#define MJD_OFFSET  2400000.5                       //Modified Julian Date Offset
#define MJD_J2000 	51544.5             			// Modified Julian Date of J2000
#define T_B1950   	-0.500002108        			// Epoch B1950
#define c_light   	299792458.000000000 			// Speed of light  [m/s]; DE430
#define AU        	149597870700.000000 			// Astronomical unit [m]; DE430

// Physical parameters of the Earth, Sun and Moon

// Equatorial radius and flattening
#define R_Earth   	exp(6378.137, 3)          		// Earth's radius [m]; WGS-84
#define f_Earth   	1/298.257223563     			// Flattening; WGS-84   
#define R_Sun     	exp(696000, 3)		            // Sun's radius [m]; DE430
#define R_Moon    	exp(1738, 3)              		// Moon's radius [m]; DE430

// Earth rotation (derivative of GMST at J2000; differs from inertial period by precession)
#define omega_Earth 15.04106717866910/3600*Rad; 	// [rad/s]; WGS-84

// Gravitational coefficients
#define GM_Earth    exp(398600.4418, 9)    	   		// [m^3/s^2]; WGS-84
#define GM_Sun     	exp(132712440041.939400, 9)     // [m^3/s^2]; DE430
              
// Solar radiation pressure at 1 AU 
#define P_Sol 		1367/c_light 					// [N/m^2] (1367 W/m^2); IERS 96

//Variables para conversion LLA
#define LLA_a          6378137
#define LLA_b          6356752.31424518
#define LLA_e          0.08181919084
#define LLA_ep         0.08209443794
//Variables para sgp4
#define Earth_radius   6371.2 //Reference radius used in IGRF
#define xke            0.07436691613
#define qoms2t         1.88027915e-9

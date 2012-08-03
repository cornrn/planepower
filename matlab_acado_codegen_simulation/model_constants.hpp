#ifndef MODEL_CONSTANTS_H
#define MODEL_CONSTANTS_H

// Plane + Carousel model constants used by MHE and NMPC

//  PARAMETERS OF THE KITE :
//  -----------------------------
double mk =  0.626;      //  mass of the kite               //  [ kg    ]
//  A =  0.2;      //  effective area                 //  [ m^2   ]

//   PHYSICAL CONSTANTS :
//  -----------------------------
double   g =    9.81;      //  gravitational constant         //  [ m /s^2]
double rho =    1.23;      //  density of the air             //  [ kg/m^3]

//  PARAMETERS OF THE CABLE :
//  -----------------------------
double rhoc = 1450.00;      //  density of the cable           //  [ kg/m^3]
double dc = 1e-3;      //  diameter                       //  [ m     ]

// Cross-sectional area of the cable [m^2]
double AQ      =  PI*dc*dc/4.0; 

//CAROUSEL ARM LENGTH [m]
double rA = 1.085; 

double zT = -0.02; // Bridle length [m]
// 		double XT = [0;0;-0.01];

// 		double ZT = 0;
//             YT = 0.005;

//INERTIA MATRIX (Kurt's direct measurements)
double I1 = 0.0163;
double I31 = 0.0006;
double I2 = 0.0078;
double I3 = 0.0229;

//IMU POSITION & ANGLE
// 		double XIMU = [0.0246;-0.0116;-0.0315];
// 		double alphaIMU = 0*pi/180;//4
// 		double betaIMU = 0*pi/180;
// 		double deltaIMU = 0*pi/180;

// 		double alpha0 = -0*PI/180; 

//TAIL LENGTH
double LT = 0.4;


//ROLL DAMPING
double RD = 1e-2; 
double PD = 0*5e-3;
double YD = 0*5e-3;
//WIND-TUNNEL PARAMETERS

//Lift (report p. 67)
double CLA = 5.064;

double CLe = -1.924;//e-5;//0.318;//

double CL0 = 0.239;

//Drag (report p. 70)
double CDA = -0.195;
double CDA2 = 4.268;
double CDB2 = 5;//0;//
// 		double CDe = 0.044;
// 		double CDr = 0.111;
double CD0 = 0.026;

//Roll (report p. 72)
double CRB = -0.062;
double CRAB = -0.271; 
double CRr = -5.637e-1;//e-6;//-0.244;//

//Pitch (report p. 74)
double CPA = 0.293;
double CPe = -4.9766e-1;//e-6;//-0.821;//

double CP0 = 0.03;

//Yaw (report p. 76)
double CYB = 0.05;
double CYAB = 0.229;

double SPAN = 0.96;
double CHORD = 0.1;

#endif
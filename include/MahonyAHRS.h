//=====================================================================================================
// MahonyAHRS.h
//=====================================================================================================
//
// Madgwick's implementation of Mayhony's AHRS algorithm.
// See: http://www.x-io.co.uk/node/8#open_source_ahrs_and_imu_algorithms
//
// Date			Author			Notes
// 29/09/2011	SOH Madgwick    Initial release
// 02/10/2011	SOH Madgwick	Optimised for reduced CPU load
//
//=====================================================================================================
#ifndef MahonyAHRS_h
#define MahonyAHRS_h

#define DEG_TO_RAD 0.01745329251994329576923690768489f
#define RAD_TO_DEG 57.295779513082320876798154814105f

//----------------------------------------------------------------------------------------------------
// Variable declaration

extern volatile float twoKp; // 2 * proportional gain (Kp)
extern volatile float twoKi; // 2 * integral gain (Ki)
// volatile float q0, q1, q2, q3;	// quaternion of sensor frame relative to
// auxiliary frame

//---------------------------------------------------------------------------------------------------
// Function declarations

void MahonyAHRSupdate(
  float gx, float gy, float gz, float ax, float ay, float az, float mx,
  float my, float mz);
void MahonyAHRSupdateIMU(
  float gx, float gy, float gz, float ax, float ay, float az, float * pitch,
  float * roll, float * yaw);
float invSqrt(float x);
#endif
//=====================================================================================================
// End of file
//=====================================================================================================

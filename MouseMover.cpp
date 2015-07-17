#define _USE_MATH_DEFINES

#include "MouseMover.hpp"

#include "Utilities.hpp"

#include <cmath>

namespace {

// For Gain function.
// This -may- need to be adjusted based on some system-provided value to handle differences in
// resolutions.. but probably not. Leaving this in just in case we need to play with it.
// As a fixed value it acts as a constant scaling factor on the gain function.
const float pixelDensity = 0.83f;

// Framerate, though leaving it as-is is likely fine. Ultimately this is just
// another number that can be adjusted to achieve desired effect.
const float frameRate = 60.0f;

// These are the constants that characterize the shape of the gain function.
// vMax, vMin, CDMax and CDMin have been carefully tuned and should likely be left as they are.
// TODO: add reference to the paper that explains the gain function.

// Input range values
const float vMax = float(M_PI);                        // 180 degrees in radians
const float vMin = 0.174532925f;                       // 10 degrees in radians

// Output range values
// Sizes in mm of smallest and largest clickable targets,
// divided by how much movement is desired for those targets
// (for some idealized screen characterized by the pixel density above).
const float CDMax = 4580.0f / (float(M_PI) / 6.0f);    //45.8 cm / 30 deg in radians
const float CDMin = 16.0f / 0.274532925f;              // 16 mm / 15.7 deg in radians

// For sensitivity adjustments: Use inflectionRatio and lambda.
// For lambda: roughly between 4.0 and 5.0.
// A value of 0 will result in constant gain, which may be suitable for gaming.
// For inflection ratio: roughly between 0.4 and 0.7.

// Location of inflection point between vmin and vmax
// "Sensitivity", roughly, with a lower inflection ratio yielding more sensitivity.
const float inflectionRatioMin = 0.4f;
const float inflectionRatioMax = 0.7f;

// Slope of curve at inflection point.
// Simple proportionality constant.
// "Acceleration", roughly.
const float lambdaMin = 4.0f / (vMax - vMin);
const float lambdaMax = 5.0f / (vMax - vMin);

// Maps angular velocity to an acceleration curve
float getGain(float deviceSpeed, float sensitivity, float acceleration)
{
    // Constant chunk of parameter to exp function below.
    const float inflectionVelocity = sensitivity * (vMax - vMin) + vMin;

    //gain, in mm/rad
    float CDGain = CDMin + (CDMax - CDMin) /
        ( 1 + std::exp(-acceleration * (deviceSpeed - inflectionVelocity)));

    //in px/rad, pixel density scaling factor (px/mm)
    return CDGain * pixelDensity;
}

float deg2rad(float deg)
{
    return deg * float(M_PI) / 180.0f;
}

} // namespace

const float MouseMover::defaultAcceleration = 0.3f;
const float MouseMover::defaultSensitivity = 0.5f;

MouseMover::MouseMover()
: _dx(0.0f)
, _dy(0.0f)
, _dxFractional(0.0f)
, _dyFractional(0.0f)
, _XTowardsWrist(false)
, _acceleration(lambdaMin, lambdaMax, defaultAcceleration)
, _sensitivity(inflectionRatioMax, inflectionRatioMin, defaultSensitivity)
{

}

void MouseMover::onOrientation(const myo::Quaternion<float>& quat)
{
    _quat = quat;
}

void MouseMover::onGyroscope(const myo::Vector3<float> &gyro)
{
    // Accel vector in world space
    auto gyroRad = myo::Vector3<float>(deg2rad(gyro.x()), deg2rad(gyro.y()), deg2rad(gyro.z()) );

    // Gyro vector in world space
    auto gyroRadWorld = myo::rotate(_quat, gyroRad);

    // Forward vector
    auto forwardSource = _XTowardsWrist ?
        myo::Vector3<float>(1.0f, 0.0f, 0.0f) :
        myo::Vector3<float>(-1.0f, 0.0f, 0.0f);
    auto forward = myo::rotate(_quat, forwardSource);

    // Right vector
    auto right = forward.cross(myo::Vector3<float>(0.0f, 0.0f, -1.0f));

    // Get quat that rotates Myo's right vector
    auto up = myo::Vector3<float>(0.0f, 1.0f, 0.0f);
    auto yCompensationQuat = myo::rotate<float>(right, up).normalized();

    // Rotate accel vector through y-compensation quat
    auto gyroVectorCompensated = myo::rotate<float>(yCompensationQuat, gyroRadWorld);

    // Get x and y components of accel vector
    float dx = -gyroRadWorld.z();
    float dy = gyroVectorCompensated.y();

    // Feed components through the gain function and store.
    updateMouseDeltas(dx, dy);
}

void MouseMover::updateMouseDeltas(float dx, float dy)
{
    // Get inverse of fps (time)
    float frameDuration = 1.0f / frameRate; // e.g. 60

    // Norm of dx + dy
    float norm = std::sqrt(dx * dx + dy * dy);

    // Get calculated gain based on norm
    float gain = getGain(norm, _sensitivity.output(), _acceleration.output());

    // Multiply input by gain, returns px (rad/s * px/rad * s)
    _dx = dx * gain * frameDuration;
    _dy = dy * gain * frameDuration;

    // Update fractional accumulators
    _dxFractional += utils::extractFractional<float>(_dx);
    _dyFractional += utils::extractFractional<float>(_dy);

    // Normalize deltas
    _dx = std::trunc(_dx);
    _dy = std::trunc(_dy);

    // Add back in any fractional values that > |1|, adjust fractional accumulators accordingly
    if (std::abs(_dxFractional) > 1.0f) {
        _dx += std::trunc(_dxFractional);
        _dxFractional = utils::extractFractional<float>(_dxFractional);
    }
    if (std::abs(_dyFractional) > 1.0f) {
        _dy += std::trunc(_dyFractional);
        _dyFractional = utils::extractFractional<float>(_dyFractional);
    }
}

void MouseMover::setAcceleration(float value)
{
    _acceleration.setInput(value);
}

float MouseMover::acceleration() const
{
    return _acceleration.input();
}

void MouseMover::setSensitivity(float value)
{
    _sensitivity.setInput(value);
}

float MouseMover::sensitivity() const
{
    return _sensitivity.input();
}


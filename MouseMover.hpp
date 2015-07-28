#pragma once

#include <myo/myo.hpp>

#include "LinearParameter.hpp"

/// Provides a black-box implementation of the mouse movement algorithm.
/// Algorithm takes orientation and gyroscope data as input and computes screen deltas
/// based on that.
class MouseMover {
public:
    MouseMover();

    /// Process orientation information from the Myo being used to control the mouse.
    void onOrientation(const myo::Quaternion<float>& quat);

    /// Process gyroscope information from the Myo being used to control the mouse.
    /// Updates the internal deltas _dx and _dy.
    void onGyroscope(const myo::Vector3<float> &gyro);

    /// Returns the current computed screen deltas.
    float dx() const { return _dx; }
    float dy() const { return _dy; }

    /// Get and set the sensitivity adjustable parameters for the movement algorithm.
    /// Each value is between within [0.0, 1.0], initialized to current best defaults.
    /// Values are mapped internally to within reasonable ranges.
    void setAcceleration(float value);
    float acceleration() const;
    void setSensitivity(float value);
    float sensitivity() const;

    /// Default values for acceleration and sensitivity.
    static const float defaultAcceleration;
    static const float defaultSensitivity;

    /// Set the arm and orientation information so we can compensate for how the armband is put on.
    void setXTowardsWrist(bool XTowardsWrist) { _XTowardsWrist = XTowardsWrist; }

private:
    /// The screen deltas to be exposed. These should be integral.
    float _dx, _dy;

    /// The fractional components of the current deltas.
    float _dxFractional, _dyFractional;

    /// The most recent orientation input.
    myo::Quaternion<float> _quat;

    /// Whether the armband's positive X axis is facing the wearer's elbow or wrist;
    bool _XTowardsWrist;

    /// Sensitivity parameters.
    LinearParameter _acceleration;
    LinearParameter _sensitivity;

    /// Process the delta values computed from the orientation and gyroscope input,
    /// running them through the gain function, some additional filters, and the
    /// accumulator process to produce the final deltas
    void updateMouseDeltas(float dx, float dy);
};

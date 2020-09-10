// Copyright 2015 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "Utilities.hpp"

/// Encapsulates a pair of values and interpolates linearly between them.
/// valueA and valueB represent the values being interpolated between, while
/// the input is a value within [0.0, 1.0] that represents the relative proportion
/// of each. (0.0 means valueA, 1.0 means valueB).
/// Call output() to get the interpolated value.
class LinearParameter {
public:
    /// valueA and valueB can be any valid value, while initialInput represents the
    /// initial proportion between them, and thus must be within [0.0, 1.0].
    LinearParameter(float valueA, float valueB, float initialInput)
    : _valueA(valueA)
    , _valueB(valueB)
    , _input(initialInput)
    {
    }

    /// Set the value within [0.0, 1.0] with which to linearly interpolate the result.
    void setInput(float input) {
        _input = utils::clamp<float>(input, 0.0f, 1.0f);
    }

    /// Get the current parameters.
    float input() const { return _input; }
    float valueA() const { return _valueA; }
    float valueB() const { return _valueB; }

    /// Return the linearly interpolated value between valueA and valueB represented by
    /// the proportion _input.
    float output() const {
        return (1.0f - _input) * _valueA + _input * _valueB;
    }

private:
    const float _valueA;
    const float _valueB;
    float _input;
};

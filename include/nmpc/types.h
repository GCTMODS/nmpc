/*
Copyright (C) 2013 Daniel Dyer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef TYPES_H_
#define TYPES_H_

#include "config.h"

#ifdef NMPC_SINGLE_PRECISION
typedef float real_t;
#endif

#ifdef NMPC_DOUBLE_PRECISION
typedef double real_t;
#endif

#define G_ACCEL ((real_t)9.80665)
#define RHO ((real_t)1.225)

#include <Eigen/Core>
#include <Eigen/Geometry>

typedef Eigen::Matrix<real_t, 1, 1> Vector1r;
typedef Eigen::Matrix<real_t, 2, 1> Vector2r;
typedef Eigen::Matrix<real_t, 3, 1> Vector3r;
typedef Eigen::Matrix<real_t, 4, 1> Vector4r;
typedef Eigen::Matrix<real_t, 5, 1> Vector5r;
typedef Eigen::Quaternion<real_t> Quaternionr;
typedef Eigen::Matrix<real_t, 3, 3> Matrix3x3r;
typedef Eigen::Matrix<real_t, Eigen::Dynamic, Eigen::Dynamic> MatrixXr;
typedef Eigen::Matrix<real_t, Eigen::Dynamic, 1> VectorXr;

typedef Eigen::Matrix<real_t, NMPC_STATE_DIM, 1> StateVector;
typedef Eigen::Matrix<real_t, NMPC_STATE_DIM, 1> StateVectorDerivative;

/*
Typedef for control vector. It is dynamic in order to allow different dynamics
models to accept different numbers of control inputs.
*/
typedef Eigen::Matrix<
    real_t,
    Eigen::Dynamic,
    1,
    0,
    NMPC_CONTROL_DIM> ControlVector;

typedef Eigen::Matrix<real_t, 6, 1> AccelerationVector;

#endif

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

#include <cmath>
#include <stdint.h>
#include <acado_toolkit.hpp>
#include <include/acado_gnuplot/gnuplot_window.hpp>

#include "state.h"
#include "dynamics.h"

#include <iostream>

#define SIM_TIMESTEP	0.02
#define SIM_LENGTH		10
#define HORIZON_LENGTH	10

void dynamics(double *x, double *f, void *user_data) {
	int i;

	/* Create new state vector using input array. */
	State s;
	s.position() << x[0], x[1], x[2];
	s.velocity() << x[3], x[4], x[5];
	s.acceleration() << 0, 0, 0;
	s.attitude() << x[6], x[7], x[8], x[9];
	Quaternionr q(s.attitude());
	q.normalize();
	s.attitude() << q.vec(), q.w();
	s.angular_velocity() << x[10], x[11], x[12];
	s.angular_acceleration() << 0, 0, 0;
	s.wind_velocity() << 0, 0, 0;
	s.gyro_bias() << 0, 0, 0;
	s.acceleration() << 0, 0, 0;

	ControlVector c(4);
	c << x[13], x[14], x[15], 0;

	/* Evaluate the dynamics model. */
	FixedWingFlightDynamicsModel *d =
		static_cast<FixedWingFlightDynamicsModel *>(user_data);
	AccelerationVector a = d->evaluate(s, c);
/*
	std::cout << a << std::endl << std::endl;
	std::cout << s << std::endl;
	std::cout << "--------------" << std::endl << std::endl;
*/
	/* Evaluate the process model using state and dynamics outputs. */
	s.acceleration() << a[0], a[1], a[2];
	s.angular_acceleration() << a[3], a[4], a[5];

	StateVectorDerivative dot = s.model();

	/* Copy results to output vector. */
	for(i=0; i<3; ++i) f[i] = dot[i];
	for(i=0; i<3; ++i) f[i+3] = dot[i+3];
	for(i=0; i<4; ++i) f[i+6] = dot[i+9];
	for(i=0; i<3; ++i) f[i+10] = dot[i+13];
}

int main()
{
    USING_NAMESPACE_ACADO

	int i;

	/* State vector. */
	DifferentialState position(3);
	DifferentialState velocity(3);
	DifferentialState attitude(4);
	DifferentialState angular_velocity(3);

	/* Control vector. */
	Control motor;
	Control elevon(2);

	/* Dynamics model. */
	FixedWingFlightDynamicsModel dynamics_model;

	/* Dynamics model parameters. */
	dynamics_model.set_mass(3.8);
	dynamics_model.set_inertia_tensor((Matrix3x3r() <<
		2.59e-1, 0, -0.334e-1,
		0, 1.47e-1, 0,
		-0.334e-1, 0, 4.05e-1).finished());
	dynamics_model.set_prop_coeffs(0.0246, 0.00300);
	dynamics_model.set_lift_coeffs((Vector5r() <<
		-0.208, -0.587, -0.004, 4.119, 0.143).finished());
	dynamics_model.set_drag_coeffs((Vector5r() <<
		73.641, -5.365, -1.614, 0.153, 0.0176).finished());
	dynamics_model.set_side_coeffs((Vector8r() <<
		0.0, 0.0, 0.0, 0.0,
		0.220, 0.01, 0.0, 0.0).finished(),
		(ControlVector(4) << 0.0, 0.0, 0.0, 0.0).finished());
	dynamics_model.set_pitch_moment_coeffs(
		(Vector2r() << -0.001, -0.014).finished(),
		(ControlVector(4) << 0.0, -0.03, -0.03, 0.0).finished());
	dynamics_model.set_roll_moment_coeffs(
		(Vector1r() << -0.002).finished(),
		(ControlVector(4) << 0.0, -0.03, 0.03, 0.0).finished());
	dynamics_model.set_yaw_moment_coeffs(
		(Vector2r() << 0.0, -0.005).finished(),
		(ControlVector(4) << 0.0, 0.0, 0.0, 0.0).finished());
	dynamics_model.set_motor_index(0);

	/* Combine the above vectors into one to pass to the dynamics function. */
	IntermediateState is(16);

	for(i=0; i<3; ++i) is(i) = position(i);
	for(i=0; i<3; ++i) is(i+3) = velocity(i);
	for(i=0; i<4; ++i) is(i+6) = attitude(i);
	for(i=0; i<3; ++i) is(i+10) = angular_velocity(i);
	is(13) = motor;
	for(i=0; i<2; ++i) is(i+14) = elevon(i);

	/* Define the differential equation. */
	CFunction flight_model(13, dynamics);
	flight_model.setUserData(&dynamics_model);
	DifferentialEquation f;
	f << flight_model(is);

	/* Set up the simulation process. */
	OutputFcn identity;
	DynamicSystem dynamicSystem(f, identity);
	Process process(dynamicSystem, INT_RK78);

	/* Define the optimal control problem. */
	Function h;

	h << position;
	h << velocity;
	h << attitude;
	h << angular_velocity;

	/* Least-squares weighting matrix. */
	Matrix Q = zeros(13, 13);
	Q(0, 0) = 0.0;
	Q(1, 1) = 0.0;
	Q(2, 2) = 1.0;
	Q(3, 3) = 1.0;
	Q(4, 4) = 1.0;
	Q(5, 5) = 1.0;
	Q(6, 6) = 5.0;
	Q(7, 7) = 5.0;
	Q(8, 8) = 5.0;
	Q(9, 9) = 5.0;
	Q(10, 10) = 0.0;
	Q(11, 11) = 0.0;
	Q(12, 12) = 0.0;

	Vector r(13);
	r.setAll(0.0);

	OCP ocp(0.0, HORIZON_LENGTH, 50);

	ocp.minimizeLSQ(Q, h, r);

	ocp.subjectTo(f);

	/* Flight envelope constraints. */
	ocp.subjectTo(-M_PI <= angular_velocity <= M_PI);

	/* Control constraints. */
	ocp.subjectTo(0.0 <= motor <= 19000.0);
	ocp.subjectTo(-0.8 <= elevon <= 0.8);

	/* Set up the realtime algorithm. */
	RealTimeAlgorithm alg(ocp, SIM_TIMESTEP);
	alg.set(INTEGRATOR_TYPE, INT_RK78);

	/* Set up controller and load reference trajectory. */
	VariablesGrid ref_data("data/reference.txt");
	StaticReferenceTrajectory reference(ref_data);
	Controller controller(alg, reference);

	/* Initialise and run the simulation. */
	SimulationEnvironment sim(0.0, SIM_LENGTH, process, controller);

	Vector x0(13);
	x0 = ref_data.getFirstVector();
	sim.init(x0);
	sim.run();

	/* Plot the results. */
	VariablesGrid diffStates;
	sim.getProcessDifferentialStates(diffStates);

	VariablesGrid feedbackControl;
	sim.getFeedbackControl(feedbackControl);

	OutputFcn plot_fn;
	plot_fn << position(2);
	plot_fn << velocity;
	plot_fn << angular_velocity;
	VariablesGrid plot_out;
	plot_fn.evaluate(&diffStates, NULL, NULL, NULL, NULL, &plot_out);

	GnuplotWindow window;
	window.addSubplot(plot_out(0), "Altitude [m]");
	window.addSubplot(plot_out(1), "Velocity X [m/s]");
	window.addSubplot(plot_out(2), "Velocity Y [m/s]");
	window.addSubplot(plot_out(3), "Velocity Z [m/s]");
	window.addSubplot(plot_out(4), "Angular Rate P [rad/s]");
	window.addSubplot(plot_out(5), "Angular Rate Q [rad/s]");
	window.addSubplot(plot_out(6), "Angular Rate R [rad/s]");
	window.addSubplot(feedbackControl(0), "Throttle");
	window.addSubplot(feedbackControl(1), "Elevon 1 Deflection [rad]");
	window.addSubplot(feedbackControl(2), "Elevon 2 Deflection [rad]");
	window.plot();

	OutputFcn xplane_fn;
	xplane_fn << position;
	xplane_fn << attitude;
	VariablesGrid xplane_out;
	xplane_fn.evaluate(&diffStates, NULL, NULL, NULL, NULL, &xplane_out);
	Grid xplane_grid(0.0, SIM_LENGTH-SIM_TIMESTEP, SIM_LENGTH / SIM_TIMESTEP);
	xplane_out.coarsenGrid(xplane_grid);
	xplane_out.print("", "", "", DEFAULT_WIDTH, DEFAULT_PRECISION, "\t", "\n");

    return 0;
}




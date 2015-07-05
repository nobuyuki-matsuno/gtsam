/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * testTriangulationFactor.cpp
 *
 *  Created on: July 30th, 2013
 *      Author: cbeall3
 */

#include <gtsam/geometry/triangulation.h>
#include <gtsam/geometry/SimpleCamera.h>
#include <gtsam/geometry/Cal3Bundler.h>
#include <gtsam/base/numericalDerivative.h>
#include <CppUnitLite/TestHarness.h>

#include <boost/assign.hpp>
#include <boost/assign/std/vector.hpp>

using namespace std;
using namespace gtsam;
using namespace boost::assign;

// Some common constants
static const boost::shared_ptr<Cal3_S2> sharedCal = //
    boost::make_shared<Cal3_S2>(1500, 1200, 0, 640, 480);

// Looking along X-axis, 1 meter above ground plane (x-y)
static const Rot3 upright = Rot3::ypr(-M_PI / 2, 0., -M_PI / 2);
static const Pose3 pose1 = Pose3(upright, gtsam::Point3(0, 0, 1));
SimpleCamera camera1(pose1, *sharedCal);

// landmark ~5 meters infront of camera
static const Point3 landmark(5, 0.5, 1.2);

// 1. Project two landmarks into two cameras and triangulate
Point2 z1 = camera1.project(landmark);

//******************************************************************************
TEST( triangulation, TriangulationFactor ) {
  // Create the factor with a measurement that is 3 pixels off in x
  Key pointKey(1);
  SharedNoiseModel model;
  typedef TriangulationFactor<SimpleCamera> Factor;
  Factor factor(camera1, z1, model, pointKey);

  // Use the factor to calculate the Jacobians
  Matrix HActual;
  factor.evaluateError(landmark, HActual);

  Matrix HExpected = numericalDerivative11<Vector,Point3>(
      boost::bind(&Factor::evaluateError, &factor, _1, boost::none), landmark);

  // Verify the Jacobians are correct
  CHECK(assert_equal(HExpected, HActual, 1e-3));
}

//******************************************************************************
int main() {
  TestResult tr;
  return TestRegistry::runAllTests(tr);
}
//******************************************************************************
/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 *  @file  PreintegrationBase.h
 *  @author Luca Carlone
 *  @author Stephen Williams
 *  @author Richard Roberts
 *  @author Vadim Indelman
 *  @author David Jensen
 *  @author Frank Dellaert
 **/

#pragma once

#include <gtsam/navigation/PreintegrationParams.h>
#include <gtsam/navigation/NavState.h>
#include <gtsam/navigation/ImuBias.h>

namespace gtsam {

#ifdef ALLOW_DEPRECATED_IN_GTSAM4
/// @deprecated
struct PoseVelocityBias {
  Pose3 pose;
  Vector3 velocity;
  imuBias::ConstantBias bias;
  PoseVelocityBias(const Pose3& _pose, const Vector3& _velocity,
      const imuBias::ConstantBias _bias) :
      pose(_pose), velocity(_velocity), bias(_bias) {
  }
  PoseVelocityBias(const NavState& navState, const imuBias::ConstantBias _bias) :
      pose(navState.pose()), velocity(navState.velocity()), bias(_bias) {
  }
  NavState navState() const {
    return NavState(pose, velocity);
  }
};
#endif

/**
 * PreintegrationBase is the base class for PreintegratedMeasurements
 * (in ImuFactor) and CombinedPreintegratedMeasurements (in CombinedImuFactor).
 * It includes the definitions of the preintegrated variables and the methods
 * to access, print, and compare them.
 */
class PreintegrationBase {

protected:

  /// Parameters. Declared mutable only for deprecated predict method.
  /// TODO(frank): make const once deprecated method is removed
#ifdef ALLOW_DEPRECATED_IN_GTSAM4
  mutable
#endif
  boost::shared_ptr<PreintegrationParams> p_;

  /// Acceleration and gyro bias used for preintegration
  imuBias::ConstantBias biasHat_;

  /// Time interval from i to j
  double deltaTij_;

  /**
   * Preintegrated navigation state, from frame i to frame j
   * Note: relative position does not take into account velocity at time i, see deltap+, in [2]
   * Note: velocity is now also in frame i, as opposed to deltaVij in [2]
   */
  NavState deltaXij_;

  Matrix3 delRdelBiasOmega_; ///< Jacobian of preintegrated rotation w.r.t. angular rate bias
  Matrix3 delPdelBiasAcc_;   ///< Jacobian of preintegrated position w.r.t. acceleration bias
  Matrix3 delPdelBiasOmega_; ///< Jacobian of preintegrated position w.r.t. angular rate bias
  Matrix3 delVdelBiasAcc_;   ///< Jacobian of preintegrated velocity w.r.t. acceleration bias
  Matrix3 delVdelBiasOmega_; ///< Jacobian of preintegrated velocity w.r.t. angular rate bias

  /// Default constructor for serialization
  PreintegrationBase() {
    resetIntegration();
  }

public:

  /// @name Constructors
  /// @{

  /**
   *  Constructor, initializes the variables in the base class
   *  @param p    Parameters, typically fixed in a single application
   *  @param bias Current estimate of acceleration and rotation rate biases
   */
  PreintegrationBase(const boost::shared_ptr<PreintegrationParams>& p,
      const imuBias::ConstantBias& biasHat = imuBias::ConstantBias()) :
      p_(p), biasHat_(biasHat) {
    resetIntegration();
  }

  /**
   *  Constructor which takes in all members for generic construction
   */
  PreintegrationBase(const boost::shared_ptr<PreintegrationParams>& p, const imuBias::ConstantBias& biasHat,
                     double deltaTij, const NavState& deltaXij, const Matrix3& delPdelBiasAcc,
                     const Matrix3& delPdelBiasOmega, const Matrix3& delVdelBiasAcc,
                     const Matrix3& delVdelBiasOmega)
      : p_(p),
        biasHat_(biasHat),
        deltaTij_(deltaTij),
        deltaXij_(deltaXij),
        delPdelBiasAcc_(delPdelBiasAcc),
        delPdelBiasOmega_(delPdelBiasOmega),
        delVdelBiasAcc_(delVdelBiasAcc),
        delVdelBiasOmega_(delVdelBiasOmega) {}
  /// @}

  /// @name Basic utilities
  /// @{
  /// Re-initialize PreintegratedMeasurements
  void resetIntegration();

  /// check parameters equality: checks whether shared pointer points to same PreintegrationParams object.
  bool matchesParamsWith(const PreintegrationBase& other) const {
    return p_ == other.p_;
  }

  /// shared pointer to params
  const boost::shared_ptr<PreintegrationParams>& params() const {
    return p_;
  }

  /// const reference to params
  const PreintegrationParams& p() const {
    return *boost::static_pointer_cast<PreintegrationParams>(p_);
  }

  void set_body_P_sensor(const Pose3& body_P_sensor) {
    p_->body_P_sensor = body_P_sensor;
  }
  /// @}

  /// @name Instance variables access
  /// @{
  const NavState& deltaXij() const {
    return deltaXij_;
  }
  const double& deltaTij() const {
    return deltaTij_;
  }
  const Rot3& deltaRij() const {
    return deltaXij_.attitude();
  }
  Vector3 deltaPij() const {
    return deltaXij_.position().vector();
  }
  Vector3 deltaVij() const {
    return deltaXij_.velocity();
  }
  const imuBias::ConstantBias& biasHat() const {
    return biasHat_;
  }
  const Matrix3& delRdelBiasOmega() const {
    return delRdelBiasOmega_;
  }
  const Matrix3& delPdelBiasAcc() const {
    return delPdelBiasAcc_;
  }
  const Matrix3& delPdelBiasOmega() const {
    return delPdelBiasOmega_;
  }
  const Matrix3& delVdelBiasAcc() const {
    return delVdelBiasAcc_;
  }
  const Matrix3& delVdelBiasOmega() const {
    return delVdelBiasOmega_;
  }
  // Exposed for MATLAB
  Vector6 biasHatVector() const {
    return biasHat_.vector();
  }
  /// @}

  /// @name Testable
  /// @{
  void print(const std::string& s) const;
  bool equals(const PreintegrationBase& other, double tol) const;
  /// @}

  /// @name Main functionality
  /// @{

  /// Subtract estimate and correct for sensor pose
  /// Compute the derivatives due to non-identity body_P_sensor (rotation and centrifugal acc)
  /// Ignore D_correctedOmega_measuredAcc as it is trivially zero
  std::pair<Vector3, Vector3> correctMeasurementsByBiasAndSensorPose(
      const Vector3& j_measuredAcc, const Vector3& j_measuredOmega,
      OptionalJacobian<3, 3> D_correctedAcc_measuredAcc = boost::none,
      OptionalJacobian<3, 3> D_correctedAcc_measuredOmega = boost::none,
      OptionalJacobian<3, 3> D_correctedOmega_measuredOmega = boost::none) const;

  /// Calculate the updated preintegrated measurement, does not modify
  /// It takes measured quantities in the j frame
  NavState updatedDeltaXij(const Vector3& j_measuredAcc,
      const Vector3& j_measuredOmega, const double dt,
      OptionalJacobian<9, 9> D_updated_current = boost::none,
      OptionalJacobian<9, 3> D_updated_measuredAcc = boost::none,
      OptionalJacobian<9, 3> D_updated_measuredOmega = boost::none) const;

  /// Update preintegrated measurements and get derivatives
  /// It takes measured quantities in the j frame
  void update(const Vector3& j_measuredAcc, const Vector3& j_measuredOmega,
      const double deltaT, Matrix3* D_incrR_integratedOmega, Matrix9* D_updated_current,
      Matrix93* D_udpated_measuredAcc, Matrix93* D_updated_measuredOmega);

  /// Given the estimate of the bias, return a NavState tangent vector
  /// summarizing the preintegrated IMU measurements so far
  Vector9 biasCorrectedDelta(const imuBias::ConstantBias& bias_i,
      OptionalJacobian<9, 6> H = boost::none) const;

  /// Predict state at time j
  NavState predict(const NavState& state_i, const imuBias::ConstantBias& bias_i,
      OptionalJacobian<9, 9> H1 = boost::none, OptionalJacobian<9, 6> H2 =
          boost::none) const;

  /// Compute errors w.r.t. preintegrated measurements and jacobians wrt pose_i, vel_i, bias_i, pose_j, bias_j
  Vector9 computeErrorAndJacobians(const Pose3& pose_i, const Vector3& vel_i,
      const Pose3& pose_j, const Vector3& vel_j,
      const imuBias::ConstantBias& bias_i, OptionalJacobian<9, 6> H1 =
          boost::none, OptionalJacobian<9, 3> H2 = boost::none,
      OptionalJacobian<9, 6> H3 = boost::none, OptionalJacobian<9, 3> H4 =
          boost::none, OptionalJacobian<9, 6> H5 = boost::none) const;

  /// @}

#ifdef ALLOW_DEPRECATED_IN_GTSAM4
  /// @name Deprecated
  /// @{

  /// @deprecated predict
  PoseVelocityBias predict(const Pose3& pose_i, const Vector3& vel_i,
      const imuBias::ConstantBias& bias_i, const Vector3& n_gravity,
      const Vector3& omegaCoriolis, const bool use2ndOrderCoriolis = false) const;

  /// @}
#endif

private:
  /** Serialization function */
  friend class boost::serialization::access;
  template<class ARCHIVE>
  void serialize(ARCHIVE & ar, const unsigned int /*version*/) {
    namespace bs = ::boost::serialization;
    ar & BOOST_SERIALIZATION_NVP(p_);
    ar & BOOST_SERIALIZATION_NVP(deltaTij_);
    ar & BOOST_SERIALIZATION_NVP(deltaXij_);
    ar & BOOST_SERIALIZATION_NVP(biasHat_);
    ar & bs::make_nvp("delRdelBiasOmega_", bs::make_array(delRdelBiasOmega_.data(), delRdelBiasOmega_.size()));
    ar & bs::make_nvp("delPdelBiasAcc_", bs::make_array(delPdelBiasAcc_.data(), delPdelBiasAcc_.size()));
    ar & bs::make_nvp("delPdelBiasOmega_", bs::make_array(delPdelBiasOmega_.data(), delPdelBiasOmega_.size()));
    ar & bs::make_nvp("delVdelBiasAcc_", bs::make_array(delVdelBiasAcc_.data(), delVdelBiasAcc_.size()));
    ar & bs::make_nvp("delVdelBiasOmega_", bs::make_array(delVdelBiasOmega_.data(), delVdelBiasOmega_.size()));
  }
};

} /// namespace gtsam

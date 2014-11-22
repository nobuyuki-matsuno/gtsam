/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 *  @file   Pose3AttitudeFactor.h
 *  @author Frank Dellaert
 *  @brief  Header file for Attitude factor
 *  @date   January 28, 2014
 **/
#pragma once

#include <gtsam/nonlinear/NonlinearFactor.h>
#include <gtsam/geometry/Pose3.h>
#include <gtsam/geometry/Unit3.h>

namespace gtsam {

/**
 * Base class for prior on attitude
 * Example:
 * - measurement is direction of gravity in body frame bF
 * - reference is direction of gravity in navigation frame nG
 * This factor will give zero error if nRb * bF == nG
 * @addtogroup Navigation
 */
class AttitudeFactor {

protected:

  const Unit3 nZ_, bRef_; ///< Position measurement in

public:

  /** default constructor - only use for serialization */
  AttitudeFactor() {
  }

  /**
   * @brief Constructor
   * @param nZ measured direction in navigation frame
   * @param bRef reference direction in body frame (default Z-axis in NED frame, i.e., [0; 0; 1])
   */
  AttitudeFactor(const Unit3& nZ, const Unit3& bRef = Unit3(0, 0, 1)) :
      nZ_(nZ), bRef_(bRef) {
  }

  /** vector of errors */
  Vector attitudeError(const Rot3& p,
      boost::optional<Matrix&> H = boost::none) const;
};

/**
 * Version of AttitudeFactor for Rot3
 * @addtogroup Navigation
 */
class GTSAM_EXPORT Rot3AttitudeFactor: public NoiseModelFactor1<Rot3>, public AttitudeFactor {

  typedef NoiseModelFactor1<Rot3> Base;

public:

  /// shorthand for a smart pointer to a factor
  typedef boost::shared_ptr<Rot3AttitudeFactor> shared_ptr;

  /// Typedef to this class
  typedef Rot3AttitudeFactor This;

  /** default constructor - only use for serialization */
  Rot3AttitudeFactor() {
  }

  virtual ~Rot3AttitudeFactor() {
  }

  /**
   * @brief Constructor
   * @param key of the Rot3 variable that will be constrained
   * @param nZ measured direction in navigation frame
   * @param model Gaussian noise model
   * @param bRef reference direction in body frame (default Z-axis)
   */
  Rot3AttitudeFactor(Key key, const Unit3& nZ, const SharedNoiseModel& model,
      const Unit3& bRef = Unit3(0, 0, 1)) :
      Base(model, key), AttitudeFactor(nZ, bRef) {
  }

  /// @return a deep copy of this factor
  virtual gtsam::NonlinearFactor::shared_ptr clone() const {
    return boost::static_pointer_cast<gtsam::NonlinearFactor>(
        gtsam::NonlinearFactor::shared_ptr(new This(*this)));
  }

  /** print */
  virtual void print(const std::string& s, const KeyFormatter& keyFormatter =
      DefaultKeyFormatter) const;

  /** equals */
  virtual bool equals(const NonlinearFactor& expected, double tol = 1e-9) const;

  /** vector of errors */
  virtual Vector evaluateError(const Rot3& nRb, //
      boost::optional<Matrix&> H = boost::none) const {
    return attitudeError(nRb, H);
  }
  Unit3 nZ() const {
    return nZ_;
  }
  Unit3 bRef() const {
    return bRef_;
  }

private:

  /** Serialization function */
  friend class boost::serialization::access;
  template<class ARCHIVE>
  void serialize(ARCHIVE & ar, const unsigned int version) {
    ar
        & boost::serialization::make_nvp("NoiseModelFactor1",
            boost::serialization::base_object<Base>(*this));
    ar & BOOST_SERIALIZATION_NVP(nZ_);
    ar & BOOST_SERIALIZATION_NVP(bRef_);
  }
};

/**
 * Version of AttitudeFactor for Pose3
 * @addtogroup Navigation
 */
class GTSAM_EXPORT Pose3AttitudeFactor: public NoiseModelFactor1<Pose3>,
    public AttitudeFactor {

  typedef NoiseModelFactor1<Pose3> Base;

public:

  /// shorthand for a smart pointer to a factor
  typedef boost::shared_ptr<Pose3AttitudeFactor> shared_ptr;

  /// Typedef to this class
  typedef Pose3AttitudeFactor This;

  /** default constructor - only use for serialization */
  Pose3AttitudeFactor() {
  }

  virtual ~Pose3AttitudeFactor() {
  }

  /**
   * @brief Constructor
   * @param key of the Pose3 variable that will be constrained
   * @param nZ measured direction in navigation frame
   * @param model Gaussian noise model
   * @param bRef reference direction in body frame (default Z-axis)
   */
  Pose3AttitudeFactor(Key key, const Unit3& nZ, const SharedNoiseModel& model,
      const Unit3& bRef = Unit3(0, 0, 1)) :
      Base(model, key), AttitudeFactor(nZ, bRef) {
  }

  /// @return a deep copy of this factor
  virtual gtsam::NonlinearFactor::shared_ptr clone() const {
    return boost::static_pointer_cast<gtsam::NonlinearFactor>(
        gtsam::NonlinearFactor::shared_ptr(new This(*this)));
  }

  /** print */
  virtual void print(const std::string& s, const KeyFormatter& keyFormatter =
      DefaultKeyFormatter) const;

  /** equals */
  virtual bool equals(const NonlinearFactor& expected, double tol = 1e-9) const;

  /** vector of errors */
  virtual Vector evaluateError(const Pose3& nTb, //
      boost::optional<Matrix&> H = boost::none) const {
    Vector e = attitudeError(nTb.rotation(), H);
    if (H) {
      Matrix H23 = *H;
      *H = Matrix::Zero(2, 6);
      H->block<2,3>(0,0) = H23;
    }
    return e;
  }
  Unit3 nZ() const {
    return nZ_;
  }
  Unit3 bRef() const {
    return bRef_;
  }

private:

  /** Serialization function */
  friend class boost::serialization::access;
  template<class ARCHIVE>
  void serialize(ARCHIVE & ar, const unsigned int version) {
    ar
        & boost::serialization::make_nvp("NoiseModelFactor1",
            boost::serialization::base_object<Base>(*this));
    ar & BOOST_SERIALIZATION_NVP(nZ_);
    ar & BOOST_SERIALIZATION_NVP(bRef_);
  }
};

} /// namespace gtsam


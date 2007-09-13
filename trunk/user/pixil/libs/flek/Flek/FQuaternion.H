/* -*-C++-*- 

   "$Id$"
  
   Copyright 1999-2000 by the Flek development team.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA.
   
   Please report all bugs and problems to "flek-devel@sourceforge.net".

*/

// The original vector, matrix, and quaternion code was written by
// Vinod Srinivasan and then adapted for Flek.

#ifndef __FQUATERNION_H__
#define __FQUATERNION_H__

#include <Flek/FBase.H>
#include <Flek/FVector3.H>
#include <Flek/FMatrix4x4.H>

/** @package libflek_core
 * The Quaternion class.
 */
class FQuaternion : public FBase
{
public:

  /**
   * Default constructor - create an identity quaternion
   */
  FQuaternion ()
    : FBase (), v (), w (1.0)
    {}

  /** 
   * Construct from a vector. Scalar is set to 0
   */
  FQuaternion (const FVector3& vec)
    : FBase (), v (vec), w (0.0)
    {}
  
  /**
   * Constructor from a vector and a scalar
   */
  FQuaternion (const FVector3& vec, float scalar)
    : FBase (), v (vec), w (scalar)
    {}

  /**
   * Same as above, but with reverse order
   */
  FQuaternion (float scalar, const FVector3& vec)
    : FBase (), v (vec), w (scalar)
    {}

  /**
   * Construct from 3/4 individual values. Scalar is set to 0 by default
   */
  FQuaternion (float x, float y, float z, float scalar=0.0)
    : FBase (), v (x, y, z), w (scalar)
    {}

  /**
   * Copy constructor
   */
  FQuaternion (const FQuaternion& quat)
    : FBase (quat), v (quat.v), w (quat.w)
    {}

  /**
   * Destructor
   */
  ~FQuaternion ()
    {}

  /**
   * Assignment operator
   */
  FQuaternion& operator = (const FQuaternion& quat)
    {
      FBase :: operator = (quat);
      v = quat.v; w = quat.w;
      return (*this);
    }

  /**
   * Assignment from FVector3
   */
  FQuaternion& operator = (const FVector3& vec)
    {
      v = vec; w = 0.0;
      return (*this);
    }
     
  /**
   * Set the vector and scalar parts of the quaternion
   */
  void set (const FVector3& vec, float scalar=0.0)
    {
      v = vec; w = scalar;
    }

  /**
   * Set the vector and scalar parts of the quaternion
   */
  void set (float x, float y, float z, float scalar=0.0)
    {
      v.set (x, y, z); w = scalar;
    }

  /**
   * Reset the quaternion to its default state - identity.
   */
  void reset (void)
    {
      v.reset (); w = 1.0;
    }

  /**
   *  Make a copy of the object implement FBase class pure virtual function
   */
  virtual FBase * copy (void) const
    {
      FQuaternion * quat = new FQuaternion (*this);
      return quat;
    }
  
  /**
   * Access the elements of the quaternion. Indices start at 0 and the
   *  scalar is at index=3
   */
  float& operator [] (uint index)
    {
      if ( index == 3 ) return w;
      return v[index];
    }
     
  float operator [] (uint index) const
    {
      if ( index == 3 ) return w;
      return v[index];
    }

  /**
   * Class member functions which return the identity quaternion
   * Identity quaternion is with the all vector components=0 and scalar=1
   */
  static FQuaternion identity(void)
    {
      FQuaternion ident;  // Default constructor creates an identity quaternion
      return ident;
    }

  /**
   * Class member functions which return the identity quaternion
   * Identity quaternion is with the all vector components=0 and scalar=1
   */     
  static FQuaternion I (void)
    {
      FQuaternion ident;  // Default constructor creates an identity quaternion
      return ident;
    }

  void operator += (const FQuaternion& quat)
    {
      w += quat.w;
      v += quat.v;
    }
  
  void operator -= (const FQuaternion& quat)
    {
      w -= quat.w;
      v -= quat.v;
    }
  
  void operator *= (const FQuaternion& quat)
    {
      FVector3 vec;
      float s;
      
      s   = w*quat.w - v*quat.v;
      vec = quat.v*w + v*quat.w + (v % quat.v);
      
      w = s; v = vec;
    }
  
  void operator *= (const FVector3& vec)
    {
      FQuaternion v2q (vec);
      
      this->operator *= (v2q);
    }
  
  void operator *= (float scalar)
    {
      w *= scalar;
      v *= scalar;
    }
  
  void operator /= (float scalar)
    {
      w /= scalar;
      v /= scalar;
    }
  
  /**
   * Square of the length of a quaternion
   */
  friend float lengthsqr(const FQuaternion& quat)
    {
      float lensqr = quat.v*quat.v + quat.w*quat.w;
      return lensqr;
    }
  
  /**
   * Length of a quaternion
   */
  friend float length(const FQuaternion& quat)
    {
      return sqrt(lengthsqr(quat));
    }
  
  /**
   * For consistency define norm and normsqr also
   */
  friend float normsqr(const FQuaternion& quat)
    {
      return lengthsqr(quat);
    }

  friend float norm(const FQuaternion& quat)
    {
      return length(quat);
    }
  
  /**
   * Compute conjugate of this quaternion which -v,s
   */
  friend FQuaternion conjugate(const FQuaternion& quat)
    {
      return FQuaternion(-quat.v,quat.w);
    }
  
  /**
   * Normalize a quaternion - make it a unit quaternion
   * Returns the original length of the quaternion
   * If length is 0, nothing is changed
   */
  friend float normalize(FQuaternion& quat)
    {
      float len = length(quat);
      if ( is_non_zero(len) == true )
	quat /= len;
      return len;
    }

  friend FQuaternion normalized(const FQuaternion& quat)
    {
      FQuaternion unitq = quat;
      normalize(unitq);
      return unitq;
    }

  /**
   * Negation
   */
  friend FQuaternion operator - (const FQuaternion& q)
    {
      FQuaternion neg(-q.w, -q.v);
      return neg;
    }
  
  friend FQuaternion operator + (const FQuaternion& q1, const FQuaternion& q2)
    {
      FQuaternion sum = q1;
      sum += q2;
      return sum;
    }
  
  friend FQuaternion operator - (const FQuaternion& q1, const FQuaternion& q2)
    {
      FQuaternion diff = q1;
      diff -= q2;
      return diff;
    }
  
  /**
   * Post-multiplication by a scalar
   */
  friend FQuaternion operator * (const FQuaternion& q, float scalar)
    {
      FQuaternion prod = q;
      prod *= scalar;
      return prod;
    }
  
  /**
   * Pre-multiplication by a scalara
   */
  friend FQuaternion operator * (float scalar, const FQuaternion& q)
    {
      FQuaternion prod = q;
      prod *= scalar;
      return prod;
    }

  /** 
   * Division by a scalar
   */
  friend FQuaternion operator / (const FQuaternion& q, float scalar)
    {
      FQuaternion quot = q;
      quot /= scalar;
      return quot;
    }
  
  /**
   * Multiplication of 2 quaternions
   */
  friend FQuaternion operator * (const FQuaternion& q1, const FQuaternion& q2)
    {
      FQuaternion prod = q1;
      prod *= q2;
      return prod;
    }

  /**
   * Post-multiplication of a quaternion by a FVector3
   * Same as above, except FVector3 is promoted to quaternion with 
   * scalar value 0.0
   */
  friend FQuaternion operator * (const FQuaternion& q, const FVector3& v)
    {
      FQuaternion prod = q;
      prod *= v;
      return prod;
    }
  
  /**
   * Pre-multiplication of a quaternion by a FVector3
   */
  friend FQuaternion operator * (const FVector3& v, const FQuaternion& q)
    {
      FQuaternion prod = q;
      prod *= v;
      return prod;
    }

  /**
   * Convert to a rotation matrix. Assumes that quaternion has been normalized
   */
  FMatrix3x3 to_matrix(void) const
    {
      FMatrix3x3 mat;
      float x,y,z,s;
      
      v.get(x,y,z); s = w;
      
      mat[0].set( 1.0 - 2.0*(y*y + z*z), 2.0*(x*y - s*z), 2.0*(x*z + s*y) );
      mat[1].set( 2.0*(x*y + s*z), 1.0 - 2.0*(x*x + z*z), 2.0*(y*z - s*x) );
      mat[2].set( 2.0*(x*z - s*y), 2.0*(y*z + s*x), 1.0 - 2.0*(x*x + y*y) );
      
      return mat;
    }
  
  FMatrix4x4 to_matrix4(void) const
    {
      FMatrix4x4 mat4 = to_matrix();
      mat4[3][3] = 1.0;
      return mat4;
    }
  
  void to_matrix(float array[16]) const
    {
      // Similar to toMatrix4, but fills a given array of 16 elements with the
      // rotation matrix corresponding to this quaternion in column major form
      FMatrix4x4 mat4 = to_matrix4();
      mat4.fill_array_column_major(array);
    }
     
  friend FMatrix3x3 to_matrix(const FQuaternion& quat)
    {
      return quat.to_matrix();
    }
  
  friend FMatrix4x4 to_matrix4(const FQuaternion& quat)
    {
      return quat.to_matrix4();
    }
  
  //--- Conversion to and from axis and angle ---
  //--- All angles are specified in radians ---
  
  /**
   * Get the angle of rotation. Returns value between 0 and PI
   */
  float get_angle(void) const
    {
      return 2.0 * acos(w);
    }
  
  /**
   * Get the angle of rotation. Returns value between 0 and PI
   * Computes angle after normalizing the quaternion
   */
  friend float get_angle(const FQuaternion& quat)
    {
      // Make the quaternion a unit quaternion first
      FQuaternion nq = normalized(quat);
      
      return nq.get_angle();
    }
     
  /**
   *  Get the axis of rotation. Returns unit axis
   */
  friend FVector3 get_axis(const FQuaternion& quat)
    {
      // Make the quaternion a unit quaternion first
      FQuaternion nq = normalized(quat);
      
      FVector3 axis = normalized(nq.v);
      return axis;
    }
     
  /**
   * Get the axis and angle.
   */
  void get_axis_and_angle(FVector3& axis, float& theta) const
    {
      FQuaternion nq = normalized(*this);
      
      theta = 2.0 * acos(nq.w);
      axis = normalized(nq.v);
    }
  
  /**
   * Set the axis and angle.
   */
  void set_axis_and_angle(const FVector3& axis, float theta)
    {
      w = cos(theta/2.0);
      v = normalized(axis);
      v *= sin(theta/2.0);
    }

  /**
   * Change the angle to a new value. Use same axis as before
   */
  void set_angle(float theta)
    {
      w = cos(theta/2.0);
      normalize(v);
      v *= sin(theta/2.0);
    }
  
  /**
   * Scale the angle by the given scale factor
   */
  void scale_angle(float scale_factor)
    {
      set_angle(get_angle()*scale_factor);
    }
  
  /**
   * Insertion and extraction operators
   */
  friend ostream& operator << (ostream& o, const FQuaternion& quat)
    {
      o << "{ " << quat.v << "," << quat.w << " }";
      return o;
    }

  /**
   * Insertion and extraction operators
   */  
  friend istream& operator >> (istream& i, FQuaternion& quat)
    {
      char c;
      i >> c >> quat.v >> c >> quat.w >> c;
      return i;
    }

protected:

  /**
   * Vector component
   */
  FVector3 v;
  
  /**
   * Scalar component
   */
  float w;

};

#endif // #ifndef FQuaternion_H_

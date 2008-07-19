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

#ifndef _FTRANSFORMATION_H_
#define _FTRANSFORMATION_H_

#include <Flek/FVector3.H>
#include <Flek/FMatrix4x4.H>
#include <Flek/FQuaternion.H>
#include <math.h>
#include <GL/gl.h>

/**
 * @package libflek_core
 *
 * Class for transformations (translation, scaling, rotation).
 */
class FTransformation {
  
public:

  typedef FTransformation* Ptr;
  
  /**
   * Default constructor.
   */
  FTransformation ()
    : transform() {}
  
  /**
   * Copy constructor.
   */
  FTransformation(const FTransformation& tr)
    : transform(tr.transform) {}
  
  /** 
   * Construct from a matrix
   */
  FTransformation (const FMatrix4x4& mat)
    : transform(mat) {}
  
  /**
   * Destructor.
   */
  ~FTransformation() {}
  
  /**
   * Assignment operator.
   */
  FTransformation& operator = (const FTransformation& tr) {
    transform = tr.transform;
    return (*this);
  }
  
  /**
   * Static function to compute translation matrix from a vector.
   */
  static FMatrix4x4 translation (const FVector3& t) {
    FMatrix4x4 transmat;
    transmat[0][3] = t[0];
    transmat[1][3] = t[1];
    transmat[2][3] = t[2];
    return transmat;
  }

  /**
   * Static function to compute translation matrix from x, y, z values.
   */  
  static FMatrix4x4 translation (float tx, float ty, float tz) {
    FMatrix4x4 transmat;
    transmat[0][3] = tx;
    transmat[1][3] = ty;
    transmat[2][3] = tz;
    return transmat;
  }

  /**
   * Static function to compute rotation matrix about the x axis.
   */  
  static FMatrix4x4 rotation_x (float angle) {
    FMatrix4x4 rotmat;
    float ct = cos(angle), st = sin(angle);
    rotmat[0].set( 1,  0,  0, 0);
    rotmat[1].set( 0, ct, st, 0);
    rotmat[2].set( 0,-st, ct, 0);
    rotmat[3].set( 0,  0,  0, 1);
    return rotmat;
  }
  
  /**
   * Static function to compute rotation matrix about the y axis.
   */  
  static FMatrix4x4 rotation_y (float angle) {
    FMatrix4x4 rotmat;
    float ct = cos(angle), st = sin(angle);
    rotmat[0].set(ct, 0,-st, 0);
    rotmat[1].set( 0, 1,  0, 0);
    rotmat[2].set(st, 0, ct, 0);
    rotmat[3].set( 0, 0,  0, 1);
    return rotmat;
  }
  
  /**
   * Static function to compute rotation matrix about the z axis.
   */  
  static FMatrix4x4 rotation_z (float angle) {
    FMatrix4x4 rotmat;
    float ct = cos(angle), st = sin(angle);
    rotmat[0].set( ct, st, 0, 0);
    rotmat[1].set(-st, ct, 0, 0);
    rotmat[2].set(  0,  0, 1, 0);
    rotmat[3].set(  0,  0, 0, 1);
    return rotmat;
  }
  
  /**
   * Static function to compute scale matrix from a vector.
   */  
  static FMatrix4x4 scaling (const FVector3& s) {
    FMatrix4x4 scmat;
    scmat[0][0] = s[0];
    scmat[1][1] = s[1];
    scmat[2][2] = s[2];
    return scmat;
  }

  /**
   * Static function to compute scale matrix from x, y, z values.
   */
  static FMatrix4x4 scaling (float sx, float sy, float sz) {
    FMatrix4x4 scmat;
    scmat[0][0] = sx;
    scmat[1][1] = sy;
    scmat[2][2] = sz;
    return scmat;
  }
  
  /**
   * Combining two FTransformations.
   * Post-multiply with given FTransformation.
   */
  void operator *= (const FTransformation& tr) {
    transform *= tr.transform;
  }

  /**
   * Combining FTransformation and matrix.
   * Post-multiply with given matrix.
   */  
  void operator *= (const FMatrix4x4& mat) {
    transform *= mat;
  }
  
  /** 
   * Pre-multiply with given FTransformation/matrix
   * The operator chosen is not the most intuitive, but the only one that makes
   * some kind of sense
   */
  void operator /= (const FTransformation& tr) {
    transform = tr.transform * transform;
  }

  /** 
   * Pre-multiply with given FTransformation/matrix
   * The operator chosen is not the most intuitive, but the only one that makes
   * some kind of sense.
   */
  void operator /= (const FMatrix4x4& mat) {
    transform = mat * transform;
  }
  
  FTransformation operator * (const FTransformation& tr) {
    FTransformation newtr(*this);
    newtr *= tr;
    return newtr;
  }
  
  FTransformation operator * (const FMatrix4x4& mat) {
    FTransformation newtr(*this);
    newtr *= mat;
    return newtr;
  }
  
  friend FTransformation operator * (const FMatrix4x4& mat, const FTransformation& tr) {
    FTransformation newtr(mat);
    newtr *= tr;
    return newtr;
  }

  /**
   * Invert the FTransformation matrix.
   */
  void invert (void) {
    transform.invert();
  }
  
  // Apply FTransformations - pre-multiply by corresponding FTransformation matrices
  void translate (const FVector3& t) {
    transform = FTransformation :: translation(t) * transform;
  }
  
  void translate (float tx, float ty, float tz) {
    transform = FTransformation::translation (tx,ty,tz) * transform;
  }
  
  void rotate_x (float angle) {
    transform = FTransformation::rotation_x (angle) * transform;
  }
  
  void rotate_y (float angle) {
    transform = FTransformation::rotation_y (angle) * transform;
  }
  
  void rotate_z (float angle) {
    transform = FTransformation::rotation_z (angle) * transform;
  }

  /**
   * Rotate according to the rotation specified by the FQuaternion.
   */
  void rotate (const FQuaternion& quat) {
    transform = quat.to_matrix4() * transform;
  }
  
  void scale (const FVector3& s) {
    transform = FTransformation :: scaling(s) * transform;
  }
  
  void scale (float sx, float sy, float sz) {
    transform = FTransformation :: scaling(sx,sy,sz) * transform;
  }
  
  /**
   * Apply FTransformations - post-multiply
   */
  void post_translate (const FVector3& t) {
    transform *= FTransformation :: translation(t);
  }
  
  void post_translate (float tx, float ty, float tz) {
    transform *= FTransformation :: translation(tx,ty,tz);
  }
  
  void post_rotateX (float angle) {
    transform *= FTransformation :: rotation_x(angle);
  }
  
  void post_rotateY (float angle) {
    transform *= FTransformation :: rotation_y(angle);
  }
  
  void post_rotateZ (float angle) {
    transform *= FTransformation :: rotation_z(angle);
  }

  /**
   * Rotate according to the rotation specified by the FQuaternion.
   */
  void post_rotate (const FQuaternion& quat) {
    transform *= quat.to_matrix4();
  }
  
  void post_scale (const FVector3& s) {
    transform *= FTransformation :: scaling(s);
  }
  
  void post_scale (float sx, float sy, float sz) {
    transform *= FTransformation :: scaling(sx,sy,sz);
  }
  
  /**
   * Apply the FTransformation in OpenGL. Calls only glMultMatrix.
   */
  void apply (void) const {
    float mat[16];
    transform.fill_array_column_major(mat);
    glMultMatrixd(mat);
  }
  
  /**
   * Get the FTransformation matrix.
   */
  FMatrix4x4 matrix (void) {
    return transform;
  }
  
  /**
   * Reset the FTransformation matrix.
   */
  void reset (void) {
    transform.reset();
  }
  
  /**
   * Set the FTransformation matrix.
   */
  void set (const FMatrix4x4 mat) {
    transform = mat;
  }

protected:
  
  // Combined FTransformation matrix
  FMatrix4x4 transform;
  
};

#endif

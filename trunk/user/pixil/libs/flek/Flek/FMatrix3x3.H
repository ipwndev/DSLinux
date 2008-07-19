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

#ifndef _FMATRIX3x3_H_
#define _FMATRIX3x3_H_

#include <Flek/FBase.H>
#include <Flek/FVector3.H>

/** @package libflek_core
 * Class for a 3x3 matrix. Built from Vector3d
 * Row-major form is used. (ie) each row of the matrix
 * is a Vector3d. This makes inversion easier, since elementary
 * row operations are simplified
 */

class FMatrix3x3 : public FBase
{
public:
  
  typedef FMatrix3x3 * Ptr;
  
  /**
   * Default constructor - creates an identity matrix.
   */
  FMatrix3x3 ()
    : FBase ()
    {
      row[0].set (1.0, 0.0, 0.0);
      row[1].set (0.0, 1.0, 0.0);
      row[2].set (0.0, 0.0, 1.0);
    }

  /**
   * One argument constructor - from scalar, set all elements to given value.
   */
  FMatrix3x3 (float scalar)
    : FBase ()
    {
      row[0] = scalar; row[1] = scalar; row[2] = scalar;
    }
  
  /**
   * Three argument constructor - from 3 FVector3s.
   */
  FMatrix3x3 (const FVector3& r0, const FVector3& r1, const FVector3& r2)
    : FBase ()
    {
      row[0] = r0; row[1] = r1; row[2] = r2;
    }

  /**
   * Copy constructor.
   */
  FMatrix3x3 (const FMatrix3x3& mat)
    : FBase ()
    {
      copy_from (mat);
    }

  /**
   * Destructor
   */
  virtual ~FMatrix3x3 ()
    {}
  
  /**
   *  Assignment operator
   */
  FMatrix3x3& operator = (const FMatrix3x3& mat)
    {
      copy_from (mat);
      return (*this);
    }
  
  /**
   * Assignment from a scalar
   */
  void operator = (float scalar)
    {
      row[0] = scalar; row[1] = scalar; row[2] = scalar;
    }
  
  /**
   * Sets the rows of the matrix.
   * 
   * @param r0 The new row "0" for this matrix.
   * @param r1 The new row "0" for this matrix.
   * @param r2 The new row "0" for this matrix.
   */
  void set(const FVector3& r0, const FVector3& r1, const FVector3& r2)
    {
      row[0] = r0; row[1] = r1; row[2] = r2;
    }

  /**
   * Sets every element in the matrix to a scalar.
   * 
   * @param scalar Value assigned to each element.
   */
  void set (float scalar)
    {
      row[0] = scalar; row[1] = scalar; row[2] = scalar;
    }
     
  /**
   *  Reset the matrix to it's default state - identity
   */
  void reset ()
    {
      row[0].set (1.0, 0.0, 0.0);
      row[1].set (0.0, 1.0, 0.0);
      row[2].set (0.0, 0.0, 1.0);
    }

  /*
   * Make a copy of the object
   * implement FBase class pure virtual function
   */
  virtual FBase::Ptr copy (void) const
    {
      Ptr mat = new FMatrix3x3 (*this);
      return mat;
    }

  /**
   * Access a row of the matrix - no range checks
   * 
   * @param index The index of the row that should be returned.
   * @return The specified row of the matrix.
   */
  FVector3& operator [] (uint index) {
    return row[index];
  }

  /**
   * Access a row of the matrix - no range checks : const version
   * 
   * @param index The index of the row that should be returned.
   * @return The specified row of the matrix.
   */
  const FVector3& operator [] (uint index) const
    {
      return row[index];
    }

  /**
   * Creates the identity matrix.
   * 
   * @return The identity matrix.
   */
  static FMatrix3x3 identity ()
    {
      FMatrix3x3 imat;  // Default constructor creates an identity matrix
      return imat;
    }

  /**
   * Creates the identity matrix.
   * 
   * @return The identity matrix.
   */
  static FMatrix3x3 I (void)
    {
      FMatrix3x3 imat;  // Default constructor creates an identity matrix
      return imat;
    }
  
  /**  
   * Adds each element in the given matrix to this matrix.
   * 
   * @param mat The matrix of values to add to this matrix.
   */
  void operator += (const FMatrix3x3& mat)
    {
      for (int i=0; i < 3; ++i)
	row[i] += mat.row[i];
    }

  /**  
   * Subtracts each element in the given matrix from this matrix.
   * 
   * @param mat The matrix of values to subtract from this matrix.
   */
  void operator -= (const FMatrix3x3& mat)
    {
      for (int i=0; i < 3; ++i)
	row[i] -= mat.row[i];
    }

  /**  
   * Multiplies each element in the matrix by a scalar.
   * 
   * @param scalar The scalar to multiply by.
   */
  void operator *= (float scalar)
    {
      for (int i=0; i < 3; ++i)
	row[i] *= scalar;
    }

  /**  
   * Divides each element in the matrix by a scalar.
   * 
   * @param scalar The scalar to divide by.
   */
  void operator /= (float scalar)
    {
      for (int i=0; i < 3; ++i)
	row[i] /= scalar;
    }
  
  /**
   * Transposes this matrix.
   */
  void transpose (void)
    {
      for (int i=0; i < 3; ++i)
	for (int j=i+1; j < 3; ++j)
	  swap (row[i][j], row[j][i]);
    }

  /**
   * Finds the transpose of a given matrix.
   * 
   * @param mat The matrix to find a transpose for.
   * @return The transpose of the giben matrix.
   */
  friend FMatrix3x3 transpose (const FMatrix3x3& mat)
    {
      FMatrix3x3 trans (mat); trans.transpose ();
      return trans;
    }

  /**
   * Finds the determinant of a given matrix.
   * 
   * @param mat The source matrix.
   * @return The detrminant of the source matrix.
   */  
  friend float determinant(const FMatrix3x3& mat)
    {
      float det = 0.0;
      
      det =   mat[0][0] * mat[1][1] * mat[2][2] 
	+ mat[0][1] * mat[1][2] * mat[2][0] 
	+ mat[0][2] * mat[1][0] * mat[2][1] 
	- mat[2][0] * mat[1][1] * mat[0][2] 
	- mat[2][1] * mat[1][2] * mat[0][0] 
	- mat[2][2] * mat[1][0] * mat[0][1];
      return det;
    }
  
  /**
   * Inverts this matrix, using elementary row operations.
   */
  void invert (void)
    {
      FVector3 inv[3];
      int i,j,swaprow;
     
      for (i=0; i<3; ++i)
	inv[i][i] = 1.0;
      
      // inv will be identity initially and will become the inverse at the end
      for (i=0; i < 3; ++i)
	{
	  // i is column
	  // Find row in this column which has largest element (magnitude)
	  swaprow = i;
	  for (j=i+1; j < 3; ++j)
	    if ( fabs (row[j][i]) > fabs (row[i][i]) ) swaprow = j;
	  
	  if ( swaprow != i )
	    {
	      // Swap the two rows to get largest element in main diagonal
	      // Do this for the RHS also
	      swap (row[i],row[swaprow]); 
	      swap (inv[i],inv[swaprow]);
	    }
	  
	  // Check if pivot is non-zero
	  if ( !is_non_zero(row[i][i]) )
	    {
	      cerr << "FMatrix3x3 inverse(const FMatrix3x3&) : Singular matrix!" << endl;
	      // Return original matrix without change
	      return;
	    }
	  
	  // Divide matrix by main diagonal element to make it 1.0
	  float fact = row[i][i];
	  for (j=0; j < 3; ++j)
	    {
	      row[j] /= fact;
	      inv[j] /= fact;
	    }
	  
	  // Make non-main-diagonal elements in this column 0 using main-diagonal row
	  for (j=0; j < 3; ++j)
	    {
	      if ( j != i )
		{
		  float temp = row[j][i];
		  row[j] -= row[i]*temp;
		  inv[j] -= inv[i]*temp;
		}
	    }
	}
      
      // Main-diagonal elements on LHS may not be 1.0 now. Divide to make LHS identity
      // Last row will be 1.0
      for (i=0; i < 2; ++i)
	{
	  float pivot = row[i][i];
	  row[i] /= pivot;
	  inv[i] /= pivot;
	}
      for (i=0; i < 3; ++i)
	row[i] = inv[i];
    }
  
  /**
   * Find the inverse of a given matrix using elementary row operations.
   * 
   * @param mat The source matrix.
   * @return The inverse of the source matrix.
   */
  friend FMatrix3x3 invert (const FMatrix3x3& mat)
    {
      FMatrix3x3 inv (mat); inv.invert ();
      return inv;
    }
  
  /**
   * Negation.  Each element in the given matrix is returned
   * negated in a new matrix.
   * 
   * @param mat The source matrix.
   * @return The negated matrix.
   */
  friend FMatrix3x3 operator - (const FMatrix3x3& mat)
    {
      FMatrix3x3 neg;
      
      for (int i=0; i < 3; ++i)
	neg.row[i] = -mat.row[i];
      
      return neg;
    }
  
  friend FMatrix3x3 operator + (const FMatrix3x3& mat1, const FMatrix3x3& mat2)
    {
      FMatrix3x3 sum (mat1);
      sum += mat2;
      return sum;
    }
  
  friend FMatrix3x3 operator - (const FMatrix3x3& mat1, const FMatrix3x3& mat2)
    {
      FMatrix3x3 diff (mat1);
      diff -= mat2;
      return diff;
    }
  
  /**
   * Post-multiplication by a scalar.
   */
  friend FMatrix3x3 operator * (const FMatrix3x3& mat, float scalar)
    {
      FMatrix3x3 prod (mat);
      prod *= scalar;
      return prod;
    }
  
  /**
   * Pre-multiplication by a scalar.
   */
  friend FMatrix3x3 operator * (float scalar, const FMatrix3x3& mat)
    {
      FMatrix3x3 prod (mat);
      prod *= scalar;
      return prod;
    }
  
  /**
   * Division by a scalar.
   */
  friend FMatrix3x3 operator / (const FMatrix3x3& mat, float scalar)
    {
      FMatrix3x3 quot (mat);
      quot /= scalar;
      return quot;
    }
  
  /**
   * Multiplication of 2 matrices - outer product.
   */
  friend FMatrix3x3 operator * (const FMatrix3x3& mat1, const FMatrix3x3& mat2);
  
  /** 
   * Element-by-element multiplication of 2 matrices.
   */
  friend FMatrix3x3 product (const FMatrix3x3& mat1, const FMatrix3x3& mat2)
    {
      FMatrix3x3 prod;
      
      prod.row[0] = product (mat1[0], mat2[0]);
      prod.row[1] = product (mat1[1], mat2[1]);
      prod.row[2] = product (mat1[2], mat2[2]);
      
      return prod;
    }
  
  /**
   * Post-multiplication by a FVector3. Vector is assumed to be a column vector.
   */
  friend FVector3 operator * (const FMatrix3x3& mat, const FVector3& vec)
    {
      FVector3 prod;
      
      prod.set (mat.row[0]*vec, mat.row[1]*vec, mat.row[2]*vec);
      return prod;
    }
  
  /**
   * Pre-multiplication by a FVector3. Vector is assumed to be a row vector.
   */
  friend FVector3 operator * (const FVector3& vec, const FMatrix3x3& mat);
  
  /** 
   * Multiplication of two FVector3s to produce a FMatrix3x3 - outer product
   * or tensor product of two Vectors.  Same as multiplying row vector (v1) 
   * with column vector (v2) 
   */
  friend FMatrix3x3 operator ^ (const FVector3& v1, const FVector3& v2)
    {
      FMatrix3x3 prod;
      
      prod.row[0] = v1[0]*v2;
      prod.row[1] = v1[1]*v2;
      prod.row[2] = v1[2]*v2;
      
      return prod;
    }
  
  friend void swap (FMatrix3x3& mat1, FMatrix3x3& mat2)
    {
      for (int i=0; i < 3; ++i)
	swap (mat1.row[i], mat2.row[i]);
    }
  
  /**
   * Multiply self with another matrix. Simply calls above defined 
   * friend function.
   */
  void operator *= (const FMatrix3x3& mat)
    {
      FMatrix3x3 prod = (*this) * mat;
      *this = prod;
    }
  
  friend ostream& operator << (ostream& o, const FMatrix3x3& mat)
    {
      o << "[ " << mat.row[0] << "," << endl
	<< "  " << mat.row[1] << "," << endl
	<< "  " << mat.row[2] << " ]" << endl;
      return o;
    }
  
  friend istream& operator >> (istream& i, FMatrix3x3& mat)
    {
      char rowsep, dummy;
      i >> dummy >> mat.row[0] >> rowsep
	>> mat.row[1] >> rowsep
	>> mat.row[2] >> dummy;
      return i;
    }
  
protected:
  
  /**
   * 3 rows of the matrix
   */
  FVector3 row[3];
  
  /**
   * Copy values from another matrix.
   */
  void copy_from (const FMatrix3x3& mat)
    {
      for (int i=0; i < 3; ++i)
	row[i] = mat.row[i];
    }
};

#endif // #ifndef FMatrix_3x3_H_ 

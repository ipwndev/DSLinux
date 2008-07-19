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

#ifndef _FMATRIX_4x4_H_
#define _FMATRIX_4x4_H_

#include <Flek/FBase.H>
#include <Flek/FVector4.H>
#include <Flek/FMatrix3x3.H>

/** @package libflek_core
 * Class for a 4x4 matrix. Built from Vector4d
 * Row-major form is used. (ie) each row of the matrix
 * is a Vector3d. This makes inversion easier, since elementary
 * row operations are simplified
 */

class FMatrix4x4 : public FBase
{
public:
  
  typedef FMatrix4x4 * Ptr;
  
  /**
   * Default constructor - creates an identity matrix.
   */
  FMatrix4x4 ()
    : FBase ()
    {
      row[0].set (1.0, 0.0, 0.0, 0.0);
      row[1].set (0.0, 1.0, 0.0, 0.0);
      row[2].set (0.0, 0.0, 1.0, 0.0);
      row[3].set (0.0, 0.0, 0.0, 1.0);
    }
  
  /**
   * 1 argument constructor - from scalar, set all elements to given value
   */
  FMatrix4x4 (float scalar)
    : FBase ()
    {
      row[0] = scalar; row[1] = scalar; row[2] = scalar; row[3] = scalar;
    }
  
  /**
   * 4 argument constructor - from 4 FVector4s
   */
  FMatrix4x4 (const FVector4& r0, const FVector4& r1, const FVector4& r2, const FVector4& r3)
    : FBase ()
    {
      row[0] = r0; row[1] = r1; row[2] = r2; row[3] = r3;
    }
  
  /**
   * Copy constructor
   */
  FMatrix4x4 (const FMatrix4x4& mat)
    : FBase ()
    {
      copy_from (mat);
    }
  
  /**
   * Constructor from a 3x3 matrix
   */
  FMatrix4x4 (const FMatrix3x3& mat3)
    : FBase ()
    {
      copy_from (mat3);
    }
  
  /**
   * Destructor
   */
  virtual ~FMatrix4x4 ()
    {}
  
  /**
   * Assignment operator
   */
  FMatrix4x4& operator = (const FMatrix4x4& mat)
    {
      copy_from (mat);
      return (*this);
    }
  
  /**
   * Assignment from a FMatrix3x3
   */
  FMatrix4x4& operator = (const FMatrix3x3& mat3)
    {
      copy_from (mat3);
      return (*this);
    }
  
  /**
   * Assignment from a scalar
   */
  void operator = (float scalar)
    {
      row[0] = scalar; row[1] = scalar; row[2] = scalar; row[3] = scalar;
    }
  
  /**
   * Set the rows of the matrix to the given FVector4s
   */
  void set (const FVector4& r0, const FVector4& r1, const FVector4& r2, const FVector4& r3)
    {
      row[0] = r0; row[1] = r1; row[2] = r2; row[3] = r3;
    }
  
  void set (float scalar)
    {
      row[0] = scalar; row[1] = scalar; row[2] = scalar; row[3] = scalar;
    }
  
  /**
   * Reset the matrix to it's default state - identity
   */
  void reset (void)
    {
      row[0].set (1.0, 0.0, 0.0, 0.0);
      row[1].set (0.0, 1.0, 0.0, 0.0);
      row[2].set (0.0, 0.0, 1.0, 0.0);
      row[3].set (0.0, 0.0, 0.0, 1.0);
    }
  
  /** 
   * Make a copy of the object implement FBase class pure virtual function
   */
  virtual FBase::Ptr copy (void) const
    {
      Ptr mat = new FMatrix4x4 (*this);
      return mat;
    }
  
  /**
   * Access a row of the matrix - no range checks.
   */
  FVector4& operator [] (uint index)
    {
      return row[index];
    }
  
  /**
   * Access a row of the matrix - no range checks : const version
   */
  const FVector4& operator [] (uint index) const
    {
      return row[index];
    }
  
  /**
   * Class member functions which return the identity matrix
   */
  static FMatrix4x4 identity (void)
    {
      FMatrix4x4 imat;  // Default constructor creates an identity matrix
      return imat;
    }
  
  /**
   * Class member functions which return the identity matrix
   */
  static FMatrix4x4 I (void)
    {
      FMatrix4x4 imat;  // Default constructor creates an identity matrix
      return imat;
    }
  
  // Arithmetic assignment operators
  
  void operator += (const FMatrix4x4& mat)
    {
      for (int i=0; i < 4; ++i)
	row[i] += mat.row[i];
    }
  
  void operator -= (const FMatrix4x4& mat)
    {
      for (int i=0; i < 4; ++i)
	row[i] -= mat.row[i];
    }
  
  void operator *= (float scalar)
    {
      for (int i=0; i < 4; ++i)
	row[i] *= scalar;
    }
  
  void operator /= (float scalar)
    {
      for (int i=0; i < 4; ++i)
	row[i] /= scalar;
    }
  
  // Special functions - inverse, transpose, etc.
  
  /**
   * Find the 3x3 sub-matrix which is the co-factor for the given element
   */
  FMatrix3x3 cofactor(uint r, uint c) const
    {
      FMatrix3x3 cof;
      FVector3 cofrow;
      uint cfcol, cfrow;
      
      if ( (r > 3) || (c > 3) )
	{
	  cerr << "FMatrix3x3 FMatrix4x4 :: cofactor(int,int) : Index out of range!" << endl;
	  return cof;
	}
      
      cfrow = 0;
      for (uint i=0; i < 4; ++i)
	{
	  if ( i != r )
	    {
	      cfcol = 0;
	      for (uint j=0; j < 4; ++j)
		if ( j != c ) cofrow[cfcol++] = row[i][j];
	      cof[cfrow++] = cofrow;
	    }
	}
      return cof;
    }
  
  void transpose(void)
    {
      for (int i=0; i < 4; ++i)
	for (int j=i+1; j < 4; ++j)
	  swap(row[i][j],row[j][i]);
    }
  
  /**
   * Find the transpose of a given matrix.
   */
  friend FMatrix4x4 transpose(const FMatrix4x4& mat)
    {
      FMatrix4x4 trans (mat); trans.transpose ();
      return trans;
    }
  
  friend float determinant(const FMatrix4x4& mat)
    {
      float det = 0.0;
      
      for (int i=0; i < 3; ++i)
	det += mat[0][i] * cofsign (0, i) * determinant (mat.cofactor (0, i));
      return det;
    }
  
  /**
   * Invert the matrix. Using elementary row operations
   */
  void invert(void)
    {
      FVector4 inv[4];
      int i,j,swaprow;
      
      for (i=0; i < 4; ++i)
	inv[i][i] = 1.0;
      
      // inv will be identity initially and will become the inverse at the end
      for (i=0; i < 4; ++i)
	{
	  // i is column
	  // Find row in this column which has largest element (magnitude)
	  swaprow = i;
	  for (j=i+1; j < 4; ++j)
	    if ( fabs (row[j][i]) > fabs (row[i][i]) ) swaprow = j;
	  
	  if ( swaprow != i )
	    {
	      // Swap the two rows to get largest element in main diagonal
	      // Do this for the RHS also
	      swap (row[i], row[swaprow]); swap (inv[i], inv[swaprow]);
	    }
	  
	  // Check if pivot is non-zero
	  if ( !is_non_zero (row[i][i]) )
	    {
	      cerr << "FMatrix4x4 inverse(const FMatrix4x4&) : Singular matrix!" << endl;
	      // Return original matrix without change
	      return;
	    }
	  
	  // Divide matrix by main diagonal element to make it 1.0
	  float fact = row[i][i];
	  for (j=0; j < 4; ++j)
	    {
	      row[j] /= fact;
	      inv[j] /= fact;
	    }
	  
	  // Make non-main-diagonal elements in this column 0 using main-diagonal row
	  for (j=0; j < 4; ++j)
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
      for (i=0; i < 3; ++i)
	{
	  float pivot = row[i][i];
	  row[i] /= pivot;
	  inv[i] /= pivot;
	}
      for (i=0; i < 4; ++i)
	row[i] = inv[i];
    }
  
  /** 
   * Find the inverse of a given matrix using elementary row operations
   */
  friend FMatrix4x4 invert (const FMatrix4x4& mat)
    {
      FMatrix4x4 inv (mat); inv.invert ();
      return inv;
    }
  
  // Arithmetic operators, implemented as friend functions
  
  /**
   * Negation
   */
  friend FMatrix4x4 operator - (const FMatrix4x4& mat)
    {
      FMatrix4x4 neg;
      
      for (int i=0; i < 4; ++i)
	neg.row[i] = -mat.row[i];
      
      return neg;
    }
  
  friend FMatrix4x4 operator + (const FMatrix4x4& mat1, const FMatrix4x4& mat2)
    {
      FMatrix4x4 sum (mat1);
      sum += mat2;
      return sum;
    }
  
  friend FMatrix4x4 operator - (const FMatrix4x4& mat1, const FMatrix4x4& mat2)
    {
      FMatrix4x4 diff (mat1);
      diff -= mat2;
      return diff;
    }
  
  /**
   * Post-multiplication by a scalar.
   */
  friend FMatrix4x4 operator * (const FMatrix4x4& mat, float scalar)
    {
      FMatrix4x4 prod (mat);
      prod *= scalar;
      return prod;
    }
  
  /**
   * Pre-multiplication by a scalar.
   */
  friend FMatrix4x4 operator * (float scalar, const FMatrix4x4& mat)
    {
      FMatrix4x4 prod (mat);
      prod *= scalar;
      return prod;
    }
  
  /**
   * Division by a scalar.
   */
  friend FMatrix4x4 operator / (const FMatrix4x4& mat, float scalar)
    {
      FMatrix4x4 quot (mat);
      quot /= scalar;
      return quot;
    }
  
  /**
   * Multiplication of 2 matrices - outer product.
   */
  friend FMatrix4x4 operator * (const FMatrix4x4& mat1, const FMatrix4x4& mat2);
  
  /**
   * Element-by-element multiplication of 2 matrices.
   */
  friend FMatrix4x4 product (const FMatrix4x4& mat1, const FMatrix4x4& mat2)
    {
      FMatrix4x4 prod;
      
      prod.row[0] = product (mat1[0], mat2[0]);
      prod.row[1] = product (mat1[1], mat2[1]);
      prod.row[2] = product (mat1[2], mat2[2]);
      prod.row[3] = product (mat1[3], mat2[3]);
      
      return prod;
    }
  
  /**
   * Post-multiplication by a FVector4. Vector is assumed to be a column vector.
   */
  friend FVector4 operator * (const FMatrix4x4& mat, const FVector4& vec)
    {
      FVector4 prod;
      
      prod.set (mat.row[0]*vec, 
		mat.row[1]*vec, 
		mat.row[2]*vec, 
		mat.row[3]*vec);
      return prod;
    }
  
  /**
   * Pre-multiplication by a FVector4. Vector is assumed to be a row vector.
   */
  friend FVector4 operator * (const FVector4& vec, const FMatrix4x4& mat);
  
  /**
   * Multiplication of two FVector4s to produce a FMatrix4x4 - outer product
   * or tensor product of two Vectors.  
   * Same as multiplying row vector (v1) with column vector (v2)
   */
  friend FMatrix4x4 operator ^ (const FVector4& v1, const FVector4& v2)
    {
      FMatrix4x4 prod;
      
      prod.row[0] = v1[0]*v2;
      prod.row[1] = v1[1]*v2;
      prod.row[2] = v1[2]*v2;
      prod.row[3] = v1[3]*v2;
      
      return prod;
    }
  
  friend void swap (FMatrix4x4& mat1, FMatrix4x4& mat2)
    {
      for (int i=0; i < 4; ++i)
	swap (mat1.row[i], mat2.row[i]);
    }
  
  /** 
   * Multiply self with another matrix. Simply calls above defined friend 
   * function.
   */
  void operator *= (const FMatrix4x4& mat)
    {
      FMatrix4x4 prod = (*this) * mat;
      *this = prod;
    }

  /**
   * Fill an array with contents of the matrix
   * Row - major form -> Row 1 == { array[0], array[1], array[2], array[3] }
   */
  void fill_array_row_major (float array[16]) const
    {
      int index=0;
      for (int i=0; i < 4; ++i)
	{
	  row[i].get (array[index],
		      array[index+1],
		      array[index+2],
		      array[index+3]);
	  index += 4;
	}
    }

  /**
   * Fill an array with contents of the matrix
   * Row - major form -> Column 1 == { array[0], array[1], array[2], array[3] }  
   */
  void fill_array_column_major (float array[16]) const
    {
      int index=0;
      for (int i=0; i < 4; ++i)
	{
	  row[i].get (array[index], 
		      array[index+4], 
		      array[index+8], 
		      array[index+12]);
	  index += 1;
	}
    }
  
  friend ostream& operator << (ostream& o, const FMatrix4x4& mat)
    {
      o << "[ " << mat.row[0] << "," << endl
	<< "  " << mat.row[1] << "," << endl
	<< "  " << mat.row[2] << "," << endl
	<< "  " << mat.row[3] << " ]" << endl;
      return o;
    }
  
  friend istream& operator >> (istream& i, FMatrix4x4& mat)
    {
      char rowsep, dummy;
      i >> dummy >> mat.row[0] >> rowsep
	>> mat.row[1] >> rowsep
	>> mat.row[2] >> rowsep
	>> mat.row[3] >> dummy;
      return i;
    }
protected:
  
  FVector4   row[4];                                // 4 rows of the matrix
  
  /**
   * Copy values from another matrix
   */
  void copy_from (const FMatrix4x4& mat)
    {
      for (int i=0; i < 4; ++i)
	row[i] = mat.row[i];
    }
  
  /**
   * Copy from a 3x3 matrix. 4th row and column elements are all set to 0
   */
  void copy_from (const FMatrix3x3& mat3)
    {
      for (int i=0; i < 3; ++i)
	row[i] = mat3[i];
    }
};

#endif // #ifndef FMatrix_4x4_H_

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

#ifndef _FVECTOR4_H_
#define _FVECTOR4_H_

#include <Flek/FVector.H>

/** @package libflek_core
 * FVector4 is a 4 dimensional vector represented internally as an array of 
 * floats.  This class is related to FVector2 and FVector3 which are 
 * 2-D and 3-D versions of this class.  All FVector classes are forward
 * declared in FVector.h.
 */
class FVector4 : public FBase
{

public:

  typedef FVector4* Ptr;
  
  /**
   * The default constructor sets each element in the vector to 0.
   */
  FVector4 ()
    : FBase ()
    {
      elem[0] = elem[1] = elem[2] = elem[3] = 0.0;
    }
  
  /**
   * This one argument constructor intializes all elements in the vector
   * with the given value.
   */
  FVector4 (float val)
    : FBase ()
    {
      elem[0] = elem[1] = elem[2] = elem[3] = val;
    }
  
  /**
   * This one argument constructor initializes the vector with the first
   * three elements in the given array.
   */
  FVector4 (float * arr)
    : FBase ()
    {
      elem[0] = arr[0]; elem[1] = arr[1]; elem[2] = arr[2]; elem[3] = arr[3];
    }
  
  /**
   * This four argument constructor initializes the vector with the
   * passed values.
   */
  FVector4 (float val1, float val2, float val3, float val4)
    : FBase ()
    {
      elem[0] = val1; elem[1] = val2; elem[2] = val3; elem[3] = val4;
    }
  
  /**
   * The copy constructor initializes this vector with the contents 
   * of another vector.
   */
  FVector4 (const FVector4& vec)
    : FBase (vec)
    {
      elem[0] = vec.elem[0]; elem[1] = vec.elem[1];
      elem[2] = vec.elem[2]; elem[3] = vec.elem[3];
    }
  
  /**
   * This constructor initializes the vector from the contents of a
   * FVector2 (a 2D vector).  The third and forth elements are set to zero.
   */
  FVector4 (const FVector2& vec)
    : FBase ()
    {
      copy_from (vec);
    }
  
  /**
   * This constructor initializes the vector from the contents of a
   * FVector2 (a 2D vector).  The third element is set to zero.
   */
  FVector4 (const FVector3& vec)
    : FBase ()
    {
      copy_from (vec);
    }
  
  /**
   * The virtual destructor does nothing.
   */
  virtual ~FVector4 ()
    {}

  /**
   * Assignment operator from another FVector3.
   */
  FVector4& operator = (const FVector4& vec)
    {
      elem[0] = vec.elem[0]; elem[1] = vec.elem[1];
      elem[2] = vec.elem[2]; elem[3] = vec.elem[3];
      return (*this);
    }
  
  /** 
   * Assignment operator from a scalar.  All elements are set to 
   * the scalar value.
   */
  FVector4& operator = (float scalar)
    {
      elem[0] = elem[1] = elem[2] = elem[3] = scalar;
      return (*this);
    }
  
  /** 
   * Assignment operator from a FVector2.  The third and forth elements 
   * are set to 0.
   */
  FVector4& operator = (const FVector2& vec)
    {
      copy_from (vec);
      return (*this);
    }
  
  /** 
   * Assignment operator from a FVector3.  The forth element is set to 0.
   */
  FVector4& operator = (const FVector3& vec)
    {
      copy_from (vec);
      return (*this);
    }
  
  /**
   * Make a copy of the object.
   */
  virtual FBase::Ptr copy (void) const
    {
      Ptr vec = new FVector4 (*this);
      return vec;
    }
  
  /** 
   * Set each element vector to the given values.
   */
  void set (float v1, float v2, float v3, float v4)
    {
      elem[0] = v1; elem[1] = v2; elem[2] = v3; elem[3] = v4;
    }
  
  /** 
   * Set each element vector to the given value.
   */
  void set (float val)
    {
      elem[0] = elem[1] = elem[2] = elem[3] = val;
    }
  
  /** 
   * Set elements of vector to default values.
   */
  void reset (void)
    {
      set (0.0);
    }
  
  /** 
   * Get the elements of vector into given values.
   */
  void get (float& v1, float& v2, float& v3, float& v4) const
    {
      v1 = elem[0]; v2 = elem[1]; v3 = elem[2]; v4 = elem[3];
    }
  
  /**
   * Fill an array with the elements of the vector.
   */
  void fill_array (float arr[4]) const
    {
      arr[0] = elem[0]; arr[1] = elem[1]; arr[2] = elem[2]; arr[3] = elem[3];
    }
  
  /** 
   * Element access operator.  For efficiency, this doesn't check for 
   * valid indices
   */
  float& operator [] (uint index)
    {
      return elem[index];
    }
  
  /** 
   * Element access operator.  For efficiency, this doesn't check for 
   * valid indices
   */
  float operator [] (uint index) const
    {
      return elem[index];
    }
  
  /**
   * Arithmetic operator for additive assignment.
   */
  void operator += (const FVector4& vec)
    {
      elem[0] += vec.elem[0]; elem[1] += vec.elem[1];
      elem[2] += vec.elem[2]; elem[2] += vec.elem[2];
    }
  
  /**
   * Arithmetic operator for subtractive assignment.
   */
  void operator -= (const FVector4& vec)
    {
      elem[0] -= vec.elem[0]; elem[1] -= vec.elem[1];
      elem[2] -= vec.elem[2]; elem[3] -= vec.elem[3];
    }
  
  /**
   * Arithmetic operator for multiplicative (scalar) assignment.
   */
  void operator *= (float scalar)
    {
      elem[0] *= scalar; elem[1] *= scalar; elem[2] *= scalar; elem[3] *= scalar;
    }
  
  /**
   * Arithmetic operator for divisive (scalar) assignment.
   */
  void operator /= (float scalar)
    {
      elem[0] /= scalar; elem[1] /= scalar; elem[2] /= scalar; elem[3] /= scalar;
    }
  
  /**
   * Arithmetic operator for addition.
   */      
  FVector4 operator + (const FVector4& vec) const
    {
      FVector4 sum(*this);
      sum += vec;
      return sum;
    }
  
  /**
   * Arithmetic operator for subtraction.
   */ 
  FVector4 operator - (const FVector4& vec) const
    {
      FVector4 diff(*this);
      diff -= vec;
      return diff;
    }
  
  /**
   * Operator for scalar multiplication (dot product).
   */
  float operator * (const FVector4& vec) const
    {
      float dotprod = elem[0]*vec.elem[0] + elem[1]*vec.elem[1]
	+ elem[2]*vec.elem[2] + elem[3]*vec.elem[3];
      return dotprod;
    }
  
  /**
   * Friend operator for negation.
   */
  friend FVector4 operator - (const FVector4& vec)
    {
      FVector4 negv (-vec.elem[0], -vec.elem[1], -vec.elem[2], -vec.elem[3]);
      return negv;
    }
  
  /**
   * Friend operator for scalar pre-multiplication.
   */
  friend FVector4 operator * (float scalar, const FVector4& vec)
    {
      FVector4 prod (vec);
      
      prod *= scalar;
      return prod;
    }
  
  /**
   * Friend operator for scalar post-multiplication.
   */  
  friend FVector4 operator * (const FVector4& vec, float scalar)
    {
      FVector4 prod (vec);
      
      prod *= scalar;
      return prod;
    }
  
  /**
   * Friend operator for scalar division.
   */
  friend FVector4 operator / (const FVector4& vec, float scalar)
    {
      FVector4 prod (vec);
      
      prod /= scalar;
      return prod;
    }
  
  /**
   * Friend operator for element-by-element product.
   */
  friend FVector4 product(const FVector4& vec1, const FVector4& vec2)
    {
      FVector4 prod (vec1[0]*vec2[0], vec1[1]*vec2[1], vec1[2]*vec2[2], vec1[3]*vec2[3]);
      
      return prod;
    }
  
  /**
   * Boolean equality operator.
   */     
  bool operator == (const FVector4& vec) const
    {
      if ( (fabs(elem[0]-vec.elem[0]) > ZERO) ||
	   (fabs(elem[1]-vec.elem[1]) > ZERO) ||
	   (fabs(elem[2]-vec.elem[2]) > ZERO) ||
	   (fabs(elem[3]-vec.elem[3]) > ZERO) )
	return false;
      return true;
    }
  
  /**
   * Boolean ineqality operator.
   */
  bool operator != (const FVector4& vec) const
    {
      return !( (*this) == vec );
    }
  
  // Other functions
  
  /**
   * Square of the norm of the vector.
   */     
  friend float normsqr (const FVector4& vec)
    {
      float nsq = sqr (vec.elem[0]) + sqr (vec.elem[1]) + sqr (vec.elem[2]) + sqr (vec.elem[3]);
      return nsq;
    }
  
  /**
   * Norm of the vector.
   */
  friend float norm (const FVector4& vec)
    {
      return sqrt (normsqr (vec));
    }

  /** 
   * Length (norm) of the vector.
   */
  float length ()
    {
      return norm (*this);
    }

  /**
   * Normalize.  Returns previous norm.
   */
  friend float normalize(FVector4& vec)
    {
      float n = norm(vec);
      if ( is_non_zero (n) == true ) vec /= n;
      return n;
    }
  
  /** 
   * Returns normalized vector.
   */
  friend FVector4 normalized (const FVector4& vec)
    {
      FVector4 nvec (vec);
      normalize (nvec);
      return nvec;
    }
  
  /**
   * Swap the elements of two FVector4s.
   */     
  friend void swap(FVector4& vec1, FVector4& vec2)
    {
      swap (vec1.elem[0], vec2.elem[0]);
      swap (vec1.elem[1], vec2.elem[1]);
      swap (vec1.elem[2], vec2.elem[2]);
      swap (vec1.elem[3], vec2.elem[3]);
    }
  
  /**
   * I/O Stream extraction operator.  Of the form "[ x y z ]". 
   */ 
  friend istream& operator >> (istream& i, FVector4& vec)
    {
      remove_white_space(i);
      if ( i.peek() == '[' )
	{
	  // Correct format
	  int numread = 0;  // No. of elements read
	  float val;
	  char c;
	  
	  i >> c;  // Read opening square bracket
	  while ( numread < 4 )
	    {
	      i >> val; vec.elem[numread] = val;
	      numread++;
	    }
	  
	  // Read all characters till closing bracket is found
	  // If no. of chars read is more than 10 then print an error
	  // message and exit;
	  i >> c;
	  numread = 1;
	  while ( c != ']' && numread < 10 )
	    {
	      i >> c; numread++;
	    }
	  
	  if ( numread >= 10 )
	    {
	      cerr << "operator >> FVector4 : Incorrect format. Closing ']' not"
		   << " found upto 10 characters after 4th element of vector"
		   << endl;
	      exit(0);
	    }
	}
      
      return i;
    }
  
  /**
   * I/O Stream insertion operator.  Of the form "[ x y z ]". 
   */ 
  friend ostream& operator << (ostream& o, const FVector4& vec)
    {
      // User can set precision from 0 to 6
      o << setiosflags (ios::fixed) << setiosflags (ios::showpoint);
      
      int oldprec = o.precision ();
      if ( oldprec < 0 ) o << setprecision(0);
      if ( oldprec > 6 ) o << setprecision(6);
      
      o << "["
	<< vec.elem[0] << " "
	<< vec.elem[1] << " "
	<< vec.elem[2] << " "
	<< vec.elem[3]
	<< "]";
      
      o << setprecision (oldprec);
      return o;
    }

protected:
  
  float elem[4];
  
  /**
   * Initialize the elements from a FVector2,
   */
  void copy_from (const FVector2& vec);
  
  /**
   * Initialize the elements from a FVector4,
   */
  void copy_from (const FVector3& vec);
  
};

#endif // #ifndef FVector_4_H


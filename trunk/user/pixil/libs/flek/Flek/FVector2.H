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

#ifndef _FVECTOR2_H_
#define _FVECTOR2_H_

#include <Flek/FVector.H>

/** @package libflek_core
 * fVector2 is a 2 dimensional vector represented internally as an array of 
 * floats.  This class is related to fVector3 and fVector4 which are 
 * 3-D and 4-D versions of this class.  All fVector classes are forward
 * declared in fVector.h.
 */

class FVector2 : public FBase
{
  
public:

  typedef FVector2* Ptr;
  
  /**
   * The default constructor sets each element in the vector to 0.
   */
  FVector2 ()
    : FBase ()
    {
      elem[0] = elem[1] = 0.0;
    }
  
  /**
   * This one argument constructor intializes all elements in the vector
   * with the given value.
   */
  FVector2 (float val)
    : FBase ()
    {
      elem[0] = elem[1] = val;
    }

  /**
   * This one argument constructor initializes the vector with the first
   * three elements in the given array.
   */     
  FVector2 (float * arr)
    : FBase ()
    {
      elem[0] = arr[0]; elem[1] = arr[1];
    }
  
  /**
   * This two argument constructor initializes the vector with the
   * passed values.
   */
  FVector2 (float val1, float val2)
    : FBase ()
    {
      elem[0] = val1; elem[1] = val2;
    }

  /**
   * The copy constructor initializes this vector with the contents 
   * of another vector.
   */     
  FVector2 (const FVector2& vec)
    : FBase (vec)
    {
      elem[0] = vec.elem[0]; elem[1] = vec.elem[1];
    }

  /**
   * This constructor initializes the vector from the contents of a
   * FVector3.  The third element in the FVector3 is ignored.
   */
  FVector2 (const FVector3& vec)
    : FBase ()
    {
      copy_from (vec);
    }
     
  /**
   * This constructor initializes the vector from the contents of a
   * FVector4.  The third element in the FVector4 is ignored.
   */
  FVector2 (const FVector4& vec)
    : FBase ()
    {
      copy_from (vec);
    }

  /**
   * The virtual destructor does nothing.
   */
  virtual ~FVector2 ()
    {}

  /**
   * Assignment operator from another FVector2.
   */
  FVector2& operator = (const FVector2& vec)
    {
      elem[0] = vec.elem[0]; elem[1] = vec.elem[1];
      return (*this);
    }

  /** 
   * Assignment operator from a scalar.  All elements are set to 
   * the scalar value.
   */
  FVector2& operator = (float scalar)
    {
      elem[0] = elem[1] = scalar;
      return (*this);
    }

  /**
   * Assignment operator from a FVector3.  Copies first 2 elements.
   */
  FVector2& operator = (const FVector3& vec)
    {
      copy_from (vec);
      return (*this);
    }

  /**
   * Assignment operator from a FVector4.  Copies first 2 elements.
   */
  FVector2& operator = (const FVector4& vec)
    {
      copy_from (vec);
      return (*this);
    }

  /**
   * Make a copy of the object.
   */
  virtual FBase::Ptr copy (void) const
    {
      Ptr vec = new FVector2 (*this);
      return vec;
    }

  /** 
   * Set each element vector to the given values.
   */
  void set (float v1, float v2)
    {
      elem[0] = v1; elem[1] = v2;
    }
  
  /** 
   * Set each element vector to the given value.
   */
  void set (float val)
    {
      elem[0] = elem[1] = val;
    }
     
  /** 
   * Set elements of vector to default values.
   */
  void reset (void)
    {
      set(0.0);
    }

  /** 
   * Get the elements of vector into given values.
   */
  void get (float& v1, float& v2) const
    {
      v1 = elem[0]; v2 = elem[1];
    }

  /**
   * Fill an array with the elements of the vector.
   */
  void fill_array (float arr[2]) const
    {
      arr[0] = elem[0]; arr[1] = elem[1];
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
  void operator += (const FVector2& vec)
    {
      elem[0] += vec.elem[0]; elem[1] += vec.elem[1];
    }

  /**
   * Arithmetic operator for subtractive assignment.
   */
  void operator -= (const FVector2& vec) 
    {
      elem[0] -= vec.elem[0]; elem[1] -= vec.elem[1];
    }
  
  /**
   * Arithmetic operator for multiplicative (scalar) assignment.
   */
  void operator *= (float scalar)
    {
      elem[0] *= scalar; elem[1] *= scalar;
    }
    
  /**
   * Arithmetic operator for divisive (scalar) assignment.
   */ 
  void operator /= (float scalar)
    {
      elem[0] /= scalar; elem[1] /= scalar;
    }
    
  /**
   * Arithmetic operator for addition.
   */
  FVector2 operator + (const FVector2& vec) const
    {
      FVector2 sum(*this);
      sum += vec;
      return sum;
    }
  
  /**
   * Arithmetic operator for subtraction.
   */ 
  FVector2 operator - (const FVector2& vec) const
    {
      FVector2 diff(*this);
      diff -= vec;
      return diff;
    }
  
  /**
   * Operator for scalar multiplication (dot product).
   */
  float operator * (const FVector2& vec) const
    {
      float dotprod = elem[0]*vec.elem[0] + elem[1]*vec.elem[1];
      return dotprod;
    }

  /**
   * Friend operator for negation.
   */
  friend FVector2 operator - (const FVector2& vec)
    {
      FVector2 negv (-vec.elem[0], -vec.elem[1]);
      return negv;
    }
  
  /**
   * Friend operator for scalar pre-multiplication.
   */
  friend FVector2 operator * (float scalar, const FVector2& vec)
    {
      FVector2 prod (vec);
      
      prod *= scalar;
      return prod;
    }

  /**
   * Friend operator for scalar post-multiplication.
   */  
  friend FVector2 operator * (const FVector2& vec, float scalar)
    {
      FVector2 prod (vec);
      
      prod *= scalar;
      return prod;
    }

  /**
   * Friend operator for scalar division.
   */
  friend FVector2 operator / (const FVector2& vec, float scalar)
    {
      FVector2 prod (vec);
      
      prod /= scalar;
      return prod;
    }

  /**
   * Friend operator for element-by-element product.
   */ 
  friend FVector2 product(const FVector2& vec1, const FVector2& vec2)
    {
      FVector2 prod(vec1[0]*vec2[0],vec1[1]*vec2[1]);
      
      return prod;
    }
     
  /**
   * Boolean equality operator.
   */       
  bool operator == (const FVector2& vec) const
    {
      if ( (fabs(elem[0]-vec.elem[0]) > ZERO) ||
	   (fabs(elem[1]-vec.elem[1]) > ZERO) )
	return false;
      return true;
    }

  /**
   * Boolean ineqality operator.
   */
  bool operator != (const FVector2& vec) const
    {
      return !( (*this) == vec );
    }

  /**
   * Square of the norm of the vector.
   */ 
  friend float normsqr(const FVector2& vec)
    {
      float nsq = sqr (vec.elem[0]) + sqr (vec.elem[1]);
      return nsq;
    }
     
  /**
   * Norm of the vector.
   */
  friend float norm (const FVector2& vec)
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
  friend float normalize (FVector2& vec)
    {
      float n = norm (vec);
      if ( is_non_zero (n) == true ) vec /= n;
      return n;
    }

  /** 
   * Returns normalized vector.
   */
  friend FVector2 normalized (const FVector2& vec)
    {
      FVector2 nvec (vec);
      normalize (nvec);
      return nvec;
    }

  /**
   * Swap the elements of two FVector2s.
   */     
  friend void swap (FVector2& vec1, FVector2& vec2)
    {
      swap (vec1.elem[0], vec2.elem[0]);
      swap (vec1.elem[1], vec2.elem[1]);
    }

  /**
   * I/O Stream extraction operator.  Of the form "[ x y ]". 
   */ 
  friend istream& operator >> (istream& i, FVector2& vec)
    {
      remove_white_space (i);
      if ( i.peek() == '[' )
	{
	  // Correct format
	  int numread = 0;  // No. of elements read
	  float val;
	  char c;
	  
	  i >> c;  // Read opening square bracket
	  while ( numread < 2 )
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
	      cerr << "operator >> FVector2 : Incorrect format. Closing ']' not"
		   << " found upto 10 characters after 2nd element of vector"
		   << endl;
	      exit(0);
	    }
	}
      
      return i;
    }

  /**
   * I/O Stream insertion operator.  Of the form "[ x y z ]". 
   */ 
  friend ostream& operator << (ostream& o, const FVector2& vec)
    {
      // User can set precision from 0 to 6
      o << setiosflags(ios::fixed) << setiosflags (ios::showpoint);
      
      int oldprec = o.precision ();
      if ( oldprec < 0 ) o << setprecision(0);
      if ( oldprec > 6 ) o << setprecision(6);
      
      o << "["
	<< vec.elem[0] << " "
	<< vec.elem[1]
	<< "]";
      
      o << setprecision (oldprec);
      return o;
    }
  
protected:
  
  float elem[2];

  /**
   * Initialize the elements from a FVector3,
   */
  void copy_from (const FVector3& vec);

  /**
   * Initialize the elements from a FVector4,
   */
  void copy_from (const FVector4& vec);

};

#endif // #ifndef FVector2_H_


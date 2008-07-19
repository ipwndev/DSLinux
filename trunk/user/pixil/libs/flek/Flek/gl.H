/* -*-c++-*- 

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

#ifndef _FGL_H_
#define _FGL_H_

#include <Flek/FVector2.H>
#include <Flek/FVector3.H>
#include <Flek/FVector4.H>
#include <Flek/FMatrix4x4.H>
#include <Flek/FMatrix3x3.H>
#include <Flek/FArcball_Control.H>
#include <Flek/FImage.H>

#include <GL/gl.h>
#include <FL/gl.h>

/** @package libflek_gl
 * Multiply the current GL matrix by the fMatrix M. 
 * This function is a wrapper around the GL function,
 * glMultMatrixd.
 */
void glMultMatrix (const FMatrix4x4& M);

/** 
 * Multiply the current GL matrix by an arcball rotation 
 * matrix.
 */
inline void glMultMatrix (const FArcball_Control& a)
{
  glMultMatrix (a.value ());
}

/**
 * glVertex is used within glBegin - glEnd pairs to specify point, 
 * line, and polygon vertices.  This form takes an fVector4 as
 * it's argument.
 */
inline void glVertex (const FVector4& v)
{
  glVertex4d (v[0], v[1], v[2], v[3]);
}

/**
 * This form of glVertex takes an fVector3 as it's argument.
 */
inline void glVertex (const FVector3& v)
{
  glVertex3d (v[0], v[1], v[2]);
}

/**
 * This form of glVertex takes an fVector2 as it's argument.
 */
inline void glVertex (const FVector2& v)
{
  glVertex2d (v[0], v[1]);
}

/**
 * This form of glVertex takes 4 floats as it's arguments.
 */
inline void glVertex (const float& x, const float& y, const float& z, const float& w)
{
  glVertex4d (x, y, z, w);
}

/**
 * This form of glVertex takes 3 floats as it's arguments.
 */
inline void glVertex (const float& x, const float& y, const float& z)
{
  glVertex3d (x, y, z);
}

/**
 * This form of glVertex takes 2 floats as it's arguments.
 */
inline void glVertex (const float& x, const float& y)
{
  glVertex2d (x, y);
}

/**
 * glColor is used to specify the current color. 
 * This form takes an fVector4 as it's argument (RGBA).
 */
inline void glColor (const FVector4& v)
{
  glColor4d (v[0], v[1], v[2], v[3]);
}

/**
 * This form takes an fVector3 as it's argument (RGB).
 */
inline void glColor (const FVector3& v)
{
  glColor3d (v[0], v[1], v[2]);
}

/**
 * This form takes 4 floats as it's arguments (RGBA).
 */
inline void glColor (const float& x, const float& y, const float& z, const float& w)
{
  glColor4d (x, y, z, w);
}

/**
 * This form takes 3 floats as it's arguments (RGB).
 */
inline void glColor (const float& x, const float& y, const float& z)
{
  glColor3d (x, y, z);
}

/**
 * glNormal is used to specify the current normal. 
 * This form takes an fVector4 as it's argument.
 * It ignores the w, or 4th term in the fVector4.
 */
inline void glNormal (const FVector4& v)
{
  glNormal3d (v[0], v[1], v[2]);
}

/**
 * This form takes an fVector3 as it's argument.
 */
inline void glNormal (const FVector3& v)
{
  glNormal3d (v[0], v[1], v[2]);
}

/**
 * This form takes an fVector2 and a z value as it's arguments.
 */
inline void glNormal (const FVector2& v, const float &z)
{
  glNormal3d (v[0], v[1], z);
}

/**
 * This form takes 3 floats as it's arguments.
 */
inline void glNormal (const float& x, const float& y, const float& z)
{
  glNormal3d (x, y, z);
}

/**
 * glScale scales the current transforamtion matrix by
 * an x, y and z scale component.
 * This form takes an fVector3 as it's argument.
 */
inline void glScale (const FVector3& v)
{
  glScaled (v[0], v[1], v[2]);
}

/**
 * This form takes 3 floats as it's argument.
 */
inline void glScale (const float &x, const float &y=1, const float &z=1)
{
  glScaled (x, y, z);
}

/**
 * glTranslate translates the current transforamtion matrix by
 * an x, y and z translation component.
 * This form takes an fVector3 as it's argument.
 */
inline void glTranslate (const FVector3& v)
{
  glTranslated (v[0], v[1], v[2]);
}

/**
 * This form takes 3 floats as it's arguments.
 */
inline void glTranslate (const float &x, const float &y=0, const float &z=0)
{
  glTranslated (x, y, z);
}

/**
 * glRotate rotates the current transformation matrix by an angle a about an axis v.
 */
inline void glRotate (const float &a, const FVector3 &v)
{
  glRotated (a, v[0], v[1], v[2]);
}

/**
 * glRotate rotates the current transformation matrix by an angle a about an axis (x, y, z).
 */
inline void glRotate (const float &a, const float &x, const float &y, const float &z)
{
  glRotated (a, x, y, z);
}

/**
 * This function loads an image for texturing in OpenGL.
 */
inline void glTexImage2D (FImage* img, GLenum target=GL_TEXTURE_2D, GLint level=0)
{
  glTexImage2D (target,
                level,
                4,
                img->width (),
                img->height (),
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                (GLvoid *)(*img->begin()) );
}

#endif

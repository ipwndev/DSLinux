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

#ifndef _FL_GL_ARCBALL_WINDOW_H_
#define _FL_GL_ARCBALL_WINDOW_H_


// Define macros for FLTK mouse buttons.
#define FL_LEFT_MOUSE 1
#define FL_MIDDLE_MOUSE 2
#define FL_RIGHT_MOUSE 3

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/gl.h>
#include <GL/glu.h>
#include <Flek/FTransformation.H>
#include <Flek/FArcball_Control.H>
#include <Flek/FTrans_Control.H>
#include <Flek/FZoom_Control.H>
#include <Flek/FDolly_Control.H>
#include <stdio.h>

enum TransformType { None=0, Pan=1, Rotate=2, Zoom=3, Dolly=4 };
enum ProjectionType { Orthographic=0, Perspective=1 };

/**
 * @package libflek_gl
 * 
 * Class for a FLTK GL Arcball window, which handles arcball rotations by default
 */
class Fl_Gl_Arcball_Window : public Fl_Gl_Window {
  
protected:
  
  FArcball_Control arcball;                                  // The arcball controller
  FTrans_Control trcontrol;                           // Translation controller
  FZoom_Control zoomcontrol;                          // Zoom controller
  FDolly_Control dollycontrol;                        // Dolly controller (10.0 scale)
  
  FTransformation transform;                         // Combined transformation
  TransformType currenttr;                          // Current transformation
  
  // View/projection parameters
  ProjectionType projtype;                          // Ortho/Perspective projection
  
  // For perspective projection
  float fovy;                                      // Field-of-view in degrees in y
  float aspect;                                    // Ratio of width to height
  float near;                                      // Near clipping plane
  float far;                                       // Far clipping plane
  
  // For orthographic projection
  float umin,umax,vmin,vmax;                       // Viewing frustum
  
  float cenx, ceny, cenz;                          // Center point for gluLookAt
  float eyex, eyey, eyez;                          // Eye point for gluLookAt
  float upx, upy, upz;                             // Up vector for gluLookAt
  float dist;                                      // Dist from eye pt to center pt
  
  FVector3 near_color_;
  FVector3 far_color_;
  FVector3 drag_color_;
  FVector3 rim_color_;
  
public:
  
  Fl_Gl_Arcball_Window(int x, int y, int w, int h, const char * l = NULL)
    : Fl_Gl_Window(x,y,w,h,l),
      arcball(FVector3(0,0,0),1.0), trcontrol(), zoomcontrol(), dollycontrol(10.0),
      transform(), currenttr(None),
      fovy(45.0), aspect(float(w)/float(h)), near(1.0), far(1000.0),
      umin(-1.0), umax(1.0), vmin(-1.0), vmax(1.0),
      cenx(0), ceny(0), cenz(0), eyex(0), eyey(0), eyez(50.0), upx(0), upy(1), upz(0)
  {}
  
  // Derived classes can override this
  virtual void reshape(void)
  {
    // This function should be called by the draw() function when the view
    // needs to be setup
    
    // This function sets up the view to be a perspective projection
    // using the view/projection parameters
    // Derived classes can override this function to do an orthographic projection
    // or they can change the view parameters and then call this function
    // If a perspective projection is set up using glFrustum instead of gluPerspective,
    // the fovy and aspect should be calculated and set correctly
    glViewport(0,0,w(),h());
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if ( projtype == Perspective ) 
      gluPerspective(fovy,aspect,near,far);
    else if ( projtype == Orthographic)
      glOrtho(umin,umax,vmin,vmax,near,far);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eyex,eyey,eyez,cenx,ceny,cenz,upx,upy,upz);
    
    dist = sqrt( sqr(cenx-eyex) + sqr(ceny-eyey) + sqr(cenz-eyez) );
  }
  
  // Handle rotation by mouse. Will do the rotation irrespective of which mouse
  // button was used and irrespective of other modifiers. Does not call
  // redraw or return any value. Will always update the controller. So this
  // function should be called only when rotation is required
  virtual void handle_rotate(int event) {
    // Don't do anything if we aren't in a neutral state or aren't rotating
    if ( currenttr != None && currenttr != Rotate ) return;
    
    float x,y;
    
    currenttr = Rotate;
    
    // Transform coords to lie between -1 to 1
    x = ( float(Fl::event_x() << 1) / w() ) - 1.0;
    y = ( -float(Fl::event_y() << 1) / h() ) + 1.0;
    
    arcball.mouse(x,y); arcball.update();
    
    if ( event == FL_PUSH ) arcball.begin_drag();
    else if ( event == FL_RELEASE ) {
      arcball.end_drag();
      transform.rotate(arcball.quaternion_value());  // Update the combined transformation
      arcball.reset(); currenttr = None;
    }
  }
  
  // Handle panning by mouse. Will do the panning irrespective of which mouse
  // button was used and irrespective of other modifiers. Does not call
  // redraw or return any value. Will always update the controller. So this
  // function should be called only when panning is required
  virtual void handle_pan(int event) {
    // Don't do anything if we aren't in a neutral state or aren't panning
    if ( currenttr != None && currenttr != Pan ) return;
    
    float x, y;
    float tx=0, ty=0;
    
    currenttr = Pan;
    
    // Transform coords to lie between -1 to 1
    x = ( float(Fl::event_x() << 1) / w() ) - 1.0;
    y = ( -float(Fl::event_y() << 1) / h() ) + 1.0;
    
    // Adjust the x and y values so that moving mouse by 1 pixel
    // on screen moves point under mouse by 1 pixel
    if ( projtype == Orthographic ) {
      // Orthographic projection
      tx = umin*(1.0-x)*0.5 + umax*(1.0+x)*0.5;
      ty = vmin*(1.0-y)*0.5 + vmax*(1.0+y)*0.5;
    }
    else if ( projtype == Perspective ) {
      // Perspective projection
      tx = x*dist*tan(deg2rad(fovy*aspect*0.5));
      ty = y*dist*tan(deg2rad(fovy*0.5));
    }
    trcontrol.mouse(tx,ty); trcontrol.update();
    
    if ( event == FL_PUSH ) trcontrol.begin_drag();
    else if ( event == FL_RELEASE ) {
      trcontrol.end_drag();
      transform.translate(trcontrol.trans_value());// Update the combined transformation
      trcontrol.reset(); currenttr = None;
    }
  }
  
  // Handle zooming by mouse. Will do the zooming irrespective of which mouse
  // button was used and irrespective of other modifiers. Does not call
  // redraw or return any value. Will always update the controller. So this
  // function should be called only when zooming is required
  
  // Only the x movement is used for zooming. Zooming is done using the Zoom
  // controller which computes the apprpriate transformation matrix
  virtual void handle_zoom(int event) {
    // Don't do anything if we aren't in a neutral state or aren't zooming
    if ( currenttr != None && currenttr != Zoom ) return;
    
    float z;
    
    currenttr = Zoom;
    
    // Transform coords to lie between -1 to 1
    z = ( float(Fl::event_x() << 1) / w() ) - 1.0;
    
    // Currently both orthographic and perspective zoom is handled by same
    // controller which simply does a scaling
    /*
      if ( projtype == Orthographic )
      {
      }
      else if ( projtype == Perspective )
      {
            }
            */
    zoomcontrol.mouse(z); zoomcontrol.update();
    
    if ( event == FL_PUSH ) zoomcontrol.begin_drag();
    else if ( event == FL_RELEASE )
      {
	zoomcontrol.end_drag();
	transform.scale(zoomcontrol.zoom_value());// Update the combined transformation
	zoomcontrol.reset(); currenttr = None;
      }
  }
  
  // Handle dollying by mouse. Will do the dollying irrespective of which mouse
  // button was used and irrespective of other modifiers. Does not call
  // redraw or return any value. Will always update the controller. So this
  // function should be called only when dollying is required
  
  // Only the x movement is used for dollying. Dollying is done using the Dolly
  // controller which computes the appropriate transformation matrix
  // The transformation from dollying is not multiplied onto the transformation
  // matrix since this should be done outside of all other transformations
  virtual void handle_dolly(int event) {
    // Don't do anything if we aren't in a neutral state or aren't dollying
    if ( currenttr != None && currenttr != Dolly ) return;
    
    float z;
    
    currenttr = Dolly;
    
    // Transform coords to lie between -1 to 1
    z = ( float(Fl::event_x() << 1) / w() ) - 1.0;
    
    dollycontrol.mouse(z); dollycontrol.update();
    
    if ( event == FL_PUSH ) dollycontrol.begin_drag();
    else if ( event == FL_RELEASE ) {
      dollycontrol.end_drag();
      // Shouldn't reset since the value is directly used for transformations
      // separate from other transformations
      currenttr = None;
    }
  }
  
  // Implement handle() function for backward compatibility
  virtual int handle(int event) {
    if ( event == FL_PUSH || event == FL_RELEASE || event == FL_DRAG ) {
      if ( Fl::event_button() == FL_LEFT_MOUSE ) handle_rotate(event);
      else if ( Fl::event_button() == FL_MIDDLE_MOUSE ) handle_pan(event);
      else if (( Fl::event_button() == FL_RIGHT_MOUSE ) && Fl::event_state(FL_CTRL)) handle_dolly(event);
      else if ( Fl::event_button() == FL_RIGHT_MOUSE ) handle_zoom(event);
      
      if ( event != FL_PUSH ) redraw();
      return 1;
    }
    return Fl_Gl_Window::handle(event);
  }
  
  void arcball_draw(void);                           // Draw the arcball
  void drawConstraints (void) const;
  void drawDragArc (void);
  void arcball_transform(void) {                     // Apply the arcball transformation
    float mat[16];
    
    // Do the dollying separately before everything else
    glTranslated(0,0,dollycontrol.dolly_value());
    switch ( currenttr ) {
    case Pan :
      trcontrol.value().fill_array_row_major(mat);
      glMultMatrixd(mat);
      break;
    case Zoom :
      zoomcontrol.value().fill_array_row_major(mat);
      glMultMatrixd(mat);
      break;
    case Rotate :
      arcball.value().fill_array_row_major(mat);
      glMultMatrixd(mat);
      break;
    default:
      break;
    }
    transform.apply();
  }
  
  //--- Access arcball parameters ---//
  
  void arcball_center(const FVector3& cen)  {
    arcball.center(cen);
  }
  
  void arcball_radius(float rad) {
    arcball.radius(rad);
  }
  
  FVector3 arcball_center(void) const {
    return arcball.center();
  }
  
  float arcball_radius(void) const {
    return arcball.radius();
  }

  void reset () {
    transform.reset ();
  }
   
  /**
   * Get the current FArcball_Control being used for rotation.
   */
  FArcball_Control get_arcball () { return arcball; }
   
  /**
   * Set the current FArcball_Control being used for rotation.
   */
  void set_arcball (const FArcball_Control& arcball_) { arcball = arcball_; }
   
};

#endif

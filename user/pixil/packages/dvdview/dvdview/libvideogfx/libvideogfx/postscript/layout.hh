/*********************************************************************
  graphics/visualize/layout.hh

  purpose:
    Data structures and layout manager for the positioning of
    postscript output on paper.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
    27/Jul/99 - Dirk Farin - implemented "Document Structuring Conventions"
    21/Jul/99 - Dirk Farin - first implementation
 *********************************************************************/

#ifndef LIBVIDEOGFX_POSTSCRIPT_LAYOUT_HH
#define LIBVIDEOGFX_POSTSCRIPT_LAYOUT_HH

#include "libvideogfx/types.hh"
#include <iostream.h>

/* Size of paper to be printed on. */
struct PaperSize
{
  PaperSize() : border(1.5) { }
  PaperSize(float w,float h) : width(w), height(h), border(1.5) { }

  // All measures in 'cm'.
  float width,height;

  float border; // Unprintable area on each side of paper.
};

// Several common paper sizes
extern PaperSize Paper_A3;
extern PaperSize Paper_A4;
extern PaperSize Paper_A5;




/* Area where to print on. The offset defines the location of the
   top left corner of the area. The coordinate system has 0/0 in
   the bottom left (postscript standard).
*/
struct PrintingArea
{
  // All measures in 'cm'.
  float xoffs,yoffs;
  float width,height;
};

#define cm2pts 28.5634



/* A PostscriptModule gives every postscript generating object the
   possibility to emit its own code into the postscript document
   header.
*/
class PostscriptModule
{
public:
  virtual ~PostscriptModule() { }

  virtual void EmitPrologCode() { }
  virtual void EmitSetupCode()  { }
};

#define MAX_PS_MODULES 10  // Maximum number of PostscriptModules that may be attached to a PrintAreaLayouter.


class PrintAreaLayouter
{
public:
           PrintAreaLayouter();
  virtual ~PrintAreaLayouter() { }

  void     SetOutput(ostream& str) { d_str = &str;  } // Set output stream.
  ostream& AskOutput()             { return *d_str; } // Ask output stream.

  void     AddPostscriptModule(PostscriptModule& mod);

  void     DrawBorder(bool flag=true) { d_DrawBorder=flag; } // Toggle drawing of borders on and off.

  virtual void         Init()        = 0;
  virtual PrintingArea GetNextArea() = 0;
  virtual void         Finish()      = 0;

protected:
  void _SetParams(bool landscape,float paperwidth) { d_landscape=landscape; d_paperwidth=paperwidth; }

  void _Init();
  void _BeginNewPage();
  void _Finish();
  void _DoDrawBorder(PrintingArea&);

  ostream* d_str;

private:
  bool     d_landscape;
  float   d_paperwidth;

  bool     d_DrawBorder;
  int      d_currPage;

  PostscriptModule* d_PSmodule[MAX_PS_MODULES];
  int d_nPSmodules;
};


/* This layouter makes the automatic placement of PrintingAreas on all sorts of paper easy.
   Specify the aspect ratio of the images you want to place on the page, whether the page should
   be turned to landscape, the number of images you want on one sheet, the spacing between the
   images and the paper size. The layouter will try to find the best arrangement of the images
   so that the images size is maximized.

   Other nice features are:
   - It takes care of all necessary postscript headers and 'showpage's .
   - It can automatically draw a border around each image.
*/
class PrintAreaLayouter_nUp : public  PrintAreaLayouter,
			      private PostscriptModule
{
public:
   PrintAreaLayouter_nUp();
  ~PrintAreaLayouter_nUp() { }

  int  SetParams(float aspect,          // width/height
		 bool   landscape=false,
		 int nup=2,              // Number of areas on one sheet of paper (actual number may be greater)
		 float spacing=1.0,     // spacing between areas (in 'cm')
		 PaperSize paper=Paper_A4);


  // Do initializations and write postscript header.
  void Init();

  /* Get next PrintingArea. If enabled, a border will be automatically drawn around it.
     Takes even care of full pages in which case it will emit a new-page postscript sequence.
  */
  PrintingArea GetNextArea();

  // Do cleanup and write some postscript cleanup code.
  void Finish();

private:
  int      d_nCols,d_nRows;
  float   d_xOffs,d_yOffs;
  float   d_xStep,d_yStep;
  float   d_xSize,d_ySize;

  int      d_nextAreaNr;


  // overriding methods from PostscriptModule

  void EmitSetupCode();
};

#endif

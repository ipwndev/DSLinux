
#include "libvideogfx/postscript/layout.hh"

// struct PaperSize_A4 Paper_A4;
struct PaperSize Paper_A3(29.70,42.00);
struct PaperSize Paper_A4(21.00,29.70);
struct PaperSize Paper_A5(14.85,21.00);


PrintAreaLayouter::PrintAreaLayouter()
  : d_str(NULL), d_DrawBorder(true), d_currPage(1),
    d_nPSmodules(0)
{
}


void PrintAreaLayouter::AddPostscriptModule(PostscriptModule& mod)
{
  assert(d_nPSmodules<MAX_PS_MODULES);

  d_PSmodule[d_nPSmodules++] = &mod;
}


void PrintAreaLayouter::_Init()
{
  assert(d_str);

  *d_str << "%!PS-Adobe-2.0\n"
	 << "%%Creator: libvideogfx visualization class\n"
	 << "%%Orientation: " << (d_landscape ? "Landscape\n" : "Portrait\n")
	 << "%%PageOrder: Ascend\n"
	 << "%%EndComments\n"
	 << "%%BeginProlog\n"
	 << "/cm { 28.5634 mul } def\n";

  if (d_landscape)
    {
      *d_str << "/turntolandscape { 90 rotate 0 " << (-d_paperwidth*cm2pts) << " translate } def\n";
    }

  for (int i=0;i<d_nPSmodules;i++)
    d_PSmodule[i]->EmitPrologCode();

  *d_str << "%%EndProlog\n"
	 << "%%BeginSetup\n";

  for (int i=0;i<d_nPSmodules;i++)
    d_PSmodule[i]->EmitSetupCode();

  *d_str << "%%EndSetup\n";
}


void PrintAreaLayouter::_BeginNewPage()
{
  assert(d_str);

  if (d_currPage>1) *d_str << "showpage\n";
  *d_str << "%%Page: " << d_currPage << endl;
  if (d_landscape) *d_str << "turntolandscape\n";
  d_currPage++;
}

void PrintAreaLayouter::_Finish()
{
  assert(d_str);
  *d_str << "showpage\n";
}

void PrintAreaLayouter::_DoDrawBorder(PrintingArea& area)
{
  if (d_DrawBorder)
    {
      *d_str << "gsave\n1 setlinewidth\n"
	     << (area.xoffs*cm2pts) << " "
	     << (area.yoffs*cm2pts) << " moveto\n"
	     << (area.width*cm2pts) << " 0 rlineto\n"
	     << "0 " << (-area.height*cm2pts) << " rlineto\n"
	     << (-area.width*cm2pts) << " 0 rlineto\n"
	     << "0 " << (area.height*cm2pts) << " rlineto\nstroke\ngrestore\n";
    }
}




// ------------------ PrintAreaLayouter_nUp ------------------------



PrintAreaLayouter_nUp::PrintAreaLayouter_nUp()
  : d_nextAreaNr(0)
{
  AddPostscriptModule(*this);
}

int PrintAreaLayouter_nUp::SetParams(float aspect,bool landscape,int nup,float spacing,PaperSize paper)
{
try_again:
  float width  = (landscape ? paper.height : paper.width )-2*paper.border;
  float height = (landscape ? paper.width  : paper.height)-2*paper.border;

  int    bestcol=0;
  int    bestrow=0;
  float bestaw=0.0;
  float bestah=0.0;

  // Try all possible number of columns and take that one that results in largest area sizes.
  for (int ncol=1;ncol<=nup;ncol++)
    {
      int nrow = (nup+ncol-1)/ncol;

      float aw = (width -(ncol-1)*spacing)/ncol;
      float ah = (height-(nrow-1)*spacing)/nrow;

      if (aw<=0.0 || ah <= 0.0)
	continue;

      if (aw/ah >= aspect)
	{
	  aw = aspect*ah;
	}
      else
	{
	  ah = aw/aspect;
	}

      if (aw>bestaw)
	{
	  bestaw =aw;   bestah =ah;
	  bestcol=ncol; bestrow=nrow;
	}
    }

  // If it does not fit on the page, try to place fewer areas on the page.
  if (bestcol==0)
    {
      assert(nup>0);
      nup--;
      goto try_again;
    }

  float aox = (width -(bestcol-1)*spacing - bestcol*bestaw)/bestcol/2;
  float aoy = (height-(bestrow-1)*spacing - bestrow*bestah)/bestrow/2;

  d_nCols = bestcol;
  d_nRows = bestrow;
  d_xOffs = paper.border+aox;
  d_yOffs = height+paper.border-aoy;
  d_xStep = (width -(bestcol-1)*spacing)/bestcol+spacing;
  d_yStep = (height-(bestrow-1)*spacing)/bestrow+spacing;
  d_xSize = bestaw;
  d_ySize = bestah;

  _SetParams(landscape,paper.width);
}

void PrintAreaLayouter_nUp::Init()
{
  _Init();
}

PrintingArea PrintAreaLayouter_nUp::GetNextArea()
{
  assert(d_str);

  if (d_nextAreaNr==0 || d_nextAreaNr == d_nCols*d_nRows)
    {
      _BeginNewPage();
      d_nextAreaNr=0;
    }

  PrintingArea area;
  area.xoffs  = (d_xOffs + (d_nextAreaNr%d_nCols)*d_xStep);
  area.yoffs  = (d_yOffs - (d_nextAreaNr/d_nCols)*d_yStep);
  area.width  = d_xSize;
  area.height = d_ySize;

  _DoDrawBorder(area);

  d_nextAreaNr++;

  return area;
}

void PrintAreaLayouter_nUp::Finish()
{
  _Finish();
}

void PrintAreaLayouter_nUp::EmitSetupCode()
{
  *d_str << "0 setgray\n"
	 << "0 setlinewidth\n";
}

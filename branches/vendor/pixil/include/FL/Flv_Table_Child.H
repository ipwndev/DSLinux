#ifndef FLV_TABLE_CHILD_H
#define FLV_TABLE_CHILD_H

#include <Flek/Flv_Table.H>

#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Pixmap.H>

#include <stdio.h>
#include <string.h>

struct node_data {
  int row;
  void * data;
};

struct node_image {
  int id;
  int R;
  int C;
  Fl_Pixmap * image_data;
  node_image * next;
};

struct col_data {

  char * _data;

};

struct table_row {
  col_data * col;
};

//	Demonstrate Simplest Fl_List_Child
class Flv_Table_Child : public Flv_Table
{

private:
  int numRows;
  int numCols;
  int lastNumRecs;
  node_data * node;
  node_image * images;
  table_row * val;
  int col0, col1, col2, col3;
  Fl_Color hilight_dark;
  int save_h;
  bool move;

public:
  Flv_Table_Child( int X, int Y, int W, int H, const char *l=0, int _col0=25, 
		   int _col1=75, int _col2=0, int _col3=0 );
  virtual ~Flv_Table_Child();
  void movable(bool flag) { move = flag; }

  virtual void resize(int x, int y, int w, int h) { 
    int new_h;
    if ( h == save_h )
      new_h = save_h;
    else {
      new_h = h - (this->y() - y) - 5;
    }      

    //printf("*********** h(%d),save_h(%d),new_h(%d)\n", h, save_h,new_h);

    if ( move ) 
      Fl_Widget::resize(x, y, w, new_h); 
    else
      Fl_Widget::resize(this->x(), this->y(), w, new_h); 
  }

  //
  // Image Managegment
  //

  // Methods
  void _set_image(Fl_Pixmap * image, int R, int C, int id);
  void set_image(Fl_Pixmap * image, int R, int C, int id);
  Fl_Pixmap * get_image(int id);
  int find_image(int R, int C);
  
  char * get_value(int R, int C, int &image);
  void set_value(int R, int C, char * data);
  void Init(int _rows);
  void Add(int row, void * data);
	void *data(int row);
  void * selected();

  virtual void draw_cell( int Offset, int &X, int &Y, int &W, int &H, int R, int C );
  void get_style( Flv_Style &s, int R, int C );
  int text_width();
	virtual int col_width( int C );
	virtual int col_width( int w, int C ){return Flv_Table::col_width(w,C);}
  void SetCols(int _cols) { numCols = _cols; cols(numCols); }

};

#endif

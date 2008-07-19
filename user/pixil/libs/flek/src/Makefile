### Flek Makefile
# This makefile has been changed to adjust to the Pixil build environement.
# Check the readme for details on the changes that we have made

LIB_STATIC=libflek.a
LIB_SHARED=libflek.so

CPPFILES= FDate.cxx FSocket.cxx FFile.cxx FVector.cxx \
	FImage.cxx FSGI.cxx FPNM.cxx Fl_App_Window.cxx \
	Fl_Better_Window.cxx Fl_Calendar.cxx Fl_Dockable_Window.cxx \
	Fl_Stock.cxx Fl_Toggle_Node_Base.cxx Fl_Toggle_Node.cxx \
	Fl_Toggle_Tree_Base.cxx Fl_Toggle_Tree.cxx Flv_CStyle.cxx \
	Flv_Style.cxx Flv_List.cxx Flv_Table.cxx Flve_Check_Button.cxx \
	Flve_Input.cxx Flve_Combo.cxx Fl_Time.cxx

OBJS = $(CPPFILES:.cxx=.o)

CFLAGS ?=
CFLAGS += -DPIXIL
CXXFLAGS += -DPIXIL

ifeq ($(COMPILE_CROSS),y)
# CFLAGS += -fpermissive
endif

INCLUDES=-I. -I../ 

include $(BASE_DIR)/Rules.make

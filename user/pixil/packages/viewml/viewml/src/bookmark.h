/*******************************************************************************\
**
**	Header file:	bookmark.h
**	Desc:			Class definition for bookmark capabilities within the viewml
**					browswer.
**	History:		06/08/01	J Webb	<jeffw@censoft.com>
**								Initial Version
**
\*******************************************************************************/

#ifndef BOOKMARK_H_INCLUDED
#define	BOOKMARK_H_INCLUDED	1


// System header files
#include <string>
#include <vector>
using namespace std;
// Local header files
#include <FL/Fl_Menu_Button.H>

// External functions
// These 3 functions are the external wrappers for the class function needed in
// varying scopes
extern void AddBookmark_cb(Fl_Widget *w, void *data);
extern void DelBookmark_cb(Fl_Widget *w, void *data);
extern void SelBookmark_cb(Fl_Widget *w, void *data);


class BookMark
{
	typedef struct
	{
		string				pgName,									// Name of the Page (taken from html <Title>)
							pgURL;									// Actual URL
	} bookmark_t;

	private:
		int					m_nelem,								// Number of elements currently in vector
							m_maxelem;								// Maximum number of elements allowed (0 = No limit)
		string				m_txtfile;								// FQPN of the text file
		Fl_Menu_Button		*m_mb;									// Menu button for this container
		vector<bookmark_t>	m_bmv_srt;								// Bookmark vector (alpha sort be pgName)

	public:
		BookMark(int mxelem = 0, char *tpath = "./.bookmark.vml");
		~BookMark();
		int AddBookmark(char *name, char *URL);
		void DelBookmark(int idx);
		char *GetURL(int idx);
		void SetMenuWidget(Fl_Menu_Button *w) {if (w) {m_mb = w; UpdateMenus();} return;}

	private:
		// These functions are just for internal class operation
		int findVElem(bookmark_t *bmptr);
		char *fltkmStrParse(char *str);
		void UpdateMenus();
		void UpdateTxtFile();
		void ReadDataFile();

}; // end of class BookMark definition

#endif // BOOKMARK_H_INCLUDED

/*******************************************************************************\
**
**	File:	bookmark.cpp
**	Desc:	Adds definition to the methods within the bookmark class object
**	
\*******************************************************************************/

// System header files
#include <cstdio>


// Local header files
#include <FL/Fl_Menu_Item.H>
#include "bookmark.h"


////////////////////////////////////////////////////////////////////////////////
//
//	Public member functions
//
////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************\
**
**	Function:	BookMark()
**	Desc:		Default constructor for the BookMark class.  Handles the initialization
**				of class members.
**	Accepts:	int mxelem = Maximum number of elements to allow (0 = no limit).
**				char *tpath = The path to the bookmark file
**
\*******************************************************************************/
BookMark::BookMark(int mxelem, char *tpath) :
	m_nelem(0),													// Initialize to 0
	m_maxelem(mxelem),											// Initialize to mxelem
	m_txtfile(tpath)											// Set the text path file
{
	// Default initialization for class BookMark
	m_mb = NULL;

	// Initialize the vector
	ReadDataFile();

} // end of BookMark() -- constructor

/*******************************************************************************\
**
**	Function:	~BookMark()
**	Desc:		Destructor for the BookMark class.  Updates any file with current
**				bookmark data, and handles proper clean up
**	
\*******************************************************************************/
BookMark::~BookMark()
{
	// Update the bookmark file with current contents of vector
	UpdateTxtFile();

	// Clear out the vector
	m_bmv_srt.clear();
	
	// Delete any dynamically allocated memory
	delete m_mb;
} // end of ~BookMark() -- destructor

/*******************************************************************************\
**
**	Function:	int BookMark::AddBookmark()
**	Desc:		Adds a Name/URL pair to the vector (if there is room)
**	Accepts:	char *name = Name of the URL page
**				char *URL = URL of the page
**	Returns:	int; 0 on success, -1 on error
**
\*******************************************************************************/
int
BookMark::AddBookmark(char *name, char *URL)
{
	int					idx,										// Index to insert
						namedup = 0;								// Set if name was dup'd
	bookmark_t			bmData;										// Bookmark data

	
	// Only add if there we haven't reached any imposed limit
	if (m_maxelem > 0 && m_nelem + 1 > m_maxelem)
		return (-1);

	// Validate incoming data
	if (URL == NULL || *URL == 0)
		return (-1);
	if (name == NULL || *name == 0)
	{
		name = strdup(URL);
		namedup = 1;
	} // end of if 

	// Strip out all invalid name character (as defined by fltk's menu rules)
	bmData.pgName = fltkmStrParse(name);					// Possible memory leak???
	bmData.pgURL = URL;

	if (namedup)
		free(name);

	// Find where to insert it
	if ( (idx = findVElem(&bmData)) >= 0)
	{
		// Already in the list
		return (-1);
	} // end of if 

	idx *= -1;
	idx--;
	m_bmv_srt.insert(&m_bmv_srt[idx], bmData);
	m_nelem = m_bmv_srt.size();

	// Decide if updates are to be set...
	if (m_mb)
	{
		UpdateMenus();
		UpdateTxtFile();
	} // end of m_mb

	return (0);
} // end of AddBookmark()

/*******************************************************************************\
**
**	Function:	void BookMark::DelBookmark()
**	Desc:		Deletes the bookmark entry at the specified index
**	Accepts:	int idx = The index of the bookmark to delete
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
BookMark::DelBookmark(int idx)
{
	// Validate the request
	if (!m_nelem)
		return;

	if (idx < 0 || idx >= m_nelem)
		return;

	m_bmv_srt.erase(&m_bmv_srt[idx]);
	m_nelem = m_bmv_srt.size();

	if (m_mb)
	{
		UpdateMenus();
		UpdateTxtFile();
	} // end of if

	return;
} // end of DelBookmark

/*******************************************************************************\
**
**	Function:	char *BookMark::GetURL()
**	Desc:		Returns the URL of the bookmark stored at index idx
**	Accepts:	int idx = The index of the bookmark to return
**	Returns:	char *; The URL value of the bookmark_t bookmark entry
**
\*******************************************************************************/
char *
BookMark::GetURL(int idx)
{
	if (idx >= 0 && idx < m_nelem)
		return ((char *)m_bmv_srt[idx].pgURL.c_str());
	else
		return ((char *)0);
} // end of BookMark::GetURL()

////////////////////////////////////////////////////////////////////////////////
//
//	Private/Protected memober functions
//
////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************\
**
**	Function:	int BookMark::findVElem()
**	Desc:		Does a binary search on the elements in the vector looking for
**				the name (to see if it already exists) and returns the index into
**				the vector, otherwise it returns the location of where it should
**				be inserted (see below)
**	Accepts:	bookmark_t *ptr = Ptr to the bookmark_t elem to find
**	Returns:	>= 0 the index of the found item, < 0 the index of where it should
**				be inserted, using the following algorithm:
**					abs(idx) - 1;
**
\*******************************************************************************/
int
BookMark::findVElem(bookmark_t *bmptr)
{
	int					found = -1,									// Flag to indicate a found status
						hi,											// Hi limit
						lo,											// Lo limit
						mid,										// Middle ground
						rc;											// Result code

	// Set the initial limit values
	lo = 0;
	hi = m_nelem - 1;

	// See if something is in the vector
	if (m_nelem <= 0)
		return ( (lo + 1) * -1);

	// Check the initial lo point
	if ( (rc = m_bmv_srt[lo].pgName.compare(bmptr->pgName)) == 0)
		return (lo);
	else if (rc > 0)
		return ((lo + 1) * -1);

	// Check the initial hi point
	if ( (rc = m_bmv_srt[hi].pgName.compare(bmptr->pgName)) == 0)
		return (hi);
	else if (rc < 0)
		return ( (hi + 1 + 1) * -1);

	// Do the binary search.
	while (found < 0 && lo <= hi)
	{
		mid = ((lo + hi) / 2);
		if ( (rc = m_bmv_srt[mid].pgName.compare(bmptr->pgName)) == 0)
			found = mid;
		else if (rc > 0)
			hi = mid - 1;
		else
			lo = mid + 1;
	} // end of while

	if (found >= 0)
		return (found);
	else
		return ( (lo + 1) * -1);

} // end of BookMark::findVElem() 

/*******************************************************************************\
**
**	Function:	char *BookMark::fltkmStrParse()
**	Desc:		Scans through the string and converts the following special
**				characters to spaces:
**				',' == Field delimiter within the bookmark text file
**				'|' == Fltk menu char
**				'\' == Fltk menu char
**				'/' == Fltk menu char
**	Accepts:	char *str = string to parse
**	Returns:	char *ptr to the converted string
**
\*******************************************************************************/
char *
BookMark::fltkmStrParse(char *str)
{
	char 					*cp;									// Character pointer

	for (cp = str; *cp; cp++)
	{
		if (*cp == ',' || *cp == '/' || 
			*cp == '|' || *cp == '\\')
		{
			*cp = ' ';
		} // end of if
	} // end of for 

	return (str);
} // end of BookMark::fltkmStrParse()

/*******************************************************************************\
**
**	Function:	void BookMark::ReadDataFile()
**	Desc:		Reads the data stored in m_txtfile and populates the vector with
**				the dat from the text file
**				Text File layout is:
**				Page Name,Page URL
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
BookMark::ReadDataFile(void)
{
	char				*cpName,									// Ptr to the name field
						*cpURL,										// Ptr to the URL
						txtbuf[256];								// Should I use a string???
	int					addcnt = 0;									// Number added successfully
	FILE				*fpBm;										// File pointer to bookmark data

	// open the file for reading
	if ( (fpBm = fopen(m_txtfile.c_str(), "r")) == NULL)
		return;

	// Clear out the vector (since its going to be repopulated)
	m_bmv_srt.clear();

	while (fgets(txtbuf, sizeof(txtbuf), fpBm))
	{
		if (m_maxelem && addcnt + 1 >= m_maxelem)
			break;

		if ( (cpURL = strchr(txtbuf, ',')) == NULL)
		{
			fprintf(stderr, "Invalid bookmark entry %s\n", txtbuf);
			continue;
		} // end of if
		*cpURL = '\0';
		cpURL++;
		if (cpURL[strlen(cpURL) - 1] == '\n')
			cpURL[strlen(cpURL) - 1] = '\0';
		cpName = txtbuf;

		if (AddBookmark(cpName, cpURL))
			++addcnt;
	} // end of while

	fclose(fpBm);
	return;
} // end of BookMark::ReadDataFile()

/*******************************************************************************\
**
**	Function:	void BookMark::UpdateMenus()
**	Desc:		Dynamically updates the Fl_Menu_Button menu list with the data
**				that is in the bookmark vector
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
BookMark::UpdateMenus(void)
{
	char				menuTitleBuf[64] = {0};						// Buffer for the title
	int					ii;											// Loop iterator
	const int			MAX_MENU_LEN = 32;							// Maximum menu length
	Fl_Menu_Item		baseMenus[] = 
		{
			{"&Add Bookmark", FL_ALT + 'a', AddBookmark_cb},
			{"&Delete Bookmark", FL_ALT + 'd', 0, 0, FL_SUBMENU | (m_nelem ? 0 : FL_MENU_INACTIVE)},
				{0},
			{"&Select Bookmark", FL_ALT + 's', 0, 0, FL_SUBMENU | (m_nelem ? 0 : FL_MENU_INACTIVE)},
				{0},
			{0}
		},															// Default menu list
						*curMenuList;								// Current Menu list

	// Replace the menu....(as a private copy)
	m_mb->copy(baseMenus);
//	curMenuList = m_mb->menu();
	for (ii = 0; ii < m_nelem; ii++)
	{
		int len = sprintf(menuTitleBuf, "&Delete Bookmark/");
		strncpy(menuTitleBuf + len, m_bmv_srt[ii].pgName.c_str(), MAX_MENU_LEN);
		m_mb->add(menuTitleBuf,0, DelBookmark_cb, (void *)ii);
		len = sprintf(menuTitleBuf, "&Select Bookmark/");
		strncpy(menuTitleBuf + len, m_bmv_srt[ii].pgName.c_str(), MAX_MENU_LEN);
		m_mb->add(menuTitleBuf, 0, SelBookmark_cb, (void *)ii);
	} // end of for

	return;
} // end of UpdateMenus()

/*******************************************************************************\
**
**	Function:	void BookMark::UpdateTxtFile()
**	Desc:		Updates the bookmark textfile with the data that is in the bookmark
**				vector
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
BookMark::UpdateTxtFile(void)
{
	FILE				*fpBm;										// Bookmark file pointer

	if ( (fpBm = fopen(m_txtfile.c_str(), "w")) == NULL)
		return;

	for (int ii = 0; ii < m_nelem; ii++)
		fprintf(fpBm, "%s,%s\n", m_bmv_srt[ii].pgName.c_str(),
				m_bmv_srt[ii].pgURL.c_str());

	fclose(fpBm);

	return;
} // end of BookMark::UpdateTxtFile()


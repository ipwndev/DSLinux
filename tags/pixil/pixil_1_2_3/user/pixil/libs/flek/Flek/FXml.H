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

#ifndef _FXML_H__
#define _FXML_H__

#include <libxml/parser.h>

/**
 * @package libflek_xml
 */

typedef enum {
  FXmlElementNode = XML_ELEMENT_NODE,
  FXmlAttributeNode = XML_ATTRIBUTE_NODE,
  FXmlTextNode = XML_TEXT_NODE,
  FXmlCDataSectionNode = XML_CDATA_SECTION_NODE,
  FXmlEntityRefNode = XML_ENTITY_REF_NODE,
  FXmlEntityNode = XML_ENTITY_NODE,
  FXmlPiNode = XML_PI_NODE,
  FXmlCommentNode = XML_COMMENT_NODE,
  FXmlDocumentNode = XML_DOCUMENT_NODE,
  FXmlDocumentTypeNode = XML_DOCUMENT_TYPE_NODE,
  FXmlDocumentFragNode = XML_DOCUMENT_FRAG_NODE,
  FXmlNotationNode = XML_NOTATION_NODE,
  FXmlHtmlDocumentNode = XML_HTML_DOCUMENT_NODE,
  FXmlDtdNode = XML_DTD_NODE,
  FXmlElementDecl = XML_ELEMENT_DECL,
  FXmlAttributeDecl = XML_ATTRIBUTE_DECL,
  FXmlEntityDecl = XML_ENTITY_DECL
} FXmlElementType;

typedef enum {
  FXmlAttributeCData = XML_ATTRIBUTE_CDATA,
  FXmlAttributeId = XML_ATTRIBUTE_ID,
  FXmlAttributeIdRef = XML_ATTRIBUTE_IDREF,
  FXmlAttributeIdRefs = XML_ATTRIBUTE_IDREFS,
  FXmlAttributeEntity = XML_ATTRIBUTE_ENTITY,
  FXmlAttributeEntities = XML_ATTRIBUTE_ENTITIES,
  FXmlAttributeNMToken = XML_ATTRIBUTE_NMTOKEN,
  FXmlAttributeNMTokens = XML_ATTRIBUTE_NMTOKENS,
  FXmlAttributeEnumeration = XML_ATTRIBUTE_ENUMERATION,
  FXmlAttributeNotation = XML_ATTRIBUTE_NOTATION
} FXmlAttributeType;

typedef enum {
  FXmlAttributeNone = XML_ATTRIBUTE_NONE,
  FXmlAttributeRequired = XML_ATTRIBUTE_REQUIRED,
  FXmlAttributeImplied = XML_ATTRIBUTE_IMPLIED,
  FXmlAttributeFixed = XML_ATTRIBUTE_FIXED
} FXmlAttributeDefault;

class FXmlDocument;
class FXmlNode;

/**
 * Attribute
 */
class FXmlAttr {

 public:

  FXmlAttr () { Attr = 0; }
  FXmlAttr (xmlAttrPtr);
  bool valid () { if (Attr) return true; return false; }
  FXmlElementType type ();

  const char* name ();  
  FXmlNode children ();
  FXmlAttr next ();
  FXmlAttr previous ();
  FXmlNode parent ();
  FXmlNode last ();
  FXmlDocument document ();

  FXmlAttributeType atype ();
  //FXmlAttrDefault def ();
  //const char* defaultValue ();

  FXmlNode value ();

 protected:

  xmlAttrPtr Attr;

};

/**
 * Node.
 */
class FXmlNode {

 public:

  FXmlNode () { Node = 0; }
  FXmlNode (xmlNodePtr);
  bool valid () { if (Node) return true; return false; }
  FXmlElementType type ();

  const char* name ();  
  FXmlNode children ();
  FXmlNode next ();
  FXmlNode previous ();
  FXmlNode parent ();
  FXmlNode last ();
  FXmlDocument document ();
  // FXmlNs ns ();
  char* content ();  

  FXmlAttr properties ();
  char* property (const char* name);

  FXmlNode& operator = (const xmlNodePtr& p)
    {
      Node = p;
      return (*this);
    }

  FXmlNode& operator = (const FXmlNode& p)
    {
      Node = p.Node;
      return (*this);
    }
  
 protected:

  xmlNodePtr Node;

};

/**
 * Document
 */
class FXmlDocument {

 public:

  FXmlDocument () { Doc = 0; }
  FXmlDocument (xmlDocPtr);
  bool valid () { if (Doc) return true; return false; }
  FXmlElementType type ();

  const char* name ();
  FXmlNode children ();
  FXmlNode next ();
  FXmlNode previous ();
  FXmlNode parent ();
  FXmlNode last ();
  FXmlDocument document ();

  // Basic parsing interface:
  static FXmlDocument parse_file (const char* filename);
  static FXmlDocument parse_buffer (const char* buffer, int size);

 protected:

  xmlDocPtr Doc;

};

#endif

------------------------------------------------------------------------------
--                                                                          --
--                           GNAT ncurses Binding                           --
--                                                                          --
--          Terminal_Interface.Curses.Forms.Field_Types.Enumeration         --
--                                                                          --
--                                 B O D Y                                  --
--                                                                          --
------------------------------------------------------------------------------
-- Copyright (c) 1998 Free Software Foundation, Inc.                        --
--                                                                          --
-- Permission is hereby granted, free of charge, to any person obtaining a  --
-- copy of this software and associated documentation files (the            --
-- "Software"), to deal in the Software without restriction, including      --
-- without limitation the rights to use, copy, modify, merge, publish,      --
-- distribute, distribute with modifications, sublicense, and/or sell       --
-- copies of the Software, and to permit persons to whom the Software is    --
-- furnished to do so, subject to the following conditions:                 --
--                                                                          --
-- The above copyright notice and this permission notice shall be included  --
-- in all copies or substantial portions of the Software.                   --
--                                                                          --
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  --
-- OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               --
-- MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   --
-- IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   --
-- DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    --
-- OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    --
-- THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               --
--                                                                          --
-- Except as contained in this notice, the name(s) of the above copyright   --
-- holders shall not be used in advertising or otherwise to promote the     --
-- sale, use or other dealings in this Software without prior written       --
-- authorization.                                                           --
------------------------------------------------------------------------------
--  Author:  Juergen Pfeifer, 1996
--  Version Control:
--  $Revision$
--  Binding Version 01.00
------------------------------------------------------------------------------
with Ada.Unchecked_Deallocation;
with Interfaces.C; use Interfaces.C;
with Interfaces.C.Strings; use Interfaces.C.Strings;
with Terminal_Interface.Curses.Aux; use Terminal_Interface.Curses.Aux;

package body Terminal_Interface.Curses.Forms.Field_Types.Enumeration is

   function Create (Info               : Enumeration_Info;
                    Auto_Release_Names : Boolean := False)
                    return Enumeration_Field
   is
      procedure Release_String is
        new Ada.Unchecked_Deallocation (String,
                                        String_Access);
      E : Enumeration_Field;
      L : constant size_t := 1 + size_t (Info.C);
      S : String_Access;
   begin
      E.Case_Sensitive       := Info.Case_Sensitive;
      E.Match_Must_Be_Unique := Info.Match_Must_Be_Unique;
      E.Arr := new chars_ptr_array (size_t (1) .. L);
      for I in 1 .. Positive (L - 1) loop
         if Info.Names (I) = null then
            raise Form_Exception;
         end if;
         E.Arr (size_t (I)) := New_String (Info.Names (I).all);
         if Auto_Release_Names then
            S := Info.Names (I);
            Release_String (S);
         end if;
      end loop;
      E.Arr (L) := Null_Ptr;
      return E;
   end Create;

   procedure Release (Enum : in out Enumeration_Field)
   is
      I : size_t := 0;
      P : chars_ptr;
   begin
      loop
         P := Enum.Arr (I);
         exit when P = Null_Ptr;
         Free (P);
         Enum.Arr (I) := Null_Ptr;
         I := I + 1;
      end loop;
      Enum.Arr := null;
   end Release;

   procedure Set_Field_Type (Fld : in Field;
                             Typ : in Enumeration_Field)
   is
      C_Enum_Type : C_Field_Type;
      pragma Import (C, C_Enum_Type, "TYPE_ENUM");

      function Set_Fld_Type (F    : Field := Fld;
                             Cft  : C_Field_Type := C_Enum_Type;
                             Arg1 : chars_ptr_array;
                             Arg2 : C_Int;
                             Arg3 : C_Int) return C_Int;
      pragma Import (C, Set_Fld_Type, "set_field_type");

      Res : Eti_Error;
   begin
      if Typ.Arr = null then
         raise Form_Exception;
      end if;
      Res := Set_Fld_Type (Arg1 => Typ.Arr.all,
                           Arg2 => C_Int (Boolean'Pos (Typ.Case_Sensitive)),
                           Arg3 => C_Int (Boolean'Pos
                                          (Typ.Match_Must_Be_Unique)));
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
      Wrap_Builtin (Fld, Typ, C_Choice_Router);
   end Set_Field_Type;

end Terminal_Interface.Curses.Forms.Field_Types.Enumeration;

------------------------------------------------------------------------------
--                                                                          --
--                           GNAT ncurses Binding                           --
--                                                                          --
--              Terminal_Interface.Curses.Forms.Field_Types.Alpha           --
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
with Interfaces.C;
with Terminal_Interface.Curses.Aux; use Terminal_Interface.Curses.Aux;

package body Terminal_Interface.Curses.Forms.Field_Types.Alpha is

   use type Interfaces.C.int;

   procedure Set_Field_Type (Fld : in Field;
                             Typ : in Alpha_Field)
   is
      C_Alpha_Field_Type : C_Field_Type;
      pragma Import (C, C_Alpha_Field_Type, "TYPE_ALPHA");

      function Set_Fld_Type (F    : Field := Fld;
                             Cft  : C_Field_Type := C_Alpha_Field_Type;
                             Arg1 : C_Int) return C_Int;
      pragma Import (C, Set_Fld_Type, "set_field_type");

      Res : Eti_Error;
   begin
      Res := Set_Fld_Type (Arg1 => C_Int (Typ.Minimum_Field_Width));
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
      Wrap_Builtin (Fld, Typ);
   end Set_Field_Type;

end Terminal_Interface.Curses.Forms.Field_Types.Alpha;

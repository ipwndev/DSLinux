; ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;
;  Six-2-Four - a 4Kb intro packer - The decompression part.
;
;  Copyright (c) Kim Holviala a.k.a Kimmy/PULP Productions 1997
;
;  Linux version - Sed october 1999
;
; ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;
;    This program is free software; you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation; either version 2 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program; if not, write to the Free Software
;    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

;Note :
;This is a stub file. Some vars will be added by 624 after it has packed
;the data (data_length for example, sand others).

start:
; Decompress the data -------------------------------------------------------

decompress:
	xor     ebp,ebp			; Initialize regs
	xor	ecx, ecx

	mov     edi,decompress_to	; Destination offset

	push    edi			; Push edi for the final ret
	pusha
	mov     esi,data_start		; Start of the data


; Main loop -----------------------------------------------------------------

main_loop:
eoff:   cmp     esi,data_start+data_length      ; EOF?
	jb     noquit                    	; Yepp -> Return to 100h
	popa
	xor	edi, edi
	ret
noquit:

	call    get_bit                 ; Compressed/uncompressed data next?
	jc      compressed


; Handle uncompressed data --------------------------------------------------

	mov     cl,8                    ; Uncompressed -> Get next 8 bits
	cdq                             ; No fix-ups (clear edx)

	call    get_data                ; Get byte

	xchg    eax,ebx                   ; Store the byte
	stosb

	;jmp is 5 bytes long, clc jnc is only 3, we win 2 bytes !
	;jmp     main_loop               ; Loop
	clc
	jnc	main_loop


; Handle compressed data ----------------------------------------------------

compressed:

ll:     mov     dl,llstuff                   ; Maximum number of bits in a row

bit_loop:
	call    get_bit                 ; Loop until CF = 0
	jnc     c_getpos

	inc     ecx                      ; Increase lenght

	dec     edx                      ; Max number of bits read?
	jnz     bit_loop                ; Nope -> Keep reading

	call    get_huffman             ; Yepp -> Get huffman-coded lenght
	mov     ecx,ebx                   ; Lenght must be in cx

c_getpos:
	jecxz    lenght_1                ; Lenght 1? -> Do something else....

	push    ecx                      ; Save lengt
	call    get_huffman             ; Get huffman-coded position
	pop     ecx                      ; Restore lenght

c_copy:
	push    esi                      ; Save old source

	mov     esi,edi                   ; Calculate new source offset
	sub     esi,ebx

	inc     ecx                      ; Fix lenght
	rep movsb                       ; Copy duplicate data

	pop     esi                      ; Restore source offset

	;jmp     main_loop               ; Loop
	clc
	jnc	main_loop

lenght_1:
	mov     cl,4                    ; Get 4 bits of data for position
	cdq                             ; Fix-up value 
	inc     edx                      ; (dx = 1)

	call    get_data                ; Get data

	;jmp     c_copy                  ; Go back to loop
	clc
	jnc	c_copy

; Get one bit of data -------------------------------------------------------

; Returns:
;     CF - Value of the bit

gb_next_byte:
	lodsb                           ; Get one byte

	mov     ah,1                    ; Mark the end of data
	xchg    ebp,eax                   ; Move it to bp

get_bit:
	shr     ebp,1                    ; Get bit to CF
	jz      gb_next_byte            ; No more bits left in dx?
	ret

; Get huffman-coded number --------------------------------------------------

; Returns:
;     bx - The number

get_huffman:
	mov     cx,hl1stuff                    ; Assume 3 bits for the number
	cdq                             ; Fix-up value
	inc     edx                      ; (dx = 1)

	call    get_bit                 ; Get huffman bit
	jnc     get_data                ; Was 0 -> Values were correct

	mov     cl,hl2stuff                    ; Was 1 -> 5 bits for the number
	mov     dl,hf2stuff                   ; Fix-up value


; Get n bits of data --------------------------------------------------------

; Input:
;     cx - Number of bits wanted
;     dx - Fix-up value to be added to the result

; Returns:
;     bx - The requested number

get_data:
	xor     ebx,ebx

gd_loop:
	call    get_bit                 ; Get bit
	rcl     ebx,1                    ; Store it

	loop    gd_loop                 ; Loop

	add     ebx,edx                   ; Fix the value

	ret


; Start of the compressed data ----------------------------------------------

data_start:

;------------------------------------------------------------------------------
;   Mad Mouse
;---------------------------------------------------------------------
; version:	0.2
; last update:	03/06/2012
; changed by:	Marat Zakiyanov aka Mario79, aka Mario
; changes:	Some optimizations.
;---------------------------------------------------------------------
;   ���� �⮣� 㦠᭮ ��६���� ����: Sourcerer, 23.04.2010
;   popovpa (29.05.2012)
;   1. ��������� ������ ࠡ��� �ணࠬ��.
;   2. ��������� "���᪠�������" ᢥ��� � ���� :)
;   3. ��⨬����� ����.
;------------------------------------------------------------------------------
	use32		; �࠭����, �ᯮ����騩 32-� ࠧ�來� �������
	org 0x0 	; ������ ���� ����, �ᥣ�� 0x0

	db 'MENUET01'	; 1. �����䨪��� �ᯮ��塞��� 䠩�� (8 ����)
	dd 0x01 	; 2. ����� �ଠ� ��������� �ᯮ��塞��� 䠩��
	dd START	; 3. ����, �� ����� ��⥬� ��।��� �ࠢ�����
			; ��᫥ ����㧪� �ਫ������ � ������
	dd IM_END	; 4. ࠧ��� �ਫ������
	dd I_END	; 5. ���� ����室���� �ਫ������ �����
			; ����� �������� � ����� � ��������� �� 0x0
			; �� ���祭��, ��।��񭭮�� �����
	dd stack_area	; 6. ���設� �⥪� � ��������� �����, 㪠������ ���
	dd 0x0		; 7. 㪠��⥫� �� ��ப� � ��ࠬ��ࠬ�.
			; �᫨ ��᫥ ����᪠ ��ࠢ�� ���, �ਫ������ �뫮
			; ����饭� � ��ࠬ��ࠬ� �� ��������� ��ப�
	dd 0x0		; 8. 㪠��⥫� �� ��ப�, � ������ ����ᠭ ����,
			; ��㤠 ����饭� �ਫ������
;------------------------------------------------------------------------------
;---  ������ ���������	----------------------------------------------
;------------------------------------------------------------------------------
START:				;���� ��砫� �ணࠬ��
	mov	eax,40		;��⠭����� ���� ��� ��������� ᮡ�⨩.
	mov	ebx,100000b	;�㦭� ⮫쪮 ᮡ�⨥ ���
	int	0x40
;------------------------------------------------------------------------------
;---  ���� ��������� �������  ----------------------------------------
;------------------------------------------------------------------------------
align 4 
still:
	mov	eax,10		;������� ᮡ�⨩
	int	0x40
	
	mov	eax,14		;�㭪�� 14 - ������� ࠧ��� ��࠭�
	int	0x40		;�맮� �.14 � 横�� ��⮬� �� �����
				;���������� ࠧ�襭�� ��࠭�
	mov	ebx,eax
	shl	ebx,16
	shr	ebx,16
	mov	edi,ebx 	;��࠭�� �
	shr	eax,16		;ᤢ����� eax ��ࠢ� �� 16 - ����砥� x
	mov	esi,eax 	;��࠭�� x
;����砥� ���न���� �����
	mov	eax,37		;�㭪�� 37 - ࠡ�� � �����
	xor	ebx,ebx 	;������� 0 - ���न���� ���
				;�⭮�⥫쭮 ��࠭�
	int	0x40

	mov	ebx,eax 	;���������� ���न����
	shr	eax,16		;⥯��� � ��� ⮫쪮 x. �㦥� � y
	mov	ecx,eax 	;���������� x
	shl	ebx,16		;ᤢ���� ���� �� 16
	shr	ebx,16		;ᤢ����� �ࠢ� �� 16, � ��� ���� �
;------------------------------------------------------------------------------
;� esi � edi ���न���� ࠧ��� ��࠭� X � Y ᮮ⢥�ᢥ���
;� ecx � ebx ���न���� ����� X � Y ᮮ⢥⢥���
;------------------------------------------------------------------------------
;�ࠢ����� ���न��� x
	test	ecx,ecx 	;ࠢ�� 0?
	jz	left_border	;���室 � ��ࠡ�⪥ � ������ ���

	cmp	ecx,esi 	;ࠢ�� �ਭ� ��࠭�?
	jz	right_border	;���室�� � ��ࠡ�⪥ � �ࠢ��� ���
;�ࠢ����� ���न��� y
	test	ebx,ebx 	;ࠢ�� 0?
	jz	top_border	;�᫨ �� ����� ����� ������

	cmp	ebx,edi 	;ࠢ�� ���� ��࠭�?
	jz	bottom_border	;���室 � ��ࠡ�⪥ � ������� ���

	jmp	still		;���� ��祣� ������ �� �㦭�
;------------------------------------------------------------------------------
align 4 
left_border:
	mov	edx,esi 	;� edx ����� �ਭ� ��࠭�
	dec	edx		;㬥��訬 �� 1
	shl	edx,16		;⥯��� edx=(x-1)*65536
	add	edx,ebx 	;� ⥯��� edx=(x-1)*65536+y
;------------------------------------------------------------------------------
align 4 
set_mouse_position:
	mov	eax,18		;�㭪�� 18: ��⠭����� ����ன�� ���
	mov	ebx,19		;����㭪�� 19
	mov	ecx,4		;�������㭪�� 4: ��⠭����� ���������
				;�����
	int	0x40
	jmp	still		;���堥�
;------------------------------------------------------------------------------
align 4 
right_border:			;����� � �ࠢ��� ���

	xor	edx,edx
	inc	edx		;edx=1
	shl	edx,16		;edx = 1*65536
	add	edx,ebx 	;edx=1*65536+y
	jmp	set_mouse_position
;------------------------------------------------------------------------------
align 4 
top_border:			;����� � ���孥�� ���

	mov	edx,ecx 	;� ���न��� �����
	shl	edx,16		;⥯��� edx=(x)*65536
	add	edx,edi 	;� ⥯��� edx=(x)*65536+y
	dec	edx		;� ⥯��� edx=(x)*65536+(�-1)
	jmp	set_mouse_position
;------------------------------------------------------------------------------
align 4 
bottom_border:			;����� � ������� ���
	mov	edx,ecx 	;edx=ecx x ���न��� �����
	shl	edx,16		;edx = �*65536
	inc	edx		;� ���न��� ࠢ�� 1
	jmp	set_mouse_position
;------------------------------------------------------------------------------
IM_END: 			; ��⪠ ���� ����
;------------------------------------------------------------------------------
align 4
	rb 1024
stack_area:
;------------------------------------------------------------------------------
I_END:				; ��⪠ ���� �ணࠬ��
;------------------------------------------------------------------------------
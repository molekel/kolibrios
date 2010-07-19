;;===Pause_mode================================================================================================================

Pause_mode:

    mov  [action],  0
    mov  eax, [time_wait_limit]
    mov  [time_to_wait],    eax

  .redraw:
      mcall     12,1
    mov  ebx, [wp_x]
    shl  ebx, 16
    add  ebx, dword[window_width]
    mov  ecx, [wp_y]
    shl  ecx, 16
    add  ecx, dword[window_height]
      mcall     0, , ,[window_style], ,window_title

      call      Draw_decorations
      call      Draw_pause_picture
      call      Draw_pause_strings

      mcall     12,2
    
    
  .still:
      mcall     10                              ; wait for event
                                                ; ok, what an event?
    dec  al                                     ; has the window been moved or resized?
     jz  .redraw
    dec  al                                     ; was a key pressed?
     jz  .key


  .button:                                      ; a button was pressed
      mcall     17                              ; get button number
    shr  eax, 8                                 ; we should do it to get the real button code

    cmp  eax, 1
     je  Exit

     jmp .still


  .key:                                         ; a key was pressed
      mcall     2                               ; get keycode
    
    cmp  ah,  0x1B                              ; Escape - go to menu
     je  First_menu
    cmp  ah,  0x20                              ; Space - resume game
     je  Level_body
    
     jmp .still

;;---Pause_mode----------------------------------------------------------------------------------------------------------------


;;===Some_functions============================================================================================================

Draw_pause_picture:
    ;;===Draw_pause_picture========================================================================================================

    mov  ax,  0*0x100+29
    mov  cx,  4*0x100+6
    mov  edx, [pause_picture_color]
    mov  esi, picture_pause
      call      Draw_picture

    ret
            
    ;;---Draw_pause_picture--------------------------------------------------------------------------------------------------------
                
            
Draw_pause_strings:
    ;;===Draw_pause_strings================================================================================================

    mov  ebx, [window_width]
    shr  ebx, 1
    sub  ebx, (string_menu_esc-string_resume_space-1)*3+6
    shl  ebx, 16
    add  ebx, dword[bottom_middle_strings]
      mcall     4, ,[navigation_strings_color],string_resume_space ; Show 'RESUME - SPACE' string
      
    call    Draw_menu_esc                       ; Show 'MENU - ESC' string


    ret

    ;;---Draw_pause_strings------------------------------------------------------------------------------------------------
        
;;---Some_functions------------------------------------------------------------------------------------------------------------
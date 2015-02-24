#ifdef LANG_RUS
	?define WINDOW_TITLE_PROPERTIES "�����⢠"
	?define BTN_CLOSE "�������"
	?define PR_T_NAME "���:"
	?define PR_T_DEST "��ᯮ�������:"
	?define PR_T_SIZE "������:"
	?define SET_3 "������"
	?define SET_4 "�����"
	?define SET_5 "�������"
	?define SET_6 "������: "
	?define SET_7 " �����: "
	?define PR_T_CONTAINS "����ন�: "
	?define FLAGS " ���ਡ��� "
	?define PR_T_HIDDEN "������"
	?define PR_T_SYSTEM "���⥬��"
	?define PR_T_ONLY_READ "���쪮 �⥭��"
#elif LANG_EST
	?define WINDOW_TITLE_PROPERTIES "�����⢠"
	?define BTN_CLOSE "�������"
	?define PR_T_NAME "���:"
	?define PR_T_DEST "��ᯮ�������:"
	?define PR_T_SIZE "������:"
	?define SET_3 "������"
	?define SET_4 "�����"
	?define SET_5 "�������"
	?define SET_6 "������: "
	?define SET_7 " �����: "
	?define PR_T_CONTAINS "����ন�: "
	?define FLAGS " ���ਡ��� "
	?define PR_T_HIDDEN "������"
	?define PR_T_SYSTEM "���⥬��"
	?define PR_T_ONLY_READ "���쪮 �⥭��"
#else
	?define WINDOW_TITLE_PROPERTIES "Properties"
	?define BTN_CLOSE "Close"
	?define PR_T_NAME "Name:"
	?define PR_T_DEST "Destination:"
	?define PR_T_SIZE "Size:"
	?define SET_3 "Created"
	?define SET_4 "Opened"
	?define SET_5 "Modified"
	?define SET_6 "Files: "
	?define SET_7 " Folders: "
	?define PR_T_CONTAINS "Contains: "
	?define FLAGS " Attributes "
	?define PR_T_HIDDEN "Hidden"
	?define PR_T_SYSTEM "System"
	?define PR_T_ONLY_READ "Read-only"
#endif

dword mouse_ddd2;
char path_to_file[4096]="\0";
char file_name2[4096]="\0";
edit_box file_name_ed = {195,50,25,0xffffff,0x94AECE,0x000000,0xffffff,2,4098,#file_name2,#mouse_ddd2, 1000000000000000b,2,2};
edit_box path_to_file_ed = {145,100,46,0xffffff,0x94AECE,0x000000,0xffffff,2,4098,#path_to_file,#mouse_ddd2, 1000000000000000b,2,2};
frame flags_frame = { 0, 280, 10, 83, 106, 0x000111, 0xFFFfff, 1, FLAGS, 0, 0, 6, 0x000111, 0xCCCccc };

byte HIDDEN_chb,
     SYSTEM_chb,
     ONLY_READ_chb;

int file_count, dir_count, size_dir;
char folder_info[200];
BDVK file_info_general;
BDVK file_info_dirsize;

void GetSizeDir(dword way)
{
	dword dirbuf, fcount, i, filename;
	char cur_file[4096];
	if (isdir(way))
	{
		GetDir(#dirbuf, #fcount, way, DIRS_ONLYREAL);
		for (i=0; i<fcount; i++)
		{
			filename = i*304+dirbuf+72;
			strcpy(#cur_file, way);
			chrcat(#cur_file, '/');
			strcat(#cur_file, filename);
			if ( TestBit(ESDWORD[filename-40], 4) )
			{
				dir_count++;
				GetSizeDir(#cur_file);
			}
			else
			{
				GetFileInfo(#cur_file, #file_info_dirsize);
				size_dir = size_dir + file_info_dirsize.sizelo;
				file_count++;
			}
		}
	}
}

void properties_dialog()
{
	byte id;
	unsigned int key;
	dword file_name_off;
	proc_info settings_form;
	
	strcpy(#folder_info, "\0");
	file_count = 0;
	dir_count = 0;	
	size_dir = 0;
	GetFileInfo(#file_path, #file_info_general);
	strcpy(#file_name2, #file_name);
	file_name_ed.size = strlen(#file_name2);
	strcpy(#path_to_file, #path);
	path_to_file_ed.size = strlen(#path_to_file);

	if (itdir) GetSizeDir(#file_path);
	
	SetEventMask(0x27);
	loop() switch(WaitEvent())
	{
		case evButton: 
				id=GetButtonID();
				IF (id==1) || (id==10) ExitProcess();
				break;
				
		case evMouse:
				edit_box_mouse stdcall (#file_name_ed);
				edit_box_mouse stdcall (#path_to_file_ed);
				break;
			
		case evKey:
				key = GetKey();
				IF (key==27) ExitProcess();
				EAX=key<<8;
				edit_box_key stdcall(#file_name_ed);
				edit_box_key stdcall(#path_to_file_ed);
				break;
				
		case evReDraw:
				DefineAndDrawWindow(Form.left + 150,150,270,240+GetSkinHeight(),0x34,sc.work,WINDOW_TITLE_PROPERTIES);
				GetProcessInfo(#settings_form, SelfInfo);
				DrawFlatButton(settings_form.cwidth - 70 - 13, settings_form.cheight - 34, 70, 22, 10, 0xE4DFE1, BTN_CLOSE);
				DrawBar(10, 10, 32, 32, 0xFFFfff);
				if (! TestBit(file_info_general.attr, 4) ) 
					Put_icon(#file_name2+strrchr(#file_name2,'.'), 18, 20, 0xFFFfff, 0);
				else 
					Put_icon("<DIR>", 18, 20, 0xFFFfff, 0);

				WriteText(50, 13, 0x80, 0x000000, PR_T_NAME);				
				edit_box_draw stdcall (#file_name_ed);

				WriteText(10, 50, 0x80, 0x000000, PR_T_DEST);
				edit_box_draw stdcall (#path_to_file_ed);

				/*WriteText(10, 63, 0x80, 0x000000, SET_3);
				if (!itdir)
				{
					WriteText(10, 78, 0x80, 0x000000, SET_4);
					WriteText(10, 93, 0x80, 0x000000, SET_5);					
				}*/
				WriteText(10, 65, 0x80, 0x000000, PR_T_SIZE);
				if (!itdir)
				{
					WriteText(100, 65, 0x80, 0x000000, ConvertSize(file_info_general.sizelo));
				}
				else
				{
					WriteText(10, 80, 0x80, 0x000000, PR_T_CONTAINS);				
					strcpy(#folder_info, SET_6);
					strcat(#folder_info, itoa(file_count));
					strcat(#folder_info, SET_7);
					strcat(#folder_info, itoa(dir_count));
					WriteText(100, 80, 0x80, 0x000000, #folder_info);
					WriteText(100, 65, 0x80, 0x000000, ConvertSize(size_dir));
				}

				flags_frame.size_x = - flags_frame.start_x * 2 + settings_form.cwidth - 2;
				flags_frame.font_color = sc.work_text;
				flags_frame.font_backgr_color = sc.work;
				flags_frame.ext_col = sc.work_graph;
				frame_draw stdcall (#flags_frame);

				DrawPropertiesCheckBoxes();
	}
}

void DrawPropertiesCheckBoxes()
{
	ONLY_READ_chb = TestBit(file_info_general.attr, 0);
	HIDDEN_chb = TestBit(file_info_general.attr, 1);
	SYSTEM_chb = TestBit(file_info_general.attr, 2);
	CheckBox2(22, 120, 20, PR_T_ONLY_READ,  ONLY_READ_chb);
	CheckBox2(22, 142, 21, PR_T_HIDDEN,  HIDDEN_chb);
	CheckBox2(22, 164, 22, PR_T_SYSTEM,  SYSTEM_chb);
}
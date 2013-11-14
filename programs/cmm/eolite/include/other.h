//Leency 2008-2013


dword onLeft(dword right,left) {EAX=Form.width-right-left;}
dword onTop(dword down,up) {EAX=Form.height-GetSkinHeight()-down-up;}


void ShowMessage(dword message, pause_duration)
{
	int form_x=files.w-220/2+files.x;
	int form_y=160;
	DrawPopup(form_x,form_y,220,80,1,sc.work,sc.work_graph);
	WriteText(-strlen(message)*3+110+form_x,80/2-4+form_y,0x80,sc.work_text,message);
	pause(pause_duration);
	if (pause_duration) List_ReDraw();
}

inline fastcall signed int _strrchr( ESI,BL)
{
	int jj=0, last=strlen(ESI);
	do{
		jj++;
		$lodsb
		IF(AL==BL) last=jj;
	} while(AL!=0);
	return last;
}


dword col_palette[14] = {0xD2D3D3,0xD4D4D4,0xD6D5D6,0xD8D7D8,0xDAD8D9,0xDCDADB,
0xDFDCDD,0xE1DDDE,0xE2DEE0,0xE4DFE1,0xE3DFE1,0xE3DFE1,0xE3DFE1,0xE3DFE1,0xE3DFE1};

inline fastcall void Scroll() { //���������
	dword on_y, i;
	if (files.count<=0)
	{
		on_y = 57;
		scroll_size = onTop(22,58);
	}
	else
	{
		on_y = files.first * onTop(22,57) / files.count +57;
		scroll_size=onTop(22,57) * files.visible - files.visible / files.count;
		if (scroll_size<20) scroll_size = 20; //������������� ����������� ������ �������
		if (scroll_size>onTop(22,57)-on_y+56) || (files.first+files.visible>=files.count) on_y=onTop(23+scroll_size,0); //��� �������� ������ 
	}
	DrawFlatButton(onLeft(27,0),on_y,16,scroll_size,0,-1,"");//��������
	if (!scroll_used) for (i=0; i<13; i++) DrawBar(onLeft(25-i,0), on_y+2, 1, scroll_size-3, col_palette[13-i]);
	if (scroll_used)  for (i=0; i<13; i++) DrawBar(onLeft(25-i,0), on_y+2, 1, scroll_size-3, col_palette[i]);
	//���� �� ��������
	if (on_y>58) DrawBar(onLeft(26,0),57,15,1, 0xC7C9C9);
	DrawBar(onLeft(26,0),58,1, on_y-58,0xC7C9C9);
	DrawBar(onLeft(25,0),58,14,on_y-58,0xCED0D0);
	//���� ����� ��������
	if (onTop(22,57)-scroll_size+55>on_y) DrawBar(onLeft(26,0),on_y+scroll_size+1,15,1,0xC7C9C9);
	DrawBar(onLeft(26,0),on_y+scroll_size+2,1,onTop(22,57)-scroll_size-on_y+55,0xC7C9C9);
	DrawBar(onLeft(25,0),on_y+scroll_size+2,14,onTop(22,57)-scroll_size-on_y+55,0xCED0D0);
}

void DrawFlatButton(dword x,y,width,height,id,color,text)
{
	int fill_h;
	DrawRectangle(x,y,width,height,sc.work_graph);
	DrawRectangle3D(x+1,y+1,width-2,height-2,0xFEFEFE,col_padding);
	PutPixel(x+width-1, y+1, col_padding);
	if (color!=-1) DrawFilledBar(x+2, y+2, width-3, height-3);
	IF (id<>0)	DefineButton(x+1,y+1,width-2,height-2,id+BT_HIDE,0xEFEBEF);
	WriteText(-strlen(text)*6+width/2+x+1,height/2-3+y,0x80,sc.work_text,text);
}

void DrawFilledBar(dword x, y, w, h)
{
	int i, fill_h;
	if (h <= 14) fill_h = h; else fill_h = 14;
	for (i=0; i<fill_h; i++) DrawBar(x, y+i, w, 1, col_palette[14-i]);	
	DrawBar(x, y+i, w, h-fill_h, col_palette[14-i]);		
}






struct rd_info
{
	dword function_number, reserved[4];
	char path[4];
} rd_info;

#define ALL_RD_CLUSTERS 2847
int GetFreeRamDiskClusters()
{
	dword free_size;
	static dword old_free_size;

	rd_info.function_number = 15;
	strcpy(#rd_info.path, "/rd");
	$mov eax,58
	$mov ebx, #rd_info;
	$int 0x40
	if (EAX==0)
	{
		free_size=ECX;
		old_free_size = ECX;
	}
	else
	{
		debugi(EAX);
		free_size = old_free_size;
	}
	return free_size;
}
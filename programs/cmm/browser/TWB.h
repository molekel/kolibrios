
int	downloader_id;

dword
	buf,
	filesize,
	blink;

char download_path[]="/rd/1/.download";
char search_path[]="http://nigma.ru/index.php?s=";
char version[]=" Text-based Browser 0.99.02";


struct TWebBrowser {
	int left, top, width, height, line_h;
	void Scan(int);
	void GetNewUrl();
	void OpenPage();
	void ReadHtml(byte);
	void ShowPage();
	void ParseHTML(dword);
	void WhatTextStyle(int left1, top1, width1);
	void DrawPage();
	void DrawScroller();
};	
TWebBrowser WB1;

byte rez, b_text, i_text, u_text, s_text, pre_text, blq_text, li_text,
	link, ignor_text, li_tab, cur_encoding;

dword text_colors[300],
	text_color_index,
	link_color,
	bg_color;

int stroka,
	stolbec,
	tab_len,
	anchor_line_num;
	
char line[500],
	tag[100],
	tagparam[10000],
	parametr[1200],
	options[4096],
	anchor[256];

#include "include\history.h"
#include "include\colors.h"
#include "include\unicode_tags.h"
#include "include\img_cache.h"
#include "include\some_code.h"
#include "include\parce_tag.h"

//=======================================================================
dword drawbuf;
void DrawBufInit()
{
	free(drawbuf);
	drawbuf = malloc(WB1.width * WB1.line_h +4 * 4 + 8); //+1 for good luck
	ESDWORD[drawbuf] = WB1.width;
	ESDWORD[drawbuf+4] = WB1.line_h;
}
void DrawBufFill()
{
	int i;
	for (i=0; i<WB1.width* WB1.line_h +4 * 4; i+=4) ESDWORD[drawbuf+i+8] = bg_color;
}
void DrawBufBar(dword x, y, w, h, color)
{
	int i, j;
	for (j=0; j<h; j++)
	{
		for (i = y+j*WB1.width+x*4; i<y+j*WB1.width+x+w*4 ; i+=4) ESDWORD[drawbuf+i+8] = color;
	}
}

char shift[]={8,8,4,4};
void DrawBufSkew(dword x, y, w, h)
{
	int i, j;
	stolbec++;
	for (j=0; j<=3; j++)
	{
		for (i = y+j*WB1.width+x+w+h*4; i>y+j*WB1.width+x+h-12*4 ; i-=4)
								ESDWORD[drawbuf+i+8] = ESDWORD[-shift[j]+drawbuf+i+8];
	}
}

void TextGoDown(int left1, top1, width1)
{
	if (!stroka) DrawBar(WB1.left, WB1.top, WB1.width, 5, bg_color); //����������� ��� ��� ������ �������
	stroka++;
	if (blq_text) stolbec = 8; else stolbec = 0;
	if (li_text) stolbec = li_tab * 5;
	if (top1>=WB1.top) && ( top1 < WB1.height+WB1.top-10)  && (!anchor)
	{
		PutPaletteImage(drawbuf+8, WB1.width, WB1.line_h, left1-5, top1, 32,0);
		DrawBufFill();
	}
}

void TWebBrowser::DrawPage()
{
	int start_x, start_y, line_length, magrin_left=5;
	
	if (!header)
	{
		strcpy(#header, #line);
		strcat(#header, " -");
		strcat(#header, #version);
		line = 0;
		return;
	}
	
	if (stroka >= 0) && (stroka - 2 < lines.visible) && (line) && (!anchor)
	{
		start_x = stolbec * 6 + left + magrin_left;
		start_y = stroka * 10 + top + magrin_left;
		line_length = strlen(#line) * 6;

		if (use_truetype == 1)
		{
			//line_length =  get_length stdcall (#line,-1,16,line_length);
			text_out stdcall (#line, #fontlol, 17, text_colors[text_color_index], start_x, start_y-3);
		}
		else
		{
			WriteBufText(start_x, 0, 0x88, text_colors[text_color_index], #line, drawbuf);
		}
		IF (b_text)	WriteBufText(start_x+1, 0, 0x88, text_colors[text_color_index], #line, drawbuf);
		IF (i_text) DrawBufSkew(start_x, 0, line_length, line_h);
		IF (s_text) DrawBufBar(start_x, 4, line_length, 1, text_colors[text_color_index]);
		IF (u_text) DrawBufBar(start_x, 8, line_length, 1, text_colors[text_color_index]);
		IF (link) {
			UnsafeDefineButton(start_x-2, start_y, line_length + 3, 9, blink + BT_HIDE, 0xB5BFC9);
			DrawBufBar(start_x, 8, line_length, 1, text_colors[text_color_index]);
		}
		stolbec += strlen(#line);
	}
}
//=======================================================================

void TWebBrowser::Scan(int id)
{
	if (id >= 400)
	{
		GetURLfromPageLinks(id);
		
		//#1
		if (URL[0] == '#')
		{
			strcpy(#anchor, #URL+strrchr(#URL, '#'));
			
			strcpy(#URL, BrowserHistory.CurrentUrl());
			
			lines.first=lines.all-lines.visible;
			ShowPage();
			return;
		}
		//liner.ru#1
		if (strrchr(#URL, '#')<>-1)
		{
			strcpy(#anchor, #URL+strrchr(#URL, '#'));
			URL[strrchr(#URL, '#')-1] = 0x00; //��������
		}
		
		GetNewUrl();
		
		if (!strcmp(#URL + strlen(#URL) - 4, ".gif")) || (!strcmp(#URL + strlen(#URL) - 4, ".png")) || (!strcmp(#URL + strlen(#URL) - 4, ".jpg"))
		{
			//if (strstr(#URL,"http:")) 
			RunProgram("/sys/media/kiv", #URL);
			strcpy(#editURL, BrowserHistory.CurrentUrl());
			strcpy(#URL, BrowserHistory.CurrentUrl());
			return;
		}
		if (!strcmpn(#URL,"mailto:", 7))
		{
			RunProgram("@notify", #URL);
			strcpy(#editURL, BrowserHistory.CurrentUrl());
			strcpy(#URL, BrowserHistory.CurrentUrl());
			return;
		}

		OpenPage();
		return;
	}
	
	IF(lines.all < lines.visible) SWITCH(id) //���� ���� ����� ���������� ��������� ������
	{ CASE 183: CASE 184: CASE 180: CASE 181: return; } 
	
	switch (id)
	{
		case 011: //Ctrk+K 
			ReadHtml(_KOI);
			break;
		case 021: //Ctrl+U
			ReadHtml(_UTF);
			break;
		case 004: //Ctrl+D
			ReadHtml(_DOS);
			break;
		case 002: //free img cache
			FreeImgCache();
			break;
		case 003:
			if (!pre_text) pre_text=2;
				else pre_text=0;
			break;			
		case 005: //truetype
			if (use_truetype == 2) 
			{
				RunProgram("@notify", "Library does not exists /rd/1/lib/truetype.obj"w);
				return;
			}
			if (use_truetype == 1) use_truetype=0; else use_truetype=1;
			break;
		case BACK:
			if (!BrowserHistory.GoBack()) return;
			OpenPage();
			return;
		case FORWARD:
			if (!BrowserHistory.GoForward()) return;
			OpenPage();
			return;
		case 255:  //F12
			RunProgram("/rd/1/HTMLv_old", #URL);
			return;
		case 052:  //F3
			if (strcmp(get_URL_part(5),"http:")<>0) RunProgram("/rd/1/tinypad", #URL); else RunProgram("/rd/1/tinypad", #download_path);
			return;
		case 054: //F5
			IF(address_box.flags & 0b10) break;
		case REFRESH:
			if (GetProcessSlot(downloader_id)<>0)
			{
				KillProcess(downloader_id);
				pause(20);
				Draw_Window();
				return;
			}
			anchor_line_num=lines.first; //������ ������� :�
			anchor[0]='|';
			OpenPage();
			return;
		case 014: //Ctrl+N ����� ����
		case 020: //Ctrl+T ����� �������
		case NEWTAB:
			MoveSize(190,80,OLD,OLD);
			RunProgram(#program_path, #URL);
			return;
			
		case HOME:
			strcpy(#editURL, "http://kolibri-n.org/index.php");
		case GOTOURL:
		case 0x0D: //enter
			strcpy(#URL, #editURL);
			OpenPage();
			return;
		case 173:	//ctrl+enter
		case SEARCHWEB:
			strcpy(#URL, #search_path);
			strcat(#URL, #editURL);
			OpenPage();
			return;

		case ID1: //������ �����
			IF(lines.first <= 0) return;
			lines.first--;
			break; 
		case ID2: //������ ����
			IF(lines.visible + lines.first >= lines.all) return;
			lines.first++;
			break; 
		case 183: //PgDown
			IF(lines.first == lines.all - lines.visible) return;
			lines.first += lines.visible + 2;
			IF(lines.visible + lines.first > lines.all) lines.first = lines.all - lines.visible;
			break;
		case 184: //PgUp
			IF(lines.first == 0) return;
			lines.first -= lines.visible - 2;
			IF(lines.first < 0) lines.first = 0;
			break;
		case 180: //home
			IF(lines.first == 0) return;
			lines.first = 0;
			break; 
		case 181: //end
			IF (lines.first == lines.all - lines.visible) return;
			lines.first = lines.all - lines.visible;
			break; 
		default:
			return;
	}
	ParseHTML(buf);
}


char *ABSOLUTE_LINKS[]={ "http:", "mailto:", "ftp:", "/sys/", "/rd/", "/fd/", "/bd/", "/hd/", "/cd/", "/tmp/", 0};
//dword TWebBrowser::GetNewUrl(dword CUR_URL, NEW_URL){
void TWebBrowser::GetNewUrl(){
	int i, len;
	
	for (i=0; ABSOLUTE_LINKS[i]; i++)
	{
		len=strlen(ABSOLUTE_LINKS[i]);
		if (!strcmpn(#URL, ABSOLUTE_LINKS[i], len)) return;
	}
		
	IF (!strcmpn(#URL,"./", 2)) strcpy(#URL, #URL+2); //������� :)
	if (URL[0] == '/') strcpy(#URL, #URL+1);

	strcpy(#editURL, BrowserHistory.CurrentUrl()); //������ ����� ������� ��������
		
	_CUT_ST_LEVEL_MARK:
		
		if (editURL[strrchr(#editURL, '/')-2]<>'/')  // ���� �� http://
		{
			editURL[strrchr(#editURL, '/')] = 0x00; //�������� � ��� �� ���������� /
		}
		
		IF (!strcmp(get_URL_part(3),"../")) //�� ������� �����
		{
			strcpy(#URL,#URL+3);
			editURL[strrchr(#editURL, '/')-1] = 0x00; //�������� � ��� �� ���������� /
			goto _CUT_ST_LEVEL_MARK;
		}
		
		if (editURL[strlen(#editURL)-1]<>'/') strcat(#editURL, "/"); 
		
		strcat(#editURL, #URL); //����� ����� �����
		strcpy(#URL, #editURL);
}


	
void TWebBrowser::ReadHtml(byte encoding)
{
	if (!strcmp(get_URL_part(5),"http:"))) 
		file_size stdcall (#download_path);
	else
		file_size stdcall (#URL);
	
	filesize = EBX;
	if (!filesize) return;
	
	mem_Free(buf);
	buf = mem_Alloc(filesize);
	if (!strcmp(get_URL_part(5),"http:"))) 
		ReadFile(0, filesize, buf, #download_path);
	else
		ReadFile(0, filesize, buf, #URL);
		
	cur_encoding = encoding;
	if (encoding==_WIN) wintodos(buf);
	if (encoding==_UTF) utf8rutodos(buf);
	if (encoding==_KOI) koitodos(buf);
}


void TWebBrowser::OpenPage()
{
	if (GetProcessSlot(downloader_id)<>0) PutPaletteImage(#toolbar,200,42,0,0,8,#toolbar_pal);
	KillProcess(downloader_id);
	strcpy(#editURL, #URL);
	BrowserHistory.AddUrl();
	strcpy(#header, #version);
	pre_text =0;
	if (!strcmp(get_URL_part(5),"http:")))
	{
		KillProcess(downloader_id); //������� ������ �������
		DeleteFile(#download_path);
		IF (URL[strlen(#URL)-1]=='/') URL[strlen(#URL)-1]=NULL;
		downloader_id = RunProgram("/sys/network/downloader", #URL);
		//Browser Hack v2.0
		pause(60);
		if (GetProcessSlot(downloader_id)<>0)
		{
			debug("Browser Hack v2.0: Killing downloader and trying to run it one more!");
			KillProcess(downloader_id); //������� ������ �������
			downloader_id = RunProgram("/sys/network/downloader", #URL);
		}
		//
		IF (downloader_id<0) RunProgram("@notify", "Error running Downloader. Internet unavilable.");
		Draw_Window();
		return;
	}
	lines.first = lines.all =0;
	ReadHtml(_WIN);
	WB1.ShowPage();
}


void TWebBrowser::ShowPage()
{
	address_box.size = address_box.pos = strlen(#editURL);
	address_box.offset=0;
	edit_box_draw stdcall(#address_box);

	if (!filesize)
	{
		DrawBar(left, top, width+scroll1.size_x+1, height, 0xFFFFFF); //fill all
		if (GetProcessSlot(downloader_id)<>0) WriteText(left + 10, top + 18, 0x80, 0, "Loading...");
		else
		{
			WriteText(left + 10, top + 18, 0x80, 0, "Page not found. May be, URL contains some errors.");
			if (!strcmp(get_URL_part(5),"http:"))) WriteText(left + 10, top + 32, 0x80, 0, "Or Internet unavilable for your configuration.");
		}
		//return;
	}
	else
		ParseHTML(buf);

	if (!header) strcpy(#header, #version);
	if (!strcmp(#version, #header)) DrawTitle(#header);
}



void TWebBrowser::ParseHTML(dword bword){
	word bukva[2];
	int j, perenos_num;
	byte ignor_param;
	char temp[768];
	
	if (blink<400) blink=400; else for ( ; blink>400; blink--;) DeleteButton(blink);
	b_text = i_text = u_text = s_text = blq_text = 
	li_text = link = ignor_text = text_color_index = text_colors[0] = li_tab = 0; //�������� ����
	link_color = 0x0000FF;
	bg_color = 0xFFFFFF;
	DrawBufFill();
	line = NULL;
	strcpy(#page_links,"|");
	strcpy(#header, #version);
	stroka = -lines.first;
	stolbec = 0;

	if (pre_text<>2)
	{
		pre_text=0;
		if (!strcmp(#URL + strlen(#URL) - 4, ".txt")) pre_text = 1;
		if (!strcmp(#URL + strlen(#URL) - 4, ".mht")) ignor_text = 1;
	}
	
	for ( ; buf+filesize > bword; bword++;)
	{
		bukva = ESBYTE[bword];
		if (ignor_text) && (bukva<>'<') continue;
		switch (bukva)
		{
		case 0x0a:
			if (pre_text)
			{
				bukva = temp = NULL;
				goto NEXT_MARK;
			}
		case '\9':
			if (pre_text) //����� ��� �� 0x0d	
			{
				tab_len=strlen(#line)/8;
				tab_len=tab_len*8;
				tab_len=8+tab_len-strlen(#line);
				for (j=0; j<tab_len; j++;) chrcat(#line,' ');
				break;
			}
			goto DEFAULT_MARK;		
		case '=': //quoted printable
			if (strcmp(#URL + strlen(#URL) - 4, ".mht")<>0) goto DEFAULT_MARK;

			temp[0] = ESBYTE[bword+1];
			temp[1] = ESBYTE[bword+2];
			temp[2] = '\0';
			if (bukva = Hex2Symb(#temp))
			{
				bword+=2;
				goto DEFAULT_MARK;
			}
			break;
			
		case '&': //&nbsp; and so on
			bword++;
			tag=0;
			for (j=0; (ESBYTE[bword]<>';') && (j<7);   j++, bword++;)
			{
				bukva = ESBYTE[bword];
				chrcat(#tag, bukva);
			}
			
			bukva = GetUnicodeSymbol();
			if (bukva) goto DEFAULT_MARK;
			break;
		case '<':
			bword++; //��������� ������ <
			tag = parametr = tagparam = ignor_param = NULL;
			if (ESBYTE[bword] == '!') //���������� ������ <!-- -->, ������
			{
				bword++;
				if (ESBYTE[bword] == '-')
				{
				HH_:
					do
					{
						bword++;
						if (buf + filesize <= bword) break 2;
					}
					while (ESBYTE[bword] <>'-');
					
					bword++;
					if (ESBYTE[bword] <>'-') goto HH_;
				}
			}
			while (ESBYTE[bword] !='>') && (bword < buf + filesize) //�������� ��� � ��� ���������
			{
				bukva = ESBYTE[bword];
				if (bukva == '\9') || (bukva == '\x0a') || (bukva == '\x0d') bukva = ' ';
				if (!ignor_param) && (bukva <>' ')
				{
					if (strlen(#tag)<sizeof(tag)) strcat(#tag, #bukva);
				}
				else
				{
					ignor_param = true;
					if (!ignor_text) && (strlen(#tagparam)+1<sizeof(tagparam)) strcat(#tagparam, #bukva);
				}
				bword++;
			}
			strlwr(#tag);
			strlwr(#tagparam);

			if (tag[strlen(#tag)-1]=='/') tag[strlen(#tag)-1]=NULL; //for br/
			if (tagparam) && (strlen(#tagparam) < 4000) GetNextParam();

			if (stolbec + strlen(#line) > lines.column_max) //============the same as NEXT_MARK
			{
				perenos_num = strrchr(#line, ' ');
				if (!perenos_num) && (strlen(#line)>lines.column_max) perenos_num=lines.column_max;
				strcpy(#temp, #line + perenos_num); //������� �� ������
				line[perenos_num] = 0x00;
				if (stroka-1 > lines.visible) && (lines.first <>0) break 1; //������...
				DrawPage();
				strcpy(#line, #temp);				
				TextGoDown(left + 5, stroka * 10 + top + 5, width - 20); //����������� �������� ������
			}
			DrawPage();
			line=NULL;

			if (tag) WhatTextStyle(left + 5, stroka * 10 + top + 5, width - 20); //��������� �����

			tag = parametr = tagparam = ignor_param = NULL;
			break;
		default:
			DEFAULT_MARK:
			if (bukva<=15) bukva=' ';
			if (!pre_text) && (bukva == ' ')
			{
				if (line[strlen(#line)-1]==' ') break; //������ 2 ������� ������
				if (!stolbec) && (!line) break; //������ �� ����� ���������� � �������
			}
			if (strlen(#line)<sizeof(line)) chrcat(#line, bukva);

			if (stolbec + strlen(#line) > lines.column_max)
			{
			NEXT_MARK:
				perenos_num = strrchr(#line, ' ');
				if (!perenos_num) && (strlen(#line)>lines.column_max) perenos_num=lines.column_max;
				strcpy(#temp, #line + perenos_num); //������� �� ������
				line[perenos_num] = 0x00;
				if (stroka-1 > lines.visible) && (lines.first <>0) break 1; //������...
				DrawPage();
				strcpy(#line, #temp);			
				TextGoDown(left + 5, stroka * 10 + top + 5, width - 20); //����������� �������� ������
			}
		}
	}

	DrawPage(); //������ ��������� ������, ����� ��� ���� ������, ������������� ���
	TextGoDown(left + 5, stroka * 10 + top + 5, width - 20); //����������� �������� ������

	if (lines.visible * 10 + 25 <= height)
		DrawBar(left, lines.visible * 10 + top + 25, width, -lines.visible * 10 + height - 25, bg_color);
	if (stroka * 10 + 5 <= height)
		DrawBar(left, stroka * 10 + top + 5, width, -stroka * 10 + height - 5, bg_color); //����������� �� �� �����
	if (lines.first == 0) lines.all = stroka;
	if (anchor) //���� ������� ������ �������� ����� ����� - ����� ����������� ����
	{
		anchor=NULL;
		lines.first=anchor_line_num;
		ParseHTML(buf);
	}
	DrawScroller();
}



char oldtag[100];
void TWebBrowser::WhatTextStyle(int left1, top1, width1) {
	dword hr_color;

	//��������� ��� ����������� ��� �����������
	if (tag[0] == '/') 
	{
		 rez = 0;
		 strcpy(#tag, #tag+1);
	}
	else rez = 1;
		
	if (!chTag("html"))
	{
		IF(!strcmp(#URL + strlen(#URL) - 4, ".mht")) IF (rez==0) ignor_text = 1; ELSE ignor_text = 0;
		return;
	}

	if (!chTag("script")) || (!chTag("style")) || (!chTag("binary")) ignor_text = rez;

	if(!chTag("title"))
	{
		if (rez) header=NULL;
		else if (!stroka) DrawTitle(#header); //��� �������� - ������ ������
		return;
	}

	if (ignor_text) return;


	
	IF(!chTag("q")) strcat(#line, "\"");

	if (anchor) && (!strcmp(#parametr, "id=")) //����� �����!!! ������ ��� ���� �� ��������� ���, �������� �� �����
	{
		if (!strcmp(#anchor, #options))	anchor_line_num=lines.first+stroka;
	}
	
	if (!chTag("body"))
	{
		do{
			if (!strcmp(#parametr, "link=")) link_color = GetColor(#options);
			if (!strcmp(#parametr, "text=")) text_colors[0]=GetColor(#options);
			if (!strcmp(#parametr, "bgcolor="))
			{
				bg_color=GetColor(#options);
				DrawBufFill();
			}
		} while(GetNextParam());
		return;
	}

	if (!chTag("a"))
	{
		if (rez)
		{
			if (link) IF(text_color_index > 0) text_color_index--; //���� ���������� ��� � �� ��� ������

			do{
				if (!strcmp(#parametr, "href="))
				{
					if (stroka - 1 > lines.visible) || (stroka < -2) return;
					
					text_color_index++;
					text_colors[text_color_index] = text_colors[text_color_index-1];
					
					link = 1;
					blink++;
					text_colors[text_color_index] = link_color;
					strcat(#page_links, #options);
					strcat(#page_links, "|");
				}
				if (anchor) && (!strcmp(#parametr, "name="))
				{
					if (!strcmp(#anchor, #options))
					{
						anchor_line_num=lines.first+stroka;
					}
				}
			} while(GetNextParam());
		}
		else {
			link = 0;
			IF(text_color_index > 0) text_color_index--;
		}
		return;
	}

	if (!chTag("font"))
	{
		if (rez)
		{
			text_color_index++;
			text_colors[text_color_index] = text_colors[text_color_index-1];
		
			do{
				if (strcmp(#parametr, "color=") == 0) //&& (parametr[1] == '#')
				{
					text_colors[text_color_index] = GetColor(#options);
				}
			} while(GetNextParam());
		}
		else
			if (text_color_index > 0) text_color_index--;
		return;
	}
	if(!chTag("tr")) || (!chTag("br")) {
		TextGoDown(left1, top1, width1);
		return;
	}
	if (!chTag("div")) {
		IF(oldtag[0] <>'h') TextGoDown(left1, top1, width1);
		return;
	}
	if (!chTag("p")) {
		IF(oldtag[0] == 'h') return;
		TextGoDown(left1, top1, width1);
		IF(rez) TextGoDown(left1, top1 + 10, width1);
		return;
	}
	if (!chTag("h1")) || (!chTag("h2")) || (!chTag("h3")) || (!chTag("h4")) {
		TextGoDown(left1, top1, width1);
		IF(rez) TextGoDown(left1, top1 + 10, width1);
		b_text = rez;
		strcpy(#oldtag, #tag);
		return;
	}
	else
		oldtag=NULL;
		
	if (!chTag("b")) || (!chTag("strong")) || (!chTag("big")) {
		b_text = rez;
		return;
	}
	if(!chTag("i")) || (!chTag("em")) || (!chTag("subtitle")) {
		i_text = rez;
		return;
	}	
	if (!chTag("dt"))
	{
		li_text = rez;
		IF(rez == 0) return;
		TextGoDown(left1, top1, width1);
		return;
	}
	if(!chTag("li")) || (!chTag("dt")) //���� ������� ��������� ������
	{
		li_text = rez;
		if (rez)
		{
			TextGoDown(left1, top1, width1);
			if (stroka > -1) && (stroka - 2 < lines.visible) DrawBufBar(li_tab * 5 * 6 + left1 - 5, line_h/2-3, 2, 2, 0x555555);
		}
		return;
	}
	if (!chTag("u")) || (!chTag("ins")) u_text = rez;
	if (!chTag("s")) || (!chTag("strike")) || (!chTag("del")) s_text = rez;
	if (!chTag("ul")) || (!chTag("ol")) IF(!rez)
	{
		li_text = rez;
		li_tab--;
		TextGoDown(left1, top1, width1);
	} ELSE li_tab++;
	if (!chTag("dd")) stolbec += 5;
	if (!chTag("blockquote")) blq_text = rez;
	if (!chTag("pre")) pre_text = rez; 
	if (!chTag("hr"))
	{
		if (anchor) || (stroka < -1)
		{
			stroka+=2;
			return;
		}
		if (strcmp(#parametr, "color=") == 0) hr_color = GetColor(#options); else hr_color = 0x999999;
		TextGoDown(left1, top1, width1);
		DrawBufBar(5, WB1.line_h/2, WB1.width-10, 1, hr_color);
		TextGoDown(left1, top1+WB1.line_h, width1);
	}
	/*
	if (!chTag("input"))
	{
		do{
			if (!strcmp(#parametr, "type="))
			{
				if ((!strcmp(#options, "radio")) || (!strcmp(#options, "checkbox")))
				{
					if (!anchor) && (stroka > 0) CheckBox(stolbec*6 + left1,top1-2,10,10, 0, "\0", 0x888888, text_colors[text_color_index], 0);
					stolbec+=2;
				}
				if ((!strcmp(#options, "text")) || (!strcmp(#options, "password")))
				{
					if (!anchor) && (stroka > 0) CheckBox(stolbec*6 + left1,top1-2,90,10, 0, "\0", 0x555555, 0, 0);
					stolbec+=16;
				}
				if ((!strcmp(#options, "button")) || (!strcmp(#options, "file")) || (!strcmp(#options, "submit")))
				{
					if (!anchor) && (stroka > 0) DrawCaptButton(stolbec*6 + left1,top1-2,60,10, 0, 0xCCCccc, 0, "Button");
					stolbec+=21;
				}
			}
		} while(GetNextParam());
	}
	*/
	if (!chTag("img"))
	{
		Images( left1, top1, width1);
		return;
	}
	if (!chTag("meta")) || (!chTag("?xml"))
	{
		do{
			if (!strcmp(#parametr, "charset=")) || (!strcmp(#parametr, "content=")) || (!strcmp(#parametr, "encoding="))
			{
				strcpy(#options, #options[strrchr(#options, '=')]); //����� � content=
				if (!strcmp(#options,"utf-8"))   || (!strcmp(#options,"utf8"))      ReadHtml(_UTF);
				if (!strcmp(#options, "koi8-r")) || (!strcmp(#options, "koi8-u"))   ReadHtml(_KOI);
				if (!strcmp(#options, "dos"))    || (!strcmp(#options, "cp-866"))   ReadHtml(_DOS);
			}
		} while(GetNextParam());
		return;
	}
}



void TWebBrowser::DrawScroller() //�� ����������� ���������, �� ���� � ����� �����
{
	scroll1.max_area = lines.all;
	scroll1.cur_area = lines.visible;
	scroll1.position = lines.first;

	scroll1.all_redraw=1;
	scroll1.start_x = WB1.left + WB1.width;
	scroll1.size_y=WB1.height;

	scrollbar_v_draw(#scroll1);
}


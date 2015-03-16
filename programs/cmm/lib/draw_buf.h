
dword buf_data;
dword zbuf_data;


struct DrawBufer {
	int bufx, bufy, bufw, bufh;
	int zbufx, zbufy, zbufw, zbufh;
	byte zoomf;

	void Init();
	void Show();
	void Fill();
	void Skew();
	void DrawBar();
	void PutPixel();
	void AlignCenter();
	void AlignRight();
	void Zoom2x();
};

void DrawBufer::Init(int i_bufx, i_bufy, i_bufw, i_bufh)
{
	bufx = i_bufx;
	bufy = i_bufy;
	bufw = i_bufw; 
	bufh = i_bufh;
	free(buf_data);
	buf_data = malloc(bufw * bufh * 4 + 8);
	ESDWORD[buf_data] = bufw;
	ESDWORD[buf_data+4] = bufh;

	if (zoomf != 1)
	{
		zbufx = bufx;
		zbufy = bufy;
		zbufw = bufw * zoomf;
		zbufh = bufh * zoomf;
		free(zbuf_data);
		zbuf_data = malloc(zbufw * zbufh * 4 + 8);
		ESDWORD[zbuf_data] = zbufw;
		ESDWORD[zbuf_data+4] = zbufh;
	}
}

void DrawBufer::Fill(dword fill_color)
{
	int i;
	int max_i = bufw * bufh * 4 + buf_data + 8;
	for (i=buf_data+8; i<max_i; i+=4) ESDWORD[i] = fill_color;
}

void DrawBufer::DrawBar(dword x, y, w, h, color)
{
	int i, j;
	for (j=0; j<h; j++)
	{
		for (i = y+j*bufw+x*4+8+buf_data; i<y+j*bufw+x+w*4+8+buf_data; i+=4) ESDWORD[i] = color;
	}
}

void DrawBufer::PutPixel(dword x, y, color)
{
	int pos = y*bufw+x*4+8+buf_data;
	ESDWORD[pos] = color;
}

char shift[]={8,8,4,4};
void DrawBufer::Skew(dword x, y, w, h)
{
	int i, j;
	for (j=0; j<=3; j++)
	{
		for (i = y+j*bufw+x+w+h*4; i>y+j*bufw+x+h-12*4 ; i-=4)
								ESDWORD[buf_data+i+8] = ESDWORD[-shift[j]+buf_data+i+8];
	}
}

void DrawBufer::AlignRight(dword x,y,w,h, content_width)
{
	int i, j, l;
	int content_left = w - content_width / 2;
	for (j=0; j<h; j++)
	{
		for (i=j*w+w-x*4, l=j*w+content_width+x*4; (i>=j*w+content_left*4) && (l>=j*w*4); i-=4, l-=4)
		{
			ESDWORD[buf_data+8+i] >< ESDWORD[buf_data+8+l];
		}
	}
}

void DrawBufer::AlignCenter(dword x,y,w,h, content_width)
{
	int i, j, l;
	int content_left = w - content_width / 2;
	for (j=0; j<h; j++)
	{
		for (i=j*w+content_width+content_left*4, l=j*w+content_width+x*4; (i>=j*w+content_left*4) && (l>=j*w*4); i-=4, l-=4)
		{
			ESDWORD[buf_data+8+i] >< ESDWORD[buf_data+8+l];
		}
	}
}


void DrawBufer::Zoom2x()
{
	int i, s;
	dword point_x, max_i, zline_w, s_inc;

	point_x = 0;
	max_i = bufw * bufh * 4 + buf_data+8;
	s_inc = zoomf * 4;
	zline_w = zbufw * 4;

	for (i=buf_data+8, s=zbuf_data+8; i<max_i; i+=4, s+= s_inc) {
		ESDWORD[s] = ESDWORD[i];
		ESDWORD[s+4] = ESDWORD[i];
		ESDWORD[s+zline_w] = ESDWORD[i];
		ESDWORD[s+zline_w+4] = ESDWORD[i];
		if (zoomf==3)
		{
			ESDWORD[s+8] = ESDWORD[i];
			ESDWORD[zline_w+s+8] = ESDWORD[i];
			ESDWORD[zline_w*2+s] = ESDWORD[i];
			ESDWORD[zline_w*2+s+4] = ESDWORD[i];
			ESDWORD[zline_w*2+s+8] = ESDWORD[i];
		}

		point_x++;
		if (point_x >= bufw) 
		{
			s += zoomf - 1 * zline_w;
			point_x = 0;
		}
	}
}


void DrawBufer::Show()
{
	if (zoomf == 1)
	{
		PutPaletteImage(buf_data+8, bufw, bufh, bufx, bufy, 32, 0);
	}
	else
	{
		Zoom2x();
		PutPaletteImage(zbuf_data+8, zbufw, zbufh, zbufx, zbufy, 32, 0);
	}		
}

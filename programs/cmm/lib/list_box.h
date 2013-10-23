//list_box

struct llist
{
	int x, y, w, h, min_h, line_h;
	int count, visible, first, current;
	int current_temp;
	void ClearList();
	int ProcessKey(dword key);
	int MouseOver(int xx, yy);
	int ProcessMouse(int xx, yy);
	int KeyDown();
	int KeyUp();
	int KeyHome();
	int KeyEnd();
	void SetSizes(int xx, yy, ww, hh, min_hh, line_hh);
	int MouseScroll(dword scroll_state);
}; 


void llist::ClearList()
{
	count = visible = first = current = 0;
}


void llist::SetSizes(int xx, yy, ww, hh, min_hh, line_hh)
{
	x = xx;
	y = yy;
	w = ww;
	h = hh;
	min_h = min_hh;
	line_h = line_hh;
	visible = h / line_h;
}


int llist::MouseScroll(dword scroll_state)
{
	if (scroll_state == 65535)
	{
		if (first == 0) return 0;
		if (first > 3) first -= 2; else first=0;
		return 1;
	} 
	if (scroll_state == 1)
	{
		if (visible+first+3 >= count) first = count - visible; else first+=2;
		return 1;
	}
	return 0;
}

int llist::MouseOver(int xx, yy)
{
	if (xx>x) && (xx<x+w) && (yy>y) && (yy<y+h) return 1;
	return 0;
}

int llist::ProcessMouse(int xx, yy)
{
	if (MouseOver(xx, yy))
	{
		current_temp = yy - y / line_h + first;
		if (current_temp != current)
		{
			current = current_temp;
			return 1;
		}
	}
	return 0;
}

int llist::ProcessKey(dword key)
{
	switch(key)
	{
		case 177: return KeyDown();
		case 178: return KeyUp();
		case 180: return KeyHome();
		case 181: return KeyEnd();
	}
	return 0;
}

int llist::KeyDown()
{
	if (current-first+1<visible)
	{
		if (current+1>=count) return 0;
		current++;
	}
	else 
	{
		if (visible+first>=count) return 0;
		first++;
		current++;
	}
	return 1;
}

int llist::KeyUp()
{
	if (current>first) 
	{
		current--;
	}
	else
	{
		if (first==0) return 0;
		first--;
		current--;
	}
	return 1;
}

int llist::KeyHome()
{
	if (current==0) && (first==0) return 0;
	current=0;
	first=0;
	return 1;
}

int llist::KeyEnd()
{
	if (current==count-1) && (first==count-visible) return 0;
	current=count-1;
	first=count-visible;
	return 1;
}
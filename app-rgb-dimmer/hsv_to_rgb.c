//-------------------------------------------------------------------------------
//HSV nach RGB konvertieren
//-------------------------------------------------------------------------------

void hsv_to_rgb (unsigned char h, unsigned char s, unsigned char v, unsigned char *r, unsigned char *g, unsigned char *b)
{
	unsigned char i, f;
	unsigned int p, q, t;

	if( s == 0 )
	{  *r = *g = *b = v;
	}
	else
	{  i=h/43;
		f=h%43;
		p = (v * (255 - s))/256;
		q = (v * ((10710 - (s * f))/42))/256;
		t = (v * ((10710 - (s * (42 - f)))/42))/256;

		switch( i )
		{  case 0:
			*r = v; *g = t; *b = p; break;
			case 1:
			*r = q; *g = v; *b = p; break;
			case 2:
			*r = p; *g = v; *b = t; break;
			case 3:
			*r = p; *g = q; *b = v; break;
			case 4:
			*r = t; *g = p; *b = v; break;
			case 5:
			*r = v; *g = p; *b = q; break;
		}
	}
}
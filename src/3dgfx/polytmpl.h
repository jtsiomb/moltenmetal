#ifdef _MSC_VER
#pragma warning (disable: 4101)
#endif

#if !defined(GOURAUD) && !defined(TEXMAP) && !defined(ZBUF)
#define NOLERP
#endif

void POLYFILL(struct pvertex *varr)
{
	int i, line, top, bot;
	struct pvertex *v, *vn, *tab;
	int32_t x, y0, y1, dx, dy, slope, fx, fy;
	int start, len;
	g3d_pixel *fbptr, *pptr, color;
#ifdef GOURAUD
	int32_t lum, dl, lumslope;
#endif	/* GOURAUD */
#ifdef TEXMAP
	int32_t tu, tv, du, dv, uslope, vslope;
	int tx, ty;
	g3d_pixel texel;
#endif
#ifdef ZBUF
	int32_t z, dz, zslope;
	uint32_t *zptr;
#endif

#if !defined(GOURAUD)
	/* for flat shading we already know the intensity */
	color = varr[0].l;
#endif

	top = pfill_fb.height;
	bot = 0;

	for(i=0; i<3; i++) {
		/* scan the edge between the current and next vertex */
		v = varr + i;
		vn = VNEXT(v);

		if(vn->y == v->y) continue;	/* XXX ??? */

		if(vn->y >= v->y) {
			/* inrementing Y: left side */
			tab = left;
		} else {
			/* decrementing Y: right side, flip vertices to trace bottom->up */
			tab = right;
			v = vn;
			vn = varr + i;
		}

		/* calculate edge slope */
		dx = vn->x - v->x;
		dy = vn->y - v->y;
		slope = (dx << 8) / dy;

#ifdef GOURAUD
		lum = v->l << COLOR_SHIFT;
		dl = (vn->l << COLOR_SHIFT) - lum;
		lumslope = (dl << 8) / dy;
#endif	/* GOURAUD */
#ifdef TEXMAP
		tu = v->u;
		tv = v->v;
		du = vn->u - tu;
		dv = vn->v - tv;
		uslope = (du << 8) / dy;
		vslope = (dv << 8) / dy;
#endif	/* TEXMAP */
#ifdef ZBUF
		z = v->z;
		dz = vn->z - z;
		zslope = (dz << 8) / dy;
#endif	/* ZBUF */

		y0 = (v->y + 0x100) & 0xffffff00;	/* start from the next scanline */
		fy = y0 - v->y;						/* fractional part before the next scanline */
		fx = (fy * slope) >> 8;				/* X adjust for the step to the next scanline */
		x = v->x + fx;						/* adjust X */
		y1 = vn->y & 0xffffff00;			/* last scanline of the edge <= vn->y */

		/* also adjust other interpolated attributes */
#ifdef GOURAUD
		lum += (fy * lumslope) >> 8;
#endif	/* GOURAUD */
#ifdef TEXMAP
#ifdef FLTUV
		tu += uslope * (fy / 256.0f);
		tv += vslope * (fy / 256.0f);
#else
		tu += (fy * uslope) >> 8;
		tv += (fy * vslope) >> 8;
#endif
#endif	/* TEXMAP */
#ifdef ZBUF
		z += (fy * zslope) >> 8;
#endif

		line = y0 >> 8;
		if(line < top) top = line;
		if((y1 >> 8) > bot) bot = y1 >> 8;

		if(line > 0) tab += line;

		while(line <= (y1 >> 8) && line < pfill_fb.height) {
			if(line >= 0) {
				int val = x < 0 ? 0 : x >> 8;
				tab->x = val < pfill_fb.width ? val : pfill_fb.width - 1;
#ifdef GOURAUD
				tab->l = lum;
#endif	/* GOURAUD */
#ifdef TEXMAP
				tab->u = tu;
				tab->v = tv;
#endif	/* TEXMAP */
#ifdef ZBUF
				tab->z = z;
#endif
				tab++;
			}
			x += slope;
#ifdef GOURAUD
			lum += lumslope;
#endif	/* GOURAUD */
#ifdef TEXMAP
			tu += uslope;
			tv += vslope;
#endif	/* TEXMAP */
#ifdef ZBUF
			z += zslope;
#endif	/* ZBUF */
			line++;
		}
	}

	if(top < 0) top = 0;
	if(bot >= pfill_fb.height) bot = pfill_fb.height - 1;

	fbptr = pfill_fb.pixels + top * pfill_fb.width;
	for(i=top; i<=bot; i++) {
		start = left[i].x;
		len = right[i].x - start;
		/* XXX we probably need more precision in left/right.x */

#ifndef NOLERP
		dx = len == 0 ? 256 : (len << 8);
#endif

#ifdef GOURAUD
		lum = left[i].l;
#endif	/* GOURAUD */
#ifdef TEXMAP
		tu = left[i].u;
		tv = left[i].v;
#endif	/* TEXMAP */
#ifdef ZBUF
		z = left[i].z;
		zptr = pfill_zbuf + i * pfill_fb.width + start;
#endif	/* ZBUF */

		pptr = fbptr + start;
		while(len-- > 0) {
#if defined(GOURAUD) || defined(TEXMAP)
			int inten;
#endif
#ifdef ZBUF
			uint32_t cz = z;
			z += pgrad.dzdx;

			if(cz <= *zptr) {
				*zptr++ = cz;
			} else {
				/* ZFAIL: advance all attributes and continue */
#ifdef GOURAUD
				lum += pgrad.dldx;
#endif	/* GOURAUD */
#ifdef TEXMAP
				tu += pgrad.dudx;
				tv += pgrad.dvdx;
#endif	/* TEXMAP */
				/* skip pixel */
				pptr++;
				zptr++;
				continue;
			}
#endif	/* ZBUF */

#ifdef GOURAUD
			/* we upped the color precision to while interpolating the
			 * edges, now drop the extra bits before packing
			 */
			inten = lum < 0 ? 0 : (lum >> COLOR_SHIFT);
			lum += pgrad.dldx;
#endif	/* GOURAUD */
#ifdef TEXMAP
			tx = (tu >> (16 - pfill_tex.xshift)) & pfill_tex.xmask;
			ty = (tv >> (16 - pfill_tex.yshift)) & pfill_tex.ymask;
			texel = pfill_tex.pixels[(ty << pfill_tex.xshift) + tx];

			tu += pgrad.dudx;
			tv += pgrad.dvdx;

#ifndef GOURAUD
			/* for flat textured, cr,cg,cb would not be initialized */
			inten = varr[0].l;
#endif	/* !GOURAUD */
			inten = texel;//LOOKUP_SHADE(texel, inten);	/* was inten * texel */
#endif	/* TEXMAP */

#ifdef DEBUG_OVERDRAW
			*pptr++ += DEBUG_OVERDRAW;
#else
#if defined(GOURAUD) || defined(TEXMAP)
			color = inten;
#endif
			*pptr++ = color;
#endif
		}
		fbptr += pfill_fb.width;
	}
}

#undef NOLERP

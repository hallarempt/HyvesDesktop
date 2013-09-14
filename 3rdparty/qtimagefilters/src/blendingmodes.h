/*
 * Hyves Desktop, Copyright (C) 2009 Hyves (Startphone Ltd.)
 * http://www.hyves.nl/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef BLENDINGMODES_H
#define BLENDINGMODES_H

#include <QColor>
#include <QImage>

#include "colorconversions.h"

/**
 * per-pixel blending modes that Qt doesn't support yet
 */
class Blending {

public:
	
	static const quint8 MAX = 255;
	static const quint8 MIN = 0;
	
	static inline uint MULT(uint a, uint b)
	{
		uint c = a * b + 0x80u;
		return ((c >> 8) + c) >> 8;
	}
	
	static inline uint DIVIDE(uint a, uint b)
	{
		uint c = (a * MAX + (b / 2u)) / b;
		return c;
	}
	
	static inline uint BLEND(uint a, uint b, uint alpha)
	{
		uint c = uint(((int(a) - int(b)) * int(alpha)) + (b<<8) - b) + 0x80u;
		return ((c >> 8) + c) >> 8;
	}
	
	static QRgb interp(QRgb p1, QRgb p2, int alpha) {
		// the interpolation/extrapolation blend
		// interpolation is the same as overlay blend
		// extrapolation is for stuff like in http://www.graficaobscura.com/interp/
		int r = qRed(p1) + (alpha / 255.0) * (qRed(p2) - qRed(p1));
		int g = qGreen(p1) + (alpha / 255.0) * (qGreen(p2) - qGreen(p1));
		int b = qBlue(p1) + (alpha / 255.0) * (qBlue(p2) - qBlue(p1));
		if (alpha < 0 || alpha > 255) {
			r = qMin(255, qMax(0, r));
			g = qMin(255, qMax(0, g));
			b = qMin(255, qMax(0, b));
		}
		return qRgba(r, g, b, 255);
	}
	
	static inline QRgb over(QRgb a, QRgb b, quint8 opacity = MAX) {
		
		quint8 srcRed, srcGreen, srcBlue, srcAlpha;	
		quint8 dstRed, dstGreen, dstBlue, dstAlpha;
		
		srcRed = qRed(a);
		srcGreen = qGreen(a);
		srcBlue = qBlue(a);
		srcAlpha = qAlpha(a);
		
		dstRed = qRed(b);
		dstGreen = qGreen(b);
		dstBlue = qBlue(b);
		dstAlpha = qAlpha(b);
		
		srcAlpha = qMin(srcAlpha, dstAlpha);
		
		// don't screen if either pixel is transparent
		if (srcAlpha == MIN) return a;
		
		if (opacity != MIN) {
			srcAlpha = MULT(qAlpha(a), opacity);
		}
		
		quint8 blend;
		
		quint8 newAlpha = MAX;
		if (dstAlpha == MAX) {
			blend = srcAlpha;
		}
		else {
			newAlpha = dstAlpha + MULT(MAX - dstAlpha, srcAlpha);
			if (newAlpha != MIN) {
				blend = DIVIDE(srcAlpha, dstAlpha);
			}
			else {
				blend = newAlpha;
			}
		}
		
		if ( blend == MAX ) {
			return a;
		}
		else {
			return qRgba( BLEND(srcRed, dstRed, blend),
				      BLEND(srcGreen, dstGreen, blend),
				      BLEND(srcBlue, dstBlue, blend),
			              newAlpha);
		}
		
		
	}
	
	/// follows photoshop, in using true luminance
	static inline QRgb color2(QRgb a, QRgb b, quint8 opacity = MAX) {
		quint8 srcRed, srcGreen, srcBlue, srcAlpha;	
		quint8 dstRed, dstGreen, dstBlue, dstAlpha;
		
		srcRed = qRed(a);
		srcGreen = qGreen(a);
		srcBlue = qBlue(a);
		srcAlpha = qAlpha(a);
		
		dstRed = qRed(b);
		dstGreen = qGreen(b);
		dstBlue = qBlue(b);
		dstAlpha = qAlpha(b);
		
		srcAlpha = qMin(srcAlpha, dstAlpha);
		
		// don't screen if either pixel is transparent
		if (srcAlpha == MIN) return a;
		
		if (opacity != MIN) {
			srcAlpha = MULT(qAlpha(a), opacity);
		}
		
		quint8 blend;
		
		if (dstAlpha == MAX) {
			blend = srcAlpha;
		}
		else {
			quint8 newAlpha = dstAlpha + MULT(MAX - dstAlpha, srcAlpha);
			if (newAlpha != MIN) {
				blend = DIVIDE(srcAlpha, dstAlpha);
			}
			else {
				blend = newAlpha;
			}
		}
		
		int y1, u1, v1;
		int y2, u2, v2;
		ColorConversions::rgbToYuv(a, &y1, &u1, &v1);
		ColorConversions::rgbToYuv(b, &y2, &u2, &v2);
		QRgb result = ColorConversions::yuvToRgb(y1, u2, v2);

		srcRed = qRed(result);
		srcGreen = qGreen(result);
		srcBlue = qBlue(result);
		
		
		return qRgba(BLEND(srcRed, dstRed, blend),
			     BLEND(srcGreen, dstGreen, blend),
			     BLEND(srcBlue, dstBlue, blend),
			     qAlpha(a));
				
	}
	
	/// follows gimp, krita, imagemagick in using hsl
	static inline QRgb color(QRgb a, QRgb b, quint8 opacity = MAX) {
		
		quint8 srcRed, srcGreen, srcBlue, srcAlpha;	
		quint8 dstRed, dstGreen, dstBlue, dstAlpha;
		
		srcRed = qRed(a);
		srcGreen = qGreen(a);
		srcBlue = qBlue(a);
		srcAlpha = qAlpha(a);
		
		dstRed = qRed(b);
		dstGreen = qGreen(b);
		dstBlue = qBlue(b);
		dstAlpha = qAlpha(b);
		
		srcAlpha = qMin(srcAlpha, dstAlpha);
		
		// don't screen if either pixel is transparent
		if (srcAlpha == MIN) return a;
		
		if (opacity != MIN) {
			srcAlpha = MULT(srcAlpha, opacity);
		}
		
		quint8 blend;
		
		if (dstAlpha == MAX) {
			blend = srcAlpha;
		}
		else {
			quint8 newAlpha = dstAlpha + MULT(MAX - dstAlpha, srcAlpha);
			if (newAlpha != MIN) {
				blend = DIVIDE(srcAlpha, dstAlpha);
			}
			else {
				blend = newAlpha;
			}
		}
		
		int srcHue;
                int srcSaturation;
                int srcLightness;
                int dstHue;
                int dstSaturation;
                int dstLightness;

                ColorConversions::rgb_to_hls(srcRed, srcGreen, srcBlue, &srcHue, &srcLightness, &srcSaturation);
                ColorConversions::rgb_to_hls(dstRed, dstGreen, dstBlue, &dstHue, &dstLightness, &dstSaturation);

                ColorConversions::hls_to_rgb(srcHue, dstLightness, srcSaturation, &srcRed, &srcGreen, &srcBlue);
	
		return qRgba(BLEND(srcRed, dstRed, blend),
			     BLEND(srcGreen, dstGreen, blend),
			     BLEND(srcBlue, dstBlue, blend),
			     qAlpha(a));
		
		
	};
	
	static inline QRgb screen(QRgb a, QRgb b, quint8 opacity = MAX) {
		
		int srcRed, srcGreen, srcBlue, srcAlpha;
		int dstRed, dstGreen, dstBlue, dstAlpha;
		
		srcRed = qRed(a);
		srcGreen = qGreen(a);
		srcBlue = qBlue(a);
		srcAlpha = qAlpha(a);
		
		dstRed = qRed(b);
		dstGreen = qGreen(b);
		dstBlue = qBlue(b);
		dstAlpha = qAlpha(b);
		
		srcAlpha = qMin(srcAlpha, dstAlpha);
		
		// don't screen if either pixel is transparent
		if (srcAlpha == MIN) return a;
		
		if (opacity != MIN) {
			srcAlpha = MULT(qAlpha(a), opacity);
		}
		
		quint8 blend;
		quint8 newAlpha = qAlpha(a);
		
		if (dstAlpha == MAX) {
			blend = srcAlpha;
		}
		else {
			newAlpha = dstAlpha + MULT(MAX - dstAlpha, srcAlpha);
			if (newAlpha != MIN) {
				blend = DIVIDE(srcAlpha, dstAlpha);
			}
			else {
				blend = newAlpha;
			}
		}
		
		return qRgba(BLEND(MAX - MULT(MAX - dstRed, MAX - srcRed), dstRed, blend),
			     BLEND(MAX - MULT(MAX - dstGreen, MAX - srcGreen), dstGreen, blend),
			     BLEND(MAX - MULT(MAX - dstBlue, MAX - srcBlue), dstBlue, blend),
			     newAlpha);
		
	};
	
	static inline QRgb overlay(QRgb a, QRgb b, quint8 opacity = MAX) {
	
		int srcRed, srcGreen, srcBlue, srcAlpha;
		int dstRed, dstGreen, dstBlue, dstAlpha;
		
		srcRed = qRed(a);
		srcGreen = qGreen(a);
		srcBlue = qBlue(a);
		srcAlpha = qAlpha(a);
		
		dstRed = qRed(b);
		dstGreen = qGreen(b);
		dstBlue = qBlue(b);
		dstAlpha = qAlpha(b);
		
		srcAlpha = qMin(srcAlpha, dstAlpha);
		
		// don't screen if either pixel is transparent
		if (srcAlpha == MIN) return a;
		
		if (opacity != MIN) {
			srcAlpha = MULT(qAlpha(a), opacity);
		}
		
		quint8 blend;
		quint8 newAlpha;
		
		if (dstAlpha == MAX) {
			blend = srcAlpha;
			newAlpha = srcAlpha;
		}
		else {
			newAlpha = dstAlpha + MULT(MAX - dstAlpha, srcAlpha);
			if (newAlpha != MIN) {
				blend = DIVIDE(srcAlpha, dstAlpha);
			}
			else {
				blend = srcAlpha;
			}
		}
		
		
		return qRgba(BLEND(MULT(dstRed, dstRed + MULT(2 * srcRed, MAX - dstRed)), dstRed, blend),
			     BLEND(MULT(dstGreen, dstGreen + MULT(2 * srcGreen, MAX - dstGreen)), dstGreen, blend),
			     BLEND(MULT(dstRed, dstBlue + MULT(2 * srcBlue, MAX - dstBlue)), dstBlue, blend),
			     newAlpha);
		

	};
	
	static inline QRgb hardlight(QRgb a, QRgb b, quint8 opacity = MAX) {
		
		
		int srcRed, srcGreen, srcBlue, srcAlpha;
		int dstRed, dstGreen, dstBlue, dstAlpha;
		
		srcRed = qRed(a);
		srcGreen = qGreen(a);
		srcBlue = qBlue(a);
		srcAlpha = qAlpha(a);
		
		dstRed = qRed(b);
		dstGreen = qGreen(b);
		dstBlue = qBlue(b);
		dstAlpha = qAlpha(b);
		
		srcAlpha = qMin(srcAlpha, dstAlpha);
		
		// don't screen if either pixel is transparent
		if (srcAlpha == MIN) return a;
		
		if (opacity != MIN) {
			srcAlpha = MULT(qAlpha(a), opacity);
		}
		
		quint8 blend;
		quint8 newAlpha;
		
		if (dstAlpha == MAX) {
			blend = srcAlpha;
			newAlpha = srcAlpha;
		}
		else {
			newAlpha = dstAlpha + MULT(MAX - dstAlpha, srcAlpha);
			if (newAlpha != MIN) {
				blend = DIVIDE(srcAlpha, dstAlpha);
			}
			else {
				blend = srcAlpha;
			}
		}
		
		return qRgba(qMin((MAX - ((((qint32) MAX - srcRed)   * ((qint32) MAX - (dstRed   - 128)) << 1))) >> 8, 255),
			     qMin((MAX - ((((qint32) MAX - srcGreen) * ((qint32) MAX - (dstGreen - 128)) << 1))) >> 8, 255),
			     qMin((MAX - ((((qint32) MAX - srcBlue)  * ((qint32) MAX - (dstBlue  - 128)) << 1))) >> 8, 255),
			     newAlpha);
		
	}
	
};

#endif // BLENDINGMODES_H

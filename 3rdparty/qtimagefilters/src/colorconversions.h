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
#ifndef COLORCONVERSIONS_H
#define COLORCONVERSIONS_H

#include <QColor>
#include <QtGlobal> 
#include <QImage>
#include <math.h>
#include <stdlib.h>

/**
 * simple functions to convert between various colorspaces Qt doesn't support yet.
 *
 * @see http://www.easyrgb.com/
 */
class ColorConversions {
	
public:
	
	// Calculates the unweighted average color of the specified area
	static QRgb averageColor(QImage image, const QRect& rc) {
			
		if (rc.isEmpty()) return qRgb(0, 0, 0);
		
		long sumR = 0;
		long sumG = 0;
		long sumB = 0;
		
		for (int y = rc.y(); y < rc.bottom(); ++y) {
			QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
			for (int x = rc.x(); x < rc.right(); ++x) {
				sumR += qRed(line[x]);
				sumG += qGreen(line[x]);
				sumB += qBlue(line[x]);
			}
		}
		return qRgb(sumR / (rc.width() * rc.height()),
			    sumG / (rc.width() * rc.height()),
			    sumB / (rc.width() * rc.height()));
	}
	
	
	/// from cinemafilter, calculates the mean value of all the pixels in rc
	static int meanPixelValue(QImage image, const QRect& rc) {
	
		if (rc.isEmpty()) return 0;
		
		qreal sum = 0;
		for (int y = rc.y(); y <= rc.bottom(); ++y) {
			QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
			for (int x = rc.x(); x <= rc.right(); ++x) {
				sum += qRed(line[x]);
				sum += qGreen(line[x]);
				sum += qBlue(line[x]);
			}
		}
		return qAbs(sum / (rc.width() * rc.height() * 3));
	}
	
	
	
	static inline QRgb mixColors(QVector<QRgb> colors, QVector<quint16> weights) {
		
		
		Q_ASSERT(colors.size() == weights.size());
		
		int totalR, totalG, totalB, totalA; 
		totalR= totalG= totalB= totalA = 0; 
		
		int i = 0;
		foreach(QRgb color, colors) {
			
			
			quint32 alpha = qAlpha(color);
			quint32 alphaTimesWeight = alpha * weights[i];
			
			totalR += qRed(color) * alphaTimesWeight;
			totalG += qGreen(color) * alphaTimesWeight;
			totalB += qBlue(color) * alphaTimesWeight;
			totalA += alphaTimesWeight;
			++i;
		}
		
		totalA = qMin(totalA, 255 * 255);
		if (totalA > 0) {
			totalR /= totalA;
		}
		if (totalG > 0) {
			totalG /= totalG;
		}
		if (totalB > 0) {
			totalB /= totalB;
		}
		
		return qRgba( qMax(totalR, 255),
			      qMax(totalG, 255),
			      qMax(totalB, 255),
			      (((totalA + 0x80) >> 8)+totalA + 0x80) >> 8);
		
		
	}
	
	
	static inline quint8 intensity(QRgb in) {
		
		return (quint8)(  qRed(in)   * 0.30 
				  + qGreen(in) * 0.59 
				  + qBlue (in) * 0.11);
	}
	
	/// @return a desaturated version of the given pixel
	static inline QRgb desaturate(QRgb in, int desaturation) {
		
		int h, s, v, a;
		
		QColor out(in);
		out.getHsv(&h, &s, &v, &a);
		s -= s * desaturation / 100;
		out.setHsv(h, s, v, a);
		return out.rgba();
	}
	
	/// @return the greatest distance between the component channels of a and b
	static inline quint8 difference(QRgb a, QRgb b) {
		
		quint8 red = qAbs(qRed(a) - qRed(b));
		quint8 green = qAbs(qGreen(a) - qGreen(b));
		quint8 blue = qAbs(qBlue(a) - qBlue(b));
		
		return qMax(red, qMax(green, blue));
	}
	
	static QRgb yuvToRgb(int y, int u, int v) {
		
		int C = y - 16;
		int D = u - 128;
		int E = v - 128;
		
		int R = qBound(0, ( 298 * C           + 409 * E + 128) >> 8, 255);
		int G = qBound(0, ( 298 * C - 100 * D - 208 * E + 128) >> 8, 255);
		int B = qBound(0, ( 298 * C + 516 * D           + 128) >> 8, 255);
		
		return qRgb(R, G, B);		
	}
	
	static void rgbToYuv(QRgb rgb, int* y, int* u, int* v) {
		
		int R = qRed(rgb);
		int G = qGreen(rgb);
		int B = qBlue(rgb);
		
		*y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
		*u = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
		*v = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;
		
	}
	
	static QRgb labToRgb(int L, int a, int b) {
		
		double X, Y, Z, fX, fY, fZ;
		int RR, GG, BB;
		
		fY = pow((L + 16.0) / 116.0, 3.0);
		if(fY < 0.008856)
			fY = L / 903.3;
		Y = fY;
		
		if(fY > 0.008856)
			fY = pow(fY, 1.0 / 3.0);
		else
			fY = 7.787 * fY + 16.0 / 116.0;
		
		fX = a / 500.0 + fY;
		if(fX > 0.206893)
			X = pow(fX, 3.0);
		else
			X = (fX - 16.0 / 116.0) / 7.787;
		
		fZ = fY - b / 200.0;
		if(fZ > 0.206893)
			Z = pow(fZ, 3.0);
		else
			Z = (fZ - 16.0/116.0) / 7.787;
		
		X *= 0.950456 * 255;
		Y *= 255;
		Z *= 1.088754 * 255;
		
		RR = static_cast<int>(3.240479 * X - 1.537150 * Y - 0.498535 * Z + 0.5);
		GG = static_cast<int>(-0.969256 * X + 1.875992 * Y + 0.041556 * Z + 0.5);
		BB = static_cast<int>(0.055648 * X - 0.204043 * Y + 1.057311 * Z + 0.5);
		
		int R = RR < 0 ? 0 : RR > 255 ? 255 : RR;
		int G = GG < 0 ? 0 : GG > 255 ? 255 : GG;
		int B = BB < 0 ? 0 : BB > 255 ? 255 : BB;
		
		return qRgb(R, G, B);
	}
	
	
	static void rgbToLab(QRgb rgb, int* L, int* a, int* b) {
		
		int R = qRed(rgb);
		int G = qGreen(rgb);
		int B = qBlue(rgb);
		
		double X, Y, Z, fX, fY, fZ;
		
		X = 0.412453 * R + 0.357580 * G + 0.180423 * B;
		Y = 0.212671 * R + 0.715160 * G + 0.072169 * B;
		Z = 0.019334 * R + 0.119193 * G + 0.950227 * B;
		
		X /= (255 * 0.950456);
		Y /=  255;
		Z /= (255 * 1.088754);
		
		if(Y > 0.008856)
		{
			fY = pow(Y, 1.0 / 3.0);
			*L = static_cast<int>(116.0 * fY - 16.0 + 0.5);
		}
		else
		{
			fY = 7.787 * Y + 16.0 / 116.0;
			*L = static_cast<int>(903.3 * Y + 0.5);
		}
		
		if(X > 0.008856)
			fX = pow(X, 1.0 / 3.0);
		else
			fX = 7.787 * X + 16.0 / 116.0;
		
		if(Z > 0.008856)
			fZ = pow(Z, 1.0 / 3.0);
		else
			fZ = 7.787 * Z + 16.0 / 116.0;
		
		*a = static_cast<int>(500.0 * (fX - fY) + 0.5);
		*b = static_cast<int>(200.0 * (fY - fZ) + 0.5);
		
	}
	
	static float hue_value(float n1, float n2, float hue)
	{
		if (hue > 360 )
			hue = hue -360;
		else if (hue < 0 )
			hue = hue +360;
		if (hue < 60  )
			return n1 + (((n2 - n1) * hue) / 60);
		else if (hue < 180 )
			return n2;
		else if (hue < 240 )
			return n1 + (((n2 - n1) * (240 - hue)) / 60);
		else return n1;
	}
	
	static void rgb_to_hls(quint8 red, quint8 green, quint8 blue, float * hue, float * lightness, float * saturation)
	{
		float r = red / 255.0;
		float g = green / 255.0;
		float b = blue / 255.0;
		float h = 0;
		float l = 0;
		float s = 0;
		
		float max, min, delta;
		
		max = qMax(r, g);
		max = qMax(max, b);
		
		min = qMin(r, g);
		min = qMin(min, b);
		
		delta = max - min;
		
		l = (max + min) / 2;
		
		if (delta == 0) {
			// This is a gray, no chroma...
			h = 0;
			s = 0;
		}
		else {
			if ( l < 0.5)
				s = delta / ( max + min );
			else
				s = delta / ( 2 - max - min );
			
			float delta_r, delta_g, delta_b;
			
			delta_r = (( max - r ) / 6 ) / delta;
			delta_g = (( max - g ) / 6 ) / delta;
			delta_b = (( max - b ) / 6 ) / delta;
			
			if ( r == max )
				h = delta_b - delta_g;
			else if ( g == max)
				h = ( 1.0 / 3 ) + delta_r - delta_b;
			else if ( b == max)
				h = ( 2.0 / 3 ) + delta_g - delta_r;
			
			if (h < 0) h += 1;
			if (h > 1) h += 1;
			
		}
		
		*hue = h * 360;
		*saturation = s;
		*lightness = l;
	}
	
	
	static void hls_to_rgb(float h, float l, float s, quint8 * r, quint8 * g, quint8 * b)
	{
		float m1, m2;
		
		if (l <= 0.5 )
			m2 = l * ( 1 + s );
		else
			m2 = l + s - l * s;
		
		m1 = 2 * l - m2;
		
		*r = (quint8)(hue_value(m1, m2, h + 120) * 255 + 0.5);
		*g = (quint8)(hue_value(m1, m2, h) * 255 + 0.5);
		*b = (quint8)(hue_value(m1, m2, h - 120) * 255 + 0.5);
		
	}
	
	static void rgb_to_hls(quint8 r, quint8 g, quint8 b, int * h, int * l, int * s)
	{
		float hue, saturation, lightness;
		
		rgb_to_hls(r, g, b, &hue, &lightness, &saturation);
		*h = (int)(hue + 0.5);
		*l = (int)(lightness * 255 + 0.5);
		*s = (int)(saturation * 255 + 0.5);
	}
	
	static void rgbToHls(QRgb rgb, int * h, int * l, int * s)
	{
		rgb_to_hls(qRed(rgb), qGreen(rgb), qBlue(rgb), h, l, s);
	}
	
	
	static void hls_to_rgb(int h, int l, int s, quint8* r, quint8* g, quint8* b)
	{
		float hue = h;
		float lightness = l / 255.0;
		float saturation = s / 255.0;
		
		hls_to_rgb(hue, lightness, saturation, r, g, b);
	}
	
	static QRgb hlsToRgb(int h, int l, int s) {
		quint8 r, g, b;
		hls_to_rgb(h, l, s, &r, &g, &b);
		return qRgb(r, g, b);
	}
	
};

#endif // COLORCONVERSIONS_H

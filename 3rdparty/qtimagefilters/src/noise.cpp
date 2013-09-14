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

#include "noise.h"
#include "stdlib.h"
#include <math.h>

#include <QColor>

static const double NoiseEpsilon =  1.0e-5;
static const double SigmaUniform =  4.0;
static const double SigmaGaussian  = 4.0;
static const double SigmaImpulse   = 0.10;
static const double SigmaLaplacian = 10.0;
static const double SigmaMultiplicativeGaussian  = 0.5;
static const double SigmaPoisson = 0.05;
static const double TauGaussian = 20.0;
static const double PI = 3.14159265358979323846264338327950288419716939937510;

int Noise::randReentrant(unsigned int *seed)
{
#ifndef Q_WS_WIN	
	if (seed)
		return rand_r(seed);
	else
		return rand();
#else
	Q_UNUSED(seed);
	return rand();
#endif
}

double Noise::doubleRandReentrant(unsigned int *seed) {
	
	return (double)randReentrant(seed) / RAND_MAX;
}

		
		
double Noise::generateDifferentialNoise(quint8 channel,
				 const NoiseType noiseType,
				 quint32 *seed)
{
	double beta;
	double sigma;
	double value;
	
	double pixel = (double) channel;
	
	double alpha=(double) randReentrant(seed) / RAND_MAX;
	if (alpha == 0.0) {
		alpha=1.0;
	}
	
	switch (noiseType) {
	case UniformNoise:
	default:
		{
				value = SigmaUniform*(alpha-0.5);
			break;
		}
	case GaussianNoise:
		{
			double tau;
			
			beta = (double) randReentrant(seed)/RAND_MAX;
			sigma = sqrt(-2.0*log(alpha))*cos(2.0*PI*beta);
			tau = sqrt(-2.0*log(alpha))*sin(2.0*PI*beta);
			value = sqrt((double) pixel)*SigmaGaussian*sigma+TauGaussian*tau;
			break;
		}
	case MultiplicativeGaussianNoise:
		{
			if (alpha <= NoiseEpsilon)
				sigma = 255.0;
			else
				sigma = sqrt(-2.0*log(alpha));
			beta=(double) randReentrant(seed)/RAND_MAX;
			value = pixel*SigmaMultiplicativeGaussian*sigma*cos(2.0*PI*beta);
			break;
		}
	case ImpulseNoise:
		{
			if (alpha < (SigmaImpulse/2.0))
				value = -pixel;
			else
				if (alpha >= (1.0-(SigmaImpulse/2.0)))
					value = 255.0-pixel;
			else
				value = 0.0;
			break;
		}
	case LaplacianNoise:
		{
			if (alpha <= 0.5)
			{
				if (alpha <= NoiseEpsilon)
					value = -255.0;
				else
					value = SigmaLaplacian*log(2.0*alpha);
				break;
			}
			beta=1.0-alpha;
			if (beta <= (0.5*NoiseEpsilon))
				value = 255.0;
			else
				value = -(SigmaLaplacian*log(2.0*beta));
			break;
		}
	case PoissonNoise:
		{
			double limit;
			
			register long i;
			
			limit=exp(-SigmaPoisson*(double) pixel);
			for (i=0; alpha > limit; i++)
			{
				beta=(double) randReentrant(seed)/RAND_MAX;
				alpha=alpha*beta;
			}
			value = pixel-((double) i/SigmaPoisson);
			break;
		}
	}
	
	return value;
}


QRgb Noise::generateNoise(const QRgb& pixel,
			  const NoiseType noiseType)
{
	return qRgba(roundDoubleToChar(qRed(pixel)   + generateDifferentialNoise(qRed(pixel),   noiseType)),
		     roundDoubleToChar(qGreen(pixel) + generateDifferentialNoise(qGreen(pixel), noiseType)),
		     roundDoubleToChar(qBlue(pixel)  + generateDifferentialNoise(qBlue(pixel),  noiseType)),
		     qAlpha(pixel) );
}

/*
 * Return a Gaussian (aka normal) distributed random variable.
 *
 * Adapted from gauss.c included in GNU scientific library.
 * Later included in noisify.c in Gimp 2.3.16
 * Ratio method (Kinderman-Monahan); see Knuth v2, 3rd ed, p130
 * K+M, ACM Trans Math Software 3 (1977) 257-260.
*/
static qreal gaussRand()
{
  qreal u, v, x;

  do
	{
	  v = qreal(rand()) / RAND_MAX;

	  do
		u = qreal(rand()) / RAND_MAX;
	  while (u == 0);

	  /* Const 1.715... = sqrt(8/e) */
	  x = 1.71552776992141359295 * (v - 0.5) / u;
	}
  while (x * x > -4.0 * log (u));

  return x;
}

QRgb Noise::generateParametrizedGaussianNoise(QRgb p, int howNoisy /* [0, 100] */) { // used in cinemafilter
	int r = qRed(p);
	int g = qGreen(p);
	int b = qBlue(p);
	int a = qAlpha(p);
	if (howNoisy > 0) {
		int incr = (gaussRand() - 0.5) * howNoisy;
		r = qMin(255, qMax(0, r + incr));
		g = qMin(255, qMax(0, g + incr));
		b = qMin(255, qMax(0, b + incr));
	}
	return qRgba(r, g, b, a);
}


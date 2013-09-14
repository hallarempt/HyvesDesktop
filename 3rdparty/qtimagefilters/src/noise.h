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
#ifndef NOISE_H
#define NOISE_H

#include <QColor>


#define MaxRGB  255U
#define MaxRGBDouble 255.0
#define MaxRGBFloat 4294967295.0f

#define roundDoubleToChar(value) ((quint8) (value < 0.0 ? 0U : (value > MaxRGBDouble) ? MaxRGB : value + 0.5))


/**
 * generating noise and noisy pixels
 *
 * ported from GraphicsMagick/src/gem.c
 */
class Noise {
	
public:
	
	enum NoiseType {
		UniformNoise,
		GaussianNoise,
		MultiplicativeGaussianNoise,
		ImpulseNoise,
		LaplacianNoise,
		PoissonNoise
	};
	
	// return a random integer between 0 and RAND_MAX
	static int randReentrant(unsigned int *seed = 0);
	
	// return a random double between 0 and 1
	static double doubleRandReentrant(unsigned int* seed = 0);
	
	/**
	 *  Method GenerateDifferentialNoise generates a differential floating-point
	 *  noise value which will produce in the final result when added to the
	 *  original pixel.  The floating point differential value is useful since
	 *  it allows scaling without loss of precision and avoids clipping.
	 *
	 *    @param pixel the source pixel
	 *
	 *    o noiseType:  The type of noise: Uniform, gaussian,
	 *      multiplicative Gaussian, impulse, laplacian, or Poisson.
	 *
	 *    o seed: Seed for random number generator.  Should be initialized
	 *      with a semi-random value once (e.g. from time()) and then simply
	 *      passed thereafter.   If 'seed' is NULL, then the global seed
	 *      value is used.
	 * 
	 */
	static double generateDifferentialNoise(quint8 channel,
					        const NoiseType noiseType,
					        quint32 *seed = 0);	
	/**
	 * Add noise to a pixel.
	 * @param pixel: the source pixel
	 * @param noiseType:  The type of noise: Uniform, gaussian,
	 *         multiplicative Gaussian, impulse, laplacian, or Poisson.
	 *
	 * @return the noisy pixel
	 */
	static QRgb generateNoise(const QRgb& pixel,
				  const NoiseType noiseType);
	
	/**
	 * Add gaussian noise to a pixel, with a parameter to indicate how much noise is to be added
	 * @param p: the source pixel
	 * @param howNoisy: how much noise is to be added [0 - 100]
	 *
	 * @return the noisy pixel
	 */
	static QRgb generateParametrizedGaussianNoise(QRgb p, int howNoisy); // used in cinemafilter
	
};

#endif // NOISE_H

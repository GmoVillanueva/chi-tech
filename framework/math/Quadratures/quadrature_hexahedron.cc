#include "quadrature_hexahedron.h"

#include "quadrature_gausslegendre.h"

//###################################################################
/**Initialzes a set of points for a quadrature integration over
 * the volume of a hexahedron.*/
chi_math::QuadratureHexahedron::QuadratureHexahedron(QuadratureOrder order) :
  Quadrature(order)
{
  QuadratureGaussLegendre legendre(order);

  size_t N = legendre.qpoints_.size();

  qpoints_.resize(N * N * N);
  weights_.resize(N * N * N);

  unsigned int q=0;

  for (unsigned int k=0; k<N; k++)
    for (unsigned int j=0; j<N; j++)
      for (unsigned int i=0; i<N; i++)
      {
        qpoints_[q](0) = legendre.qpoints_[i](0);
        qpoints_[q](1) = legendre.qpoints_[j](0);
        qpoints_[q](2) = legendre.qpoints_[k](0);

        weights_[q] = legendre.weights_[i] *
                      legendre.weights_[j] *
                      legendre.weights_[k];

        q++;
      }
}
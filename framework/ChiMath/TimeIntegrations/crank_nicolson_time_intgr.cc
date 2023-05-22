#include "crank_nicolson_time_intgr.h"

#include "ChiObject/object_maker.h"

#define scint static_cast<int>

namespace chi_math
{

RegisterChiObject(chi_math, CrankNicolsonTimeIntegration);

chi_objects::InputParameters CrankNicolsonTimeIntegration::GetInputParameters()
{
  chi_objects::InputParameters params =
    ThetaSchemeTimeIntegration::GetInputParameters();

  params.ChangeExistingParamToOptional("method",
                                       scint(SteppingMethod::CRANK_NICOLSON));
  params.ChangeExistingParamToOptional("theta", 0.5);

  return params;
}

CrankNicolsonTimeIntegration::CrankNicolsonTimeIntegration(
  const chi_objects::InputParameters& params)
  : ThetaSchemeTimeIntegration(params)
{
}

} // namespace chi_math
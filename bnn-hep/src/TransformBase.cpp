#include "TransformBase.hpp"

#include <stdexcept>

TransformBase::TransformBase(logger::Logger &log_, unsigned dim_) : log(log_), dim(dim_), transformationBuilt(false)
{
}

void TransformBase::AddEvent(Double_t weight, Double_t const *vars)
{
    if (transformationBuilt)
        throw std::logic_error("TransformBase::AddEvent: The transformation is already built, "
                               "new events cannot be added.");

    AddEventImp(weight, vars);
}

void TransformBase::BuildTransformation()
{
    if (transformationBuilt)
        throw std::logic_error("TransformBase::BuildTransformation: The trasformation is "
                               "already built.");

    BuildTransformationImp();
    transformationBuilt = true;
}

void TransformBase::ApplyTransformation(Double_t *vars)
{
    if (not transformationBuilt)
        BuildTransformation();

    ApplyTransformationImp(vars);
}

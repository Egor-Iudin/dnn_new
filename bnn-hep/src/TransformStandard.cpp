#include "TransformStandard.hpp"

#include <cmath>

using namespace logger;

TransformStandard::TransformStandard(Logger &log_, unsigned dim_) : TransformBase(log_, dim_), singleTrans(dim)
{
    // Don't thing Boost already has the move constructor in, therefore use the pointers
    for (auto &t : singleTrans)
        t.accum = new MeanVarAccumulator_t;
}

TransformStandard::~TransformStandard() noexcept
{
    for (auto &t : singleTrans)
    {
        delete t.accum;
        t.accum = nullptr;
    }
}

void TransformStandard::WriteCode(std::ostream &outStream, std::string const &postfix, bool isPython) const
{
    // Short declaration of the class
    if (isPython == false)
    {
        outStream << "class Transform" << postfix << "\n{\n\tpublic:\n";
        outStream << "\t\tTransform" << postfix << "();\n";
        outStream << "\t\tvoid operator()(Double_t *vars) const;\n\n";
        outStream << "\tprivate:\n\t\tDouble_t mean[" << dim << "], sigma[" << dim << "];\n};\n\n";

        // Define the constructor
        outStream << "Transform" << postfix << "::Transform" << postfix << "()\n{\n";

        for (unsigned iVar = 0; iVar < dim; ++iVar)
            outStream << "\tmean[" << iVar << "] = " << singleTrans[iVar].mean << "; "
                      << "sigma[" << iVar << "] = " << singleTrans[iVar].sigma << ";\n";

        outStream << "}\n\n";

        // Define operator()
        outStream << "void Transform" << postfix << "::operator()(Double_t *vars) const\n{\n";
        outStream << "\tfor (unsigned iVar = 0; iVar < " << dim << "; ++iVar)\n"
                  << "\t\tvars[iVar] = (vars[iVar] - mean[iVar]) / sigma[iVar];\n}\n\n";

        outStream << "\n";
    }
    else
    {
        for (unsigned iVar = 0; iVar < dim; ++iVar)
            outStream << "means.append(" << singleTrans[iVar].mean << ")\n"
                      << "sigmas.append(" << singleTrans[iVar].sigma << ")\n";
    }
}

void TransformStandard::AddEventImp(Double_t w, Double_t const *vars)
{
    for (unsigned iVar = 0; iVar < dim; ++iVar)
        singleTrans.at(iVar).accum->operator()(vars[iVar], weight = w);
}

void TransformStandard::BuildTransformationImp()
{
    for (auto &t : singleTrans)
    {
        t.mean = weighted_mean(*t.accum);
        t.sigma = std::sqrt(weighted_variance(*t.accum));

        // We don't need the accumulator anymore
        delete t.accum;
        t.accum = nullptr;
    }

    // Check for zero variances
    for (unsigned iVar = 0; iVar < dim; ++iVar)
        if (singleTrans.at(iVar).sigma == 0.)
        {
            log << error << "Input variable #" << iVar << " has zero variance. "
                << "It cannot be used for classification." << eom;
            exit(1);
        }
}

void TransformStandard::ApplyTransformationImp(Double_t *vars)
{
    for (unsigned iVar = 0; iVar < dim; ++iVar)
    {
        SingleVarTransform const &t = singleTrans.at(iVar);
        vars[iVar] = (vars[iVar] - t.mean) / t.sigma;
    }
}

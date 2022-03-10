#include "TransformPCA.hpp"

using namespace logger;

TransformPCA::TransformPCA(Logger &log_, unsigned dim_) : TransformBase(log, dim)
{
    log << error << "Principal component analysis is not implemented yet." << eom;
    exit(1);
}

TransformPCA::~TransformPCA() noexcept
{
}

void TransformPCA::WriteCode(std::ostream & /*outStream*/, std::string const & /*postfix*/, bool isPython) const
{
}

void TransformPCA::AddEventImp(Double_t /*weight*/, Double_t const * /*vars*/)
{
}

void TransformPCA::BuildTransformationImp()
{
}

void TransformPCA::ApplyTransformationImp(Double_t * /*vars*/)
{
}

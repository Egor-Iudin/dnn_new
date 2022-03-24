/**
 * \author Andrey Popov
 *
 * The module performs principal component analysis with the input variables.
 */

#pragma once

#include "TransformBase.hpp"

//#include <TPrincipal.h>

/**
 * \brief Performs principal component analysis (PCA).
 *
 * Performs principal component analysis (PCA).
 *
 * ROOT implementation of PCA through TPrincipal class does not handle weighted events. One can
 * reimplement this class adding weights in the calculation of the covariance matrix or implement
 * decorrelation of the input variables (à la TMVA) by hands. In the latter case ROOT class
 * TMatrixDSymEigen and TMVA class VariableDecorrTransform might be useful.
 *
 * The transformation is switched off at the moment.
 */
class TransformPCA : public TransformBase
{
public:
    /// Constructor
    TransformPCA(logger::Logger &log_, unsigned dim_);

    /// Destructor
    ~TransformPCA() noexcept;

    /// Copy constructor (not allowed to be used)
    TransformPCA(TransformPCA const &) = delete;

    /// Assignment operator (not allowed to be used)
    TransformPCA const &operator=(TransformPCA const &) = delete;

public:
    /// Generates C++ code reperesenting a class to perform the transformation
    void WriteCode(std::ostream &outStream, std::string const &postfix, bool isPython) const;
    // void WriteCode(std::ostream &outStream, std::string const &postfix) const;

private:
    /// Presents an event to update the information needed to build the transformation
    void AddEventImp(Double_t weight, Double_t const *vars);

    /// Builds the transformation
    void BuildTransformationImp();

    /// Transforms the given input
    void ApplyTransformationImp(Double_t *vars);
};

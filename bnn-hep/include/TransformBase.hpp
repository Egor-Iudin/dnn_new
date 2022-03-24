/**
 * \author Andrey Popov
 *
 * The module defines the base class for the transformation of input variables.
 */

#pragma once

#include "Logger.hpp"

#include <Rtypes.h>

#include <ostream>
#include <string>

/// The abstract base class for the transformation of input variables.
class TransformBase
{
public:
    /// Constructor
    TransformBase(logger::Logger &log_, unsigned dim_);

    /// Destructor
    virtual ~TransformBase() = default;

public:
    /// Presents an event to update the information needed to build the transformation
    void AddEvent(Double_t weight, Double_t const *vars);

    /// Builds the transformation
    void BuildTransformation();

    /// Transforms the given input
    void ApplyTransformation(Double_t *vars);

    /// Generates C++ code reperesenting a class to perform the transformation
    //        virtual void WriteCode(std::ostream &outStream, std::string const &postfix,bool isPython) const = 0;
    virtual void WriteCode(std::ostream &outStream, std::string const &postfix, bool isPython) const = 0;
    // virtual void WriteCode(std::ostream &outStream, std::string const &postfix) const = 0;

protected:
    /// Virtual implementation of AddEvent functionality
    virtual void AddEventImp(Double_t weight, Double_t const *vars) = 0;

    /// Virtual implementation of BuildTrasformation functionality
    virtual void BuildTransformationImp() = 0;

    /// Virtual implementation of ApplyTransformation functionality
    virtual void ApplyTransformationImp(Double_t *vars) = 0;

protected:
    logger::Logger &log; ///< Logger instance
    unsigned dim;        ///< Dimensionality of the inputs

public:
    bool transformationBuilt; ///< Shows whether the transformation is build
};

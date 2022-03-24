/**
 * \author Andrey Popov
 *
 * The module defines the transformation of standardization for the input variables.
 */

#pragma once

#include "TransformBase.hpp"

#include <vector>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/weighted_mean.hpp>
#include <boost/accumulators/statistics/weighted_variance.hpp>
// http://www.boost.org/doc/libs/1_50_0/doc/html/accumulators/user_s_guide.html

using std::vector;
using namespace boost::accumulators;

typedef accumulator_set<Double_t,
                        stats<tag::weighted_mean(immediate), tag::weighted_variance(immediate)>, Double_t>
    MeanVarAccumulator_t;

/// Standardize the input vaiables (makes them zero mean and unit variance)
class TransformStandard : public TransformBase
{
private:
    /// Transformation for a single variable
    struct SingleVarTransform
    {
        Double_t mean, sigma;        ///< Mean and variance over the given variable
        MeanVarAccumulator_t *accum; ///< Boost accumulator

        /// Default constructor
        SingleVarTransform() : mean(0.), sigma(0.), accum(nullptr)
        {
        }
    };

public:
    /// Constuctor
    TransformStandard(logger::Logger &log_, unsigned dim_);

    /// Descructor
    ~TransformStandard() noexcept;

    /// Copy constructor (not allowed to be used)
    TransformStandard(TransformStandard const &) = delete;

    /// Assignment operator (not allowed to be used)
    TransformStandard const &operator=(TransformStandard const &) = delete;

public:
    /// Generates C++ code reperesenting a class to perform the transformation
    void WriteCode(std::ostream &outStream, std::string const &postfix, bool isPython) const;

private:
    /// Presents an event to update the information needed to build the transformation
    void AddEventImp(Double_t weight, Double_t const *vars);

    /// Builds the transformation
    void BuildTransformationImp();

    /// Transforms the given input
    void ApplyTransformationImp(Double_t *vars);

private:
    /// Individual (independent) transformations for each variable
    vector<SingleVarTransform> singleTrans;
};

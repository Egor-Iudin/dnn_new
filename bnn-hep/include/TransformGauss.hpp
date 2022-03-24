/**
 * \author Andrey Popov
 *
 * The module performs "gaussianisation" of the input variables, i.e. it transforms them in such a
 * way that they are distributed according to Gaussian. The initial distribution is expected to be
 * continious (otherwise the resulting distribution will not be Gaussian).
 */

#pragma once

#include "TransformBase.hpp"

#include <vector>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/weighted_p_square_cumul_dist.hpp>
#include <boost/accumulators/statistics/weighted_extended_p_square.hpp>

using std::vector;
using namespace boost::accumulators;

typedef accumulator_set<Double_t, stats<tag::weighted_p_square_cumulative_distribution>, Double_t>
    cumulative_t;
typedef accumulator_set<Double_t, stats<tag::weighted_extended_p_square>, Double_t> quantile_t;

/**
 * \brief Performs "gaussianisation" of the input variables.
 *
 * Transfroms the input variables in order to make thier distributions be Gaussian (each variable is
 * considered independently). The initial distributions are expected to be continuous. The program
 * does not fail otherwise but the resulted distributions will not be Gaussian. The transformation
 * is taken from TMVA users' guide.
 *
 * Cumulative distribution needed for the transformation is calculated with the help of Boost
 * accumulators. Additionally quantiles implemented in the same package are used to determine the
 * range for each variable. However the range boundaries are merely added as two additional points,
 * all the calculated points in CDF (also those falling outside the range) are kept. CDF is
 * interpolated linearly.
 */
class TransformGauss : public TransformBase
{
private:
    /// Struct defines the transformation for single variable
    struct SingleVarTransform
    {
        /// Default constructor
        SingleVarTransform() : accum(nullptr), range(nullptr),
                               cdfBins(0), x(nullptr), cdf(nullptr)
        {
        }

        /// Boost accumulator to calculate the cumulative distribution
        cumulative_t *accum;
        /// Boost accumulators to estimate the quantiles needed to determine the range
        quantile_t *range;
        /// Number of bins in CDF histogram
        UInt_t cdfBins;
        /// Arrays with values of the variable and corresponding values of CDF
        Double_t *x, *cdf;
    };

public:
    /**
     * \brief Constructor.
     *
     * Constructor. The input arguements are:
     *  \param log_ Reference to the logger object.
     *  \param dim_ Number of input variables.
     *  \param nBins Desired number of bins in the CDF histogram.
     *  \param tailFraction Quantiles tailFraction and (1 - tailFraction) are used to define
     * additional points in CDF. If a negative value is provided, it is set to a reasonable
     * default.
     */
    TransformGauss(logger::Logger &log_, unsigned dim_, unsigned nBins = 50,
                   double tailFraction_ = -1.);

    /// Descructor
    ~TransformGauss() noexcept;

    /// Copy constructor (not allowed to be used)
    TransformGauss(TransformGauss const &) = delete;

    /// Assignment operator (not allowed to be used)
    TransformGauss const &operator=(TransformGauss const &) = delete;

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

private:
    /// Individual (independent) transformations for each variable
    vector<SingleVarTransform> singleTrans;
    /// Quantiles tailFracton and (1. - tailFraction) are used to set additional points in CDF
    double const tailFraction;
};

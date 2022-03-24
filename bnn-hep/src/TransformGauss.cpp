#include "TransformGauss.hpp"

#include <cmath>
#include <list>
#include <algorithm>

#include <TMath.h>

using namespace logger;

typedef boost::iterator_range<std::vector<std::pair<Double_t, Double_t>>::iterator> histogram_t;

// A structure to represent a histogram bin
struct Bin
{
    Bin(Double_t edge_, Double_t content_) : edge(edge_), content(content_)
    {
    }

    bool operator<(Bin const &rhs) const
    {
        return (edge < rhs.edge);
    }

    Double_t edge;
    Double_t content;
};

TransformGauss::TransformGauss(Logger &log_, unsigned dim_, unsigned nBins /*= 50*/,
                               double tailFraction_ /*= -1.*/) : TransformBase(log_, dim_), singleTrans(dim_),
                                                                 tailFraction((tailFraction_ > 0.) ? tailFraction_ : 0.5 / nBins)
{
    for (auto &t : singleTrans)
    {
        t.accum =
            new cumulative_t(tag::weighted_p_square_cumulative_distribution::num_cells = nBins);

        double const probs[2] = {tailFraction, 1. - tailFraction};
        t.range = new quantile_t(tag::weighted_extended_p_square::probabilities = probs);
    }
}

TransformGauss::~TransformGauss() noexcept
{
    for (auto &t : singleTrans)
    {
        delete t.accum;
        t.accum = nullptr;

        delete t.range;
        t.range = nullptr;

        delete[] t.x;
        t.x = nullptr;

        delete[] t.cdf;
        t.cdf = nullptr;
    }
}

void TransformGauss::WriteCode(std::ostream &outStream, std::string const &postfix, bool isPython) const
{
    // Find the maximal number of bins in a CDF (most likely they all have the equal numbers of
    // bins, but still)
    unsigned maxBins = 0;

    for (auto const &t : singleTrans)
        if (t.cdfBins > maxBins)
            maxBins = t.cdfBins;

    // Short declaration of the class
    outStream << "class Transform" << postfix << "\n{\n\tpublic:\n";
    outStream << "\t\tTransform" << postfix << "();\n";
    outStream << "\t\tvoid operator()(Double_t *vars) const;\n\n";
    outStream << "\tprivate:\n"
              << "\t\tUInt_t nBins[" << dim << "];\n"
              << "\t\tDouble_t x[" << dim << "][" << maxBins << "], cdf[" << dim << "][" << maxBins << "];\n};\n\n";

    // Define the constructor
    outStream << "Transform" << postfix << "::Transform" << postfix << "()\n{\n";
    unsigned iVar = 0;

    for (auto const &t : singleTrans)
    {
        outStream << "\tnBins[" << iVar << "] = " << t.cdfBins << ";\n\t";
        unsigned bin = 0;

        for (; bin < t.cdfBins; ++bin)
            outStream << "x[" << iVar << "][" << bin << "] = " << t.x[bin] << "; ";

        for (; bin < maxBins; ++bin)
            outStream << "x[" << iVar << "][" << bin << "] = 0.; ";

        outStream << "\n\t";

        for (bin = 0; bin < t.cdfBins; ++bin)
            outStream << "cdf[" << iVar << "][" << bin << "] = " << t.cdf[bin] << "; ";

        for (; bin < maxBins; ++bin)
            outStream << "cdf[" << iVar << "][" << bin << "] = 0.; ";

        outStream << "\n";
        ++iVar;
    }

    outStream << "}\n\n";

    // Define operator()
    outStream << "void Transform" << postfix << "::operator()(Double_t *vars) const\n{\n";
    outStream << "\tfor (unsigned iVar = 0; iVar < " << dim << "; ++iVar)\n\t{\n";
    outStream << "\t\tDouble_t cumulative;\n\t\tint bin = -1;\n\t\t\n";

    outStream << "\t\twhile (bin + 1 < int(nBins[iVar])  &&  x[iVar][bin + 1] < vars[iVar])\n"
              << "\t\t\t++bin;\n\t\t\n";

    outStream << "\t\tif (bin == -1)\n\t\t\tcumulative = 1.e-5;\n"
              << "\t\telse if (bin == int(nBins[iVar]) - 1)\n\t\t\tcumulative = cdf[iVar][bin];\n";
    outStream << "\t\telse\n\t\t{\n\t\t\tcumulative = cdf[iVar][bin];\n\t\t\t\n"
              << "\t\t\tif (x[iVar][bin + 1] != x[iVar][bin])\n"
              << "\t\t\t\tcumulative += (cdf[iVar][bin + 1] - cdf[iVar][bin]) / "
              << "(x[iVar][bin + 1] - x[iVar][bin]) * (vars[iVar] - x[iVar][bin]);\n\t\t}\n\t\t\n";

    outStream << "\t\tif (cumulative < 1.e-5)\n\t\t\tcumulative = 1.e-5;\n"
              << "\t\telse if (cumulative > 1. - 1.e-5)\n\t\t\tcumulative = 1. - 1.e-5;\n\t\t\n";

    outStream << "\t\tvars[iVar] = M_SQRT2 * TMath::ErfInverse(2. * cumulative - 1.);\n";
    outStream << "\t}\n}\n\n\n";
}

void TransformGauss::AddEventImp(Double_t w, Double_t const *vars)
{
    for (unsigned iVar = 0; iVar < dim; ++iVar)
    {
        singleTrans.at(iVar).accum->operator()(vars[iVar], weight = w);
        singleTrans.at(iVar).range->operator()(vars[iVar], weight = w);
    }
}

void TransformGauss::BuildTransformationImp()
{
    for (auto &t : singleTrans)
    {
        // Extract the results from the accumulators
        histogram_t cdfHist = weighted_p_square_cumulative_distribution(*t.accum);
        auto const range = weighted_extended_p_square(*t.range);

        // Copy the Boost histogram into a list
        std::list<Bin> hist;

        for (auto const &b : cdfHist)
            hist.emplace_back(b.first, b.second);

        // Find the position one can insert the tailFraction quantile at
        Bin bin(range[0], tailFraction);
        auto it = std::upper_bound(hist.begin(), hist.end(), bin);
        hist.insert(it, bin);

        // Same for (1. - tailFraction)
        bin = Bin(range[1], 1. - tailFraction);
        it = std::upper_bound(hist.begin(), hist.end(), bin);
        hist.insert(it, bin);

        t.cdfBins = hist.size();
        t.x = new Double_t[t.cdfBins];
        t.cdf = new Double_t[t.cdfBins];

        // Copy the list to the two arrays in the current SingleVarTransform
        unsigned i = 0;
        for (auto const &b : hist)
        {
            t.x[i] = b.edge;
            t.cdf[i] = b.content;
            ++i;
        }

        // We don't need the accumulator anymore
        delete t.accum;
        t.accum = nullptr;
        delete t.range;
        t.range = nullptr;
    }
}

void TransformGauss::ApplyTransformationImp(Double_t *vars)
{
    for (unsigned iVar = 0; iVar < dim; ++iVar)
    {
        SingleVarTransform const &t = singleTrans.at(iVar);
        Double_t cumulative;

        // Find the bin in CDF histogram which the variable gets in
        int bin = -1; // (-1) is underflow, (t.cdfBins - 1) is overflow

        while (bin + 1 < int(t.cdfBins) and t.x[bin + 1] < vars[iVar])
            ++bin;

        if (bin == -1)
            cumulative = 1.e-5;
        else if (bin == int(t.cdfBins) - 1)
            cumulative = t.cdf[bin];
        else
        {
            cumulative = t.cdf[bin];

            // Interpolate
            if (t.x[bin + 1] != t.x[bin]) // they can be exactly equal in some pathologic cases
                cumulative += (t.cdf[bin + 1] - t.cdf[bin]) / (t.x[bin + 1] - t.x[bin]) *
                              (vars[iVar] - t.x[bin]);
        }

        // Make sure CDF is not equal to 0 or 1 (as erf^-1 will be calculated with it)
        if (cumulative < 1.e-5)
            cumulative = 1.e-5;
        else if (cumulative > 1. - 1.e-5)
            cumulative = 1. - 1.e-5;

        // Complete the transformation
        vars[iVar] = M_SQRT2 * TMath::ErfInverse(2. * cumulative - 1.);
    }
}

#include "NeuralNetwork.hpp"

#include <algorithm>
#include <stdexcept>
#include <cmath>

NeuralNetwork::NeuralNetwork() : nLayers(0), nNodes(nullptr), biases(nullptr), weights(nullptr),
                                 layerOutputs(nullptr), isClassification(true)
{
}

NeuralNetwork::NeuralNetwork(unsigned nLayers_, unsigned const *nNodes_) : nLayers(0), nNodes(nullptr), biases(nullptr), weights(nullptr),
                                                                           layerOutputs(nullptr), isClassification(true)
{
    SetArchitecture(nLayers_, nNodes_);
}

NeuralNetwork::NeuralNetwork(std::vector<unsigned> const &nNodes_) : NeuralNetwork(nNodes_.size(), nNodes_.data())
{
}

NeuralNetwork::NeuralNetwork(NeuralNetwork const &nn)
{
    SetArchitecture(nn.nLayers, nn.nNodes);
    isClassification = nn.isClassification;

    // Copy the biases and weights
    for (unsigned l = 1; l < nLayers; ++l)
    {
        std::copy(nn.biases[l - 1], nn.biases[l - 1] + nn.nLayers, biases[l - 1]);

        for (unsigned n = 0; n < nNodes[l]; ++n)
            std::copy(nn.weights[l - 1][n], nn.weights[l - 1][n] + nn.nNodes[l - 1],
                      weights[l - 1][n]);
    }
}

NeuralNetwork::NeuralNetwork(NeuralNetwork &&nn) : nLayers(nn.nLayers), nNodes(nn.nNodes), biases(nn.biases), weights(nn.weights),
                                                   layerOutputs(nullptr), isClassification(nn.isClassification)
{
    nn.nLayers = 0;
    nn.nNodes = nullptr;
    nn.biases = nullptr;
    nn.weights = nullptr;
}

NeuralNetwork::~NeuralNetwork()
{
    Delete();
    delete[] layerOutputs;
}

NeuralNetwork const &NeuralNetwork::operator=(NeuralNetwork const &nn)
{
    SetArchitecture(nn.nLayers, nn.nNodes);
    isClassification = nn.isClassification;

    // Copy the biases and weights
    for (unsigned l = 1; l < nLayers; ++l)
    {
        std::copy(nn.biases[l - 1], nn.biases[l - 1] + nn.nLayers, biases[l - 1]);

        for (unsigned n = 0; n < nNodes[l]; ++n)
            std::copy(nn.weights[l - 1][n], nn.weights[l - 1][n] + nn.nNodes[l - 1],
                      weights[l - 1][n]);
    }

    return *this;
}

void NeuralNetwork::SetArchitecture(unsigned nLayers_, unsigned const *nNodes_)
{
    // Check if the current architecture is the same as the desired one
    if (nLayers == nLayers_ and std::equal(nNodes, nNodes + nLayers, nNodes_))
        return;

    if (nLayers_ < 3)
        throw std::logic_error("The neural network cannot contain less than 3 layers.");

    Delete();
    delete[] layerOutputs;

    // Allocate the memory
    nLayers = nLayers_;
    nNodes = new unsigned[nLayers];
    std::copy(nNodes_, nNodes_ + nLayers_, nNodes);
    unsigned maxNNodes = nNodes[0]; // needed to allocate layerOutputs

    biases = new double *[nLayers - 1];
    weights = new double **[nLayers - 1];

    for (unsigned l = 1; l < nLayers; ++l)
    {
        biases[l - 1] = new double[nNodes[l]];
        weights[l - 1] = new double *[nNodes[l]];

        for (unsigned n = 0; n < nNodes[l]; ++n)
            weights[l - 1][n] = new double[nNodes[l - 1]];

        if (nNodes[l] > maxNNodes)
            maxNNodes = nNodes[l];
    }

    layerOutputs = new double[maxNNodes];
}

void NeuralNetwork::SetArchitecture(std::initializer_list<unsigned> const &nNodes_)
{
    std::vector<unsigned> nums;
    nums.reserve(nNodes_.size());

    for (auto const &n : nNodes_)
        nums.push_back(n);

    SetArchitecture(nums.size(), nums.data());
}

void NeuralNetwork::SetBiases(unsigned layer, double const *biases_)
{
    if (layer == 0 or layer >= nLayers)
        throw std::range_error("Illegal layer index.");

    std::copy(biases_, biases_ + nNodes[layer], biases[layer - 1]);
}

void NeuralNetwork::SetBiases(unsigned layer, std::vector<double> const &biases_)
{
    if (layer == 0 or layer >= nLayers)
        throw std::range_error("Illegal layer index.");

    if (biases_.size() != nNodes[layer])
        throw std::length_error("The length of the given vector does not match the architecture.");

    std::copy(biases_.begin(), biases_.end(), biases[layer - 1]);
}

void NeuralNetwork::SetWeights(unsigned layer, unsigned node, double const *weights_)
{
    if (layer == 0 or layer >= nLayers)
        throw std::range_error("Illegal layer index.");

    if (node >= nNodes[layer])
        throw std::range_error("Illegal node index.");

    std::copy(weights_, weights_ + nNodes[layer - 1], weights[layer - 1][node]);
}

void NeuralNetwork::SetWeights(unsigned layer, unsigned node, std::vector<double> const &weights_)
{
    if (layer == 0 or layer >= nLayers)
        throw std::range_error("Illegal layer index.");

    if (node >= nNodes[layer])
        throw std::range_error("Illegal node index.");

    if (weights_.size() != nNodes[layer - 1])
        throw std::length_error("The length of the given vector does not match the architecture.");

    std::copy(weights_.begin(), weights_.end(), weights[layer - 1][node]);
}

double const *NeuralNetwork::Apply(double const *vars) const
// CAVEAT: This does not work correctly: layerOutput is spoiled while it is still needed!
{
    std::copy(vars, vars + nNodes[0], layerOutputs);

    // Evaluate the output layer-by-layer
    for (unsigned l = 1; l < nLayers; ++l)
    {
        for (unsigned n = 0; n < nNodes[l]; ++n)
        {
            double sum = biases[l - 1][n];

            for (unsigned np = 0; np < nNodes[l - 1]; ++np) // iterate over the previous layer
                sum += layerOutputs[np] * weights[l - 1][n][np];

            if (l == nLayers - 1) // the activation function is not applied to be output layer
                layerOutputs[n] = sum;
            else
                layerOutputs[n] = std::tanh(sum);
        }
    }

    // Transfrom the outputs
    if (isClassification)
        for (unsigned n = 0; n < nNodes[nLayers - 1]; ++n)
            layerOutputs[n] = 1. / (1 + std::exp(-layerOutputs[n]));

    return layerOutputs;
}

double const *NeuralNetwork::Apply(std::vector<double> const &vars) const
{
    if (vars.size() != nNodes[0])
        throw std::length_error("The length of the given vector does not match the number of input "
                                "neurons.");

    return Apply(vars.data());
}

void NeuralNetwork::SetClassification(bool switcher /*=true*/)
{
    isClassification = switcher;
}

double &NeuralNetwork::GetWeight(unsigned layer, unsigned node, unsigned nodePrev)
{
    if (layer == 0)
        throw std::logic_error("No weights are associated with the layer #0.");

    if (layer >= nLayers or node >= nNodes[layer] or nodePrev >= nNodes[layer - 1])
        throw std::range_error("Illegal index when accessing weights.");

    return weights[layer - 1][node][nodePrev];
}

double &NeuralNetwork::GetBias(unsigned layer, unsigned node)
{
    if (layer == 0)
        throw std::logic_error("No biases are associated with the layer #0.");

    if (layer >= nLayers or node >= nNodes[layer])
        throw std::range_error("Illegal index when accessing biases.");

    return biases[layer - 1][node];
}

void NeuralNetwork::WriteClass(std::ostream &outStream) const
{
    // Write the short class description
    outStream << "class NN\n"
              << "{\n"
              << "\tpublic:\n"
              << "\t\tNN();\n"
              << "\t\n"
              << "\tpublic:\n";

    for (unsigned l = 1; l < nLayers; ++l)
        outStream << "\t\tvoid SetWeightsL" << l << "(Double_t const [" << nNodes[l] << "][" << nNodes[l - 1] << "]);\n"
                  << "\t\tvoid SetBiasesL" << l << "(Double_t const [" << nNodes[l] << "]);\n";

    outStream << "\t\tDouble_t const * Apply(Double_t const *) const;\n"
              << "\t\n\tprivate:\n";

    for (unsigned l = 1; l < nLayers; ++l)
        outStream << "\t\tDouble_t weightsL" << l << "[" << nNodes[l] << "][" << nNodes[l - 1] << "];\n"
                  << "\t\tDouble_t biasesL" << l << "[" << nNodes[l] << "];\n";

    unsigned const maxNodes = *std::max_element(nNodes, nNodes + nLayers);
    outStream << "\t\tmutable Double_t bufferIn[" << maxNodes << "];\n"
              << "\t\tmutable Double_t bufferOut[" << maxNodes << "];\n"
              << "};\n\n\n";

    // Define the methods
    outStream << "NN::NN()\n{}\n\n\n";

    for (unsigned l = 1; l < nLayers; ++l)
    {
        outStream << "void NN::SetWeightsL" << l << "(Double_t const weights[" << nNodes[l] << "][" << nNodes[l - 1] << "])\n"
                  << "{\n"
                  << "\tfor (unsigned n = 0; n < " << nNodes[l] << "; ++n)\n"
                  << "\t\tfor (unsigned np = 0; np < " << nNodes[l - 1] << "; ++np)\n"
                  << "\t\t\tweightsL" << l << "[n][np] = weights[n][np];\n"
                  << "}\n\n\n";
        outStream << "void NN::SetBiasesL" << l << "(Double_t const biases[" << nNodes[l] << "])\n"
                  << "{\n"
                  << "\tstd::copy(biases, biases + " << nNodes[l] << ", biasesL" << l << ");\n"
                  << "}\n\n\n";
    }

    // The Apply() method is the most complex one
    outStream << "Double_t const * NN::Apply(Double_t const *vars) const\n"
              << "{\n"
              << "\tstd::copy(vars, vars + " << nNodes[0] << ", bufferIn);\n\t\n";

    // Loop over all the layers but the last one (and, of course, the input one)
    for (unsigned l = 1; l < nLayers - 1; ++l)
    {
        outStream << "\tfor (unsigned n = 0; n < " << nNodes[l] << "; ++n)\n"
                  << "\t{\n"
                  << "\t\tbufferOut[n] = biasesL" << l << "[n];\n\t\n"
                  << "\t\tfor (unsigned np = 0; np < " << nNodes[l - 1] << "; ++np)\n"
                  << "\t\t\tbufferOut[n] += weightsL" << l << "[n][np] * bufferIn[np];\n"
                  << "\t}\n\t\n"
                  << "\tfor (unsigned n = 0; n < " << nNodes[l] << "; ++n)\n"
                  << "\t\tbufferIn[n] = TMath::TanH(bufferOut[n]);\n\t\n";
    }

    // Now the output layer (the difference is in the activation function)
    outStream << "\tfor (unsigned n = 0; n < " << nNodes[nLayers - 1] << "; ++n)\n"
              << "\t{\n"
              << "\t\tbufferOut[n] = biasesL" << nLayers - 1 << "[n];\n\t\n"
              << "\t\tfor (unsigned np = 0; np < " << nNodes[nLayers - 2] << "; ++np)\n"
              << "\t\t\tbufferOut[n] += weightsL" << nLayers - 1 << "[n][np] * bufferIn[np];\n"
              << "\t}\n\t\n";

    if (isClassification)
        outStream << "\tfor (unsigned n = 0; n < " << nNodes[nLayers - 1] << "; ++n)\n"
                  << "\t\tbufferIn[n] = 1. / (1 + TMath::Exp(-bufferOut[n]));\n\t\n";

    outStream << "\treturn bufferIn;\n"
              << "}\n\n\n";
}

void NeuralNetwork::WriteInitialization(std::ostream &outStream, std::string const &indent,
                                        std::string const &netPrefix, std::string const &uniquePostfix) const
{
    for (unsigned l = 1; l < nLayers; ++l)
    {
        outStream << indent << "Double_t weightsL" << l << "_" << uniquePostfix << "[" << nNodes[l] << "][" << nNodes[l - 1] << "] = {";

        for (unsigned n = 0; n < nNodes[l]; ++n)
        {
            outStream << "{" << weights[l - 1][n][0];

            for (unsigned np = 1; np < nNodes[l - 1]; ++np)
                outStream << ", " << weights[l - 1][n][np];

            outStream << "}";

            if (n != nNodes[l] - 1)
                outStream << ", ";
        }

        outStream << "};\n";

        outStream << indent << "Double_t biasesL" << l << "_" << uniquePostfix << "[" << nNodes[l] << "] = {" << biases[l - 1][0];

        for (unsigned n = 1; n < nNodes[l]; ++n)
            outStream << ", " << biases[l - 1][n];

        outStream << "};\n";

        outStream << indent << netPrefix << "SetWeightsL" << l << "(weightsL" << l << "_" << uniquePostfix << ");\n"
                  << indent << netPrefix << "SetBiasesL" << l << "(biasesL" << l << "_" << uniquePostfix << ");\n";

        outStream << indent << '\n';
    }
}

void NeuralNetwork::Delete()
{
    if (nNodes)
    {
        if (biases)
        {
            for (unsigned l = 1; l < nLayers; ++l)
                delete[] biases[l - 1];

            delete[] biases;
            biases = nullptr;
        }

        if (weights)
        {
            for (unsigned l = 1; l < nLayers; ++l)
            {
                for (unsigned n = 0; n < nNodes[l]; ++n)
                    delete[] weights[l - 1][n];

                delete[] weights[l - 1];
            }

            delete[] weights;
            weights = nullptr;
        }

        delete[] nNodes;
        nNodes = nullptr;
    }
}

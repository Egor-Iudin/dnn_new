/**
 * \author Andrey Popov
 *
 * The module describes a neural network (NN).
 */

#pragma once

#include <initializer_list>
#include <vector>
#include <ostream>

/**
 * \brief The class describes a neural network.
 *
 * The class describes an artificial neural network. The architecture is a multilayer perceptron.
 * No method to apply the network is currently implemented.
 */
class NeuralNetwork
{
public:
    /// Default constructor
    NeuralNetwork();

    /// Constructor that defines the architecture of the network
    NeuralNetwork(unsigned nLayers_, unsigned const *nNodes_);

    /// Constructor that defines the architecture of the network
    NeuralNetwork(std::vector<unsigned> const &nNodes_);

    /// Copy constructor
    NeuralNetwork(NeuralNetwork const &nn);

    /// Move constructor
    NeuralNetwork(NeuralNetwork &&nn);

    /// Destructor
    ~NeuralNetwork();

    /// Assignment operator
    NeuralNetwork const &operator=(NeuralNetwork const &nn);

public:
    /**
     * \brief Defines the architecture.
     *
     * Defines the architecture i.e. the number of layers and number of nodes in each layer. The
     * input and the output layers are counted, hence nLayers_ cannot be smaller than 3.
     */
    void SetArchitecture(unsigned nLayers_, unsigned const *nNodes_);
    /**
     * \brief Defines the architecture.
     *
     * Defines the architecture i.e. the number of layers and number of nodes in each layer. The
     * number of layers is defined implicitly as the number of elements in the initializer list.
     * The input and the output layers are counted, hence there cannot be smaller than 3 layers.
     */
    void SetArchitecture(std::initializer_list<unsigned> const &nNodes_);
    /// Sets biasess for the given layer
    void SetBiases(unsigned layer, double const *biases_);
    /// Sets biases for the given layer
    void SetBiases(unsigned layer, std::vector<double> const &biases_);
    /// Sets the weights for the given node in the given layer
    void SetWeights(unsigned layer, unsigned node, double const *weights_);
    /// Sets the weights for the given node in the given layer
    void SetWeights(unsigned layer, unsigned node, std::vector<double> const &weights_);
    /// Applies the NN to the given input
    double const *Apply(double const *vars) const;
    /// Applies the NN to the given input
    double const *Apply(std::vector<double> const &vars) const;
    /// Sets whether the outputs should be translated to [0, 1] range
    void SetClassification(bool switcher = true);
    /// Access the weights (intended for modification)
    double &GetWeight(unsigned layer, unsigned node, unsigned nodePrev);
    /// Access the biases (intended for modification)
    double &GetBias(unsigned layer, unsigned node);
    /// Writes a C++ class to handle the neural network
    void WriteClass(std::ostream &outStream) const;
    /**
     * \brief Writes C++ code to initialized a neural network.
     *
     * Writes C++ code to initialized a neural network. It should be constructed with the
     * default constructor in advance.
     *  \param outStream The stream to write the code to.
     *  \param indent An indent to be added at the beginning of each line.
     *  \param netPrefix The NN object name. It should include also the access specificator
     * (i.e. the dot or the arrow ->).
     *  \param uniquePostfix A postfix to add to the names of all the constructed arrays.
     */
    void WriteInitialization(std::ostream &outStream, std::string const &indent,
                             std::string const &netPrefix, std::string const &uniquePostfix) const;

private:
    /// Frees the memory used to keep the neural network's definition
    void Delete();

private:
    /// Total number of layers
    unsigned nLayers;
    /// Number of nodes in each layer
    unsigned *nNodes;
    /**
     * \brief Biases.
     *
     * Biases in each layer except for the input one (biases not applied to it). The first
     * index of the 2D array is the index of the layer (minus 1), the second index is the index
     * of the node in the layer.
     */
    double **biases;
    /**
     * \brief Weights.
     *
     * Weights for each consequitive pair of layers. The first index of the 3D array is the
     * index of the backward layer in the pair, subtracted 1, the second index is the index of
     * the node in the backward layer, and the third index is the index of paired node in the
     * forward layer. The input layer does not have weights associated with it.
     */
    double ***weights;
    /// Outputs of the nodes in some layer. Used to simplify the evaluation of the network
    mutable double *layerOutputs;
    /// Indicated whether the outputs should be translated to [0, 1] range
    bool isClassification;
};

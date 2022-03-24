/**
 * \author Andrey Popov
 *
 * The module executes FBM routines providing them the necessary input.
 */

#pragma once

#include "Logger.hpp"
#include "Config.hpp"
#include "InputProcessor.hpp"
#include "NeuralNetwork.hpp"

/**
 * \brief Class executes FBM routines.
 */
class FBMWrapper
{
public:
    /// Constructor
    FBMWrapper(logger::Logger &log_, Config const &config_,
               InputProcessor const &inputProcessor_);

    /// Destructor
    ~FBMWrapper();

    /// Copy constructor (not allowed to be used)
    FBMWrapper(FBMWrapper const &) = delete;

    /// Assignment operator (not allowed to be used)
    FBMWrapper const &operator=(FBMWrapper const &) = delete;

private:
    /// Runs FBM utilities to perform the training
    std::vector<std::string> TrainBNN() const;
    /// Displays an error message and exits
    void ErrorWrongOutput(std::string const &command) const;

public:
    /**
     * \brief Reads the given NN from the binary BNN file.
     *
     * Reads a NN with the given index from the binary BNN file. The net-display routine from
     * FBM is used, therefore the rounding errors can be introduced. The index is translated
     * unchanged to FBM; please note that normally the NN at position 0 corresponds to the
     * initial state and should never be considered for any inference.
     */
    NeuralNetwork ReadNN(unsigned index) const;

    std::vector<std::string> netFiles;

private:
    logger::Logger &log;                  ///< Logger instance
    Config const &config;                 ///< Config instance
    InputProcessor const &inputProcessor; ///< Input processor instance
    std::string const &FBMPath;           ///< Local reference to the path to FBM routines
    std::string const &BNNFileName;       ///< Local reference to the name of binary BNN file
    std::vector<unsigned> NNArchitecture; ///< Number of nodes in each layer of the NN
};

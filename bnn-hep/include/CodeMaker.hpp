/**
 * \author Andrey Popov
 *
 * The module is responsible for building C++ code to perform variable transformation and apply
 * the trained BNN.
 */

#pragma once

#include "Logger.hpp"
#include "Config.hpp"
#include "InputProcessor.hpp"
#include "FBMWrapper.hpp"

#include <fstream>
#include <vector>

/**
 * \brief The class writes C++ code to apply the BNN.
 *
 * The class produces a header file containing the requested transformations of the input variables
 * and applies the selected neural networks after the transformation.
 */

class DNNCodeMaker
{
public:
    /**
     * \brief Constructor.
     *
     * Constructor. All the actions are executed within its body.
     */
    DNNCodeMaker(logger::Logger &log_, Config const &config_,
                 InputProcessor const &inputProcessor_);

    /// Destructor
    ~DNNCodeMaker();

    /// Copy constructor (not allowed to be used)
    DNNCodeMaker(DNNCodeMaker const &) = delete;

    /// Assignment operator (not allowed to be used)
    DNNCodeMaker const &operator=(DNNCodeMaker const &) = delete;

private:
    Logger &log;                          ///< Logger instance
    Config const &config;                 ///< Config instance
    InputProcessor const &inputProcessor; ///< Input processor instance
    std::ofstream filePy;                 ///< File that will store the source code
};

class CodeMaker
{
public:
    /**
     * \brief Constructor.
     *
     * Constructor. All the actions are executed within its body.
     */
    CodeMaker(logger::Logger &log_, Config const &config_,
              InputProcessor const &inputProcessor_, FBMWrapper const &fbm_);

    /// Destructor
    ~CodeMaker();

    /// Copy constructor (not allowed to be used)
    CodeMaker(CodeMaker const &) = delete;

    /// Assignment operator (not allowed to be used)
    CodeMaker const &operator=(CodeMaker const &) = delete;

private:
    /// Writes a class to incorporate the BNN
    void WriteBNNClass();

private:
    Logger &log;                          ///< Logger instance
    Config const &config;                 ///< Config instance
    InputProcessor const &inputProcessor; ///< Input processor instance
    FBMWrapper const &fbm;                ///< Instance of FBM wrapper
    std::ofstream file;                   ///< File that will store the source code
    std::vector<NeuralNetwork> nets;      ///< Ensamble of the neural networks
};

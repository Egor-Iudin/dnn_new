/**
 * \author Andrey Popov
 *
 * The module is responsible for parsing the configuration file. Internally it uses a version of
 * libconfig package included in bnn-hep.
 */

#pragma once

#include "libconfig.h++"
#include "Logger.hpp"

#include <string>
#include <vector>
#include <utility>

using std::string;
using std::vector;
using namespace logger;
using libconfig::Setting;
using libconfig::SettingNotFoundException;
using libconfig::SettingTypeException;

/**
 * \brief The class provides access to the configuration.
 *
 * The class provides access to the configuration. The configuration is read and parsed at
 * once; each request for a parameter is then resolved without consulting to the file.
 */
class Config
{
public:
    /// Structure to keep the information about a single input sample
    struct Sample
    {
        /// The class index. Currently it is either 1 ("signal") or 0 ("background")
        unsigned type;
        /// Name of the plain ROOT file with input variables
        string fileName;
        /// Relevant ROOT trees in the file
        vector<string> trees;
        /// Event selection and weight to construct the training set
        string trainWeight;
        /// Event selection and weight to construct the exam set (not used)
        string examWeight;
        /// Maximum number of events in the training set
        unsigned long maxTrainEvents;
        /// Maximum fraction of all the events that can be used for training
        float maxFractionTrainEvents;
        /// Name of the file with the list of events for training
        string trainEventsFileName;
    };

    /// Supported variats to preprocess the inputs
    enum class InputTransformation
    {
        Standard, ///< Zero mean and unit variance
        Gauss,    ///< Distributions are transformed to gaussian
        PCA       ///< Principal component analysis
    };

    /// Supported variats for the reweighting
    enum class Reweighting
    {
        /// Signal and background are rescaled simultaneously to get the proper normalization
        Common,
        /// Signal and background are rescaled independently to have equal impacts
        OneToOne
    };

public:
    /**
     * \brief Consructor.
     *
     * Constructor. Takes the name of the configuration file and a reference to logger object.
     * The latter is not constant as the logger's verbosity level is adjusted according to the
     * configuration. All the parsing and interpretation is performed in constructor.
     */
    Config(string const &fileName, Logger &log_);

    /// Destructor
    ~Config() = default;

    /// Copy constructor (not allowed to be used)
    Config(Config const &) = delete;

    /// Assignment operator (not allowed to be used)
    Config const &operator=(Config const &) = delete;

private:
    /**
     * \brief Finds the setting given its path.
     *
     * Locates the setting specified by the given path. If the value of the setting is of the
     * form "@another-path" the path is expanded accordingly. Recursion is allowed and there is
     * no protection against the loop references.
     */
    Setting const &LookupSetting(string const &path) throw(SettingNotFoundException);

    /**
     * \brief Finds the setting given its path. Does not throw exceptions.
     *
     * Locates the setting specified by the given path. If the value of the setting is of the
     * form "@another-path" the path is expanded accordingly. Recursion is allowed and there is
     * no protection against the loop references. If the setting is not found an error is
     * printed and the program is terminated.
     */
    Setting const &LookupMandatorySetting(string const &path) throw();

    /**
     * \brief Processes the recursive references.
     *
     * Checks if the given setting is a reference and expands it accordingly. Recursion is
     * allowed and there is no protection against the loop references.
     */
    Setting const &ExpandSetting(Setting const &setting) throw(SettingNotFoundException);

    /**
     * \brief Reads a parameter given its path.
     *
     * Method reads the value of the parameter given its path. The second argument is only to
     * provide the type of the parameter, its value is ignored. The references to other
     * parameters are expanded properly. If the setting is not found or has an unexpected type
     * an error message is printed and the program terminates.
     */
    template <typename T>
    T ReadParameter(string const &path, T const &)
    {
        try
        {
            Setting const &setting = LookupSetting(path);
            T res = setting;
            return res;
        }
        catch (SettingNotFoundException excp)
        {
            log << critical << "Mandatory setting \"" << excp.getPath() << "\" is not found "
                << "in the configuration." << eom;
            exit(1);
        }
        catch (SettingTypeException excp)
        {
            log << critical << "Setting \"" << excp.getPath() << "\" is of unexpected type." << eom;
            exit(1);
        }

        return T(); // this is never reached
    }

    /**
     * \brief Reads a parameter given its path. Default value is supported.
     *
     * Method reads the value of the parameter given its path. The references to other
     * parameters are expanded properly. If the setting is not found the provided default value
     * is used. If the setting has an unexpected type an error message is printed and the
     * program terminates.
     */
    template <typename T>
    T ReadParameterDef(string const &path, T const &defValue)
    {
        try
        {
            Setting const &setting = LookupSetting(path);
            T res = setting;
            return res;
        }
        catch (SettingNotFoundException excp)
        {
            return defValue;
        }
        catch (SettingTypeException excp)
        {
            log << critical << "Setting \"" << excp.getPath() << "\" is of unexpected type." << eom;
            exit(1);
        }

        return T(); // this is never reached
    }

    /**
     * \brief Reads a child parameter.
     *
     * Given the section, reads its child parameter. The second argument is only to provide
     * the type of the parameter, its value is ignored. The references to other parameters are
     * expanded properly. If the setting is not found or has an unexpected type an error message
     * is printed and the program is terminated.
     */
    template <typename T, typename I>
    T ReadChildParameter(Setting const &setting, I const &name, T const &)
    {
        try
        {
            Setting const &childSetting = ExpandSetting(setting[name]);
            T res = childSetting;
            return res;
        }
        catch (SettingNotFoundException excp)
        {
            log << critical << "Mandatory setting \"" << excp.getPath() << "\" is not found "
                << "in the configuration." << eom;
            exit(1);
        }
        catch (SettingTypeException excp)
        {
            log << critical << "Setting \"" << excp.getPath() << "\" is of unexpected type." << eom;
            exit(1);
        }

        return T(); // this is never reached
    }

    /**
     * \brief Reads a child parameter. Default value is supported.
     *
     * Given the section, reads its child parameter. The references to other parameters are
     * expanded properly. If the setting is not found the provided default value is used. If it
     * has an unexpected type an error message is printed and the program is terminated.
     */
    template <typename T, typename I>
    T ReadChildParameterDef(Setting const &setting, I const &name, T const &defValue)
    {
        try
        {
            Setting const &childSetting = ExpandSetting(setting[name]);
            T res = childSetting;
            return res;
        }
        catch (SettingNotFoundException excp)
        {
            return defValue;
        }
        catch (SettingTypeException excp)
        {
            log << critical << "Setting \"" << excp.getPath() << "\" is of unexpected type." << eom;
            exit(1);
        }

        return T(); // this is never reached
    }

    /**
     * \brief Extends the vector of input samples with the info from the given path.
     *
     * Processes the list of samples in the given path and add them to vector 'samples' with the
     * specified type (class decision).
     */
    void ReadSamples(string const &path, unsigned type, string const &defTrainWeight,
                     string const &defExamWeight, vector<string> const &defTreeNames);

public:
    // void SetEventListFile()
    void PrintEventListFile();
    void SetEventListFile();
    /// Returns the task name
    string const &GetTaskName() const;

    /// Returns the vector of input samples
    vector<Sample> const &GetSamples() const;

    /// Returns the vector of input variables
    vector<string> const &GetVariables() const;

    /// Returns the vector of transformations of input variables
    vector<InputTransformation> const &GetTransformations() const;

    /// Returns the path to FBM routines
    string const &GetFBMPath() const;

    /// Checks whether the temporary .net and .root files should be kept
    bool GetKeepTempFiles() const;

    /// Returns the path to network's binary file name
    string const &GetBNNFileName() const;
    string const &GetPyFileName() const;

    /// Returns the reweighting type
    Reweighting GetReweightingType() const;

    /// Returns the number of neurons in the hidden layer
    unsigned GetBNNNumberNeurons() const;

    /// Returns the hyperparameters defining the prior for BNN
    string const &GetBNNHyperparameters() const;

    /// Returns BNN generation parameters
    string const &GetBNNGenerationParameters() const;

    /// Returns MCMC parameters for BNN sampling: for the first and for the rest iterations.
    std::pair<string const &, string const &> GetBNNMCMCParameters() const;

    /// Returns the total number of iterations used for BNN sampling (uncluding the burn-in)
    unsigned GetBNNMCMCIterations() const;

    /// Returns the number of iterations for the burn-in (not used to create the final C++ code)
    unsigned GetBNNMCMCBurnIn() const;

    /// Reads the name of the file to store the final C++ code for the BNN
    string const &GetCPPFileName() const;

    std::vector<std::string> GetRootFiles();

private:
    Logger &log;                                      ///< Logger instance
    libconfig::Config cfg;                            ///< libconfig object to parse the configuration
    string taskName;                                  ///< Name of the task (used to construct some file names, etc.)
    string FBMPath;                                   ///< Path to FBM executables
    bool keepTempFiles;                               ///< Indicates whether the temporary files should be kept
    vector<string> variables;                         ///< The input variables to be read from files
    vector<Sample> samples;                           ///< The input samples
    string networkName;                               ///< Name of the binary file to store the BNN
    string networkShortName;                          ///< networkName with directories and extension stripped off
    Reweighting reweightingType;                      ///< Requested type of the reweighting
    unsigned numberNeurons;                           ///< Number of neurons in the hidden layer
    string networkHyperparameters;                    ///< Hyperparameters for BNN prior
    string networkGenerationParameters;               ///< Parameters used to generate the initial NNs
    string MCMCParametersFirstIt;                     ///< MCMC parameters for the first iteration
    string MCMCParameters;                            ///< MCMC parameters for all the rest iterations
    unsigned numberIterations;                        ///< Total number of MCMC iterations (burn-in included)
    unsigned burnInIterations;                        ///< Number of MCMC iterations to skip (burn-in)
    string networkCPPFileName;                        ///< Name of the output file to store C++ code of BNN
    string networkPYFileName;                         ///< Name of the output file to store C++ code of BNN
    vector<InputTransformation> inputTransformations; ///< Transformation for input vars
};

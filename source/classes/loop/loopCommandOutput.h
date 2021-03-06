/***********************************************/
/**
* @file loopCommandOutput.h
*
* @brief Loop over lines of command output.
*
* @author Sebastian Strasser
* @date 2017-02-10
*
*/
/***********************************************/

#ifndef __GROOPS_LOOPCOMMANDOUTPUT__
#define __GROOPS_LOOPCOMMANDOUTPUT__

// Latex documentation
#ifdef DOCSTRING_Loop
static const char *docstringLoopCommandOutput = R"(
\subsection{CommandOutput}\label{loopType:commandOutput}
Loop over lines of command output.
)";
#endif

/***********************************************/

#include "base/import.h"
#include "parallel/parallel.h"
#include "inputOutput/logging.h"
#include "inputOutput/system.h"
#include "classes/loop/loop.h"

/***** CLASS ***********************************/

/** @brief Loop over lines of command output.
* @ingroup LoopGroup
* @see Loop */
class LoopCommandOutput : public Loop
{
  std::string              nameString, nameIndex, nameCount;
  std::vector<std::string> strings;
  UInt                     index;

public:
  LoopCommandOutput(Config &config);

  UInt count() const override {return strings.size();}
  Bool iteration(VariableList &varList) override;
};

/***********************************************/
/***** Inlines *********************************/
/***********************************************/

inline LoopCommandOutput::LoopCommandOutput(Config &config)
{
  try
  {
    std::vector<FileName> command;
    Bool                  silently;

    readConfig(config, "command",            command,    Config::MUSTSET,   "",            "each output line becomes a loop iteration");
    readConfig(config, "silently",           silently,   Config::DEFAULT,   "0",           "without showing the output.");
    readConfig(config, "variableLoopString", nameString, Config::OPTIONAL, "loopCommand", "name of the variable to be replaced");
    readConfig(config, "variableLoopIndex",  nameIndex,  Config::OPTIONAL, "",            "variable with index of current iteration (starts with zero)");
    readConfig(config, "variableLoopCount",  nameCount,  Config::OPTIONAL, "",            "variable with total number of iterations");
    if(isCreateSchema(config)) return;

    if(Parallel::isMaster())
    {
      UInt count = 0;
      for(UInt i=0; i<command.size(); i++)
      {
        if(!System::exec(command.at(i), strings))
          throw(Exception("Command \""+command.at(i).str()+"\" exited with error"));
        if(!silently)
          for(const auto &str : strings)
            logInfo<<str<<Log::endl;
        if(strings.size() == count)
          logWarning<<"Command \""+command.at(i).str()+"\" returned no output"<<Log::endl;
        count = strings.size();
      } // for(i)
    }
    Parallel::broadCast(strings);

    index = 0;
  }
  catch(std::exception &e)
  {
    GROOPS_RETHROW(e)
  }
}

/***********************************************/

inline Bool LoopCommandOutput::iteration(VariableList &varList)
{
  if(index >= count())
    return FALSE;

  if(!nameString.empty()) addVariable(nameString, strings.at(index), varList);
  if(!nameIndex.empty())  addVariable(nameIndex,  index,             varList);
  if(!nameCount.empty())  addVariable(nameCount,  count(),           varList);

  index++;
  return TRUE;
}

/***********************************************/

#endif

#pragma once

#include <memory>
#include <vector>

namespace opensn
{
class FieldFunctionGridBased;

enum class FieldFunctionInterpolationType : int
{
  SLICE = 1,
  LINE = 2,
  VOLUME = 3,
  POINT = 4
};

enum class FieldFunctionInterpolationOperation : int
{
  OP_SUM = 10,
  OP_AVG = 11,
  OP_MAX = 12,
  OP_SUM_FUNC = 13,
  OP_AVG_FUNC = 14,
  OP_MAX_FUNC = 15,
};

enum class FieldFunctionInterpolationProperty : int
{
  PROBEPOINT = 0,
  SLICEPOINT = 1,
  SLICENORMAL = 2,
  SLICETANGENT = 3,
  SLICEBINORM = 4,
  OPERATION = 5,
  LOGICAL_VOLUME = 8,

  ADD_FIELD_FUNCTION = 9,
  SET_FIELD_FUNCTIONS = 10,

  FIRSTPOINT = 11,
  SECONDPOINT = 12,
  NUMBEROFPOINTS = 13,
  CUSTOM_ARRAY = 14,
};

// ###################################################################
/** Base class for field-function interpolation objects.*/
class FieldFunctionInterpolation
{
protected:
  FieldFunctionInterpolationType type_;
  unsigned int ref_component_ = 0;
  std::vector<std::shared_ptr<FieldFunctionGridBased>> field_functions_;

public:
  explicit FieldFunctionInterpolation(FieldFunctionInterpolationType type) : type_(type) {}

  std::vector<std::shared_ptr<FieldFunctionGridBased>>& GetFieldFunctions()
  {
    return field_functions_;
  }

  FieldFunctionInterpolationType Type() const { return type_; }

  /**Initializes the point interpolator.*/
  virtual void Initialize(){};
  /**Executes the point interpolator.*/
  virtual void Execute(){};

  virtual std::string GetDefaultFileBaseName() const = 0;
  virtual void ExportPython(std::string base_name) = 0;
};

} // namespace opensn

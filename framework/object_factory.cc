#include "framework/object_factory.h"
#include "framework/runtime.h"
#include "framework/logging/log.h"

namespace opensn
{

ObjectFactory&
ObjectFactory::GetInstance() noexcept
{
  static ObjectFactory singleton;
  return singleton;
}

const std::map<std::string, ObjectFactory::ObjectRegistryEntry>&
ObjectFactory::Registry() const
{
  return object_registry_;
}

bool
ObjectFactory::RegistryHasKey(const std::string& key) const
{
  return object_registry_.count(key) > 0;
}

size_t
ObjectFactory::MakeRegisteredObject(const ParameterBlock& params) const
{
  if (Chi::log.GetVerbosity() >= 2) Chi::log.Log() << "Making object with type from parameters";

  const std::string fname = __PRETTY_FUNCTION__;

  if (not params.Has("chi_obj_type"))
    throw std::invalid_argument(fname +
                                ": Requires a parameter block with a field called "
                                "\"chi_obj_type\". The given parameter block does not seem to "
                                "have this parameter.");

  const auto type = params.GetParamValue<std::string>("chi_obj_type");

  return MakeRegisteredObjectOfType(type, params);
}

size_t
ObjectFactory::MakeRegisteredObjectOfType(const std::string& type,
                                          const ParameterBlock& params) const
{
  if (Chi::log.GetVerbosity() >= 2) Chi::log.Log() << "Making object with specified type";

  const std::string fname = __PRETTY_FUNCTION__;

  if (object_registry_.count(type) == 0)
    throw std::logic_error(fname + ": No registered type \"" + type + "\" found.");

  if (Chi::log.GetVerbosity() >= 2) Chi::log.Log() << "Making object type " << type;

  auto object_entry = object_registry_.at(type);

  ChiLogicalErrorIf(not object_entry.constructor_func,
                    "Object is not constructable since it has no registered "
                    "constructor");

  auto input_params = object_entry.get_in_params_func();

  input_params.SetObjectType(type);
  input_params.SetErrorOriginScope(type);

  if (Chi::log.GetVerbosity() >= 2) Chi::log.Log() << "Assigning parameters for object " << type;

  input_params.AssignParameters(params);

  if (Chi::log.GetVerbosity() >= 2) Chi::log.Log() << "Constructing object " << type;

  auto new_object = object_entry.constructor_func(input_params);

  new_object->PushOntoStack(new_object);

  if (Chi::log.GetVerbosity() >= 2)
    Chi::log.Log() << "Done making object type " << type << " with handle "
                   << new_object->StackID();

  return new_object->StackID();
}

InputParameters
ObjectFactory::GetRegisteredObjectParameters(const std::string& type) const
{
  auto iter = object_registry_.find(type);
  ChiInvalidArgumentIf(iter == object_registry_.end(),
                       "Object type \"" + type + "\" is not registered in ObjectFactory.");

  auto& reg_entry = iter->second;

  return reg_entry.get_in_params_func();
}

void
ObjectFactory::DumpRegister() const
{
  Chi::log.Log() << "\n\n";
  for (const auto& [key, entry] : object_registry_)
  {
    if (Chi::log.GetVerbosity() == 0)
    {
      Chi::log.Log() << key;
      continue;
    }

    Chi::log.Log() << "OBJECT_BEGIN " << key;

    if (entry.constructor_func == nullptr) Chi::log.Log() << "NOT_CONSTRUCTIBLE";

    const auto in_params = entry.get_in_params_func();
    in_params.DumpParameters();

    Chi::log.Log() << "OBJECT_END\n\n";
  }
  Chi::log.Log() << "\n\n";
}

void
ObjectFactory::AssertRegistryKeyAvailable(const std::string& key,
                                          const std::string& calling_function) const
{
  if (RegistryHasKey(key))
    ChiLogicalError(calling_function + ": Attempted to register Object \"" + key +
                    "\" but an object with the same name is already registered.");
}

} // namespace opensn

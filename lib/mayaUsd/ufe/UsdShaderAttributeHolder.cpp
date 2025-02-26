//
// Copyright 2022 Autodesk
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "UsdShaderAttributeHolder.h"

#include "UsdShaderAttributeDef.h"
#include "Utils.h"

#include <mayaUsd/utils/util.h>

#ifdef UFE_V3_FEATURES_AVAILABLE
#include <mayaUsd/base/tokens.h>
#endif

#include <pxr/base/tf/diagnostic.h>
#include <pxr/base/tf/token.h>
#include <pxr/base/vt/value.h>
#include <pxr/pxr.h>

#include <ufe/log.h>

namespace MAYAUSD_NS_DEF {
namespace ufe {

//------------------------------------------------------------------------------
// UsdShaderAttributeHolder:
//------------------------------------------------------------------------------

UsdShaderAttributeHolder::UsdShaderAttributeHolder(
    PXR_NS::UsdPrim                   usdPrim,
    PXR_NS::SdrShaderPropertyConstPtr sdrProp,
    PXR_NS::UsdShadeAttributeType     sdrType)
    : UsdAttributeHolder(
        usdPrim.GetAttribute(PXR_NS::UsdShadeUtils::GetFullName(sdrProp->GetName(), sdrType)))
    , _sdrProp(sdrProp)
    , _sdrType(sdrType)
{
    // The _usdAttr in the base class will be in an invalid state if the attribute was never
    // created, but it will still remember correctly the prim used to initialize it.

    // sdrProp must be valid at creation and will stay valid.
    PXR_NAMESPACE_USING_DIRECTIVE
    if (!TF_VERIFY(sdrProp && sdrType != PXR_NS::UsdShadeAttributeType::Invalid)) {
        throw std::runtime_error("Invalid shader attribute holder");
    }
}

UsdAttributeHolder::UPtr UsdShaderAttributeHolder::create(
    PXR_NS::UsdPrim                   usdPrim,
    PXR_NS::SdrShaderPropertyConstPtr sdrProp,
    PXR_NS::UsdShadeAttributeType     sdrType)
{
    return std::unique_ptr<UsdShaderAttributeHolder>(
        new UsdShaderAttributeHolder(usdPrim, sdrProp, sdrType));
}
std::string UsdShaderAttributeHolder::isEditAllowedMsg() const
{
    if (_Base::isValid()) {
        return _Base::isEditAllowedMsg();
    } else if (usdPrim()) {
        return std::string();
    } else {
        return "Editing is not allowed.";
    }
}

std::string UsdShaderAttributeHolder::defaultValue() const
{
    // TODO: Add a PXR_VERSION if a fix is introduced in OpenUSD.
    if (_sdrProp->GetType() == PXR_NS::SdfValueTypeNames->Matrix3d.GetAsToken()) {
        const std::string val = UsdShaderAttributeDef(_sdrProp).defaultValue();
        if (val.empty()) {
            // There is no Matrix3d type in Sdr, so the MaterialX default value is not kept
            return "0,0,0,0,0,0,0,0,0";
        }
        // But if https://github.com/PixarAnimationStudios/OpenUSD/issues/2523 gets fixed
        // then return that value:
        return val;
    }
#if PXR_VERSION < 2205
    if (_sdrProp->GetType() == PXR_NS::SdfValueTypeNames->Bool.GetAsToken()) {
        // Pre-22.05 there was no Boolean type in Sdr, so no default value
        return "false";
    }
#endif
    return UsdShaderAttributeDef(_sdrProp).defaultValue();
}

std::string UsdShaderAttributeHolder::nativeType() const { return _sdrProp->GetType(); }

//! A List of {info:id|property_name} of the properties that lack the default value, to avoid
//! repeating the warnings on console.
static std::unordered_set<std::string> s_ListPropertiesWithoutDefaultValue {};

bool UsdShaderAttributeHolder::get(PXR_NS::VtValue& value, PXR_NS::UsdTimeCode time) const
{
    if (isAuthored()) {
        return _Base::get(value, time);
    }
    // No prim check is required as we can get the value from the attribute definition
    value = vtValueFromString(usdAttributeType(), defaultValue());

    if (defaultValue().empty()) {
        PXR_NS::VtValue infoIdVariant;
        usdPrim().GetAttribute(PXR_NS::TfToken("info:id")).Get(&infoIdVariant);
        const auto shaderInfoId = infoIdVariant.Get<PXR_NS::TfToken>().GetString();
        const auto propertyName = _sdrProp->GetName().GetString();
        const auto listElement = shaderInfoId + "/" + propertyName;
        const auto warningAlreadySpawn = s_ListPropertiesWithoutDefaultValue.find(listElement)
            != s_ListPropertiesWithoutDefaultValue.end();
        if (!warningAlreadySpawn) {
            s_ListPropertiesWithoutDefaultValue.insert(listElement);
            const std::string msg = "Warning: Shader property '" + propertyName
                + "' does not have a default value. (Shader info:id: " + shaderInfoId + ").";
            UFE_LOG(msg.c_str());
        }
    }

    return !value.IsEmpty();
}

bool UsdShaderAttributeHolder::set(const PXR_NS::VtValue& value, PXR_NS::UsdTimeCode time)
{
    if (!isValid()) {
        PXR_NS::VtValue currentValue;
        get(currentValue, time);
        if (currentValue == value) {
            return true;
        } else if (usdPrim()) {
            _CreateUsdAttribute();
        } else {
            return false;
        }
    }

    return _Base::set(value, time);
}

bool UsdShaderAttributeHolder::hasValue() const
{
    return _Base::hasValue() || !defaultValue().empty();
}

std::string UsdShaderAttributeHolder::name() const
{
    return PXR_NS::UsdShadeUtils::GetFullName(_sdrProp->GetName(), _sdrType);
}

std::string UsdShaderAttributeHolder::displayName() const
{
    Ufe::Value retVal
        = UsdShaderAttributeDef(_sdrProp).getMetadata(PXR_NS::MayaUsdMetadata->UIName);
    std::string name = retVal.safeGet<std::string>({});
    if (!name.empty()) {
        return name;
    }
    return _Base::displayName();
}

std::string UsdShaderAttributeHolder::documentation() const { return _sdrProp->GetHelp(); }

#ifdef UFE_V3_FEATURES_AVAILABLE
Ufe::Value UsdShaderAttributeHolder::getMetadata(const std::string& key) const
{
    Ufe::Value retVal;
    if (key == PXR_NS::MayaUsdMetadata->UIName) {
        retVal = UsdShaderAttributeDef(_sdrProp).getMetadata(key);
        if (!retVal.empty()) {
            return retVal;
        }
    }
    retVal = _Base::getMetadata(key);
    if (retVal.empty()) {
        return UsdShaderAttributeDef(_sdrProp).getMetadata(key);
    }
    return retVal;
}

bool UsdShaderAttributeHolder::setMetadata(const std::string& key, const Ufe::Value& value)
{
    if (!isValid() && usdPrim()) {
        _CreateUsdAttribute();
    }

    return _Base::setMetadata(key, value);
}

bool UsdShaderAttributeHolder::hasMetadata(const std::string& key) const
{
    bool retVal = _Base::hasMetadata(key);
    if (!retVal) {
        retVal = UsdShaderAttributeDef(_sdrProp).hasMetadata(key);
    }
    return retVal;
}
#endif

PXR_NS::SdfValueTypeName UsdShaderAttributeHolder::usdAttributeType() const
{
    if (_sdrProp->GetType() == PXR_NS::SdfValueTypeNames->Matrix3d.GetAsToken()) {
        // There is no Matrix3d type in Sdr
        return PXR_NS::SdfValueTypeNames->Matrix3d;
    }
#if PXR_VERSION < 2205
    if (_sdrProp->GetType() == PXR_NS::SdfValueTypeNames->Bool.GetAsToken()) {
        // Pre-22.05 there was no Boolean type in Sdr
        return PXR_NS::SdfValueTypeNames->Bool;
    }
#endif
    return _sdrProp->GetTypeAsSdfType().first;
}

Ufe::AttributeEnumString::EnumValues UsdShaderAttributeHolder::getEnumValues() const
{
    Ufe::AttributeEnumString::EnumValues retVal;
    for (auto const& option : getEnums()) {
        retVal.push_back(option.first);
    }
    return retVal;
}

UsdAttributeHolder::EnumOptions UsdShaderAttributeHolder::getEnums() const
{
    auto retVal = _Base::getEnums();
    for (auto const& option : _sdrProp->GetOptions()) {
        retVal.emplace_back(option.first, option.second);
    }
    return retVal;
}

void UsdShaderAttributeHolder::_CreateUsdAttribute()
{
    PXR_NS::UsdShadeShader shader(usdPrim());
    if (_sdrType == PXR_NS::UsdShadeAttributeType::Output) {
        _usdAttr = shader.CreateOutput(_sdrProp->GetName(), usdAttributeType()).GetAttr();
    } else {
        _usdAttr = shader.CreateInput(_sdrProp->GetName(), usdAttributeType()).GetAttr();
    }
}

} // namespace ufe
} // namespace MAYAUSD_NS_DEF

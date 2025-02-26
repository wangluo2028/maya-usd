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
#ifndef MAYAUSD_LAYERMUTING_H
#define MAYAUSD_LAYERMUTING_H

#include <mayaUsd/base/api.h>
#include <mayaUsd/nodes/proxyShapeBase.h>

#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/usd/stage.h>

namespace MAYAUSD_NS_DEF {

// The muted state of a layer is stage-level data. As such, it is not saved
// within the layer (i.e. in the USD files that have been staged.) The reason
// behind this is that two stages could have different muted layers, a single
// layer could be muted in one stage and not muted in another stage. So, the
// muted state cannot be a layer-level data.
//
// Furthermore, stages in USD are not saved but are a pure run-time entity,
// part of the hosting application. It is thus the host responsibility to save
// stage-level state. So, we need to explicitly save the layer muted state.
//
// We thus saved the muted state of layers in the proxy shape as an attribute.

/*! \brief copy the stage layers muting in the corresponding attribute of the proxy shape.
 */
MAYAUSD_CORE_PUBLIC
MStatus
copyLayerMutingToAttribute(const PXR_NS::UsdStage& stage, MayaUsdProxyShapeBase& proxyShape);

/*! \brief set the stage layers muting from data in the corresponding attribute of the proxy shape.
 */
MAYAUSD_CORE_PUBLIC
MStatus
copyLayerMutingFromAttribute(const MayaUsdProxyShapeBase& proxyShape, PXR_NS::UsdStage& stage);

// OpenUSD forget everything about muted layers. The OpenUSD documentation for
// the MuteLayer function says:
//
//    Note that muting a layer will cause this stage to release all references
//    to that layer. If no other client is holding on to references to that
//    layer, it will be unloaded.In this case, if there are unsaved edits to
//    the muted layer, those edits are lost.
//
//    Since anonymous layers are not serialized, muting an anonymous layer will
//    cause that layer and its contents to be lost in this case.
//
// So we need to hold on to muted layers. We do this in a private global list
// of muted layers. That list gets cleared when a new Maya scene is created.

MAYAUSD_CORE_PUBLIC
void addMutedLayer(const PXR_NS::SdfLayerRefPtr& layer);

MAYAUSD_CORE_PUBLIC
void removeMutedLayer(const PXR_NS::SdfLayerRefPtr& layer);

MAYAUSD_CORE_PUBLIC
void forgetMutedLayers();

} // namespace MAYAUSD_NS_DEF

#endif

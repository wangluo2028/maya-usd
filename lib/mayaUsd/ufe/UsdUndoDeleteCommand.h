//
// Copyright 2019 Autodesk
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
#pragma once

#include <mayaUsd/base/api.h>

#include <usdUfe/ufe/UfeVersionCompat.h>
#include <usdUfe/ufe/UsdSceneItem.h>
#include <usdUfe/undo/UsdUndoableItem.h>

#include <pxr/usd/usd/prim.h>

#include <ufe/undoableCommand.h>

namespace MAYAUSD_NS_DEF {
namespace ufe {

//! \brief UsdUndoDeleteCommand
class MAYAUSD_CORE_PUBLIC UsdUndoDeleteCommand : public Ufe::UndoableCommand
{
public:
    typedef std::shared_ptr<UsdUndoDeleteCommand> Ptr;

    UsdUndoDeleteCommand(const PXR_NS::UsdPrim& prim);
    ~UsdUndoDeleteCommand() override;

    // Delete the copy/move constructors assignment operators.
    UsdUndoDeleteCommand(const UsdUndoDeleteCommand&) = delete;
    UsdUndoDeleteCommand& operator=(const UsdUndoDeleteCommand&) = delete;
    UsdUndoDeleteCommand(UsdUndoDeleteCommand&&) = delete;
    UsdUndoDeleteCommand& operator=(UsdUndoDeleteCommand&&) = delete;

    //! Create a UsdUndoDeleteCommand from a USD prim.
    static UsdUndoDeleteCommand::Ptr create(const PXR_NS::UsdPrim& prim);

    void execute() override;
    void undo() override;
    void redo() override;

private:
    PXR_NS::UsdPrim _prim;
    UsdUndoableItem _undoableItem;

}; // UsdUndoDeleteCommand

} // namespace ufe
} // namespace MAYAUSD_NS_DEF

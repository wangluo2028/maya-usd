//
// Copyright 2016 Pixar
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
#ifndef PXRUSDMAYA_PROXY_SHAPE_BASE_H
#define PXRUSDMAYA_PROXY_SHAPE_BASE_H

#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/tf/staticTokens.h>
#include <pxr/pxr.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/notice.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/timeCode.h>

#include <maya/MBoundingBox.h>
#include <maya/MDGContext.h>
#include <maya/MDagPath.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MPxSurfaceShape.h>
#include <maya/MSelectionMask.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <ufe/ufe.h>

#include <map>

UFE_NS_DEF { class Path; }

#include <mayaUsd/base/api.h>
#include <mayaUsd/listeners/stageNoticeListener.h>
#include <mayaUsd/nodes/proxyAccessor.h>
#include <mayaUsd/nodes/proxyStageProvider.h>
#include <mayaUsd/nodes/usdPrimProvider.h>

PXR_NAMESPACE_OPEN_SCOPE

// clang-format off
#define MAYAUSD_PROXY_SHAPE_BASE_TOKENS \
    ((MayaTypeName, "mayaUsdProxyShapeBase"))
// clang-format on

TF_DECLARE_PUBLIC_TOKENS(
    MayaUsdProxyShapeBaseTokens,
    MAYAUSD_CORE_PUBLIC,
    MAYAUSD_PROXY_SHAPE_BASE_TOKENS);

class MayaUsdProxyShapeBase
    : public MPxSurfaceShape
    , public ProxyStageProvider
    , public UsdMayaUsdPrimProvider
{

public:
    typedef MayaUsdProxyShapeBase ThisClass;

    MAYAUSD_CORE_PUBLIC
    static const MTypeId typeId;
    MAYAUSD_CORE_PUBLIC
    static const MString typeName;

    MAYAUSD_CORE_PUBLIC
    static const MString displayFilterName;
    MAYAUSD_CORE_PUBLIC
    static const MString displayFilterLabel;

    // Attributes
    MAYAUSD_CORE_PUBLIC
    static MObject filePathAttr;
    MAYAUSD_CORE_PUBLIC
    static MObject filePathRelativeAttr;
    MAYAUSD_CORE_PUBLIC
    static MObject primPathAttr;
    MAYAUSD_CORE_PUBLIC
    static MObject excludePrimPathsAttr;
    MAYAUSD_CORE_PUBLIC
    static MObject loadPayloadsAttr;
    MAYAUSD_CORE_PUBLIC
    static MObject shareStageAttr;
    MAYAUSD_CORE_PUBLIC
    static MObject timeAttr;
    MAYAUSD_CORE_PUBLIC
    static MObject complexityAttr;
    MAYAUSD_CORE_PUBLIC
    static MObject inStageDataAttr;
    MAYAUSD_CORE_PUBLIC
    static MObject inStageDataCachedAttr;
    MAYAUSD_CORE_PUBLIC
    static MObject stageCacheIdAttr;
    MAYAUSD_CORE_PUBLIC
    static MObject drawRenderPurposeAttr;
    MAYAUSD_CORE_PUBLIC
    static MObject drawProxyPurposeAttr;
    MAYAUSD_CORE_PUBLIC
    static MObject drawGuidePurposeAttr;

    MAYAUSD_CORE_PUBLIC
    static MObject sessionLayerNameAttr;
    MAYAUSD_CORE_PUBLIC
    static MObject rootLayerNameAttr;
    MAYAUSD_CORE_PUBLIC
    static MObject mutedLayersAttr;

    // Change counter attributes
    MAYAUSD_CORE_PUBLIC
    static MObject updateCounterAttr;
    MAYAUSD_CORE_PUBLIC
    static MObject resyncCounterAttr;

    // Output attributes
    MAYAUSD_CORE_PUBLIC
    static MObject outTimeAttr;
    MAYAUSD_CORE_PUBLIC
    static MObject outStageDataAttr;
    MAYAUSD_CORE_PUBLIC
    static MObject outStageCacheIdAttr;

    /// Delegate function for computing the closest point and surface normal
    /// on the proxy shape to a given ray.
    /// The input ray, output point, and output normal should be in the
    /// proxy shape's local space.
    /// Should return true if a point was found, and false otherwise.
    /// (You could just treat this as a ray intersection and return true
    /// if intersected, false if missed.)
    typedef std::function<bool(const MayaUsdProxyShapeBase&, const GfRay&, GfVec3d*, GfVec3d*)>
        ClosestPointDelegate;

    MAYAUSD_CORE_PUBLIC
    static void* creator();

    MAYAUSD_CORE_PUBLIC
    static MStatus initialize();

    MAYAUSD_CORE_PUBLIC
    static MayaUsdProxyShapeBase* GetShapeAtDagPath(const MDagPath& dagPath);

    MAYAUSD_CORE_PUBLIC
    static int countProxyShapeInstances();

    MAYAUSD_CORE_PUBLIC
    static void SetClosestPointDelegate(ClosestPointDelegate delegate);

    // UsdMayaUsdPrimProvider overrides:
    /**
     * accessor to get the usdprim
     *
     * This method pulls the usdstage data from outData, and will evaluate
     * the dependencies necessary to do so. It should be called instead of
     * pulling on the data directly.
     */
    MAYAUSD_CORE_PUBLIC
    UsdPrim usdPrim() const override;

    // Virtual function overrides

    MAYAUSD_CORE_PUBLIC
    void postConstructor() override;
    MAYAUSD_CORE_PUBLIC
    bool getInternalValue(const MPlug&, MDataHandle&) override;
    MAYAUSD_CORE_PUBLIC
    MStatus compute(const MPlug& plug, MDataBlock& dataBlock) override;
    MAYAUSD_CORE_PUBLIC
    bool isBounded() const override;
    MAYAUSD_CORE_PUBLIC
    MBoundingBox boundingBox() const override;
    MAYAUSD_CORE_PUBLIC
    MSelectionMask getShapeSelectionMask() const override;

    MAYAUSD_CORE_PUBLIC
    bool closestPoint(
        const MPoint&  raySource,
        const MVector& rayDirection,
        MPoint&        theClosestPoint,
        MVector&       theClosestNormal,
        bool           findClosestOnMiss,
        double         tolerance) override;

    MAYAUSD_CORE_PUBLIC
    bool canMakeLive() const override;

    // Public functions
    MAYAUSD_CORE_PUBLIC
    virtual SdfPathVector getExcludePrimPaths() const;
    MAYAUSD_CORE_PUBLIC
    size_t getExcludePrimPathsVersion() const;

    MAYAUSD_CORE_PUBLIC
    int getComplexity() const;

    MAYAUSD_CORE_PUBLIC
    std::vector<std::string> getMutedLayers() const;

    MAYAUSD_CORE_PUBLIC
    MStatus setMutedLayers(const std::vector<std::string>& muted);

    MAYAUSD_CORE_PUBLIC
    UsdTimeCode getTime() const override;
    MAYAUSD_CORE_PUBLIC
    UsdStageRefPtr getUsdStage() const override;
    MAYAUSD_CORE_PUBLIC
    size_t getUsdStageVersion() const;
    MAYAUSD_CORE_PUBLIC
    void getDrawPurposeToggles(
        bool* drawRenderPurpose,
        bool* drawProxyPurpose,
        bool* drawGuidePurpose) const;

    MAYAUSD_CORE_PUBLIC
    bool GetAllRenderAttributes(
        UsdPrim*       usdPrimOut,
        SdfPathVector* excludePrimPathsOut,
        int*           complexityOut,
        UsdTimeCode*   timeOut,
        bool*          drawRenderPurpose,
        bool*          drawProxyPurpose,
        bool*          drawGuidePurpose);

    MAYAUSD_CORE_PUBLIC
    MStatus
    preEvaluation(const MDGContext& context, const MEvaluationNode& evaluationNode) override;

    MAYAUSD_CORE_PUBLIC
    MStatus postEvaluation(
        const MDGContext&      context,
        const MEvaluationNode& evaluationNode,
        PostEvaluationType     evalType) override;

    MAYAUSD_CORE_PUBLIC
    void getCacheSetup(
        const MEvaluationNode&   evalNode,
        MNodeCacheDisablingInfo& disablingInfo,
        MNodeCacheSetupInfo&     cacheSetupInfo,
        MObjectArray&            monitoredAttributes) const override;

    MAYAUSD_CORE_PUBLIC
    void configCache(const MEvaluationNode& evalNode, MCacheSchema& schema) const override;

    MAYAUSD_CORE_PUBLIC
    MStatus setDependentsDirty(const MPlug& plug, MPlugArray& plugArray) override;

    /// \brief  Clears the bounding box cache of the shape
    MAYAUSD_CORE_PUBLIC
    void clearBoundingBoxCache();

    // returns the shape's parent transform
    MAYAUSD_CORE_PUBLIC
    MDagPath parentTransform();

    // Is this required if there is parentTransform?
    MAYAUSD_CORE_PUBLIC
    Ufe::Path ufePath() const;

    /// Returns whether the proxy shape allows subpaths within its
    /// hierarchy to be selected independently when using the Viewport 2.0
    /// render delegate.
    ///
    /// UFE/subpath selection must be enabled or disabled when constructing
    /// the proxy shape. This is primarily intended as a mechanism for
    /// UsdMayaProxyShape to disable UFE/subpath selection. Most of the
    /// usage of pxrUsdProxyShape nodes is when they are brought in by
    /// activating the "Collapsed" representation of
    /// pxrUsdReferenceAssembly nodes. In that case, they are intended to
    /// be read-only proxies, and any edits to prims within the hierarchy
    /// should be represented as assembly edits.
    bool isUfeSelectionEnabled() const { return _isUfeSelectionEnabled; }

    MAYAUSD_CORE_PUBLIC
    bool isShareableStage() const;

    MAYAUSD_CORE_PUBLIC
    bool isStageIncoming() const;

    MAYAUSD_CORE_PUBLIC
    bool isIncomingLayer(const std::string& layerIdentifier) const;

    MAYAUSD_CORE_PUBLIC
    void onAncestorPlugDirty(MPlug& plug);

protected:
    MAYAUSD_CORE_PUBLIC
    MayaUsdProxyShapeBase(
        const bool enableUfeSelection = true,
        const bool useLoadRulesHandling = true);

    MAYAUSD_CORE_PUBLIC
    ~MayaUsdProxyShapeBase() override;

    MAYAUSD_CORE_PUBLIC
    bool isStageValid() const;

    //! \brief  Create and register proxy accessor on this proxy. Should be called from
    //! postConstructor.
    MAYAUSD_CORE_PUBLIC
    void enableProxyAccessor();

    // Hook method for derived classes.  This class returns a nullptr.
    MAYAUSD_CORE_PUBLIC
    virtual SdfLayerRefPtr computeSessionLayer(MDataBlock&);

    // Hook method for derived classes.  This class returns a nullptr.
    MAYAUSD_CORE_PUBLIC
    virtual SdfLayerRefPtr computeRootLayer(MDataBlock&, const std::string&);

    // Hook method for derived classes: can this object be soft selected?
    // This class returns false.
    MAYAUSD_CORE_PUBLIC
    virtual bool canBeSoftSelected() const;

    // Hook method for derived classes: is soft select enabled?  This
    // class returns false.
    MAYAUSD_CORE_PUBLIC
    virtual bool GetObjectSoftSelectEnabled() const;

    MAYAUSD_CORE_PUBLIC
    UsdPrim _GetUsdPrim(MDataBlock dataBlock) const;

    // Hook method for derived classes: cache an empty computed bounding
    // box.  This class does nothing.
    MAYAUSD_CORE_PUBLIC
    virtual void CacheEmptyBoundingBox(MBoundingBox&);

    // Return the output time.  This class returns the value of the
    // input time attribute.
    MAYAUSD_CORE_PUBLIC
    virtual UsdTimeCode GetOutputTime(MDataBlock) const;

    MAYAUSD_CORE_PUBLIC
    void _IncreaseExcludePrimPathsVersion() { _excludePrimPathsVersion++; }

    MAYAUSD_CORE_PUBLIC
    void _IncreaseUsdStageVersion() { _UsdStageVersion++; }

    MAYAUSD_CORE_PUBLIC
    void copyInternalData(MPxNode* srcNode) override;

private:
    // The possible the shared mode of the stage.
    // The 'Unknown' mode is when the proxy shape is created and has not yet been computed.
    enum class ShareMode
    {
        Unknown,
        Shared,
        Unshared
    };

    MayaUsdProxyShapeBase(const MayaUsdProxyShapeBase&);
    MayaUsdProxyShapeBase& operator=(const MayaUsdProxyShapeBase&);

    MStatus computeOutputTime(MDataBlock& dataBlock);
    MStatus computeInStageDataCached(MDataBlock& dataBlock);
    MStatus computeOutStageData(MDataBlock& dataBlock);
    MStatus computeOutStageCacheId(MDataBlock& dataBlock);

    void clearAncestorCallbacks();
    void updateAncestorCallbacks();

    void updateShareMode(
        const UsdStageRefPtr&    sharedUsdStage,
        const UsdStageRefPtr&    unsharedUsdStage,
        UsdStage::InitialLoadSet loadSet);

    void transferSessionLayer(
        ShareMode                currentMode,
        const UsdStageRefPtr&    sharedUsdStage,
        const UsdStageRefPtr&    unsharedUsdStage,
        UsdStage::InitialLoadSet loadSet);

    UsdStageRefPtr getUnsharedStage(UsdStage::InitialLoadSet loadSet);

    SdfPathVector _GetExcludePrimPaths(MDataBlock dataBlock) const;
    int           _GetComplexity(MDataBlock dataBlock) const;
    UsdTimeCode   _GetTime(MDataBlock dataBlock) const;

    bool _GetDrawPurposeToggles(
        MDataBlock dataBlock,
        bool*      drawRenderPurpose,
        bool*      drawProxyPurpose,
        bool*      drawGuidePurpose) const;

    void _OnStageContentsChanged(const UsdNotice::StageContentsChanged& notice);
    void _OnStageObjectsChanged(const UsdNotice::ObjectsChanged& notice);
    void _OnLayerMutingChanged(const UsdNotice::LayerMutingChanged& notice);
    void _OnStageEditTargetChanged(const UsdNotice::StageEditTargetChanged& notice);

    static void renameCallback(MObject& node, const MString& str, void* clientData);

    UsdMayaStageNoticeListener _stageNoticeListener;

    std::map<UsdTimeCode, MBoundingBox> _boundingBoxCache;
    size_t                              _excludePrimPathsVersion { 1 };
    size_t                              _UsdStageVersion { 1 };

    // Notification counters:
    MInt64 _UsdStageUpdateCounter { 1 };
    MInt64 _UsdStageResyncCounter { 1 };

    MayaUsd::ProxyAccessor::Owner _usdAccessor;

    static ClosestPointDelegate _sharedClosestPointDelegate;

    // Whether or not the proxy shape has enabled UFE/subpath selection
    const bool _isUfeSelectionEnabled;

    // Track the shared mode of the stage as seen in the last compute.
    // Starts off as Unknown when the proxy shape is first created.
    ShareMode _previousShareMode { ShareMode::Unknown };

    // Anonymous layer that was created when a new proxy shape is created without a named layer.
    SdfLayerRefPtr _anonymousRootLayer;

    // For unshared composition
    SdfLayerRefPtr _unsharedStageSessionLayer;
    SdfLayerRefPtr _unsharedStageRootLayer;

    // Current edit target for the stage. Kept in a dynamic attribute for save/load,
    // transferred to this variable on the first compute. Afterward, when the edit
    // target is changed, this gets updated via a notification listener.
    SdfLayerRefPtr _targetLayer;

    // We need to keep track of unshared sublayers (otherwise they get removed)
    std::vector<SdfLayerRefPtr> _unsharedStageRootSublayers;

    // Keep track of the incoming layers
    std::set<std::string> _incomingLayers;

    // Callbacks for listening to ancestor dirty messages.
    // That includes the proxy shape itself.
    std::vector<MCallbackId> _ancestorCallbacks;
    MString                  _ancestorCallbacksPath;
    bool                     _inAncestorCallback { false };

    MCallbackId _preSaveCallbackId { 0 };
    MCallbackId _renameCallbackId { 0 };

public:
    // Counter for the number of times compute is re-entered
    static std::atomic<int> in_compute;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif

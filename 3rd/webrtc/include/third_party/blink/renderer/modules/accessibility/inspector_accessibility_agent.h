// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_ACCESSIBILITY_INSPECTOR_ACCESSIBILITY_AGENT_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_ACCESSIBILITY_INSPECTOR_ACCESSIBILITY_AGENT_H_

#include "base/macros.h"
#include "third_party/blink/renderer/core/accessibility/ax_context.h"
#include "third_party/blink/renderer/core/inspector/inspector_base_agent.h"
#include "third_party/blink/renderer/core/inspector/protocol/Accessibility.h"
#include "third_party/blink/renderer/modules/modules_export.h"

namespace blink {

class AXObject;
class AXObjectCacheImpl;
class InspectorDOMAgent;
class InspectedFrames;
class LocalFrame;

using protocol::Accessibility::AXNode;
using protocol::Accessibility::AXNodeId;

class MODULES_EXPORT InspectorAccessibilityAgent
    : public InspectorBaseAgent<protocol::Accessibility::Metainfo> {
 public:
  InspectorAccessibilityAgent(InspectedFrames*, InspectorDOMAgent*);

  static void ProvideTo(LocalFrame* frame);

  // Base agent methods.
  void Trace(Visitor*) const override;
  void Restore() override;

  // Protocol methods.
  protocol::Response enable() override;
  protocol::Response disable() override;
  protocol::Response getPartialAXTree(
      protocol::Maybe<int> dom_node_id,
      protocol::Maybe<int> backend_node_id,
      protocol::Maybe<String> object_id,
      protocol::Maybe<bool> fetch_relatives,
      std::unique_ptr<protocol::Array<protocol::Accessibility::AXNode>>*)
      override;
  protocol::Response getFullAXTree(
      protocol::Maybe<int> depth,
      protocol::Maybe<int> max_depth,
      protocol::Maybe<String> frame_id,
      std::unique_ptr<protocol::Array<protocol::Accessibility::AXNode>>*)
      override;
  protocol::Response getChildAXNodes(
      const String& in_id,
      protocol::Maybe<String> frame_id,
      std::unique_ptr<protocol::Array<protocol::Accessibility::AXNode>>*
          out_nodes) override;
  protocol::Response queryAXTree(
      protocol::Maybe<int> dom_node_id,
      protocol::Maybe<int> backend_node_id,
      protocol::Maybe<String> object_id,
      protocol::Maybe<String> accessibleName,
      protocol::Maybe<String> role,
      std::unique_ptr<protocol::Array<protocol::Accessibility::AXNode>>*)
      override;

 private:
  // Unconditionally enables the agent, even if |enabled_.Get()==true|.
  // For idempotence, call enable().
  void EnableAndReset();
  std::unique_ptr<protocol::Array<AXNode>> WalkAllAXNodes(Document* document);
  std::unique_ptr<protocol::Array<AXNode>> WalkAXNodesToDepth(
      Document* document,
      int max_depth);
  std::unique_ptr<AXNode> BuildObjectForIgnoredNode(
      Node* dom_node,
      AXObject*,
      bool fetch_relatives,
      std::unique_ptr<protocol::Array<AXNode>>& nodes,
      AXObjectCacheImpl&) const;
  void PopulateDOMNodeAncestors(Node& inspected_dom_node,
                                AXNode&,
                                std::unique_ptr<protocol::Array<AXNode>>& nodes,
                                AXObjectCacheImpl&) const;
  std::unique_ptr<AXNode> BuildProtocolAXObject(
      AXObject&,
      AXObject* inspected_ax_object,
      bool fetch_relatives,
      std::unique_ptr<protocol::Array<AXNode>>& nodes,
      AXObjectCacheImpl&) const;
  void FillCoreProperties(AXObject&,
                          AXObject* inspected_ax_object,
                          bool fetch_relatives,
                          AXNode&,
                          std::unique_ptr<protocol::Array<AXNode>>& nodes,
                          AXObjectCacheImpl&) const;
  void AddAncestors(AXObject& first_ancestor,
                    AXObject* inspected_ax_object,
                    std::unique_ptr<protocol::Array<AXNode>>& nodes,
                    AXObjectCacheImpl&) const;
  void PopulateRelatives(AXObject&,
                         AXObject* inspected_ax_object,
                         AXNode&,
                         std::unique_ptr<protocol::Array<AXNode>>& nodes,
                         AXObjectCacheImpl&) const;
  void AddSiblingsOfIgnored(
      std::unique_ptr<protocol::Array<AXNodeId>>& child_ids,
      AXObject& parent_ax_object,
      AXObject* inspected_ax_object,
      std::unique_ptr<protocol::Array<AXNode>>& nodes,
      AXObjectCacheImpl&) const;
  void addChild(std::unique_ptr<protocol::Array<AXNodeId>>& child_ids,
                AXObject& child_ax_object,
                AXObject* inspected_ax_object,
                std::unique_ptr<protocol::Array<AXNode>>& nodes,
                AXObjectCacheImpl&) const;
  void AddChildren(AXObject&,
                   AXObject* inspected_ax_object,
                   std::unique_ptr<protocol::Array<AXNodeId>>& child_ids,
                   std::unique_ptr<protocol::Array<AXNode>>& nodes,
                   AXObjectCacheImpl&) const;
  LocalFrame* FrameFromIdOrRoot(const protocol::Maybe<String>& frame_id);
  void RetainAXContextForDocument(Document* document);

  Member<InspectedFrames> inspected_frames_;
  Member<InspectorDOMAgent> dom_agent_;
  InspectorAgentState::Boolean enabled_;

  // The agent needs to keep AXContext because it enables caching of a11y nodes.
  HeapHashMap<WeakMember<Document>, std::unique_ptr<AXContext>>
      document_to_context_map_;

  DISALLOW_COPY_AND_ASSIGN(InspectorAccessibilityAgent);
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_ACCESSIBILITY_INSPECTOR_ACCESSIBILITY_AGENT_H_

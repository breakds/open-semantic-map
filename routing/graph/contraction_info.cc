#include "graph/contraction_info.h"

#include "spdlog/spdlog.h"

namespace open_semap {
namespace graph {

void WriteNodeOrder(const std::vector<VertexID> &ordered_vertex_ids,
                    osmium::io::Writer *writer) {
  constexpr size_t kDefaultBufferSize = 1024 * 20;  // By default, 20KB per buffer.
  constexpr size_t kNumPerBuffer      = 256;        // Maximum nodes per buffer.

  for (size_t i = 0; i < ordered_vertex_ids.size(); ++i) {
    osmium::memory::Buffer output_buffer(kDefaultBufferSize,
                                         osmium::memory::auto_grow::yes);
    size_t end = std::min(i + kNumPerBuffer, ordered_vertex_ids.size());
    for (; i < end; ++i) {
      osmium::builder::NodeBuilder builder(output_buffer);
      builder.set_id(ordered_vertex_ids[i]);
    }
    (*writer)(std::move(ouptut_buffer);
  }
}

void WriteShortcuts(const std::vector<std::unique_ptr<graph::Edge>> &shortcuts,
                    osmium::io::Writer *writer) {
  constexpr size_t kDefaultBufferSize = 1024 * 20;  // By default, 20KB per buffer.
  constexpr size_t kNumPerBuffer      = 128;        // Maximum shortcuts per buffer.

  for (size_t i = 0; i < shortcuts.size(); ++i) {
    osmium::memory::Buffer output_buffer(kDefaultBufferSize,
                                         osmium::memory::auto_grow::yes);
    size_t end = std::min(i + kNumPerBuffer, shortcuts.size());
    for (; i < end; ++i) {
      if (shortcuts[i]->car == nullptr || shortcuts[i]->cdr == nullptr ||
          shortcuts[i]->car()->to().id() != shortcuts[i]->cdr()->from().id()) {
        spdlog::warn("WriteShortcuts: Skip edge {} -> {} as it is not a valid shortcut.",
                     shortcuts[i]->from().id(), shortcuts[i]->to().id());
      }
      osmium::builder::RelationBuilder builder(output_buffer);
      osmium::builder::RelationMemberListBuilder member_builder(output_buffer, &builder);
      member_builder.add_member(osmium::item_type::node, shortcuts[i]->car()->from().id(),
                                "shortcut_from");
      member_builder.add_member(osmium::item_type::node, shortcuts[i]->car()->to().id(),
                                "shortcut_via");
      member_builder.add_member(osmium::item_type::node, shortcuts[i]->cdr()->to().id(),
                                "shortcut_to");
    }
  }
}

void ContractionInfo::Save(const std::string &path) const {
  osmium::io::Header header = reader.header();
  header.set("generator", "contraction_info");
  osmium::io::Writer writer(path, header, osmium::io::overwrite::allow);
  WriteNodeOrder(ordered_vertex_ids, &writer);
  WriteShortcuts(shortcuts, &writer);
}

}  // namespace graph
}  // namespace open_semap

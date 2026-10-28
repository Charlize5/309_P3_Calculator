// src/tools/tool_registry.cpp — YOU IMPLEMENT (spec §4.3).
//
// A hand-written hash table. No std::unordered_map / std::map / std::set.
// Every function below is a stub so the starter compiles. Replace each body.

#include "tools/tool_registry.h"
//provided my private functions
//says we can use <utility> 
#include <utility>
#include <cstdint> //allowed for uint64_t
//64 max probe length
//kill tools once 

ToolRegistry::ToolRegistry() : buckets_(kInitialBucketCount), size_(0) {}
 
ToolRegistry::~ToolRegistry() = default;   // unique_ptrs in the buckets free the tools
 
// 64 bit (From partA).
// returns the full-width value
std::size_t ToolRegistry::hash(std::string_view key) noexcept {
    std::uint64_t h = 1469598103934665603ULL;      // FNV offset basis, copied from the specs 
    for (unsigned char c : key) { //no sign extension 
        h ^= c; // XOR with the byte
        h *= 1099511628211ULL;                     // FNV prime num from specs
    }
    return static_cast<std::size_t>(h); //full width value (low bits for index)
}
 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::size_t ToolRegistry::index_for(std::string_view key, std::size_t bucket_total) noexcept {
    return hash(key) & (bucket_total - 1);         // because bucket_total is a power of two
}
 
void ToolRegistry::rehash(std::size_t new_bucket_count) {
    std::vector<std::vector<Entry>> fresh(new_bucket_count);
    for (auto& bucket : buckets_) {
        for (Entry& e : bucket) {
            std::size_t i = index_for(e.name, new_bucket_count);   // index changes with the size
            fresh[i].push_back(std::move(e));                      // moves the pointer only
        }
    }
    buckets_ = std::move(fresh);
}
 
bool ToolRegistry::add(std::unique_ptr<Tool> tool) {
    if (!tool) return false;
 
    std::string name = tool->name();
 
    // a 'rejected; add must not trigger a resize. kills the tool 
    if (find(name) != nullptr) return false;
 
    // forces load_factor() <= 0.75 (< 1.0) 
    if (static_cast<double>(size_ + 1) >
        kMaxLoadFactor * static_cast<double>(buckets_.size())) {
        rehash(buckets_.size() * 2);
    }
 
    std::size_t i = index_for(name, buckets_.size());
    buckets_[i].push_back(Entry{std::move(name), std::move(tool)});
    ++size_;
    return true;
}
 
Tool* ToolRegistry::find(std::string_view name) const { 
    const auto& bucket = buckets_[index_for(name, buckets_.size())];
    for (const Entry& e : bucket)
        if (e.name == name) return e.tool.get(); //valid for the whole lifespam of the registry
    return nullptr; 
}
 
std::size_t ToolRegistry::size() const noexcept { return size_; } 
 
void ToolRegistry::for_each(const std::function<void(const Tool&)>& fn) const {
    for (const auto& bucket : buckets_)
        for (const Entry& e : bucket)
            fn(*e.tool); 
}
 
std::size_t ToolRegistry::bucket_count() const noexcept { return buckets_.size(); }
 
double ToolRegistry::load_factor() const noexcept {
    return static_cast<double>(size_) / static_cast<double>(buckets_.size());
}
 
std::size_t ToolRegistry::max_probe_length() const {
    std::size_t longest = 0;                       // empty table -> 0
    for (const auto& bucket : buckets_)
        if (bucket.size() > longest) longest = bucket.size();
    return longest;
}
